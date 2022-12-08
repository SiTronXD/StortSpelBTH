#include "logInScene.h"
#include "LobbyScene.h"
#include "MainMenu.h"

logInScene::logInScene() {}
logInScene::~logInScene() {}

void logInScene::start()
{
    selected = nullptr;

    TextureSamplerSettings samplerSettings{};
    samplerSettings.filterMode = vk::Filter::eNearest;
    samplerSettings.unnormalizedCoordinates = VK_TRUE;

    this->backgroundId =
        this->getResourceManager()->addTexture("assets/textures/blackTex.png"
        );
    this->buttonId =
        this->getResourceManager()->addTexture("assets/textures/UI/button.png");

    this->fontTextureId = Scene::getResourceManager()->addTexture(
        "assets/textures/UI/font.png", {samplerSettings, true}
    );
    this->buttonSound = this->getResourceManager()->addSound("assets/Sounds/buttonClick.ogg");

    Scene::getUIRenderer()->setBitmapFont(
        {"abcdefghij",
        "klmnopqrst",
        "uvwxyz+-.'",
        "0123456789",
        "!?,<>:()#^",
        "@%        "},
        this->fontTextureId,
        glm::vec2(50,50)
    );

    int camEntity = this->createEntity();
    this->setComponent<Camera>(camEntity);
    this->setMainCamera(camEntity);

    nameButton.position = glm::vec2(0.0f, 50.0f);
    nameButton.dimension = glm::vec2(600.0f, 75.0f);

    ipButton.position = glm::vec2(0.0f, -50.0f);
    ipButton.dimension = glm::vec2(600.0f, 75.0f);

    startButton.position = glm::vec2(0.0f, -400.f);
    startButton.dimension = glm::vec2(275.0f, 100.0f);

    backButton.position = glm::vec2(-700.f, -400.f);
    backButton.dimension = glm::vec2(225.0f, 100.0f);

    title = this->getNetworkHandler()->hasServer() ? "create" : "join";
}

