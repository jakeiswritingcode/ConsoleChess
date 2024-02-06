// chess_control.cpp
// by Jake Charles Osborne III



#include "chess_control.h"
#include "chess_model.h"
#include "chess_view.h"
#include "../AI/chess_ai.h"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <tuple>
#include <optional>

using namespace chess;

using std::cin;
using std::string;
using std::vector;
using std::unordered_set;
using std::optional;
using std::nullopt;



namespace {

	enum UserAction {
		indicatePosition,
		algebraicNotation,
		deselectPiece,
		availableMoves,
		help,
		reset,
		aiWhite,
		aiBlack,
		exitGame,
		invalidAction
	};

	void processUserAction(
		UserAction& userAction,
		string& input,
		model::Board& board,
		optional<model::Piece::Position>& selectedPiece,
		optional<model::Piece::Color>& ai,
		string& message
	) {
		switch (userAction) {
		case UserAction::indicatePosition:
			model::Piece::Position indicatedPosition = { input[0] - '0', input[1] - '0' };
			if (selectedPiece) {
				optional<int> optIndex = nullopt;
				auto availableMoves = board.getAvailableMoves();
				for (int i = 0; i < availableMoves.size(); ++i) {
					if (availableMoves[i].from == selectedPiece &&
						availableMoves[i].to == indicatedPosition) {
						optIndex = i;
					}
				}
				if (optIndex) {
					board.makeMove(*optIndex);
					selectedPiece = nullopt;
					message = "Move complete.";
					view::updateBoardString(board);
				}
				else {
					message = "Select an available move.";
				}
			}
			else {
				for (const auto& move : board.getAvailableMoves()) {
					if (move.from == indicatedPosition) {
						selectedPiece = move.from;
						message = "Piece at " + string(1, selectedPiece->x) + string(1, '0' + selectedPiece->y) + " selected.";
						view::updateBoardString(board, getSelectedPieceMoves(selectedPiece, board.getAvailableMoves()));
					}
				}
				if (!selectedPiece) {
					optional<int> optIndex = getAlgebraicNotationMove(input, board);
					if (optIndex) {
						board.makeMove(*optIndex);
						selectedPiece = nullopt;
						message = "Move complete.";
						view::updateBoardString(board);
					}
					else {
						message = "Invalid prompt.";
					}
				}
			}
			break;

		case UserAction::algebraicNotation: {
			optional<int> optIndex = nullopt;
			if (input.size() == 3 && std::isalpha(input[0]) && std::isalpha(input[1]) && isdigit(input[2])) {
				optIndex = getAlgebraicNotationPrefixMove(input, board);
			}
			else if (input.size() == 2 && std::isalpha(input[0]) && input[1] == 'x') {
				optIndex = getAlgebraicNotationCaptureMove(input, board);
			}
			else if (input.size() == 3 && std::isalpha(input[0]) && input[1] == 'x' && isalpha(input[2])) {
				optIndex = getAlgebraicNotationPrefixCaptureMove(input, board);
			}
			else if (input == "00" || input == "0-0" || input == "oo" || input == "o-o") {
				optIndex = getKingsideCastlingMove(board);
			}
			else if (input == "000" || input == "0-0-0" || input == "ooo" || input == "o-o-o") {
				optIndex = getQueensideCastlingMove(board);
			}

			if (optIndex) {
				board.makeMove(*optIndex);
				selectedPiece = nullopt;
				message = "Move complete.";
				view::updateBoardString(board);
			}
			else {
				message = "Invalid prompt.";
			}
			break;
		}
		case UserAction::deselectPiece:
			if (selectedPiece != nullopt) {
				selectedPiece = nullopt;
				message = "Piece deselected.";
				view::updateBoardString(board);
			}
			else {
				message = "No piece selected yet.";
			}
			break;

		case UserAction::availableMoves: {
			message = "Moves are available for pieces on the following positions: ";
			unordered_set<model::Piece::Position> movablePieces;
			for (const model::Move& availableMove : board.getAvailableMoves()) {
				movablePieces.insert(availableMove.from);
			}
			for (const auto& movablePiece : movablePieces) {
				message += " " + string(1, movablePiece.x) + string(1, '0' + movablePiece.y);
			}
			break;
		}
		case UserAction::help:
			view::printHelpMenu();
			break;

		case UserAction::reset:
			board.setDefaultGame();
			selectedPiece = nullopt;
			ai = nullopt;
			message = "Game reset.";
			view::updateBoardString(board);
			break;

		case UserAction::aiWhite:
			board.setDefaultGame();
			selectedPiece = nullopt;
			ai = model::Piece::Color::white;
			message = "Game reset with AI enabled as white.";
			view::updateBoardString(board);
			break;

		case UserAction::aiBlack:
			board.setDefaultGame();
			selectedPiece = nullopt;
			ai = model::Piece::Color::black;
			message = "Game reset with AI enabled as black.";
			view::updateBoardString(board);
			break;
		}
	}

