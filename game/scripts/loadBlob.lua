local script = {}

function script:init()
    local blobMesh = resources.addMesh("assets/models/Swarm_Model.obj");
    scene.setComponent(self.ID, CompType.Mesh, blobMesh);
    local col = {
        radius  = 4,
        type = ColliderType.Sphere,
        isTrigger = false
    };
    scene.setComponent(self.ID, CompType.Collider, col)
end

return script