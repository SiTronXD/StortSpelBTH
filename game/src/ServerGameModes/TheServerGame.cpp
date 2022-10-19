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
        std::cout << "spawn enemies" << std::endl;
        serverEntities.push_back(
            ServerEntity { glm::vec3(-0.83, 0, 126), glm::vec3(0, 0, 0), 1 });
        addEvent({ (int)GameEvents::SpawnEnemy, 1 }, { -0.83f, 0.f, 126.f });
        serverEntities.push_back(
            ServerEntity { glm::vec3(-0.83, 0, 126), glm::vec3(0, 0, 0), 1 });
        addEvent({ (int)GameEvents::SpawnEnemy, 1 }, { -0.83f, 0.f, 126.f });
    }

    for (int i = 0; i < serverEntities.size(); i++) {
        serverEntities[i].rotation.x += dt * 5.f * ((i * 0.25f) + 1.f);
        if (serverEntities[i].rotation.x > 360) {
            serverEntities[i].rotation.x = 0;
        }
        glm::vec3 dir =
            pf.getDirTo(serverEntities[i].position, players[0].position) *
            (dt * 20);
        serverEntities[i].position += dir;
    }
    
}