#pragma once
#include "path_finder.h"
#include <glm/vec3.hpp>
#include <iostream>

class PathFindingManager
{
 private:
  NavMesh::PathFinder pf;
  std::vector<NavMesh::Polygon> polygons;

 public:
  PathFindingManager();

  glm::vec3 getDirTo(glm::vec3 &from, glm::vec3 &to);
  void addPolygon(NavMesh::Polygon polygon);

  //don't use this all to often very heavy tasks
  void removePolygon(int id);
  void removeAllPolygons();

};
