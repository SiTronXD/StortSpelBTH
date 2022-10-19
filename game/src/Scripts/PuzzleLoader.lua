package.path = package.path..";./../game/src/Scripts/?.lua"
require "Prefabs"

function loadPuzzle(filepath, offset) 
	local objectPadding = 3
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
				local newObject = Prefab:New()
				newObject:Copy(Prefabs[type])
			
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
					local xneg
					local zneg
					if(newObject.polyPoints[x].x ~= 0) then
						xneg = (newObject.polyPoints[x].x/math.abs(newObject.polyPoints[x].x)) * objectPadding
					end
					if(newObject.polyPoints[x].z ~= 0) then
						zneg = (newObject.polyPoints[x].z/math.abs(newObject.polyPoints[x].z)) * objectPadding
					end
					
					newObject.polyPoints[x] = newObject.polyPoints[x] + vector(xneg, yneg, zneg)

					--rotate the points
					newObject.polyPoints[x]:rotate(xrot, yrot, zrot)

					--move the points
					newObject.polyPoints[x] = newObject.polyPoints[x] + vector(xpos, ypos, zpos)

				end
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
