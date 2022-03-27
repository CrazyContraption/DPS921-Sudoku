#pragma once
#include <algorithm>
#include <chrono>
#include <iostream>
#include <math.h>

// Sue me for using a global constant
#define N 9
// That's what I thought... chickens

class Sudoku {
private:
	// Magical dynamic math constant for magic thingies later on.
	int SRN = (int)std::sqrt(N);

	// The sudoku game board
	short m_Board[N][N];

	// The set seed, for vanity purposes
	int m_Seed = -1; // Random by default, positives is considered "set"

	/// <summary>Checks if the relative line is fully filled, and contains only one of every digit.
	/// <para>Lines are identified from 0 - 8 inclusive. Top-leftmost is 0, incrementing from left-to-right or top-to-bottom.</para>
	/// <param name='lineID'>- Short identifying the line to search within.</param>
	/// <param name='vertical'>- Boolean denoting if the column (vertical row) should be scanned instead of horizontal.</param>
	/// <returns>Boolean denoting if the line is considered valid and completed.</returns>
	/// </summary>
	bool lineCorrect(short lineID, bool vertical = false) {
		bool hasNumbers[N] = { false, false, false, false, false, false, false, false, false };

		for (size_t index = 0; index < N; index++)
			if (!vertical && m_Board[lineID][index] > 0 && m_Board[lineID][index] <= N)
				hasNumbers[m_Board[lineID][index] - 1] = true;
			else if (vertical && m_Board[lineID][index] > 0 && m_Board[lineID][index] <= N)
				hasNumbers[m_Board[lineID][index] - 1] = true;
			else
				return false;

		for (size_t index = 0; index < N && hasNumbers[0]; index++)
			hasNumbers[0] = hasNumbers[0] && hasNumbers[index];

		return hasNumbers[0];
	}

	/// <summary>Checks if the relative 3x3 segment is fully filled, and contains only one of every digit.
	/// <para>Segments are identified from 0 - 8 inclusive. Top-left is 0, incrementing from left-to-right, and then top-to-bottom.</para>
	/// <param name='segmentID'>- Short identifying the segment to search within.</param>
	/// <returns>Boolean denoting if the segment is considered valid and completed.</returns>
	/// </summary>
	bool segmentCorrect(short segmentID) {
		bool hasNumbers[N] = { false, false, false, false, false, false, false, false, false };

		short row = std::floor(segmentID / 3) * 3;
		short col = (segmentID % 3) * 3;

		for (size_t row_index = row; row_index < row + 3; row_index++)
			for (size_t col_index = col; col_index < col + 3; col_index++)
				if (m_Board[row_index][col_index] > 0 && m_Board[row_index][col_index] <= N)
					hasNumbers[m_Board[row_index][col_index] - 1] = true;
				else
					return false;

		for (size_t index = 0; index < N && hasNumbers[0]; index++)
			hasNumbers[0] = hasNumbers[0] && hasNumbers[index];

		return hasNumbers[0];
	}

	/// <summary>Checks the relative line segment for a number to see if a specified number exists there already.
	/// <para>Lines are identified from 0 - 8 inclusive. Top-leftmost is 0, incrementing from left-to-right or top-to-bottom.</para>
	/// <param name='lineID'>- Short identifying the line to search within.</param>
	/// <param name='num'>- Short to search for within the 3x3 segment.</param>
	/// <param name='ignoredLine'>- Optional short to IGNORE if that's the location you're scanning from (ignore self). Negative/omit to disable.</param>
	/// <param name='vertical'>- Boolean denoting if the column (vertical row) should be scanned instead of horizontal.</param>
	/// <returns>Boolean denoting if the number specified already exists in the provided line.</returns>
	/// </summary>
	bool usedInLine(short lineID, short num, short ignoredLine = -1, bool vertical = false) {
		for (size_t index = 0; index < N; index++)
			if (ignoredLine != index)
				if (!vertical && m_Board[lineID][index] == num)
					return true;
				else if (vertical && m_Board[index][lineID] == num)
					return true;
		return false;
	}

	/// <summary>Checks the relative 3x3 segment for a number to see if a specified number exists there already.
	/// <para>Segments are identified from 0 - 8 inclusive. Top-left is 0, incrementing from left-to-right, and then top-to-bottom.</para>
	/// <param name='segmentID'>- Short identifying the segment to search within.</param>
	/// <param name='num'>- Short to search for within the 3x3 segment.</param>
	/// <param name='ignoreIndex'>- Optional short to IGNORE if that's the location you're scanning from (ignore self). Negative/omit to disable.</param>
	/// <returns>Boolean denoting if the number specified already exists in the provided segment.</returns>
	/// </summary>
	bool usedInSegment(short segmentID, short num, short ignoreIndex = -1) {
		short row = std::floor(segmentID / 3) * 3;
		short col = (segmentID % 3) * 3;
		for (size_t row_index = row, index = 0; row_index < row + 3; row_index++)
			for (size_t col_index = col; col_index < col + 3; col_index++)
				if (index != ignoreIndex)
					if (m_Board[row_index][col_index] == num)
						return true;
		return false;
	}

