#include "NetworkHandlerGame.h"
#include "../Systems/CombatSystem.hpp"
#include "ServerGameMode.h"
#include "../Scenes/GameScene.h"
#include "vengine/network/ServerEngine/Timer.h"

const float NetworkHandlerGame::UPDATE_RATE = ServerUpdateRate;

Entity NetworkHandlerGame::spawnItem(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir)
{
	Scene* scene = this->sceneHandler->getScene();
	Perks perk{ .multiplier = multiplier, .perkType = type };
	
	Entity e = scene->createEntity();
	scene->setComponent<MeshComponent>(e, perkMeshes[type]);
	
	Transform& perkTrans = sceneHandler->getScene()->getComponent<Transform>(e);
	perkTrans.position = pos;
	perkTrans.scale = glm::vec3(2.0f);
	scene->setComponent<Collider>(e, Collider::createSphere(4.0f, glm::vec3(0.0f), true));

	if (shootDir != glm::vec3(0.0f))
	{
		scene->setComponent<Rigidbody>(e);
		Rigidbody& perkRb = sceneHandler->getScene()->getComponent<Rigidbody>(e);
		perkRb.gravityMult = 6.0f;
		perkRb.velocity = shootDir * 20.0f;
	}

	scene->setComponent<Perks>(e, perk);
	scene->setComponent<PointLight>(e, glm::vec3(0.0f), glm::vec3(5.0f, 7.0f, 9.0f));

	return e;
}

Entity NetworkHandlerGame::spawnItem(AbilityType type, glm::vec3 pos, glm::vec3 shootDir)
{
	Scene* scene = this->sceneHandler->getScene();
	Entity e = scene->createEntity();
	scene->setComponent<MeshComponent>(e, abilityMeshes[type]);

	Transform& perkTrans = sceneHandler->getScene()->getComponent<Transform>(e);
	perkTrans.position = pos;
	perkTrans.scale = glm::vec3(2.0f);
	scene->setComponent<Collider>(e, Collider::createSphere(5.0f, glm::vec3(0.0f), true));

	if (shootDir != glm::vec3(0.0f))
	{
		scene->setComponent<Rigidbody>(e);
		Rigidbody& perkRb = sceneHandler->getScene()->getComponent<Rigidbody>(e);
		perkRb.gravityMult = 6.0f;
		perkRb.velocity = shootDir * 20.0f;
	}

	scene->setComponent<Abilities>(e, type);
	scene->setComponent<PointLight>(e, glm::vec3(0.0f), glm::vec3(7.0f, 9.0f, 5.0f));

	return e;
}

Entity NetworkHandlerGame::spawnEnemy(const int& type, const glm::vec3& pos) {
    int e = sceneHandler->getScene()->createEntity();
    switch (type)
	{
	case 0:
		// Load blob
		this->sceneHandler->getScene()->setScriptComponent(e, "scripts/loadBlob.lua");
		this->sceneHandler->getScene()->setComponent<SwarmComponent>(e);
		break;
	case 1:
		// Load lich
		this->sceneHandler->getScene()->setScriptComponent(e, "scripts/loadLich.lua");
		this->sceneHandler->getScene()->setComponent<LichComponent>(e);
		break;
	case 2:
		// Load tank
		this->sceneHandler->getScene()->setScriptComponent(e, "scripts/loadTank.lua");
		this->sceneHandler->getScene()->setComponent<TankComponent>(e);
		break;
	default:
		break;
	}
    return e;
}

void NetworkHandlerGame::initParticleSystems() {
    deletedParticleSystems = false;

    // Heal particle system
    ParticleSystem healPS{};
    healPS.maxlifeTime = 3.0f;
    healPS.numParticles = 32;
    healPS.textureIndex = resourceManger->addTexture("assets/textures/UI/HealingAbilityParticle.png");
    healPS.startSize = glm::vec2(1.7f);
    healPS.endSize = glm::vec2(0.3f);
    healPS.startColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    healPS.endColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
    healPS.velocityStrength = 10.0f;
    healPS.acceleration = glm::vec3(0.0f, -3.0f, 0.0f);
    healPS.coneSpawnVolume.diskRadius = 25.0f;
    healPS.coneSpawnVolume.coneAngle = 0.0f;
    healPS.coneSpawnVolume.localDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    healPS.coneSpawnVolume.localPosition = glm::vec3(0.0f, -0.5f, 0.0f);
    this->healParticleSystem.create(this->sceneHandler->getScene(), healPS, 1);

    // Blood particle system
    ParticleSystem bloodPS{};
    std::strcpy(bloodPS.name, "BloodPS");
    bloodPS.maxlifeTime = 0.7f;
    bloodPS.numParticles = 64;
    bloodPS.textureIndex = resourceManger->addTexture("assets/textures/bloodParticle.png");
    bloodPS.startSize = glm::vec2(0.4f);
    bloodPS.endSize = glm::vec2(0.0f);
    bloodPS.startColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    bloodPS.endColor = glm::vec4(0.2f, 0.0f, 0.0f, 0.0f);
    bloodPS.velocityStrength = 30.0f;
    bloodPS.acceleration = glm::vec3(0.0f, -30.0f, 0.0f);
    bloodPS.spawnRate = 0.01f;
    bloodPS.respawnSetting = RespawnSetting::EXPLOSION;
    bloodPS.coneSpawnVolume.diskRadius = 1.5f;
    bloodPS.coneSpawnVolume.coneAngle = 70.0f;
    bloodPS.coneSpawnVolume.localDirection = glm::vec3(0.0f, 0.0f, 1.0f);
    bloodPS.coneSpawnVolume.localPosition = glm::vec3(0.0f, 10.0f, 0.0f);
    this->bloodParticleSystems.create(this->sceneHandler->getScene(), bloodPS, 3);

    // Swarm particle system
    ParticleSystem swarmPS{};
    std::strcpy(swarmPS.name, "SwarmPS");
    swarmPS.maxlifeTime = 2.0f;
    swarmPS.numParticles = 8;
    swarmPS.textureIndex = resourceManger->addTexture("assets/textures/slimeParticle.png");
    swarmPS.startSize = glm::vec2(0.4f);
    swarmPS.endSize = glm::vec2(0.0f);
    swarmPS.startColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    swarmPS.endColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
    swarmPS.velocityStrength = 5.0f;
    swarmPS.acceleration = glm::vec3(0.0f, -7.0f, 0.0f);
    swarmPS.spawnRate = 0.01f;
    swarmPS.respawnSetting = RespawnSetting::EXPLOSION;
    swarmPS.coneSpawnVolume.diskRadius = 1.5f;
    swarmPS.coneSpawnVolume.coneAngle = 160.0f;
    swarmPS.coneSpawnVolume.localDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    swarmPS.coneSpawnVolume.localPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    this->swarmParticleSystems.create(this->sceneHandler->getScene(), swarmPS, 5);

    // Fotstep particle system
    this->sceneHandler->getScene()->setComponent<ParticleSystem>(this->player);
    ParticleSystem& footstepPS = this->sceneHandler->getScene()->getComponent<ParticleSystem>(this->player);
    footstepPS.maxlifeTime = 1.2f;
    footstepPS.numParticles = 12;
    footstepPS.textureIndex = resourceManger->addTexture("assets/textures/grassDustParticle.png");
    footstepPS.startSize = glm::vec2(0.0f);
    footstepPS.endSize = glm::vec2(1.7f);
    footstepPS.startColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    footstepPS.endColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    footstepPS.velocityStrength = 2.0f;
    footstepPS.acceleration = glm::vec3(0.0f, -0.5f, 0.0f);
    footstepPS.coneSpawnVolume.diskRadius = 3.0f;
    footstepPS.coneSpawnVolume.coneAngle = 0.0f;
    footstepPS.coneSpawnVolume.localDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    footstepPS.coneSpawnVolume.localPosition = glm::vec3(0.0f);
    footstepPS.spawn = false;

    // Portal particle system
    ParticleSystem portalPS0{};
    portalPS0.maxlifeTime = 3.0f;
    portalPS0.numParticles = 32;
    portalPS0.textureIndex = resourceManger->addTexture("assets/textures/portalParticle.png");
    portalPS0.startSize = glm::vec2(0.0f);
    portalPS0.endSize = glm::vec2(1.2f);
    portalPS0.startColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    portalPS0.endColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
    portalPS0.velocityStrength = 10.0f;
    portalPS0.acceleration = glm::vec3(0.0f, 2.0f, 0.0f);
    portalPS0.coneSpawnVolume.diskRadius = 15.0f;
    portalPS0.coneSpawnVolume.coneAngle = 0.0f;
    portalPS0.coneSpawnVolume.localDirection = glm::vec3(0.0f, 0.0f, 1.0f);
    portalPS0.coneSpawnVolume.localPosition = glm::vec3(0.0f, 5.0f, -30.0f);
    ParticleSystem portalPS1{};
    portalPS1 = portalPS0;
    portalPS1.coneSpawnVolume.localDirection.z *= -1.0f;
    portalPS1.coneSpawnVolume.localPosition.z *= -1.0f;
    this->portalParticleSystemSide0.create(this->sceneHandler->getScene(), portalPS0, 1);
    this->portalParticleSystemSide1.create(this->sceneHandler->getScene(), portalPS1, 1);
}

