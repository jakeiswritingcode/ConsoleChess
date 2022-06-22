// chess_game.cpp
// by Jake Charles Osborne III



#include "chess_game.h"
#include <vector>
#include <tuple>



using std::vector;
using std::tuple;



Chess::Piece::Position::Position(char x, int y) {
    this->x = x;
    this->y = y;
}

Chess::Piece::Position::Position(int x, int y) {
    this->x = char('A' - 1 + x);
    this->y = y;
}

bool Chess::Piece::Position::operator ==(const Position& p) const { return this->x == p.x && this->y == p.y; }
bool Chess::Piece::Position::operator !=(const Position& p) const { return !(*this == p); }

bool Chess::Piece::Position::sameCol(const Position& position1, const Position& position2) { return position1.x == position2.x; }
bool Chess::Piece::Position::sameRow(const Position& position1, const Position& position2) { return position1.y == position2.y; }
bool Chess::Piece::Position::sameMainDiagonal(const Position& position1, const Position& position2) { return position1.x + position1.y == position2.x + position2.y; }
bool Chess::Piece::Position::sameAntidiagonal(const Position& position1, const Position& position2) { return position1.x - position1.y == position2.x - position2.y; }
bool Chess::Piece::Position::inBounds(const Position& position) { return position.x >= 'A' && position.x <= ('A' - 1) + 8 && position.y >= 1 && position.y <= 8; }

Chess::Piece::Proximity::Proximity(const Position& position, const vector<Piece*>& pieces) {
    for (Piece* const& piece : pieces) {
        if (piece) {
            // North
            if (Piece::Position::sameCol(piece->position, position) && piece->position.y > position.y)
                if (!n || piece->position.y < n->position.y)
                    n = piece;
            // East
            if (Piece::Position::sameRow(piece->position, position) && piece->position.x > position.x)
                if (!e || piece->position.x < e->position.x)
                    e = piece;
            // South
            if (Piece::Position::sameCol(piece->position, position) && piece->position.y < position.y)
                if (!s || piece->position.y > s->position.y)
                    s = piece;
            // West
            if (Piece::Position::sameRow(piece->position, position) && piece->position.x < position.x)
                if (!w || piece->position.x > w->position.x)
                    w = piece;
            // Northeast
            if (Piece::Position::sameAntidiagonal(piece->position, position) && piece->position.x > position.x)
                if (!ne || piece->position.x < ne->position.x)
                    ne = piece;
            // Southeast
            if (Piece::Position::sameMainDiagonal(piece->position, position) && piece->position.x > position.x)
                if (!se || piece->position.x < se->position.x)
                    se = piece;
            // Southwest
            if (Piece::Position::sameAntidiagonal(piece->position, position) && piece->position.x < position.x)
                if (!sw || piece->position.x > sw->position.x)
                    sw = piece;
            // Northwest
            if (Piece::Position::sameMainDiagonal(piece->position, position) && piece->position.x < position.x)
                if (!nw || piece->position.x > nw->position.x)
                    nw = piece;
        }
    }
}

bool Chess::Piece::isPinned(const Piece& side1, const Piece& side2) const {
    char auxiliaryType = '\0';
    if (Position::sameCol(side1.position, side2.position) || Position::sameRow(side1.position, side2.position)) {
        auxiliaryType = 'R';
    }
    else if (Position::sameMainDiagonal(side1.position, side2.position) || Position::sameAntidiagonal(side1.position, side2.position)) {
        auxiliaryType = 'B';
    }
    if (auxiliaryType) {
        if (side1.getNotation() == 'K' && side1.color == color) {
            if ((side2.getNotation() == auxiliaryType || side2.getNotation() == 'Q') && side2.color != color) {
                return true;
            }
        }
        else if (side2.getNotation() == 'K' && side2.color == color) {
            if ((side1.getNotation() == auxiliaryType || side1.getNotation() == 'Q') && side1.color != color) {
                return true;
            }
        }
    }
    return false;
}

Chess::Piece::Pin Chess::Piece::getPinType(const Proximity& proximity) const {
    if (proximity.n && proximity.s && isPinned(*proximity.n, *proximity.s)) return Pin::vertical;
    if (proximity.e && proximity.w && isPinned(*proximity.e, *proximity.w)) return Pin::horizontal;
    if (proximity.nw && proximity.se && isPinned(*proximity.nw, *proximity.se)) return Pin::mainDiagonal;
    if (proximity.ne && proximity.sw && isPinned(*proximity.ne, *proximity.sw)) return Pin::antidiagonal;
    return Pin::none;
}

