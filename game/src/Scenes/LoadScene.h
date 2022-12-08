#pragma once
#include "vengine.h"
#include "MainMenu.h"
#include <thread>

class LoadScene : public Scene
{
private:
  std::thread loadThread;
  float howMuchDone;
  int loadTexture;
  int loadBar;
  int background;
  void loadData();
public:
  virtual ~LoadScene();
  virtual void init() override;
  virtual void update() override;
};