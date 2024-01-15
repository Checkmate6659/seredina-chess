#include "search.hpp"
#include "chess.hpp"
#include "order.hpp" //move scoring
#include "posix.hpp" //kbhit equivalent on linux
#include "tt.hpp"

uint64_t nodes = 0;
uint64_t max_nodes = UINT64_MAX; //only used when SEARCH_NODES is on
clock_t search_end_time;
bool panic = false;

Move killers[MAX_DEPTH][2];

int lmr_table[MAX_DEPTH][constants::MAX_MOVES];
void init_search_tables()
{
    for (int depth = 0; depth < MAX_DEPTH; depth++)
    {
        for (int i = 0; i < constants::MAX_MOVES; i++)
        {
            if (depth == 0 || i == 0) //for safety! (also to avoid doing log(0))
                lmr_table[depth][i] = 0;
            else //log*log formula (TODO: tune constants!)
                lmr_table[depth][i] = std::round(0.8 + log(depth) * log(i) * 0.4);
        }
    }
}

//clear killer move table and history
void clear_small_tables()
{
    //reset killers
    for (int i = 0; i < MAX_DEPTH; i++) killers[i][0] = killers[i][1] = Move::NO_MOVE;

    //reset histories to 0 (note: starting value can be adjusted to minimize clampings)
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 64; j++)
            hist[i][j] = 0;
}

//clear hash table
void clear_hash()
{
    for (int i = 0; i < hash_size; i++) hash_table[i] = {};
}

Value quiesce(W_Board &board, Value alpha, Value beta)
{
    //node budget for "go nodes" command
#ifdef SEARCH_NODES
    if (nodes >= max_nodes)
    {
        panic = true;
        return PANIC_VALUE;
    }
#endif

    //stand pat
    Value static_eval = eval(board);
    if (static_eval > alpha)
    {
        alpha = static_eval;
        if (alpha >= beta)
            return alpha; //no effect of fail soft here
    }

    Movelist moves;
    //only generate captures
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);

    /* HASHE* phashe = ProbeHash(board, MAX_DEPTH); //qs TT (TODO: test!!!)
    if (phashe != nullptr) //we have a hit (NO depth check here!)
    {
        if (phashe->flags == hashfEXACT) //exact hit! great
            return phashe->val;
        else if ((phashe->flags == hashfALPHA) && //window resizing!
            (phashe->val < beta))
            beta = phashe->val;
        else if ((phashe->flags == hashfBETA) &&
            (phashe->val > alpha)) //same
            alpha = phashe->val;

        if (alpha >= beta) //tt cutoff
            return alpha;
    } */
    //TODO: use TT move here as well?

    Move best_move = Move::NO_MOVE; //best move (for TT)
    score_moves_quiesce(board, moves);
    for (int i = 0; i < moves.size(); i++) {
        pick_move(moves, i); //get the best-scored move to the index i
        const auto move = moves[i];

        //SEE pruning: ignore any SEE-losing move (do trades tho)
        if (!SEE(board, move, QS_SEEPRUNE_THRESH))
            continue;

        board.makeMove(move);
        nodes++; //1 move made = 1 node
        Value cur_score = -quiesce(board, -beta, -alpha);
        board.unmakeMove(move);

        if (cur_score > alpha)
        {
            alpha = cur_score;
            best_move = move;
            if (cur_score >= beta) //beta cutoff (fail soft)
            {
                // RecordHash(board, 0, alpha, hashfBETA, move, MAX_DEPTH);
                return cur_score; //no effect of fail soft here
            }
        }
    }

    //Storing tt_move instead of best_move when failing low makes like 0 change
    /* uint8_t hashf = (best_move == Move::NO_MOVE) ? hashfALPHA : hashfEXACT;
    RecordHash(board, 0, alpha, hashf, best_move, MAX_DEPTH); */

    return alpha;
}

