package.path = package.path..";./../game/src/Scripts/?.lua"
require "Prefabs"
require "PuzzleLoader"

local script = {} -- Script table

script.entities = {}
script.nrOfEntities = 0
script.theEntity = 0

script.i = 1
script.iMax = 0
script.loadedPuzzle = false

script.iDebug = 0

function script:init()
	self.iMax = #Prefabs
end

function script:update(dt)

	if(input.isKeyPressed(Keys.P))then
		self:save();
	end
	if(input.isKeyPressed(Keys.L) and not self.loadedPuzzle)then
		self.loadedPuzzle = true
		loadPuzzle("assets/puzzleParts/test.puz", vector(0,0,0))
	end
	if(input.isKeyPressed(Keys.C)) then

		print("trying to create prefab")

		--Prefabs[self.i].Transform.position = scene.getComponent(scene.getMainCamera(),CompType.Transform).position
		if (self.iDebug == 0) then
			Prefabs[self.i].Transform.position = vector(0,0,60)
		elseif (self.iDebug == 1) then
			Prefabs[self.i].Transform.position = vector(20,0,50)
		elseif (self.iDebug == 2) then
			Prefabs[self.i].Transform.position = vector(-20,0,50)
		else
			return
		end
		
		self.iDebug = self.iDebug + 1

		local e = scene.createPrefab(Prefabs[self.i])

		self.nrOfEntities = self.nrOfEntities + 1;

		self.entities[self.nrOfEntities] = {e, self.i}

		print("created prefab")

	end
	if(input.isMouseButtonPressed(Mouse.LEFT)) then
		self.i = self.i - 1;
		if(self.i < 1) then
			self.i = self.i + 1;
		end
		print(self.i .. Prefabs[self.i].Name)
	end
	if(input.isMouseButtonPressed(Mouse.RIGHT)) then
		self.i = self.i + 1;
		if(self.i > self.iMax) then
			self.i = self.i - 1;
		end
		print(self.i .. Prefabs[self.i].Name)
	end

end

function script:save() 

		file = io.open("assets/puzzleParts/test.puz", "w")
		io.output(file)

		--write the number of entites
		io.write(tostring(#self.entities) .. "\n")
		for i = 1, #self.entities do
			
				local pos = scene.getComponent(self.entities[i][1], CompType.Transform).position
				local rot = scene.getComponent(self.entities[i][1], CompType.Transform).rotation
				local scale = scene.getComponent(self.entities[i][1], CompType.Transform).scale
				--write position, rotation, scale and what type it is
				io.write(tostring(math.floor(pos.x).."\n"))
				io.write(tostring(math.floor(pos.y).."\n"))
				io.write(tostring(math.floor(pos.z).."\n"))
				io.write(tostring(math.floor(rot.x).."\n"))
				io.write(tostring(math.floor(rot.y).."\n"))
				io.write(tostring(math.floor(rot.z).."\n"))
				io.write(tostring(math.floor(scale.x).."\n"))
				io.write(tostring(math.floor(scale.y).."\n"))
				io.write(tostring(math.floor(scale.z).."\n"))
				io.write(tostring(self.entities[i][2].."\n"))--type
			
		end
		print("saved puz")
		io.close(file)
end

function script:load(filepath) 

		--file = io.open("assets/puzzleParts/test.puz", "r")
		file = io.open(filepath, "r")

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
				local e = scene.createPrefab(newObject)
				self.nrOfEntities = self.nrOfEntities + 1;
				self.entities[self.nrOfEntities] = {e, type}

				--get its poly points for AI
				for i = 1, #newObject.polyPoints do
					--scale the vectors
					newObject.polyPoints[i] = newObject.polyPoints[i] * vector(xscale, yscale, zscale)

					--rotate the points
					newObject.polyPoints[i]:rotate(xrot, yrot, zrot)

					--move the points
					newObject.polyPoints[i] = newObject.polyPoints[i] + vector(xpos, ypos, zpos)
				end
				network.sendPolygons(newObject.polyPoints)
			
			end
			print("saved puz")
			io.close(file)
		end
end

return script -- Return script table