bool Chess::Piece::conformsToPin(const Position& nextMove, const Position& currentPosition, const Pin& pin) {
    switch (pin) {
    case Piece::Pin::none:
        return true;
    case Piece::Pin::vertical:
        return Piece::Position::sameCol(currentPosition, nextMove);
    case Piece::Pin::horizontal:
        return Piece::Position::sameRow(currentPosition, nextMove);
    case Piece::Pin::mainDiagonal:
        return Piece::Position::sameMainDiagonal(currentPosition, nextMove);
    case Piece::Pin::antidiagonal:
        return Piece::Position::sameAntidiagonal(currentPosition, nextMove);
    default:
        return false;
    }
}

void Chess::Piece::addMovesFromPieceToLimit(vector<Position>& moves, const Piece& piece, Piece* const& limit, const Pin& pin, void modify(Position&)) {
    Position nextMove = piece.position;
    modify(nextMove);
    if (nextMove == piece.position) return;
    while (Position::inBounds(nextMove) && conformsToPin(nextMove, piece.position, pin) && (!limit || nextMove != limit->position)) {
        moves.push_back(nextMove);
        modify(nextMove);
    }
    if (limit && limit->color != piece.color && conformsToPin(nextMove, piece.position, pin)) {
        moves.push_back(nextMove);
    }
}

Chess::Pawn::Pawn(Color color, Position position, bool enPassantCapturable) {
    this->color = color;
    this->position = position;
    this->enPassantCapturable = enPassantCapturable;
}

Chess::Piece* Chess::Pawn::newCopy() const {
    return new Pawn(color, position, enPassantCapturable);
}

char Chess::Pawn::getNotation() const { return 'P'; }

