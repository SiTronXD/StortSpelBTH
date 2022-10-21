#pragma once

#include "vengine.h"
#include <random>

struct Tile 
{
    enum Type : int
    {
        // values are currently dependant on obj names
        Invalid = -1,
        Border = 0,
        OneXOne = 1, 
        OneXTwo = 2, // one tree
        TwoXOne = 3, // two trees
        TwoXTwo = 4
    };

    Type type = Type::Border;
    glm::vec2 position;
};

class RoomGenerator 
{
private:
    int TILE_TYPES;
    int ROOM_SIZE;
    int HALF_ROOM  = ROOM_SIZE / 2;

    Tile::Type* room;
    std::vector<Tile> tiles;

    // Order: x, -x, z, -z
    glm::vec2 minMaxPos[4]; 
    glm::vec2 exitTilesPos[4];

    int getArrayIndexFromPosition(int x, int y)
    {
        return (y + HALF_ROOM) * ROOM_SIZE + (x + HALF_ROOM);
    }
    void getIJIndex(int index, int* output)
    {
        output[1] = (index / ROOM_SIZE);
        output[0] = (index - ROOM_SIZE * output[1]);
    }

    void      addPiece(glm::vec2 position, int depth);
    void      placeTile(Tile::Type tileType, glm::vec2 gridPosition, glm::vec2 worldPosition);
    glm::vec2 getFreeLarge(glm::vec2 position);
    glm::vec2 getFreeAdjacent(glm::vec2 position, glm::vec2 dir);

public:
    RoomGenerator();
    ~RoomGenerator();
    void init(int roomSize, int tileTypes);

    void generateRoom();
    void generateBorders(const bool* hasDoors);

    const glm::vec2* getExitTilesPos() const;

    int getRoomTile(int index) 
    {
        return room[index];
    }

    int getNrTiles()
    {
        return (int)tiles.size();
    }

    Tile getTile(int index) 
    {
        return tiles[index];
    }
    
    const glm::vec2* getMinMaxPos() const 
    {
        return minMaxPos;
    }

    void reset()
    {
        memset(room, Tile::Invalid, sizeof(int) * ROOM_SIZE * ROOM_SIZE);
        tiles.clear();

        minMaxPos[0] = glm::vec2((float)-ROOM_SIZE);
        minMaxPos[1] = glm::vec2((float)ROOM_SIZE);
        minMaxPos[2] = glm::vec2((float)-ROOM_SIZE);
        minMaxPos[3] = glm::vec2((float)ROOM_SIZE);

        for (int i = 0; i < 4; i++)
        {
            exitTilesPos[i] = glm::vec2(0.f);
        }
    }
};