void NetworkHandlerGame::deleteInitialParticleSystems() {
	    // Only delete particle systems once
    if (this->deletedParticleSystems)
    {
        return;
    }
    this->deletedParticleSystems = true;

    this->healParticleSystem.removeEntities(this->sceneHandler->getScene());
    this->bloodParticleSystems.removeEntities(this->sceneHandler->getScene());
    this->swarmParticleSystems.removeEntities(this->sceneHandler->getScene());
    this->portalParticleSystemSide0.removeEntities(this->sceneHandler->getScene());
    this->portalParticleSystemSide1.removeEntities(this->sceneHandler->getScene());

    // Find all particle entities
    std::vector<Entity> particleEntities;
    particleEntities.reserve(32);
    auto pView =
        this->sceneHandler->getScene()->getSceneReg().view<Transform, ParticleSystem>();
    pView.each(
        [&](const auto entity,
            const Transform& transform,
            const ParticleSystem& particleSystem)
        {
            particleEntities.push_back((Entity) entity);
        }
    );
}

Entity NetworkHandlerGame::spawnHealArea(glm::vec3 pos)
{
	Scene* scene = this->sceneHandler->getScene();
	Entity heal = scene->createEntity();

	scene->setComponent<MeshComponent>(heal, this->healAreaMesh);
	scene->setComponent<PointLight>(heal, { glm::vec3(0.f, 10.f, 0.f), glm::vec3(9.f, 7.f, 9.f) });
	scene->setComponent<HealArea>(heal);
	scene->getComponent<Transform>(heal).position = pos;

	// Particle system
	scene->setComponent<ParticleSystem>(heal);
	ParticleSystem scenePartSys = getHealParticleSystem();
	scene->getComponent<ParticleSystem>(heal) = scenePartSys;

	return heal;
}

NetworkHandlerGame::~NetworkHandlerGame() {
    if (lich_fire != nullptr)
    {
		delete lich_fire;    
	}
    if (lich_ice != nullptr)
    {
        delete lich_ice;
    }
    if (lich_light != nullptr)
    {
		delete lich_light;    
	}
    this->cleanUp();
}

void NetworkHandlerGame::init()
{
    this->cleanUp();
    this->deletedParticleSystems = false;
	this->perkMeshes[0] = this->resourceManger->addMesh("assets/models/Perk_Hp.obj");
	this->perkMeshes[1] = this->resourceManger->addMesh("assets/models/Perk_Dmg.obj");
	this->perkMeshes[2] = this->resourceManger->addMesh("assets/models/Perk_AtkSpeed.obj");
	this->perkMeshes[3] = this->resourceManger->addMesh("assets/models/Perk_Movement.obj");
	this->perkMeshes[4] = this->resourceManger->addMesh("assets/models/Perk_Stamina.obj");
	this->abilityMeshes[0] = this->resourceManger->addMesh("assets/models/KnockbackAbility.obj");
	this->abilityMeshes[1] = this->resourceManger->addMesh("assets/models/Ability_Healing.obj");
	this->healAreaMesh = this->resourceManger->addMesh("assets/models/HealingAbility.obj");
	this->swordMesh = this->resourceManger->addMesh("assets/models/MainSword.fbx", "assets/textures");

    this->graveMesh = this->resourceManger->addMesh("assets/models/grave.obj");
    this->alterMesh = this->resourceManger->addMesh("assets/models/alter.obj");
    this->humpMesh = this->resourceManger->addMesh("assets/models/hump.obj");//TODO : ADD THE humpMesh!!!

	if (!TankComponent::s_initialized)
	{
		TankComponent::s_takeDmg =
			this->resourceManger->addSound("assets/Sounds/OufSound.ogg");
		TankComponent::s_initialized = true;
	}
	if (!LichComponent::s_initialized)
	{
		LichComponent::s_takeDmg =
			this->resourceManger->addSound("assets/Sounds/OufSound.ogg");
		LichComponent::s_lightning =
			this->resourceManger->addSound("assets/Sounds/OufSound.ogg");
		LichComponent::s_fire =
			this->resourceManger->addSound("assets/Sounds/OufSound.ogg");
		LichComponent::s_ice =
			this->resourceManger->addSound("assets/Sounds/OufSound.ogg");
		LichComponent::s_initialized = true;
	}
	if (!SwarmComponent::s_initialized)
	{
		SwarmComponent::s_takeDmg =
			this->resourceManger->addSound("assets/Sounds/OufSound.ogg");
		SwarmComponent::s_move =
			this->resourceManger->addSound("assets/Sounds/RunningSound.ogg");
		SwarmComponent::s_attack =
			this->resourceManger->addSound("assets/Sounds/SwishSound.ogg");
		SwarmComponent::s_initialized = true;
	}
	
	lich_fire = new LichAttack();
    lich_ice = new LichAttack();
    lich_light = new LichAttack();

    this->lich_fire->setStats(ATTACK_STRATEGY::FIRE);
    this->lich_ice->setStats(ATTACK_STRATEGY::ICE);
    this->lich_light->setStats(ATTACK_STRATEGY::LIGHT);
}

