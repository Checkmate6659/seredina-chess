#ifndef CHESS_EXT_H
#define CHESS_EXT_H

#include "chess.hpp"
using namespace chess;

//https://disservin.github.io/chess-library/pages/extending-the-library.html
class W_Board : public Board {
    public:
    W_Board() : Board() {}
    W_Board(std::string_view fen) : Board(fen) {}

    virtual void setFen(std::string_view fen) {
        inc = 0;
        Board::setFen(fen);
    }

    //declare members here
    int inc = 0; //for now this just counts number of pieces

    protected:
    void placePiece(Piece piece, Square sq) {
        Board::placePiece(piece, sq);
        inc++;
    }

    void removePiece(Piece piece, Square sq) {
        Board::removePiece(piece, sq);
        inc--;
    }
};

#endif
