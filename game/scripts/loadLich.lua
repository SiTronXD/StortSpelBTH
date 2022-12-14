local script = {}

function script:init()
    local lichMesh = resources.addMesh("assets/models/Lich/Lich_Walk.fbx");
    scene.setComponent(self.ID, CompType.Mesh, lichMesh);
    local col = {
        radius = 6,
        height = 22,
        type = ColliderType.Capsule,
        offset = vector(0, 15, 0),
        isTrigger = false
    };
    scene.setComponent(self.ID, CompType.Collider, col)
    scene.setComponent(self.ID, CompType.Animation, {})
end

return script