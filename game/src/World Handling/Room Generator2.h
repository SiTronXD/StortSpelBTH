#pragma once
#include "vengine.h"

#define POS_IDX(x, y) WIDTH_NUM * y + x

struct Tile2
{
    enum Type : int
    {
        // values are currently dependant on obj names
        Invalid = -1,
        Border = 0,
        OneXOne = 1,
        OneXTwo = 2,
        TwoXTwo = 3
    };

    Type type = Type::Border;
    glm::vec2 position;
};

class RoomGen
{
public:
    static const uint32_t WIDTH_NUM = 15u;
    static const uint32_t CIRCLE_RADIUS = 3u;
    static const uint32_t NUM_BRANCHES = 3u;
    static const uint32_t BRANCH_LENGTH = 2u;
private:
    Tile2::Type* tiles2D;
    std::vector<Tile2> tiles;
    std::vector<Tile2> borders;
    std::vector<Tile2> exitPathsTiles;
    glm::vec2 minMaxPos[4]; // x, -x, z, -z
    glm::vec2 exitTilesPos[4];

    Tile2::Type getType(int xPos, int yPos) const;
    void genCircle(const glm::ivec2& center);

public:
    RoomGen();
    ~RoomGen();

    void generate();

    int getNrTiles() const;
    int getNrBorders() const;
    int getNrExitTiles() const;

    Tile2::Type getRoomTile(int index) const;
    const Tile2& getTile(int index) const;
    const Tile2& getBorder(int index) const;
    const Tile2& getExitTiles(int index) const;
};