void NetworkHandlerGame::cleanUp()
{
    playerEntities.clear();
    swords.clear();
    playerPosLast.clear();
    playerPosCurrent.clear();
    serverEntities.clear();
    itemIDs.clear();
    entityToPosScale.clear();
    entityLastPosScale.clear();
}

int NetworkHandlerGame::getSeed()
{
	if (this->hasServer())
	{
		sf::Packet packet;
		packet << (int)GameEvent::SEED;
		this->sendDataToServerTCP(packet);
	}

	this->seed = -1;
    int tries = 0;
    Timer timer;
    static float maxWaitTimeForSeed = 3.5f; 
    float waitingTimeForSeed = 0;
	while (this->seed == -1 && tries < 4)
	{
		this->update();
        this->getClient()->update(timer.getDT());  //double force to get a send
        waitingTimeForSeed += timer.getDT();
        if (waitingTimeForSeed > maxWaitTimeForSeed)
        {
			sf::Packet packet;
			packet << (int)GameEvent::SEED;
			this->sendDataToServerTCP(packet);
            waitingTimeForSeed = 0;
            tries++;
            std::cout << "Client: asking for seed again" << std::endl;
		}
        timer.updateDeltaTime();
	}
	return this->seed;
}

void NetworkHandlerGame::setCombatSystem(CombatSystem* system)
{
	this->combatSystem = system;
}

void NetworkHandlerGame::setGhostMat(Material* ghostMat)
{
	this->ghostMat = ghostMat;
}

