#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#include "Square.cpp"
#include "Move.cpp"
#include <vector>
#include <cfloat>
#include <cmath>

//
// Representation of a board state. Has an array of squares, as well as info on whose turn it is and castling.
// You can initiate a move on a board state to return the new board state.
//

class BoardState {
public:
    // Engine
    BoardState();
    BoardState(const BoardState &old);
    BoardState movePiece(Move move);
    std::string display();
    bool isWhiteTurn() const;
    bool legalMove(Move move);
    Square getSquare(int x, int y);
    bool inCheck(bool white);
    std::string printMoves();
    std::vector< std::pair<int,int> > getChecks(bool white);
    // AI
    double eval();
    Move bestMove();
    double minimax(BoardState current, int depth, double alpha, double beta);
    bool checkmate();
    void getMoves();
    bool operator () (const Move& move1, const Move& move2);


private:
    Square squares [8][8];
    bool whiteTurn; // true for white, false for black
    bool canCastle [4]{}; // white short castle, white long castle, black short castle, black long castle
    int king [4]{}; // white king x, white king y, black king x, black king y

    std::vector<Move> moves;

    // how many moves in the future we look with minimax
    // depth 5 recommended for quick response
    const static int searchDepth = 5;

    // heuristic eval constants
    constexpr const static auto centerSquareVal = 0.1;
    constexpr const static auto pawnStructDeduct = 0.2;
    constexpr const static auto develop = 0.2;
    constexpr const static auto doubledPawnDeduct = 0.2;
    //constexpr const static auto safeKing = 1;
    constexpr const static auto openRook = .2;
    constexpr const static auto badBishop = .2;
};

// creates a new board in standard configuration
BoardState::BoardState() {
    whiteTurn = true;
    for (bool &i: canCastle) i = true;
    king[0] = 4;
    king[1] = 0;
    king[2] = 4;
    king[3] = 7;

    bool w = true;
    squares [0][0] = Square(w, 1);
    squares [1][0] = Square(w, 2);
    squares [2][0] = Square(w, 3);
    squares [3][0] = Square(w, 4);
    squares [4][0] = Square(w, 5);
    squares [5][0] = Square(w, 3);
    squares [6][0] = Square(w, 2);
    squares [7][0] = Square(w, 1);

    for(auto & square : squares) {
        square[1] = Square(w, 6);
        for(int i = 2; i < 6; i++) {
            square[i] = Square();
        }
    }

    w = false;

    for(auto & square : squares) {
        square[6] = Square(w, 6);
    }

    squares [0][7] = Square(w, 1);
    squares [1][7] = Square(w, 2);
    squares [2][7] = Square(w, 3);
    squares [3][7] = Square(w, 4);
    squares [4][7] = Square(w, 5);
    squares [5][7] = Square(w, 3);
    squares [6][7] = Square(w, 2);
    squares [7][7] = Square(w, 1);

    getMoves();
}

// prints out the board
std::string BoardState::display() {
    std::string board;
    board += "\n";
    if(whiteTurn) {
        for(int i = 7; i >= 0; i--) {
            board.push_back('1' + i);
            board += " ";
            for(auto & square : squares) {
                board += square[i].toUni();
                board += " ";
            }
            board += "\n";
        }
        board += "  ";
        for(int j = 0; j < 8; j++) {
            board.push_back('a' + j);
            board += " ";
        }
    } else {
        for(int i = 0; i < 8; i++) {
            board.push_back('1' + i);
            board += " ";
            for(int j = 7; j >= 0; j--) {
                board += squares[j][i].toUni();
                board += " ";
            }
            board += "\n";
        }
        board += "  ";
        for(int j = 0; j < 8; j++) {
            board.push_back('h' - j);
            board += " ";
        }
    }

    board += "\n\n";

    return board;
}

// copies the board state to a new board, moves the piece on that board, then returns the resulting new board
BoardState BoardState::movePiece(Move move) {
    auto newBoard = BoardState( * this);
    // update king position
    if(squares[move.ox][move.oy].id() == 5) {
        newBoard.king[whiteTurn ? 0 : 2] = move.nx;
        newBoard.king[whiteTurn ? 1 : 3] = move.ny;
    }
    // en passant
    if(newBoard.squares[move.ox][move.oy].id() == 6 && newBoard.squares[move.nx][move.ny].id() == 0) {
        newBoard.squares[move.nx][move.oy] = Square();
    }
    // normal move
    if(move.special == 0) {
        // if moving rook a then no long castle
        if(move.ox == 0 && move.oy == (whiteTurn ? 0 : 7) && squares[move.ox][move.oy].id() == 1) {
            newBoard.canCastle[whiteTurn ? 1 : 3] = false;
        }
        // if moving rook h then no short castle
        if(move.ox == 7 && move.oy == (whiteTurn ? 0 : 7) && squares[move.ox][move.oy].id() == 1) {
            newBoard.canCastle[whiteTurn ? 0 : 2] = false;
        }
        // if moving king then no castle
        if(squares[move.ox][move.oy].id() == 5) {
            newBoard.canCastle[whiteTurn ? 1 : 3] = false;
            newBoard.canCastle[whiteTurn ? 0 : 2] = false;
        }
        newBoard.squares[move.nx][move.ny] = Square(squares[move.ox][move.oy].isWhite(), squares[move.ox][move.oy].id());
        newBoard.squares[move.ox][move.oy] = Square();
    }
    // short castle
    if(move.special == 1) {
        newBoard.canCastle[whiteTurn ? 0 : 2] = false;
        newBoard.canCastle[whiteTurn ? 1 : 3] = false;
        newBoard.king[whiteTurn ? 0 : 2] = 6;
        newBoard.squares[6][whiteTurn ? 0 : 7] = Square(whiteTurn, 5);
        newBoard.squares[5][whiteTurn ? 0 : 7] = Square(whiteTurn, 1);
        newBoard.squares[4][whiteTurn ? 0 : 7] = Square();
        newBoard.squares[7][whiteTurn ? 0 : 7] = Square();
    }
    // long castle
    if(move.special == 2) {
        newBoard.canCastle[whiteTurn ? 0 : 2] = false;
        newBoard.canCastle[whiteTurn ? 1 : 3] = false;
        newBoard.king[whiteTurn ? 0 : 2] = 2;
        newBoard.squares[2][whiteTurn ? 0 : 7] = Square(whiteTurn, 5);
        newBoard.squares[3][whiteTurn ? 0 : 7] = Square(whiteTurn, 1);
        newBoard.squares[1][whiteTurn ? 0 : 7] = Square();
        newBoard.squares[0][whiteTurn ? 0 : 7] = Square();
        newBoard.squares[4][whiteTurn ? 0 : 7] = Square();
    }
    // generate en passant takeable
    if(move.oy == (whiteTurn ? 1 : 6) && move.ny == (whiteTurn ? 3 : 4) && newBoard.squares[move.nx][move.ny].id() == 6) {
        newBoard.squares[move.ox][whiteTurn ? 2 : 5] = Square(whiteTurn, -1);
    }

    // pawn promote
    if(move.special > 2) {
        newBoard.squares[move.nx][move.ny] = Square(whiteTurn, move.special - 2);
        newBoard.squares[move.ox][move.oy] = Square();
    }

    newBoard.whiteTurn = !newBoard.whiteTurn;

    newBoard.getMoves();

    return newBoard;
}

