#pragma once

#include "vengine.h"
#include <fstream>

void createPuzzel(Scene* scene, std::vector<int>* objects, std::string puzzelName)
{
    std::cout << "creating puzzel file" << std::endl;

    std::ofstream wf((puzzelName + ".puz").c_str(), std::ios::out | std::ios::binary);
    if (!wf) {
        std::cout << "Cannot open/create file!" << std::endl;
        return;
    }

    //write amount of entities
    int nrOfEntities = objects->size();
    wf.write((char*)&nrOfEntities, sizeof(int));

    for (size_t i = 0; i < nrOfEntities; i++) {
        //write every entity
        MeshComponent mesh = scene->getComponent<MeshComponent>(objects->at(i));
        Transform     tran = scene->getComponent<Transform>(objects->at(i));
        wf.write((char*)&mesh, sizeof(MeshComponent));
        wf.write((char*)&tran, sizeof(Transform));
    }

    wf.close();
    if (!wf.good()) {
        std::cout << "Error occurred at writing time!" << std::endl;
        return;
    }
}
Puzzle readPuzzle(std::string puzzelName)
{
    Puzzle puzzle;

    std::ifstream rf(puzzelName + ".puz", std::ios::out | std::ios::binary);
    if (!rf) {
        std::cout << "Cannot open file!" << std::endl;
        return puzzle;
    }
    int nrOfEntities;
    //the first thing that we shall rescive is how many entities there are
    rf.read((char*)&nrOfEntities, sizeof(int));
    puzzle.nrOfObjects = nrOfEntities;

    for (int i = 0; i < nrOfEntities; i++) {
        MeshComponent mesh;
        Transform     tran;
        rf.read((char*)&mesh, sizeof(MeshComponent));
        rf.read((char*)&tran, sizeof(Transform));
        puzzle.meshes.push_back(mesh);
        puzzle.transforms.push_back(tran);
    }
    return puzzle;
}