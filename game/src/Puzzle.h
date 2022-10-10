#pragma once
#include "vengine.h"

struct Puzzle
{
  //objects
  int nrOfObjects;
  std::vector<Transform> transforms;  //have all the transforms'
  std::vector<std::string> objectNames;
  std::vector<int> mesh;  //give index to transforms what objectname they are
  //need to add there collision boxes here

  //the puzzels offset
  glm::vec2 offset = glm::vec2(0, 0);

  void addToScene(Scene* scene, std::map<std::string, int>& allMeshes, glm::vec2 offset)
  {
    for (int i = 0; i < this->nrOfObjects; i++)
      {
        //check if resourcemanager already has the mesh
        allMeshes.find(objectNames[i])->second;
  
        int t = scene->createEntity();
        scene->setComponent<MeshComponent>(t, 1);
        Transform tempTrans = transforms[i];
        tempTrans.position.x += offset.x;
        tempTrans.position.z += offset.y;
        scene->setComponent<Transform>(t, tempTrans);
      }
  }
  //save all the objects in vector
  void addToScene(Scene* scene, std::vector<int>& objects)
  {
    for (int i = 0; i < this->nrOfObjects; i++)
      {
        int t = scene->createEntity();
        objects.push_back(t);
        scene->setComponent<MeshComponent>(t, mesh[i]);
        Transform tempTrans = transforms[i];
        tempTrans.position.x += offset.x;
        tempTrans.position.z += offset.y;
        scene->setComponent<Transform>(t, tempTrans);
      }
  }
  void addToScene(Scene* scene)
  {
    for (int i = 0; i < this->nrOfObjects; i++)
      {
        int t = scene->createEntity();
        scene->setComponent<MeshComponent>(t, mesh[i]);
        Transform tempTrans = transforms[i];
        tempTrans.position.x += offset.x;
        tempTrans.position.z += offset.y;
        scene->setComponent<Transform>(t, tempTrans);
      }
  }
};