// chess_game.cpp
// by Jake Charles Osborne III



#include "chess_game.h"
#include <vector>
#include <functional>
#include <optional>
#include <tuple>



using std::vector;
using std::function;
using std::optional;
using std::nullopt;
using std::tuple;



std::vector<Chess::Piece*> Chess::Piece::getPieces(const Board& board) { return board.pieces; }

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

bool Chess::Piece::Position::heldByAlly(const Position& position, const Color& allyColor, const Board& board) {
    for (Piece* const& piece : board.pieces) {
        if (piece && piece->color == allyColor && piece->position == position) {
            return true;
        }
    }
    return false;
}

Chess::Pawn::Pawn(Color color, Position position, bool firstMove, bool enPassantCapturable) {
    this->color = color;
    this->position = position;
    this->firstMove = firstMove;
    this->enPassantCapturable = enPassantCapturable;
}

Chess::Piece* Chess::Pawn::newCopy() const {
    return new Pawn(color, position, enPassantCapturable);
}

vector<vector<Chess::Move>> Chess::Pawn::getMoves(const Board& board) {
    vector<vector<Move>> result;

    if (color == Color::white) {
        bool canMoveForward = Position::inBounds(Position{ position.x, position.y + 1 });
        bool extraSpaceAvailable = true;
        Piece* neCapture = nullptr;
        Piece* nwCapture = nullptr;
        Piece* eEnPassantCapture = nullptr;
        Piece* wEnPassantCapture = nullptr;
        for (Piece* const& piece : getPieces(board)) {
            if (piece) {
                if (piece->position == Position{ position.x, position.y + 1 })
                    canMoveForward = false;
                if (piece->position == Position{ position.x, position.y + 2 })
                    extraSpaceAvailable = false;
                if (piece->position == Position{ char(position.x + 1), position.y + 1 })
                    neCapture = piece;
                if (piece->position == Position{ char(position.x - 1), position.y + 1 })
                    nwCapture = piece;
                if (piece->position == Position{ char(position.x + 1), position.y } && piece->getNotation() == 'P' && static_cast<Pawn*>(piece)->enPassantCapturable)
                    eEnPassantCapture = piece;
                if (piece->position == Position{ char(position.x - 1), position.y } && piece->getNotation() == 'P' && static_cast<Pawn*>(piece)->enPassantCapturable)
                    eEnPassantCapture = piece;
            }
        }

        function<void()> pawnMoveEffect = [&]() {
            firstMove = false;
            enPassantCapturable = false; // ... this isn't done when another move is made ... it's done when another turn is made ...
        };

        // forwards movement
        if (canMoveForward) {
            vector<Move>forwardsMovement;

            Move forwardMove(position, Position{ position.x, position.y + 1 }, pawnMoveEffect);
            forwardsMovement.push_back(forwardMove);

            // extra move on first turn
            if (firstMove && extraSpaceAvailable) {
                Move extraMove(position, Position{ position.x, position.y + 2 }, pawnMoveEffect);
                forwardsMovement.push_back(extraMove);
            }

            result.push_back(forwardsMovement);
        }

        // ne capture
        if (neCapture) {
            Move neMove(position, neCapture->position, pawnMoveEffect);
            result.push_back(vector{ neMove });
        }

        // nw capture
        if (nwCapture) {
            Move nwMove(position, nwCapture->position, pawnMoveEffect);
            result.push_back(vector{ nwMove });
        }

        /*
        
        // east en passant
        if (eEnPassantCapture) {
            // discovery checks:
            // copy the board with modified positions
            // iterate through and check positionUnderAttack on pieceToCapture
            errorSquiglesText;
            // maybe only offer when neCapture isn't available? uWu

            Move eEnPassantMove(position, Position{ char(position.x + 1), position.y + 1 }, [&]() {
                firstMove = false;
                enPassantCapturable = false; // ... this isn't done when another move is made ... it's done when another turn is made ...
                if (position.y == 8) {
                    for (Piece*& piece : getPieces(board)) {
                        if (piece && piece->position == position) {
                            delete piece;
                            piece = new Queen(color, position);
                        }
                    }
                }
                delete eEnPassantCapture;
                eEnPassantCapture = nullptr;
            });
            result.push_back(vector{ eEnPassantMove });
        }

        // west en passant
        if (eEnPassantCapture) {
            errorSquiglesText;
        }

        */

    }
    else if (color == Color::black) {
        bool canMoveForward = Position::inBounds(Position{ position.x, position.y - 1 });
        bool extraSpaceAvailable = true;
        Piece* seCapture = nullptr;
        Piece* swCapture = nullptr;
        Piece* eEnPassantCapture = nullptr;
        Piece* wEnPassantCapture = nullptr;
        for (Piece* const& piece : getPieces(board)) {
            if (piece) {
                if (piece->position == Position{ position.x, position.y - 1 })
                    canMoveForward = false;
                if (piece->position == Position{ position.x, position.y - 2 })
                    extraSpaceAvailable = false;
                if (piece->position == Position{ char(position.x + 1), position.y - 1 })
                    seCapture = piece;
                if (piece->position == Position{ char(position.x - 1), position.y - 1 })
                    swCapture = piece;
                if (piece->position == Position{ char(position.x + 1), position.y } && piece->getNotation() == 'P' && static_cast<Pawn*>(piece)->enPassantCapturable)
                    eEnPassantCapture = piece;
                if (piece->position == Position{ char(position.x - 1), position.y } && piece->getNotation() == 'P' && static_cast<Pawn*>(piece)->enPassantCapturable)
                    eEnPassantCapture = piece;
            }
        }

        function<void()> pawnMoveEffect = [&]() {
            firstMove = false;
            enPassantCapturable = false; // ... this isn't done when another move is made ... it's done when another turn is made ...
        };

        // forwards movement
        if (canMoveForward) {
            vector<Move>forwardsMovement;

            Move forwardMove(position, Position{ position.x, position.y - 1 }, pawnMoveEffect);
            forwardsMovement.push_back(forwardMove);

            // extra move on first turn
            if (firstMove && extraSpaceAvailable) {
                Move extraMove(position, Position{ position.x, position.y - 2 }, pawnMoveEffect);
                forwardsMovement.push_back(extraMove);
            }

            result.push_back(forwardsMovement);
        }

        // se capture
        if (seCapture) {
            Move neMove(position, seCapture->position, pawnMoveEffect);
            result.push_back(vector{ neMove });
        }

        // sw capture
        if (swCapture) {
            Move nwMove(position, swCapture->position, pawnMoveEffect);
            result.push_back(vector{ nwMove });
        }

        // copy en passant result here
    }

    return result;
}

