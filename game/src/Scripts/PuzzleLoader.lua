package.path = package.path..";./../game/src/Scripts/?.lua"
require "Prefabs"

local script = {} -- Script table

function script:load() 

		file = io.open("assets/puzzleParts/test.puz", "r")
		io.input(file)
		
		local nrOfEntity = tonumber(io.read())
		print(nrOfEntity);
		
		for i = 1, nrOfEntity do
			local xpos =	tonumber(io.read())
			local ypos =	tonumber(io.read())
			local zpos =	tonumber(io.read())
			local xrot =	tonumber(io.read())
			local yrot =	tonumber(io.read())
			local zrot =	tonumber(io.read())
			local xscale =	tonumber(io.read())
			local yscale =	tonumber(io.read())
			local zscale =	tonumber(io.read())
			local type =	math.floor(tonumber(io.read()))
		
			--load the prefab
			print("type: ".. type .. " Name: " .. Prefabs[type].Mesh)
			local newObject = Prefab:New(Prefabs[type])
			print("Mesh Type: " .. newObject.Mesh)
		
			--give entity data
			newObject.Transform.position.x = xpos
			newObject.Transform.position.y = ypos
			newObject.Transform.position.z = zpos
			newObject.Transform.rotation.x = xrot
			newObject.Transform.rotation.y = yrot
			newObject.Transform.rotation.z = zrot
			newObject.Transform.scale.x = xscale
			newObject.Transform.scale.y = yscale
			newObject.Transform.scale.z = zscale
		
			--create it
			scene.createPrefab(newObject)
		
		end
		print("saved puz")
		io.close(file)

end

function script:init()
	--check some kind of table here instead
	local puzzleName =  "assets/puzzleParts/test.puz"
	
	self:load()
end

return script -- Return script table