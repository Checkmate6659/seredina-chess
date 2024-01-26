#ifndef ORDER_H
#define ORDER_H
#include "chess.hpp"
#include "chess_ext.hpp"
#include "bb_util.hpp"
#include <algorithm>
#include <cstdint>
using namespace chess;

#define MAX_CHIST 0x4000
#define MIN_CHIST 0x0000
#define MAX_HIST 0x0000
#define MIN_HIST (-0x8000)

//history tables
int16_t capthist[12][64][5]; //piece-to-captured
int16_t hist[12][64]; //piece-to

//pawn, knight, bishop, rook, queen
const int16_t mvv_bonus[] = { 0, 2048, 2048, 4096, 8192 };

inline void update_hist(W_Board board, Movelist &moves, Move best, int8_t depth)
{
    int delta = depth * depth; //simple depth² (experiment with this!); no overflows!
    int piece = (int)board.at(best.from());
    int to = best.to().index();
    PieceType victim = board.at<PieceType>(to);

    if (victim == PieceType::NONE) //quiet move
    {
        //boost best move
        int new_hist = hist[piece][to] + delta; //score not constrained yet!
        //constrain history score
        hist[piece][to] = std::min(MAX_HIST, new_hist); //score can only go up here
    }
    else //capture: capthist
    {
        //boost best move
        int new_hist = capthist[piece][to][(int)victim] + delta; //score not constrained yet!
        //constrain history score
        capthist[piece][to][(int)victim] = std::min(MAX_CHIST, new_hist); //score can only go up here
    }

    //penalize other moves (always captures; quiets only if best was quiet)
    for (int i = 0; i < moves.size(); i++)
    {
        //calculate params of new move
        Move move = moves[i];
        int m_piece = (int)board.at(move.from());
        int m_to = move.to().index();
        PieceType m_victim = board.at<PieceType>(m_to);

        if (m_victim != PieceType::NONE) //capture: always penalize
        {
            int new_hist = capthist[m_piece][m_to][(int)m_victim] - delta; //score not constrained yet!
            //constrain history score
            capthist[m_piece][m_to][(int)m_victim] = std::max(MIN_CHIST, new_hist); //score can only go down here
        }
        else if (victim == PieceType::NONE) //quiet: only penalize when best was quiet
        {
            int new_hist = hist[m_piece][m_to] - delta; //score not constrained yet!
            //constrain history score
            hist[m_piece][m_to] = std::max(MIN_HIST, new_hist); //score can only go down here
        }
    }
}

//Give a score to all the moves (don't order them immediately!)
inline void score_moves(W_Board &board, Movelist &moves, Move &tt_move, Move* cur_killers)
{
    //WARNING: move scores in chess-library are int16_t, so careful with 32-bit hist
    //Also it goes from -32768 to 32767; there are negative values!
    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];

        if (moves[i] == tt_move) //TT MOVE!!!
            moves[i].setScore(0x7FFF); //highest score
        //score promotions! (no library function for that tho); only up queen promos a lot
        else if (
            moves[i].promotionType() == PieceType::QUEEN //promotes to a queen (might be enough by itself?)
            && board.at<PieceType>(moves[i].from()) == PieceType::PAWN //moved a pawn
            && moves[i].to().rank() % 7 == 0) //to a back rank
        {
            //like MVV-LVA really
            PieceType victim = board.at<PieceType>(moves[i].to());
            moves[i].setScore(0x7FF8 + (int)victim); //really high score!
        }
        else if (board.isCapture(moves[i]))
        {
            //MVV (no LVA)
            PieceType victim = board.at<PieceType>(moves[i].to());
            // PieceType aggressor = board.at<PieceType>(moves[i].from());

            //TODO: try searching these later!
            int16_t bonus = SEE(board, moves[i], -1) ? 0x2000 : -0x1000;
            int16_t score = bonus + mvv_bonus[(int)victim];

            score += capthist //piece-to-captured capthist score (we have to cap it tho)
                [(int)board.at<Piece>(moves[i].from())]
                [moves[i].to().index()][board.at<PieceType>(moves[i].to())];

            moves[i].setScore(score);
        }
        else if (move == cur_killers[0])
        {
            moves[i].setScore(0x0002);
        }
        else if (move == cur_killers[1])
        {
            moves[i].setScore(0x0001);
        }
        else
        {
            moves[i].setScore(hist //piece-to hist score (we have to cap it tho)
                [(int)board.at<Piece>(moves[i].from())]
                [moves[i].to().index()]);
        }
    }
}

//Same for qsearch
inline void score_moves_quiesce(W_Board &board, Movelist &moves)
{
    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];

        if (board.isCapture(moves[i]))
        {
            //MVV-LVA
            PieceType victim = board.at<PieceType>(moves[i].to());
            PieceType aggressor = board.at<PieceType>(moves[i].from());
            moves[i].setScore(0x4010 + (int)victim * 16 - (int)aggressor);
        }
        else //could get rid of this; keeping it in for safety
        {
            moves[i].setScore(-32000);
        }
    }
}

//swap moves[i] with the best scored move after i
//TODO: look at Movelist.sort(int index = 0)
inline void pick_move(Movelist &moves, int i)
{
    Move moves_i = moves[i];
    int16_t best_score = moves_i.score();
    int best_index = i;

    //search for highest-scored move after this one
    for (int j = i + 1; j < moves.size(); j++)
    {
        const auto move = moves[j];
        if (move.score() > best_score)
        {
            best_score = move.score();
            best_index = j;
        }
    }

    //swap it in!
    moves[i] = moves[best_index];
    moves[best_index] = moves_i;
}
#endif
