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
        LevelEdit = 4,
        Quit = 5
    };

    Entity backgroundScene;
    Entity character;
    Entity camera;
    Entity signpost;
    Entity light;

    Entity leaves;

    State state;
    uint32_t fontTextureId;
    uint32_t settingsBackgroundId;
    uint32_t howToPlayBackgroundId;
    bool startGame;

    void howToPlay();
    void settings();

    //buttons
    //menu buttons
    int hostButton;
    int joinGameButton;
    int settingsButton;
    int levelEditButton;
    int singlePlayerButton;
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