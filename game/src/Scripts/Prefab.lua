Prefab = {
	Transform = { 
		position = vector(0, 0, 0),
		rotation = vector(0, 0, 0),
		scale = vector.fill(1) 
	}, 
	Mesh = "",
	Name = "",
	id = 0
}

function Prefab.New(o)
	o = o or {}

	setmetatable(o, self)

	o.Transform = {
		position = vector(0, 0, 0),
		rotation = vector(0, 0, 0),
		scale = vector.fill(1) 
	}
	o.Mesh = "vengine_assets/models/cube.obj"
	o.Name = ""
	o.id = 0

	return o;
end