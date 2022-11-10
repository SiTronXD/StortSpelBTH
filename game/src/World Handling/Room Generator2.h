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