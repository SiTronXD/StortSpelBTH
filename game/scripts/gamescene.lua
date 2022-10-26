local ghost = resources.addMesh("assets/models/ghost.obj")
print(ghost)

--local cam = scene.createEntity()
--scene.setComponent(cam, CompType.Camera)
--scene.setMainCamera(cam)

local cam = scene.createPrefab("scripts/prefabs/CameraPrefab.lua")
scene.setMainCamera(cam)

playerID = scene.createEntity()
scene.setComponent(playerID, CompType.Mesh, ghost)
scene.setComponent(playerID, CompType.Script, "scripts/Movement.lua")
scene.getComponent(cam, CompType.Script).playerID = playerID
scene.getComponent(playerID, CompType.Script).camID = cam
network.sendPlayer(player)

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
