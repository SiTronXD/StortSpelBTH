package.path = package.path..";./../game/src/Scripts/?.lua"
require "Prefabs"


function xVectorRotation(theVector, xrot)
	local theReturn = vector(0,0,0)
	theReturn.x = theVector.x
	theReturn.y = theVector.y * math.cos(xrot) - theVector.z * math.sin(xrot)
	theReturn.z = theVector.y * math.sin(xrot) + theVector.z * math.cos(xrot)
	return theReturn
end
function yVectorRotation(theVector, yrot)
	local theReturn = vector(0,0,0)
	theReturn.x = theVector.x * math.cos(yrot) + theVector.z * math.sin(yrot)
	theReturn.y = theVector.y
	theReturn.z = -theVector.x * math.sin(yrot) + theVector.z * math.cos(yrot)
	return theReturn
end
function zVectorRotation(theVector, zrot)
	local theReturn = vector(0,0,0)
	theReturn.x = theVector.x * math.cos(zrot) - theVector.y * math.sin(zrot)
	theReturn.y = theVector.x * math.sin(zrot) + theVector.z * math.cos(zrot)
	theReturn.z = theVector.z
	return theReturn
end

function rotateVector(theVector, xrot, yrot, zrot)
	theVector = zVectorRotation(theVector, zrot)
	theVector = yVectorRotation(theVector, yrot)
	theVector = xVectorRotation(theVector, xrot)
	return theVector
end

local script = {} -- Script table

function script:load() 

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

				--get its poly points for AI
				for i = 1, #newObject.polyPoints do
					--scale the vectors
					newObject.polyPoints[i] = newObject.polyPoints[i] * vector(xscale, yscale, zscale)

					--rotate the points
					newObject.polyPoints[i] = rotateVector(newObject.polyPoints[i], xrot, yrot, zrot)

					--move the points
					newObject.polyPoints[i] = newObject.polyPoints[i] + vector(xpos, ypos, zpos)
				end
				network.sendPolygons(newObject.polyPoints)
			
			end
			print("saved puz")
			io.close(file)
		end
end

function script:init()
	--check some kind of table here instead
	local puzzleName =  "assets/puzzleParts/test.puz"
	
	self:load()

end

return script -- Return script table