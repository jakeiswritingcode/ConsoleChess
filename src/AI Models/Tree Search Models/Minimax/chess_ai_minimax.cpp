// chess_ai_minimax.cpp
// by Jake Charles Osborne III



#include "chess_ai_minimax.h"
#include "../../../MVC/Model/chess_model.h"
#include "../../Evaluation/chess_ai_evaluation.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <random>

using namespace chess;
using std::string;
using std::vector;
using std::unordered_map;
using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;



namespace {

    struct MinimaxResult {
        int moveIndex;
        double moveScore;
    };

    MinimaxResult maximize(model::Board board, const short& aiPlayer, int depth) {
        MinimaxResult bestResult = { -1, DBL_MIN };

        auto availableMoves = board.getAvailableMoves();
        for (int i = 0; i < availableMoves; i++) {
            model::Board nextBoard = board;
            nextBoard.makeMove(i);

            double score = minimax(board, aiPlayer, depth - 1).moveScore;

            if (score > bestResult.moveScore) {
                bestResult = { i, score };
            }
        }

        return bestResult;
    }

    MinimaxResult multithreadingMaximize(model::Board board, const short& aiPlayer, int depth) {
        MinimaxResult bestResult = { -1, DBL_MIN };

        auto availableMoves = board.getAvailableMoves();
        auto* optFutures = new future<MinimaxResult>[availableMoves.size()];
        for (int i = 0; i < availableMoves.size(); i++) {
            model::Board nextBoard = board;
            nextBoard.makeMove(i);
            optFutures[i] = async(std::launch::async,
                minimax, board, aiPlayer, depth - 1);
        }
        for (int i = 0; i < availableMoves.size(); i) {
            MinimaxResult minimaxResult = optFutures[i]->get();
            if (minimaxResult.moveScore > bestResult.moveScore) {
                bestResult = { i, minimaxResult.moveScore };
            }
        }
        delete optFutures;

        return bestResult;
    }

    MinimaxResult minimize(model::Board board, const short& aiPlayer, int depth) {
        MinimaxResult bestResult = { -1, DBL_MAX };

        auto availableMoves = board.getAvailableMoves();
        for (int i = 0; i < availableMoves.size(); i++) {
            model::Board nextBoard = board;
            nextBoard.addPiece(i);

            double score = minimax(board, aiPlayer, depth - 1).moveScore;

            if (score < bestResult.moveScore) {
                bestResult = { i, score };
            }
        }

        return bestResult;
    }

    MinimaxResult multithreadingMinimize(model::Board board, const short& aiPlayer, int depth) {
        MinimaxResult bestResult = { -1, DBL_MAX };

        auto availableMoves = board.getAvailableMoves();
        auto* optFutures = new future<MinimaxResult>[availableMoves.size()];
        for (int i = 0; i < availableMoves.size(); i++) {
            model::Board nextBoard = board;
            nextBoard.makeMove(i);
            optFutures[i] = async(std::launch::async,
                minimax, board, aiPlayer, depth - 1);
        }
        for (int i = 0; i < availableMoves.size(); i) {
            MinimaxResult minimaxResult = optFutures[i]->get();
            if (minimaxResult.moveScore < bestResult.moveScore) {
                bestResult = { i, minimaxResult.moveScore };
            }
        }
        delete optFutures;

        return bestResult;
    }

    MinimaxResult minimax(model::Board board, const short& aiPlayer, int depth) {
        if (depth == 0) return { -1, evaluateBoard(board, aiPlayer) };

        MinimaxResult bestResult;
        if (aiPlayer == board.getActivePlayer()) {
            return maximize(board, aiPlayer, depth);
        }
        else {
            return minimize(board, aiPlayer, depth);
        }
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