char Chess::Pawn::getNotation() const { return 'P'; }

Chess::Knight::Knight(Color color, Position position) {
    this->color = color;
    this->position = position;
}

Chess::Piece* Chess::Knight::newCopy() const {
    return new Knight(color, position);
}

vector<vector<Chess::Move>> Chess::Knight::getMoves(const Board& board) {
    vector<vector<Move>> result;

    vector<Move> pattern;
    Position posCopy;

    // NNE
    pattern.clear();
    posCopy = position;
    posCopy.x += 1;
    posCopy.y += 2;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // NEE
    pattern.clear();
    posCopy = position;
    posCopy.x += 2;
    posCopy.y += 1;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // SEE
    pattern.clear();
    posCopy = position;
    posCopy.x += 2;
    posCopy.y -= 1;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // SSE
    pattern.clear();
    posCopy = position;
    posCopy.x += 1;
    posCopy.y -= 2;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // SSW
    pattern.clear();
    posCopy = position;
    posCopy.x -= 1;
    posCopy.y -= 2;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // SWW
    pattern.clear();
    posCopy = position;
    posCopy.x -= 2;
    posCopy.y -= 1;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // NWW
    pattern.clear();
    posCopy = position;
    posCopy.x -= 2;
    posCopy.y += 1;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // NNW
    pattern.clear();
    posCopy = position;
    posCopy.x -= 1;
    posCopy.y += 2;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    return result;
}

