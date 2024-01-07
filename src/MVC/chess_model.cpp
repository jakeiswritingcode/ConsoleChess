// chess_model.cpp
// by Jake Charles Osborne III



#include "chess_model.h"
#include <vector>
#include <unordered_set>
#include <functional>
#include <optional>
#include <tuple>
#include <future>
#include <exception>

using std::vector;
using std::unordered_set;
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
    else throw std::exception("nonstandard pawn color not implemented");

    function<void()> pawnMoveEffect = [&]() { firstMove = false; };
    if (Position::inBounds(forward) && !Position::inBounds(doubleStep)) {
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

    function<void()> kingMoveEffect = [&]() { canCastle = false; };
    for (vector<Move>& subset : moves) {
        for (Move& move : subset) {
            move.effect = kingMoveEffect;
        }
    }

    // Castling
    if (canCastle) {
        bool underAttack = false;
        bool shortPathClear = true;
        bool longPathClear = true;
        unordered_set<Position> positionsUnderAttack = board.getPositionsUnderAttack();
        if (positionsUnderAttack.find(this->position) != positionsUnderAttack.end()) underAttack = true;
        if (positionsUnderAttack.find(Position(x - 1, y)) != positionsUnderAttack.end() ||
            positionsUnderAttack.find(Position(x - 2, y)) != positionsUnderAttack.end()) shortPathClear = false;
        if (positionsUnderAttack.find(Position(x + 1, y)) != positionsUnderAttack.end() ||
            positionsUnderAttack.find(Position(x + 2, y)) != positionsUnderAttack.end()) longPathClear = false;

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

void Chess::Board::advanceTurn(vector<Chess::Piece::Color>::const_iterator& i) const {
    ++i;
    if (i == turnOrder.end()) {
        i = turnOrder.begin();
    }
}

optional<Chess::Piece::Position> Chess::Board::getPieceToCapturePosition(Piece::Color color) const {
    for (auto piece : pieces) {
        if (piece && piece->color == color && piece->getNotation() == pieceTypeToCapture) {
            return piece->position;
        }
    }
    return nullopt;
}

optional<unordered_set<Chess::Piece::Position>> Chess::Board::getPositionsBlockingCheck() const {
    optional<unordered_set<Piece::Position>> positionsBlockingCheck = nullopt;

    optional<Piece::Position> pieceToCapturePosition = getPieceToCapturePosition(getCurrentTurn());
    if (!pieceToCapturePosition) return nullopt;
    for (auto piece : pieces) {
        if (piece && piece->color != getCurrentTurn()) {
            auto movePatterns = piece->getMoves(*this);
            for (auto movePattern : movePatterns) {
                for (auto move : movePattern) {
                    if (move.to == pieceToCapturePosition) {
                        if (!positionsBlockingCheck) {
                            for (auto move : movePattern) positionsBlockingCheck.emplace(move.to);
                        }
                        else {
                            unordered_set<Piece::Position> newPositionsBlockingCheck;
                            for (auto move : movePattern) {
                                if (positionsBlockingCheck->find(move.to) != positionsBlockingCheck->end()) {
                                    newPositionsBlockingCheck.emplace(move.to);
                                }
                            }
                            positionsBlockingCheck = newPositionsBlockingCheck;
                        }

                        positionsBlockingCheck.emplace(move.to);
                        // overlap movePattern with existing positionsBlockingCheck



                    }

                }
            }
        }
    }

    return positionsBlockingCheck;
}

void Chess::Board::updateAvailableMoves() {
    availableMoves.clear();

    if (!winByCheckmate) { // checks and pins disabled for boards with an atypical player count, turn order, move generation etc.
        vector<future<vector<vector<Move>>>> futures;
        for (auto piece : pieces) {
            if (piece && piece->color == getCurrentTurn()) {
                futures.push_back(std::async(std::launch::async, [this, &piece]() {
                    return piece->getMoves(*this); }));
            }
        }
        for (auto& future : futures) {
            auto movePatterns = future.get();
            for (auto movePattern : movePatterns) {
                availableMoves.insert(availableMoves.end(), movePattern.begin(), movePattern.end());
            }
        }
        // TODO: determine if there is a winner here?
        return;
    }

    vector<future<vector<Move>>> futures;
    optional<unordered_set<Chess::Piece::Position>> positionsBlockingCheck = getPositionsBlockingCheck();
    for (auto piece : pieces) {
        if (piece && piece->color == getCurrentTurn()) {
            futures.push_back(std::async(std::launch::async, [this, &piece, &positionsBlockingCheck]() {
                return this->getValidMoves(piece, positionsBlockingCheck); }));
        }
    }
    for (auto& future : futures) {
        auto validMoves = future.get();
        availableMoves.insert(availableMoves.end(), validMoves.begin(), validMoves.end());
    }

    // TODO: determine if there is a winner here?
}

vector<Chess::Move> Chess::Board::getValidMoves(Piece*& piece, const optional<unordered_set<Chess::Piece::Position>>& positionsBlockingCheck) const {
    vector<Move> moves;
    for (auto patterns : piece->getMoves(*this)) moves.insert(moves.end(), patterns.begin(), patterns.end());

    auto pieceToCapturePosition = getPieceToCapturePosition(getCurrentTurn());
    if (pieceToCapturePosition && *pieceToCapturePosition == piece->position) {
        auto enemyAttacks = getPositionsUnderAttack();
        vector<Move> safeMoves;
        for (auto move : moves) {
            if (enemyAttacks.find(move.to) == enemyAttacks.end()) {
                safeMoves.push_back(move);
            }
        }
        return safeMoves;
    }
    else {
        // curate moves if in check
        if (positionsBlockingCheck) {
            vector<Move> movesBlockingCheck;
            for (auto move : moves) {
                if (positionsBlockingCheck->find(move.to) != positionsBlockingCheck->end()) movesBlockingCheck.push_back(move);
            }
            moves = movesBlockingCheck;
        }

        // curate moves if pinned
        Board boardWithoutPiece(*this, piece);
        optional<unordered_set<Piece::Position>> positionsKeepingPin = nullopt;
        positionsKeepingPin = boardWithoutPiece.getPositionsBlockingCheck();
        if (positionsKeepingPin) {
            vector<Move> movesKeepingPin;
            for (auto move : moves) {
                if (positionsKeepingPin->find(move.to) != positionsKeepingPin->end()) movesKeepingPin.push_back(move);
            }
            moves = movesKeepingPin;
        }

        return moves;
    }
}

Chess::Board::Board() {
    setDefaultGame();
}

Chess::Board::Board(const Board& board) {
    for (Piece* piece : board.pieces) {
        if (piece) pieces.push_back(piece->newCopy());
        else pieces.push_back(nullptr);
    }  

    pieceTypeToCapture = board.pieceTypeToCapture;

    turnOrder = board.turnOrder;

    ptrdiff_t turnCount = std::distance(board.turnOrder.begin(), board.currentTurn);
    currentTurn = std::next(turnOrder.begin(), turnCount);

    winByCheckmate = board.winByCheckmate;

    updateAvailableMoves();
}

Chess::Board::Board(const Board& board, const Piece*& removedPiece) {
    for (auto piece : board.pieces) {
        if (piece && piece != removedPiece) pieces.push_back(piece->newCopy());
        else pieces.push_back(nullptr);
    }

    pieceTypeToCapture = board.pieceTypeToCapture;

    turnOrder = board.turnOrder;

    ptrdiff_t turnCount = std::distance(board.turnOrder.begin(), board.currentTurn);
    currentTurn = std::next(turnOrder.begin(), turnCount);

    winByCheckmate = board.winByCheckmate;

    updateAvailableMoves();
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

    pieceTypeToCapture = 'K'; // TODO: unordered_map<Piece::Color, char>

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

    winByCheckmate = true;

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

void Chess::Board::makeMove(const int& moveIndex) {
    Move move = availableMoves[moveIndex];

    int f = -1; // from piece index
    int t = -1; // to piece index
    for (int i = 0; i < pieces.size(); ++i) {
        if (pieces[i]) {
            if (pieces[i]->position == move.from) f = i;
            if (pieces[i]->position == move.to) t = i;
        }
    }

    pieces[f]->position = move.to;
    if (t != -1) {
        delete pieces[t];
        pieces[t] = nullptr;
    }
    if (move.effect) move.effect.value()();

    advanceTurn(currentTurn);
    for (Piece* const& piece : pieces) {
        if (piece->color == *currentTurn && piece->updateEffect) {
            piece->updateEffect.value();
        }
    }

    updateAvailableMoves();
}