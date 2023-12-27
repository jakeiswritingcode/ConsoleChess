// chess_control.cpp
// by Jake Charles Osborne III



#include "chess_control.h"
#include "chess_model.h"
#include "chess_view.h"
#include "chess_ai.h"

#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <optional>



using std::cin;
using std::string;
using std::vector;
using std::optional;
using std::nullopt;
using std::exception;
using namespace Chess;



namespace {
	bool parseInput(string& input, string& message, optional<Piece::Position>& selectedPiece, optional<Piece::Color>& ai, Board& board) {
		for (auto& inputChar : input) {
			inputChar = tolower(inputChar);
		}

		if (input == "h" || input == "help") {
			printHelpMenu();
			return false;
		}

		if (input == "c" || input == "cancel") {
			if (selectedPiece != nullopt) {
				selectedPiece = nullopt;
				message = "Piece deselected.";
				return true;
			}
			else {
				message = "No piece selected yet.";
				return false;
			}
		}

		auto availableMoves = board.getAvailableMoves();
		if (input == "a" || input == "available") {
			message = "Moves are available for pieces on the following positions: ";

			vector<Piece::Position> uniqueMovablePieces;
			for (const Move& availableMove : availableMoves) {
				bool unique = true;
				for (const Piece::Position& uniqueMovablePiece : uniqueMovablePieces) {
					if (availableMove.from == uniqueMovablePiece) {
						unique = false;
					}
				}
				if (unique) {
					uniqueMovablePieces.push_back(availableMove.from);
				}
			}

			for (const auto& uniqueMovablePiece : uniqueMovablePieces) {
				message += " ";
				message += uniqueMovablePiece.x;
				message += '0' + uniqueMovablePiece.y;
			}

			return false;
		}

		if (input == "ai-white") {
			board.setDefaultGame();
			selectedPiece = nullopt;
			ai = Piece::Color::white;
			message = "AI enabled as white player.";
			return true;
		}

		if (input == "ai-black") {
			board.setDefaultGame();
			selectedPiece = nullopt;
			message = "AI enabled as black player.";
			ai = Piece::Color::black;
			return true;
		}

		if (input == "reset") {
			board.setDefaultGame();
			selectedPiece = nullopt;
			message = "Game reset.";
			ai = nullopt;
			return true;
		}

		if (input.size() != 2 || !isalpha(input[0]) || !isdigit(input[1])) {
			message = "Provide a letter and a number for your move.";
			return false;
		}

		char inputFile = toupper(input[0]);
		int inputRank = input[1] - '0';
		Piece::Position inputPosition(inputFile, inputRank);
		if (inputPosition.x > 'H' || inputPosition.x < 'A' || inputPosition.y > 8 || inputPosition.y < 1) {
			message = "Move must be in bounds.";
			return false;
		}


		if (selectedPiece == nullopt) {
			for (int moveIndex = 0; moveIndex < availableMoves.size() && selectedPiece == nullopt; ++moveIndex) {
				if (availableMoves[moveIndex].from == inputPosition) {
					selectedPiece = inputPosition;
				}
			}
			if (selectedPiece == nullopt) {
				message = "Invalid selection.";
				return false;
			}
			else {
				message = "Piece at ";
				message += selectedPiece->x;
				message += '0' + selectedPiece->y;
				message += " selected.";
				return true;
			}
		}

		vector<Move> moves = board.getAvailableMoves();
		int moveIndex = -1;
		for (int i = 0; i < moves.size(); ++i) {
			if (*selectedPiece == moves[i].from && inputPosition == moves[i].to) moveIndex = i;
		}

		bool moveSuccessful = false;
		if (moveIndex != -1) {
			moveSuccessful = board.makeMove(moveIndex);
		}
		if (moveSuccessful) {
			selectedPiece = nullopt;
			message = "Move complete.";
			return true;
		}

		message = "Please select a highlighted move.";
		return false;
	}

	vector<Piece::Position> getSelectedPieceMoves(const optional<Piece::Position>& selectedPiece, const vector<Move>& availableMoves) {
		vector<Piece::Position> result;
		if (selectedPiece) {
			for (const Move& move : availableMoves) {
				if (move.from == selectedPiece) {
					result.push_back(move.to);
				}
			}
		}
		return result;
	}
}

namespace Chess {
	void play() {
		string input = "";
		while (std::cin && input != "exit") {

			Board board;

			optional<Piece::Position> selectedPiece = nullopt;
			optional<Piece::Color> ai = nullopt;

			vector<Piece::Position> selectedPieceMoves = { };
			updateBoardString(board);

			string message = "\"You may begin. Enter 'help' for a list of commands.\"";
			while (cin && !board.getAvailableMoves().empty()) {
				if (ai && *ai == board.getCurrentTurn()) {
					ChessAI::makeMove(board);
					message = "AI move complete.";
				}
				else {
					bool inputValid = false;
					do {
						printHeader();
						printCurrentTurn(board);
						printBoardString();
						printMessage(message);
						input = promptUser();

						if (input == "exit") return;
						inputValid = parseInput(input, message, selectedPiece, ai, board);
					} while (std::cin && !inputValid);
				}

				selectedPieceMoves = getSelectedPieceMoves(selectedPiece, board.getAvailableMoves());
				updateBoardString(board)(board, selectedPieceMoves);
			}

			printHeader();
			if (board.pieceToCaptureInCheck(board.getCurrentTurn())) {
				if (board.getCurrentTurn() == Piece::Color::white) printMessage("Black Wins!");
				else printMessage("White Wins!");
			}
			else {
				printMessage("Stalemate!");
			}
			printBoardString();
			printMessage("\"Thanks for playing!\"");
			printMessage("Press enter to start a new game.");
			promptUser()
		}
	}
}