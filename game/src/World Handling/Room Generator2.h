#pragma once
#include "vengine.h"


struct Tile2
{
    enum Type : int
    {
        Unused = -1,
        Border = 0,
        OneXOne = 1,
        OneXTwo = 2,
        TwoXTwo = 3,
        Exit = 4
    };

    Tile2() = default;
    Tile2(Type type, const glm::ivec2& pos)
        :type(type), position(pos)
    {
    }

    Type type = Type::Unused;
    glm::ivec2 position = glm::ivec2(0);
};

class RoomGen
{
public:
    static const uint32_t WIDTH_HEIGHT = 30u;

    struct RoomGenDescription
    {
        uint32_t borderSize = 3u;
        uint32_t radius = 3u;
        uint32_t numBranches = 4u;
        uint32_t branchDepth = 3u;
        uint32_t branchDist = 3u;
        uint32_t maxAngle = 90u;
        bool exits[4] = {true, true, true, true}; // true temp
    };

private:
    RoomGenDescription desc;

    Tile2::Type** tiles2D;
    std::vector<Tile2> tiles;
    std::vector<Tile2> borders;
    std::vector<Tile2> exitPathsTiles;
    glm::ivec2 minMaxPos[4]; // x, -x, z, -z
    glm::ivec2 exitTilesPos[4];

    void genCircle(const glm::ivec2& center);

    void setBorders();
    void setExits();
    void finalize();

    // Helpers
    inline Tile2::Type& getType(const glm::ivec2& pos);
    inline bool isValid(const glm::ivec2& pos);
    inline bool onEdge(const glm::ivec2& pos);

public:
    RoomGen();
    ~RoomGen();

    void setDesc(const RoomGenDescription& desc);
    void clear();
    void generate();

    const glm::ivec2* getMinMax() const;

    uint32_t getNumTiles() const;
    uint32_t getNumBorders() const;
    uint32_t getNumExitTiles() const;

    const Tile2& getTile(uint32_t index) const;
    const Tile2& getBorder(uint32_t index) const;
    const Tile2& getExitTile(uint32_t index) const;
};