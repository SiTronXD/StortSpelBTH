require "Prefab"
--require ("../../../game/src/Scripts/Prefab")

--Meshes
m1 = resources.addMesh("vengine_assets/models/cube.obj")
m2 = resources.addMesh("vengine_assets/models/ghost.obj")

p_Cube = Prefab:New()
p_Cube.Mesh = m1
p_Cube.Name = "cube"
p_Cube.id = 1

p_Ghost = Prefab:New()
p_Ghost.Mesh = m2
p_Ghost.Name = "ghost"
p_Ghost.id = 2

Prefabs = {
	p_Cube,
	p_Ghost
}
