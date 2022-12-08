#include "LoadScene.h"
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

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
    int thingsToLoad = loadAnimations.size() + loadMesh.size() + loadCollisions.size() + 
    for (int i = 0; i < loadAnimations.size(); i++)
    {
        this->getResourceManager()->addAnimations(loadAnimations[i], loadAnimationsTexture[i].c_str());
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

    if (howMuchDone >= 100)
    {
        this->switchScene(new MainMenu());    
    }
}

void LoadScene::update() {
    if (Input::isKeyDown(Keys::D)) {
        howMuchDone += Time::getDT() * 10;
    }
    else if (Input::isKeyDown(Keys::A)) {
        howMuchDone -= Time::getDT() * 10;
    }

    this->getUIRenderer()->setTexture(background);
    this->getUIRenderer()->renderTexture(glm::vec2(0.f,0.f), glm::vec2(1920.f, 1920.f));

    this->getUIRenderer()->setTexture(loadTexture);
    this->getUIRenderer()->renderTexture(glm::vec2(0, 0), glm::vec2(697, 1080));

    this->getUIRenderer()->setTexture(loadBar);
    this->getUIRenderer()->renderTexture(glm::vec2(-1920/2.f + (19.2 * howMuchDone)/2, -530.0f), glm::vec2(19.2 * howMuchDone, 20.0f));
}
