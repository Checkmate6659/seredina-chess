#include "eval.hpp"

#include "nn_values.hpp" //NN weights/biases

/* #define EVALHASH_SIZE (1<<18) //real size is 8*this in bytes (2MB here)
uint64_t evalhash[EVALHASH_SIZE]; */

//1024 hidden layer
//before quantize: 1295991 nodes 338739 nps
//after quantize: 1152604 nodes 544994 nps

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
//sth more important: https://github.com/jw1912/akimbo/blob/main/src/network.rs#L91
float calc_nnue(NNUEAccumulator us, NNUEAccumulator them)
{
    //output node
    int32_t output = 0;
    
    //add up stuff for both accumulators, do activation function here too
    for (int i = 0; i < HL_SIZE; i++)
    {
        output += screlu(us.h1[i]) * L2_WEIGHTS[i];
        output += screlu(them.h1[i]) * L2_WEIGHTS[HL_SIZE + i];
    }

    return ((output / QA) + L2_BIAS) * 400 / QAB; //scaling number
}

Value eval(W_Board board)
{
    //probe eval hash table
    /* uint64_t hash = board.hash();
    uint64_t evalhash_entry = evalhash[hash % EVALHASH_SIZE];
    if (!((evalhash_entry ^ hash) & 0xFFFFFFFF00000000)) //eval TT hit
        return (Value)(evalhash_entry & UINT32_MAX); */

    //calculate accumulators
    NNUEAccumulator us, them;
    // us = calc_acc(board, board.sideToMove());
    // them = calc_acc(board, !board.sideToMove());

    //EFFICIENT updates!
    if (board.sideToMove() == Color::WHITE)
    {
        us = board.white_acc;
        them = board.black_acc;
    }
    else
    {
        us = board.black_acc;
        them = board.white_acc;
    }

    //I store it in a separate variable for now, if i want to do stuff with it first
    //Like manual scaling for endgames?
    float nnue_value = calc_nnue(us, them);

    //value that will be returned and stored in eval hash table
    Value final_value = std::min(std::max(nnue_value, (float)-9999), (float)9999);
    //store it in eval hash table
    // evalhash[hash % EVALHASH_SIZE] = (uint64_t)(uint32_t)final_value | (hash & 0xFFFFFFFF00000000);
    return final_value;
}
