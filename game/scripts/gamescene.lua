local ghost = resources.addMesh("assets/models/ghost.obj")
print(ghost)

--local cam = scene.createEntity()
--scene.setComponent(cam, CompType.Camera)
--scene.setMainCamera(cam)

-- Camera
local cam = scene.createPrefab("scripts/prefabs/CameraPrefab.lua")
scene.setMainCamera(cam)

-- Player
playerID = scene.createEntity()
scene.setComponent(playerID, CompType.Mesh, ghost)
scene.setComponent(playerID, CompType.Script, "scripts/Player.lua")
scene.getComponent(cam, CompType.Script).playerID = playerID
scene.getComponent(playerID, CompType.Script).camID = cam
network.sendPlayer(player)

-- UI
--local uiID = scene.createEntity()
--scene.setComponent(uiID, CompType.Script, "scripts/UI.lua")
--scene.getComponent(uiID, CompType.Script).hpBarTextureID = 
--	resources.addTexture("assets/textures/UI/hpBar.png")

--[[local p = scene.createPrefab("scripts/prefabs/prefab.lua")

local prefab = {
	Transform = {
		position = vector(3, 0, 5),
		rotation = vector(0, 45, -90),
		scale = vector.fill(1)
	},
	Mesh = 0,
	Script = "scripts/script.lua"
}
scene.createPrefab(prefab)

prefab.Transform = {
	position = vector(-3, 0, 5),
	rotation = vector(0, -45, -90),
	scale = vector.fill(1)
}
scene.createPrefab(prefab)]]

--scene.createSystem("scripts/system.lua")
