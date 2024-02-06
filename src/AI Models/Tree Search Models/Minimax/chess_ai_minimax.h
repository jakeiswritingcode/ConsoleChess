// chess_ai_minimax.h
// by Jake Charles Osborne III
#pragma once



#include "../../../MVC/Model/chess_model.h"



namespace chess::ai {

	MinimaxResult minimax(const model::Board&, const model::Piece::Color& maximizingPlayer, const int& depth);
	MinimaxResult multithreadingMinimax(const model::Board&, const model::Piece::Color& maximizingPlayer, const int& depth);

}