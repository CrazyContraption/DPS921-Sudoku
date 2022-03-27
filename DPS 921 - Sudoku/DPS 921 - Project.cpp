/*
* DPS 921 - Sudoku.cpp
* Created: March 2022
* 
* Developed by:
* - Darien B
* - Jaron B.
* 
* Dynamically creates and solves Sudoku puzzles using different algorithms
* to optimize for different conditional requirements.
*/ 

#include <iostream>
#include "Sudoku.h"

int main()
{
    // Create our object
    Sudoku myGame = Sudoku(123, 0); // 123 is the seed. Set to any integer. Set to negative to disable seed (IE: -1). 0 is the difficulty, which can go up to 20.

    // Print out the board
    myGame.print();

    std::cout << "  Solved: " << (myGame.solveBoard() ? "Solved" : "Unsolved") << std::endl;
    std::cout << "  Valid:  " << (myGame.isBoardValid() ? "YES" : "NO") << std::endl;
}