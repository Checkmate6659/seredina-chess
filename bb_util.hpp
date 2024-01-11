#ifndef BB_UTIL_H
#define BB_UTIL_H

#include "chess_ext.hpp"
using namespace chess;

//Big pile of bitboard helper functions
//https://www.chessprogramming.org/General_Setwise_Operations#OneStepOnly
typedef Bitboard U64;
inline U64 soutOne (U64 b) {return  b >> 8;}
inline U64 nortOne (U64 b) {return  b << 8;}
const U64 notAFile = 0xfefefefefefefefe; // ~0x0101010101010101
const U64 notHFile = 0x7f7f7f7f7f7f7f7f; // ~0x8080808080808080
inline U64 eastOne (U64 b) {return (b << 1) & notAFile;}
inline U64 noEaOne (U64 b) {return (b << 9) & notAFile;}
inline U64 soEaOne (U64 b) {return (b >> 7) & notAFile;}
inline U64 westOne (U64 b) {return (b >> 1) & notHFile;}
inline U64 soWeOne (U64 b) {return (b >> 9) & notHFile;}
inline U64 noWeOne (U64 b) {return (b << 7) & notHFile;}
inline //https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)
U64 wPawnEastAttacks(U64 wpawns) {return noEaOne(wpawns);}
inline U64 wPawnWestAttacks(U64 wpawns) {return noWeOne(wpawns);}
inline U64 bPawnEastAttacks(U64 bpawns) {return soEaOne(bpawns);}
inline U64 bPawnWestAttacks(U64 bpawns) {return soWeOne(bpawns);}
inline U64 wPawnAttacks(U64 wpawns) {return wPawnEastAttacks(wpawns) | wPawnWestAttacks(wpawns);}
inline U64 bPawnAttacks(U64 bpawns) {return bPawnEastAttacks(bpawns) | bPawnWestAttacks(bpawns);}

constexpr static uint16_t SEEValues[6] = {
    100, 450, 450, 650, 1250, 16384
};

// Static Exchange Evaluation (from Weiss)
//Returns true if the exchange is better than threshold, and false if it's (strictly) worse
inline bool SEE(const W_Board &board, const Move &move, const int32_t threshold)
{
    Square to = move.to();
    Square from = move.from();

    // Making the move and not losing it must beat the threshold
    int value = SEEValues[board.at<PieceType>(to)] - threshold;
    if (value < 0) return false;

    // Trivial if we still beat the threshold after losing the piece
    value -= SEEValues[board.at<PieceType>(from)];
    if (value >= 0) return true;

    // It doesn't matter if the to square is occupied or not
    U64 occupied = board.occ() ^ (1ULL << from.index());
    U64 attackers = //calculate attackers & defenders
        attacks::attackers(board, Color::WHITE, to, occupied) |
        attacks::attackers(board, Color::BLACK, to, occupied);

    U64 bishops = board.pieces(PieceType::BISHOP) | board.pieces(PieceType::QUEEN);
    U64 rooks   = board.pieces(PieceType::ROOK) | board.pieces(PieceType::QUEEN);

    Color side = !board.at(from).color();

    // Make captures until one side runs out, or fail to beat threshold
    while (true) {

        // Remove used pieces from attackers
        attackers &= occupied;

        U64 myAttackers = attackers & board.us(side);
        if (!myAttackers) break; // Dammit we ran out of pieces!

        // Pick next least valuable piece to capture with
        int pt; //current lowest value
        for (pt = 0; pt < 6; pt++)
            if (myAttackers & board.pieces(PieceType((PieceType::underlying)pt)))
                break;

        side = !side;

        // Value beats threshold, or can't beat threshold (negamaxed)
        if ((value = -value - 1 - SEEValues[pt]) >= 0)
        {
            //if we captured with the king, and there is an enemy piece: switch winning side
            if (pt == 5 && (attackers & board.us(side)))
                side = !side;
            //exit the loop
            break;
        }

        // Remove the used piece from occupied (take lowest index bit every time; that's 100% fine!!!)
        U64 cheapest_attackers = myAttackers & board.pieces(
            PieceType((PieceType::underlying)pt));
        occupied ^= U64::fromSquare(cheapest_attackers.lsb());

        // Add possible discovered attacks from behind the used piece (batteries!!!)
        //TODO: try redoing this only if cheapest_attackers has a single set bit
        if (!(pt & 1)) //pawn, bishop or queen (0, 2 or 4)
            attackers |= attacks::bishop(to, occupied) & bishops;
        if (pt == 3 || pt == 4) //rook or queen (3 or 4); can't do >=3 because king == 5
            attackers |= attacks::rook(to, occupied) & rooks;
    }

    return side != board.at(from).color();
}

#endif
