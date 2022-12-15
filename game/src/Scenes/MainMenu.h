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
    Entity settingsEntity;

    Entity leaves;

    State state;
    uint32_t fontTextureId;
    uint32_t settingsBackgroundId;
    uint32_t howToPlayBackgroundId;
    uint32_t buttonTexture;
	uint32_t qrCodeID;
    uint32_t logoTextureId;
    uint32_t fogGradientVerticalTextureId;
    uint32_t fogGradientHorizontalTextureId;
    uint32_t fogTextureId;

    uint32_t buttonSound;

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

    void renderUiFogs(
        const float& gradientHorizontalPos,
        const float& gradientVerticalPos);

public:
    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};