void NetworkHandlerGame::handleTCPEventClient(sf::Packet& tcpPacket, int event)
{
	sf::Packet packet;
	Scene* scene = this->sceneHandler->getScene();
	switch ((GameEvent)event)
	{
	case GameEvent::SEED:
		tcpPacket >> this->seed;
		break;
	case GameEvent::SPAWN_ITEM:
		tcpPacket >> i0 >> i1 >> i2 >> f0;
		v0 = this->getVec(tcpPacket);
		v1 = this->getVec(tcpPacket);
		if ((ItemType)i1 == ItemType::PERK)
		{
			this->itemIDs.push_back(this->spawnItem((PerkType)i2, f0, v0, v1));
		}
		else
		{
			this->itemIDs.push_back(this->spawnItem((AbilityType)i2, v0, v1));
		}
		break;
    case GameEvent::SPAWN_OBJECT:
         //id, type, position, rotation, scale 
         tcpPacket >> i0 >> i1;
         v0 = this->getVec(tcpPacket);
         v1 = this->getVec(tcpPacket);
         v2 = this->getVec(tcpPacket);

         this->serverEntities.insert({i0, spawnObject((ObjectTypes)i1, v0,v1,v2)});

        break;
    case GameEvent::SPAWN_GROUND_HUMP:
        tcpPacket >> i0;
        for(int i = 0; i < i0; i++){
            tcpPacket >> i1;
            serverEntities.insert({i1, createHump()});
        }
        break;
    case GameEvent::DO_HUMP:
        tcpPacket >> i0;
        v0 = this->getVec(tcpPacket);
        this->sceneHandler->getScene()->setActive(serverEntities[i0]);
        this->sceneHandler->getScene()->getComponent<Transform>(serverEntities[i0]).position = v0;

        break;
    case GameEvent::UPDATE_HUMP:
        tcpPacket >> i0;
        v0 = this->getVec(tcpPacket);
        this->sceneHandler->getScene()->getComponent<Transform>(serverEntities[i0]).scale = v0;

        break;
    case GameEvent::SET_POS_OBJECT:
         //id, position
         tcpPacket >> i0;
         v0 = this->getVec(tcpPacket);

        this->sceneHandler->getScene()->getComponent<Transform>(serverEntities[i0]).position = v0;
         
        break;
	case GameEvent::PICKUP_ITEM:
		// Index -> ItemType
		tcpPacket >> i0 >> i1;
		if (this->sceneHandler->getScene()->entityValid(this->itemIDs[i0]))
		{
			if ((ItemType)i1 == ItemType::PERK)
			{
				this->combatSystem->pickupPerk(this->itemIDs[i0]);
			}
			else
			{
				this->combatSystem->pickUpAbility(this->itemIDs[i0]);
			}
			std::swap(this->itemIDs[i0], this->itemIDs[this->itemIDs.size() - 1]);
			this->itemIDs.pop_back();
		}
		break;
	case GameEvent::DELETE_ITEM:
		// Index -> ItemType
		tcpPacket >> i0 >> i1;
		this->sceneHandler->getScene()->removeEntity(this->itemIDs[i0]);
		std::swap(this->itemIDs[i0], this->itemIDs[this->itemIDs.size() - 1]);
		this->itemIDs.pop_back();
		break;
	case GameEvent::USE_HEAL :
		v0 = this->getVec(tcpPacket);
		this->spawnHealArea(v0);
		break;
    case GameEvent::SPAWN_ENEMY: {
        tcpPacket >> i0 >> i1;
        v0 = this->getVec(tcpPacket);
		serverEntities.insert(std::pair<int, int>(i1, spawnEnemy(i0, v0)));
		//just create a enemy with interpolation
        entityToPosScale.insert({i0, std::pair<glm::vec3, glm::vec3>(v0, glm::vec3(1))});
        entityLastPosScale.insert({i0, std::pair<glm::vec3, glm::vec3>(v0, glm::vec3(1))});
		}
        break;
    case GameEvent::PUSH_PLAYER: // Can't confirm yet if this works
        tcpPacket >> i0; 
		if(i0 == ID)
		{
			v0 = this->getVec(tcpPacket);
			this->sceneHandler->getScene()->getComponent<Rigidbody>(player).velocity = v0;
			this->sceneHandler->getScriptHandler()->setScriptComponentValue(
			    this->sceneHandler->getScene()->getComponent<Script>(player),
			    1.0f,
			    "pushTimer"
			);
		}
        break;
	case GameEvent::PLAY_ENEMY_SOUND:
		tcpPacket >> i0 >> i1 >> i2 >> i3;
		if (i1 == 0)
		{
			if (i2 == 0)
			{
				
				this->sceneHandler->getAudioHandler()->playSound(serverEntities[i0], this->sceneHandler->getScene()->getComponent<SwarmComponent>(serverEntities[i0]).s_takeDmg, 30.f);
			}
			else if (i2 == 1)
			{
				// DEAL DAMAGE SOUND
				this->sceneHandler->getAudioHandler()->playSound(serverEntities[i0], this->sceneHandler->getScene()->getComponent<SwarmComponent>(serverEntities[i0]).s_attack, 30.f);
			}
			else if (i2 == 3)
			{
				// Move sound?
			}
		}		
		else if (i1 == 1)
		{
			if (i2 == 0)
			{
				this->sceneHandler->getAudioHandler()->playSound(serverEntities[i0], this->sceneHandler->getScene()->getComponent<TankComponent>(serverEntities[i0]).s_takeDmg, 30.f);
			}
			else if (i2 == 1)
			{
				// DEAL DAMAGE SOUND
				if (i3 == 0)
				{
					this->sceneHandler->getAudioHandler()->playSound(serverEntities[i0], this->sceneHandler->getScene()->getComponent<TankComponent>(serverEntities[i0]).s_shockwave, 30.f);
				}
				else if (i3 == 1)
				{
					this->sceneHandler->getAudioHandler()->playSound(serverEntities[i0], this->sceneHandler->getScene()->getComponent<TankComponent>(serverEntities[i0]).s_charge, 30.f);
				}
			}
			else if (i2 == 3)
			{
				// Move sound?
			}
		}
		else if (i1 == 2)
		{
			if (i2 == 0)
			{
				this->sceneHandler->getAudioHandler()->playSound(serverEntities[i0], this->sceneHandler->getScene()->getComponent<LichComponent>(serverEntities[i0]).s_takeDmg, 30.f);
			}
			else if (i2 == 1)
			{
				// DEAL DAMAGE SOUND
				if (i3 == 0)
				{
					this->sceneHandler->getAudioHandler()->playSound(serverEntities[i0], this->sceneHandler->getScene()->getComponent<LichComponent>(serverEntities[i0]).s_fire, 30.f);
				}
				else if (i3 == 1)
				{
					this->sceneHandler->getAudioHandler()->playSound(serverEntities[i0], this->sceneHandler->getScene()->getComponent<LichComponent>(serverEntities[i0]).s_lightning, 30.f);
				}
				else if (i3 == 2)
				{
					this->sceneHandler->getAudioHandler()->playSound(serverEntities[i0], this->sceneHandler->getScene()->getComponent<LichComponent>(serverEntities[i0]).s_ice, 30.f);
				}
			}
			else if (i2 == 3)
			{
				// Move sound?
			}
		}
		break;
    case GameEvent::PLAYER_SETHP:
        tcpPacket >> i0 >> i1;
        if (i0 == this->ID)
        {
			this->sceneHandler->getScene()->getComponent<HealthComp>(player).health = i1;   
		}
		break;
    case GameEvent::SPAWN_ORB:
        tcpPacket >> i0 >> i1;
        serverEntities.insert({i0,spawnOrbs(i1)});
        
		break;
    case GameEvent::THROW_ORB:
        tcpPacket >> i0;
        v0 = this->getVec(tcpPacket);
        v1 = this->getVec(tcpPacket);
        
        if(serverEntities.find(i0) != serverEntities.end())
        {
            this->sceneHandler->getScene()->setActive(serverEntities[i0]);
            this->sceneHandler->getScene()->getComponent<Transform>(serverEntities[i0]).position = v0;
            this->sceneHandler->getScene()->getComponent<Rigidbody>(serverEntities[i0]).velocity = v1;
            this->sceneHandler->getScene()->getComponent<Orb>(serverEntities[i0]).timeAtCast = Time::getTimeSinceStart();
        }

        
		break;
    case GameEvent::ENTITY_SET_HP:
        tcpPacket >> i0 >> i1;
		if (serverEntities.find(i0) != serverEntities.end())
        {
			int e = serverEntities.find(i0)->second;
			if (this->sceneHandler->getScene()->hasComponents<SwarmComponent>(e)) {
				this->sceneHandler->getScene()->getComponent<SwarmComponent>(e).life = i1;
			}
            else if(this->sceneHandler->getScene()->hasComponents<LichComponent>(e)) {
				this->sceneHandler->getScene()->getComponent<LichComponent>(e).life = i1;
			}
			else if(this->sceneHandler->getScene()->hasComponents<TankComponent>(e)) {
				this->sceneHandler->getScene()->getComponent<TankComponent>(e).life = i1;
			}
            
		}
		break;
    case GameEvent::ROOM_CLEAR:
        this->newRoomFrame = false;
        roomHandler->roomCompleted();
        this->numRoomsCleared++; // ++ on ptr ?
		std::cout << "GameScene: number of rooms cleared:" << this->numRoomsCleared << std::endl;  
		
		dynamic_cast<GameScene*>(scene)->revivePlayer();
		for (int i = 0; i < (int)this->playerEntities.size(); i++)
		{
			MeshComponent& mesh = scene->getComponent<MeshComponent>(this->playerEntities[i]);
			mesh.overrideMaterials[0] = this->origMat;
			mesh.overrideMaterials[0].tintColor = this->playerColors[i + 1];
			scene->setActive(this->swords[i]);
		}
        break;
    case GameEvent::NEXT_LEVEL:
        this->cleanUp();
        this->sceneHandler->setScene(
            new GameScene(
                ((GameScene*)this->sceneHandler->getScene())->setNewLevel()
            ),
            "scripts/gamescene.lua"
        );
		break;
    case GameEvent::PLAY_PARTICLE:
        tcpPacket >> i0 >> i1;
        if (serverEntities.find(i1) != serverEntities.end()){
			if (!this->sceneHandler->getScene()->hasComponents<ParticleSystem>(serverEntities[i1]))
			{
				this->sceneHandler->getScene()->setComponent<ParticleSystem>(serverEntities[i1]);
				ParticleSystem& partSys = this->sceneHandler->getScene()->getComponent<ParticleSystem>(serverEntities[i1]);
				partSys = getSwarmParticleSystem();
				partSys.spawn = true;
			}
		}
		break;
    case GameEvent::INACTIVATE:
        tcpPacket >> i0;
        if (serverEntities.find(i0) != serverEntities.end())
        {
            this->sceneHandler->getScene()->setInactive(serverEntities.find(i0)->second);
		}
		break;
    case GameEvent::ACTIVATE:
        tcpPacket >> i0;
        if (serverEntities.find(i0) != serverEntities.end())
        {
            this->sceneHandler->getScene()->setActive(serverEntities.find(i0)->second);
		}
		break;
	case GameEvent::UPDATE_ANIM:
		tcpPacket >> i0 >> i1 >> i2 >> i3;
		if (serverEntities.find(i0) != serverEntities.end())
		{
			if (i1 == 0) // Tank
			{
				str = i3 == 0 ? "LowerBody" : i3 == 1 ? "UpperBody" : "";
				if (i2 == 2) // GroundHump
				{
					this->sceneHandler->getScene()->blendToAnimation(serverEntities.find(i0)->second, this->tankAnims[i2], str, 0.18f, 1.09f);
				}
				else
				{
					this->sceneHandler->getScene()->blendToAnimation(serverEntities.find(i0)->second, this->tankAnims[i2], str);
				}
			}
			else // Lich
			{
				if (i2 == 1) // Attack
				{
					this->sceneHandler->getScene()->blendToAnimation(serverEntities.find(i0)->second, "Attack", "", 0.18f, 2.0f);
				}
				else
				{
					this->sceneHandler->getScene()->blendToAnimation(serverEntities.find(i0)->second, "Walk");
				}
			}
		}
		break;
	case GameEvent::UPDATE_ANIM_TIMESCALE:
		tcpPacket >> i0 >> i1 >> f0;
		if (serverEntities.find(i0) != serverEntities.end())
		{
			i2 = serverEntities.find(i0)->second;
			str = i1 == 0 ? "LowerBody" : i1 == 1 ? "UpperBody" : "";
			if (str == "UpperBody" && f0 == 0.0f) // Shield anim for tank (ugly fix due to latency)
			{
				this->sceneHandler->getScene()->blendToAnimation(i2, "RaiseShield", str, 0.18f, 0.0f);
				this->sceneHandler->getScene()->getAnimationSlot(i2, str).nTimer = 0.725f * 24.0f;
			}
			else
			{
				this->sceneHandler->getScene()->setAnimationTimeScale(i2, f0, str);
			}
		}
		break;
	case GameEvent::PLAYER_SET_GHOST:
		tcpPacket >> i0;
		i1 = -1;
		for (int i = 0; i < this->otherPlayersServerId.size(); i++)
		{
			if (this->otherPlayersServerId[i] == i0)
			{
				i1 = i;
				break;
			}
		}
		if (i1 != -1)
		{
			this->sceneHandler->getScene()->getComponent<MeshComponent>(this->playerEntities[i1]).overrideMaterials[0] = *this->ghostMat;
			this->sceneHandler->getScene()->setInactive(this->swords[i1]);
		}
		break;
	case GameEvent::END_GAME:
		((GameScene*)this->sceneHandler->getScene())->endGame();
		break;
	case GameEvent::CLOSE_OLD_DOORS:
		tcpPacket >> i0;
		roomHandler->multiplayerToggleCurrentDoors(i0);
		break;
	case GameEvent::CLOSE_NEW_DOORS:
		roomHandler->mutliplayerCloseDoors();
		break;
	default:
		break;
	}
}

