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
    
    //create material representing maya default material lamber1
    Material mat;
    mat.diffuseTextureIndex = resourceManager->addTexture("assets/textures/defaultMaterial.png");
    mat.specularTextureIndex = 0;
    mat.descriptorIndex = ~0u;
    uint32_t materialIndex = resourceManager->addMaterial("lambert1", mat);
    meshData.submeshes[0].materialIndex = materialIndex;

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
    MeshHeader meshHeader;
    memcpy(&meshHeader, data, sizeof(MeshHeader));
   
    //get current mesh data and save previous submeshdata for material info
    MeshData meshData = resourceManager->getMesh(scene->getComponent<MeshComponent>(this->mayaObjects[meshHeader.name]).meshID).getMeshData();
    std::vector<SubmeshData> submeshData = resourceManager->getMesh(scene->getComponent<MeshComponent>(this->mayaObjects[meshHeader.name]).meshID).getSubmeshData();
    //update mesh data and set correct material
    updateMeshData(&meshData, data);
    meshData.submeshes[0].materialIndex = submeshData[0].materialIndex;
    (int)resourceManager->addMesh(meshHeader.name, meshData);
}

void ObjectHandler::addOrUpdateMaterial(Scene* scene, ResourceManager* resourceManager, char* data)
{
    MaterialHeader materialHeader;
    memcpy(&materialHeader, data, sizeof(MaterialHeader));
    Material mat;
    mat.diffuseTextureIndex = resourceManager->addTexture(materialHeader.diffusePath);
    mat.specularTextureIndex = 0;
    mat.descriptorIndex = ~0u;
    resourceManager->addMaterial(materialHeader.materialName, mat);
}

void ObjectHandler::setMeshMaterial(Scene* scene, ResourceManager* resourceManager, char* data)
{
    MeshMaterialConnectionHeader connectionHeader;
    memcpy(&connectionHeader, data, sizeof(MeshMaterialConnectionHeader));
    Mesh& mesh = resourceManager->getMesh(scene->getComponent<MeshComponent>(this->mayaObjects[connectionHeader.meshName]).meshID);
    mesh.getSubmesh(0).materialIndex = resourceManager->addMaterial(connectionHeader.materialName);

}
MeshData ObjectHandler::createMeshData(char* data)
{
    MeshHeader mesh;
    int offset = 0;
    
    memcpy(&mesh, data + offset, sizeof(MeshHeader));
    offset += sizeof(MeshHeader);
    
    VertexHeader* vertices = new(__FILE__, __LINE__) VertexHeader[mesh.vertexCount];
    memcpy(vertices, data + offset, sizeof(VertexHeader) * mesh.vertexCount);
    offset += sizeof(VertexHeader) * mesh.vertexCount;
    
    int* indices = new(__FILE__, __LINE__) int[mesh.indexCount];
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
        .materialIndex = 0,
        .startIndex = 0,
        .numIndicies = static_cast<uint32_t>(mesh.indexCount),
    });

    if (vertices)
        delete[] vertices;
    if (indices)
        delete[] indices;

    return meshData;
}

void ObjectHandler::updateMeshData(MeshData* meshData, char* data) 
{
    MeshHeader mesh;
    int offset = 0;
    
    memcpy(&mesh, data + offset, sizeof(MeshHeader));
    offset += sizeof(MeshHeader); 
    VertexHeader* vertices = new(__FILE__, __LINE__) VertexHeader[mesh.vertexCount];
    memcpy(vertices, data + offset, sizeof(VertexHeader) * mesh.vertexCount);
    offset += sizeof(VertexHeader) * mesh.vertexCount;
    int* indices = new(__FILE__, __LINE__) int[mesh.indexCount];
    memcpy(indices, data + offset, sizeof(int) * mesh.indexCount);
    
    meshData->vertexStreams.positions.clear();
    meshData->vertexStreams.normals.clear();
    meshData->vertexStreams.texCoords.clear();
    meshData->indicies.clear();

    for (int i = 0; i < mesh.vertexCount; i++)
      {
        meshData->vertexStreams.positions.push_back(glm::vec3(
            vertices[i].position[0],
            vertices[i].position[1],
            vertices[i].position[2]
        ));
        meshData->vertexStreams.normals.push_back(glm::vec3(
            vertices[i].normal[0], vertices[i].normal[1], vertices[i].normal[2]
        ));
        meshData->vertexStreams.texCoords.push_back(
            glm::vec2(vertices[i].uv[0], vertices[i].uv[1])
        );
      }
    for (int i = 0; i < mesh.indexCount; i++)
      meshData->indicies.push_back(static_cast<uint32_t>(indices[i]));

    meshData->submeshes[0].numIndicies = mesh.indexCount;

    if (vertices)
        delete[] vertices;
    if (indices)
        delete[] indices;
}