vector<Chess::Piece::Position> Chess::Pawn::getMoves(const vector<Piece*>& pieces) const {
    vector<Position> result;
    Proximity proximity(position, pieces);

    Pin pin = getPinType(proximity);

    if (color == Color::white) {
        // forwards movement
        if (!proximity.n || proximity.n->position.y != position.y + 1) {
            if (conformsToPin(Position{ position.x, position.y + 1 }, position, pin)) {
                result.push_back(Position{ position.x, position.y + 1 });
            }
            if (position.y == 2 && (!proximity.n || proximity.n->position.y != position.y + 2)) {
                if (conformsToPin(Position{ position.x, position.y + 2 }, position, pin)) {
                    result.push_back(Position{ position.x, position.y + 2 });
                }
            }
        }
        // diagonal captures
        if (proximity.ne && proximity.ne->color != color && proximity.ne->position.y == position.y + 1) {
            if (conformsToPin(proximity.ne->position, position, pin)) {
                result.push_back(proximity.ne->position);
            }
        }
        if (proximity.nw && proximity.nw->color != color && proximity.nw->position.y == position.y + 1) {
            if (conformsToPin(proximity.nw->position, position, pin)) {
                result.push_back(proximity.nw->position);
            }
        }
        // en passant
        if (position.y == 5) {
            if ((proximity.e && proximity.e->position.x == position.x + 1 && proximity.e->color != color && proximity.e->getNotation() == 'P') &&
                (!proximity.ne || proximity.ne->position.y != position.y + 1)) {
                Position enPassantCapture = Position{ char(position.x + 1), position.y + 1 };
                if (static_cast<Pawn*>(proximity.e)->enPassantCapturable && conformsToPin(enPassantCapture, position, pin)) {
                    Pawn enemyPawn(white, proximity.e->position); // given ally color to determine pin type
                    Proximity enemyProximity(enemyPawn.position, pieces);
                    Pin enemyPin = enemyPawn.getPinType(enemyProximity);
                    if ((enemyPin == Pin::none || enemyPin == Pin::vertical) &&
                        (!proximity.w || !enemyProximity.e || !isPinned(*proximity.w, *enemyProximity.e))) {
                        result.push_back(enPassantCapture);
                    }
                }
            }
            if ((proximity.w && proximity.w->position.x == position.x - 1 && proximity.w->color != color && proximity.w->getNotation() == 'P') &&
                (!proximity.nw || proximity.nw->position.y != position.y + 1)) {
                Position enPassantCapture = Position{ char(position.x - 1), position.y + 1 };
                if (static_cast<Pawn*>(proximity.w)->enPassantCapturable && conformsToPin(enPassantCapture, position, pin)) {
                    Pawn enemyPawn(white, proximity.w->position); // given ally color to determine pin type
                    Proximity enemyProximity(enemyPawn.position, pieces);
                    Pin enemyPin = enemyPawn.getPinType(enemyProximity);
                    if ((enemyPin == Pin::none || enemyPin == Pin::vertical) &&
                        (!proximity.e || !enemyProximity.w || !isPinned(*proximity.e, *enemyProximity.w))) {
                        result.push_back(enPassantCapture);
                    }
                }
            }
        }
    }
    else if (color == Color::black) {
        // forwards movement
        if (!proximity.s || proximity.s->position.y != position.y - 1) {
            if (conformsToPin(Position{ position.x, position.y - 1 }, position, pin)) {
                result.push_back(Position{ position.x, position.y - 1 });
            }
            if (position.y == 7 && (!proximity.s || proximity.s->position.y != position.y - 2)) {
                if (conformsToPin(Position{ position.x, position.y - 2 }, position, pin)) {
                    result.push_back(Position{ position.x, position.y - 2 });
                }
            }
        }
        // diagonal captures
        if (proximity.se && proximity.se->color != color && proximity.se->position.y == position.y - 1) {
            if (conformsToPin(proximity.se->position, position, pin)) {
                result.push_back(proximity.se->position);
            }
        }
        if (proximity.sw && proximity.sw->color != color && proximity.sw->position.y == position.y + 1) {
            if (conformsToPin(proximity.sw->position, position, pin)) {
                result.push_back(proximity.sw->position);
            }
        }
        // en passant
        if (position.y == 4) {
            if ((proximity.e && proximity.e->position.x == position.x + 1 && proximity.e->color != color && proximity.e->getNotation() == 'P') &&
                (!proximity.se || proximity.se->position.y != position.y - 1)) {
                Position enPassantCapture = Position{ char(position.x + 1), position.y - 1 };
                if (static_cast<Pawn*>(proximity.e)->enPassantCapturable && conformsToPin(enPassantCapture, position, pin)) {
                    Pawn enemyPawn(black, proximity.e->position); // given ally color to determine pin type
                    Proximity enemyProximity(enemyPawn.position, pieces);
                    Pin enemyPin = enemyPawn.getPinType(enemyProximity);
                    if ((enemyPin == Pin::none || enemyPin == Pin::vertical) &&
                        (!proximity.w || !enemyProximity.e || !isPinned(*proximity.w, *enemyProximity.e))) {
                        result.push_back(enPassantCapture);
                    }
                }
            }
            if ((proximity.w && proximity.w->position.x == position.x - 1 && proximity.w->color != color && proximity.w->getNotation() == 'P') &&
                (!proximity.sw || proximity.sw->position.y != position.y + 1)) {
                Position enPassantCapture = Position{ char(position.x - 1), position.y - 1 };
                if (static_cast<Pawn*>(proximity.w)->enPassantCapturable && conformsToPin(enPassantCapture, position, pin)) {
                    Pawn enemyPawn(black, proximity.w->position); // given ally color to determine pin type
                    Proximity enemyProximity(enemyPawn.position, pieces);
                    Pin enemyPin = enemyPawn.getPinType(enemyProximity);
                    if ((enemyPin == Pin::none || enemyPin == Pin::vertical) &&
                        (!proximity.e || !enemyProximity.w || !isPinned(*proximity.e, *enemyProximity.w))) {
                        result.push_back(enPassantCapture);
                    }
                }
            }
        }
    }
    // promotion handled in board's makeMove()

    return result;
}

Chess::Knight::Knight(Color color, Position position) {
    this->color = color;
    this->position = position;
}

Chess::Piece* Chess::Knight::newCopy() const {
    return new Knight(color, position);
}

char Chess::Knight::getNotation() const { return 'N'; }

