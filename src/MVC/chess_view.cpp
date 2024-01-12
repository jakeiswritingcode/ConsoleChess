// chess_view.cpp
// by Jake Charles Osborne III



#include "chess_view.h"
#include "chess_model.h"

#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <optional>

using std::cin;
using std::cout;
using std::getline;
using std::string;
using std::vector;
using std::optional;
using std::nullopt;
using std::exception;

using namespace chess::model;



namespace chess::view {

	const string WINDOW_MARGIN = "    ";
	const string PROMPT = WINDOW_MARGIN + "->";
	string boardString = "";

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
			throw exception("3+ players cannot be represented in console window");
		}
		cout << '\n';
	}

	string updateBoardString(const Board& board, const vector<Piece::Position>& selectedPieceMoves) {
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
				throw exception("3+ players cannot be represented in console window");
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

	void printBoardString() {
		cout << boardString;
	}

	void printMessage(string message) {
		cout << WINDOW_MARGIN << message << "\n\n";
	}

	void printHelpMenu() {
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
	}

	string promptUser() {
		cout << PROMPT;

		string input;
		getline(cin, input);
		return input;
	}

}