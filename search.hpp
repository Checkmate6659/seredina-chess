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
extern float lmp00, lmp10, lmp01, lmp11;
extern int lmp_depth;
#else
const float lmr_f1 = 0.653, lmr_f2 = 0.293; //used in LMR lookup table initialization
const int iir_depth = 2; //IIR minimum depth
const int nmp_const = 4; //NMP constant term
const int see_multiplier = 77, see_const = 63; //SEE linear parameters
const int lmr_mindepth = 3, lmr_reduceafter = 5; //min depth and first reduced move
const float lmr_pv = 0.584, lmr_improving = 0.109; //reducing less when PV and improving (TODO)
const int rfp_depth = 7, rfp_margin = 97, rfp_impr = 64; //RFP parameters (rfp_impr should be positive!)
const int aspi_width = 75; //aspiration window width
const int se_mindepth = 5, se_ttdepth_margin = 3, se_depth_mul = 3; //SE params
const int se_dbl_margin = 24, se_dbl_maxdepth = 10; //SE double extension stuff
const float lmp00 = 3.036, lmp10 = 5.136, lmp01 = 1.595, lmp11 = 3.078;
const int lmp_depth = 8;
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
