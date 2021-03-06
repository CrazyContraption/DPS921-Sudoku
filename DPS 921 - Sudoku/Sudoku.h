#pragma once
#include <algorithm> // Random
#include <chrono>    // Seeding
#include <iostream>  // IO
#include <math.h>    // Rounding
#include <sstream>   // StringStream
#include <omp.h>     // OpenMP

// Sue me for using a global constant
#define N 9
// That's what I thought... chickens

enum State { IN_PROGRESS, SOLVED, STALEMATE, INVALID };

struct Cell {
	int numIs = 0; // Number stored in the cell, 0 is "empty"
	bool canBe[N] = { false, false, false, false, false, false, false, false, false }; // Given an empty cell, what can the cell be?

	Cell(int num = 0) {
		numIs = num;
		if (num != 0)
			wipe();
	}

	void note(int numeral) { canBe[numeral - 1] = true; }
	void denote(int numeral) { canBe[numeral - 1] = false; }
	bool notes(int numeral) { return canBe[numeral - 1]; }
	void wipe() { for (short index = 0; index < N; index++) canBe[index] = false; }
	short onlyNote() { // Gives the only valid numeral note if one, -1 if none are valid, 0 if multiple notes are valid
		short noteIndex = -1;
		for (short index = 0; index < N; index++)
			if (canBe[index] == true)
				if (noteIndex <= -1)
					noteIndex = index + 1;
				else if (noteIndex >= 1)
					return 0;
		return noteIndex;
	}

	int operator = (int num) { numIs = num; if (num != 0) wipe(); } // remove?
	int operator()() { return numIs; }
	operator int() { return numIs; }

	bool operator == (Cell cell) { return numIs == cell.numIs; }
	bool operator == (int num) { return numIs == num; }
	bool operator == (short num) { return numIs == num; }
	bool operator >= (int num) { return numIs >= num; }
	bool operator <= (int num) { return numIs <= num; }
	int operator += (int num) { return numIs += num; }
	int operator -= (int num) { return numIs -= num; }
	int operator *= (int num) { return numIs *= num; }
	bool operator > (int num) { return numIs > num; }
	bool operator < (int num) { return numIs < num; }
	int operator + (int num) { return numIs + num; }
	int operator - (int num) { return numIs - num; }
	int operator * (int num) { return numIs * num; }
};

std::ostream& operator << (std::ostream& os, const Cell& cell) { os << cell.numIs; return os; }

class Sudoku {
private:
	// Magical dynamic math constant for magic thingies later on.
	int SRN = (int)std::sqrt(N);

	// The sudoku game board
	Cell m_Board[N][N];

	State m_State = State::IN_PROGRESS;

	// The set seed, for vanity purposes
	int m_seed = -1; // Random by default, positives is considered "set"
	int m_difficulty = -1; // Invalid by default, positives is considered "set"

	/// <summary>Checks if the relative line is fully filled, and contains only one of every digit.
	/// <para>Lines are identified from 0 - 8 inclusive. Top-leftmost is 0, incrementing from left-to-right or top-to-bottom.</para>
	/// <param name='lineID'>- Short identifying the line to search within.</param>
	/// <param name='vertical'>- Boolean denoting if the column (vertical row) should be scanned instead of horizontal.</param>
	/// <returns>Boolean denoting if the line is considered valid and completed.</returns>
	/// </summary>
	bool lineCorrect(short lineID, bool vertical = false) {
		bool hasNumbers[N] = { false, false, false, false, false, false, false, false, false };

		for (short index = 0; index < N; index++)
			if (!vertical && m_Board[index][lineID] > 0 && m_Board[index][lineID] <= N)
				hasNumbers[m_Board[index][lineID] - 1] = true;
			else if (vertical && m_Board[lineID][index] > 0 && m_Board[lineID][index] <= N)
				hasNumbers[m_Board[lineID][index] - 1] = true;
			else
				return false;

		for (short index = 0; index < N && hasNumbers[0]; index++)
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

		for (short row_index = row; row_index < row + 3; row_index++)
			for (short col_index = col; col_index < col + 3; col_index++)
				if (m_Board[col_index][row_index] > 0 && m_Board[col_index][row_index] <= N)
					hasNumbers[m_Board[col_index][row_index] - 1] = true;
				else
					return false;

		for (short index = 0; index < N && hasNumbers[0]; index++)
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
		for (short index = 0; index < N; index++)
			if (ignoredLine != index)
				if (!vertical && m_Board[index][lineID] == num)
					return true;
				else if (vertical && m_Board[lineID][index] == num)
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
		for (short row_index = row, index = 0; row_index < row + 3; row_index++)
			for (short col_index = col; col_index < col + 3; col_index++)
				if (index != ignoreIndex)
					if (m_Board[col_index][row_index] == num)
						return true;
		return false;
	}

