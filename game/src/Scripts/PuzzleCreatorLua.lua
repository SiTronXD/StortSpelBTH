package.path = package.path..";./../game/src/Scripts/?.lua"
require "Prefabs"

local script = {} -- Script table

script.entities = {}
script.nrOfEntities = 0
script.theEntity = 0

script.i = 1
script.iMax = 0

function script:update(dt)

	if(input.isKeyPressed(Keys.P))then
		self:save();
	end
	if(input.isKeyPressed(Keys.C)) then

		print("trying to create prefab")

		Prefabs[self.i].Transform.position = scene.getComponent(scene.getMainCamera(),CompType.Transform).position
		local e = scene.createPrefab(Prefabs[self.i])

		self.nrOfEntities = self.nrOfEntities + 1;
		--the ent
		--type
		--self.entities[self.nrOfEntities] = {e, Prefabs[self.i].id}
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

function script:init()
	self.iMax = #Prefabs
end

return script -- Return script table
