// chess_ai_minimax.h
// by Jake Charles Osborne III
#pragma once



#include "../../../MVC/Model/chess_model.h"



namespace chess::ai {

	enum class Difficulty { random, easy, medium, hard};
	int getMove(const chess::model::Board&, Difficulty = Difficulty::medium);

}