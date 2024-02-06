#ifndef ORDER_H
#define ORDER_H
#include "chess_ext.hpp"
#include "bb_util.hpp"
#include <algorithm>
#include <cstdint>
using namespace chess;

//target bench: 530864
#define MAX_HIST 0x3800//0x3FFFFC00
#define MIN_HIST (-0x4000)//(-0x40000000)
#define MAX_CONTHIST (0x3800)//0x3FFFFC00
#define MIN_CONTHIST (-0x4000)//(-0x40000000)

//history table (piece; to)
int32_t hist[12][64];
//conthist table (an enemy move first (uncolored piece), and our own move later)
int32_t conthist[6][64][12][64];
uint16_t cm_heuristic[12][64]; //countermove table (piece; to)

inline void boost_hist(Piece piece, Square to, int8_t depth)
{
    int boost = depth * depth; //simple depth² (experiment with this!); no overflows!
    int new_hist = hist[(int)piece][to.index()] + boost; //score not constrained yet!
    //constrain history score
    hist[(int)piece][to.index()] = std::min(MAX_HIST, new_hist); //score can only go up here
}

inline void penal_hist(Piece piece, Square to, int8_t depth)
{
    int penalty = depth * depth; //simple depth² (experiment with this!); no overflows!
    int new_hist = hist[(int)piece][to.index()] - penalty; //score not constrained yet!
    //constrain history score
    hist[(int)piece][to.index()] = std::max(MIN_HIST, new_hist); //score can only go down here
}

inline void boost_conthist(W_Board &board, const Move &move, int8_t depth)
{
    if (board.move_history.empty()) return; //don't segfault!

    int boost = depth * depth;
    std::pair<Piece, uint16_t> last_move = board.move_history[board.move_history.size() - 1];
    int new_conthist = conthist[(int)last_move.first.type()][(new Move(last_move.second))->to().index()]
        [(int)board.at<Piece>(move.from())][move.to().index()] + boost; //score not constrained yet!
    //constrain history score
    conthist[(int)last_move.first.type()][(new Move(last_move.second))->to().index()]
        [(int)board.at<Piece>(move.from())][move.to().index()] =
        std::min(MAX_CONTHIST, new_conthist); //score can only go up here
}

inline void penal_conthist(W_Board &board, const Move &move, int8_t depth)
{
    if (board.move_history.empty()) return; //don't segfault!

    int penalty = depth * depth;
    std::pair<Piece, uint16_t> last_move = board.move_history[board.move_history.size() - 1];
    int new_conthist = conthist[(int)last_move.first.type()][(new Move(last_move.second))->to().index()]
        [(int)board.at<Piece>(move.from())][move.to().index()] - penalty; //score not constrained yet!
    //constrain history score
    conthist[(int)last_move.first.type()][(new Move(last_move.second))->to().index()]
        [(int)board.at<Piece>(move.from())][move.to().index()] =
        std::max(MIN_CONTHIST, new_conthist); //score can only go up here
}

//Give a score to all the moves (don't order them immediately!)
inline void score_moves(W_Board &board, W_Movelist &moves, Move &tt_move, Move* cur_killers)
{
    //WARNING: move scores in chess-library are int16_t, so careful with 32-bit hist
    //Also it goes from -32768 to 32767; there are negative values!
    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];

        if (moves[i] == tt_move) //TT MOVE!!!
            moves.scores[i] = 0x7FFFFFFF; //highest score
        //score promotions! only up queen promos a lot (the rest is most likely worse)
        else if (moves[i].typeOf() == Move::PROMOTION &&
            moves[i].promotionType() == PieceType::QUEEN) //promotes to a queen (might be enough by itself?)
        {
            //like MVV-LVA really
            PieceType victim = board.at<PieceType>(moves[i].to());
            moves.scores[i] = 0x7FFFFFF8 + (int)victim; //really high score!
        }
        else if (board.isCapture(moves[i]))
        {
            //MVV-LVA
            PieceType victim = board.at<PieceType>(moves[i].to());
            PieceType aggressor = board.at<PieceType>(moves[i].from());

            //TODO: try searching these later!
            int32_t bonus = SEE(board, moves[i], -1) ? 0x7810 : 0x7001;
            moves.scores[i] = bonus + (int)victim * 12 - (int)aggressor;
            // if ((int)victim * 12 - (int)aggressor < -10) printf("wtf"); //nothing here
        }
        //killers
        else if (move == cur_killers[0])
        {
            moves.scores[i] = 0x7803;
        }
        else if (move == cur_killers[1])
        {
            moves.scores[i] = 0x7802;
        }
        //countermove heuristic
        else if (board.move_history.size() >= 1 &&
            move.move() == cm_heuristic[(int)board.move_history[board.move_history.size() - 1].first]
            [(new Move(board.move_history[board.move_history.size() - 1].second))->to().index()])
            moves.scores[i] = 0x7801;
        else
        {
            int32_t hist_val = hist //piece-to hist score (we have to cap it tho)
                [(int)board.at<Piece>(moves[i].from())]
                [moves[i].to().index()];

            int32_t conthist_val = 0;
            if (board.move_history.size() >= 1) //don't segfault!
            {
                //conthist: index by current move as well as last move
                std::pair<Piece, uint16_t> last_move = board.move_history[board.move_history.size() - 1];

                conthist_val = conthist[(int)last_move.first.type()][(new Move(last_move.second))->to().index()]
                    [(int)board.at<Piece>(moves[i].from())][moves[i].to().index()];
            }

            moves.scores[i] = hist_val + conthist_val;
        }
    }
}

//Same for qsearch
inline void score_moves_quiesce(W_Board &board, W_Movelist &moves)
{
    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];

        if (board.isCapture(moves[i]))
        {
            //MVV-LVA
            PieceType victim = board.at<PieceType>(moves[i].to());
            PieceType aggressor = board.at<PieceType>(moves[i].from());
            moves.scores[i] = 0x4010 + (int)victim * 16 - (int)aggressor;
        }
        else //could get rid of this; keeping it in for safety
        {
            moves.scores[i] = -32000;
        }
    }
}

//swap moves[i] with the best scored move after i
//TODO: look at Movelist.sort(int index = 0)
inline void pick_move(W_Movelist &moves, int i)
{
    Move moves_i = moves[i];
    int32_t best_score = moves.scores[i];
    int best_index = i;

    //search for highest-scored move after this one
    for (int j = i + 1; j < moves.size(); j++)
    {
        const auto move = moves[j];
        const int32_t cur_score = moves.scores[j];
        if (cur_score > best_score)
        {
            best_score = cur_score;
            best_index = j;
        }
    }

    //swap it in!
    moves[i] = moves[best_index];
    moves[best_index] = moves_i;
    moves.scores[best_index] = moves.scores[i];
    moves.scores[i] = best_score;
}
#endif