	optional<int> getAlgebraicNotationMove(const string& input, const model::Board& board) {
		if (input.size() != 2) throw std::logic_error("invalid input size");

		optional<int> optMoveIndex = nullopt;

		auto availableMoves = board.getAvailableMoves();
		for (int i = 0; i < availableMoves.size(); ++i) {
			if (availableMoves[i].to.x == input[0] &&
				'0' + availableMoves[i].to.y == input[1])
			{
				if (!optMoveIndex) optMoveIndex = i;
				else return nullopt;
			}
		}

		return optMoveIndex;
	}

	optional<int> getAlgebraicNotationPrefixMove(const string& input, const model::Board& board) {
		if (input.size() != 3) throw std::logic_error("invalid input size");

		unordered_set<model::Piece::Position> possibleFroms;
		for (const auto& [notation, color, position] : board.getPieces()) {
			if (tolower(notation) == input[0] && color == board.getCurrentTurn()) possibleFroms.emplace(position);
		}

		optional<int> optMoveIndex = nullopt;

		auto availableMoves = board.getAvailableMoves();
		for (int i = 0; i < availableMoves.size(); ++i) {
			if (possibleFroms.find(availableMoves[i].from) != possibleFroms.end() &&
				availableMoves[i].to.x == input[1] &&
				'0' + availableMoves[i].to.y == input[2])
			{
				if (!optMoveIndex) optMoveIndex = i;
				else return nullopt;
			}
		}

		return optMoveIndex;
	}

	optional<int> getAlgebraicNotationCaptureMove(const string& input, const model::Board& board) {
		if (input.size() != 2) throw std::logic_error("invalid input size");

		unordered_set<model::Piece::Position> possibleFroms;
		unordered_set<model::Piece::Position> possibleTos;
		for (const auto& [notation, color, position] : board.getPieces()) {
			if (tolower(notation) == input[0] && color == board.getCurrentTurn()) possibleFroms.emplace(position);
			if (color != board.getCurrentTurn()) possibleTos.emplace(position);
		}

		optional<int> optMoveIndex = nullopt;
		auto availableMoves = board.getAvailableMoves();
		for (int i = 0; i < availableMoves.size(); ++i) {
			if (possibleFroms.find(availableMoves[i].from) != possibleFroms.end() &&
				possibleTos.find(availableMoves[i].to) != possibleTos.end())
			{
				if (!optMoveIndex) optMoveIndex = i;
				else return nullopt;
			}
		}

		return optMoveIndex;
	}

	optional<int> getAlgebraicNotationPrefixCaptureMove(const string& input, const model::Board& board) {
		if (input.size() != 3) throw std::logic_error("invalid input size");

		unordered_set<model::Piece::Position> possibleFroms;
		unordered_set<model::Piece::Position> possibleTos;
		for (const auto& [notation, color, position] : board.getPieces()) {
			if (tolower(notation) == input[0] && color == board.getCurrentTurn()) possibleFroms.emplace(position);
			if (tolower(notation) == input[2] && color != board.getCurrentTurn()) possibleTos.emplace(position);
		}

		optional<int> optMoveIndex = nullopt;
		auto availableMoves = board.getAvailableMoves();
		for (int i = 0; i < availableMoves.size(); ++i) {
			if (possibleFroms.find(availableMoves[i].from) != possibleFroms.end() &&
				possibleTos.find(availableMoves[i].to) != possibleTos.end())
			{
				if (!optMoveIndex) optMoveIndex = i;
				else return nullopt;
			}
		}

		return optMoveIndex;
	}

	optional<int> getKingsideCastlingMove(const model::Board& board) {
		optional<int> optMoveIndex = nullopt;

		vector<model::Piece::Position> kings;
		for (const auto& [notation, color, position] : board.getPieces()) {
			if (board.getCurrentTurn() == color && tolower(notation) == 'k') {
				kings.push_back(position);
			}
		}
		auto moves = board.getAvailableMoves();
		for (int i = 0; i < moves.size(); ++i) {
			if (std::find(kings.begin(), kings.end(), moves[i].from) != kings.end() && moves[i].to.x == moves[i].from.x + 2) {
				if (!optMoveIndex) optMoveIndex = i;
				else return nullopt;
			}
		}

		return optMoveIndex;
	}

