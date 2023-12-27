// chess_game.h
// by Jake Charles Osborne III
#pragma once

#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <tuple>



namespace Chess {
    class Board;

    struct Move;

    struct Piece
    {
    public:

        virtual Piece* newCopy() const = 0;

        enum class Color { white, black };

        struct Position
        {
            char x; // file
            int y; // rank

            Position() = default;
            Position(char x, int y);
            Position(int x, int y);

            bool operator ==(const Position& p) const;
            bool operator !=(const Position& p) const;

            static bool sameCol(const Position& position1, const Position& position2);
            static bool sameRow(const Position& position1, const Position& position2);
            static bool sameMainDiagonal(const Position& position1, const Position& position2);
            static bool sameAntidiagonal(const Position& position1, const Position& position2);
            static bool inBounds(const Position& position);
            static std::optional<Chess::Piece::Color> heldBy(const Position& position, const Board& board);

        };

        virtual std::vector<std::vector<Move>> getMoves(const Chess::Board&) = 0;

        Color color;
        Position position;
        std::optional<std::function<void()>> updateEffect = std::nullopt;
        virtual char getNotation() const = 0;

    protected:

        static std::vector<Piece*> getPieces(const Board&);

        std::vector<std::vector<Move>> generateMovesWithPattern(
            const std::vector<Position>& pattern,
            const Board& board);
        std::vector<std::vector<Move>> generateMovesWithPattern(
            const std::vector<std::function<Position(Position)>>& pattern,
            const Board& board);
    };

    struct Pawn : public Piece
    {
        Pawn(Color color, Position position, bool firstMove = true, bool enPassantCapturable = false);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Chess::Board&);
        char getNotation() const;

        bool firstMove;
        bool enPassantCapturable;
    };

    struct Knight : public Piece
    {
        Knight(Color color, Position position);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Chess::Board&);
        char getNotation() const;
    };

    struct Bishop : public Piece
    {
        Bishop(Color color, Position position);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Chess::Board&);
        char getNotation() const;
    };

    struct Rook : public Piece
    {
        Rook(Color color, Position position, bool canCastle = true);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Chess::Board&);
        char getNotation() const;

        bool canCastle;
    };

    struct Queen : public Piece
    {
        Queen(Color color, Position position);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Chess::Board&);
        char getNotation() const;
    };

    struct King : public Piece
    {
        King(Color color, Position position, bool canCastle = true);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Chess::Board&);
        char getNotation() const;

        bool canCastle;
    };

    struct Move
    {
        Move(Piece::Position from, Piece::Position to, std::optional<std::function<void()>> effect = std::nullopt);

        Piece::Position from;
        Piece::Position to;
        std::optional<std::function<void()>> effect;
    };

    class Board
    {
    private:

        std::vector<Piece*> pieces;
        char pieceTypeToCapture;
        std::vector<Piece::Color> turnOrder;
        std::vector<Piece::Color>::iterator currentTurn;
        std::vector<Move> availableMoves;

        std::optional<Piece*> getPieceToCapture(Piece::Color) const;
        std::optional<std::vector<Piece::Position>> isPinned(Piece* const&);

        void updateAvailableMoves();

    public:

        Board();
        Board(const Board&);
        ~Board();

        void setDefaultGame();

        std::vector<std::tuple<char, Piece::Color, Piece::Position>> getPieces() const;
        Piece::Color getCurrentTurn() const;
        std::vector<Move> getAvailableMoves() const;
        std::optional<std::vector<Chess::Piece::Position>> positionUnderAttack(const Piece::Color&, const Piece::Position&) const;
        std::optional<std::vector<Chess::Piece::Position>> positionUnderAttack(const Piece&) const;
        bool pieceToCaptureInCheck(const Piece::Color&) const;

        bool makeMove(const int& selectedMove);

        friend struct Piece;
    };
}