#pragma once 

#include "vengine.h"

class NewAINetwork : public Scene
{
private:
  int camEntity;
  int player;

public:
  NewAINetwork();
  virtual ~NewAINetwork();

  // Inherited via Scene
  virtual void init() override;
  virtual void start() override;
  virtual void update() override;
};