Value search(W_Board& board, int depth, Value alpha, Value beta, SearchStack* ss)
{
    if (panic || !(nodes & 0xFFF)) //check for panic every 4096 nodes
        if (panic || clock() > search_end_time ||
        (!(nodes & 0xFFFF) && kbhit())) //make costly key check extra rare
        {
            panic = true;
            return PANIC_VALUE; //PANIC; NOTE: negating INT32_MIN is UB!!!
        }

    //are we in a PV-node? (useless for now)
    bool pv_node = (beta - alpha > 1);

    //test if we are in check
    bool incheck = board.inCheck();

    //original depth is const; to keep original depth value
    depth += incheck; //check extension

    if (depth <= 0 && ss->ply != 0) //don't get in qsearch when we are in check!
        return quiesce(board, alpha, beta);

    //check repetition BEFORE probing (no worry about priority for repetition!!!)
    if (board.isRepetition(1) && ss->ply != 0)
        return DRAW;

    //probe hash table
    //https://gitlab.com/mhouppin/stash-bot/-/blob/8ec0469cdcef022ee1bc304299f7c0e3e2674652/sources/engine/search_bestmove.c
    Move tt_move = Move::NO_MOVE; //tt miss => it will stay like this

    HASHE* phashe = ProbeHash(board, ss->ply);
    if (phashe != nullptr /* && board.halfMoveClock() <= 60 */) //we have a hit
    {
        //entry has enough depth
        if (phashe->depth >= depth && ss->ply >= 1) { //Test this with ss->ply >= 1 instead
            if (phashe->flags == hashfEXACT) //exact hit! great
                return phashe->val;
            else if ((phashe->flags == hashfALPHA) && //window resizing!
                (phashe->val < beta))
                beta = phashe->val;
            else if ((phashe->flags == hashfBETA) &&
                (phashe->val > alpha)) //same
                alpha = phashe->val;

            if (alpha >= beta)
                return alpha; //hit with a bound
        } //phashe->depth >= depth

        //this is executed even when we can't return from search immediately
        tt_move = Move(phashe->best); //write best move out of there
    }
    else //TT miss
    {
        //IIR (reducing by 1)
        /* if (depth >= 3 && pv_node)
        {
            depth -= 1;
        } */
    }

    //Speculative prunings (NMP, RFP, ...)
    if (!pv_node && ss->ply != 0)
    {
        //NMP: enough depth, not in check, no zugzwang condition
        if (!incheck && board.hasNonPawnMaterial(board.sideToMove()))
        {
            int reduced_depth = depth - 2; //constant R = 2
            reduced_depth = std::max(reduced_depth, 1); //don't use depth < 1

            board.makeNullMove(); //make null move
            ss->ply++; //inc ply, so that the indices into stuff is correct

            Value nmp_val = -search(board, reduced_depth - 1, -beta, -alpha, ss);

            ss->ply--;
            board.unmakeNullMove();

            if (nmp_val >= beta) //null move search failed high
                //TODO: experiment with clamping it by beta + some constant
                return std::min(nmp_val, 9999); //don't return bad mate scores!
        }
    }

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.size() == 0 && ss->ply != 0) //no legal moves
        return incheck ? (ss->ply + 128 - INT32_MAX) : DRAW; //return checkmate or stalemate
    if (board.isHalfMoveDraw() && ss->ply != 0) //repetitions or 50-move rule
        return DRAW;

    //score moves
    score_moves(board, moves, tt_move, killers[ss->ply]);

    Move best_move = Move::NO_MOVE; //for hash table (if fail low, best move unknown)
    for (int i = 0; i < moves.size(); i++) {
        pick_move(moves, i); //get the best-scored move to the index i
        const auto move = moves[i];

        //Don't SEE-prune first move (leave possibility if all moves losing)
        //also, more chance of most/all legal moves losing material when in check
        if (i != 0 && !incheck && depth <= 7 && ss->ply >= 2)
        {
            //TODO: decrease multiplier after doing LMR
            int32_t see_threshold = (depth + 1) * 100; //ultra BASIC formula
            //SEE pruning: ignore any big sacrifices
            if (!SEE(board, move, -see_threshold))
                continue;
        }

        board.makeMove(move);
        nodes++; //1 move made = 1 node
        ss->ply++;

        //does current move give check?
        bool gives_check = board.inCheck();

        Value cur_score;
        //PVS; TODO: try exclude nodes with alpha TT flag or a TT miss
        if (i == 0)
        //if (alpha < 256 - INT32_MAX || move == tt_move) //mate score warning! (???)
        {
            cur_score = -search(board, depth - 1, -beta, -alpha, ss);
        }
        else //not first move
        {
            //ZWS with LMR if proper conditions
            int lmr = 0; //LMR reduction
            //these are LMR conditions
            if (depth >= 3 && i >= 4 //don't LMR good captures and promos either
                && move.score() < 0x7810 /* && !incheck && !gives_check */)
            {
                lmr = lmr_table[depth][i]; //use precalculated table
                // lmr -= pv_node; //reduce less in PV-node

                lmr = std::max(std::min(lmr, depth - 2), 0); //don't negative-reduce
            }
            cur_score = -search(board, depth - 1 - lmr, -alpha - 1, -alpha, ss);

            if (cur_score > alpha && (lmr > 0 || cur_score < beta)) //beat alpha: re-search
            {
                cur_score = -search(board, depth - 1, -beta, -alpha, ss);
            }
        }

        ss->ply--;
        board.unmakeMove(move);

        //score checks probably useless! (still, avoid storing PANIC_VALUE in TT!)
        if (panic || cur_score == PANIC_VALUE || cur_score == -PANIC_VALUE)
        {
            if (ss->ply != 0) //if not at root, just keep the panic chain going
                return PANIC_VALUE;
            else
                return alpha; //allow partial search results to be returned
        }

        if (cur_score > alpha)
        {
            alpha = cur_score;
            best_move = move; //update best move
            if (ss->ply == 0) //get best root move (IMPORTANT!)
                ss->best_root_move = move;

            if (!board.isCapture(move))
            {
                //boost history
                boost_hist(board.at<Piece>(move.from()), move.to(), depth);
            }

            if (cur_score >= beta) //beta cutoff (fail soft)
            {
                //killer move update: quiet move; avoid duplicate killers
                //(TODO: test if better or worse)
                if (!board.isCapture(move) && move != killers[ss->ply][0])
                {
                    killers[ss->ply][1] = killers[ss->ply][0];
                    killers[ss->ply][0] = move;
                }

                //store in hash table (beta = lower bound flag)
                //why does fail soft give really bad results?
                RecordHash(board, depth, beta, hashfBETA, move, ss->ply);
                return cur_score; //fail soft here: no effect!
            }
        }
        else //failed low: bad move!
        {
            if (!board.isCapture(move))
            {
                //penalize history
                penal_hist(board.at<Piece>(move.from()), move.to(), depth);
            }
        }
    }
    if (!panic && alpha != PANIC_VALUE && alpha != -PANIC_VALUE)
    {
        //Storing tt_move instead of best_move when failing low makes like 0 change
        uint8_t hashf = (best_move == Move::NO_MOVE) ? hashfALPHA : hashfEXACT;
        RecordHash(board, depth, alpha, hashf, best_move, ss->ply);
    }
    return alpha;
}

