#pragma once
#include "vengine.h"

#define VALID(vec) (vec.x >= 0 && vec.x < WIDTH_NUM && vec.y >= 0 && vec.y < WIDTH_NUM)

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

    Tile2() = default;
    Tile2(Type type, const glm::ivec2& pos)
        :type(type), position(pos)
    {
    }

    Type type = Type::Invalid;
    glm::ivec2 position = glm::ivec2(0);
};

class RoomGen
{
public:
    uint32_t WIDTH_NUM = 15u;
    uint32_t CIRCLE_RADIUS = 3u;
    uint32_t NUM_BRANCHES = 4u;
    uint32_t BRANCH_LENGTH = 3u;

private:
    Tile2::Type** tiles2D;
    std::vector<Tile2> tiles;
    std::vector<Tile2> borders;
    std::vector<Tile2> exitPathsTiles;
    glm::vec2 minMaxPos[4]; // x, -x, z, -z
    glm::vec2 exitTilesPos[4];

    void genCircle(const glm::ivec2& center);

public:
    RoomGen();
    ~RoomGen();

    void generate(uint32_t width_num, uint32_t radius, uint32_t numBranches, uint32_t branchLength);

    int getNrTiles() const;
    int getNrBorders() const;
    int getNrExitTiles() const;

    const Tile2& getTile(int index) const;
    const Tile2& getBorder(int index) const;
    const Tile2& getExitTiles(int index) const;
};