bool BoardState::isWhiteTurn() const{
    return whiteTurn;
}

// copy constructor
BoardState::BoardState(const BoardState &old) {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(old.squares[i][j].id() > -1) squares[i][j] = old.squares[i][j];
        }
    }
    for(int i = 0; i < 4; i++) {
        king[i] = old.king[i];
        canCastle[i] = old.canCastle[i];
    }
    whiteTurn = old.whiteTurn;
}

// verifies if the player-entered move is legal (not used for AI-generated move)
bool BoardState::legalMove(Move move) {
    // check for castles
    if (move.special == 1) {
        if(canCastle[whiteTurn ? 0 : 2]) {
            return squares[5][whiteTurn ? 0 : 7].id() == 0 && squares[6][whiteTurn ? 0 : 7].id() == 0;
        } else {
            return false;
        }
    }
    if (move.special == 2) {
        if(canCastle[whiteTurn ? 1 : 3]) {
            return squares[1][whiteTurn ? 0 : 7].id() == 0 && squares[2][whiteTurn ? 0 : 7].id() == 0
            && squares[3][whiteTurn ? 0 : 7].id() == 0;
        } else {
            return false;
        }
    }
    // check for valid promotion
    if(move.special > 2) {
        if(move.ny != (whiteTurn ? 7 : 0)) return false;
    }
    // only allow pawns to en passant
    if(squares[move.ox][move.oy].id() != 6 && squares[move.nx][move.ny].id() == -1) return false;
    //check if piece doesn't move
    if(move.nx == move.ox && move.ny == move.oy) return false;
    // check if taking a piece of the same color
    if(squares[move.nx][move.ny].id() != 0 && squares[move.nx][move.ny].isWhite() == whiteTurn) {
        return false;
    }

    // check geometry of move
    switch (squares[move.ox][move.oy].id()) {
        case 1: // check rook
            if(move.nx != move.ox && move.ny != move.oy) return false;

            if(move.ny == move.oy) {
                for(int i = move.ox + ((move.nx - move.ox) > 0 ? 1 : -1);
                i != move.nx + ((move.nx - move.ox) > 0 ? 1 : -1); i += ((move.nx - move.ox) > 0 ? 1 : -1)) {
                    if(squares[i][move.oy].id() != 0) {
                        if(i == move.nx) {
                            if(squares[i][move.oy].isWhite() == whiteTurn) return false;
                        } else {
                            return false;
                        }
                    }
                }
            } else {
                for(int i = move.oy + ((move.ny - move.oy) > 0 ? 1 : -1);
                i != move.ny + ((move.ny - move.oy) > 0 ? 1 : -1); i += ((move.ny - move.oy) > 0 ? 1 : -1)) {
                    if(squares[move.ox][i].id() != 0) {
                        if(i == move.ny) {
                            if(squares[move.ox][i].isWhite() == whiteTurn) return false;
                        } else {
                            return false;
                        }
                    }
                }
            }
            break;
        case 2: // check knight
            if(!(abs(move.nx - move.ox) == 1 && abs(move.ny - move.oy) == 2)
            && !(abs(move.nx - move.ox) == 2 && abs(move.ny - move.oy) == 1)) return false;

            if(squares[move.nx][move.ny].id() != 0
            && squares[move.nx][move.ny].isWhite() == whiteTurn) return false;

            break;
        case 3: // check bishop
            if(abs(move.nx - move.ox) - abs(move.ny - move.oy) != 0) return false;
            for(int i = 1; i <= abs(move.nx - move.ox); i++) {
                if(squares[move.ox + i * (move.nx > move.ox ? 1 : -1)]
                [move.oy + i * (move.ny > move.oy ? 1 : -1)].id() != 0) {
                    if(move.ox + i * (move.nx > move.ox ? 1 : -1) == move.nx) {
                        if(squares[move.ox + i * (move.nx > move.ox ? 1 : -1)]
                        [move.oy + i * (move.ny > move.oy ? 1 : -1)].isWhite() == whiteTurn) return false;
                    } else {
                        return false;
                    }
                }
            }
            break;
        case 4: // check queen
            if(move.nx == move.ox || move.ny == move.oy) { //check rook moves
                if(move.ny == move.oy) {
                    for(int i = move.ox + 1; i <= move.nx; i += move.nx - move.ox > 0 ? 1 : -1) {
                        if(squares[i][move.oy].id() != 0) {
                            if(i == move.nx) {
                                if(squares[i][move.oy].isWhite() == whiteTurn) return false;
                            } else {
                                return false;
                            }
                        }
                    }
                } else {
                    for(int i = move.oy + 1; i <= move.ny; i += move.ny - move.oy > 0 ? 1 : -1) {
                        if(squares[move.ox][i].id() != 0) {
                            if(i == move.ny) {
                                if(squares[move.ox][i].isWhite() == whiteTurn) return false;
                            } else {
                                return false;
                            }
                        }
                    }
                }
            } else if(abs(move.nx - move.ox) - abs(move.ny - move.oy) == 0) { // check bishop moves
                for(int i = 1; i <= abs(move.nx - move.ox); i++) {
                    if(squares[move.ox + i * (move.nx > move.ox ? 1 : -1)]
                       [move.oy + i * (move.ny > move.oy ? 1 : -1)].id() != 0) {
                        if(move.ox + i * (move.nx > move.ox ? 1 : -1) == move.nx) {
                            if(squares[move.ox + i * (move.nx > move.ox ? 1 : -1)]
                               [move.oy + i * (move.ny > move.oy ? 1 : -1)].isWhite() == whiteTurn) return false;
                        } else {
                            return false;
                        }
                    }
                }
            } else {
                return false;
            }
            break;
        case 5: // check king
            if(abs(move.nx - move.ox) > 1 || abs(move.ny - move.oy) > 1) return false;

            if(squares[move.nx][move.ny].id() != 0
               && squares[move.nx][move.ny].isWhite() == whiteTurn) return false;

            break;
        case 6: // check pawn
            if(move.ox == move.nx) {
                if(squares[move.nx][move.ny].id() > 0) return false;
                if(move.ny - move.oy != (whiteTurn ? 1 : -1)) {
                    if(move.ny - move.oy == (whiteTurn ? 2 : -2)) {
                        if(move.oy != (whiteTurn ? 1 : 6)) return false;
                        if(squares[move.ox][move.oy + (whiteTurn ? 1 : -1)].id() != 0) return false;
                    } else {
                        return false;
                    }
                }
            } else {
                if(move.nx - move.ox == 1 || move.nx - move.ox == -1) {
                    if(move.ny - move.oy != (whiteTurn ? 1 : -1)) return false;
                    if(squares[move.nx][move.ny].isWhite() == whiteTurn
                    || squares[move.nx][move.ny].id() == 0) return false;
                } else {
                    return false;
                }
            }
            break;
        default:
            break;
    }

    // check if your king is in check after move
    if(movePiece(move).inCheck(whiteTurn)) {
        return false;
    }

    return true;
}

