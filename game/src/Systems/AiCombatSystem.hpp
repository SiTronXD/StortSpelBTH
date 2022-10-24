//#pragma once
//
//#include <vengine.h>
//#include "../Components/AiCombat.h"
//#include "../Components/AiMovement.h"
//
//class AiCombatSystem : public System
//{
//private:
//	Scene* scene;
//	SceneHandler* sceneHandler;
//	int playerID;
//public:
//	AiCombatSystem(SceneHandler* sceneHandler) : sceneHandler(sceneHandler), playerID(-1)
//	{
//		scene = sceneHandler->getScene();
//	}
//
//	bool update(entt::registry& reg, float dt) final
//	{
//		auto view = reg.view<AiCombat, AiMovement>();
//		auto foo = [&](AiCombat& combat, AiMovement& movement)
//		{
//			if (combat.timer > 0.f)
//			{
//				combat.timer -= dt;
//			}
//			else if (movement.distance <= 10.f)
//			{
//				attack(combat, movement);
//			}
//		};
//		view.each(foo);
//
//		return false;
//	}
//
//	void attack(AiCombat& combat, AiMovement& movement)
//	{
//		if (playerID == -1)
//		{
//			Entity mainCamID = scene->getMainCameraID();
//			sceneHandler->getScriptHandler()->getScriptComponentValue(
//				scene->getComponent<Script>(mainCamID), playerID, "playerID");
//		}
//
//		Combat& playerCombat = scene->getComponent<Combat>(playerID);
//		playerCombat.health -= 10.f;
//		std::cout << playerCombat.health << std::endl;
//	}
//};