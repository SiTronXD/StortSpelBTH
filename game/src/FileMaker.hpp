#pragma once

#include <fstream>
#include "vengine.h"
const std::string PUZZLE_DIR = "../assets/puzzles/";

void createPuzzel(
    Scene* scene, std::vector<int>* objects,
    std::map<std::string, int> *meshesUsed, std::string puzzelName
)
{
  //std::cout << "creating puzzel file" << std::endl;
  //
  //std::ofstream wf(
  //    (PUZZLE_DIR + puzzelName + ".puz").c_str(),
  //    std::ios::out | std::ios::binary
  //);
  //if (!wf)
  //  {
  //    std::cout << "Cannot open/create file!" << std::endl;
  //    return;
  //  }
  //
  ////write amount of entities
  //int nrOfEntities = (int)objects->size();
  //wf.write((char*)&nrOfEntities, sizeof(int));
  //
  //for (size_t i = 0; i < nrOfEntities; i++)
  //  {
  //    //write every entity
  //    std::string meshName;
  //    for (auto it = meshesUsed.begin(); it != meshesUsed.end(); ++it)
  //      {
  //        if (it->second == objects->at(i))
  //          {
  //            meshName = it->first;
  //            continue;
  //          }
  //      }
  //    Transform tran = scene->getComponent<Transform>(objects->at(i));
  //    wf.write((char*)std::atoi(meshName.c_str()), sizeof(int));
  //    wf.write((char*)&tran, sizeof(Transform));
  //  }
  //
  //wf.close();
  //if (!wf.good())
  //  {
  //    std::cout << "Error occurred at writing time!" << std::endl;
  //    return;
  //  }
}
Puzzle readPuzzle(std::string puzzelName)
{
  Puzzle puzzle;
  //
  //std::ifstream rf(
  //    PUZZLE_DIR + puzzelName + ".puz", std::ios::out | std::ios::binary
  //);
  //if (!rf)
  //  {
  //    std::cout << "Cannot open file!" << std::endl;
  //    return puzzle;
  //  }
  //int nrOfEntities;
  //
  ////the first thing that we shall rescive is how many entities there are
  //rf.read((char*)&nrOfEntities, sizeof(int));
  //puzzle.nrOfObjects = nrOfEntities;
  //
  //for (int i = 0; i < nrOfEntities; i++)
  //  {
  //    std::string meshName;
  //    Transform tran;
  //    rf.read(
  //        (char*)&meshName, sizeof(int)
  //    );
  //    rf.read((char*)&tran, sizeof(Transform));
  //    puzzle.objectNames.push_back(meshName);
  //    puzzle.transforms.push_back(tran);
  //  }
  return puzzle;
}