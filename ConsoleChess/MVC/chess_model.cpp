// chess_model.cpp
// by Jake Charles Osborne III



#include "chess_game.h"
#include <vector>
#include <functional>
#include <optional>
#include <tuple>
#include <future>

using std::vector;
using std::function;
using std::optional;
using std::nullopt;
using std::tuple;
using std::async;
using std::future;



vector<Chess::Piece*> Chess::Piece::getPieces(const Board& board) { return board.pieces; }


vector<vector<Chess::Move>> Chess::Piece::generateMovesWithPattern(const vector<Position>& pattern, const Board& board) {
    vector<vector<Move>> result;

    for (Position nextPosition : pattern) {
        optional<Color> heldBy = Position::heldBy(nextPosition, board);
        if (Position::inBounds(nextPosition) && (!heldBy || heldBy.value() != color)) {
            result.push_back( { Move(position, nextPosition) } );
        }
    }

    return result;
}

vector<vector<Chess::Move>> Chess::Piece::generateMovesWithPattern(const vector<function<Position(Position) >> &pattern, const Board& board) {
    vector<vector<Move>> result;

    vector<Move> subset;
    for (function f : pattern) {
        Position nextPosition = f(position);
        optional<Color> heldBy = Position::heldBy(nextPosition, board);
        while (Position::inBounds(nextPosition) && (!heldBy || heldBy.value() != color)) {
            subset.push_back(Move(position, nextPosition));
            if (heldBy) break;
            nextPosition = f(nextPosition);
            heldBy = Position::heldBy(nextPosition, board);
        }

        result.push_back(subset);
        subset.clear();
    }

    return result;
}

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

std::optional<Chess::Piece::Color> Chess::Piece::Position::heldBy(const Position& position, const Board& board) {
    for (Chess::Piece* const& piece : board.pieces) {
        if (piece && piece->position == position) {
            return piece->color;
        }
    }
    return nullopt;
}

Chess::Pawn::Pawn(Color color, Position position, bool firstMove, bool enPassantCapturable) {
    this->color = color;
    this->position = position;
    this->firstMove = firstMove;
    this->enPassantCapturable = enPassantCapturable;
    updateEffect = [&]() { enPassantCapturable = false; };
}

Chess::Piece* Chess::Pawn::newCopy() const {
    return new Pawn(color, position, enPassantCapturable);
}

