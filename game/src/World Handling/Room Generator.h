#pragma once

#include "vengine.h"
#include <random>

struct Tile {
    int type = 0;
    glm::vec2 position;
};

class RoomGenerator {
  private:
    int TILE_TYPES;
    int ROOM_SIZE;
    int HALF_ROOM  = ROOM_SIZE / 2;

    int* room;
    std::vector<Tile> tiles;

    int getArrayIndexFromPosition(int x, int y)
    {
        return (y + HALF_ROOM) * ROOM_SIZE + (x + HALF_ROOM);
    }

    void      addPiece(glm::vec2 position, int depth);
    void      placeTile(int tileType, glm::vec2 gridPosition, glm::vec2 worldPosition);
    glm::vec2 getFreeLarge(glm::vec2 position);
    glm::vec2 getFreeAdjacent(glm::vec2 position, glm::vec2 dir);

  public:
    RoomGenerator();
    ~RoomGenerator();
    void init(int roomSize, int tileTypes);
    void generateRoom();

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
    
    void clear()
    {
        memset(room, 0, sizeof(int) * ROOM_SIZE * ROOM_SIZE);
        tiles.clear();
    }
};
