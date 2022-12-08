#pragma once

typedef int Entity;
class SceneHandler;

struct Settings
{
private:
	static Entity settingsEntity;
public:
	static SceneHandler* sceneHandler;
	static float volume;
	static float musicVolume;
	static bool fullScreen;
	static float sensitivity;
	static float cameraFOV;

	static void setEntity(Entity entity);
	static void updateValues();
};