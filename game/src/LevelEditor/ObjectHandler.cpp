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
    meshData.submeshes[0].materialIndex = resourceManager->addMaterial("lambert1", mat);

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
   
    //get current mesh data
    MeshData meshData = resourceManager->getMesh(resourceManager->addMesh(mesh.name)).getMeshData();
    //update mesh data and replace old
    updateMeshData(&meshData, data);
    (int)resourceManager->addMesh(mesh.name, meshData);
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
    //get mesh component!!
    Mesh& mesh = resourceManager->getMesh(scene->getComponent<MeshComponent>(this->mayaObjects[connectionHeader.meshName]).meshID);
    mesh.getSubmesh(0).materialIndex = resourceManager->addMaterial(connectionHeader.materialName);
    //resourceManager->getMesh(resourceManager->addMesh(connectionHeader.meshName)).getSubmeshData()[0].materialIndex = resourceManager->addMaterial(connectionHeader.materialName);
    //for (int i = 0; i < submeshes.size(); i++)
    //{
    //    submeshes[i].materialIndex = resourceManager->addMaterial(connectionHeader.materialName);
    //}
}
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
        .materialIndex = 0,
        .startIndex = 0,
        .numIndicies = static_cast<uint32_t>(mesh.indexCount),
    });

    return meshData;
}

void ObjectHandler::updateMeshData(MeshData* meshData, char* data) 
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
}