//before integrating root node into search(): 406152 nodes 2434015 nps
//after: 289680 nodes 2308483 nps NOPE garbage it disconnects when threefold repetition!
//Did a *few* changes, didn't change bench: not immediately returning draw, mate or TT at root
Move search_root(W_Board &board, int alloc_time_ms, int depth)
{
    //convert from ms to clock ticks; set this up for panic return
    //NOTE: clock_t is 64-bit signed on this system, but if it is 32-bit this can overflow!
    clock_t start_time = clock();
    clock_t alloc_time_clk = alloc_time_ms * CLOCKS_PER_SEC / 1000;
    search_end_time = start_time + alloc_time_clk;

    if (depth != MAX_DEPTH) //"go depth ..." command
        search_end_time = (uint64_t)((clock_t)(-1)) >> 1; //maximum value of a clock_t

    //shift killers by 2 ply (expecting chess game conditions)
    for (int i = 0; i < MAX_DEPTH - 2; i++)
    {
        killers[i][0] = killers[i + 2][0];
        killers[i][1] = killers[i + 2][1];
    }

    nodes = 0; //reset node count
    panic = false; //reset panic flag

    Move best_move = Move::NO_MOVE; //no move
    Movelist moves;
    movegen::legalmoves(moves, board);
    best_move = moves[0]; //default move (panic; hopefully not used)

    int8_t cur_depth = 0; //starting depth - 1 (may need to be increased)

    SearchStack ss;
    ss.ply = 0;
    // ss.eval[0] = eval(board); //initial eval

    Value old_best = -INT32_MAX; //last finished iteration score, for partial search result
    while (++cur_depth <= depth && !panic)
    {
        //iterate over all legal moves, try find the best one
        Value best_score = search(board, cur_depth, 1 - INT32_MAX, INT32_MAX - 1, &ss);
        Move cur_best_move = ss.best_root_move; //get best root move out!

        //record this in TT so that next ply will immediately go for current best move
        RecordHash(board, cur_depth, best_score, hashfEXACT, best_move, 0);

        if (!panic || best_score >= old_best) //normal OR partial search results
        {
            best_move = cur_best_move; //update best move
            old_best = best_score; //update best score

            //print out all the juicy info
            uint32_t curtime = (clock() - start_time) * 1000 / CLOCKS_PER_SEC;
            std::cout << "info depth " << (int)cur_depth << " score cp " << old_best <<
                " nodes " << nodes << " time " << curtime <<
                " nps " << (curtime ? (nodes * 1000 / curtime) : 0) << " pv ";
            std::cout << uci::moveToUci(best_move) << " "; //print best move (not in TT)

            W_Board pv_board(board.getFen()); //copy board to avoid destroying it
            pv_board.makeMove(best_move); //make best move

            //extract PV from TT
            uint64_t hash = pv_board.hash();
            HASHE *phashe = ProbeHash(pv_board, 0);
            //sometimes TT entries don't have a move; also have a counter in case of repetition
            for (int i = 0; phashe != nullptr && phashe->best && i < MAX_DEPTH; i++)
            {
                Move pv_move = phashe->best;
                std::cout << uci::moveToUci(pv_move) << " "; //print pv move
                pv_board.makeMove(pv_move); //push the move
                phashe = ProbeHash(pv_board, 0); //next tt entry
            }

            std::cout << std::endl; //print newline
        }
    }

    return best_move;
}
