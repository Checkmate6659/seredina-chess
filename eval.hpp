#ifndef EVAL_H
#define EVAL_H

#include <cstdint>

#include "chess_ext.hpp"
#include "bb_util.hpp"
using namespace chess;

//pack 2 16-bit signed values into a 32-bit value
#define S(mg__, eg__) (((uint16_t)(mg__) << 16) | ((uint16_t)(eg__) & 0xFFFF))
//extract MG value from a S pair (upper 16 bits)
#define MG(s_pair__) ((int16_t)((s_pair__) >> 16))
//extract EG value from a S pair (lower 16 bits)
#define EG(s_pair__) ((int16_t)(s_pair__))

typedef int32_t Value;
typedef int32_t ValPair; //for S pairs (signed so that we can narrow things properly)

//CReLU (between 0 and 1)
//TODO: change when quantizing
inline float crelu(float input)
{
    return std::min(std::max(input, (float)0), (float)1);
}

void init_tables();
Value eval(W_Board board);

#endif
