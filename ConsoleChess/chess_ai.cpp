// chess_ai.cpp
// by Quenton Wood



#include "chess_ai.h"
#include <random>
// for Q: extra include statements from the STL go here
#include "chess_game.h"



using std::vector;
using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;
using namespace Chess;



namespace {
	Move selectRandomMove(const vector<Move>& availableMoves) {
		random_device rd; // used to obtain a seed for the random number engine
		mt19937 mt(rd()); // mersenne_twister_engine seeded with rd()
		uniform_int_distribution<> moveDistribution(0, availableMoves.size() - 1);

		return availableMoves[moveDistribution(mt)];
	}

	// for Q: helper functions go here
}

namespace ChessAI {
	void makeMove(Board& board) { // for Q: difficulty could be added as an additional argument (enum type or char, your preference), but the IO would need to be changed
		vector<Move> availableMoves = board.getAvailableMoves();

		// for Q: determine best move here
		Move selectedMove = selectRandomMove(availableMoves);
		
		board.makeMove(selectedMove);
	}
}