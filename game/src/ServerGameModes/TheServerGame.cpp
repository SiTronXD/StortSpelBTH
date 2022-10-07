#pragma once
#include "TheServerGame.h"

TheServerGame::TheServerGame() 
{
  roomHandler.init(5, 4);
  roomHandler.generateRoom();
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
        serverEntities.push_back(ServerEntity{
            glm::vec3(20, 0, 200), glm::vec3(0, 0, 0), 1});
        addEvent({(int)GameEvents::SpawnEnemy, 1}, {20.f, 0.f, 200.f});
    }

    for (int i = 0; i < serverEntities.size(); i++) {
        serverEntities[i].rotation.x += dt * 5.f * ((i * 0.25f) + 1.f);
        if (serverEntities[i].rotation.x > 360) {
            serverEntities[i].rotation.x = 0;
        }
        glm::vec3 dir =
            pfm.getDirTo(serverEntities[i].position, players[0].position) *
            (dt * 20);
        std::cout << dir.x << ", " << dir.z << std::endl;
        serverEntities[i].position += dir;
    }
    
}