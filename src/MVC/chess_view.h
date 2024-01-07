// chess_view.h
// by Jake Charles Osborne III
#pragma once

#include "chess_model.h"

namespace Chess {
	namespace View {
		void printHeader();
		void printCurrentTurn(const Chess::Board& board);
		std::string updateBoardString(const Chess::Board& board, const std::vector<Chess::Piece::Position>& selectedPieceMoves = { });
		void printBoardString();
		void printMessage(std::string message);
		void printHelpMenu();
		std::string promptUser();
	}
}