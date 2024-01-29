# ConsoleChess
A console application for playing chess against a friend or AI.

Checkmate Test Input:  
RESET G2 G4 E7 E6 F2 F3 D8

Pinned Piece Test Input:  
RESET F2 F3 B8 C6 E1 F2 B7 B5 F2 G3 A8 B8 G3 H4 B8 B6 D1 E1 B6 A6 E1 F2 C6 B8 F2 D4 A6 A4 D4

En Passant Test Input:  
RESET E2 E4 B8 A6 E4 E5 D7 D5 E5

Castling Test Input:  
RESET G2 G3 B8 A6 G1 F3 A6 B8 F1 H3 B8 A6 E1

Promotion Test Input:  
RESET B2 B3 B8 A6 B3 B4 A6 B8 B4 B5 B8 A6 B5 A6 G8 H6 A6 B7 H6 G8 B7

## TODO:
- resolve build errrors from initial AI commit
- automatic stalemate/victory detected upon insufficient material
- automated display using the ncurses/PDCurses library for console applications
- prompt user for preferred type of pawn promotion