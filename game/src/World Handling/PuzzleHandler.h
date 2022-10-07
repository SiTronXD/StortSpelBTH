#pragma once
#include "../Puzzle.h"

class PuzzleHandler
{
 private:
  std::vector<Puzzle> puzzles;


 public:
  PuzzleHandler();
  void loadAllPuzzles();
};