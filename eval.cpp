#include "eval.hpp"

#include "nn_values_quant_16bit.hpp" //NN weights/biases

//before quantization: 430799 nodes 1212947 nps
//after quantization:  473325 nodes 1120383 nps
//after changing QA = 16383 and QB = 256: GARBAGE
//QA = 4095 and QB = 128: 462146 nodes 1747448 nps

/* #define EVALHASH_SIZE (1<<18) //real size is 8*this in bytes (2MB here)
uint64_t evalhash[EVALHASH_SIZE]; */

#define HL_SIZE 16 //hidden layer size
//NNUE accumulator
//TODO: UE = incremental update
typedef struct {
    int32_t h1[HL_SIZE];
} Accumulator;

//for now this function does nothing (TODO: see if eval hash gains with larger NNUEs?)
void init_tables()
{
    //clear eval hash table
    /* for (int i = 0; i < EVALHASH_SIZE; i++)
    {
        //"mate" value: if we get a wrong hit, avoid that position
        evalhash[i] = 192 - 32767; //getting lots of 0s in a hash is ultra-rare tho...
    } */
}

//Stuff to look at:
//https://github.com/jw1912/bullet/blob/main/examples/akimbo-main.rs
//NOTE: line 98 not needed when not doing quantized screlu
//eval: https://github.com/jw1912/akimbo/blob/main/src/position.rs#L154

//Calculate accumulator every time (inefficient!!!)
//TODO: replace with incremental updating system (we need to do it at the root tho)
//Also need to modify when quantizing
Accumulator calc_acc(Board &board, Color color)
{
    //Accumulator, initialize to biases
    Accumulator output;
    for (int j = 0; j < HL_SIZE; j++)
        output.h1[j] = L1_BIASES[j];

    for (int i = 0; i < 64; i++)
    {
        Square sq(i);
        //NOTE: pc = 6 * color_of_piece + type_of_piece
        int pc = board.at<Piece>(i);

        //skip if no piece!
        if (pc == (int)Piece::NONE)
            continue;

        //we have to flip the feature when calculating black's accumulator!
        if (color == Color::BLACK)
        {
            sq = sq.flip(); //flip the square!
            pc = (pc + 6) % 12; //toggle piece color
        }

        //index = 384*color + 64*piece_type + square
        int feat_index = 64 * (int)pc + sq.index();

        //update accumulator
        for (int j = 0; j < HL_SIZE; j++)
        {
            //L1_WEIGHTS is column major
            output.h1[j] += L1_WEIGHTS[feat_index * HL_SIZE + j];
        }
    }

    return output; //yeah, kind of ugly and inefficient :(, but we'll fix this later
}

int calc_nnue(Accumulator us, Accumulator them)
{
    //output node
    int32_t output = L2_BIAS;
    
    //add up stuff for both accumulators, do activation function here too
    for (int i = 0; i < HL_SIZE; i++)
    {
        output += crelu(us.h1[i], QA) * L2_WEIGHTS[i];
        output += crelu(them.h1[i], QA) * L2_WEIGHTS[HL_SIZE + i];
    }

    return output * NN_SCALE / (QA * QB); //scaling number
}

Value eval(Board board)
{
    //probe eval hash table
    /* uint64_t hash = board.hash();
    uint64_t evalhash_entry = evalhash[hash % EVALHASH_SIZE];
    if (!((evalhash_entry ^ hash) & 0xFFFFFFFF00000000)) //eval TT hit
        return (Value)(evalhash_entry & UINT32_MAX); */

    //calculate accumulators
    Accumulator us, them;
    //TODO: UE!!!
    us = calc_acc(board, board.sideToMove());
    them = calc_acc(board, !board.sideToMove());

    //I store it in a separate variable for now, if i want to do stuff with it first
    //Like manual scaling for endgames?
    int nnue_value = calc_nnue(us, them);

    //value that will be returned and stored in eval hash table
    Value final_value = std::min(std::max(nnue_value, -9999), 9999);
    //store it in eval hash table
    // evalhash[hash % EVALHASH_SIZE] = (uint64_t)(uint32_t)final_value | (hash & 0xFFFFFFFF00000000);
    return final_value;
}
