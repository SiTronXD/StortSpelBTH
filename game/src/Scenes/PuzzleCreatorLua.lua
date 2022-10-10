local models

local prefab = { 
	Transform = { 
		rotation = vector(0, 45, -90),
		scale = vector.fill(5) 
	}, 
	Mesh = "vengine_assets/models/ghost.obj",
	--Script = "script.lua"
} 


local script = {} -- Script table

function script:update(dt)

	
	if(scene.hasComponent(scene.getMainCamera(), CompType.CameraMovement)) then
		--local camMove = scene.getComponent(scene.getMainCamera(), CompType.CameraMovement)
		--camMove.camDist = camMove.camDist + dt * 2
	else
		print("penis" .. tostring(dt) .. "\n")
	end

end

function script:init()

	--scene.setComponent(scene.getMainCamera(), CompType.CameraMovement);
	--scene.createSystem(SystemType.CameraMovementSystem, scene, scene.getMainCamera())
	for i=0,10 do
		prefab.Transform.position = vector(i*10,0,0)
		scene.createPrefab(prefab)
	end
	

end

return script -- Return script table
