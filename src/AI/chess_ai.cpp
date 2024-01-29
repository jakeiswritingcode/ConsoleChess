// chess_ai.cpp
// by Jake Charles Osborne III



#include "chess_ai.h"
#include "../MVC/chess_model.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <random>

using std::string;
using std::vector;
using std::unordered_map;
using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;

using namespace chess::model;



namespace {

    unordered_map<string, double> parseHeuristics(string filename) {
        std::ifstream heuristics_file("filename");
        if (!heuristics_file.is_open()) throw std::runtime_error(filename + " not available");

        unordered_map<string, double> keyValuePairs;

        string line;
        while (std::getline(heuristics_file, line)) {
            size_t pos = line.find('=');

            if (pos != string::npos) {
                string key = line.substr(0, pos);
                double value = std::stod(line.substr(pos + 1));
                keyValuePairs[key] = value;
            }
        }

        heuristics_file.close();

        return keyValuePairs;
    }

	int selectRandomMove(const Board& board) {
		random_device rd; // used to obtain a seed for the random number engine
		mt19937 mt(rd()); // mersenne_twister_engine seeded with rd()
		uniform_int_distribution<> moveDistribution(0, board.getAvailableMoves().size() - 1);

		return moveDistribution(mt);
	}

    double minValue(const Board& board, int depth, double alpha, double beta) {
        if (depth == 0 || board.getWinner()) return evaluate(board);

        double value = DBL_MAX;

        for (const Move& move : board.getAvailableMoves()) {
            Board nextState = simulateMove(board, move);
            value = std::min(value, maxValue(nextState, depth - 1, alpha, beta));

            if (value <= alpha) {
                return value;  // Prune the search tree
            }

            beta = std::min(beta, value);
        }

        return value;
    }

    double maxValue(const Board& board, int depth, double alpha, double beta) {
        if (depth == 0 || board.getWinner()) return evaluate(board);

        double value = DBL_MIN;

        for (const Move& move : board.getAvailableMoves()) {
            Board nextState = simulateMove(board, move);
            value = std::max(value, minValue(nextState, depth - 1, alpha, beta));

            if (value >= beta) {
                return value;  // Prune the search tree
            }

            alpha = std::max(alpha, value);
        }

        return value;
    }

    double evaluate(const Board& board, const unordered_map<string, double>& heuristics) {
        std::optional<Piece::Color> winner = board.getWinner();
        if (winner && winner* == board.getCurrentTurn()) return DBL_MAX;
        if (winner && winner* != board.getCurrentTurn()) return DBL_MIN;

        auto pieces = board.getPieces();


        double boardValue = 0;
        boardValue += pieceValues(pieces, heuristics, aiColor);
        boardValue += doubledPawnValue(pieces, heuristics, aiColor);
        boardValue += castlingAvailableValue(pieces, heuristics, aiColor);
        boardValue += castlingObstructedValue(pieces, heuristics.aiColor);
    
        return boardValue;
    }

    double pieceValues(
        vector<std::tuple<char, Piece::Color, Piece::Position>> pieces,
        const unordered_map<string, double>& heuristics,
        Piece::Color aiColor
    ) {
        double boardValue = 0;
        
        for (const auto& [notation, color, position] : pieces) {
            auto pieceValueItr = heuristics.find(std::string(1, notation));
            if (pieceValueItr == heuristics.end()) throw std::runtime_error("piece of type \'" + std::string(1, notation) + "\' not found");

            if (color == aiColor) boardValue += pieceValueItr->second;
            else boardValue -= pieceValueItr->second;
        }

        return boardValue;
    }

    double doubledPawnValue(
        vector<std::tuple<char, Piece::Color, Piece::Position>> pieces,
        const unordered_map<string, double>& heuristics,
        Piece::Color aiColor
    ) {
        double boardValue = 0;

        auto pieceValueItr = heuristics.find("doubledPawn");
        if (pieceValueItr == heuristics.end()) throw std::runtime_error("doubledPawn value not found");


        for (const auto& [notation, color, position] : pieces) {

            if (color == aiColor) boardValue += pieceValueItr->second;
            else boardValue -= pieceValueItr->second;
        }

        return boardValue;
    }

    double castlingAvailableValue(
        vector<std::tuple<char, Piece::Color, Piece::Position>> pieces,
        const unordered_map<string, double>& heuristics,
        Piece::Color aiColor
    ) {
        double boardValue = 0;



        return boardValue;
    }

    double castlingObstructedValue(
        vector<std::tuple<char, Piece::Color, Piece::Position>> pieces,
        const unordered_map<string, double>& heuristics,
        Piece::Color aiColor
    ) {
        double boardValue = 0;



        return boardValue;
    }




    Board simulateMove(const Board& board, const Move& move) {
        // Implement a function to simulate making a move on the board and return the resulting board state
        // Make a copy of the board, apply the move, and return the modified board
        // Ensure that this function does not modify the original board
        return board;  // Placeholder, replace with your actual logic
    }
}

namespace chess::ai {

    int getMove(const Board& board, Difficulty difficulty) {
        switch (difficulty) {
            case Difficulty::easy:
            return alphaBetaSearch(board, 3);

            case Difficulty::medium:
            return alphaBetaSearch(board, 5);

            case Difficulty::hard:
            return alphaBetaSearch(board, 7);

            default:
            return selectRandomMove(board);
        }
    }

    static int alphaBetaSearch(const Board& board, int depth) {
        double alpha = std::numeric_limits<double>::min();
        double beta = std::numeric_limits<double>::max();
        int bestMove;

        setHeuristicValues();

        auto availableMoves = board.getAvailableMoves();
        for (int i = 0; i < availableMoves.size(); ++i) {
            Board nextState = simulateMove(board, i);
            double score = minValue(nextState, depth - 1, alpha, beta);

            if (score > alpha) {
                alpha = score;
                bestMove = i;
            }
        }

        return bestMove;
    }


}