	/// <summary> Provides a random integer.
	/// <para> Respects set seed, and the max/min of the desired ranges.</para>
	/// <param name='ignore'> - Integer to ignore when producing a random response.</param>
	/// <param name='min'> - Optional integer that marks the lower inclusive bounds for the random integer.</param>
	/// <param name='max'> - Optional integer that marks the upper inclusive bounds for the random integer.</param>
	/// <returns> A *random* positive integer.</returns>
	/// </summary>
	int getRandom(int ignore = -1, int min = 0, int max = N - 1) {
		int num = min + std::rand() % ((max + 1) - min);
		if (num == ignore)
			return getRandom(ignore, min, max);
		return num;
	}

	/// <summary> Shuffles an array around of the specified size
	/// <para> Follows seeding conventions. Swaps array locations iteratively to simulate a "random" order.</para>
	/// <param name='numbers'> - Pointer to a short array with numbers to shuffle.</param>
	/// <param name='size'> - Size_t denoting the size of the array provided.</param>
	/// <returns> Nothing directly. Modifies the numbers array passed as a parameter.</returns>
	/// </summary>
	void shuffleArray(short* numbers, short size) {
		for (short iteration = 0; iteration < getRandom(-1, 2, 6); iteration++)	// Randomise how many times we "shuffle" the array
			for (short index = 0; index < size; index++)							// For each index...
				std::swap(numbers[getRandom()], numbers[getRandom(index)]);			// ...attempt a swap with another index
	}

	/// <summary> Fills the remaining squares that aren't filled by the initial seeding of Sudoku(int seed).
	/// <para> Honestly? This is a black-box recusive function that trial-and-errors every cell to fill it out. This thing is magic.</para>
	/// <param name='col'> - Integer denoting the desired column to fill (0 - 8 inclusive).</param>
	/// <param name='row'> - Integer denoting the desired row to fill (0 - 8 inclusive).</param>
	/// <returns> This *should* return false if a valid set of remaining cells could not be filled.</returns>
	/// <seealso cref="Sudoku(int seed)"/>
	/// </summary>
	bool fillRemaining(int col, int row) {
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
			if (checkIfSafe(col, row, num)) {
				m_Board[col][row] = num;
				if (fillRemaining(col + 1, row))
					return true;

				m_Board[col][row] = 0;
			}
		}
		return false;
	}

	/// <summary> Wipes respective rows, columns, and segments when a number is added to the board.
	/// <para> When a number is placed in the grid, this will update the notes of surrounding cells to let them know that note no longer applies.</para>
	/// <param name='col'> - Short denoting the desired column to wipe with respect to (0 - 8 inclusive).</param>
	/// <param name='row'> - Short denoting the desired row to wipe with respect to (0 - 8 inclusive).</param>
	/// <param name='numeral'> - Short of the noted number to wipe/reset to false.</param>
	/// </summary>
	void wipeNotations(short col, short row, short numeral) {
		numeral--;

		// Wipe source cell
		m_Board[col][row].wipe();

		// Wipe containing segment 3x3
		for (short l_col = col - (col % 3); l_col <= col - (col % 3) + 2; l_col++)
			for (short l_row = row - (row % 3); l_row <= row - (row % 3) + 2; l_row++)
				m_Board[l_col][l_row].canBe[numeral] = false;

		// Wipe row
		for (short l_row = 0; l_row < N; l_row++)
			m_Board[col][l_row].canBe[numeral] = false;

		// Wipe col
		for (short l_col = 0; l_col < N; l_col++)
			m_Board[l_col][row].canBe[numeral] = false;
	}

