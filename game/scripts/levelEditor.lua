--local playerMesh = resources.addMesh("assets/models/Amogus/source/1.fbx")
local playerMesh = resources.addAnimations({ "assets/models/Standard_Run.fbx" }, "assets/textures/playerMesh")
--local playerAttackMesh = resources.addMesh("assets/models/Hurricane Kick.fbx", "assets/textures/playerMesh")
print(playerMesh)

-- Camera
local cam = scene.createPrefab("scripts/prefabs/CameraPrefab.lua")
scene.setMainCamera(cam)

-- Player
playerID = scene.createEntity()
scene.setComponent(playerID, CompType.Mesh, playerMesh)
local playerAnim = 
{ 
	timer = 0.0, 
	timeScale = 0.0
}
scene.setComponent(playerID, CompType.Animation, playerAnim)
scene.setComponent(playerID, CompType.Script, "scripts/Player.lua")
scene.setComponent(playerID, CompType.Collider, { type = ColliderType.Capsule, radius = 2, height = 11 })
scene.setComponent(playerID, CompType.Rigidbody, { mass = 1, gravityMult = 5, rotFactor = vector.fill(0), friction = 0.1 })
scene.getComponent(cam, CompType.Script).playerID = playerID

local player = scene.getComponent(playerID, CompType.Script)
player.camID = cam
player.playerMesh = playerMesh
