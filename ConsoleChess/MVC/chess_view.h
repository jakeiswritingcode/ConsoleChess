// chess_view.h
// by Jake Charles Osborne III
#pragma once

namespace ChessView {
	void printHeader();
	void printCurrentTurn(const Board& board);
	std::string generateBoardString(const Board& board, const vector<Piece::Position>& selectedPieceMoves = { });
	void printBoardString();
	void printMessage(string message);
	void printHelpMenu();

}