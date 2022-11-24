local playerMesh = resources.addAnimations({ "assets/models/Menu/player_menu.fbx"}, "assets/textures/playerMesh")
resources.mapAnimations(playerMesh, {"idle"})

-- Camera
--local cam = scene.createPrefab("scripts/prefabs/CameraPrefab.lua")
--scene.setMainCamera(cam)

character = scene.createEntity()
scene.setComponent(character, CompType.Mesh, playerMesh)
local playerAnim = 
{ 
	timer = 0.0, 
	timeScale = 0.0
}
scene.setComponent(character, CompType.Animation, playerAnim)