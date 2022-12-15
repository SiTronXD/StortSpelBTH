local playerMesh = resources.addAnimations({ "assets/models/Menu/SittingIdle2.fbx"}, "assets/textures/playerMesh")
resources.mapAnimations(playerMesh, {"idle"})

character = scene.createEntity()
scene.setComponent(character, CompType.Mesh, playerMesh)
scene.setComponent(character, CompType.Animation, {})