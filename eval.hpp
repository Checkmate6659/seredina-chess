#ifndef EVAL_H
#define EVAL_H

#include <cstdint>

#include "chess.hpp"
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

//QA is useful, QB is only for scaling; NOTE: screlu181 won't be compiler optimized
#define QA 255
#define QB 64
#define NN_SCALE 400

//CReLU (between 0 and qmax, which is either QA or QB)
inline int crelu(int input, int qmax)
{
    return std::min(std::max(input, 0), qmax);
}

void init_tables();
Value eval(Board board);

#endif
