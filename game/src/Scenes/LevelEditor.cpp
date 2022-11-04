#include "LevelEditor.h"

LevelEditor::LevelEditor() 
{
}

LevelEditor::~LevelEditor() 
{
  if (comlib)
    delete comlib;
  if (msg)
    delete msg;
  if (header)
    delete header;
}

void LevelEditor::init() 
{
  comlib = new Comlib(L"MayaBuffer", 150 * (1 << 20), Consumer);

  std::string playerName = "playerID";
  this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);

}

void LevelEditor::update() 
{

}