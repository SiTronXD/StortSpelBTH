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

    Entity backgroundScene;
    Entity character;
    Entity camera;

    State state;
    uint32_t fontTextureId;
    uint32_t backgroundId;
    bool startGame;

    void howToPlay();
    void settings();

    //buttons
    //menu buttons
    int playButton;
    int joinGameButton;
    int settingsButton;
    int quitButton;
    int howToPlayButton;

    //settings button
    int fullscreenButton;

    //general button
    int backButton;

public:
    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};