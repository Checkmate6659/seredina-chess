all:
	#g++ -std=c++17 -O3 -flto -DNDEBUG -march=native *.cpp
	#g++ -std=c++17 -O3 -flto -s -DNDEBUG -march=native *.cpp
	#clang++ -std=c++17 -O3 -flto -s -DNDEBUG -march=native *.cpp
	clang++ -std=c++17 -O3 -flto -s -DNDEBUG -march=native -ffast-math -fno-signed-zeros *.cpp
	#g++ -std=c++17 -O3 -flto -march=native *.cpp
	#g++ -std=c++17 -Wall -Wextra *.cpp

#g++ -std=c++17 -O3 -flto -DNDEBUG -march=native -c chess.hpp -o chess-library.o

#release build:
#linux:
#clang++ -std=c++17 -O3 -flto -s -DNDEBUG -march=x86-64 -ffast-math -fno-signed-zeros *.cpp -o seredina_linux_x64.out
#clang++ -std=c++17 -O3 -flto -s -DNDEBUG -march=core-avx2 -ffast-math -fno-signed-zeros *.cpp -o seredina_linux_avx2.out
#windows: (no -flto, otherwise crashes on startup! i also only tested with g++)
#g++ -std=c++17 -O3 -s -DNDEBUG -march=x86-64 -ffast-math -fno-signed-zeros *.cpp -o seredina_win10_x64.out
#g++ -std=c++17 -O3 -s -DNDEBUG -march=core-avx2 -ffast-math -fno-signed-zeros *.cpp -o seredina_win10_avx2.out