char Chess::Knight::getNotation() const { return 'N'; }

Chess::Bishop::Bishop(Color color, Position position) {
    this->color = color;
    this->position = position;
}

Chess::Piece* Chess::Bishop::newCopy() const {
    return new Bishop(color, position);
}

vector<vector<Chess::Move>> Chess::Bishop::getMoves(const Board& board) {
    vector<vector<Move>> result;

    vector<Move> pattern;
    Piece* limit;
    Position posCopy;
    
    // Northeast Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameAntidiagonal(piece->position, position) && piece->position.x > position.x)
                if (!limit || piece->position.x < limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    ++posCopy.x;
    ++posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        ++posCopy.x;
        ++posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // Southeast Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameMainDiagonal(piece->position, position) && piece->position.x > position.x)
                if (!limit || piece->position.x < limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    ++posCopy.x;
    --posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        ++posCopy.x;
        --posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // Southwest Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameAntidiagonal(piece->position, position) && piece->position.x < position.x)
                if (!limit || piece->position.x > limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    --posCopy.x;
    --posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        --posCopy.x;
        --posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // Northwest Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameMainDiagonal(piece->position, position) && piece->position.x < position.x)
                if (!limit || piece->position.x > limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    --posCopy.x;
    ++posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        --posCopy.x;
        ++posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    return result;
}

char Chess::Bishop::getNotation() const { return 'B'; }

Chess::Rook::Rook(Color color, Position position, bool canCastle) {
    this->color = color;
    this->position = position;
    this->canCastle = canCastle;
}

Chess::Piece* Chess::Rook::newCopy() const {
    return new Rook(color, position, canCastle);
}

vector<vector<Chess::Move>> Chess::Rook::getMoves(const Board& board) {
    vector<vector<Move>> result;

    function<void()> rookMoveEffect = [&]() {
        canCastle = false;
    };

    vector<Move> pattern;
    Piece* limit;
    Position posCopy;

    // North Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameCol(piece->position, position) && piece->position.y > position.y)
                if (!limit || piece->position.y < limit->position.y)
                    limit = piece;
    }
    posCopy = position;
    ++posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy, rookMoveEffect));
        ++posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy, rookMoveEffect));
    }
    result.push_back(pattern);

    // East Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameRow(piece->position, position) && piece->position.x > position.x)
                if (!limit || piece->position.x < limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    ++posCopy.x;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy, rookMoveEffect));
        ++posCopy.x;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy, rookMoveEffect));
    }
    result.push_back(pattern);

    // South Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameCol(piece->position, position) && piece->position.y < position.y)
                if (!limit || piece->position.y > limit->position.y)
                    limit = piece;
    }
    posCopy = position;
    --posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy, rookMoveEffect));
        --posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy, rookMoveEffect));
    }
    result.push_back(pattern);

    // West Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameRow(piece->position, position) && piece->position.x < position.x)
                if (!limit || piece->position.x > limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    --posCopy.x;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy, rookMoveEffect));
        --posCopy.x;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy, rookMoveEffect));
    }
    result.push_back(pattern);

    return result;
}

char Chess::Rook::getNotation() const { return 'R'; }

Chess::Queen::Queen(Color color, Position position) {
    this->color = color;
    this->position = position;
}

Chess::Piece* Chess::Queen::newCopy() const {
    return new Queen(color, position);
}

