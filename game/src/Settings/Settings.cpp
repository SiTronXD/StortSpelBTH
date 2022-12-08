#include "Settings.h"
#include "vengine/application/SceneHandler.hpp"

SceneHandler* Settings::sceneHandler = nullptr;
Entity Settings::settingsEntity = -1;
float Settings::volume = 0.5f;
float Settings::musicVolume = 0.5f;
bool Settings::fullScreen = false;
float Settings::sensitivity = 25.0f;
float Settings::cameraFOV = 90.0f;

void Settings::setEntity(Entity entity)
{
	Scene* scene = sceneHandler->getScene();
	if (scene->entityValid(entity) && scene->hasComponents<Script>(entity))
	{
		settingsEntity = entity;
		Script& script = scene->getComponent<Script>(settingsEntity);
		ScriptHandler* scriptHandler = sceneHandler->getScriptHandler();

		scriptHandler->setScriptComponentValue(script, volume, "volume");
		scriptHandler->setScriptComponentValue(script, musicVolume, "musicVolume");
		scriptHandler->setScriptComponentValue(script, fullScreen, "fullScreen");
		scriptHandler->setScriptComponentValue(script, sensitivity, "sensitivity");
		scriptHandler->setScriptComponentValue(script, cameraFOV, "cameraFOV");
	}
}

void Settings::updateValues()
{
	Scene* scene = sceneHandler->getScene();
	if (scene->entityValid(settingsEntity) && scene->hasComponents<Script>(settingsEntity))
	{
		Script& script = scene->getComponent<Script>(settingsEntity);
		ScriptHandler* scriptHandler = sceneHandler->getScriptHandler();
		AudioHandler* audioHandler = sceneHandler->getAudioHandler();
		bool lastFullscreenStatus = fullScreen;

		scriptHandler->getScriptComponentValue(script, volume, "volume");
		scriptHandler->getScriptComponentValue(script, musicVolume, "musicVolume");
		scriptHandler->getScriptComponentValue(script, fullScreen, "fullScreen");
		scriptHandler->getScriptComponentValue(script, sensitivity, "sensitivity");
		scriptHandler->getScriptComponentValue(script, cameraFOV, "cameraFOV");

		audioHandler->setMasterVolume(volume);
		audioHandler->setMusicVolume(musicVolume);

		if (lastFullscreenStatus != fullScreen)
		{
			sceneHandler->getWindow()->setFullscreen(fullScreen);
		}

		if (scene->entityValid(scene->getMainCameraID()) && scene->hasComponents<Script>(scene->getMainCameraID()))
		{
			Script& cameraScript = scene->getComponent<Script>(scene->getMainCameraID());
			scriptHandler->setScriptComponentValue(cameraScript, sensitivity, "sens");

			Camera* camera = scene->getMainCamera();
			if (camera->fov != cameraFOV)
			{
				camera->fov = cameraFOV;
				camera->calculateProjectionMatrix(camera->aspectRatio, camera->nearPlane, camera->farPlane);
			}
		}
	}
}
