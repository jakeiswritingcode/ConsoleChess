// chess_ai_evaluation.h
// by Jake Charles Osborne III



#include "../../MVC/Model/chess_model.h"



namespace chess::ai::evaluation {

	double evaluate(const chess::model::Board& board, const chess::model::Piece::Color& maximizingPlayer);

}