#pragma once
#include "vengine.h"

#define LEFT_P 0
#define RIGHT_P  1
#define UPPER_P  2
#define LOWER_P  3

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
        Reserved = 5,
        Exit = 6
    };

    Tile2() = default;
    Tile2(Type type, const glm::vec2& pos)
        :type(type), position(pos)
    {
    }

    Type type = Type::Unused;
    glm::vec2 position = glm::vec2(0.f);
};

class RoomGen
{
public:
    static const uint32_t WIDTH_HEIGHT = 35u;

    static const uint32_t TWO_X_TWO_CHANCE = 100u; // Percentage
    static const uint32_t DIST_TWO_X_TWO = 0u;
    static const uint32_t MAX_TWO_X_TWO = 100u;

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
    std::vector<Tile2> mainTiles;
    std::vector<Tile2> bigTiles;
    std::vector<Tile2> borders;
    std::vector<Tile2> exitPathsTiles;

    // End point of branches
    //std::vector<glm::ivec2> branchEnds;

    glm::ivec2 minMaxPos[4]; // x, -x, z, -z
    glm::ivec2 exitTilesPos[4];
    glm::ivec2 middle;
    glm::ivec2 size;

    void drawCircle(const glm::ivec2& center, uint32_t radius);

    void setBorders();
    void findMinMax();
    void setExits(bool* doors);
    void set2x2();
    void finalize();

    // Helpers
    inline Tile2::Type& getType(const glm::ivec2& pos);
    inline bool isValid(const glm::ivec2& pos);
    inline bool onEdge(const glm::ivec2& pos);

    bool canPlaceOneXTwo(const glm::ivec2& pos, bool vertical);
    bool canPlaceTwoXTwo(const glm::ivec2& pos);

    bool tryPlaceTwoXTwo(const glm::ivec2& pos);
public:

    bool print = false;
    RoomGen();
    ~RoomGen();

    void setDesc(const RoomGenDescription& desc);
    void clear();
    void generate(bool* doors);

    const glm::ivec2* getMinMax() const;
    const glm::ivec2* getExits() const;
    
    uint32_t getNumMainTiles() const;
    uint32_t getNumBigTiles() const;
    uint32_t getNumBorders() const;
    uint32_t getNumExitTiles() const;

    const Tile2& getMainTile(uint32_t index) const;
    const Tile2& getBigTile(uint32_t index) const;
    const Tile2& getBorder(uint32_t index) const;
    const Tile2& getExitTile(uint32_t index) const;
};