vector<vector<Chess::Move>> Chess::Queen::getMoves(const Board& board) {
    vector<vector<Move>> result;

    vector<Move> pattern;
    Piece* limit;
    Position posCopy;

    // North Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameCol(piece->position, position) && piece->position.y > position.y)
                if (!limit || piece->position.y < limit->position.y)
                    limit = piece;
    }
    posCopy = position;
    ++posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        ++posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // East Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameRow(piece->position, position) && piece->position.x > position.x)
                if (!limit || piece->position.x < limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    ++posCopy.x;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        ++posCopy.x;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // South Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameCol(piece->position, position) && piece->position.y < position.y)
                if (!limit || piece->position.y > limit->position.y)
                    limit = piece;
    }
    posCopy = position;
    --posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        --posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // West Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameRow(piece->position, position) && piece->position.x < position.x)
                if (!limit || piece->position.x > limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    --posCopy.x;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        --posCopy.x;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // Northeast Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameAntidiagonal(piece->position, position) && piece->position.x > position.x)
                if (!limit || piece->position.x < limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    ++posCopy.x;
    ++posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        ++posCopy.x;
        ++posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // Southeast Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameMainDiagonal(piece->position, position) && piece->position.x > position.x)
                if (!limit || piece->position.x < limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    ++posCopy.x;
    --posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        ++posCopy.x;
        --posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // Southwest Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameAntidiagonal(piece->position, position) && piece->position.x < position.x)
                if (!limit || piece->position.x > limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    --posCopy.x;
    --posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        --posCopy.x;
        --posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    // Northwest Movement
    pattern.clear();
    limit = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece)
            if (Position::sameMainDiagonal(piece->position, position) && piece->position.x < position.x)
                if (!limit || piece->position.x > limit->position.x)
                    limit = piece;
    }
    posCopy = position;
    --posCopy.x;
    ++posCopy.y;
    while (Position::inBounds(posCopy) && (!limit || posCopy != limit->position)) {
        pattern.push_back(Move(position, posCopy));
        --posCopy.x;
        ++posCopy.y;
    }
    if (limit && limit->color != color) {
        pattern.push_back(Move(position, posCopy));
    }
    result.push_back(pattern);

    return result;
}

char Chess::Queen::getNotation() const { return 'Q'; }

Chess::King::King(Color color, Position position, bool canCastle) {
    this->color = color;
    this->position = position;
    this->canCastle = canCastle;
}

Chess::Piece* Chess::King::newCopy() const {
    return new King(color, position, canCastle);
}

vector<vector<Chess::Move>> Chess::King::getMoves(const Board& board) {
    vector<vector<Move>> result;

    function<void()> kingMoveEffect = [&]() {
        canCastle = false;
    };

    vector<Move> pattern;
    Position posCopy;

    // North
    pattern.clear();
    posCopy = position;
    ++posCopy.y;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy, kingMoveEffect));
    }
    result.push_back(pattern);

    // East
    pattern.clear();
    posCopy = position;
    ++posCopy.x;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy, kingMoveEffect));
    }
    result.push_back(pattern);

    // South
    pattern.clear();
    posCopy = position;
    --posCopy.y;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy, kingMoveEffect));
    }
    result.push_back(pattern);

    // West
    pattern.clear();
    posCopy = position;
    --posCopy.x;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy, kingMoveEffect));
    }
    result.push_back(pattern);

    // Northeast
    pattern.clear();
    posCopy = position;
    ++posCopy.x;
    ++posCopy.y;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy, kingMoveEffect));
    }
    result.push_back(pattern);

    // Southeast
    pattern.clear();
    posCopy = position;
    ++posCopy.x;
    --posCopy.y;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy, kingMoveEffect));
    }
    result.push_back(pattern);

    // Southwest
    pattern.clear();
    posCopy = position;
    --posCopy.x;
    --posCopy.y;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy, kingMoveEffect));
    }
    result.push_back(pattern);

    // Northwest
    pattern.clear();
    posCopy = position;
    --posCopy.x;
    ++posCopy.y;
    if (Position::inBounds(posCopy) && !Position::heldByAlly(posCopy, color, board)) {
        pattern.push_back(Move(position, posCopy, kingMoveEffect));
    }
    result.push_back(pattern);

    // Castling
    if (canCastle && color == board.getCurrentTurn() && !board.positionUnderAttack(color, position)) {
        bool leftPathClear = true;
        bool rightPathClear = true;
        for (Piece* const& piece : getPieces(board)) {
            if (piece && piece->position.y == position.y) {
                if (piece->position.x < position.x && piece->position.x > 'A') {
                    leftPathClear = false;
                }
                else if (piece->position.x > position.x && piece->position.x < 'H') {
                    rightPathClear = false;
                }
            }
        }
        for (Piece* const& piece : getPieces(board)) {
            if (piece && piece->color == color && piece->getNotation() == 'R' && static_cast<Rook*>(piece)->canCastle) {
                if (leftPathClear && piece->position == Position{ 1, position.y }) {
                    Move leftCastle = Move(position, Position{ char(position.x - 2), position.y }, [&]() {
                        piece->position.x = char(position.x - 1);
                        canCastle = false;
                    });
                    if (board.positionUnderAttack(color, Position{ char(position.x - 1), position.y }) == nullopt && board.positionUnderAttack(color, leftCastle.to) == nullopt) {
                        result.push_back(vector{ leftCastle });
                    }
                }
                else if (rightPathClear && piece->position == Position{ 8, position.y }) {
                    Move rightCastle = Move(position, Position{ char(position.x + 2), position.y }, [&]() {
                        piece->position.x = char(position.x + 1);
                        canCastle = false;
                    });
                    if (board.positionUnderAttack(color, Position{ char(position.x + 1), position.y }) == nullopt && board.positionUnderAttack(color, rightCastle.to) == nullopt) {
                        result.push_back(vector{ rightCastle });
                    }
                }
            }
        }
    }

    return result;
}

