// chess_io.cpp
// by Jake Charles Osborne III



#include "chess_io.h"
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <optional>
#include "chess_game.h"
#include "chess_ai.h"



using std::cin;
using std::cout;
using std::string;
using std::vector;
using std::optional;
using std::nullopt;
using std::exception;
using namespace Chess;



namespace {
	const string WINDOW_MARGIN = "    ";
	const string PROMPT = WINDOW_MARGIN + "->";

	void printHeader() {
		cout << "\n\n\n"
			<< WINDOW_MARGIN << "ConsoleChess by Jake Charles Osborne III" << '\n'
			<< WINDOW_MARGIN << "available at https://github.com/jco-iii/ConsoleChess" << '\n'
			<< '\n';
	}

	void printCurrentTurn(const Board& board) {
		if (board.getCurrentTurn() == Piece::Color::white) {
			cout << WINDOW_MARGIN << "Current Turn: White (UPPER CASE)" << '\n';
		}
		else if (board.getCurrentTurn() == Piece::Color::black) {
			cout << WINDOW_MARGIN << "Current Turn: Black (lower case)" << '\n';
		}
		else {
			throw exception("modified backend cannot be represented in console window");
		}
		cout << '\n';
	}

	string generateBoardString(const Board& board, const vector<Piece::Position>& selectedPieceMoves = { }) {
		const char WHITE_SQUARE_CHAR = '-';
		const char BLACK_SQUARE_CHAR = ' ';
		const char WHITE_SQUARE_HIGHLIGHT_CHAR = '\\';
		const char BLACK_SQUARE_HIGHLIGHT_CHAR = '/';
		const int SQUARE_HEIGHT = 3;
		const int SQUARE_WIDTH = 6;

		char grid[8][8];
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j) {
				grid[i][j] = (i + j) % 2 == 1 ? WHITE_SQUARE_CHAR : BLACK_SQUARE_CHAR;
			}
		}
		for (const Piece::Position& move : selectedPieceMoves) {
			int x = move.x - 'A';
			int y = move.y - 1;
			grid[x][y] = (x + y) % 2 == 1 ? WHITE_SQUARE_HIGHLIGHT_CHAR : BLACK_SQUARE_HIGHLIGHT_CHAR;
		}
		for (const auto& [notation, color, position] : board.getPieces()) {
			if (color == Piece::Color::white) {
				grid[position.x - 'A'][position.y - 1] = notation;
			}
			else if (color == Piece::Color::black) {
				grid[position.x - 'A'][position.y - 1] = tolower(notation);
			}
			else {
				throw exception("modified backend cannot be represented in console window");
			}
		}

		string result = "";

		result += "       A     B     C     D     E     F     G     H\n";
		result += '\n';
		for (int rankIndex = 7; rankIndex >= 0; --rankIndex) {
			for (int h = 0; h < SQUARE_HEIGHT; ++h) {
				result += "  ";
				if (h == 1) result += '1' + rankIndex;
				else result += ' ';
				result += ' ';
				for (int fileIndex = 0; fileIndex < 8; ++fileIndex) {
					for (int w = 0; w < SQUARE_WIDTH; ++w) {
						if (h == 1 && w == 3) {
							result += grid[fileIndex][rankIndex];
						}
						else {
							bool squareHighlighted = false;
							for (const Piece::Position& square : selectedPieceMoves) {
								if (square == Piece::Position{ char('A' + fileIndex), 1 + rankIndex }) {
									squareHighlighted = true;
								}
							}
							if ((fileIndex + rankIndex) % 2 == 1) {
								if (squareHighlighted) {
									result += WHITE_SQUARE_HIGHLIGHT_CHAR;
								}
								else {
									result += WHITE_SQUARE_CHAR;
								}

							}
							else { // (file - 'A' + rank) % 2 == 0
								if (squareHighlighted) {
									result += BLACK_SQUARE_HIGHLIGHT_CHAR;
								}
								else {
									result += BLACK_SQUARE_CHAR;
								}
							}
						}
					}
				}
				if (h == 1) {
					result += " ";
					result += '1' + rankIndex;
				}
				result += '\n';
			}
		}
		result += '\n';
		result += "       A     B     C     D     E     F     G     H\n";
		result += '\n';
		result += '\n';
		return result;
	}

	void printMessage(string message) {
		cout << WINDOW_MARGIN << "\"" << message << "\"" << "\n\n";
	}

	bool parseInput(string& input, string& message, optional<Piece::Position>& selectedPiece, optional<Piece::Color>& ai, Board& board) {
		for (auto& inputChar : input) {
			inputChar = tolower(inputChar);
		}

		if (input == "h" || input == "help") {
			cout << "\n\n\n"
				<< WINDOW_MARGIN << "Commands:\n"
				<< "\n"
				<< "\n"
				<< WINDOW_MARGIN << "\"h\" or \"help\"\n"
				<< WINDOW_MARGIN << "display help screen\n"\
				<< "\n"
				<< WINDOW_MARGIN << "\"c\" or \"cancel\"\n"
				<< WINDOW_MARGIN << "deselect the currently selected piece\n"
				<< "\n"
				<< WINDOW_MARGIN << "\"a\" or \"available\"\n"
				<< WINDOW_MARGIN << "receive pieces with currently available moves\n"
				<< "\n"
				<< WINDOW_MARGIN << "\"ai-white\"\n"
				<< WINDOW_MARGIN << "start new game with AI playing white\n"
				<< "\n"
				<< WINDOW_MARGIN << "\"ai-black\"\n"
				<< WINDOW_MARGIN << "start new game with AI playing black\n"
				<< "\n"
				<< WINDOW_MARGIN << "\"reset\"\n"
				<< WINDOW_MARGIN << "begin a new game without AI\n"
				<< "\n"
				<< WINDOW_MARGIN << "\"exit\"\n"
				<< WINDOW_MARGIN << "end the game\n"
				<< "\n"
				<< "\n"
				<< WINDOW_MARGIN << "Press enter to resume the game.\n"
				<< PROMPT;
			while (getchar() != '\n');
			while (getchar() != '\n');
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

namespace ChessIO {
	void play() {
		string input = "";
		while (cin && input != "exit") {

			Board board;

			optional<Piece::Position> selectedPiece = nullopt;
			optional<Piece::Color> ai = nullopt;

			vector<Piece::Position> selectedPieceMoves = { };
			string boardString = generateBoardString(board);

			string message = "Begin at your leisure. Enter 'help' for a list of commands.";
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
						cout << boardString;
						printMessage(message);
						cout << PROMPT;

						cin >> input;
						if (input == "exit") return;
						inputValid = parseInput(input, message, selectedPiece, ai, board);
					} while (cin && !inputValid);
				}

				selectedPieceMoves = getSelectedPieceMoves(selectedPiece, board.getAvailableMoves());
				boardString = generateBoardString(board, selectedPieceMoves);
			}

			printHeader();
			if (board.pieceToCaptureInCheck(board.getCurrentTurn())) {
				if (board.getCurrentTurn() == Piece::Color::white) cout << WINDOW_MARGIN << "Black Wins!" << "\n\n";
				else cout << WINDOW_MARGIN << "White Wins!" << "\n\n";
			}
			else {
				cout << WINDOW_MARGIN << "Stalemate!" << "\n\n";
			}
			cout << boardString;
			printMessage("Thanks for playing!");
			cout << WINDOW_MARGIN << "Press enter to start a new game." << "\n\n"
				<< PROMPT;
			while (getchar() != '\n');
			while (getchar() != '\n');
		}
	}
}