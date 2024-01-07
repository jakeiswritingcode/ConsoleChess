// chess_ai.cpp
// by Jake Charles Osborne III



#include "chess_ai.h"
#include "../MVC/chess_model.h"
#include <random>
// TODO: extra include statements from the STL go here



using std::vector;
using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;
using namespace Chess;



namespace {
	int selectRandomMove(const vector<Move>& availableMoves) {
		random_device rd; // used to obtain a seed for the random number engine
		mt19937 mt(rd()); // mersenne_twister_engine seeded with rd()
		uniform_int_distribution<> moveDistribution(0, availableMoves.size() - 1);

		return moveDistribution(mt);
	}

	// TODO: helper functions go here
}

namespace ChessAI {
	void makeMove(Board& board) { // TODO: difficulty could be added as an additional argument (enum)
		vector<Move> availableMoves = board.getAvailableMoves();

		// TODO: determine best move here
		int selectedMove = selectRandomMove(availableMoves);
		
		board.makeMove(selectedMove);
	}
}