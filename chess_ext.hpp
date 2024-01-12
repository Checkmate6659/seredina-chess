#ifndef CHESS_EXT_H
#define CHESS_EXT_H

#include "chess.hpp"
#include "nn_values.hpp" //needed for incremental computation of NNUE accumulator
using namespace chess;

#define HL_SIZE 16 //NN hidden layer size

//NNUE accumulator
typedef struct {
    float h1[HL_SIZE];
} NNUEAccumulator;


//https://disservin.github.io/chess-library/pages/extending-the-library.html
class W_Board : public Board {
    public:
    W_Board() : Board() {}
    W_Board(std::string_view fen) : Board(fen) {}

    virtual void setFen(std::string_view fen) {
        for (int j = 0; j < HL_SIZE; j++)
            white_acc.h1[j] = black_acc.h1[j] = L1_BIASES[j];
        Board::setFen(fen);
    }

    //declare members here
    NNUEAccumulator white_acc, black_acc; //NNUE accumulator (incrementally updated)

    protected:
    //NOTE: without quantization we could have roundoff error issues!
    //hopefully they're not *too* large...
    void placePiece(Piece piece, Square sq) {
        Board::placePiece(piece, sq);

        //update NNUE accumulators
        //first, calculate features for white...
        //NOTE: pc = 6 * color_of_piece + type_of_piece
        int pc = (int)piece;

        //index = 384*color + 64*piece_type + square
        int feat_index = 64 * (int)pc + sq.index();

        //update accumulator
        for (int j = 0; j < HL_SIZE; j++)
        {
            //L1_WEIGHTS is column major
            white_acc.h1[j] += L1_WEIGHTS[feat_index * HL_SIZE + j];
        }

        //...then, do it for black
        //we have to flip the feature when calculating black's accumulator!
        sq = sq.flip(); //flip the square!
        pc = (pc + 6) % 12; //toggle piece color

        //index = 384*color + 64*piece_type + square
        feat_index = 64 * (int)pc + sq.index();

        //update accumulator
        for (int j = 0; j < HL_SIZE; j++)
        {
            //L1_WEIGHTS is column major
            black_acc.h1[j] += L1_WEIGHTS[feat_index * HL_SIZE + j];
        }
    }

    void removePiece(Piece piece, Square sq) {
        Board::removePiece(piece, sq);
        
        //downdate NNUE accumulators
        //first, calculate features for white...        
        int pc = (int)piece;

        //index = 384*color + 64*piece_type + square
        int feat_index = 64 * (int)pc + sq.index();

        //update accumulator
        for (int j = 0; j < HL_SIZE; j++)
        {
            //same as in placePiece but with - instead of +
            white_acc.h1[j] -= L1_WEIGHTS[feat_index * HL_SIZE + j];
        }

        //...then, do it for black
        //we have to flip the feature when calculating black's accumulator!
        sq = sq.flip(); //flip the square!
        pc = (pc + 6) % 12; //toggle piece color

        //index = 384*color + 64*piece_type + square
        feat_index = 64 * (int)pc + sq.index();

        //update accumulator
        for (int j = 0; j < HL_SIZE; j++)
        {
            //same as in placePiece but with - instead of +
            black_acc.h1[j] -= L1_WEIGHTS[feat_index * HL_SIZE + j];
        }
    }
};

#endif
