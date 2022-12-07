local script = {}

function script:init()
    local tankMesh = resources.addMesh("assets/models/Tank/TankWalk.fbx");
    scene.setComponent(self.ID, CompType.Mesh, tankMesh);
    local col = {
        radius  = 20,
        type = ColliderType.Sphere,
        offset = vector(0, 19.5, 0),
        isTrigger = false
    };
    scene.setComponent(self.ID, CompType.Collider, col)
    scene.setComponent(self.ID, CompType.Animation, {})
end

return script