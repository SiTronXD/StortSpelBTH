local playerMesh = resources.addAnimations({ "assets/models/Character/CharIdle.fbx", "assets/models/Character/CharRun2.fbx", 
"assets/models/Character/CharDodge.fbx", "assets/models/Character/CharOutwardAttack.fbx", "assets/models/Character/CharHeavyAttack.fbx", 
"assets/models/Character/CharSpinAttack.fbx", "assets/models/Character/CharKnockbackAttack.fbx", 
"assets/models/Character/CharInwardAttack.fbx", "assets/models/Character/CharSlashAttack.fbx", 
"assets/models/Character/DeathAnim.fbx" }, "assets/textures/playerMesh")
resources.mapAnimations(playerMesh, {"idle", "run", "dodge", "lightAttack", "heavyAttack", 
"spinAttack", "knockback", "mixAttack", "slashAttack", "dead" })
resources.createAnimationSlot(playerMesh, "LowerBody", "mixamorig:Hips")
resources.createAnimationSlot(playerMesh, "UpperBody", "mixamorig:Spine1")

paused = false
settings = false

-- Camera
local cam = scene.createPrefab("scripts/prefabs/CameraPrefab.lua")
scene.setMainCamera(cam)

-- Player
playerID = scene.createEntity()
scene.setComponent(playerID, CompType.Mesh, playerMesh)
scene.setComponent(playerID, CompType.Animation, {})

scene.setComponent(playerID, CompType.Script, "scripts/Player.lua")
scene.setComponent(playerID, CompType.Collider, { type = ColliderType.Capsule, radius = 2, height = 10, offset = vector.new(0, 7.3, 0) })
scene.setComponent(playerID, CompType.Rigidbody, { mass = 1, gravityMult = 5, rotFactor = vector.fill(0), friction = 0.1 })
scene.getComponent(cam, CompType.Script).playerID = playerID

local player = scene.getComponent(playerID, CompType.Script)
player.camID = cam
player.playerMesh = playerMesh

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
scene.getComponent(uiID, CompType.Script).uiHpMidTexID = 
	resources.addTexture("assets/textures/UI/UIBarMid.png")
scene.getComponent(uiID, CompType.Script).uiHpSideTexID = 
	resources.addTexture("assets/textures/UI/UIBarSide.png")
scene.getComponent(uiID, CompType.Script).uiStamMidTexID = 
	resources.addTexture("assets/textures/UI/UIBarMid.png")
scene.getComponent(uiID, CompType.Script).uiStamSideTexID = 
	resources.addTexture("assets/textures/UI/UIBarSide.png")
scene.getComponent(uiID, CompType.Script).uiBorderTexID = 
	resources.addTexture("assets/textures/UI/UIBorder.png")
scene.getComponent(uiID, CompType.Script).pauseBackgroundTexID = 
	resources.addTexture("assets/textures/UI/frame.png")
scene.getComponent(uiID, CompType.Script).buttonTexID = 
	resources.addTexture("assets/textures/UI/button.png")

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
