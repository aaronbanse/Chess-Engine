#include <iostream>
class Square {
public:
    int id() const;
    bool isWhite() const;
    Square();
    Square(bool w, int i);
    std::string toUni();

private:
    int mID;
    bool mIsWhite;
};

// square with piece in it
Square::Square(bool w, int i) {
    mIsWhite = w;
    mID = i;
}

// empty square
Square::Square() {
    mIsWhite = true;
    mID = 0;
}

// getters

int Square::id() const {
    return mID;
}

bool Square::isWhite() const {
    return mIsWhite;
}

// returns unicode string for the piece
std::string Square::toUni() {
    switch (mID) {
        case 0:
            return ".";
        case 1:
            if(mIsWhite) {
                return "\u2656";
            } else {
                return "\u265C";
            }
        case 2:
            if(mIsWhite) {
                return "\u2658";
            } else {
                return "\u265E";
            }
        case 3:
            if(mIsWhite) {
                return "\u2657";
            } else {
                return "\u265D";
            }
        case 4:
            if(mIsWhite) {
                return "\u2655";
            } else {
                return "\u265B";
            }
        case 5:
            if(mIsWhite) {
                return "\u2654";
            } else {
                return "\u265A";
            }
        case 6:
            if(mIsWhite) {
                return "\u2659";
            } else {
                return "\u265F";
            }
        case -1:
            return "x";
    }
    return "";
}
