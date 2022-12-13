#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class VRandom;

#define LEFT_P  0
#define RIGHT_P 1
#define UPPER_P 2
#define LOWER_P 3

struct Tile
{
    enum Type : int
    {
        Unused = -1,
        AI,
        Border,
        InnerBorder,
        OneXOne,
        TwoXOne,
        OneXTwo,
        TwoXTwo,
        Reserved,
        Exit
    };

    Tile() = default;
    Tile(Type type, const glm::vec2& pos)
        :type(type), position(pos)
    {
    }

    Type type = Type::Unused;
    glm::vec2 position = glm::vec2(0.f);
};

// TODO: Fix room when it generates to edges (can't happen with these settings)

class RoomGenerator
{
public:

    // Default values create larger rooms (higher levels) (same as extern-test)
    struct RoomDescription
    {
        uint32_t twoXTwoChance = 80u;
        uint32_t maxTwoXTwo = 6u;

        uint32_t oneXTwoChance = 80u;
        uint32_t maxOneXTwo = 6u;

        uint32_t bigTileMinDist = 1u;
        uint32_t radius = 4u;

        uint32_t numBranches = 6u;
        uint32_t branchDepth = 3u;
        uint32_t branchDist = 3u;

        uint32_t maxAngle = 45u;
    };

    static const uint32_t WIDTH_HEIGHT = 50u;
    static const uint32_t BORDER_SIZE = 3u;

private:
    RoomDescription roomDesc;
    VRandom& random;

    Tile::Type** tiles2D;
    std::vector<Tile> tiles;

    glm::ivec2 minMaxPos[4]; // x, -x, z, -z
    glm::ivec2 exitTilesPos[4];
    glm::ivec2 middle;
    glm::ivec2 size;

    void drawCircle(const glm::ivec2& center, uint32_t radius, Tile::Type target, Tile::Type placeType);

    void setBorders();
    void findMinMax();
    void setExits(bool* doors);
    void setBigTiles();
    void findAITiles(bool* doors);
    void finalize();

    // Helpers
    Tile::Type& getType(const glm::ivec2& pos);
    bool isValid(const glm::ivec2& pos);
    bool onEdge(const glm::ivec2& pos);

    bool canPlaceOneXTwo(const glm::ivec2& pos, bool vertical);
    bool canPlaceTwoXTwo(const glm::ivec2& pos);

    void placeTwoXTwo(const glm::ivec2& pos);
    void placeOneXTwo(const glm::ivec2& pos, bool vertical);
public:
    RoomGenerator(VRandom& random);
    ~RoomGenerator();

    void setDesc(const RoomDescription& roomDesc);

    void clear();
    void generate(bool* doors);

    const glm::ivec2* getMinMax() const;
    const glm::ivec2* getExits() const;

    const std::vector<Tile>& getTiles() const;
};