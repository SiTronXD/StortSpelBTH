#include "LevelEditor.h"

LevelEditor::LevelEditor() 
{
}

LevelEditor::~LevelEditor() 
{
  if (comlib)
    delete comlib;
  if (msg)
    delete msg;
  if (header)
    delete header;
}

void LevelEditor::init() 
{
  comlib = new Comlib(L"MayaBuffer", 150 * (1 << 20), Consumer);
  Entity floor = this->createEntity();
  this->setComponent<MeshComponent>(floor, 0);
  this->getComponent<Transform>(floor).scale = glm::vec3(100.0f, 1.0f, 100.0f);
  this->getComponent<Transform>(floor).position = glm::vec3(10.0f, -25.0f, 30.0f);
  this->setComponent<Collider>(floor, Collider::createBox((glm::vec3(100.0f, 1.0f, 100.0f))));
}

void LevelEditor::start()
{
  std::string playerName = "playerID";
  this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);
}

void LevelEditor::update() 
{
  while (comlib->Recieve(msg, header))
    {
      if (header->messageType == NEW_MESH)
        {
          MeshHeader mesh;
          int offset = 0;

          memcpy(&mesh, msg + offset, sizeof(MeshHeader));
          offset += sizeof(MeshHeader);

          VertexHeader* vertices = new VertexHeader[mesh.vertexCount];
          memcpy(
              vertices, msg + offset, sizeof(VertexHeader) * mesh.vertexCount
          );
          offset += sizeof(VertexHeader) * mesh.vertexCount;

          int* indices = new int[mesh.indexCount];
          memcpy(indices, msg + offset, sizeof(int) * mesh.indexCount);

          MeshData meshData;
          for (int i = 0; i < mesh.vertexCount; i++)
            {
              meshData.vertexStreams.positions.push_back(glm::vec3(
                  vertices[i].position[0],
                  vertices[i].position[1],
                  vertices[i].position[3]
              ));
              meshData.vertexStreams.normals.push_back(glm::vec3(
                  vertices[i].normal[0],
                  vertices[i].normal[1],
                  vertices[i].normal[3]
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

          this->mayaObjects.insert({mesh.name, this->createEntity()});
          this->setComponent<MeshComponent>(
              this->mayaObjects[mesh.name],
              (int)this->getResourceManager()->addMesh(mesh.name, meshData)
          );
        }
    }
}