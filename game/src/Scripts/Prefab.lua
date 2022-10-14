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