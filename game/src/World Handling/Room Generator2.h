#pragma once
#include "vengine.h"

#define VALID(vec, xySize) (vec.x >= 0 && vec.x < xySize && vec.y >= 0 && vec.y < xySize)

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
    struct RoomGenDescription
    {
        uint32_t widthHeight = 20u;
        uint32_t borderSize = 3u;
        uint32_t radius = 4u;
        uint32_t numBranches = 4u;
        uint32_t branchLength = 3u;
        uint32_t branchDist = 3u;
        uint32_t maxAngle = 90u;
    };

private:
    RoomGenDescription desc;

    Tile2::Type** tiles2D;
    std::vector<Tile2> tiles;
    std::vector<Tile2> borders;
    std::vector<Tile2> exitPathsTiles;
    glm::vec2 minMaxPos[4]; // x, -x, z, -z
    glm::vec2 exitTilesPos[4];

    void genCircle(const glm::ivec2& center);
    void setBorders();
    void finalize();

public:
    RoomGen();
    ~RoomGen();

    void set(const RoomGenDescription& desc);
    void clear();

    void generate();

    uint32_t getNumTiles() const;
    uint32_t getNumBorders() const;
    uint32_t getNumExitTiles() const;

    const Tile2& getTile(uint32_t index) const;
    const Tile2& getBorder(uint32_t index) const;
    const Tile2& getExitTiles(uint32_t index) const;
};