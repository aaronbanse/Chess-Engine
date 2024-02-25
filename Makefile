all:
	g++ -std=c++11 -g -o main main.cpp

clean:
	rm main

debug:
	g++ -std=c++11 -o main main.cpp