// chess_view.h
// by Jake Charles Osborne III
#pragma once

#include "chess_model.h"

namespace chess::view {
	void printHeader();
	void printCurrentTurn(const chess::model::Board& board);
	std::string updateBoardString(const chess::model::Board& board, const std::vector<chess::model::Piece::Position>& selectedPieceMoves = { });
	void printBoardString();
	void printMessage(std::string message);
	void printHelpMenu();
	std::string promptUser();
}