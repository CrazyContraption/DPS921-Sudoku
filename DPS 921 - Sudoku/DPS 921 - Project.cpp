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

#include <cstdlib>  // atoi
#include <iostream> // IO
#include "Sudoku.h" // Game Logic
#include "Timer.h"  // Timer Logic

int main(int argc, char* argv[]) {

    if (argc != 4) {
        std::cerr
            << "Invalid number of arugments: Expected 3, got " + argc - 1 << std::endl
            << "Expected syntax: " << argv[0] << " seed(int, -1 for random) difficulty(int, 0-20) threads(0:serial 1+: OMP)" << std::endl;
        return 1;
    }

    int seed = atoi(argv[1]);

    int difficulty = atoi(argv[2]);
    if (difficulty > Sudoku::MAX_DIFFICULTY || difficulty < 0) {
        std::cerr
            << "Difficulty was outside acceptable ranges: " << difficulty << std::endl
            << "Expected range: 0 - " << Sudoku::MAX_DIFFICULTY << " (inclusive)" << std::endl;
        return 1;
    }

    int threads = atoi(argv[3]);
    if (threads < 0) {
        std::cerr
            << "Difficulty was outside acceptable ranges: " << threads << std::endl
            << "Expected range: 0 (serial), 1 - " << omp_get_thread_limit() << " (OMP)(inclusive)" << std::endl;
        return 1;
    }

    bool boardValid = false;

    // Create our object
    Sudoku myGame = Sudoku(seed, difficulty); // 123 is the seed. Set to any integer. Set to negative to disable seed (IE: -1). 0 is the difficulty, which can go up to 20.

    // Print out the board
    myGame.print();
    
    Timer timer = Timer();
    timer.start();
    boardValid = myGame.solve(threads);
    //boardValid = myGame.solveNotationOMP(threads); // myGame.solveBacktrackingOMP(threads); /// BAD BOI
    timer.stop();

    myGame.print();

    bool solved = myGame.isBoardSolved();
    bool valid = myGame.isBoardValid();

    std::cout.sync_with_stdio(false);

    std::cout
        << "  Solvable:  " << (boardValid ? "YES" : "NOT ENOUGH INFORMATION") << std::endl
        << "  Status:    " << (solved ? "Done" : "Unsolved") << std::endl
        << "  Valid:     " << (valid ? "YES" : "NO") << std::endl
        << std::endl;
    if (threads < 0)
        std::cout << "  Method:  Serial - Backtracking" << std::endl;
    else if (threads == 0)
        std::cout << "  Method:  Serial - Notation" << std::endl;
    else
        std::cout << "  Method:  OMP - " << threads << " thread" << (threads > 1 ? "s" : "") << std::endl;
    std::cout << std::endl
        << "  Level:  " << difficulty << std::endl
        << "  Timer:  " << timer.getDuration() << "ms" << std::endl
        << "  Seed:   " << seed << std::endl;

    if (solved && boardValid && valid)
        return 0; // Board was solved, and is valid. Hurray!
    if (!solved && valid)
        return 2; // Not solved, but valid... Solver is broken in this case.
    if (!boardValid)
        return 4; // Solved... but valid? Something went HORRIBLY wrong if this is the case
    return 3; // Not solved and invalid, standard response for an unsolvable board.
}