void logInScene::update()
{
    if (this->nameButton.isClicking())
    {
        this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
        selected = &name;
    }
    else if (this->ipButton.isClicking() && !this->getNetworkHandler()->hasServer())
    {
        this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
        selected = &ipAddress;
    }
    else if (Input::isMouseButtonPressed(Mouse::LEFT))
    {
        selected = nullptr;
    }

    if (this->startButton.isClicking() && this->name == "")
    {
        this->noName = true;
    }
    else if (this->startButton.isClicking())
    {
        this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
        this->getNetworkHandler()->createClient(name);
        if (this->getNetworkHandler()->hasServer())
        {
            if (this->getNetworkHandler()->connectClientToThis())
            {
                std::string str = sf::IpAddress::getLocalAddress().toString();
                this->getSceneHandler()->setScene(new LobbyScene(str.substr(10)));
            }
            else
            {
                this->getNetworkHandler()->deleteServer();
                this->getSceneHandler()->setScene(new MainMenu(), "scripts/MainMenu.lua");
            }
        }
        else
        {
            std::string startAddress = (ipAddress.find(".") != std::string::npos) ? "" : "192.168.1.";
            if (this->getNetworkHandler()->connectClient(startAddress + ipAddress))
            {
                this->getSceneHandler()->setScene(new LobbyScene(ipAddress));
            }
            else
            {
                this->incorrectIP = true;
            }
        }
    }
    if (this->backButton.isClicking())
    {
        this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
        this->getNetworkHandler()->deleteServer();
        this->getSceneHandler()->setScene(new MainMenu(), "scripts/MainMenu.lua");
    }

    // Input fields
    this->getUIRenderer()->setTexture(this->backgroundId);
    this->getUIRenderer()->renderTexture(
        this->nameButton.position, this->nameButton.dimension,
        glm::uvec4(0, 0, 1, 1), 
        glm::vec4(1.0f, 1.0f, 1.0f, 0.1f + (this->nameButton.isHovering() || (this->selected == &this->name)) * 0.15f));

    if (!this->getNetworkHandler()->hasServer())
    {
        this->getUIRenderer()->renderTexture(
            this->ipButton.position, this->ipButton.dimension,
            glm::uvec4(0, 0, 1, 1), 
            glm::vec4(1.0f, 1.0f, 1.0f, 0.1f + (this->ipButton.isHovering() || (this->selected == &this->ipAddress)) * 0.15f));
    }

    // Button backgrounds
    this->getUIRenderer()->setTexture(this->buttonId);
    this->getUIRenderer()->renderTexture(
        this->backButton.position, this->backButton.dimension,
        glm::uvec4(0, 0, 1, 1),
        glm::vec4(1.0f, 1.0f, 1.0f, 0.85f + this->backButton.isHovering() * 0.15f
        )
    );
    this->getUIRenderer()->renderTexture(
        this->startButton.position, this->startButton.dimension,
        glm::uvec4(0, 0, 1, 1),
        glm::vec4(1.0f, 1.0f, 1.0f, 0.85f + this->startButton.isHovering() * 0.15f
        )
    );

    if (this->selected)
    {
        this->write();
    }

    this->getUIRenderer()->renderString(
        this->title + " game", glm::vec2(0.f, 350.f), glm::vec2(125.f, 125.f), 0.0, StringAlignment::CENTER
    );
    if (this->name == "" && this->selected != &this->name)
    {
        this->getUIRenderer()->renderString(
            "name", glm::vec2(-275.0f, 50.f), glm::vec2(40.0f), 0.0f, StringAlignment::LEFT, glm::vec4(glm::vec3(1.0f), 0.5f)
        );
    }
    else
    {
        this->getUIRenderer()->renderString(
            this->name, glm::vec2(-275.0f, 50.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::LEFT
        );
    }

    if (!this->getNetworkHandler()->hasServer())
    {
        if (this->ipAddress == "" && this->selected != &this->ipAddress)
        {
            this->getUIRenderer()->renderString(
                "address", glm::vec2(-275.0f, -50.f), glm::vec2(40.0f), 0.0f, StringAlignment::LEFT, glm::vec4(glm::vec3(1.0f), 0.5f)
            );
        }
        else
        {
            this->getUIRenderer()->renderString(this->ipAddress, glm::vec2(-275.0f, -50.f), glm::vec2(50.f, 50.f), 0.0f, StringAlignment::LEFT);
        }
    }

    this->getUIRenderer()->renderString(
        title, this->startButton.position, glm::vec2(50.f, 50.f), 0.0f, StringAlignment::CENTER
    );
    this->getUIRenderer()->renderString(
        "back", this->backButton.position, glm::vec2(50.f, 50.f), 0.0f, StringAlignment::CENTER
    );

    if (this->noName)
    {
        this->getUIRenderer()->renderString(
            "no name provided", glm::vec2(0.0f, -150.f), glm::vec2(40.0f), 0.0f, StringAlignment::CENTER, glm::vec4(1.0f, 0.1f, 0.1f, 1.0f)
        );
    }
    else if (this->incorrectIP)
    {
        this->getUIRenderer()->renderString(
            "address not valid", glm::vec2(0.0f, -150.f), glm::vec2(40.0f), 0.0f, StringAlignment::CENTER, glm::vec4(1.0f, 0.1f, 0.1f, 1.0f)
        );
    }

}

void logInScene::write()
{
    std::string orig = *selected;
    if (Input::isKeyPressed(Keys::BACK) && selected->length() > 0)
    {
        selected->pop_back();
    }
    if (orig.length() <= 12)
    {
        for (int i = 97; i < 123; i++)
        {
            if (Input::isKeyPressed((Keys)i))
            {
                *selected += i;
            }
        }
        for (int i = 48; i < 58; i++)
        {
            if (Input::isKeyPressed(Keys(i)))
            {
                *selected += i;
            }
        }
        if (Input::isKeyPressed(Keys::DOT))
        {
            *selected += 46;
        }
        else if (Input::isKeyPressed(Keys::SPACE))
        {
            *selected += 32;
        }
    }
    if (this->noName && orig != *selected)
    {
        this->noName = false;
    }
    else if (this->incorrectIP && orig != *selected)
    {
        this->incorrectIP = false;
    }
}