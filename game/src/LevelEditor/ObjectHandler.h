#pragma once
#include <unordered_map>
#include <vengine.h>
#include "vengine/shared_memory/mayaStructures.h"

class ObjectHandler
{
private:
  std::unordered_map<std::string, Entity> mayaObjects;

  MeshData createMeshData(char* data);
  void updateMeshData(MeshData* meshData, char* data);

public:
	ObjectHandler();
    ~ObjectHandler();
    
    void addNewMesh(Scene* scene, ResourceManager* resourceManager, char* data);
    void removeMesh(Scene* scene, char* data);
    void updateMeshTransform(Scene* scene, char* data);
    void updateMeshTopology(Scene* scene, ResourceManager* resourceManager, char* data);
    void addOrUpdateMaterial(Scene* scene, ResourceManager* resourceManager, char* data);
    void setMeshMaterial(Scene* scene, ResourceManager* resourceManager, char* data);
};