#pragma once

#include "vengine.h"
#include <random>


struct Tile 
{
    enum Type : unsigned int
    {
        // Order is currently dependant on obj names
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

    int* room;
    std::vector<Tile> tiles;
    glm::vec2 minMaxPos[4]; // x, -x, z, -z

    int getArrayIndexFromPosition(int x, int y)
    {
        return (y + HALF_ROOM) * ROOM_SIZE + (x + HALF_ROOM);
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
        memset(room, 0, sizeof(int) * ROOM_SIZE * ROOM_SIZE);
        tiles.clear();

        minMaxPos[0] = glm::vec2(-ROOM_SIZE);
        minMaxPos[1] = glm::vec2(ROOM_SIZE);
        minMaxPos[2] = glm::vec2(-ROOM_SIZE);
        minMaxPos[3] = glm::vec2(ROOM_SIZE);
    }
};