// returns square object at specified coordinates
Square BoardState::getSquare(int x, int y){
    return squares[x][y];
}

// returns true if specified player is in check
bool BoardState::inCheck(bool white) {
    int x = king[(white ? 0 : 2)];
    int y = king[(white ? 1 : 3)];
    if(abs(king[0] - king[1]) <= 1 && abs(king[2] - king[3]) <= 1) return true;
    // check up
    for(int i = y; i < 8; i++) {
        if(i != y) {
            if((squares[x][i].isWhite() == white || (squares[x][i].id() != 1 && squares[x][i].id() != 4)) && squares[x][i].id() > 0) break;
            if ((squares[x][i].id() == 1 || squares[x][i].id() == 4)
                && squares[x][i].isWhite() == !white) return true;
        }
    }
    // check down
    for(int i = y; i >= 0; i--) {
        if(i != y) {
            if((squares[x][i].isWhite() == white || (squares[x][i].id() != 1 && squares[x][i].id() != 4)) && squares[x][i].id() > 0) break;
            if ((squares[x][i].id() == 1 || squares[x][i].id() == 4)
                && squares[x][i].isWhite() == !white) return true;
        }
    }
    // check right
    for(int i = x; i < 8; i++) {
        if(i != x) {
            if((squares[i][y].isWhite() == white || (squares[i][y].id() != 1 && squares[i][y].id() != 4)) && squares[i][y].id() > 0) break;
            if ((squares[i][y].id() == 1 || squares[i][y].id() == 4)
                && squares[i][y].isWhite() == !white) return true;
        }
    }
    // check left
    for(int i = x; i >= 0; i--) {
        if(i != x) {
            if((squares[i][y].isWhite() == white  || (squares[i][y].id() != 1 && squares[i][y].id() != 4)) && squares[i][y].id() > 0) break;
            if ((squares[i][y].id() == 1 || squares[i][y].id() == 4)
                && squares[i][y].isWhite() == !white) return true;
        }
    }
    // check up right
    for(int i = 1; i < 8; i++) {
        if(i + y > 7 || i + x > 7) break;
        if((squares[i + x][i + y].isWhite() == white || (squares[x + i][y + i].id() != 3 && squares[x + i][y + i].id() != 4))
           && squares[i + x][i + y].id() > 0) break;
        if ((squares[i + x][i + y].id() == 3 || squares[i + x][i + y].id() == 4)
            && squares[i + x][i + y].isWhite() == !white) return true;
    }
    // check down right
    for(int i = 1; i < 8; i++) {
        if(y - i < 0 || i + x > 7) break;
        if((squares[x + i][y - i].isWhite() == white || (squares[x + i][y - i].id() != 3 && squares[x + i][y - i].id() != 4))
           && squares[x + i][y - i].id() > 0) break;
        if ((squares[x + i][y - i].id() == 3 || squares[x + i][y - i].id() == 4)
            && squares[x + i][y - i].isWhite() == !white) return true;
    }
    // check up left
    for(int i = 1; i < 8; i++) {
        if(y + i > 7 || x - i < 0) break;
        if((squares[x - i][y + i].isWhite() == white || (squares[x - i][y + i].id() != 3 && squares[x - i][y + i].id() != 4))
           && squares[x - i][y + i].id() > 0) break;
        if ((squares[x - i][y + i].id() == 3 || squares[x - i][y + i].id() == 4)
            && squares[x - i][y + i].isWhite() == !white) return true;
    }
    // check down left
    for(int i = 1; i < 8; i++) {
        if(y - i < 0 || x - i < 0) break;
        if ((squares[x - i][y - i].isWhite() == white ||
             (squares[x - i][y - i].id() != 3 && squares[x - i][y - i].id() != 4))
            && squares[x - i][y - i].id() > 0) break;
        if ((squares[x - i][y - i].id() == 3 || squares[x - i][y - i].id() == 4)
            && squares[x - i][y - i].isWhite() == !white) return true;
    }
    // check for knight attacks
    if(x < 7 && y < 6 && squares[x + 1][y + 2].id() == 2 && squares[x + 1][y + 2].isWhite() == !white) return true;
    if(x > 0 && y < 6 && squares[x - 1][y + 2].id() == 2 && squares[x - 1][y + 2].isWhite() == !white) return true;
    if(x < 7 && y > 1 && squares[x + 1][y - 2].id() == 2 && squares[x + 1][y - 2].isWhite() == !white) return true;
    if(x > 0 && y > 1 && squares[x - 1][y - 2].id() == 2 && squares[x - 1][y - 2].isWhite() == !white) return true;
    if(x < 6 && y < 7 && squares[x + 2][y + 1].id() == 2 && squares[x + 2][y + 1].isWhite() == !white) return true;
    if(x > 1 && y < 7 && squares[x - 2][y + 1].id() == 2 && squares[x - 2][y + 1].isWhite() == !white) return true;
    if(x < 6 && y > 0 && squares[x + 2][y - 1].id() == 2 && squares[x + 2][y - 1].isWhite() == !white) return true;
    if(x > 1 && y > 0 && squares[x - 2][y - 1].id() == 2 && squares[x - 2][y - 1].isWhite() == !white) return true;
    //check for pawn attacks
    if(squares[x + 1][y + (white ? 1 : -1)].isWhite() == !white
    && squares[x + 1][y + (white ? 1 : -1)].id() == 6) return true;
    if(squares[x - 1][y + (white ? 1 : -1)].isWhite() == !white
       && squares[x - 1][y + (white ? 1 : -1)].id() == 6) return true;

    return false;
}