vector<Chess::Piece::Position> Chess::Knight::getMoves(const vector<Piece*>& pieces) const {
    vector<Position> result;
    Proximity proximity(position, pieces);
    Pin pin = getPinType(proximity);

    if (pin == Pin::none) {
        vector<Position> possibleKnightMoves = {
            Position{char(position.x + 1), position.y + 2},
            Position{char(position.x + 2), position.y + 1},
            Position{char(position.x + 2), position.y - 1},
            Position{char(position.x + 1), position.y - 2},
            Position{char(position.x - 1), position.y - 2},
            Position{char(position.x - 2), position.y - 1},
            Position{char(position.x - 2), position.y + 1},
            Position{char(position.x - 1), position.y + 2}
        };
        for (Position& nextMove : possibleKnightMoves) {
            if (Position::inBounds(nextMove)) {
                Piece* samePosition = nullptr;
                for (int i = 0; i < pieces.size() && !samePosition; ++i) {
                    if (pieces[i] && nextMove == pieces[i]->position) {
                        samePosition = pieces[i];
                    }
                }
                if (!samePosition || samePosition->color != color) {
                    result.push_back(nextMove);
                }
            }
        }
    }

    return result;
}

Chess::Bishop::Bishop(Color color, Position position) {
    this->color = color;
    this->position = position;
}

Chess::Piece* Chess::Bishop::newCopy() const {
    return new Bishop(color, position);
}

char Chess::Bishop::getNotation() const { return 'B'; }

