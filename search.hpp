#ifndef SEARCH_H
#define SEARCH_H

//#define __NOISY_DRAW //add a bit of noise to draw score to avoid repetition

#include <iostream>
#include <cstdint>
#include <ctime>
#include <cmath>

#include "chess_ext.hpp"
#include "eval.hpp"
using namespace chess;

#define TUNING //enable tuning mode
//#define SEARCH_NODES //enable "go nodes"; maybe slows down engine a bit?

#define PANIC_VALUE INT32_MAX
#ifdef __NOISY_DRAW
#define DRAW ((nodes & 3) - 1) //pseudo-random in [[-1, 2]]
#else
#define DRAW 0 //deterministic; more chance of repetition
#endif

extern uint64_t nodes, max_nodes;
#define MAX_DEPTH 96 //can't be as high as 127! otherwise we can get infinite-looped!
#define QS_SEEPRUNE_THRESH (-1) //any strictly SEE-losing move is pruned (could also be 0)

//parameters: extern when tuning, const when not tuning
#ifdef TUNING
#define PARAM extern
#else
#define PARAM const
#endif

PARAM float lmr_f1, lmr_f2; //used in LMR lookup table initialization
PARAM int iir_depth; //IIR minimum depth
PARAM int nmp_const; //NMP constant term
PARAM int see_multiplier, see_const; //SEE linear parameters

typedef struct {
    int8_t ply;
    Move best_root_move;
    Value eval[MAX_DEPTH];
    //TODO: add killers and history here, make this suitable for multithreading later!
} SearchStack;

void init_search_tables();
bool alloc_hash(uint32_t size_mb); //repeating alloc_hash prototype here, to use in main
void clear_hash(); //WARNING: this costs a lot of time, as it clears the entire TT!
void clear_small_tables(); //killers, hist

Value quiesce(W_Board &board, Value alpha, Value beta, SearchStack* ss);
Value search(W_Board &board, int depth, Value alpha, Value beta);
Move search_root(W_Board &board, int alloc_time_ms, int depth);


#endif
