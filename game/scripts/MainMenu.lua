local playerMesh = resources.addAnimations({ "assets/models/Menu/SittingIdle2.fbx"}, "assets/textures/playerMesh")
resources.mapAnimations(playerMesh, {"idle"})

-- Camera
--local cam = scene.createPrefab("scripts/prefabs/CameraPrefab.lua")
--scene.setMainCamera(cam)

character = scene.createEntity()
scene.setComponent(character, CompType.Mesh, playerMesh)
scene.setComponent(character, CompType.Animation, {})