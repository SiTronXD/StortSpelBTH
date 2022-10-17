Prefab = {
	Transform = { 
		position = vector(0, 0, 0),
		rotation = vector(0, 0, 0),
		scale = vector.fill(1) 
	}, 
	Mesh = "vengine_assets/models/cube.obj",
	Name = "",
	id = 0
}

function Prefab:New(o)
	o = o or {}

	self.__index = self
	setmetatable(o, self)

	return o;
end

function Prefab:Copy(o)
	self.Transform.position = o.Transform.position
	self.Transform.rotation = o.Transform.rotation
	self.Transform.scale = o.Transform.scale
	self.Name = o.Name
	self.Mesh = o.Mesh
	self.id = o.id

	if o.polyPoints ~= nil then
	
	print(#o.polyPoints)
	self.polyPoints = {}
		for i = 1, #o.polyPoints do
			self.polyPoints[i] = o.polyPoints[i]
		end
	end

end