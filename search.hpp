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

// #define TUNING //enable tuning mode
//#define SEARCH_NODES //enable "go nodes"; maybe slows down engine a bit?

#define PANIC_VALUE INT32_MAX
#ifdef __NOISY_DRAW
#define DRAW ((nodes & 3) - 1) //pseudo-random in [[-1, 2]]
#else
#define DRAW 0 //deterministic; more chance of repetition
#endif

extern uint64_t nodes, max_nodes;
#define MAX_DEPTH 96 //can't be as high as 127! otherwise we can get infinite-looped!
#define NO_SCORE INT32_MIN
#define IS_GAME_OVER(x) (std::abs(x) > 9999)

#define QS_SEEPRUNE_THRESH (-1) //any strictly SEE-losing move is pruned (could also be 0)
#define NO_SCORE INT32_MIN

//TODO: make tunable (as well as LMP params)
#define LMP_DEPTH 8

//parameters: extern (and not initialized) when tuning, const when not tuning
#ifdef TUNING
extern float lmr_f1, lmr_f2; //used in LMR lookup table initialization
extern int iir_depth; //IIR minimum depth
extern int nmp_const; //NMP constant term
extern int see_multiplier, see_const; //SEE linear parameters
extern int lmr_mindepth, lmr_reduceafter; //min depth and first reduced move
extern float lmr_pv, lmr_improving; //reducing less when PV and improving (TODO)
extern int rfp_depth, rfp_margin, rfp_impr;
extern int aspi_width; //aspiration window width
extern int se_mindepth, se_ttdepth_margin, se_depth_mul; //SE params
extern int se_dbl_margin, se_dbl_maxdepth; //SE double extension stuff
#else
const float lmr_f1 = 0.766, lmr_f2 = 0.243; //used in LMR lookup table initialization
const int iir_depth = 1; //IIR minimum depth
const int nmp_const = 3; //NMP constant term
const int see_multiplier = 78, see_const = 77; //SEE linear parameters
const int lmr_mindepth = 2, lmr_reduceafter = 3; //min depth and first reduced move
const float lmr_pv = 0.458, lmr_improving = 0.03; //reducing less when PV and improving (TODO)
const int rfp_depth = 6, rfp_margin = 125, rfp_impr = -4; //RFP parameters (rfp_impr should be positive!)
const int aspi_width = 32; //aspiration window width
const int se_mindepth = 5, se_ttdepth_margin = 3, se_depth_mul = 2; //SE params
const int se_dbl_margin = 12, se_dbl_maxdepth = 10; //SE double extension stuff
#endif

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