void NetworkHandlerGame::handleUDPEventClient(sf::Packet& udpPacket, int event)
{
	sf::Packet packet;
	glm::vec3 vec;
	Transform* t;
	AnimationComponent* anim;
    this->timer = 0;
	switch ((GameEvent)event)
	{
	case GameEvent::UPDATE_PLAYER:
		udpPacket >> i0;
		i1 = -1;
		for (int i = 0; i < this->otherPlayersServerId.size(); i++)
		{
			if (this->otherPlayersServerId[i] == i0)
			{
				i1 = i;
				break;
			}
		}
		if (!this->playerEntities.size()) // Not ready
		{
			udpPacket.clear();
			break;
		}
		if (i1 == -1)
		{
			Log::warning("Client: invalid client id from server: " + std::to_string(i0));
			udpPacket.clear();
			break;
		}

		this->playerPosLast[i1] = this->playerPosCurrent[i1];
		this->playerPosCurrent[i1] = this->getVec(udpPacket);

		t = &this->sceneHandler->getScene()->getComponent<Transform>(this->playerEntities[i1]);
		t->rotation = this->getVec(udpPacket);

		anim = &this->sceneHandler->getScene()->getComponent<AnimationComponent>(this->playerEntities[i1]);
        udpPacket >> i0 >> anim->aniSlots[0].timer >> anim->aniSlots[0].timeScale >>
					 i1 >> anim->aniSlots[1].timer >> anim->aniSlots[1].timeScale;
		anim->aniSlots[0].animationIndex = (uint32_t)i0;
		anim->aniSlots[1].animationIndex = (uint32_t)i1;
		break;
    case GameEvent::UPDATE_MONSTER:
        // How many monsters we shall update
        udpPacket >> i0;
        for (int i = 0; i < i0; i++)
        {
			// The monster id on server side
			udpPacket >> i1;

			if (serverEntities.find(i1) == serverEntities.end())
            {
				udpPacket.clear();
                break;
			}
			// Get and set position and rotation
            v0 = getVec(udpPacket);
            v1 = getVec(udpPacket);
            v2 = getVec(udpPacket);
            entityLastPosScale[i1].first = entityToPosScale[i1].first;
			entityLastPosScale[i1].second = entityToPosScale[i1].second;
			entityToPosScale[i1].first = v0;
			entityToPosScale[i1].second = v2;
            
            sceneHandler->getScene()->getComponent<Transform>(serverEntities.find(i1)->second).rotation = v1;
		}
		break;
	default:
		break;
	}
}

