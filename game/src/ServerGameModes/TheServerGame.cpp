#pragma once
#include "TheServerGame.h"

TheServerGame::TheServerGame() 
{

}

#include <iostream>
void TheServerGame::init() {

}

void TheServerGame::update(float dt) 
{
  static bool wentIn = false;
  if (this->getComponent<Transform>(this->getPlayer(0))
              .position.z > 0 &&
      !wentIn)
    {
      std::cout << "step over 1" << std::endl;
      wentIn = true;
      this->scene->createEnemy(1);
    }
  for (int i = 0; i < this->scene->getEnemySize(); i++)
    {
      this->scene->getComponent<Transform>(this->scene->getEnemies(i))
          .rotation += dt * 50 * (i + 1);
    }
    
}