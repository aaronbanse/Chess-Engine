# Chess-Engine
A terminal-based chess engine written in C++.


## Usage
Uses makefile to compile.
Enter moves in [standard algebraic notation](https://en.wikipedia.org/wiki/Algebraic_notation_(chess))
Enter 'best' to compute and execute best move according to the engine.

## Reflection & Thoughts
This was my first C++ program, so I learned a lot in the process of making it. I also had to learn a lot about chess engine implementation, and I missed a few important details which would have been helpful. If I were to do it again, I would use a bitboards representation of the boardstate, as it makes use of binary operations to efficiently pull relevant information from the board. Also, since my main focus was on the best move algorithm, I would use an existing library to handle the game engine for things like move generation and rules.
