#pragma once

#include <cmath>
#include <string>

#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../World Handling/Room Handler.h"
#include "../World Handling/SpawnHandler.hpp"
#include "vengine.h"
#include "../World Handling/Room Handler.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../Ai/Behaviors/Tank/TankFSM.hpp"
#include "../Ai/Behaviors/Lich/LichFSM.hpp"

struct GameSceneLevel
{
    uint16_t level;
    Perks perks[4];
    Abilities ability;
    float hp;

    GameSceneLevel() {
        this->level = 10;
        Perks noPerk;
        noPerk.multiplier = 0.0;
        noPerk.perkType = PerkType::emptyPerk;
        for (int i = 0; i < 4; i++)
        {
            perks[i] = noPerk;
        }
        hp = 100.0f;
        ability.abilityType = AbilityType::emptyAbility;
    }
};

class NetworkHandlerGame;

class GameScene : public Scene
{
private:
    RoomHandler roomHandler;
    SpawnHandler spawnHandler;
    AIHandler* aiHandler = nullptr;
    NetworkHandlerGame* networkHandler;
    std::string levelString;

    const static float FADE_TIMER_DONE;

    // Timer that "stops" if paused
    Entity settingsEntity;
    float timer = 0.0f;
    bool paused = false;
    bool combatDisabled = false;
    float musicCounter = 0;
    UIArea resumeButton;
    UIArea settingsButton;
    UIArea howToPlayButton;
    UIArea exitButton;
    UIArea backButton;

    bool isGhost = false;
    bool hasRespawned = false;
    float ghostTransitionTimer = 0.0f;
    Material* ghostMat;
    Material origMat;

    float levelTimer;
    float portalTimer;
    float inPortalTimer;
    float fadeTimer;
    float deathTimer;
    bool isDead;
    bool end = false; // End game in multiplayer
    bool spawnPortal = false; // Spawn portal in multiplayer
  
    bool safetyCleanDone = false;
    uint32_t timeWhenEnteredRoom = 0;
    const uint32_t delayToSafetyDelete = 2;

    Entity playerID;
    Entity portal;
    Entity perk;
    Entity perk1;
    Entity perk2;
    Entity perk3;
    Entity perk4;
    Entity ability;
    Entity ability1;

    Entity dirLightEntity;

	bool newRoomFrame;
	bool portalActivated = false;

    int abilityMeshes[2];
    int perkMeshes[5];

    uint32_t abilityTextures[3];
    uint32_t perkTextures[6];
    uint32_t fontTextureIndex;
    uint32_t ghostOverlayIndex;
    uint32_t blackTextureIndex;

    uint32_t hpBarBackgroundTextureID;
    uint32_t hpBarTextureID;
    uint32_t portalOffMesh;
    uint32_t portalOnMesh;

    uint32_t buttonSound;
    float buttonListenTimer;

    GameSceneLevel currentLevel;

    void testParticleSystem(const Entity& particleSystemEntity);
    void setCurrentLevel(const GameSceneLevel& lvl);
  

public:
    GameScene(GameSceneLevel gameSceneLevel = GameSceneLevel());
    virtual ~GameScene();

    GameSceneLevel setNewLevel();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
    virtual void onTriggerStay(Entity e1, Entity e2) override;
    virtual void onTriggerEnter(Entity e1, Entity e2) override;
    virtual void onCollisionEnter(Entity e1, Entity e2) override;
    virtual void onCollisionStay(Entity e1, Entity e2) override;
    virtual void onCollisionExit(Entity e1, Entity e2) override;

    // Multiplayer
    void revivePlayer();
    void endGame();
    private:

    void imguiUpdate();
    void createPortal();

    int colliderTest;
};