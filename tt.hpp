//http://web.archive.org/web/20070809015843/www.seanet.com/%7Ebrucemo/topics/hashing.htm
#ifndef TT_H
#define TT_H

#include <cstdlib>
#include <stdint.h>
#include "chess_ext.hpp"
using namespace chess;

#define    hashfEXACT   0
#define    hashfALPHA   1
#define    hashfBETA    2

typedef struct
{
    uint64_t key;
    int8_t depth;
    uint8_t flags;
    int32_t val;
    uint16_t best;
} HASHE;

extern uint32_t hash_size;
extern HASHE* hash_table; //smaller hash table (I was using 2^24 before! that's 400MB!)

bool alloc_hash(uint32_t size_mb);
HASHE* ProbeHash(W_Board &board, int8_t ply);
void RecordHash(W_Board &board, int8_t depth, int32_t val, uint8_t flags, const Move &best_move, int8_t ply);

#endif
