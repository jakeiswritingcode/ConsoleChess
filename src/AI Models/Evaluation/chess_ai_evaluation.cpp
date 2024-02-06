// chess_ai_evaluation.h
// by Jake Charles Osborne III



#include "chess_ai_evaluation.h"
#include "../../MVC/Model/chess_model.h"
#include <string>
#include <vecor>
#include <optional>
#include <unordered_map>

using namespace chess;
using std::string;
using std::vector;
using std::optional;
using std::unordered_map;


namespace {

    const unordered_map<string, double> HEURISTICS = parseHeuristics("heuristics.dat");

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

    double pieceValues(
        vector<std::tuple<char, model::Piece::Color, model::Piece::Position>> pieces,
        Piece::Color aiColor
    ) {
        double boardValue = 0;

        for (const auto& [notation, color, position] : pieces) {
            auto pieceValueItr = HEURISTICS.find(std::string(1, notation));
            if (pieceValueItr == HEURISTICS.end()) throw std::runtime_error("piece of type \'" + std::string(1, notation) + "\' not found");

            if (color == aiColor) boardValue += pieceValueItr->second;
            else boardValue -= pieceValueItr->second;
        }

        return boardValue;
    }

    double doubledPawnValue(
        vector<std::tuple<char, model::Piece::Color, model::Piece::Position>> pieces,
        Piece::Color aiColor
    ) {
        double boardValue = 0;

        auto pieceValueItr = HEURISTICS.find("doubledPawn");
        if (pieceValueItr == HEURISTICS.end()) throw std::runtime_error("doubledPawn value not found");


        for (const auto& [notation, color, position] : pieces) {

            if (color == aiColor) boardValue += pieceValueItr->second;
            else boardValue -= pieceValueItr->second;
        }

        return boardValue;
    }

    double castlingAvailableValue(
        vector<std::tuple<char, model::Piece::Color, model::Piece::Position>> pieces,
        Piece::Color aiColor
    ) {
        double boardValue = 0;

        // TODO: evaluate board

        return boardValue;
    }

}

namespace chess::ai::evaluation {

	double evaluate(const model::Board& board, const model::Piece::Color& maximizingPlayer) {
        optional<model::Piece::Color> winner = board.getWinner();
        if (winner && *winner == maximizingPlayer) return DBL_MAX;
        if (winner && *winner != maximizingPlayer) return DBL_MIN;

        auto pieces = board.getPieces();

        double boardValue = 0;
        boardValue += pieceValues(pieces, maximizingPlayer);
        boardValue += doubledPawnValue(pieces, maximizingPlayer);
        boardValue += castlingAvailableValue(pieces, maximizingPlayer);

        return boardValue;
	}

}