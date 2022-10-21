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

    Tile()
        :type(Type::Border), position(0.f)
    {
    }

    Type type;
    glm::vec2 position;
};

class RoomGenerator 
{
public:
    static const float DEFAULT_TILE_SCALE;
private:
    int TILE_TYPES;
    int ROOM_SIZE;
    int HALF_ROOM  = ROOM_SIZE / 2;

    Tile::Type* room;
    std::vector<Tile> tiles;

    // Order: x, -x, z, -z
    glm::vec2 minMaxPos[4]; 
    glm::vec2 exitTilesPos[4];

    int getArrayIndexFromPosition(int x, int y) const;
    int getArrayIndexFromPosition(float x, float y) const;

    void getIJIndex(int index, int* output)
    {
        output[1] = (index / ROOM_SIZE);
        output[0] = (index - ROOM_SIZE * output[1]);
    }

    void addPiece(const glm::vec2& position, int depth);
    void placeTile(Tile::Type tileType, const glm::vec2& gridPosition, const glm::vec2& worldPosition);
    glm::vec2 getFreeLarge(const glm::vec2& position);
    glm::vec2 getFreeAdjacent(const glm::vec2& position, glm::vec2 dir);

public:
    RoomGenerator();
    ~RoomGenerator();
    void init(int roomSize, int tileTypes);

    void generateRoom();
    void generateBorders(const bool* hasDoors);
    void reset();

    const glm::vec2* getExitTilesPos() const;
    const glm::vec2* getMinMaxPos() const;

    Tile::Type getRoomTile(int index) const;
    int getNrTiles() const;
    const Tile& getTile(int index) const;
};
