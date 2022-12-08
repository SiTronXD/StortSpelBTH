#include "LoadScene.h"

void LoadScene::loadData() 
{
    std::vector<std::vector<std::string>> loadAnimations = {
            {"assets/models/Character/CharIdle.fbx",
              "assets/models/Character/CharRun.fbx",
              "assets/models/Character/CharDodge.fbx",
              "assets/models/Character/CharOutwardAttack.fbx",
              "assets/models/Character/CharHeavyAttack.fbx",
              "assets/models/Character/CharSpinAttack.fbx",
              "assets/models/Character/CharKnockbackAttack.fbx",
              "assets/models/Character/CharInwardAttack.fbx",
              "assets/models/Character/CharSlashAttack.fbx"},
    };
    std::vector<std::string> loadAnimationsTexture = {
        "assets/textures/playerMesh",
    };

    std::vector<std::string> loadCollisions = {

    };
    std::vector<std::string> loadMaterial = {

    };
    std::vector<std::string> loadMesh = {
        "assets/models/Menu/scene.obj",
        "assets/models/Menu/signpost.obj"
    };
    std::vector<std::string> loadSounds = {
        
    };
    std::vector<std::string> loadTexture = {
        
    };
    int thingsToLoad = 
        loadAnimations.size() + 
        loadCollisions.size() + 
        //loadMaterial.size() + 
        loadMesh.size() + 
        loadSounds.size() + 
        loadTexture.size();
    float oneLoadPercent = 100.f/thingsToLoad;

    for (int i = 0; i < loadAnimations.size(); i++)
    {
        this->getResourceManager()->addAnimations(loadAnimations[i], loadAnimationsTexture[i].c_str());
        this->howMuchDone += oneLoadPercent;
    }
    for (int i = 0; i < loadCollisions.size(); i++)
    {
        this->getResourceManager()->addCollisionShapeFromMesh(loadCollisions[i].c_str());
        this->howMuchDone += oneLoadPercent;
    }
    for (int i = 0; i < loadMesh.size(); i++)
    {
        this->getResourceManager()->addMesh(loadMesh[i].c_str());
        this->howMuchDone += oneLoadPercent;
    }
    for (int i = 0; i < loadSounds.size(); i++)
    {
        this->getResourceManager()->addSound(loadSounds[i].c_str());
        this->howMuchDone += oneLoadPercent;
    }
    for (int i = 0; i < loadTexture.size(); i++)
    {
        this->getResourceManager()->addTexture(loadTexture[i].c_str());
        this->howMuchDone += oneLoadPercent;
    }
    
}

LoadScene::~LoadScene() {
    loadThread.join();
}

void LoadScene::init() {
    int camEntity = this->createEntity();
    this->setComponent<Camera>(camEntity, 55.f);
    this->setMainCamera(camEntity);

    howMuchDone = 10.f;
    this->loadTexture = this->getResourceManager()->addTexture("assets/textures/ForestLoadMenu.jpg");
    this->loadBar = this->getResourceManager()->addTexture("assets/textures/UI/hpBar.png");
    this->background = this->getResourceManager()->addTexture("assets/textures/UI/hpBarBackground.png");
    this->loadThread = std::thread(&LoadScene::loadData, this);

}

void LoadScene::update() {
    if (Input::isKeyDown(Keys::D)) {
        howMuchDone += Time::getDT() * 10;
    }
    else if (Input::isKeyDown(Keys::A)) {
        howMuchDone -= Time::getDT() * 10;
    }

    if (howMuchDone >= 100)
    {
        this->switchScene(new MainMenu());    
    }

    this->getUIRenderer()->setTexture(background);
    this->getUIRenderer()->renderTexture(glm::vec2(0.f,0.f), glm::vec2(1920.f, 1920.f));

    this->getUIRenderer()->setTexture(loadTexture);
    this->getUIRenderer()->renderTexture(glm::vec2(0, 0), glm::vec2(697, 1080));

    this->getUIRenderer()->setTexture(loadBar);
    this->getUIRenderer()->renderTexture(glm::vec2(-1920/2.f + (19.2 * howMuchDone)/2, -530.0f), glm::vec2(19.2 * howMuchDone, 20.0f));
}
