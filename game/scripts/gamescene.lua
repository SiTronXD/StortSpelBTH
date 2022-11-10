--local playerMesh = resources.addMesh("assets/models/Amogus/source/1.fbx")
local playerMesh = resources.addAnimations({ "assets/models/Character/CharRun.fbx" }, "assets/textures/playerMesh")
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
scene.setComponent(playerID, CompType.Collider, { type = ColliderType.Capsule, radius = 2, height = 11, offset = vector.new(0, 7.3, 0) })
scene.setComponent(playerID, CompType.Rigidbody, { mass = 1, gravityMult = 5, rotFactor = vector.fill(0), friction = 0.1 })
scene.getComponent(cam, CompType.Script).playerID = playerID

local player = scene.getComponent(playerID, CompType.Script)
player.camID = cam
player.playerMesh = playerMesh
player.playerAttackMesh = playerAttackMesh

network.sendPlayer(player)

-- UI
local uiID = scene.createEntity()
scene.setComponent(uiID, CompType.Script, "scripts/UI.lua")
scene.getComponent(uiID, CompType.Script).playerScript = scene.getComponent(playerID, CompType.Script)

-- UI textures
local pixelArtSettings = {}
pixelArtSettings.samplerSettings = {}
pixelArtSettings.samplerSettings.filterMode = Filters.Nearest
scene.getComponent(uiID, CompType.Script).staminaBarBackgroundTxtID =
	resources.addTexture("assets/textures/UI/staminaBarBackground.png")
scene.getComponent(uiID, CompType.Script).staminaBarTxtID = 
	resources.addTexture("assets/textures/UI/staminaBar.png")
scene.getComponent(uiID, CompType.Script).perkSlotTextureID = 
	resources.addTexture("assets/textures/UI/perkSlot.png", pixelArtSettings)
scene.getComponent(uiID, CompType.Script).crosshairTextureID = 
	resources.addTexture("assets/textures/UI/crosshair.png", pixelArtSettings)

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
