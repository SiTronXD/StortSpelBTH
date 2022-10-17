package.path = package.path..";./../game/src/Scripts/?.lua"
require "Prefabs"

function loadPuzzle(filepath, offset) 
	local objectPadding = 1.0
		file = io.open("assets/puzzleParts/test.puz", "r")

		if(file ~= nil) then

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
				local newObject = Prefab:New()
				newObject:Copy(Prefabs[type])
				print("Mesh Type: " .. newObject.Mesh)
			
				--give entity data
				newObject.Transform.position.x = xpos + offset.x
				newObject.Transform.position.y = ypos + offset.y
				newObject.Transform.position.z = zpos + offset.z
				newObject.Transform.rotation.x = xrot
				newObject.Transform.rotation.y = yrot
				newObject.Transform.rotation.z = zrot
				newObject.Transform.scale.x = xscale
				newObject.Transform.scale.y = yscale
				newObject.Transform.scale.z = zscale
			
				--create it
				scene.createPrefab(newObject)

				--get its poly points for AI
				print(#newObject.polyPoints)
				print(newObject.polyPoints[1])
				for x = 1, #newObject.polyPoints do
					--scale the vectors
					newObject.polyPoints[x] = newObject.polyPoints[x] * vector(xscale, yscale, zscale)
					
					--add some padding
					local xneg = (newObject.polyPoints[x].x/math.abs(newObject.polyPoints[x].x)) * objectPadding
					local yneg = (newObject.polyPoints[x].y/math.abs(newObject.polyPoints[x].y)) * objectPadding
					local zneg = (newObject.polyPoints[x].z/math.abs(newObject.polyPoints[x].z)) * objectPadding
					
					newObject.polyPoints[x] = newObject.polyPoints[x] + vector(xneg, yneg, zneg)

					--rotate the points
					newObject.polyPoints[x]:rotate(xrot, yrot, zrot)

					--move the points
					print(newObject.polyPoints[x], " + ", vector(xpos, ypos, zpos))
					newObject.polyPoints[x] = newObject.polyPoints[x] + vector(xpos, ypos, zpos)
					print(newObject.polyPoints[x])

				end
								print("poly:")
				for x = 1, #newObject.polyPoints do
					print(newObject.polyPoints[x])
				end
				network.sendPolygons(newObject.polyPoints)

			
			end
			print("saved puz")
			io.close(file)
		else
			print("file doesn't exist")
		end
end
