local script = {}

function script:init()
    local tankMesh = resources.addMesh("assets/models/Tank/TankWalk.fbx");
    scene.setComponent(self.ID, CompType.Mesh, tankMesh);
    local col = {
        extents = vector(10.0, 20.0, 10.0),
        type = ColliderType.Box,
        offset = vector(0, 19.5, 0),
        isTrigger = false
    };
    scene.setComponent(self.ID, CompType.Collider, col)
    scene.setComponent(self.ID, CompType.Animation, {})
end

return script