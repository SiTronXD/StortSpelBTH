#pragma once
#include "TheServerGame.h"

TheServerGame::TheServerGame() 
{
  NavMesh::Polygon a;
  a.AddPoint(NavMesh::Point(-20, 50));
  a.AddPoint(NavMesh::Point(20, 50));
  a.AddPoint(NavMesh::Point(-20, 20));
  a.AddPoint(NavMesh::Point(20, 20));
  pfm.addPolygon(a);
}
#include <iostream>
void TheServerGame::update(float dt) 
{
    static bool yes = false;
    if (players[0].position.z > 1 && !yes) {
        yes = true;
        serverEntities.push_back(
            ServerEntity { glm::vec3(0, 0, 200), glm::vec3(0, 0, 0), 1 });
        addEvent({ (int)GameEvents::SpawnEnemy, 1 }, { 0.f, 0.f, 200.f});
    }

    for (int i = 0; i < serverEntities.size(); i++) {
        serverEntities[i].rotation.x += dt * 5 * ((i * 0.25) + 1);
        if (serverEntities[i].rotation.x > 360) {
            serverEntities[i].rotation.x = 0;
        }
        glm::vec3 dir =
            pfm.getDirTo(serverEntities[i].position, players[0].position) *
            (dt * 40);
        std::cout << dir.x << ", " << dir.z << std::endl;
        serverEntities[i].position += dir;
    }
    
}