vector<vector<Chess::Move>> Chess::Pawn::getMoves(const Board& board) {
    vector<vector<Move>> moves;

    Position forward;
    Position doubleStep;
    if (color == Color::white) {
        forward = Position(position.x, position.y + 1);
        doubleStep = Position(position.x, position.y + 2);
    }
    else if (color == Color::black) {
        forward = Position(position.x, position.y - 1);
        doubleStep = Position(position.x, position.y - 2);
    }
    else throw exception("nonstandard pawn color not implemented");

    function<void()> pawnMoveEffect;
    if (forward.inBounds && !doubleStep.inBounds) {
        pawnMoveEffect = [&]() { firstMove = false; };
    }
    else {
        pawnMoveEffect = [&]() {
            Piece* promotion;
            for (auto piece : getPieces(board)) {
                if (this->position == piece->position) promotion = piece;
            }
            delete promotion;
            promotion = new Queen(color, position);
        };
    }

    if (!Position::heldBy(forward, board)) {
        moves.push_back({ Move(position, forward, pawnMoveEffect) });
    }
    if (firstMove && !Position::heldBy(doubleStep, board)) {
        moves.push_back({
            Move(position, doubleStep, [&]() {
                firstMove = false;
                enPassantCapturable = true;
            })
        });
    }

    optional<Color> kingSideCapture = Position::heldBy(Position(forward.x + 1, forward.y), board);
    if (kingSideCapture && kingSideCapture.value() != color) {
        moves.push_back({ Move(position, Position(forward.x + 1, forward.y), pawnMoveEffect) });
    }
    optional<Color> queenSideCapture = Position::heldBy(Position(forward.x + 1, forward.y), board);
    if (queenSideCapture && queenSideCapture.value() != color) {
        moves.push_back({ Move(position, Position(forward.x + 1, forward.y), pawnMoveEffect) });
    }

    // En Passant capture
    Piece* kingSideEnPassant = nullptr;
    Piece* queenSideEnPassant = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece->position == Position(position.x + 1, position.y)) kingSideEnPassant = piece;
        if (piece->position == Position(position.x - 1, position.y)) queenSideEnPassant = piece;
    }

    bool kingSideEnPassantCaptureAvailable = kingSideEnPassant &&
        kingSideEnPassant->color != color &&
        kingSideEnPassant->getNotation() == 'P' &&
        static_cast<Pawn*>(kingSideEnPassant)->enPassantCapturable;
    if (kingSideEnPassantCaptureAvailable) {
        moves.push_back({ Move(position, Position(forward.x + 1, forward.y), [&]() { delete kingSideEnPassant; }) });
    }

    bool queenSideEnPassantCaptureAvailable = queenSideEnPassant &&
        queenSideEnPassant->color == color &&
        queenSideEnPassant->getNotation() == 'P' &&
        static_cast<Pawn*>(queenSideEnPassant)->enPassantCapturable;
    if (queenSideEnPassantCaptureAvailable) {
        moves.push_back({ Move(position, Position(forward.x + 1, forward.y), [&]() { delete queenSideEnPassant; }) });
    }

    Piece* shortSidePiece = nullptr;
    Piece* longSidePiece = nullptr;
    for (Piece* const& piece : getPieces(board)) {
        if (piece && Position::sameRow(piece->position, position)) {
            if (piece->position.x > position.x && (!shortSidePiece || shortSidePiece->position.x < piece->position.x)) {
                shortSidePiece = piece;
            }
            if (piece->position.x < position.x && (!longSidePiece || longSidePiece->position.x > piece->position.x)) {
                longSidePiece = piece;
            }
        }
    }
    return moves;
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
    char& x = position.x;
    int& y = position.y;
    return generateMovesWithPattern(
        {
            {x + 1, y + 2},
            {x + 2, y + 1},
            {x + 2, y - 1},
            {x + 1, y - 2},
            {x - 1, y - 2},
            {x - 2, y - 1},
            {x - 2, y + 1},
            {x - 1, y + 2}
        },
        board);
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
    return generateMovesWithPattern(
        {
            [&](Position p) { return Position(p.x + 1, p.y + 1); },
            [&](Position p) { return Position(p.x + 1, p.y - 1); },
            [&](Position p) { return Position(p.x - 1, p.y + 1); },
            [&](Position p) { return Position(p.x - 1, p.y - 1); }
        },
        board);
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
    auto moves = generateMovesWithPattern(
        {
            [&](Position p) { return Position(p.x + 1, p.y); },
            [&](Position p) { return Position(p.x - 1, p.y); },
            [&](Position p) { return Position(p.x, p.y + 1); },
            [&](Position p) { return Position(p.x, p.y - 1); }
        },
        board);

    function<void()> rookMoveEffect = [&]() { canCastle = false; };
    for (vector<Move>& subset : moves) {
        for (Move& move : subset) {
            move.effect = rookMoveEffect;
        }
    }

    return moves;

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
    return generateMovesWithPattern(
        {
            [&](Position p) { return Position(p.x + 1, p.y); },
            [&](Position p) { return Position(p.x - 1, p.y); },
            [&](Position p) { return Position(p.x, p.y + 1); },
            [&](Position p) { return Position(p.x, p.y - 1); },

            [&](Position p) { return Position(p.x + 1, p.y + 1); },
            [&](Position p) { return Position(p.x + 1, p.y - 1); },
            [&](Position p) { return Position(p.x - 1, p.y + 1); },
            [&](Position p) { return Position(p.x - 1, p.y - 1); },
        },
        board);
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
    char& x = position.x;
    int& y = position.y;
    auto moves = generateMovesWithPattern(
        {
            {x + 1, y},
            {x - 1, y},
            {x, y + 1},
            {x, y - 1},

            {x + 1, y + 1},
            {x + 1, y - 1},
            {x - 1, y + 1},
            {x - 1, y - 1}
        },
        board);

    function<void()> kingMoveEffect = [&]() { canCastle = false; }; // TODO: generateMovesWithPattern argument? default value replaced by this
    for (vector<Move>& subset : moves) {
        for (Move& move : subset) {
            move.effect = kingMoveEffect;
        }
    }

    // Castling
    if (canCastle && board.getCurrentTurn() == color) { // causes an infinite loop with enemy kings otherwise
        bool underAttack = false;
        bool shortPathClear = true;
        bool longPathClear = true;
        vector<Position> positionsUnderAttack = positionsUnderAttack(defendingColor);
        for (Position position : positionsUnderAttack) {
            if (position == this->position) underAttack = true;
            if (position == Position(x - 1, y) || position == Position(x - 2, y)) shortPathClear = false;
            if (position == Position(x + 1, y) || position == Position(x + 2, y)) longPathClear = false;
        }

        Piece* shortSidePiece = nullptr;
        Piece* longSidePiece = nullptr;
        for (Piece* const& piece : getPieces(board)) {
            if (piece && Position::sameRow(piece->position, position)) {
                if (piece->position.x > position.x && (!shortSidePiece || shortSidePiece->position.x < piece->position.x)) {
                    shortSidePiece = piece;
                }
                if (piece->position.x < position.x && (!longSidePiece || longSidePiece->position.x > piece->position.x)) {
                    longSidePiece = piece;
                }
            }
        }

        // corner cases for customized boards
        if (!Position::inBounds(Position(x - 1, y)) || !Position::inBounds(Position(x - 2, y))) shortPathClear = false;
        if (!Position::inBounds(Position(x + 1, y)) || !Position::inBounds(Position(x + 2, y))) longPathClear = false;
        if (shortSidePiece->position.x == position.x - 1) {
            optional<Color> heldBy = Position::heldBy(Position(position.x - 2, position.y), board);
            if (heldBy && heldBy.value() == color) {
                shortPathClear = false;
            }
        }
        if (longSidePiece->position.x == position.x + 1) {
            optional<Color> heldBy = Position::heldBy(Position(position.x + 2, position.y), board);
            if (heldBy && heldBy.value() == color) {
                longPathClear = false;
            }
        }

        bool shortSidePieceCanCastle = shortSidePiece &&
            shortSidePiece->color == color &&
            shortSidePiece->getNotation() == 'R' &&
            static_cast<Rook*>(shortSidePiece)->canCastle;
        if (!underAttack && shortPathClear && shortSidePieceCanCastle) {
            Move shortCastle = Move(position, Position(char(position.x - 2), position.y), [&]() {
                canCastle = false;
                shortSidePiece->position.x = x - 1;
                static_cast<Rook*>(shortSidePiece)->canCastle = false;
            });
            moves.push_back({ shortCastle });
        }

        bool longSidePieceCanCastle = longSidePiece &&
            longSidePiece->color == color &&
            longSidePiece->getNotation() == 'R' && 
            static_cast<Rook*>(longSidePiece)->canCastle;
        if (!underAttack && longPathClear && longSidePieceCanCastle) {
            Move longCastle = Move(position, Position(char(position.x + 2), position.y), [&]() {
                canCastle = false;
                longSidePiece->position.x = x + 1;
                static_cast<Rook*>(longSidePiece)->canCastle = false;
            });
            moves.push_back({ longCastle });
        }
    }

    return moves;
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

optional<vector<Chess::Piece::Position>> Chess::Board::positionUnderAttack(const Piece::Color& defendingColor, const Piece::Position& position) const {
    optional<vector<Piece::Position>> result = nullopt;

    for (Piece* const& piece : pieces) {
        if (piece && piece->color != defendingColor) {
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
    return positionUnderAttack(piece.color, piece.position); // TODO: inefficient?
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

bool Chess::Board::makeMove(const int& moveIndex) { // am I not ok with exceptions being thrown?? how would you even recover if the argument was wrong??
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
    if (t != -1) {
        delete pieces[t];
        pieces[t] = nullptr;
    }
    if (move.effect) move.effect.value()();

    ++currentTurn;
    if (currentTurn == turnOrder.end()) {
        currentTurn = turnOrder.begin();
    }
    for (Piece* const& piece : pieces) {
        if (piece->color == *currentTurn && piece->updateEffect) {
            piece->updateEffect.value();
        }
    }

    updateAvailableMoves();

    return true;
}