	/// <summary>Provides a random integer.
	/// <para>Respects set seed, and the max/min of the desired ranges.</para>
	/// <param name='ignore'>- Integer to ignore when producing a random response.</param>
	/// <param name='min'>- Optional integer that marks the lower inclusive bounds for the random integer.</param>
	/// <param name='max'>- Optional integer that marks the upper inclusive bounds for the random integer.</param>
	/// <returns>A *random* positive integer.</returns>
	/// </summary>
	int getRandom(int ignore = -1, int min = 0, int max = N - 1) {
		int num = min + std::rand() % ((max + 1) - min);
		if (num == ignore)
			return getRandom(ignore, min, max);
		return num;
	}

	/// <summary>Shuffles an array around of the specified size
	/// <para>Follows seeding conventions. Swaps array locations iteratively to simulate a "random" order.</para>
	/// <param name='numbers'>- Pointer to a short array with numbers to shuffle.</param>
	/// <param name='size'>- Size_t denoting the size of the array provided.</param>
	/// <returns>Nothing directly. Modifies the numbers array passed as a parameter.</returns>
	/// </summary>
	void shuffleArray(short* numbers, size_t size) {
		for (size_t iteration = 0; iteration < getRandom(-1, 2, 6); iteration++)	// Randomise how many times we "shuffle" the array
			for (size_t index = 0; index < size; index++)							// For each index...
				std::swap(numbers[getRandom()], numbers[getRandom(index)]);			// ...attempt a swap with another index
	}

	/// <summary>Fills the remaining squares that aren't filled by the initial seeding of Sudoku(int seed).
	/// <para>Honestly? This is a black-box recusive function that trial-and-errors every cell to fill it out. This thing is magic.</para>
	/// <param name='row'>- Integer denoting the desired row to fill (0 - 8 inclusive).</param>
	/// <param name='col'>- Integer denoting the desired column to fill (0 - 8 inclusive).</param>
	/// <returns>This *should* return false if a valid set of remaining cells could not be filled.</returns>
	/// <seealso cref="Sudoku(int seed)"/>
	/// </summary>
	bool fillRemaining(int row, int col) {
		if (col >= N && row < N - 1) {
			row = row + 1;
			col = 0;
		}

		if (row >= N && col >= N)
			return true;

		if (row < SRN) {
			if (col < SRN)
				col = SRN;
		} else if (row < N - SRN) {
			if (col == (int)(row / SRN) * SRN)
				col = col + SRN;
		} else {
			if (col == N - SRN) {
				row = row + 1;
				col = 0;
				if (row >= N)
					return true;
			}
		}

		for (int num = 1; num <= N; num++) {
			if (checkIfSafe(row, col, num)) {
				m_Board[row][col] = num;
				if (fillRemaining(row, col + 1))
					return true;

				m_Board[row][col] = 0;
			}
		}
		return false;
	}

public:
	/// <summary>Checks if a specified cell is safe for a specified number to be valid
	/// <para>In Sudoku, a number may not share a 3x3 grid, row, or column with itself. This will check all these corresponding cells for duplicates.</para>
	/// <param name='row'>- Short denoting the desired row to check (0 - 8 inclusive).</param>
	/// <param name='col'>- Short denoting the desired column to check (0 - 8 inclusive).</param>
	/// <param name='num'>- Short the number to check for.</param>
	/// <returns>Boolean specifying if the cell is considered to be valid/safe for that number to be written to.</returns>
	/// </summary>
	bool checkIfSafe(short row, short col, short num) {
		return m_Board[row][col] == num
			|| (!usedInLine(col, num, row, true)
				&& !usedInLine(row, num, col, false)
				&& !usedInSegment((std::floor(row / 3) * 3) + std::floor(col / 3), num, (row % 3) * 3 + (col % 3)));
	}

	/// <summary>Default constructor which creates an empty Sudoku board.
	/// <para>Does nothing other than put the board into a safe empty state.</para>
	/// </summary>
	Sudoku() {
		for (size_t row_index = 0; row_index < N; row_index++)
			for (size_t col_index = 0; col_index < N; col_index++)
				m_Board[row_index][col_index] = 0;
	}

