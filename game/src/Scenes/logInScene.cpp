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
        this->getResourceManager()->addTexture("assets/textures/blackTex.png"
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
        "@%        "},
        this->fontTextureId,
        glm::vec2(16,16)
    );

    int camEntity = this->createEntity();
    this->setComponent<Camera>(camEntity);
    this->setMainCamera(camEntity);

    nameButton.position = glm::vec2(0, 1080 / 4);
    nameButton.dimension = glm::vec2(1920, 1080/2);

    ipButton.position = glm::vec2(0, -1080 / 4);
    ipButton.dimension = glm::vec2(1920, 1080 / 2);

    startButton.position = glm::vec2(0.0f, -400.f);
    startButton.dimension = glm::vec2(275.0f, 100.0f);

    backButton.position = glm::vec2(-700.f, -400.f);
    backButton.dimension = glm::vec2(225.0f, 100.0f);

    title = this->getNetworkHandler()->hasServer() ? "create game" : "join game";
}

void logInScene::update()
{
    if (this->nameButton.isClicking())
    {
        nameOrIp = &name;
    }
    else if (this->ipButton.isClicking())
    {
        nameOrIp = &ipAddress;
    }
    if (this->startButton.isClicking() && this->name == "")
    {
        this->noName = true;
    }
    else if (this->startButton.isClicking())
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
            if (this->getNetworkHandler()->connectClient("192.168.1." + ipAddress))
            {
                this->getSceneHandler()->setScene(new LobbyScene()); 
            }
            else
            {
                this->incorrectIP = true;
            }
        }
    }
    if (this->backButton.isClicking())
    {
        this->getNetworkHandler()->deleteServer();
        this->getSceneHandler()->setScene(new MainMenu(), "scripts/MainMenu.lua");
    }

    // Button backtgrounds
    this->getUIRenderer()->setTexture(this->backgroundId);
    this->getUIRenderer()->renderTexture(
        this->backButton.position, this->backButton.dimension,
        glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, 0.1f + this->backButton.isHovering() * 0.15f));
    this->getUIRenderer()->renderTexture(
        this->startButton.position, this->startButton.dimension,
        glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, 0.1f + this->startButton.isHovering() * 0.15f));

    this->write();
    this->getUIRenderer()->setTexture(this->fontTextureId);
    this->getUIRenderer()->renderString(
        "name: ", glm::vec2(0.f, 50.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::RIGHT
    );
    this->getUIRenderer()->renderString(
        this->name, glm::vec2(0.f, 50.f), glm::vec2(50.f, 50.f), 0.0, StringAlignment::LEFT
    );

    this->getUIRenderer()->renderString(
        this->title, glm::vec2(0.f, 350.f), glm::vec2(125.f, 125.f), 0.0, StringAlignment::CENTER
    );

    if (!this->getNetworkHandler()->hasServer())
    {
        this->getUIRenderer()->renderString(
            "code: ", glm::vec2(0.f, -50.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::RIGHT
        );
        this->getUIRenderer()->renderString(this->ipAddress, glm::vec2(0.f, -50.f), glm::vec2(50.f, 50.f), 0.0, StringAlignment::LEFT);
    }

    this->getUIRenderer()->renderString(
        "start", this->startButton.position, glm::vec2(50.f, 50.f), 0.0f, StringAlignment::CENTER
    );
    this->getUIRenderer()->renderString(
        "back", this->backButton.position, glm::vec2(50.f, 50.f), 0.0f, StringAlignment::CENTER
    );

    if (this->noName)
    {
        this->getUIRenderer()->renderString(
            "no name provided", glm::vec2(0.0f, -200.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::CENTER, glm::vec4(1.0f, 0.1f, 0.1f, 1.0f)
        );
    }
    else if (this->incorrectIP)
    {
        this->getUIRenderer()->renderString(
            "code not valid", glm::vec2(0.0f, -200.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::CENTER, glm::vec4(1.0f, 0.1f, 0.1f, 1.0f)
        );
    }
}

void logInScene::write()
{
    std::string orig = *nameOrIp;
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

    if (this->noName && orig != *nameOrIp)
    {
        this->noName = false;
    }
    else if (this->incorrectIP && orig != *nameOrIp)
    {
        this->incorrectIP = false;
    }
}