	optional<int> getQueensideCastlingMove(const model::Board& board) {
		optional<int> optMoveIndex = nullopt;

		vector<model::Piece::Position> kings;
		for (const auto& [notation, color, position] : board.getPieces()) {
			if (board.getCurrentTurn() == color && tolower(notation) == 'k') {
				kings.push_back(position);
			}
		}
		auto moves = board.getAvailableMoves();
		for (int i = 0; i < moves.size(); ++i) {
			if (std::find(kings.begin(), kings.end(), moves[i].from) != kings.end() && moves[i].to.x == moves[i].from.x - 2) {
				if (!optMoveIndex) optMoveIndex = i;
				else return nullopt;
			}
		}

		return optMoveIndex;
	}

	UserAction parseInput(string& input, const model::Board& board) {
		for (auto& inputChar : input) inputChar = tolower(inputChar);

		if (input.size() == 2 && std::isalpha(input[0]) && std::isdigit(input[1])) return UserAction::indicatePosition;
		if ((input.size() == 3 && std::isalpha(input[0]) && std::isalpha(input[1]) && isdigit(input[2])) ||
			(input.size() == 2 && std::isalpha(input[0]) && input[1] == 'x') || // TODO: implement 
			(input.size() == 3 && std::isalpha(input[0]) && input[1] == 'x' && isalpha(input[2])) ||
			(input == "00" || input == "0-0" || input == "oo" || input == "o-o") ||
			(input == "000" || input == "0-0-0" || input == "ooo" || input == "o-o-o")) return UserAction::algebraicNotation;
		if (input == "c" || input == "cancel") return deselectPiece;
		if (input == "r" || input == "reset") return UserAction::reset;
		if (input == "ai-white") return UserAction::aiWhite;
		if (input == "ai-black") return UserAction::aiBlack;
		if (input == "a" || input == "available") return UserAction::availableMoves;
		if (input == "h" || input == "help") return UserAction::help;
		if (input == "e" || input == "exit") return UserAction::exitGame;

		return UserAction::invalidAction;
	}

	vector<model::Piece::Position> getSelectedPieceMoves(const optional<model::Piece::Position>& selectedPiece, const vector<model::Move>& availableMoves) {
		vector<model::Piece::Position> result;
		if (selectedPiece) {
			for (const model::Move& move : availableMoves) {
				if (move.from == selectedPiece) {
					result.push_back(move.to);
				}
			}
		}
		return result;
	}
}

namespace chess {

	void play() {
		string input;
		UserAction userAction = invalidAction;
		while (std::cin && userAction != UserAction::exitGame) {
			model::Board board;
			optional<model::Piece::Position> selectedPiece = nullopt;
			optional<model::Piece::Color> ai = nullopt;

			vector<model::Piece::Position> selectedPieceMoves = { };
			view::updateBoardString(board);

			string message = "Game start. Enter 'help' for a list of commands.";
			while (std::cin && !board.getAvailableMoves().empty()) {
				if (ai && *ai == board.getCurrentTurn()) {
					board.makeMove(chess::ai::getMove(board));
					message = "AI move complete.";
				}
				else {
					do {
						view::printHeader();
						view::printCurrentTurn(board);
						view::printBoardString();
						view::printMessage(message);
						input = view::promptUser();

						userAction = parseInput(input, board);
						processUserAction(userAction, input, board, selectedPiece, ai, message);
					} while (std::cin && userAction != exitGame);
				}

				selectedPieceMoves = getSelectedPieceMoves(selectedPiece, board.getAvailableMoves());
				view::updateBoardString(board, selectedPieceMoves);
			}

			view::printHeader();
			if (board.pieceToCaptureInCheck(board.getCurrentTurn())) {
				if (board.getCurrentTurn() == model::Piece::Color::white) view::printMessage("Black Wins!");
				else /*board.getCurrentTurn() == Piece::Color::black*/ view::printMessage("White Wins!");
			}
			else {
				view::printMessage("Stalemate!");
			}
			view::printBoardString();
			view::printMessage("Thanks for playing! Press enter to start a new game.");
			view::promptUser();
		}
	}
}