	/// <summary>Constructs a Sudoku which is complete, and 99.9% of the time valid.
	/// <para>The board will be set to a completed state based on the seed -if any- and should be valid once completed. Use isBoardSolved() for optimal results.</para>
	/// <param name='seed'>- Integer specifying a seed to use when generating the new board. Use a negative integer to omit, and use random seed generation.</param>
	/// <seealso cref="isBoardSolved()"/>
	/// </summary>
	Sudoku(int seed) : Sudoku() {
		short numbers[N] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		m_Seed = seed;
		if (m_Seed < 0)
			std::srand(time(NULL));
		else
			std::srand((unsigned)m_Seed);

		for (int set = 0; set < 3; set++) {
			shuffleArray(numbers, N);
			for (size_t row_index = 3 * set, index = 0; row_index < 3 * set + 3; row_index++)
				for (size_t col_index = 3 * set; col_index < 3 * set + 3; col_index++, index++)
					m_Board[row_index][col_index] = numbers[index];
		}

		fillRemaining(0, SRN);
	}

	/// <summary>Constructs a Sudoku which is incomplete, but solvable.
	/// <para>In addition to creating a filled board, this constructor procedurally removes cells based on the desired difficulty, allowing for other things to re-solve it.</para>
	/// <param name='seed'>Integer specifying a seed to use when generating the new board. Use a negative integer to omit, and use random seed generation.</param>
	/// <param name='difficulty'>Unsigned integer specifying the desired difficulty of the new board. Higher is harder. 0 specifies completed. 20 is max.</param>
	/// <seealso cref="isBoardSolved"/>
	/// </summary>
	Sudoku(int seed, unsigned difficulty) : Sudoku(seed) {
		if (difficulty > 20)
			difficulty = 20; // Prevents things from getting TOO crazy

		for (size_t iteration = 0; iteration < difficulty; iteration++)
			for (size_t segmentID = 0; segmentID < N; segmentID++) { // Remove a cell from each segment
				short row = std::floor(segmentID / 3) * 3;
				short col = (segmentID % 3) * 3;
				m_Board[getRandom(-1, row, row + 2)][getRandom(-1, col, col + 2)] = 0;
			}
	}

	/// <summary>Prints out the state of the Sudoku board.
	/// <para>Outputs to cout, clears and then populates from the class variables.</para>
	/// </summary>
	void print() {
		system("CLS");
		std::cout << std::endl
			<< " Seed: " << m_Seed << std::endl
			<< " Difficulty: " << 0 << std::endl << std::endl;
		for (size_t row_index = 0; row_index < N + 1; row_index++) {
			if (row_index % 3 == 0)
				std::cout << "  +-------+-------+-------+" << std::endl;
			if (row_index >= N)
				break;
			std::cout << ' ';
			for (size_t col_index = 0; col_index < N; col_index++) {
				if (col_index % 3 == 0)
					std::cout << " |";
				std::cout << ' ';
				if (m_Board[row_index][col_index] > 0)
					std::cout << m_Board[row_index][col_index];
				else
					std::cout << '-';
			}
			std::cout << " |" << std::endl;
		}
		std::cout << std::endl;
	}

	/// <summary>Tests if the board is in a solved state.
	/// <para>Board must be valid, filled, and otherwise correct according to Sudoku rules.</para>
	/// <returns>Boolean denoting if the board is considered to be solved.</returns>
	/// </summary>
	bool isBoardSolved() {
		bool solved = true;
		for (size_t lineID = 0; lineID < N && solved; lineID++)
			solved = lineCorrect(lineID, false);
		for (size_t lineID = 0; lineID < N && solved; lineID++)
			solved = lineCorrect(lineID, true);
		for (size_t segmentID = 0; segmentID < N && solved; segmentID++)
			solved = segmentCorrect(segmentID);
		return solved;
	}

	/// <summary>Tests if the board is in a valid state.
	/// <para>Denote if a board is valid, ignoring incomplete spaces, but also denoting if the board is still solvable.</para>
	/// <returns>Boolean denoting if the board is considered to valid.</returns>
	/// </summary>
	bool isBoardValid() {

		bool isValid = true;

		for (size_t row_index = 0; row_index < N && isValid; row_index++)
			for (size_t col_index = 0; col_index < N && isValid; col_index++)
				isValid = checkIfSafe(row_index, col_index, m_Board[row_index][col_index]);

		// TODO: Validate if the board is solvable.
		// THIS MUST BE DONE WITH A SOLVING ALGORITHM. I LEAVE THIS TO YOU JARON! :)

		return isValid;
	}

	/// <summary>Solves the board by filling in any empty cells
	/// <para>Unused. Should solve iteratively or recursively to determine if a board can be solved, and if so - how? Should modify m_Board.</para>
	/// <returns>Boolean denoting if the board is solvable. Failed attemps return false upon realization of an impossible outcome (multiple or no solutions).</returns>
	/// </summary>
	bool solveBoard() {

		// Unimplemented as this requires solving functions and capabilities that I was too lazy to code with the rest of this headache.
		// TODO: Jaron I leave this to you!

		return isBoardSolved();
	}
};