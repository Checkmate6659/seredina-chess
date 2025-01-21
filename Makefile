all:
	#g++ -std=c++17 -O3 -flto -DNDEBUG -march=native *.cpp
	#g++ -std=c++17 -O3 -flto -s -DNDEBUG -march=native *.cpp
	#clang++ -std=c++17 -O3 -flto -s -DNDEBUG -march=native *.cpp
	clang++ -std=c++17 -O3 -flto -s -DNDEBUG -march=native -ffast-math -fno-signed-zeros *.cpp
	#g++ -std=c++17 -O3 -flto -march=native *.cpp
	#g++ -std=c++17 -Wall -Wextra *.cpp

#g++ -std=c++17 -O3 -flto -DNDEBUG -march=native -c chess.hpp -o chess-library.o
#release build:
#clang++ -std=c++17 -O3 -flto -s -DNDEBUG -march=x86-64 -ffast-math -fno-signed-zeros *.cpp
#clang++ -std=c++17 -O3 -flto -s -DNDEBUG -march=core-avx2 -ffast-math -fno-signed-zeros *.cpp