// evaluates the board state, + for white, - for black
double BoardState::eval() {
    int wFile [8];
    int bFile [8];
    int wRooks [8];
    int bRooks [8];
    double total = 0;
    int freedom;

    // count pieces, analyze pawn structure and open files
    for(int i = 0; i < 8; i++) {
        wFile[i] = 0;
        bFile[i] = 0;
        wRooks[i] = 0;
        bRooks[i] = 0;
        for(int j = 0; j < 8; j++) {
            Square square = squares[i][j];
            switch (square.id()) {
                case 0:
                    break;
                case 1:
                    if((i < 7 && (whiteTurn ? j > 0 : j < 7) && squares[i + 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                    squares[i + 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite()) ||
                    (i > 0 && (whiteTurn ? j > 0 : j < 7) && squares[i - 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                    squares[i - 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite())) {
                        total -= (square.isWhite() ? 5 : -5);
                    }
                    if(square.isWhite()) {
                        wRooks[i]++;
                    } else {
                        bRooks[i]++;
                    }
                    total += (square.isWhite() ? 5 : -5);
                    break;
                case 2:
                    if((i < 7 && (whiteTurn ? j > 0 : j < 7) && squares[i + 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                        squares[i + 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite()) ||
                       (i > 0 && (whiteTurn ? j > 0 : j < 7) && squares[i - 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                        squares[i - 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite())) {
                        total -= (square.isWhite() ? 3 : -3);
                    }
                    total += (square.isWhite() ? 3 : -3);
                    break;
                case 3:
                    if((i < 7 && (whiteTurn ? j > 0 : j < 7) && squares[i + 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                        squares[i + 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite()) ||
                       (i > 0 && (whiteTurn ? j > 0 : j < 7) && squares[i - 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                        squares[i - 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite())) {
                        total -= (square.isWhite() ? 3 : -3);
                    }
                    freedom = 0;
                    if(i < 7) {
                        if(j < 7 && squares[i + 1][j + 1].id() == 0) freedom++;
                        if(j > 0 && squares[i + 1][j - 1].id() == 0) freedom++;
                    }
                    if(i > 0) {
                        if(j < 7 && squares[i - 1][j + 1].id() == 0) freedom++;
                        if(j > 0 && squares[i - 1][j - 1].id() == 0) freedom++;
                    }

                    if(freedom == 0) {
                        total -= 2 * badBishop * (square.isWhite() ? 1 : -1);
                    } else if(freedom == 1) {
                        total -= badBishop * (square.isWhite() ? 1 : -1);
                    }

                    total += (square.isWhite() ? 3 : -3);
                    break;
                case 4:
                    if((i < 7 && (whiteTurn ? j > 0 : j < 7) && squares[i + 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                        squares[i + 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite()) ||
                       (i > 0 && (whiteTurn ? j > 0 : j < 7) && squares[i - 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                        squares[i - 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite())) {
                        total -= (square.isWhite() ? 9 : -9);
                    }
                    total += (square.isWhite() ? 9 : -9);
                    break;
                case 6:
                    if((i < 7 && (whiteTurn ? j > 0 : j < 7) && squares[i + 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                        squares[i + 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite()) ||
                       (i > 0 && (whiteTurn ? j > 0 : j < 7) && squares[i - 1][j - (whiteTurn ? 1 : -1)].id() == 6 &&
                        squares[i - 1][j - (whiteTurn ? 1 : -1)].isWhite() != square.isWhite())) {
                        total -= (square.isWhite() ? 1 : -1);
                    }
                    if(square.isWhite()) {
                        wFile[i]++;
                    } else {
                        bFile[i]++;
                    }
                    total += (square.isWhite() ? 1 : -1);
                    if(i != (square.isWhite() ? 1 : 6)) {
                        Square l = squares[i - 1][j - (square.isWhite() ? 1 : -1)];
                        Square r = squares[i + 1][j - (square.isWhite() ? 1 : -1)];
                        if(!(l.isWhite() == square.isWhite() && l.id() == 6) && !(r.isWhite() == square.isWhite() && r.id() == 6)) {
                            total -= pawnStructDeduct * (square.isWhite() ? 1 : -1);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // check central squares
    for(int i = 2; i < 6; i++) {
        for(int j = 2; j < 6; j++) {
            int id = squares[i][j].id();
            if(id > 0) total += centerSquareVal * (squares[i][j].isWhite() ? 1 : -1);
            if(id == 2) {
                total += develop * (squares[i][j].isWhite() ? 1 : -1);
            }
        }
    }

    // check files
    for(int i = 0; i < 8; i++) {
        if (wFile[i] > 1) {
            total -= doubledPawnDeduct * (wFile[i] - 1);
        } else if(wFile[i] == 0) {
            total += openRook * wRooks[i];
        }
        if (bFile[i] > 1) {
            total += doubledPawnDeduct * (wFile[i] - 1);
        } else if(bFile[i] == 0) {
            total -= openRook * bRooks[i];
        }
    }

    if(king[0] <= 2 || king[0] >= 6) total += .5;
    if(king[2] <= 2 || king[2] >= 6) total -= .5;

    if(fabs(total) < .05) total = 0;

    return total;
}

//
bool BoardState::operator()(const Move &move1, const Move &move2)
{
    bool higher = movePiece(move1).eval() > movePiece(move2).eval();

    return whiteTurn == higher;
}

// method to find the best move from the current board state. uses recursive helper method
Move BoardState::bestMove() {

    Move best = moves[0];
    double alpha = -DBL_MAX;
    double beta = DBL_MAX;
    if(whiteTurn) {
        double maxEval = -DBL_MAX;
        for(auto move : moves) {
            double eval = minimax(movePiece(move), searchDepth - 1, alpha, beta);
            std::cout << movePiece(move).eval() << " -> " << eval << "\n";
            if(eval > maxEval && legalMove(move)) {
                std::cout << "\nnew max " << move.ox << move.oy << " " << move.nx << move.ny << " : " << eval << "\n";
                maxEval = eval;
                best = move;
            }
            alpha = std::max(eval, alpha);
            if(beta <= alpha) break;
        }
        return best;
    } else {
        double minEval = DBL_MAX;
        for(auto move : moves) {
            double eval = minimax(movePiece(move), searchDepth - 1, alpha, beta);
            std::cout << movePiece(move).eval() << " -> " << eval << "\n";
            if(eval < minEval  && legalMove(move)) {
                std::cout << "\nnew min " << move.ox << move.oy << " " << move.nx << move.ny << " : " << eval << "\n";
                minEval = eval;
                best = move;
            }
            beta = std::min(eval, beta);
            if(beta <= alpha) break;
        }
        return best;
    }
}

// minimax with alpha beta pruning
double BoardState::minimax(BoardState current, int depth, double alpha, double beta) {

    if(current.inCheck(current.whiteTurn) && checkmate()) return (current.whiteTurn ? -100 : 100);

    if(current.inCheck(!current.whiteTurn)) return DBL_MAX * (current.whiteTurn ? 1 : -1);

    if(current.moves.empty() || depth == 0) return current.eval();

    if(current.whiteTurn) {
        double maxEval = -DBL_MAX;
        for(auto move : current.moves) {
            double eval = minimax(current.movePiece(move), depth - 1, alpha, beta);
            maxEval = std::max(eval, maxEval);
            alpha = std::max(eval, alpha);
            if(beta <= alpha) break;
        }

        return maxEval;
    } else {
        double minEval = DBL_MAX;
        for(auto move : current.moves) {
            double eval = minimax(current.movePiece(move), depth - 1, alpha, beta);
            minEval = std::min(eval, minEval);
            beta = std::min(eval, beta);
            if(beta <= alpha) break;
        }

        return minEval;
    }
}

// checks if game is over, ends the program when true
bool BoardState::checkmate() {
    if(!inCheck(whiteTurn)) return false;

    int x = king[whiteTurn ? 0 : 2];
    int y = king[whiteTurn ? 1 : 3];

    std::vector< std::pair<int,int> > checks = getChecks(whiteTurn);

    for(Move move: moves) {
        // check for legal king moves
        if(move.ox == x && move.oy == y) {
            if(squares[move.nx][move.ny].id() == 0) {
                squares[x][y] = Square();
                king[whiteTurn ? 0 : 2] = move.nx;
                king[whiteTurn ? 1 : 3] = move.ny;
                if (!inCheck(whiteTurn)) {
                    squares[x][y] = Square(whiteTurn, 5);
                    king[whiteTurn ? 0 : 2] = x;
                    king[whiteTurn ? 1 : 3] = y;
                    return false;
                }
                squares[x][y] = Square(whiteTurn, 5);
                king[whiteTurn ? 0 : 2] = x;
                king[whiteTurn ? 1 : 3] = y;
            } else if(squares[move.nx][move.ny].isWhite() != whiteTurn) {
                squares[x][y] = Square();
                king[whiteTurn ? 0 : 2] = move.nx;
                king[whiteTurn ? 1 : 3] = move.ny;
                if (!inCheck(whiteTurn)) {
                    squares[x][y] = Square(whiteTurn, 5);
                    king[whiteTurn ? 0 : 2] = x;
                    king[whiteTurn ? 1 : 3] = y;
                    return false;
                }
                squares[x][y] = Square(whiteTurn, 5);
                king[whiteTurn ? 0 : 2] = x;
                king[whiteTurn ? 1 : 3] = y;
            }
        }

        // check for legal blocking moves
        if(checks.size() == 1) {
            if(checks[0].first == x && move.nx == x
            && ((checks[0].second >= move.ny && move.ny > y) || (checks[0].second <= move.ny && move.ny < y))
            && !movePiece(move).inCheck(whiteTurn)) { // vertical rook checks
                return false;
            } else if(checks[0].second == y && move.ny == y
            && ((checks[0].first >= move.nx && move.nx > x) || (checks[0].first <= move.nx && move.nx < x))
            && !movePiece(move).inCheck(whiteTurn)) { // horizontal rook checks
                return false;
            } else if(abs(checks[0].first - x) == abs(checks[0].second - y) && abs(move.nx - x) == abs(move.ny - y)) { // bishop checks
                if(((checks[0].second >= move.ny && move.ny > y) || (checks[0].second <= move.ny && move.ny < y))
                && !movePiece(move).inCheck(whiteTurn)) {
                    return false;
                }
            } else if(move.nx == checks[0].first && move.ny == checks[0].second &&
            !movePiece(move).inCheck(whiteTurn)) { // knight checks
                return false;
            }
        }
    }
    return true;
}

// generate the list of moves for the board state.
void BoardState::getMoves() {
    moves.reserve(100);

    if (canCastle[whiteTurn ? 0 : 2] && squares[1][whiteTurn ? 0 : 8].id() == 0
    && squares[2][whiteTurn ? 0 : 8].id() == 0) moves.emplace_back("O-O");
    if (canCastle[whiteTurn ? 1 : 3] && squares[4][whiteTurn ? 0 : 8].id() == 0
    && squares[5][whiteTurn ? 0 : 8].id() == 0 && squares[6][whiteTurn ? 0 : 8].id() == 0) moves.emplace_back("O-O-O");

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(squares[i][j].isWhite() == whiteTurn) {
                switch (squares[i][j].id()) {
                    case 1: // rook
                        // right
                        for(int k = i + 1; k < 8; k++) {
                            if(squares[k][j].id() <= 0) {
                                moves.emplace_back(i,j,k,j,0);
                            } else {
                                if(squares[k][j].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,k,j,0);
                                }
                                k = 8;
                            }
                        }
                        // left
                        for(int k = i - 1; k >= 0; k--) {
                            if(squares[k][j].id() <= 0) {
                                moves.emplace_back(i,j,k,j,0);
                            } else {
                                if(squares[k][j].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,k,j,0);
                                }
                                k = -1;
                            }
                        }
                        // up
                        for(int k = j + 1; k < 8; k++) {
                            if(squares[i][k].id() <= 0) {
                                moves.emplace_back(i,j,i,k,0);
                            } else {
                                if(squares[i][k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i,k,0);
                                }
                                k = 8;
                            }
                        }
                        // down
                        for(int k = j - 1; k >= 0; k--) {
                            if(squares[i][k].id() <= 0) {
                                moves.emplace_back(i,j,i,k,0);
                            } else {
                                if(squares[i][k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i,k,0);
                                }
                                k = -1;
                            }
                        }
                        break;
                    case 2: // knight
                        if(i < 7 && j < 6 && (squares[i + 1][j + 2].id() <= 0 || squares[i + 1][j + 2].isWhite() != whiteTurn))
                            moves.emplace_back(i,j,i + 1,j + 2,0);
                        if(i < 7 && j > 1 && (squares[i + 1][j - 2].id() <= 0 || squares[i + 1][j - 2].isWhite() != whiteTurn))
                            moves.emplace_back(i,j,i + 1,j - 2,0);
                        if(i > 0 && j < 6 && (squares[i - 1][j + 2].id() <= 0 || squares[i - 1][j + 2].isWhite() != whiteTurn))
                            moves.emplace_back(i,j,i - 1,j + 2,0);
                        if(i > 0 && j > 1 && (squares[i - 1][j - 2].id() <= 0 || squares[i - 1][j - 2].isWhite() != whiteTurn))
                            moves.emplace_back(i,j,i - 1,j - 2,0);
                        if(i < 6 && j < 7 && (squares[i + 2][j + 1].id() <= 0 || squares[i + 2][j + 1].isWhite() != whiteTurn))
                            moves.emplace_back(i,j,i + 2,j + 1,0);
                        if(i < 6 && j > 0 && (squares[i + 2][j - 1].id() <= 0 || squares[i + 2][j - 1].isWhite() != whiteTurn))
                            moves.emplace_back(i,j,i + 2,j - 1,0);
                        if(i > 1 && j < 7 && (squares[i - 2][j + 1].id() <= 0 || squares[i - 2][j + 1].isWhite() != whiteTurn))
                            moves.emplace_back(i,j,i - 2,j + 1,0);
                        if(i > 1 && j > 0 && (squares[i - 2][j - 1].id() <= 0 || squares[i - 2][j - 1].isWhite() != whiteTurn))
                            moves.emplace_back(i,j,i - 2,j - 1,0);
                        break;
                    case 3: // bishop
                        // up right
                        for(int k = 1; k <= std::min(7 - i, 7 - j); k++) {
                            if(squares[i + k][j + k].id() <= 0) {
                                moves.emplace_back(i,j,i + k,j + k,0);
                            } else {
                                if(squares[i + k][j + k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i + k,j + k,0);
                                }
                                break;
                            }
                        }
                        // down right
                        for(int k = 1; k <= std::min(7 - i, j); k++) {
                            if(squares[i + k][j - k].id() <= 0) {
                                moves.emplace_back(i,j,i + k,j - k,0);
                            } else {
                                if(squares[i + k][j - k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i + k,j - k,0);
                                }
                                break;
                            }
                        }
                        // up left
                        for(int k = 1; k <= std::min(i, 7 - j); k++) {
                            if(squares[i - k][j + k].id() <= 0) {
                                moves.emplace_back(i,j,i - k,j + k,0);
                            } else {
                                if(squares[i - k][j + k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i - k,j + k,0);
                                }
                                break;
                            }
                        }
                        // down left
                        for(int k = 1; k <= std::min(i, j); k++) {
                            if(squares[i - k][j - k].id() <= 0) {
                                moves.emplace_back(i,j,i - k,j - k,0);
                            } else {
                                if(squares[i - k][j - k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i - k,j - k,0);
                                }
                                break;
                            }
                        }
                        break;
                    case 4: // queen
                        // right
                        for(int k = i + 1; k < 8; k++) {
                            if(squares[k][j].id() <= 0) {
                                moves.emplace_back(i,j,k,j,0);
                            } else {
                                if(squares[k][j].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,k,j,0);
                                }
                                break;
                            }
                        }
                        // left
                        for(int k = i - 1; k >= 0; k--) {
                            if(squares[k][j].id() <= 0) {
                                moves.emplace_back(i,j,k,j,0);
                            } else {
                                if(squares[k][j].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,k,j,0);
                                }
                                break;
                            }
                        }
                        // up
                        for(int k = j + 1; k < 8; k++) {
                            if(squares[i][k].id() <= 0) {
                                moves.emplace_back(i,j,i,k,0);
                            } else {
                                if(squares[i][k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i,k,0);
                                }
                                break;
                            }
                        }
                        // up
                        for(int k = j - 1; k >= 0; k--) {
                            if(squares[i][k].id() <= 0) {
                                moves.emplace_back(i,j,i,k,0);
                            } else {
                                if(squares[i][k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i,k,0);
                                }
                                break;
                            }
                        }
                        // up right
                        for(int k = 1; k <= std::min(7 - i, 7 - j); k++) {
                            if(squares[i + k][j + k].id() <= 0) {
                                moves.emplace_back(i,j,i + k,j + k,0);
                            } else {
                                if(squares[i + k][j + k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i + k,j + k,0);
                                }
                                break;
                            }
                        }
                        // down right
                        for(int k = 1; k <= std::min(7 - i, j); k++) {
                            if(squares[i + k][j - k].id() <= 0) {
                                moves.emplace_back(i,j,i + k,j - k,0);
                            } else {
                                if(squares[i + k][j - k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i + k,j - k,0);
                                }
                                break;
                            }
                        }
                        // up left
                        for(int k = 1; k <= std::min(i, 7 - j); k++) {
                            if(squares[i - k][j + k].id() <= 0) {
                                moves.emplace_back(i,j,i - k,j + k,0);
                            } else {
                                if(squares[i - k][j + k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i - k,j + k,0);
                                }
                                break;
                            }
                        }
                        // down left
                        for(int k = 1; k <= std::min(i, j); k++) {
                            if(squares[i - k][j - k].id() <= 0) {
                                moves.emplace_back(i,j,i - k,j - k,0);
                            } else {
                                if(squares[i - k][j - k].isWhite() != whiteTurn) {
                                    moves.emplace_back(i,j,i - k,j - k,0);
                                }
                                break;
                            }
                        }
                        break;
                    case 5: // king
                        if(i < 7 && (squares[i + 1][j].id() <= 0 || squares[i + 1][j].isWhite() != whiteTurn)
                        && !(abs(i + 1 - king[whiteTurn ? 2 : 0]) <= 1 && abs(j - king[whiteTurn ? 3 : 1]) <= 1))
                            moves.emplace_back(i,j,i + 1,j,0);
                        if(i < 7 && j < 7 && (squares[i + 1][j + 1].id() <= 0 || squares[i + 1][j + 1].isWhite() != whiteTurn)
                        && !(abs(i + 1 - king[whiteTurn ? 2 : 0]) <= 1 && abs(j + 1 - king[whiteTurn ? 3 : 1]) <= 1))
                            moves.emplace_back(i,j,i + 1,j + 1,0);
                        if(j < 7 && (squares[i][j + 1].id() <= 0 || squares[i][j + 1].isWhite() != whiteTurn)
                        && !(abs(i - king[whiteTurn ? 2 : 0]) <= 1 && abs(j + 1 - king[whiteTurn ? 3 : 1]) <= 1))
                            moves.emplace_back(i,j,i,j + 1,0);
                        if(i > 0 && j < 7 && (squares[i - 1][j + 1].id() <= 0 || squares[i - 1][j + 1].isWhite() != whiteTurn)
                        && !(abs(i - 1 - king[whiteTurn ? 2 : 0]) <= 1 && abs(j + 1 - king[whiteTurn ? 3 : 1]) <= 1))
                            moves.emplace_back(i,j,i - 1,j + 1,0);
                        if(i > 0 && (squares[i - 1][j].id() <= 0 || squares[i - 1][j].isWhite() != whiteTurn)
                        && !(abs(i - 1 - king[whiteTurn ? 2 : 0]) <= 1 && abs(j - king[whiteTurn ? 3 : 1]) <= 1))
                            moves.emplace_back(i,j,i - 1,j,0);
                        if(i > 0 && j > 0 && (squares[i - 1][j - 1].id() <= 0 || squares[i - 1][j - 1].isWhite() != whiteTurn)
                        && !(abs(i - 1 - king[whiteTurn ? 2 : 0]) <= 1 && abs(j - 1 - king[whiteTurn ? 3 : 1]) <= 1))
                            moves.emplace_back(i,j,i - 1,j - 1,0);
                        if(j > 0 && (squares[i][j - 1].id() <= 0 || squares[i][j - 1].isWhite() != whiteTurn)
                        && !(abs(i - king[whiteTurn ? 2 : 0]) <= 1 && abs(j - 1 - king[whiteTurn ? 3 : 1]) <= 1))
                            moves.emplace_back(i,j,i,j - 1,0);
                        if(i < 7 && j > 0 && (squares[i + 1][j - 1].id() <= 0 || squares[i + 1][j - 1].isWhite() != whiteTurn)
                        && !(abs(i + 1 - king[whiteTurn ? 2 : 0]) <= 1 && abs(j - 1 - king[whiteTurn ? 3 : 1]) <= 1))
                            moves.emplace_back(i,j,i + 1,j - 1,0);
                        break;
                    case 6: // pawn
                        if(squares[i][j + (whiteTurn ? 1: -1)].id() == 0) {
                            // double move
                            if(j == (whiteTurn ? 1 : 6) && squares[i][j + (whiteTurn ? 1: -1)].id() == 0
                            && squares[i][j + (whiteTurn ? 2: -2)].id() == 0) {
                                moves.emplace_back(i,j,i,j + (whiteTurn ? 2 : -2), 0);
                            }
                            // normal move
                            if(j == (whiteTurn ? 6 : 1)) {
                                moves.emplace_back(i,j,i,j + (whiteTurn ? 1 : -1), 'Q');
                                moves.emplace_back(i,j,i,j + (whiteTurn ? 1 : -1), 'N');
                                moves.emplace_back(i,j,i,j + (whiteTurn ? 1 : -1), 'B');
                                moves.emplace_back(i,j,i,j + (whiteTurn ? 1 : -1), 'R');
                            } else {
                                moves.emplace_back(i,j,i,j + (whiteTurn ? 1 : -1), 0);
                            }
                        }
                        // capture right
                        if(i < 7 && squares[i + 1][j + (whiteTurn ? 1 : -1)].id() != 0
                           && squares[i + 1][j + (whiteTurn ? 1 : -1)].isWhite() != whiteTurn) {
                            if(j == (whiteTurn ? 6 : 1)) {
                                moves.emplace_back(i,j,i + 1,j + (whiteTurn ? 1 : -1), 'Q');
                                moves.emplace_back(i,j,i + 1,j + (whiteTurn ? 1 : -1), 'N');
                                moves.emplace_back(i,j,i + 1,j + (whiteTurn ? 1 : -1), 'B');
                                moves.emplace_back(i,j,i + 1,j + (whiteTurn ? 1 : -1), 'R');
                            } else {
                                moves.emplace_back(i,j,i + 1,j + (whiteTurn ? 1 : -1), 0);
                            }
                        }
                        // capture left
                        if(i > 0 && squares[i - 1][j + (whiteTurn ? 1 : -1)].id() != 0
                           && squares[i - 1][j + (whiteTurn ? 1 : -1)].isWhite() != whiteTurn) {
                            if(j == (whiteTurn ? 6 : 1)) {
                                moves.emplace_back(i,j,i - 1,j + (whiteTurn ? 1 : -1), 'Q');
                                moves.emplace_back(i,j,i - 1,j + (whiteTurn ? 1 : -1), 'N');
                                moves.emplace_back(i,j,i - 1,j + (whiteTurn ? 1 : -1), 'B');
                                moves.emplace_back(i,j,i - 1,j + (whiteTurn ? 1 : -1), 'R');
                            } else {
                                moves.emplace_back(i,j,i - 1,j + (whiteTurn ? 1 : -1), 0);
                            }
                        }
                        break;
                }
            }
        }
    }
}

// prints out all possible moves
std::string BoardState::printMoves() {
    std::string str = "\n";
    for(auto move : moves) {
        str += squares[move.ox][move.oy].toUni() + ": ";
        str.push_back('0' + move.ox);
        str += " ";
        str.push_back('0' + move.oy);
        str += " -> ";
        str.push_back('0' + move.nx);
        str += " ";
        str.push_back('0' + move.ny);
        str += "\n";
    }

    str += "Total moves: " + std::to_string(moves.size()) + "\n";
    return str;
}

// returns list of squares on the board responsible for checking the king
std::vector< std::pair<int,int> > BoardState::getChecks(bool white) {
    std::vector< std::pair<int,int> > checks;
    int x = king[(white ? 0 : 2)];
    int y = king[(white ? 1 : 3)];

    // check up
    for(int i = y; i < 8; i++) {
        if(i != y) {
            if((squares[x][i].isWhite() == white || (squares[x][i].id() != 1 && squares[x][i].id() != 4)) && squares[x][i].id() > 0) break;
            if ((squares[x][i].id() == 1 || squares[x][i].id() == 4)
                && squares[x][i].isWhite() == !white) checks.emplace_back(x,i);
        }
    }
    // check down
    for(int i = y; i >= 0; i--) {
        if(i != y) {
            if((squares[x][i].isWhite() == white || (squares[x][i].id() != 1 && squares[x][i].id() != 4)) && squares[x][i].id() > 0) break;
            if ((squares[x][i].id() == 1 || squares[x][i].id() == 4)
                && squares[x][i].isWhite() == !white) checks.emplace_back(x,i);
        }
    }
    // check right
    for(int i = x; i < 8; i++) {
        if(i != x) {
            if((squares[i][y].isWhite() == white || (squares[i][y].id() != 1 && squares[i][y].id() != 4)) && squares[i][y].id() > 0) break;
            if ((squares[i][y].id() == 1 || squares[i][y].id() == 4)
                && squares[i][y].isWhite() == !white) checks.emplace_back(i,y);
        }
    }
    // check left
    for(int i = x; i >= 0; i--) {
        if(i != x) {
            if((squares[i][y].isWhite() == white  || (squares[i][y].id() != 1 && squares[i][y].id() != 4)) && squares[i][y].id() > 0) break;
            if ((squares[i][y].id() == 1 || squares[i][y].id() == 4)
                && squares[i][y].isWhite() == !white) checks.emplace_back(i,y);
        }
    }
    // check up right
    for(int i = 1; i < 8; i++) {
        if(i + y > 7 || i + x > 7) break;
        if((squares[i + x][i + y].isWhite() == white || (squares[x + i][y + i].id() != 3 && squares[x + i][y + i].id() != 4))
           && squares[i + x][i + y].id() > 0) break;
        if ((squares[i + x][i + y].id() == 3 || squares[i + x][i + y].id() == 4)
            && squares[i + x][i + y].isWhite() == !white) checks.emplace_back(i + x,i + y);
    }
    // check down right
    for(int i = 1; i < 8; i++) {
        if(y - i < 0 || i + x > 7) break;
        if((squares[x + i][y - i].isWhite() == white || (squares[x + i][y - i].id() != 3 && squares[x + i][y - i].id() != 4))
           && squares[x + i][y - i].id() > 0) break;
        if ((squares[x + i][y - i].id() == 3 || squares[x + i][y - i].id() == 4)
            && squares[x + i][y - i].isWhite() == !white) checks.emplace_back(x + i,y - i);
    }
    // check up left
    for(int i = 1; i < 8; i++) {
        if(y + i > 7 || x - i < 0) break;
        if((squares[x - i][y + i].isWhite() == white || (squares[x - i][y + i].id() != 3 && squares[x - i][y + i].id() != 4))
           && squares[x - i][y + i].id() > 0) break;
        if ((squares[x - i][y + i].id() == 3 || squares[x - i][y + i].id() == 4)
            && squares[x - i][y + i].isWhite() == !white) checks.emplace_back(x - i,y + i);
    }
    // check down left
    for(int i = 1; i < 8; i++) {
        if(y - i < 0 || x - i < 0) break;
        if ((squares[x - i][y - i].isWhite() == white ||
             (squares[x - i][y - i].id() != 3 && squares[x - i][y - i].id() != 4))
            && squares[x - i][y - i].id() > 0) break;
        if ((squares[x - i][y - i].id() == 3 || squares[x - i][y - i].id() == 4)
            && squares[x - i][y - i].isWhite() == !white) checks.emplace_back(x - i,y - i);
    }
    // check for knight attacks
    if(x < 7 && y < 6 && squares[x + 1][y + 2].id() == 2 && squares[x + 1][y + 2].isWhite() == !white) checks.emplace_back(x + 1,y + 2);
    if(x > 0 && y < 6 && squares[x - 1][y + 2].id() == 2 && squares[x - 1][y + 2].isWhite() == !white) checks.emplace_back(x - 1,y + 2);
    if(x < 7 && y > 1 && squares[x + 1][y - 2].id() == 2 && squares[x + 1][y - 2].isWhite() == !white) checks.emplace_back(x + 1,y - 2);
    if(x > 0 && y > 1 && squares[x - 1][y - 2].id() == 2 && squares[x - 1][y - 2].isWhite() == !white) checks.emplace_back(x - 1,y - 2);
    if(x < 6 && y < 7 && squares[x + 2][y + 1].id() == 2 && squares[x + 2][y + 1].isWhite() == !white) checks.emplace_back(x + 2,y + 1);
    if(x > 1 && y < 7 && squares[x - 2][y + 1].id() == 2 && squares[x - 2][y + 1].isWhite() == !white) checks.emplace_back(x - 2,y + 1);
    if(x < 6 && y > 0 && squares[x + 2][y - 1].id() == 2 && squares[x + 2][y - 1].isWhite() == !white) checks.emplace_back(x + 2,y - 1);
    if(x > 1 && y > 0 && squares[x - 2][y - 1].id() == 2 && squares[x - 2][y - 1].isWhite() == !white) checks.emplace_back(x - 2,y - 1);
    //check for pawn attacks
    if(squares[x + 1][y + (white ? 1 : -1)].isWhite() == !white
       && squares[x + 1][y + (white ? 1 : -1)].id() == 6) checks.emplace_back(x + 1,y + (white ? 1 : -1));
    if(squares[x - 1][y + (white ? 1 : -1)].isWhite() == !white
       && squares[x - 1][y + (white ? 1 : -1)].id() == 6) checks.emplace_back(x - 1,y + (white ? 1 : -1));

    return checks;
}

#pragma clang diagnostic pop