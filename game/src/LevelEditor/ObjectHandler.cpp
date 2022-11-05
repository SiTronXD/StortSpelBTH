#include "ObjectHandler.h"


ObjectHandler::ObjectHandler() {
}

ObjectHandler::~ObjectHandler() 
{
}

void ObjectHandler::addNewMesh(Scene* scene, ResourceManager* resourceManager, char* data)
{
    MeshHeader mesh;
    memcpy(&mesh, data, sizeof(MeshHeader));
    
    MeshData meshData = createMeshData(data);

    this->mayaObjects.insert({mesh.name, scene->createEntity()});
    scene->setComponent<MeshComponent>(
        this->mayaObjects[mesh.name],
        (int)resourceManager->addMesh(mesh.name, meshData)
    );
}

void ObjectHandler::removeMesh(Scene* scene, char* data) 
{
    RemoveMeshHeader header;
    memcpy(&header, data, sizeof(RemoveMeshHeader));
    scene->removeEntity(this->mayaObjects[header.name]);
    this->mayaObjects.erase(header.name);
}

void ObjectHandler::updateMeshTransform(Scene* scene, char* data) 
{
    MeshTransformHeader header;
    memcpy(&header, data, sizeof(MeshTransformHeader));
    Transform& t = scene->getComponent<Transform>(this->mayaObjects[header.name]);
    t.position = glm::vec3(header.position[0], header.position[1], header.position[2]);
    t.rotation = glm::vec3(header.rotation[0], header.rotation[1], header.rotation[2]);
    t.scale = glm::vec3(header.scale[0], header.scale[1], header.scale[2]);

}

void ObjectHandler::updateMeshTopology(Scene* scene, ResourceManager* resourceManager, char* data) 
{
  MeshHeader mesh;
  memcpy(&mesh, data, sizeof(MeshHeader));

  MeshData meshData = createMeshData(data);
  (int)resourceManager->addMesh(mesh.name, meshData);
}

void ObjectHandler::updateMeshMaterial(Scene* scene, char* data) {}

MeshData ObjectHandler::createMeshData(char* data)
{
    MeshHeader mesh;
    int offset = 0;
    
    memcpy(&mesh, data + offset, sizeof(MeshHeader));
    offset += sizeof(MeshHeader);
    
    VertexHeader* vertices = new VertexHeader[mesh.vertexCount];
    memcpy(vertices, data + offset, sizeof(VertexHeader) * mesh.vertexCount);
    offset += sizeof(VertexHeader) * mesh.vertexCount;
    
    int* indices = new int[mesh.indexCount];
    memcpy(indices, data + offset, sizeof(int) * mesh.indexCount);
    
    MeshData meshData;
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        meshData.vertexStreams.positions.push_back(glm::vec3(
            vertices[i].position[0],
            vertices[i].position[1],
            vertices[i].position[2]
        ));
        meshData.vertexStreams.normals.push_back(glm::vec3(
            vertices[i].normal[0], vertices[i].normal[1], vertices[i].normal[2]
        ));
        meshData.vertexStreams.texCoords.push_back(
            glm::vec2(vertices[i].uv[0], vertices[i].uv[1])
        );
    }
    for (int i = 0; i < mesh.indexCount; i++)
      meshData.indicies.push_back(static_cast<uint32_t>(indices[i]));
    
    meshData.submeshes.push_back(SubmeshData{
        .materialIndex = 1,
        .startIndex = 0,
        .numIndicies = static_cast<uint32_t>(mesh.indexCount),
    });

    return meshData;
}