public:
	/// <summary>
	/// The static, constant, hard-coded maximum difficulty that a board can be set to. Should stay between 20-30.
	/// </summary>
	const static unsigned short MAX_DIFFICULTY = 30;

	/// <summary> The static, constant, hard-coded value of if the program should spend less time printing, and more time number-crunching.
	/// <para> We keep this on to keep things flashy ;)</para>
	/// <para> Keep it off for better results though...</para>
	/// </summary>
	const static bool DO_DEBUG_PRINTING = false;

	/// <summary> Default constructor which creates an empty Sudoku board.
	/// <para> Does nothing other than put the board into a safe empty state.</para>
	/// </summary>
	Sudoku() {
		for (short row_index = 0; row_index < N; row_index++)
			for (short col_index = 0; col_index < N; col_index++)
				m_Board[row_index][col_index] = 0;
	}

	/// <summary>Constructs a Sudoku which is complete, and 99.9% of the time valid.
	/// <para>The board will be set to a completed state based on the seed -if any- and should be valid once completed. Use isBoardSolved() for optimal results.</para>
	/// <param name='seed'>- Integer specifying a seed to use when generating the new board. Use a negative integer to omit, and use random seed generation.</param>
	/// <seealso cref="isBoardSolved()"/>
	/// </summary>
	Sudoku(int seed) : Sudoku() {
		short numbers[N] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		m_seed = seed;
		if (m_seed < 0)
			std::srand(time(NULL));
		else
			std::srand((unsigned)m_seed);

		for (int set = 0; set < 3; set++) {
			shuffleArray(numbers, N);
			for (short row_index = 3 * set, index = 0; row_index < 3 * set + 3; row_index++)
				for (short col_index = 3 * set; col_index < 3 * set + 3; col_index++, index++)
					m_Board[col_index][row_index] = numbers[index];
		}
		fillRemaining(SRN, 0);
	}

	/// <summary>Constructs a Sudoku which is incomplete, but solvable.
	/// <para>In addition to creating a filled board, this constructor procedurally removes cells based on the desired difficulty, allowing for other things to re-solve it.</para>
	/// <param name='seed'>Integer specifying a seed to use when generating the new board. Use a negative integer to omit, and use random seed generation.</param>
	/// <param name='difficulty'>Unsigned integer specifying the desired difficulty of the new board. Higher is harder. 0 specifies completed. 20 is max.</param>
	/// <seealso cref="isBoardSolved"/>
	/// </summary>
	Sudoku(int seed, int difficulty) : Sudoku(seed) {
		if (difficulty > MAX_DIFFICULTY)
			difficulty = MAX_DIFFICULTY; // Prevents things from getting TOO crazy
		else if (difficulty < 0)
			difficulty = 0;
		m_difficulty = difficulty;

		for (short iteration = 0; iteration < difficulty; iteration++)
			for (short segmentID = 0; segmentID < N; segmentID++) { // Remove a cell from each segment
				short row = std::floor(segmentID / 3) * 3;
				short col = (segmentID % 3) * 3;
				m_Board[getRandom(-1, row, row + 2)][getRandom(-1, col, col + 2)] = 0;
			}
	}

	/// <summary> Checks if a specified cell is safe for a specified number to be valid
	/// <para> In Sudoku, a number may not share a 3x3 grid, row, or column with itself. This will check all these corresponding cells for duplicates.</para>
	/// <param name='col'> - Short denoting the desired column to check (0 - 8 inclusive).</param>
	/// <param name='row'> - Short denoting the desired row to check (0 - 8 inclusive).</param>
	/// <param name='num'> - Short of the number to check for.</param>
	/// <returns> Boolean specifying if the cell is considered to be valid/safe for that number to be written to.</returns>
	/// </summary>
	bool checkIfSafe(short col, short row, short num) {
		bool vSafe = !usedInLine(col, num, row, true);
		bool hSafe = !usedInLine(row, num, col, false);
		bool sSafe = !usedInSegment((std::floor(row / 3) * 3) + std::floor(col / 3), num, (row % 3) * 3 + (col % 3));
		return m_Board[col][row] == num || (vSafe && hSafe && sSafe);
	}

	/// <summary>Prints out the state of the Sudoku board.
	/// <para>Outputs to cout, clears and then populates from the class variables.</para>
	/// </summary>
	void print(bool lite = false) {
		std::stringstream ss;

		ss << std::endl;
		for (short row_index = 0; row_index < N + 1; row_index++) {
			if (row_index % 3 == 0)
				ss << "  +-------+-------+-------+" << std::endl;
			if (row_index >= N)
				break;
			ss << ' ';
			for (short col_index = 0; col_index < N; col_index++) {
				if (col_index % 3 == 0)
					ss << " |";
				ss << ' ';
				if (!lite)
					if (m_Board[col_index][row_index] > 0)
						ss << m_Board[col_index][row_index];
					else {
						int notes = m_Board[col_index][row_index].onlyNote();
						if (notes == 0)
							ss << '*';
						else if (notes > 0)
							ss << '~';
						else
							ss << '-';
					}
				else
					if (m_Board[col_index][row_index] > 0)
						ss << m_Board[col_index][row_index];
					else
						ss << '-';
			}
			ss << " |" << std::endl;
		}
		ss << std::endl;

		system("CLS");
		std::cout << ss.rdbuf();
	}

	/// <summary>Tests if the board is in a solved state.
	/// <para>Board must be valid, filled, and otherwise correct according to Sudoku rules.</para>
	/// <returns>Boolean denoting if the board is considered to be solved.</returns>
	/// </summary>
	bool isBoardSolved() {
		bool solved = true;
		for (short lineID = 0; lineID < N && solved; lineID++)
			solved = lineCorrect(lineID, false);
		for (short lineID = 0; lineID < N && solved; lineID++)
			solved = lineCorrect(lineID, true);
		for (short segmentID = 0; segmentID < N && solved; segmentID++)
			solved = segmentCorrect(segmentID);
		return solved;
	}

	/// <summary>Tests if the board is in a valid state.
	/// <para>Denotes if a board is valid, ignoring incomplete spaces, but checks taht existing ones are in bounds, and empty ones have at least one possible number noted to them.</para>
	/// <returns>Boolean denoting if the board is considered to valid.</returns>
	/// </summary>
	bool isBoardValid() {

		bool isValid = false;
		bool colValid = false;
		bool rowValid = false;
		int rowArray[N];
		int colArray[N];
		int colNumber = 0;
		int rowNumber = 0;


		for (int row_fill = 0; row_fill < 9; row_fill++) {
			rowNumber = row_fill;
			for (int col_fill = 0; col_fill < 9; col_fill++) {
				colArray[col_fill] = m_Board[col_fill][row_fill];
			}

			for (int row_check = 0; row_check < 9; row_check++) {
				for (int col_check = 0; col_check < 9; col_check++) {
					if (row_check != rowNumber) {
						if (colArray[col_check] == m_Board[col_check][row_check])
							colValid = false;
						else
							colValid = true;
					}
				}
			}
		} 

		for (int col_fill = 0; col_fill < 9; col_fill++) {
			colNumber = col_fill;
			for (int row_fill = 0; row_fill < 9; row_fill++) {
				rowArray[row_fill] = m_Board[col_fill][row_fill];
			}

			for (int col_check = 0; col_check < 9; col_check++) {
				for (int row_check = 0; row_check < 9; row_check++) {
					if (col_check != colNumber) {
						if (rowArray[row_check] == m_Board[col_check][row_check])
							rowValid = false;
						else
							rowValid = true;
					}
				}
			}
		}
		if (colValid && rowValid)
			isValid = true;

		// Check for any blank spots (no notes or values), or cells out of range (0 > cell > N(9))
		if (isValid) // Skip if we already know other things are invalid
			for (int row_check = 0; row_check < N; row_check++) { // For every row
				for (int col_check = 0; col_check < N; col_check++) { // For every col
					Cell cell = m_Board[col_check][row_check]; // Save the cell
					if (cell < 0 || cell > N) // Outside acceptable bounds?
						return false; // Oops
					if (cell == 0) // Empty cell?
						if (cell.onlyNote() <= -1) // No notes?
							return false; // Something is wrong
						
				}
			}
		return isValid;
	}

	/// <summary>Uses serial operations to solve the board by filling in any empty cells
	/// <para>Solves iteratively to determine if a board can be solved, and if so - how?</para>
	/// <returns>Boolean denoting if the board is solvable. Failed attemps return false upon realization of an impossible outcome (multiple or no solutions). No solution boards will have empty cells with no notes in them (cell with no value, or possible values)</returns>
	/// </summary>
	bool solve(short threads) {
		omp_set_num_threads(threads);

		solveNotational();
		print();

		if (m_State == State::IN_PROGRESS) {
			solveFromNotes();
			print();

			if (m_State == State::IN_PROGRESS) {
				solveBacktracking();
				print();

				return true;
			}
		}
		return false;
	}

	/// <summary> Uses a notational algorithm to fill cells logically by their derived values through a sophisticated process of elimination.
	/// <para> Solves a board for one iteration using a notational process.</para>
	/// </summary>
	void solveNotational() {
		int nThreads = omp_get_max_threads();
#pragma omp parallel
		{
			int threadID = omp_get_thread_num();

			for (short box = 0; box < N && m_State <= State::IN_PROGRESS; box += nThreads) { // For each segment...
				for (short numeral = threadID + 1; numeral <= N && m_State <= State::IN_PROGRESS; numeral++) { // For each number from 1 through 9...

					if (usedInSegment(box, numeral)) // If the number we're trying exists in the segment...
						continue; // Skip this segment, move to the next.

					for (short index = 0; index < N; index++) { // For each cell in the current segment...

						int row = (short)std::floor(box / 3) * 3 + (short)std::floor(index / 3); // Save the derrived row (bleh math)
						int col = (box % 3) * 3 + (index % 3); // Save the derrived column (bleh math)
						Cell cell = m_Board[col][row]; // Save the cell locally for faster testing

						if (cell > 0) // If the cell has a number in it already...
							continue; // Skip this cell, move to the next one

						if (usedInLine(row, numeral, col)) { // If the number exists in the same row already...
							index += 2 - (index % 3); // Skip to the next row (math just skips the index to the start of the next row of the segment (0,3,6)
							continue;
						}
						if (usedInLine(col, numeral, row, true)) // If the number exists in the same column already...
							continue; // Skip this cell, move to the next one

						int countValid = 0; // Keep track of how many spots our number can go within the segment

						// Keep track of the most recent valid rows & columns, so that if there's only one valid spot, we can go back and fill it in!
						short lastValidRow = -1;
						short lastValidCol = -1;

						for (short l_row = row - (row % 3); l_row <= row - (row % 3) + 2; l_row++) { // For each row in the segment...
							for (short l_col = col - (col % 3); l_col <= col - (col % 3) + 2; l_col++) { // For each column in the segment...
								if (m_Board[l_col][l_row] > 0) // If the spot is filled in already...
									continue; // Skip this spot, move to the next one

								if (checkIfSafe(l_col, l_row, numeral)) { // Is this empty cell safe for this number...?
									countValid++; // Add to the number of -per-segment valid spots for this numeral

									// Save this location for later!
									lastValidRow = l_row;
									lastValidCol = l_col;

									m_Board[lastValidCol][lastValidRow].note(numeral); // Add a note to this cell, letting the board remember that this numeral is possible for this position
								}

							} // End column loop
						} // End row loop

						if (countValid == 1) { // Did we find only one valid spot for this numeral...?
							m_Board[lastValidCol][lastValidRow] = numeral; // Recall the saved position, and set the number!
							wipeNotations(lastValidCol, lastValidRow, numeral); // Wipe respective notes now that the board's state has permenantly changed!
							if ((threadID == 0 || nThreads <= 1) && DO_DEBUG_PRINTING) // Main thread?
								print(); // Print out the updates
						}
						else // Multiple or no valid cells...?
							if (countValid == 0) // No valid spots for this number in the segment...?
								m_State = State::INVALID; // Board cannot be solved!! (We've found an empty cell that has no possible values for it)
							else
								break; // We've already made a note of them all... there's nothing more we can do here... :(
						box = -1; // Tell the segemnt loop to start from the top (-1 because box will be ++'d by the for loop)
						numeral = N + 1;
						break;
					}
				} // End segment loop
			} // End numeral loop
		} // End of parallel region
	}

	/// <summary> Uses the stores notes of the current board in an attempt to eliminate mutually-exclusive cells.
	/// <para> Has excessive runtime, but can potentially let a notational algorithm continue.</para>
	/// </summary>
	void solveFromNotes() {
		int nThreads = omp_get_max_threads();
#pragma omp parallel
		{
			int threadID = omp_get_thread_num();
			if (threadID == 0 || nThreads <= 1)
				m_State = (isBoardSolved() ? State::SOLVED : m_State);

			for (short l_col = threadID; l_col < N && m_State <= State::IN_PROGRESS; l_col += nThreads) {
				for (short l_row = 0; l_row < N && m_State <= State::IN_PROGRESS; l_row++) {
					if (m_Board[l_col][l_row] > 0)
						continue;
					short numeral = m_Board[l_col][l_row].onlyNote(); // Returns a number > 0 if only a single note exists
					if (numeral == 0) {
						for (short index = 0; index < N; index++)
							if (m_Board[l_col][l_row].canBe[index] == true)
								m_Board[l_col][l_row].canBe[index] = checkIfSafe(l_col, l_row, index + 1);
							else
								m_Board[l_col][l_row].canBe[index] = false;
						numeral = m_Board[l_col][l_row].onlyNote(); // Returns a number > 0 if only a single note exists
					}
					if (numeral > 0) {
						m_Board[l_col][l_row] = numeral;
						wipeNotations(l_col, l_row, numeral);
						//if ((threadID == 0 || nThreads <= 1) && DO_DEBUG_PRINTING)
						//	print(); // ENABLES "SLOWMODE"
					}
				}
			}
		}
	}

	/// <summary> Brute forces empty cells based off of available notes
	/// <para> Has excessive runtime, but can potentially succeed where notational fails.</para>
	/// <param name='index'> - Index of the cell to start from. Used for recursion to pickup where it left off.</param>
	/// </summary>
	bool solveBacktracking(short index = 0) {
		if (index > N * N + 1) // Base escape case, are we beyond the grid? (index 82)
			return true;
		short col = index % 9;						// Assign our column
		short row = (short)std::floor(index / 9);	// Assign our row
		if (m_Board[col][row] > 0)					// Is the cell already filled?
			return solveBacktracking(index + 1);	// Move on to the next cell...
		else
			for (short numeral = 1; numeral <= N; numeral++) {	// Loop through all possible numerals

				// TODO: Continue if the current numeral isn't listed on the notes of the cell
				// Or, just loop through all of the notes only to begin with.

				// Currently unimplemented because we don't trust our ability to reliably denote all notes.

				if (checkIfSafe(col, row, numeral)) {	// Can the current numeral go in the current location?
					m_Board[col][row] = numeral;		// Place the numeral, assume it's correct, we'll fix it in later otherwise
					if (DO_DEBUG_PRINTING)
						print(true);
					if (solveBacktracking(index + 1))	// Continue to the next cell, returns true only if the base case triggered on cell 82 (board solved)
						return true;			// Pass along the fact that the board was solved
					m_Board[col][row] = 0;		// Board was not solved, reset back to a safe state
				}
			}
		return false; // This cell hit a dead end, recursively backtrack and try something else...
	}
};