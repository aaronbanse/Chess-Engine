#include <iostream>

// contains start and end point data as well as info for special moves
struct Move {
    int ox;
    int oy;
    int nx;
    int ny;
    int special; // this variable can signify castles, pawn promotions, and illegal moves
    void init();
    Move();
    Move(std::string s);
    Move(int oX, int oY, int nX, int nY, char promote);

};

// default constructor is for illegal moves. useful for the Game::getMove method as it allows it to return a valid move
// or just call the default constructor if the move is invalid.
Move::Move() {
    init();
    special = -1;
}
// main move constructor used. promote signifies a pawn promotion
Move::Move(int oX, int oY, int nX, int nY, char promote) {
    ox = oX;
    oy = oY;
    nx = nX;
    ny = nY;
    switch (promote) {
        default:
            special = 0;
            break;
        case 'R':
            special = 3;
            break;
        case 'N':
            special = 4;
            break;
        case 'B':
            special = 5;
            break;
        case 'Q':
            special = 6;
            break;
    }
}

// castle
Move::Move(std::string s) {
    init();
    if(s == "O-O") {
        special = 1;
    } else if(s == "O-O-O") {
        special = 2;
    }
}

// constructor delegation
void Move::init() {
    ox = 0;
    oy = 0;
    nx = 0;
    ny = 0;
}







