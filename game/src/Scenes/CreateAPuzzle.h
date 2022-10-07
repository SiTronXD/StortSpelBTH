#pragma once

#include "vengine.h"
#include "../Puzzle.h"

class PuzzleCreator: public Scene {
  private:
    int                      camEntity;
    int                      ground;
    int                      mover;
    Puzzle                   puzzleObject; // in here we should only change one puzzel
    std::vector<int>         objectID;
    std::vector<std::string> ObjNames;
    std::string              puzzleName;

  public:
    PuzzleCreator();
    virtual ~PuzzleCreator();

    // Inherited via Scene
    virtual void init() override;
    virtual void update() override;
};