vector<Chess::Piece::Position> Chess::Bishop::getMoves(const vector<Piece*>& pieces) const {
    vector<Position> result;
    Proximity proximity(position, pieces);
    Pin pin = getPinType(proximity);

    addMovesFromPieceToLimit(result, *this, proximity.ne, pin, [](Position& p) { ++p.x; ++p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.se, pin, [](Position& p) { ++p.x; --p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.sw, pin, [](Position& p) { --p.x; --p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.nw, pin, [](Position& p) { --p.x; ++p.y; });

    return result;
}

Chess::Rook::Rook(Color color, Position position, bool canCastle) {
    this->color = color;
    this->position = position;
    this->canCastle = canCastle;
}

Chess::Piece* Chess::Rook::newCopy() const {
    return new Rook(color, position, canCastle);
}

char Chess::Rook::getNotation() const { return 'R'; }

vector<Chess::Piece::Position> Chess::Rook::getMoves(const vector<Piece*>& pieces) const {
    vector<Position> result;
    Proximity proximity(position, pieces);
    Pin pin = getPinType(proximity);

    addMovesFromPieceToLimit(result, *this, proximity.n, pin, [](Position& p) { ++p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.e, pin, [](Position& p) { ++p.x; });
    addMovesFromPieceToLimit(result, *this, proximity.s, pin, [](Position& p) { --p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.w, pin, [](Position& p) { --p.x; });

    return result;
}

Chess::Queen::Queen(Color color, Position position) {
    this->color = color;
    this->position = position;
}

Chess::Piece* Chess::Queen::newCopy() const {
    return new Queen(color, position);
}

char Chess::Queen::getNotation() const { return 'Q'; }

vector<Chess::Piece::Position> Chess::Queen::getMoves(const vector<Piece*>& pieces) const {
    vector<Position> result;
    Proximity proximity(position, pieces);
    Pin pin = getPinType(proximity);

    addMovesFromPieceToLimit(result, *this, proximity.n, pin, [](Position& p) { ++p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.e, pin, [](Position& p) { ++p.x; });
    addMovesFromPieceToLimit(result, *this, proximity.s, pin, [](Position& p) { --p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.w, pin, [](Position& p) { --p.x; });
    addMovesFromPieceToLimit(result, *this, proximity.ne, pin, [](Position& p) { ++p.x; ++p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.se, pin, [](Position& p) { ++p.x; --p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.sw, pin, [](Position& p) { --p.x; --p.y; });
    addMovesFromPieceToLimit(result, *this, proximity.nw, pin, [](Position& p) { --p.x; ++p.y; });

    return result;
}

Chess::King::King(Color color, Position position, bool canCastle) {
    this->color = color;
    this->position = position;
    this->canCastle = canCastle;
}

Chess::Piece* Chess::King::newCopy() const {
    return new King(color, position, canCastle);
}

char Chess::King::getNotation() const { return 'K'; }

vector<Chess::Piece*> Chess::King::enemiesAttackingPosition(const Color& allyColor, const Position& position, const vector<Piece*>& pieces) {
    vector<Piece*> result;

    Proximity proximity(position, pieces);

    // check for pawns
    if (allyColor == Color::white) {
        if (proximity.ne && proximity.ne->getNotation() == 'P' && proximity.ne->color != allyColor && proximity.ne->position.x == position.x + 1) result.push_back(proximity.ne);
        if (proximity.nw && proximity.nw->getNotation() == 'P' && proximity.nw->color != allyColor && proximity.nw->position.x == position.x - 1) result.push_back(proximity.nw);
    }
    else { // color == Color::black
        if (proximity.se && proximity.se->getNotation() == 'P' && proximity.se->color != allyColor && proximity.se->position.x == position.x + 1) result.push_back(proximity.se);
        if (proximity.sw && proximity.sw->getNotation() == 'P' && proximity.sw->color != allyColor && proximity.sw->position.x == position.x - 1) result.push_back(proximity.sw);
    }

    // check for knights
    vector<Position> possibleKnightMoves = {
        Position{char(position.x + 1), position.y + 2},
        Position{char(position.x + 2), position.y + 1},
        Position{char(position.x + 2), position.y - 1},
        Position{char(position.x + 1), position.y - 2},
        Position{char(position.x - 1), position.y - 2},
        Position{char(position.x - 2), position.y - 1},
        Position{char(position.x - 2), position.y + 1},
        Position{char(position.x - 1), position.y + 2}
    };
    for (Piece* const& piece : pieces) {
        if (piece && piece->getNotation() == 'N' && piece->color != allyColor) {
            for (const Position& possibleKnightMove : possibleKnightMoves) {
                if (piece->position == possibleKnightMove) result.push_back(piece);
            }
        }
    }

    // check for rooks, bishops, queens, and kings
    if (proximity.n && (proximity.n->getNotation() == 'R' || proximity.n->getNotation() == 'Q' || (proximity.n->getNotation() == 'K' && proximity.n->position.y == position.y + 1)) && proximity.n->color != allyColor) result.push_back(proximity.n);
    if (proximity.e && (proximity.e->getNotation() == 'R' || proximity.e->getNotation() == 'Q' || (proximity.e->getNotation() == 'K' && proximity.e->position.y == position.x + 1)) && proximity.e->color != allyColor) result.push_back(proximity.e);
    if (proximity.s && (proximity.s->getNotation() == 'R' || proximity.s->getNotation() == 'Q' || (proximity.s->getNotation() == 'K' && proximity.s->position.y == position.y - 1)) && proximity.s->color != allyColor) result.push_back(proximity.s);
    if (proximity.w && (proximity.w->getNotation() == 'R' || proximity.w->getNotation() == 'Q' || (proximity.w->getNotation() == 'K' && proximity.w->position.y == position.x - 1)) && proximity.w->color != allyColor) result.push_back(proximity.w);
    if (proximity.ne && (proximity.ne->getNotation() == 'B' || proximity.ne->getNotation() == 'Q' || (proximity.ne->getNotation() == 'K' && proximity.ne->position.x == position.x + 1)) && proximity.ne->color != allyColor) result.push_back(proximity.ne);
    if (proximity.se && (proximity.se->getNotation() == 'B' || proximity.se->getNotation() == 'Q' || (proximity.se->getNotation() == 'K' && proximity.se->position.x == position.x + 1)) && proximity.se->color != allyColor) result.push_back(proximity.se);
    if (proximity.sw && (proximity.sw->getNotation() == 'B' || proximity.sw->getNotation() == 'Q' || (proximity.sw->getNotation() == 'K' && proximity.sw->position.x == position.x + 1)) && proximity.sw->color != allyColor) result.push_back(proximity.sw);
    if (proximity.nw && (proximity.nw->getNotation() == 'B' || proximity.nw->getNotation() == 'Q' || (proximity.nw->getNotation() == 'K' && proximity.nw->position.x == position.x + 1)) && proximity.nw->color != allyColor) result.push_back(proximity.nw);

    return result;
}

vector<Chess::Piece*> Chess::King::inCheck(const vector<Piece*>& pieces) const {
    return enemiesAttackingPosition(color, position, pieces);
}

vector<Chess::Piece::Position> Chess::King::getSavingMoves(Piece* checking) {
    vector<Piece::Position> result;
    if (Piece::Position::sameCol(position, checking->position)) {
        if (position.y > checking->position.y) {
            addMovesFromPieceToLimit(result, *this, checking, none, [](Position& p) { --p.y; });
        }
        else {
            addMovesFromPieceToLimit(result, *this, checking, none, [](Position& p) { ++p.y; });
        }
    }
    else if (Piece::Position::sameRow(position, checking->position)) {
        if (position.x > checking->position.x) {
            addMovesFromPieceToLimit(result, *this, checking, none, [](Position& p) { --p.x; });
        }
        else {
            addMovesFromPieceToLimit(result, *this, checking, none, [](Position& p) { ++p.x; });
        }
    }
    else if (Piece::Position::sameMainDiagonal(position, checking->position)) {
        if (position.x > checking->position.x) {
            addMovesFromPieceToLimit(result, *this, checking, none, [](Position& p) { --p.x; ++p.y; });
        }
        else {
            addMovesFromPieceToLimit(result, *this, checking, none, [](Position& p) { ++p.x; --p.y; });
        }
    }
    else if (Piece::Position::sameAntidiagonal(position, checking->position)) {
        if (position.x > checking->position.x) {
            addMovesFromPieceToLimit(result, *this, checking, none, [](Position& p) { --p.x; --p.y; });
        }
        else {
            addMovesFromPieceToLimit(result, *this, checking, none, [](Position& p) { ++p.x; ++p.y; });
        }
    }
    return result;
}

vector<Chess::Piece::Position> Chess::King::getMoves(const vector<Piece*>& pieces) const {
    vector<Position> result;
    Proximity proximity(position, pieces);
    vector<Position> possibleKingMoves = {
        Position{char(position.x + 1), position.y + 1},
        Position{char(position.x + 1), position.y + 0},
        Position{char(position.x + 1), position.y - 1},
        Position{char(position.x + 0), position.y + 1},
        Position{char(position.x + 0), position.y - 1},
        Position{char(position.x - 1), position.y + 1},
        Position{char(position.x - 1), position.y + 0},
        Position{char(position.x - 1), position.y - 1}
    };
    for (const Position& nextMove : possibleKingMoves) {
        for (Position& nextMove : possibleKingMoves) {
            if (Piece::Position::inBounds(nextMove)) {
                Piece* samePosition = nullptr;
                for (int i = 0; i < pieces.size() && !samePosition; ++i) {
                    if (pieces[i] && nextMove == pieces[i]->position) {
                        samePosition = pieces[i];
                    }
                }
                if ((!samePosition || samePosition->color != color) && enemiesAttackingPosition(color, nextMove, pieces).empty()) {
                    result.push_back(nextMove);
                }
            }
        }
    }
    // castling
    if (canCastle && inCheck(pieces).empty()) {
        bool leftPathClear = true;
        bool rightPathClear = true;
        for (Piece* const& piece : pieces) {
            if (piece && piece->position.y == position.y) {
                if (piece->position.x < position.x && piece->position.x > 'A') {
                    leftPathClear = false;
                }
                else if (piece->position.x > position.x && piece->position.x < 'H') {
                    rightPathClear = false;
                }
            }
        }
        for (Piece* const& piece : pieces) {
            if (piece && piece->getNotation() == 'R' && piece->color == color && static_cast<Rook*>(piece)->canCastle) {
                if (leftPathClear && piece->position == Position{ 1, position.y }) {
                    Position leftCastle = Position{ char(position.x - 2), position.y };
                    if (enemiesAttackingPosition(color, Position{ char(position.x - 1), position.y }, pieces).empty() && enemiesAttackingPosition(color, leftCastle, pieces).empty()) {
                        result.push_back(leftCastle);
                    }
                }
                else if (rightPathClear && piece->position == Position{ 8, position.y }) {
                    Position rightCastle = Position{ char(position.x + 2), position.y };
                    if (enemiesAttackingPosition(color, Position{ char(position.x + 1), position.y }, pieces).empty() && enemiesAttackingPosition(color, rightCastle, pieces).empty()) {
                        result.push_back(rightCastle);
                    }
                }
            }
        }
    }

    return result;
}

void Chess::Board::updateAvailableMoves() {
    vector<Move> result;
    Piece* allyKing = nullptr;
    for (Piece* const& piece : pieces) {
        if (piece && piece->color == getCurrentTurn()) {
            Piece::Position from = piece->position;
            vector<Piece::Position> tos = piece->getMoves(pieces);
            for (const Piece::Position& to : tos) {
                result.push_back({ from, to });
            }
            if (piece->getNotation() == 'K') {
                allyKing = piece;
            }
        }
    }
    if (!allyKing) {
        availableMoves = { };
        return;
    }
    vector<Piece*> checkingPieces = static_cast<King*>(allyKing)->inCheck(pieces);
    if (checkingPieces.size() != 0) {
        vector<Move> checkResult;
        if (checkingPieces.size() == 1) {
            vector<Piece::Position> savingMoves = static_cast<King*>(allyKing)->getSavingMoves(checkingPieces[0]);
            for (const Move& move : result) {
                if (move.from == allyKing->position) {
                    checkResult.push_back(move);
                }
                else {
                    for (const Piece::Position& savingMove : savingMoves) {
                        if (move.to == savingMove) {
                            checkResult.push_back(move);
                        }
                    }
                }
            }
        }
        else { // checkingPieces.size() > 1
            for (const auto& move : result) {
                if (move.from == allyKing->position) {
                    checkResult.push_back(move);
                }
            }
        }
        result = checkResult;
    }
    availableMoves = result;
}

Chess::Board::Board() {
    setDefaultGame();
}

Chess::Board::Board(const Board& board) {
    for (Piece* piece : board.pieces) {
        if (piece) {
            pieces.push_back(piece->newCopy());
        }
    }

    turnOrder = board.turnOrder;

    int turnCount = 0;
    auto i = board.turnOrder.begin();
    while (i != board.currentTurn) {
        ++turnCount;
    }
    currentTurn = turnOrder.begin();
    while (turnCount > 0) {
        ++currentTurn;
        --turnCount;
    }

    availableMoves = board.availableMoves;
}

Chess::Board::~Board() {
    for (auto& piece : pieces) {
        delete piece;
    }
}

vector<tuple<char, Chess::Piece::Color, Chess::Piece::Position>> Chess::Board::getPieces() const {
    vector<tuple<char, Piece::Color, Piece::Position>> result;
    for (Piece* piece : pieces) {
        if (piece) {
            tuple<char, Piece::Color, Piece::Position> pieceData;
            pieceData = { piece->getNotation(), piece->color, piece->position };
            result.push_back(pieceData);
        }
    }
    return result;
}

Chess::Piece::Color Chess::Board::getCurrentTurn() const {
    return *currentTurn;
}

vector<Chess::Move> Chess::Board::getAvailableMoves() const {
    return availableMoves;
}

void Chess::Board::setDefaultGame() {
    turnOrder = {
        Piece::Color::white,
        Piece::Color::black
    };

    currentTurn = turnOrder.begin();

    for (auto& piece : pieces) {
        delete piece;
    }

    pieces = {
        new Rook  {Piece::Color::white, Piece::Position{'A', 1}},
        new Knight{Piece::Color::white, Piece::Position{'B', 1}},
        new Bishop{Piece::Color::white, Piece::Position{'C', 1}},
        new Queen {Piece::Color::white, Piece::Position{'D', 1}},
        new King  {Piece::Color::white, Piece::Position{'E', 1}},
        new Bishop{Piece::Color::white, Piece::Position{'F', 1}},
        new Knight{Piece::Color::white, Piece::Position{'G', 1}},
        new Rook  {Piece::Color::white, Piece::Position{'H', 1}},
        new Pawn  {Piece::Color::white, Piece::Position{'A', 2}},
        new Pawn  {Piece::Color::white, Piece::Position{'B', 2}},
        new Pawn  {Piece::Color::white, Piece::Position{'C', 2}},
        new Pawn  {Piece::Color::white, Piece::Position{'D', 2}},
        new Pawn  {Piece::Color::white, Piece::Position{'E', 2}},
        new Pawn  {Piece::Color::white, Piece::Position{'F', 2}},
        new Pawn  {Piece::Color::white, Piece::Position{'G', 2}},
        new Pawn  {Piece::Color::white, Piece::Position{'H', 2}},

        new Pawn  {Piece::Color::black, Piece::Position{'A', 7}},
        new Pawn  {Piece::Color::black, Piece::Position{'B', 7}},
        new Pawn  {Piece::Color::black, Piece::Position{'C', 7}},
        new Pawn  {Piece::Color::black, Piece::Position{'D', 7}},
        new Pawn  {Piece::Color::black, Piece::Position{'E', 7}},
        new Pawn  {Piece::Color::black, Piece::Position{'F', 7}},
        new Pawn  {Piece::Color::black, Piece::Position{'G', 7}},
        new Pawn  {Piece::Color::black, Piece::Position{'H', 7}},
        new Rook  {Piece::Color::black, Piece::Position{'A', 8}},
        new Knight{Piece::Color::black, Piece::Position{'B', 8}},
        new Bishop{Piece::Color::black, Piece::Position{'C', 8}},
        new Queen {Piece::Color::black, Piece::Position{'D', 8}},
        new King  {Piece::Color::black, Piece::Position{'E', 8}},
        new Bishop{Piece::Color::black, Piece::Position{'F', 8}},
        new Knight{Piece::Color::black, Piece::Position{'G', 8}},
        new Rook  {Piece::Color::black, Piece::Position{'H', 8}}
    };

    updateAvailableMoves();
}

bool Chess::Board::makeMove(const Piece::Position& from, const Piece::Position& to) {
    if (!Piece::Position::inBounds(from) || !Piece::Position::inBounds(to) || from == to) return false;

    bool moveIsAvailable = false;
    for (const Move& availableMove : availableMoves) {
        if (availableMove.from == from && availableMove.to == to) {
            moveIsAvailable = true;
        }
    }
    if (!moveIsAvailable) {
        return false;
    }

    int f = -1; // from piece index
    int t = -1; // to piece index
    for (int i = 0; i < pieces.size(); ++i) {
        if (pieces[i]) {
            if (pieces[i]->position == from) f = i;
            if (pieces[i]->position == to) t = i;
        }
    }
    if (f == -1) return false;

    for (Piece*& piece : pieces) {
        if (piece && piece->getNotation() == 'P') {
            static_cast<Pawn*>(piece)->enPassantCapturable = false;
        }
    }

    if (pieces[f]->getNotation() == 'P') {
        static_cast<Pawn*>(pieces[f])->enPassantCapturable = (from.y - to.y == 2 || from.y - to.y == -2);
        // enPassant capture
        if (t == -1 && from.x != to.x) {
            for (Piece*& piece : pieces) {
                if (piece && piece->position == Piece::Position{ to.x, from.y }) {
                    delete piece;
                    piece = nullptr;
                }
            }
        }
        // promotion
        if ((pieces[f]->color == Piece::Color::white && to.y == 8) ||
            (pieces[f]->color == Piece::Color::black && to.y == 1)) {
            for (Piece*& piece : pieces) {
                if (piece && piece->position == from) {
                    Piece* promotedPawn = new Queen{ pieces[f]->color, to };
                    delete piece;
                    piece = promotedPawn;
                }
            }
        }
    }
    else if (pieces[f]->getNotation() == 'R') {
        static_cast<Rook*>(pieces[f])->canCastle = false;
    }
    else if (pieces[f]->getNotation() == 'K') {
        static_cast<King*>(pieces[f])->canCastle = false;
        // castling
        if (from.x - to.x > 1) {
            Piece* castlingRook = nullptr;
            for (Piece* const& piece : pieces) {
                if (piece && piece->position == Piece::Position{ 1, from.y }) {
                    castlingRook = piece;
                }
            }
            if (!castlingRook) return false;
            castlingRook->position.x = from.x - 1;
        }
        else if (from.x - to.x < -1) {
            Piece* castlingRook = nullptr;
            for (Piece* const& piece : pieces) {
                if (piece && piece->position == Piece::Position{ 8, from.y }) {
                    castlingRook = piece;
                }
            }
            if (!castlingRook) return false;
            castlingRook->position.x = from.x + 1;
        }
    }

    if (t != -1) {
        delete pieces[t];
        pieces[t] = nullptr;
    }
    pieces[f]->position = to;

    ++currentTurn;
    if (currentTurn == turnOrder.end()) {
        currentTurn = turnOrder.begin();
    }

    updateAvailableMoves();

    return true;
}

bool Chess::Board::makeMove(const Move& selectedMove) {
    return makeMove(selectedMove.from, selectedMove.to);
}