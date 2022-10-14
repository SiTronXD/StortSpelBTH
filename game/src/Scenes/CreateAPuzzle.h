#pragma once

#include "vengine.h"
#include <map>

class PuzzleCreator : public Scene
{
 private:

  int camEntity;
  int ground;
  int player;
  int puzzleCreator;
  int puzzleLoader;
  //int mover;
  //Puzzle puzzleObject;  // in here we should only change one puzzel
  //std::vector<int> objectID;
  //std::vector<std::string> objectNames;
  //std::vector<std::string> objectFilePath;
  //std::map<std::string, int> meshes;
  //std::string puzzleName;
  //int prefabSpawner;
  //int numberOfEntities;
  //
  //void getObjectFileNames();

 public:
  PuzzleCreator();
  virtual ~PuzzleCreator();

  // Inherited via Scene
  virtual void init() override;
  virtual void update() override;
};
