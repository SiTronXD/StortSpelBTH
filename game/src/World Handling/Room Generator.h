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
    static const uint32_t TWO_X_TWO_CHANCE = 20u; // Percentage
    static const uint32_t MAX_TWO_X_TWO = 8u;//3u;
    
    static const uint32_t ONE_X_TWO_CHANCE = 20u; // Percentage
    static const uint32_t MAX_ONE_X_TWO = 8u;//4u;
    
    static const uint32_t BIG_TILE_MIN_DIST = 1u;

    static const uint32_t WIDTH_HEIGHT = 50u;//40u;
    static const uint32_t BORDER_SIZE = 3u;
    static const uint32_t RADIUS = 4u;//3u;
    static const uint32_t NUM_BRANCHES = 3u;
    static const uint32_t BRANCH_DEPTH = 3u;//2u;
    static const uint32_t BRANCH_DIST = 3u;//2u;
    static const uint32_t MAX_ANGLE = 60u;
private:
    VRandom& random;

    Tile::Type** tiles2D;
    std::vector<Tile> mainTiles;
    std::vector<Tile> bigTiles;
    std::vector<Tile> borders;
    std::vector<Tile> innerBorders;
    std::vector<Tile> exitPathsTiles;

    glm::ivec2 minMaxPos[4]; // x, -x, z, -z
    glm::ivec2 exitTilesPos[4];
    glm::ivec2 middle;
    glm::ivec2 size;

    void drawCircle(const glm::ivec2& center, uint32_t radius);

    void setBorders();
    void findMinMax();
    void setExits(bool* doors);
    void setBigTiles();
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

    void clear();
    void generate(bool* doors);

    const glm::ivec2* getMinMax() const;
    const glm::ivec2* getExits() const;
    const glm::ivec2& getMiddle() const;
    const glm::ivec2& getSize() const;

    uint32_t getNumMainTiles() const;
    uint32_t getNumBigTiles() const;
    uint32_t getNumBorders() const;
    uint32_t getNumInnerBorders() const;
    uint32_t getNumExitTiles() const;

    const Tile& getMainTile(uint32_t index) const;
    const Tile& getBigTile(uint32_t index) const;
    const Tile& getBorder(uint32_t index) const;
    const Tile& getInnerBorder(uint32_t index) const;
    const Tile& getExitTile(uint32_t index) const;
};