void NetworkHandlerGame::handleTCPEventServer(Server* server, int clientIndex, sf::Packet& tcpPacket, int event)
{
	sf::Packet packet;
	ServerGameMode* serverScene;
	switch ((GameEvent)event)
	{
	case GameEvent::SEED:
		serverScene = server->getScene<ServerGameMode>();
		packet << (int)GameEvent::SEED << serverScene->getRoomSeed();
		server->sendToAllClientsTCP(packet);
		break;
	case GameEvent::SPAWN_ITEM:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> si0 >> si1 >> sf0;
		sv0 = this->getVec(tcpPacket);
		sv1 = this->getVec(tcpPacket);
		si2 = serverScene->spawnItem((ItemType)si0, si1, sf0);

		// event <- itemID <- ItemType <- otherType <- multiplier
		packet << (int)GameEvent::SPAWN_ITEM << si2 << si0 << si1 << sf0;
		this->sendVec(packet, sv0);
		this->sendVec(packet, sv1);
		server->sendToAllClientsTCP(packet);
		break;
	case GameEvent::PICKUP_ITEM:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> si0 >> si1 >> si2 >> sf0;
		serverScene->deleteItem(clientIndex, si0, (ItemType)si1, si2, sf0);
		break;

	case GameEvent::USE_HEAL:
		sv0 = this->getVec(tcpPacket);
		packet << (int)GameEvent::USE_HEAL;
		this->sendVec(packet, sv0);
		server->sendToAllClientsTCP(packet);
		break;
    case GameEvent::MONSTER_TAKE_DAMAGE:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> si0 >> si1 >> sf0;
		// Get how they should take damage
        if (serverScene->hasComponents<SwarmComponent>(si0))
        {
			si2 = (serverScene->getComponent<SwarmComponent>(si0).life -= si1);
			si3 = 0;
			si4 = 0;
			si5 = 0;
		}
        else if (serverScene->hasComponents<TankComponent>(si0))
        {
			si2 = (serverScene->getComponent<TankComponent>(si0).life -= si1);  
			si3 = 1;
			si4 = 0;
			si5 = 0;
		}
        else if (serverScene->hasComponents<LichComponent>(si0))
        {
			si2 = (serverScene->getComponent<LichComponent>(si0).life -= si1);
			si3 = 2;
			si4 = 0;
			si5 = 0;
		}
        if (serverScene->hasComponents<Rigidbody>(si0))
        {
			sv1 = serverScene->getComponent<Transform>(si0).position;
			sv2 = serverScene->getComponent<Transform>(serverScene->getPlayer(clientIndex)).position;
			sv0 = safeNormalize(sv2 - sv1);
            serverScene->getComponent<Rigidbody>(si0).velocity = glm::vec3(-sv0.x, 0.f, -sv0.z) * sf0;
        }
		
		packet << (int)GameEvent::PLAY_ENEMY_SOUND << si0 << si3 << si4 << si5;
		server->sendToAllClientsTCP(packet);
        
		break;
	case GameEvent::PLAYER_SET_GHOST:
		packet << (int)GameEvent::PLAYER_SET_GHOST << server->getClientID(clientIndex);
		server->sendToAllOtherClientsTCP(packet, clientIndex);
		break;
    case GameEvent::ROOM_CLEAR:
        this->newRoomFrame = false;
        roomHandler->roomCompleted();
		this->numRoomsCleared++;
		break;
	default:
		packet << event;
		server->sendToAllClientsTCP(packet);
		break;
	}
}

void NetworkHandlerGame::setRoomHandler(RoomHandler& roomHandler, int& numRoomsCleared)
{
    this->roomHandler = &roomHandler;
    this->numRoomsCleared = &numRoomsCleared;
    newRoomFrame = false;
}

void NetworkHandlerGame::handleUDPEventServer(Server* server, int clientIndex, sf::Packet& udpPacket, int event)
{
	sf::Packet packet;
	ServerGameMode* serverScene;
	switch ((GameEvent)event)
	{
	case GameEvent::UPDATE_PLAYER:
		serverScene = server->getScene<ServerGameMode>();
		packet << (int)GameEvent::UPDATE_PLAYER << server->getClientID(clientIndex);
		sv0 = this->getVec(udpPacket);
		this->sendVec(packet, sv0);
        
		serverScene->getComponent<Transform>(serverScene->getPlayer(clientIndex)).position = sv0;
		sv0 = this->getVec(udpPacket);
		this->sendVec(packet, sv0);

		udpPacket >> si0 >> sf0 >> sf1;
		packet << si0 << sf0 << sf1;
		udpPacket >> si0 >> sf0 >> sf1;
		packet << si0 << sf0 << sf1;

		udpPacket >> si0;
		serverScene->updatePlayerHp(clientIndex, si0);

		server->sendToAllOtherClientsUDP(packet, clientIndex);
		break;
	default:
		packet << event;
		server->sendToAllClientsUDP(packet);
		break;
	}
}

void NetworkHandlerGame::onDisconnect(int index)
{
	if (this->getStatus() == ServerStatus::RUNNING)
	{
		Entity ID = this->playerEntities[index];
		this->playerEntities.erase(this->playerEntities.begin() + index);
		this->sceneHandler->getScene()->removeEntity(ID);

		Entity swordID = this->swords[index];
		this->swords.erase(this->swords.begin() + index);
		this->sceneHandler->getScene()->removeEntity(swordID);
	}
}

void NetworkHandlerGame::sendHitOn(int entityID, int damage, float knockBack)
{
    if (this->isConnected())
    {
		for (auto it = serverEntities.begin(); it != serverEntities.end(); ++it)
		{
		    if (it->second == entityID)
		    {
				//need to send knock back
				sf::Packet p;
				p << (int)GameEvent::MONSTER_TAKE_DAMAGE << it->first << damage << knockBack;
				sendDataToServerTCP(p);
                return;
			}
		}
	}
	else
    {
		bool isEnemy = false;
		if (sceneHandler->getScene()->hasComponents<SwarmComponent>(entityID)) {
			this->sceneHandler->getAudioHandler()->playSound(entityID, SwarmComponent::s_takeDmg, 10.f);
			SwarmComponent& enemy = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
                  enemy.life -= damage;
                  isEnemy = true;
		}
		else if (sceneHandler->getScene()->hasComponents<TankComponent>(entityID)) {
			this->sceneHandler->getAudioHandler()->playSound(entityID, TankComponent::s_takeDmg, 10.f);
			TankComponent& enemy = sceneHandler->getScene()->getComponent<TankComponent>(entityID);
			enemy.life -= damage;
            isEnemy = true;
		}
        else if (sceneHandler->getScene()->hasComponents<LichComponent>(entityID)) {
			this->sceneHandler->getAudioHandler()->playSound(entityID, LichComponent::s_takeDmg, 10.f);
			LichComponent& enemy = sceneHandler->getScene()->getComponent<LichComponent>(entityID);
            enemy.life -= damage;
            isEnemy = true;
            std::cout << "LichWas HIT\n";
		}
		if (isEnemy)
        {
            Rigidbody& enemyRB = sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
			Transform& enemyTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
			Transform& playerTrans = sceneHandler->getScene()->getComponent<Transform>(player);
			glm::vec3 newDir = safeNormalize(playerTrans.position - enemyTrans.position);
			enemyRB.velocity = glm::vec3(-newDir.x, 0.f, -newDir.z) * knockBack;
		}
	}
}

void NetworkHandlerGame::setPlayerEntity(Entity player)
{
	this->player = player;
	MeshComponent& mesh = this->sceneHandler->getScene()->getComponent<MeshComponent>(this->player);
	this->resourceManger->makeUniqueMaterials(mesh);
	this->origMat = mesh.overrideMaterials[0];
}

