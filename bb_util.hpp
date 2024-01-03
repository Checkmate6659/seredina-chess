#ifndef BB_UTIL_H
#define BB_UTIL_H

#include "chess.hpp"
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

#endif
