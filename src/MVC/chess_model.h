// chess_game.h
// by Jake Charles Osborne III
#pragma once

#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <tuple>
#include <unordered_set>



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

            static bool sameCol(const Position&, const Position&);
            static bool sameRow(const Position&, const Position&);
            static bool sameMainDiagonal(const Position&, const Position&);
            static bool sameAntidiagonal(const Position&, const Position&);
            static bool inBounds(const Position&);
            static std::optional<Piece::Color> heldBy(const Position&, const Board&);

        };

        virtual std::vector<std::vector<Move>> getMoves(const Board&) = 0;

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
        Pawn(Color, Position, bool firstMove = true, bool enPassantCapturable = false);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Board&);
        char getNotation() const;

        bool firstMove;
        bool enPassantCapturable;
    };

    struct Knight : public Piece
    {
        Knight(Color, Position);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Board&);
        char getNotation() const;
    };

    struct Bishop : public Piece
    {
        Bishop(Color, Position);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Board&);
        char getNotation() const;
    };

    struct Rook : public Piece
    {
        Rook(Color, Position, bool canCastle = true);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Board&);
        char getNotation() const;

        bool canCastle;
    };

    struct Queen : public Piece
    {
        Queen(Color, Position);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Board&);
        char getNotation() const;
    };

    struct King : public Piece
    {
        King(Color, Position, bool canCastle = true);
        Piece* newCopy() const;
        std::vector<std::vector<Move>> getMoves(const Board&);
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
        std::vector<Piece::Color>::const_iterator currentTurn;
        std::vector<Move> availableMoves;
        bool winByCheckmate;

        void advanceTurn(std::vector<Piece::Color>::const_iterator&) const;
        std::optional<Piece::Position> getPieceToCapturePosition(Piece::Color color) const;
        std::optional<std::unordered_set<Piece::Position>> getPositionsBlockingCheck() const;
        std::optional<Piece*> getPieceToCapture(Piece::Color) const;
        std::vector<Move> getValidMoves(Piece*&, const std::optional<std::unordered_set<Piece::Position>>&) const;

        void updateAvailableMoves();

    public:

        Board();
        Board(const Board&);
        Board(const Board&, const Piece*&);
        ~Board();

        void setDefaultGame();

        std::vector<std::tuple<char, Piece::Color, Piece::Position>> getPieces() const;
        Piece::Color getCurrentTurn() const;
        std::vector<Move> getAvailableMoves() const;
        std::unordered_set<Piece::Position>getPositionsUnderAttack() const;
        bool pieceToCaptureInCheck(const Piece::Color&) const;

        bool makeMove(const int& selectedMove);

        friend struct Piece;

    };
}