void NetworkHandlerGame::createOtherPlayers(int playerMesh)
{
	int size = this->otherPlayersServerId.size();
	this->playerEntities.resize(size);
	this->swords.resize(size);
	this->playerPosLast.resize(size);
	this->playerPosCurrent.resize(size);
	float angle = 360.0f / (size + 1);

	Scene* scene = this->sceneHandler->getScene();
	Transform& playerTrans = scene->getComponent<Transform>(this->player);
	playerTrans.position = SMath::rotateVector(glm::vec3(0.0f, angle * (this->ID % (size + 1)), 0.0f), glm::vec3(15.0f, 12.0f, 0.0f));
	for (int i = 0; i < size; i++)
	{
		this->playerEntities[i] = scene->createEntity();
		scene->setComponent<MeshComponent>(this->playerEntities[i], playerMesh);
		scene->setComponent<AnimationComponent>(this->playerEntities[i]);
		scene->setComponent<Collider>(this->playerEntities[i], Collider::createCapsule(2, 10, glm::vec3(0, 7.3, 0)));

		// Sword
		this->swords[i] = scene->createEntity();
		scene->setComponent<MeshComponent>(this->swords[i], swordMesh);

		// Set Position
		Transform& t = scene->getComponent<Transform>(this->playerEntities[i]);
		t.position = playerTrans.position = SMath::rotateVector(glm::vec3(0.0f, angle * (this->otherPlayersServerId[i] % (size + 1)), 0.0f), glm::vec3(10.0f, 12.0f, 0.0f));

		// Set tint color
		MeshComponent& mesh = scene->getComponent<MeshComponent>(this->playerEntities[i]);
		this->resourceManger->makeUniqueMaterials(mesh);
		mesh.overrideMaterials[0].tintColor = this->playerColors[i + 1];
	}
}

void NetworkHandlerGame::updatePlayer()
{
	if (this->getClient()->getAccumulatedTime() + Time::getDT() >= UPDATE_RATE)
	{
		sf::Packet packet;
		Transform& t = this->sceneHandler->getScene()->getComponent<Transform>(this->player);
		AnimationComponent& anim = this->sceneHandler->getScene()->getComponent<AnimationComponent>(this->player);
		HealthComp& healthComp = this->sceneHandler->getScene()->getComponent<HealthComp>(this->player);

		packet << (int)GameEvent::UPDATE_PLAYER <<
			t.position.x << t.position.y << t.position.z <<
			t.rotation.x << t.rotation.y << t.rotation.z <<
			(int)anim.aniSlots[0].animationIndex << anim.aniSlots[0].timer << anim.aniSlots[0].timeScale <<
			(int)anim.aniSlots[1].animationIndex << anim.aniSlots[1].timer << anim.aniSlots[1].timeScale <<
			(int)healthComp.health;
		this->sendDataToServerUDP(packet);
	}
}

void NetworkHandlerGame::interpolatePositions()
{
	Scene* scene = this->sceneHandler->getScene();
	UIRenderer* UI = this->sceneHandler->getUIRenderer();
	this->timer += Time::getDT();

	float percent = this->timer / UPDATE_RATE;
	for (int i = 0; i < this->playerEntities.size(); i++)
	{
		Transform& t = scene->getComponent<Transform>(this->playerEntities[i]);
		t.position = this->playerPosLast[i] + percent * (this->playerPosCurrent[i] - this->playerPosLast[i]);
		UI->renderString(this->otherPlayers[i].second, t.position + glm::vec3(0.0f, 20.0f, 0.0f) + t.forward(), glm::vec2(150.0f));

		// Put sword in characters hand and keep updating it
		scene->getComponent<Transform>(this->swords[i]).setMatrix(
			this->resourceManger->getJointTransform(
				scene->getComponent<Transform>(this->playerEntities[i]),
				scene->getComponent<MeshComponent>(this->playerEntities[i]),
				scene->getComponent<AnimationComponent>(this->playerEntities[i]),
				"mixamorig:RightHand") * glm::translate(glm::mat4(1.f), glm::vec3(0.f, 1.f, 0.f)) *
			glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));
	}
	for (auto const& [key, val] : serverEntities)
	{
		Transform& t = scene->getComponent<Transform>(serverEntities[key]);
		if (entityToPosScale.find(key) != entityToPosScale.end() && sceneHandler->getScene()->hasComponents<Transform>(val))
		{
		    t.position = entityLastPosScale[key].first + percent * (entityToPosScale[key].first - entityLastPosScale[key].first);
			t.scale = entityLastPosScale[key].second + percent * (entityToPosScale[key].second - entityLastPosScale[key].second);
		}
	}
    
}

void NetworkHandlerGame::spawnItemRequest(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir)
{
	if (this->isConnected()) // Multiplayer (send to server)
	{
		sf::Packet packet;
		packet << (int)GameEvent::SPAWN_ITEM << (int)ItemType::PERK << type << multiplier;
		this->sendVec(packet, pos);
		this->sendVec(packet, shootDir);
		this->sendDataToServerTCP(packet);
	}
	else // Singleplayer
	{
		this->spawnItem(type, multiplier, pos, shootDir);
	}
}

void NetworkHandlerGame::spawnItemRequest(AbilityType type, glm::vec3 pos, glm::vec3 shootDir)
{
	if (this->isConnected()) // Multiplayer (send to server)
	{
		sf::Packet packet;
		packet << (int)GameEvent::SPAWN_ITEM << (int)ItemType::ABILITY << type << 0.0f;
		this->sendVec(packet, pos);
		this->sendVec(packet, shootDir);
		this->sendDataToServerTCP(packet);
	}
	else // Singleplayer
	{
		this->spawnItem(type, pos, shootDir);
	}
}

void NetworkHandlerGame::pickUpItemRequest(Entity itemEntity, ItemType type)
{
	if (this->isConnected()) // Multiplayer (send to server)
	{
		int index = -1;
		for (int i = 0; i < this->itemIDs.size(); i++)
		{
			if (this->itemIDs[i] == itemEntity)
			{
				index = i;
				break;
			}
		}
		sf::Packet packet;
		packet << (int)GameEvent::PICKUP_ITEM << index;

		Scene* scene = this->sceneHandler->getScene();
		if (scene->hasComponents<Perks>(itemEntity))
		{
			Perks& perk = scene->getComponent<Perks>(itemEntity);
			packet << (int)ItemType::PERK << perk.perkType << perk.multiplier;
			this->sendDataToServerTCP(packet);
		}
		else if (scene->hasComponents<Abilities>(itemEntity))
		{
			Abilities& ability = scene->getComponent<Abilities>(itemEntity);
			packet << (int)ItemType::ABILITY << ability.abilityType << 0.0f;
			this->sendDataToServerTCP(packet);
		}
	}
	else // Singleplayer
	{
		if (type == ItemType::PERK)
		{
			this->combatSystem->pickupPerk(itemEntity);
		}
		else
		{
			this->combatSystem->pickUpAbility(itemEntity);
		}
	}
}

