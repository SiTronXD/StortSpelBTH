#pragma once

#include "vengine.h"

class MainMenu : public Scene
{
private:
    enum State : unsigned int
    { 
        Menu = 0, 
        Play = 1, 
        Settings = 2,
        HowToPlay = 3,
        Quit = 4
    };

    State state;
    uint32_t fontTextureId;
    uint32_t loadingTextureId;
    uint32_t backgroundId;
    bool startGame;

    void howToPlay();
    void settings();

public:
    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};