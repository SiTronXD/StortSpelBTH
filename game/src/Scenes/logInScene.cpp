#include "logInScene.h"
#include "LobbyScene.h"
#include "MainMenu.h"

logInScene::logInScene() {}
logInScene::~logInScene() {}

void logInScene::start()
{
    nameOrIp = &name;

    TextureSamplerSettings samplerSettings{};
    samplerSettings.filterMode = vk::Filter::eNearest;
    samplerSettings.unnormalizedCoordinates = VK_TRUE;

    this->backgroundId =
        this->getResourceManager()->addTexture("assets/textures/UI/background.png"
        );

    this->fontTextureId = Scene::getResourceManager()->addTexture(
        "assets/textures/UI/testBitmapFont.png", {samplerSettings, true}
    );

    Scene::getUIRenderer()->setBitmapFont(
        {"abcdefghij",
        "klmnopqrst",
        "uvwxyz+-.'",
        "0123456789",
        "!?,<>:()#^",
        "@         "},
        this->fontTextureId,
        glm::vec2(16,16)
    );

    int camEntity = this->createEntity();
    this->setComponent<Camera>(camEntity);
    this->setMainCamera(camEntity);

    this->nameButton = this->createEntity();
    this->ipButton = this->createEntity();
    this->joinStartButton = this->createEntity();
    this->backButton = this->createEntity();

    UIArea area{};
    area.position = glm::vec2(0, 1080 / 4);
    area.dimension = glm::vec2(1920, 1080/2);
    this->setComponent<UIArea>(nameButton, area);

    area.position = glm::vec2(0, -1080 / 4);
    area.dimension = glm::vec2(1920, 1080 / 2);
    this->setComponent<UIArea>(ipButton, area);

    area.position = glm::vec2(660.f, -400.f);
    area.dimension = glm::vec2(5*50, 50);
    this->setComponent<UIArea>(joinStartButton, area);

    area.position = glm::vec2(-660.f, -400.f);
    area.dimension = glm::vec2(5*50, 50);
    this->setComponent<UIArea>(backButton, area);
}

void logInScene::update()
{
    if (this->getComponent<UIArea>(nameButton).isClicking())
    {
        nameOrIp = &name;
    }
    else if (this->getComponent<UIArea>(ipButton).isClicking())
    {
        nameOrIp = &ipAddress;
    }
    if (this->getComponent<UIArea>(joinStartButton).isClicking())
    {
        this->getNetworkHandler()->createClient(name);
        if (this->getNetworkHandler()->hasServer())
        {
            if (this->getNetworkHandler()->connectClientToThis())
            {
                this->getSceneHandler()->setScene(new LobbyScene());
            }
            else
            {
                this->getNetworkHandler()->deleteServer();
                this->getSceneHandler()->setScene(new MainMenu(), "scripts/MainMenu.lua");
            }
        
        }
        else
        {
            if (this->getNetworkHandler()->connectClient(ipAddress))
            {
                this->getSceneHandler()->setScene(new LobbyScene()); 
            }
            else
            {
                this->getSceneHandler()->setScene(new MainMenu(), "scripts/MainMenu.lua");
            }
        }
    }
    if (this->getComponent<UIArea>(backButton).isClicking())
    {
        if (this->getNetworkHandler()->hasServer())
        {
            this->getNetworkHandler()->deleteServer();  
        }
        this->getSceneHandler()->setScene(new MainMenu(), "scripts/MainMenu.lua");
    }

    this->write();
    this->getUIRenderer()->setTexture(this->fontTextureId);
    this->getUIRenderer()->renderString(
        "name: ", glm::vec2(0.f, 50.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::RIGHT
    );
    this->getUIRenderer()->renderString(
        this->name, glm::vec2(0.f, 50.f), glm::vec2(50.f, 50.f), 0.0, StringAlignment::LEFT
    );

    if (!this->getNetworkHandler()->hasServer())
    {
        this->getUIRenderer()->renderString(
            "ip: ", glm::vec2(0.f, -50.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::RIGHT
        );
        this->getUIRenderer()->renderString(this->ipAddress, glm::vec2(0.f, -50.f), glm::vec2(50.f, 50.f), 0.0, StringAlignment::LEFT);
    }

    this->getUIRenderer()->renderString(
        "start", glm::vec2(660.f, -400.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::RIGHT
    );
    this->getUIRenderer()->renderString(
        "back", glm::vec2(-660.f, -400.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::RIGHT
    );
}

void logInScene::write()
{
    for (int i = 97; i < 123; i++)
    {
        if (Input::isKeyPressed((Keys)i))
        {
            *nameOrIp += i;
        }
    }
    for (int i = 48; i < 58; i++)
    {
        if (Input::isKeyPressed(Keys(i)))
        {
            *nameOrIp += i;
        }
    }
    if (Input::isKeyPressed(Keys::DOT))
    {
        *nameOrIp += 46;
    }
    else if(Input::isKeyPressed(Keys::SPACE))
    {
        *nameOrIp += 32;
    }
    else if (Input::isKeyPressed(Keys::BACK) && nameOrIp->length() > 0)
    {
        nameOrIp->pop_back();
    }
}