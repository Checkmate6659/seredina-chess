#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <string>
#include <time.h>

#include "chess.hpp"
#include "chess_ext.hpp"
#include "search.hpp"
#include "eval.hpp"
#include "bench.hpp"
using namespace chess;

#define ENGINE_NAME "Seredina v1.2"

#define EXTRA_DELAY 50 //time to account for communication and panic delay (in ms)

#ifdef TUNING
const void* TUNING_PARAMS[] = {
    &lmr_f1, &lmr_f2,
    &iir_depth,
    &nmp_const,
    &see_multiplier, &see_const,
    &lmr_mindepth, &lmr_reduceafter,
    &lmr_pv, &lmr_improving,
    &rfp_depth, &rfp_margin, &rfp_impr,
    &aspi_width,
    &se_mindepth, &se_ttdepth_margin, &se_depth_mul,
    &se_dbl_margin, &se_dbl_maxdepth,
    &lmp00, &lmp10, &lmp01, &lmp11,
    &lmp_depth,
    &nmp_mul,
};
//F = float (all *1000); I = int
const char TUNING_TYPES[] = "FFIIIIIIFFIIIIIIIIIFFFFIF";
#endif

int main(int argc, char **argv)
{
    init_tables(); //does this do anything? (not sure)
    init_search_tables();
    if(!alloc_hash(16)) //create 16MB TT
    {
        std::cerr << "Cannot allocate 16MB hash table\n";
        return 1;
    }

    if (argc > 1) //bench
    {
        //clear tables, so that all values are initialized
        clear_hash();
        clear_small_tables();

        bench();
        return 0;
    }

    //UCI loop
    W_Board board = W_Board();
    board.setFen(constants::STARTPOS); //init NNUE accumulator; not required by uci
    while(true)
    {
		std::string input_string;
		std::getline(std::cin, input_string);
		std::stringstream input_stream(input_string);
		std::string command;
		input_stream >> command;

        switch (command[0])
        {
            case 'u': //uci & ucinewgame
            if (command.length() > 7) //for ucinewgame
            {
                clear_hash(); //time-consuming: don't do it for UCI
                clear_small_tables();
            }
            else
            {
                std::cout << "id name " ENGINE_NAME "\nid author Enigma\n";
                //options
                std::cout << "option name Hash type spin default 16 min 1 max 1024\n";
#ifdef TUNING
                for (int i = 0; i < sizeof(TUNING_TYPES) - 1; i++) //TUNING_TYPES is an array of characters
                {
                    std::cout << "option name P" << i << " type spin default ";
                    if (TUNING_TYPES[i] == 'F') //float
                    {
                        std::cout << (int)(*(float*)TUNING_PARAMS[i] * 1000);
                    }
                    else //int
                    {
                        std::cout << *(int*)TUNING_PARAMS[i];
                    }
                    std::cout << " min -1000000 max 1000000\n";
                }
#endif
                //uciok
                std::cout << "uciok\n";
            }
            break;
            case 'i': //isready
            std::cout << "readyok\n";
            break;
            case 'q': //quit
            return 0;
            case 's': //setoption
            input_stream >> command; //"name"
            input_stream >> command; //option name
            if (command[0] == 'H') //hash
            {
                input_stream >> command; //"value"
                input_stream >> command; //size in mb
                uint32_t size_mb = std::stoi(command);
                if (!alloc_hash(size_mb)) //allocate
                {
                    //allocation failed
                    std::cerr << "Cannot allocate " << size_mb << "MB hash table\n";
                    return 1;
                }
            }
#ifdef TUNING
            else if (command[0] == 'P') //param in tuning, of the form Pxx where xx is a number
            {
                int index = std::atoi(command.c_str() + 1); //drop the 'P'
                input_stream >> command; //"value"
                input_stream >> command; //parameter value
                int value = std::stoi(command);
                // printf("%d %d\n", index, value);

                char type = TUNING_TYPES[index];
                if (type == 'F') //float
                {
                    *(float*)TUNING_PARAMS[index] = value / 1000.; //divide by 1000
                }
                else //int
                {
                    *(int*)TUNING_PARAMS[index] = value;
                }
                init_search_tables(); //reinitialize LMR and LMP
            }
#endif
            break;
            case 'p': //position
            input_stream >> command; //get new word
            if (command[0] == 's') //position startpos ...
                board.setFen(constants::STARTPOS); //handy
            else //position fen <FEN> ... (ONLY ACCEPTS FEN WITH COUNTERS! otherwise it freaks out)
            {
                std::string fen_string;
                for (int i = 0; i < 6; i++) //fen string has 6 "words"
                {
                    input_stream >> command;
                    fen_string.append(command);
                    if(i != 5) fen_string.append(" "); //spaces between words
                }
                board.setFen(fen_string);
            }
            // std::cout << eval(board) << std::endl; //DEBUG
            //load moves after fen string (or startpos)
            input_stream >> command;
            if (command[0] != 'm') break; //no moves to load
            input_stream >> command;
            while (input_stream)
            {
                board.makeMove(uci::uciToMove(board, command)); //make move
                input_stream >> command; //load next move in
            }
            //std::cout << board.getFen() << std::endl; //DEBUG
            break;
            case 'g': //go command
            char engine_color = (board.sideToMove() == Color::WHITE) ? 'w' : 'b';
            unsigned engtime = 0; //engine's think time
            unsigned enginc = 0; //engine's increment
            unsigned movestogo = 30; //default to 30
            unsigned depth = MAX_DEPTH; //default depth = max
            unsigned movetime = 0; //no set movetime = 0 (special value)
            max_nodes = UINT64_MAX;
            //load parameters
            input_stream >> command;
            while (input_stream)
            {
                if (command[0] == 'd') //depth
                {
                    input_stream >> command;
                    depth = std::stoi(command);
                    movetime = 3600000; //1h of movetime (WARNING: clock_t 32bit -> overflow!!!)
                }
                else if (command[0] == 'n') //nodes (ignored if compiled w/o SEARCH_NODES)
                {
                    input_stream >> command;
                    max_nodes = std::stoull(command);
                    movetime = 3600000; //1h of movetime
                }
                else if (command[0] == engine_color) //wtime, btime, winc, binc
                {
                    if (command[1] == 't') //wtime, btime
                    {
                        input_stream >> command;
                        engtime = std::stoi(command);
                    }
                    else //winc, binc
                    {
                        input_stream >> command;
                        enginc = std::stoi(command);
                    }
                }
                else if (command[4] == 't') //movetime
                {
                    input_stream >> command;
                    movetime = std::stoi(command);
                }
                else //movestogo
                {
                    input_stream >> command;
                    movestogo = std::stoi(command);
                }
                input_stream >> command;
            }
            int alloc_time = movetime ? //time management (all in milliseconds)
                movetime :
                (engtime / movestogo + enginc)
            - EXTRA_DELAY; //account for communication delays
            //pass it on to this function in search.cpp
            Move best_move = search_root(board, alloc_time, depth);
            //print best move
            std::cout << "bestmove " << uci::moveToUci(best_move) << std::endl;
        }
    }

    return 0;
}
