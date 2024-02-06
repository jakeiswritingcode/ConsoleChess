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

    MinimaxResult minimize(const model::Board& board, const model::Piece::Color& maximizingPlayer, int depth) {
        MinimaxResult bestResult = { -1, DBL_MAX };

        auto availableMoves = board.getAvailableMoves();
        for (int i = 0; i < availableMoves.size(); i++) {
            model::Board nextBoard = board;
            nextBoard.addPiece(i);

            double score = minimax(board, maximizingPlayer, depth - 1).moveScore;

            if (score < bestResult.moveScore) {
                bestResult = { i, score };
            }
        }

        return bestResult;
    }

    MinimaxResult maximize(const model::Board& board, const model::Piece::Color& maximizingPlayer, int depth) {
        MinimaxResult bestResult = { -1, DBL_MIN };

        auto availableMoves = board.getAvailableMoves();
        for (int i = 0; i < availableMoves; i++) {
            model::Board nextBoard = board;
            nextBoard.makeMove(i);

            double score = minimax(board, maximizingPlayer, depth - 1).moveScore;

            if (score > bestResult.moveScore) {
                bestResult = { i, score };
            }
        }

        return bestResult;
    }

    MinimaxResult multithreadingMinimize(const model::Board& board, const model::Piece::Color& maximizingPlayer, int depth) {
        MinimaxResult bestResult = { -1, DBL_MAX };

        auto availableMoves = board.getAvailableMoves();
        auto* optFutures = new future<MinimaxResult>[availableMoves.size()];
        for (int i = 0; i < availableMoves.size(); i++) {
            model::Board nextBoard = board;
            nextBoard.makeMove(i);
            optFutures[i] = async(std::launch::async,
                minimax, board, maximizingPlayer, depth - 1);
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

    MinimaxResult multithreadingMaximize(const model::Board board&, const model::Piece::Color& maximizingPlayer, int depth) {
        MinimaxResult bestResult = { -1, DBL_MIN };

        auto availableMoves = board.getAvailableMoves();
        auto* optFutures = new future<MinimaxResult>[availableMoves.size()];
        for (int i = 0; i < availableMoves.size(); i++) {
            model::Board nextBoard = board;
            nextBoard.makeMove(i);
            optFutures[i] = async(std::launch::async,
                minimax, board, maximizingPlayer, depth - 1);
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

}

namespace chess::ai {

    MinimaxResult minimax(const model::Board& board, const model::Piece::Color& maximizingPlayer, const int& depth) {
        if (depth == 0) return { -1, evaluateBoard(board, maximizingPlayer) };

        MinimaxResult bestResult;
        if (maximizingPlayer == board.getActivePlayer()) {
            return maximize(board, maximizingPlayer, depth);
        }
        else {
            return minimize(board, maximizingPlayer, depth);
        }

        return bestMove;
    }

    MinimaxResult multithreadingMinimax(const model::Board& board, const model::Piece::Color& maximizingPlayer, const int& depth) {
        if (depth == 0) return { -1, evaluateBoard(board, maximizingPlayer) };

        MinimaxResult bestResult;
        if (maximizingPlayer == board.getActivePlayer()) {
            return multithreadingMaximize(board, maximizingPlayer, depth);
        }
        else {
            return multithreadingMinimize(board, maximizingPlayer, depth);
        }

        return bestMove;
    }

}