void NetworkHandlerGame::useHealAbilityRequest(glm::vec3 position)
{
	if (this->isConnected()) // Multiplayer (send to server)
	{
		sf::Packet packet;
		packet << (int)GameEvent::USE_HEAL;
		this->sendVec(packet, position);
		this->sendDataToServerTCP(packet);
	}
	else // Singleplayer
	{
		this->spawnHealArea(position);
	}
}

void NetworkHandlerGame::setGhost()
{
	sf::Packet packet;
	packet << (int)GameEvent::PLAYER_SET_GHOST;
	this->sendDataToServerTCP(packet);
}

void NetworkHandlerGame::setPerks(const Perks perk[])
{
    
    Combat& combat = sceneHandler->getScene()->getComponent<Combat>(player);
	HealthComp& healthComp = sceneHandler->getScene()->getComponent<HealthComp>(player);
	for (size_t j = 0; j < 4; j++)
	{
		if (combat.perks[j].perkType == emptyPerk)
		{
			combat.perks[j] = perk[j];
			switch (combat.perks[j].perkType)
			{
			case hpUpPerk:
				this->combatSystem->updateHealth(combat, healthComp, combat.perks[j]);
				break;
			case dmgUpPerk:
				this->combatSystem->updateDmg(combat, combat.perks[j]);
				break;
			case attackSpeedUpPerk:
				this->combatSystem->updateAttackSpeed(combat, combat.perks[j]);
				break;
			case movementUpPerk:
				this->combatSystem->updateMovementSpeed(combat, combat.perks[j]);
				break;
			case staminaUpPerk:
				this->combatSystem->updateStamina(combat, combat.perks[j]);
				break;
			}
		}
	}
}

Entity NetworkHandlerGame::spawnOrbs(int orbType)
{
    static int fireOrb_mesh  = this->resourceManger->addMesh("assets/models/fire_orb.obj");
    static int lightOrb_mesh = this->resourceManger->addMesh("assets/models/light_orb.obj");
    static int iceOrb_mesh   = this->resourceManger->addMesh("assets/models/ice_orb.obj");

    Entity orb = this->sceneHandler->getScene()->createEntity();
    if (orbType == (int)ATTACK_STRATEGY::FIRE)
    {
        this->sceneHandler->getScene()->setComponent<MeshComponent>(orb, fireOrb_mesh);
        this->sceneHandler->getScene()->setComponent<Collider>(
            orb, Collider::createSphere(LichComponent::orbRadius)
        );
        this->sceneHandler->getScene()->setComponent<Orb>(orb);
        this->sceneHandler->getScene()->setComponent<Rigidbody>(orb);
        this->sceneHandler->getScene()->getComponent<Orb>(orb).orbPower = NetworkHandlerGame::lich_fire;
        Rigidbody& rb =
            this->sceneHandler->getScene()->getComponent<Rigidbody>(orb);
        rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
        rb.gravityMult = 0.0f;
        rb.friction = 3.0f;
        rb.mass = 10.0f;
    }
    else if (orbType == (int)ATTACK_STRATEGY::ICE)
    {
        this->sceneHandler->getScene()->setComponent<MeshComponent>(orb, iceOrb_mesh);
        this->sceneHandler->getScene()->setComponent<Collider>(
            orb, Collider::createSphere(LichComponent::orbRadius)
        );
        this->sceneHandler->getScene()->setComponent<Orb>(orb);
        this->sceneHandler->getScene()->setComponent<Rigidbody>(orb);
        this->sceneHandler->getScene()->getComponent<Orb>(orb).orbPower = NetworkHandlerGame::lich_ice;
        Rigidbody& rb =
            this->sceneHandler->getScene()->getComponent<Rigidbody>(orb);
        rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
        rb.gravityMult = 0.0f;
        rb.friction = 3.0f;
        rb.mass = 10.0f;
    }
    else if (orbType == (int)ATTACK_STRATEGY::LIGHT)
    {
        this->sceneHandler->getScene()->setComponent<MeshComponent>(orb, lightOrb_mesh);
        this->sceneHandler->getScene()->setComponent<Collider>(
            orb, Collider::createSphere(LichComponent::orbRadius)
        );
        this->sceneHandler->getScene()->setComponent<Orb>(orb);
        this->sceneHandler->getScene()->setComponent<Rigidbody>(orb);
        this->sceneHandler->getScene()->getComponent<Orb>(orb).orbPower = NetworkHandlerGame::lich_light;
        Rigidbody& rb =
            this->sceneHandler->getScene()->getComponent<Rigidbody>(orb);
        rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
        rb.gravityMult = 0.0f;
        rb.friction = 3.0f;
        rb.mass = 10.0f;
    }

    this->sceneHandler->getScene()->setInactive(orb);

    return orb;
}
Entity NetworkHandlerGame::spawnObject(
    const ObjectTypes& type, const glm::vec3& pos, const glm::vec3& rot,
    const glm::vec3& scale
)
{
    Entity entity = this->sceneHandler->getScene()->createEntity();
    switch (type)
        {
            case ObjectTypes::LICH_ALTER:
                this->sceneHandler->getScene()->setComponent<MeshComponent>(
                    entity, this->alterMesh
                );
                this->sceneHandler->getScene()->setComponent<Collider>(
                    entity,
                    Collider::createBox(glm::vec3{
                        LichComponent::alterWidth,
                        LichComponent::alterHeight,
                        LichComponent::alterDepth},
                        glm::vec3(0.f,LichComponent::alterHeight/2.f, 0.f)
                        )
                );

                break;
            case ObjectTypes::LICH_GRAVE:
                this->sceneHandler->getScene()->setComponent<MeshComponent>(
                    entity, this->graveMesh
                );
                this->sceneHandler->getScene()->setComponent<Collider>(
                    entity,
                    Collider::createBox(glm::vec3{
                        LichComponent::graveWidth,
                        LichComponent::graveHeight,
                        LichComponent::graveDepth},
                        glm::vec3(0.f,LichComponent::graveHeight/2.f, 0.f)
                        )
                );
                break;
            default:
                break;
        }
    this->sceneHandler->getScene()->getComponent<Transform>(entity).position =
        pos;
    this->sceneHandler->getScene()->getComponent<Transform>(entity).rotation =
        rot;
    this->sceneHandler->getScene()->getComponent<Transform>(entity).scale =
        scale;
    return entity;
}

Entity NetworkHandlerGame::createHump(){
    int e = sceneHandler->getScene()->createEntity();

    sceneHandler->getScene()->setComponent<MeshComponent>(e, humpMesh);
    sceneHandler->getScene()->setInactive(e);

    return e;
}