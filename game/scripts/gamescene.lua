local playerMesh = resources.addMesh("assets/models/run_forward.fbx", "assets/textures/playerMesh")
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
	timeScale = 1.0 
}
scene.setComponent(playerID, CompType.Animation, playerAnim)
scene.setComponent(playerID, CompType.Script, "scripts/Player.lua")
scene.setComponent(playerID, CompType.Collider, { type = ColliderType.Capsule, radius = 2, height = 5 })
scene.setComponent(playerID, CompType.Rigidbody, { mass = 1, gravityMult = 5, rotFactor = vector.fill(0) })
scene.getComponent(cam, CompType.Script).playerID = playerID
scene.getComponent(playerID, CompType.Script).camID = cam
network.sendPlayer(player)

-- UI
local uiID = scene.createEntity()
scene.setComponent(uiID, CompType.Script, "scripts/UI.lua")
scene.getComponent(uiID, CompType.Script).playerScript = scene.getComponent(playerID, CompType.Script)

-- UI textures
local pixelArtSamplerSettings = { filterMode = Filters.Nearest }
scene.getComponent(uiID, CompType.Script).hpBarBackgroundTextureID =
	resources.addTexture("assets/textures/UI/hpBarBackground.png")
scene.getComponent(uiID, CompType.Script).hpBarTextureID = 
	resources.addTexture("assets/textures/UI/hpBar.png")
scene.getComponent(uiID, CompType.Script).perkSlotTextureID = 
	resources.addTexture("assets/textures/UI/perkSlot.png", pixelArtSamplerSettings)
scene.getComponent(uiID, CompType.Script).crosshairTextureID = 
	resources.addTexture("assets/textures/UI/crosshair.png", pixelArtSamplerSettings)

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