char Chess::King::getNotation() const { return 'K'; }

Chess::Move::Move(Piece::Position from, Piece::Position to, optional<function<void()>> effect) {
    this->from = from;
    this->to = to;
    this->effect = effect;
}

optional<vector<Chess::Piece::Position>> Chess::Board::isPinned(Piece* const& potentiallyPinned) {
    if (!potentiallyPinned) return nullopt;

    int potentiallyPinnedIndex = std::distance(pieces.begin(), std::find(pieces.begin(), pieces.end(), potentiallyPinned));
    Piece* temp = pieces[potentiallyPinnedIndex];
    pieces[potentiallyPinnedIndex] = nullptr;


    Piece* allyPieceToCapture = nullptr;
    for (Piece*& piece : pieces) {
        if (piece && piece->color == temp->color && piece->getNotation() == pieceTypeToCapture) {
            if (!allyPieceToCapture) allyPieceToCapture = piece;
            else return nullopt; // set pieces[potentiallyPinnedIndex] to temp;
        }
    }
    if (!allyPieceToCapture) return nullopt;
    if (!allyPieceToCapture || temp->position == allyPieceToCapture->position) {
        pieces[potentiallyPinned] = temp;
        return nullopt;
    }
    
    optional<vector<Piece::Position>> result = positionUnderAttack(*allyPieceToCapture);

    pieces[potentiallyPinnedIndex] = temp;

    return result;
}

