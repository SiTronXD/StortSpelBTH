#pragma once
#include "vengine.h"

struct Puzzle {
    int                        nrOfObjects;
    std::vector<Transform>     transforms;
    std::vector<MeshComponent> meshes;
    glm::vec2                  offset = glm::vec2(0,0);

    void addToScene(Scene* scene, glm::vec2 offset)
    {
        for (int i = 0; i < this->nrOfObjects; i++) {
            int t = scene->createEntity();
            scene->setComponent<MeshComponent>(t, meshes[i]);
            Transform tempTrans = transforms[i];
            tempTrans.position.x += offset.x;
            tempTrans.position.z += offset.y;
            scene->setComponent<Transform>(t, tempTrans);
        }
    }
    //save all the objects in vector
    void addToScene(Scene* scene, std::vector<int>& objects) 
    {
        for (int i = 0; i < this->nrOfObjects; i++) {
            int t = scene->createEntity();
            objects.push_back(t);
            scene->setComponent<MeshComponent>(t, meshes[i]);
            Transform tempTrans = transforms[i];
            tempTrans.position.x += offset.x;
            tempTrans.position.z += offset.y;
            scene->setComponent<Transform>(t, tempTrans);
        }
    }
    void addToScene(Scene* scene)
    {
        for (int i = 0; i < this->nrOfObjects; i++) {
            int t = scene->createEntity();
            scene->setComponent<MeshComponent>(t, meshes[i]);
            Transform tempTrans = transforms[i];
            tempTrans.position.x += offset.x;
            tempTrans.position.z += offset.y;
            scene->setComponent<Transform>(t, tempTrans);
        }
    }
};