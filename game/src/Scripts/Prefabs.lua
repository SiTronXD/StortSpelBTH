require "Prefab"
--require ("../../../game/src/Scripts/Prefab")

--Meshes
m1 = resources.addMesh("assets/models/Cube.fbx")
m2 = resources.addMesh("assets/models/ghost.obj")

p_Cube = Prefab:New()
p_Cube.Mesh = m1
p_Cube.Name = "cube"
p_Cube.id = 1
p_Cube.polyPoints = {
	vector(-1,0,-1),
	vector(1,0,-1),
	vector(-1,0,1),
	vector(1,0,1),
}

p_Ghost = Prefab:New()
p_Ghost.Mesh = m2
p_Ghost.Name = "ghost"
p_Ghost.id = 2
p_Ghost.polyPoints = {
	vector(-1,0,-1),
	vector(1,0,-1),
	vector(-1,0,1),
	vector(1,0,1),
}

Prefabs = {
	p_Cube,
	p_Ghost
}
