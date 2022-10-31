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
public:
    static const float DEFAULT_TILE_SCALE;

private:
    int TILE_TYPES;
    int ROOM_SIZE;
    int HALF_ROOM = ROOM_SIZE / 2;

    Tile::Type* room;
    std::vector<Tile> tiles;
    std::vector<Tile> borders;
    std::vector<Tile> exitPathsTiles;
    glm::vec2 minMaxPos[4]; // x, -x, z, -z
    glm::vec2 exitTilesPos[4];

    int getArrayIndexFromPosition(int x, int y) const;
    void getIJIndex(int index, int* output) const;

    void addPiece(glm::vec2 position, int depth);
    void placeTile(Tile::Type tileType, glm::vec2 gridPosition, glm::vec2 worldPosition);
    glm::vec2 getFreeLarge(glm::vec2 position);
    glm::vec2 getFreeAdjacent(glm::vec2 position, glm::vec2 dir);

public:
    RoomGenerator();
    ~RoomGenerator();
    void init(int roomSize, int tileTypes);

    void generateRoom();
    void generateBorders(const bool* hasDoors);
    
    int getRoomSize() const;
    int getNrTiles() const;
    int getNrBorders() const;
    int getNrExitTiles() const;

    int getRoomTile(int index) const;
    const Tile& getTile(int index) const;
    const Tile& getBorder(int index) const;
    const Tile& getExitTiles(int index) const;
    
    const glm::vec2* getExitTilesPos() const;
    const glm::vec2* getMinMaxPos() const;

    void reset();
};