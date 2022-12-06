#pragma once

#include "../vengine/vengine.h"

struct ParticleSystemInstance
{
	std::vector<Entity> particleSystemEntities;
	std::vector<ParticleSystem> particleSystems;
	uint32_t currentPsIndex = 0;

	void create(
		Scene* scene,
		const ParticleSystem& particleSystem, 
		const uint32_t& numParticleSystems)
	{
		this->particleSystemEntities.resize(numParticleSystems);
		this->particleSystems.resize(numParticleSystems);
		for (size_t i = 0; i < this->particleSystemEntities.size(); ++i)
		{
            this->particleSystemEntities[i] = scene->createEntity();
			scene->setComponent<ParticleSystem>(this->particleSystemEntities[i]);
            scene->getComponent<ParticleSystem>(this->particleSystemEntities[i]) = 
				particleSystem;
		}
	}

	void removeEntities(Scene* scene)
	{
		for (size_t i = 0; i < this->particleSystemEntities.size(); ++i)
		{
			// Extract component with correct indices
			this->particleSystems[i] =
				scene->getComponent<ParticleSystem>(this->particleSystemEntities[i]);

			// Remove
			scene->removeEntity(this->particleSystemEntities[i]);
		}
		this->particleSystemEntities.clear();
	}

	const ParticleSystem& getParticleSystem() 
	{
		this->currentPsIndex = (this->currentPsIndex + 1) % this->particleSystems.size();
		return this->particleSystems[this->currentPsIndex];
	}
};