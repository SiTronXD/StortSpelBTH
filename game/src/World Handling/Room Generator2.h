#pragma once
#include "vengine.h"

struct Tile2
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

class RoomGen
{
public:
    static const uint32_t WIDTH_NUM = 15u;
private:
    Tile2::Type* tiles2D;
    std::vector<Tile2> tiles;
    std::vector<Tile2> borders;
    std::vector<Tile2> exitPathsTiles;
    glm::vec2 minMaxPos[4]; // x, -x, z, -z
    glm::vec2 exitTilesPos[4];
public:
    RoomGen();
    ~RoomGen();
};