void Chess::Board::updateAvailableMoves() {
    vector<Piece*> piecesToCapture;
    for (Piece* const& piece : pieces) {
        if (piece && piece->color == getCurrentTurn() && piece->getNotation() == pieceTypeToCapture) {
            piecesToCapture.push_back(piece);
        }
    }
    if (piecesToCapture.empty()) {
        availableMoves = { };
        return;
    }
    Piece* pieceToCapture = nullptr;
    optional<vector<Piece::Position>> inCheckMoves = nullopt;
    if (piecesToCapture.size() == 1) {
        pieceToCapture = piecesToCapture[0];
        inCheckMoves = positionUnderAttack(*pieceToCapture);
    }
    
    vector<Move> result;
    for (Piece* const& piece : pieces) {
        if (piece && piece->color == getCurrentTurn()) {
            optional<vector<Piece::Position>> pinnedMoves = isPinned(piece);
            vector<vector<Move>> patterns = piece->getMoves(*this);


            for (const vector<Move>& pattern : patterns) {
                for (const Move& move : pattern) {


                    if (pieceToCapture && piece == pieceToCapture) {
                        if (positionUnderAttack(piece->color, move.to) == nullopt) {

                            result.push_back(move);

                        }
                    }
                    else {
                        if ((inCheckMoves == nullopt || std::find(inCheckMoves->begin(), inCheckMoves->end(), move.to) != inCheckMoves->end()) &&
                            (pinnedMoves == nullopt || std::find(pinnedMoves->begin(), pinnedMoves->end(), move.to) != pinnedMoves->end())) {

                            result.push_back(move);

                        }
                    }


                }
            }


        }
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

    pieceTypeToCapture = board.pieceTypeToCapture;

    turnOrder = board.turnOrder;

    int turnCount = 0;
    auto iteratorCopy = board.turnOrder.begin();
    while (iteratorCopy != board.currentTurn) {
        ++turnCount;
    }
    currentTurn = turnOrder.begin();
    while (turnCount > 0) {
        ++currentTurn;
        --turnCount;
    }

    this->updateAvailableMoves();
}

Chess::Board::~Board() {
    for (Piece*& piece : pieces) {
        delete piece;
    }
}

void Chess::Board::setDefaultGame() {
    turnOrder = {
        Piece::Color::white,
        Piece::Color::black
    };

    currentTurn = turnOrder.begin();

    pieceTypeToCapture = 'K';

    for (Piece*& piece : pieces) {
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

optional<vector<Chess::Piece::Position>> Chess::Board::positionUnderAttack(const Piece::Color& allyColor, const Piece::Position& position) const {
    optional<vector<Piece::Position>> result = nullopt;

    for (Piece* const& piece : pieces) {
        if (piece && piece->color != allyColor) {
            for (vector<Move>& pattern : piece->getMoves(*this)) {
                bool patternContainsPosition = false;
                for (Move& move : pattern) {
                    if (move.to == position) patternContainsPosition = true;
                }
                if (patternContainsPosition) {
                    vector<Piece::Position> blocksAttack;
                    blocksAttack.push_back(pattern[0].from);
                    for (Move& move : pattern) {
                        blocksAttack.push_back(move.to);
                    }
                    vector<Piece::Position> overlap;
                    for (Piece::Position& block : blocksAttack) {
                        if (result == nullopt || std::find(result->begin(), result->end(), block) != result->end()) {
                            overlap.push_back(block);
                        }
                    }
                    result = overlap;
                }
            }
        }
    }

    return result;
}

optional<vector<Chess::Piece::Position>> Chess::Board::positionUnderAttack(const Chess::Piece& piece) const {
    return positionUnderAttack(piece.color, piece.position);
}

bool Chess::Board::pieceToCaptureInCheck(const Piece::Color& color) const {
    Piece* pieceToCapture = nullptr;
    for (Piece* const& piece : pieces) {
        if (piece && piece->color == color && piece->getNotation() == pieceTypeToCapture) {
            if (!pieceToCapture) pieceToCapture = piece;
            else return false;
        }
    }
    if (!pieceToCapture) {
        return false;
    }
    return positionUnderAttack(*pieceToCapture) != nullopt;
}

bool Chess::Board::makeMove(const int& moveIndex) {
    if (moveIndex < 0 || moveIndex > availableMoves.size() - 1) return false;
    Move move = availableMoves[moveIndex];

    int f = -1; // from piece index
    int t = -1; // to piece index
    for (int i = 0; i < pieces.size(); ++i) {
        if (pieces[i]) {
            if (pieces[i]->position == move.from) f = i;
            if (pieces[i]->position == move.to) t = i;
        }
    }
    if (f == -1) return false;

    pieces[f]->position = move.to;
    if (toIndex != -1) {
        delete pieces[t];
        pieces[t] = nullptr;
    }
    if (move.effect != nullopt) {
        //(*move.effect)(); // possibly replaces the two lines below
        function<void()> f = *move.effect;
        f();
    }

    ++currentTurn;
    if (currentTurn == turnOrder.end()) {
        currentTurn = turnOrder.begin();
    }

    updateAvailableMoves();

    return true;
}