#pragma once
#include "TheServerGame.h"

TheServerGame::TheServerGame() 
{

}

void TheServerGame::update(float dt) 
{
    static bool yes = false;
    if (players[0].position.z > 50 && !yes) {
        yes = true;
        for (int i = 0; i < 50; i++) {
            for (int y = 0; y < 10; y++) {
                serverEntities.push_back(
                    ServerEntity { glm::vec3(-125 + 5 * i, y * 10, 70), glm::vec3(0, 0, 0), 1 });
                addEvent({ (int)GameEvents::SpawnEnemy, 1 }, { (float)(-50 + 5 * i), 0.f, 70.f });
            }
        }
    }

    for (int i = 0; i < serverEntities.size(); i++) {
        serverEntities[i].rotation.x += dt * 5 * ((i * 0.25) + 1);
        if (serverEntities[i].rotation.x > 360) {
            serverEntities[i].rotation.x = 0;
        }
    }
}