// chess_game.h
// by Jake Charles Osborne III
#pragma once



#include <vector>
#include <string>
#include <tuple>



namespace Chess {
    struct Piece
    {
        virtual Piece* newCopy() const = 0;

        enum Color { white, black };

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
        };

        struct Proximity
        {
            Piece* n = nullptr;
            Piece* e = nullptr;
            Piece* s = nullptr;
            Piece* w = nullptr;
            Piece* ne = nullptr;
            Piece* se = nullptr;
            Piece* sw = nullptr;
            Piece* nw = nullptr;

            Proximity(const Piece::Position& position, const std::vector<Piece*>& pieces);
        };

        enum Pin { none, vertical, horizontal, mainDiagonal, antidiagonal };
        bool isPinned(const Piece& side1, const Piece& side2) const;
        Pin getPinType(const Proximity& proximity) const;
        static bool conformsToPin(const Piece::Position& nextMove, const Piece::Position& currentPosition, const Piece::Pin& pin);

        static void addMovesFromPieceToLimit(std::vector<Piece::Position>& moves, const Piece& piece, Piece* const& limit, const Piece::Pin& pin, void modify(Piece::Position&));

        Color color;
        Position position;
        virtual char getNotation() const = 0;
        virtual std::vector<Position> getMoves(const std::vector<Piece*>& pieces) const = 0;
    };

    struct Pawn : public Piece
    {
        Pawn(Color color, Position position, bool enPassantCapturable = false);
        Piece* newCopy() const;
        bool enPassantCapturable;
        char getNotation() const;
        std::vector<Position> getMoves(const std::vector<Piece*>& pieces) const;
    };

    struct Rook : public Piece
    {
        Rook(Color color, Position position, bool canCastle = true);
        Piece* newCopy() const;
        bool canCastle = true;
        char getNotation() const;
        std::vector<Position> getMoves(const std::vector<Piece*>& pieces) const;
    };

    struct Knight : public Piece
    {
        Knight(Color color, Position position);
        Piece* newCopy() const;
        char getNotation() const;
        std::vector<Position> getMoves(const std::vector<Piece*>& pieces) const;
    };

    struct Bishop : public Piece
    {
        Bishop(Color color, Position position);
        Piece* newCopy() const;
        char getNotation() const;
        std::vector<Position> getMoves(const std::vector<Piece*>& pieces) const;
    };

    struct Queen : public Piece
    {
        Queen(Color color, Position position);
        Piece* newCopy() const;
        char getNotation() const;
        std::vector<Position> getMoves(const std::vector<Piece*>& pieces) const;
    };

    struct King : public Piece
    {
        King(Color color, Position position, bool canCastle = true);
        Piece* newCopy() const;
        bool canCastle = true;
        char getNotation() const;

        static std::vector<Piece*> enemiesAttackingPosition(const Color& allyColor, const Position& position, const std::vector<Piece*>& pieces);
        std::vector<Piece*> inCheck(const std::vector<Piece*>& pieces) const;
        std::vector<Position> getSavingMoves(Piece* checking);
        std::vector<Position> getMoves(const std::vector<Piece*>& pieces) const;
    };

    class Board
    {
    private:

        std::vector<Piece*> pieces;
        std::vector<Piece::Color> turnOrder;
        std::vector<Piece::Color>::iterator currentTurn;
        std::vector<std::pair<Piece::Position, Piece::Position>> availableMoves;

        void updateAvailableMoves();

    public:

        Board();
        Board(const Board& b);
        ~Board();

        std::vector<std::tuple<char, Piece::Color, Piece::Position>> getPieces() const;
        Piece::Color getCurrentTurn() const;
        std::vector<std::pair<Piece::Position, Piece::Position>> getAvailableMoves() const;

        void setDefaultGame();

        bool makeMove(const Piece::Position& from, const Piece::Position& to);
        bool makeMove(const std::pair<Piece::Position, Piece::Position>& selectedMove);
    };
}