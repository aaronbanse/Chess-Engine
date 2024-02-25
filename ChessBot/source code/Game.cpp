#include "BoardState.cpp"

//
// Contains the active board state and allows for player input to make moves on that board
//

class Game {
public:
    Game();
    Move getMove(std::string input);
    void play();
private:
    BoardState current;
    void turn();
};

Game::Game() {
    current = BoardState();
}

void Game::play() {

    std::cout << "\nWelcome! Use algebraic notation to make a move or type \"best\" to let the algorithm move.";

    std::cout << "\n" << current.display() << current.eval() << "\n";
    while(!current.checkmate()) {
        turn();
    }

    std::cout << (!current.isWhiteTurn() ? "White" : "Black") << " wins!";
}

void Game::turn() {

    std::cout << (current.isWhiteTurn() ? "White" : "Black") << " to move: ";
    std::string input;
    std::cin >> input;

    Move move = getMove(input);

    std::string bmove;

    if(input != "best") {
        while(move.special == -1) {
            std::cout << "Please give legal move: ";
            std::cin >> input;
            move = getMove(input);
        }
    } else {

        bmove.push_back('a' + move.ox);
        bmove.push_back('1' + move.oy);
        bmove += " -> ";
        bmove.push_back('a' + move.nx);
        bmove.push_back('1' + move.ny);
        std::cout << "\nBest: " + bmove + "\n";
    }

    current = current.movePiece(move);

    std::cout << "\n" << current.display() << current.eval() << "\n";
    if(bmove.empty()) std::cout << "\nBest: " + bmove + "\n";

}

// takes string input of algebraic chess move format, returns Move. If move is illegal or
// the string doesn't translate to a valid move, it returns the default move with special = -1 so Game::turn can ask
// for another input.
Move Game::getMove(std::string input) {
    if (input == "best") {
        return current.bestMove();
    }
    if(input == "print") {
        std::cout << current.printMoves();
        return {};
    }

    input.erase(remove(input.begin(), input.end(), 'x'), input.end());
    input.erase(remove(input.begin(), input.end(), '+'), input.end());
    input.erase(remove(input.begin(), input.end(), '='), input.end());
    input.erase(remove(input.begin(), input.end(), '#'), input.end());
    input.erase(remove(input.begin(), input.end(), '?'), input.end());
    input.erase(remove(input.begin(), input.end(), '!'), input.end());

    if(input.length() < 2) return {};

    if(input == "O-O" || input == "O-O-O") {
        if (current.legalMove(Move(input))) {
            return {input};
        } else { return {}; }
    }

    int id;
    int ox = -1;
    int oy = -1;
    int nx = 0;
    int ny = 0;
    char prom = 0;


    int i = 0;

    if(input[i] >= 'A' && input[i] <= 'Z') {
        switch (input[i]) {
            case 'R':
                id = 1;
                break;
            case 'N':
                id = 2;
                break;
            case 'B':
                id = 3;
                break;
            case 'Q':
                id = 4;
                break;
            case 'K':
                id = 5;
                break;
            default:
                return {};
        }
        i++;
    } else {
        id = 6;
    }



    if (input.length() - i < 2) return {};

    if(input[i] >= '1' && input[i] <= '8') {
        oy = input[i] - '1';
        i++;
        if(input[i] >= 'a' && input[i] <= 'h') {
            nx = input[i] - 'a';
            if(input.length() - 1 > 1) {
                i++;
                if(input[i] >= '1' && input[i] <= '8') {
                    ny = input[i] - '1';
                } else {
                    return {};
                }
            }
        }
    } else if(input[i] >= 'a' && input[i] <= 'h') {
        i++;
        if(input[i] >= '1' && input[i] <= '8') {
            nx = input[i-1] - 'a';
            ny = input[i] - '1';
        } else if(input[i] >= 'a' && input[i] <= 'h') {
            ox = input[i-1] - 'a';
            nx = input[i] - 'a';
            if(input.length() - 1 > 1) {
                i++;
                if(input[i] >= '1' && input[i] <= '8') {
                    ny = input[i] - '1';
                } else {
                    return {};
                }
            }
        } else {
            return {};
        }
    } else {
        return {};
    }

    // set default prom
    if(id == 6 && ny == (current.isWhiteTurn() ? 7 : 0)) {
        prom = 'Q';
    }

    if(input.length() - i > 1 && id == 6) {
        i++;
        if(input[i] == 'N' || input[i] == 'B' || input[i] == 'R' || input[i] == 'Q') prom = input[i];
    }

    // since we don't know origin coords, search board for squares with id that could be the one to move
    for(int j = 0; j < 8; j++) {
        for(int k = 0; k < 8; k++) {
            if(current.getSquare(j,k).id() == id && current.getSquare(j,k).isWhite() == current.isWhiteTurn()
               && (j == ox || k == oy || ox == oy) && current.legalMove(Move(j, k, nx, ny, prom))) {
                ox = j;
                oy = k;
            }
        }
    }

    if(ox == -1 || oy == -1) return {};



    return {ox, oy, nx, ny, prom};
}


