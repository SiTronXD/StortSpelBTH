#pragma once

#include <glm/glm.hpp>
#include <stack>
#include "vengine.h"

#include "../HelperFuncs.hpp"
//#include "ai/PathFinding.h"


class LichBT : public BehaviorTree
{
   protected:
	// static int perkMeshes[3];
	virtual void start() = 0;
    void registerEntityComponents(Entity entityId) override;

  public:

   protected:

	static BTStatus attack(Entity entityID);

	static BTStatus playDeathAnim(Entity entityID);
	static BTStatus die(Entity entityID);
    static BTStatus alerted(Entity entityID);

};

class Lich_idle : public LichBT
{
   protected:
	void start();
  public:
};

class Lich_alerted : public LichBT
{
   protected:
	void start();
  public:

};

class Lich_combat : public LichBT
{
   protected:
	void start();

  public:	
};

class Lich_escape : public LichBT
{
   protected:
	void start();
  public:	
};

class Lich_dead: public LichBT
{
protected:
	void start();
public:	
};

class Lich_hunt: public LichBT
{
protected:
	void start();
public:	
};


class Lich_creep: public LichBT
{
protected:
	void start();
public:	
};
