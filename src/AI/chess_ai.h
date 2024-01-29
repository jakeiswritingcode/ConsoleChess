// chess_ai.h
// by Jake Charles Osborne III
#pragma once



#include"../MVC/chess_model.h"



namespace chess::ai {
	enum class Difficulty { random, easy, medium, hard};
	int getMove(const chess::model::Board&, Difficulty = Difficulty::medium);
}