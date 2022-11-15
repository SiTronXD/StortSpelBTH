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
        TwoXOne = 3,
        TwoXTwo = 4,
        Exit = 5
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
    static const uint32_t WIDTH_HEIGHT = 35u;

    struct RoomGenDescription
    {
        uint32_t borderSize = 3u;
        uint32_t radius = 3u;
        uint32_t numBranches = 4u;
        uint32_t branchDepth = 3u;
        uint32_t branchDist = 3u;
        uint32_t maxAngle = 90u;
    };

private:
    RoomGenDescription desc;

    Tile2::Type** tiles2D;
    std::vector<Tile2> tiles;
    std::vector<Tile2> borders;
    std::vector<Tile2> exitPathsTiles;

    glm::ivec2 minMaxPos[4]; // x, -x, z, -z
    glm::ivec2 exitTilesPos[4];
    glm::ivec2 middle;
    glm::ivec2 size;

    void genCircle(const glm::ivec2& center, uint32_t radius);

    void setBorders();
    void findMinMax();
    void setExits(bool* doors);
    void finalize();

    // Helpers
    inline Tile2::Type& getType(const glm::ivec2& pos);
    inline bool isValid(const glm::ivec2& pos);
    inline bool onEdge(const glm::ivec2& pos);

    bool canPlaceOneXTwo(const glm::ivec2& pos, bool vertical);
    bool canPlaceTwoXOne(const glm::ivec2& pos);
    bool canPlaceTwoXTwo(const glm::ivec2& pos);

public:
    RoomGen();
    ~RoomGen();

    void setDesc(const RoomGenDescription& desc);
    void clear();
    void generate(bool* doors);

    const glm::ivec2* getMinMax() const;
    const glm::ivec2* getExits() const;
    glm::ivec2 getCenter() const;
    
    uint32_t getNumTiles() const;
    uint32_t getNumBorders() const;
    uint32_t getNumExitTiles() const;

    const Tile2& getTile(uint32_t index) const;
    const Tile2& getBorder(uint32_t index) const;
    const Tile2& getExitTile(uint32_t index) const;
};