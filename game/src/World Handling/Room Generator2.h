#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#define LEFT_P 0
#define RIGHT_P  1
#define UPPER_P  2
#define LOWER_P  3

struct Tile2
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

    Tile2() = default;
    Tile2(Type type, const glm::vec2& pos)
        :type(type), position(pos)
    {
    }

    Type type = Type::Unused;
    glm::vec2 position = glm::vec2(0.f);
};

// TODO: Fix room when it generates to edges
// TODO: Fix room when it generates to edges
// TODO: Fix room when it generates to edges

class RoomGen
{
public:
    static const uint32_t WIDTH_HEIGHT = 35u;

    static const uint32_t TWO_X_TWO_CHANCE = 10u; // Percentage
    static const uint32_t MAX_TWO_X_TWO = 10u;

    static const uint32_t ONE_X_TWO_CHANCE = 10u; // Percentage
    static const uint32_t MAX_ONE_X_TWO = 10u;

    static const uint32_t BIG_TILE_MIN_DIST = 1u;

private:
    static const uint32_t BORDER_SIZE = 3u;
    static const uint32_t RADIUS = 2u;
    static const uint32_t NUM_BRANCHES = 3u;
    static const uint32_t BRANCH_DEPTH = 2u;
    static const uint32_t BRANCH_DIST = 2u;
    static const uint32_t MAX_ANGLE = 180u;

    Tile2::Type** tiles2D;
    std::vector<Tile2> mainTiles;
    std::vector<Tile2> bigTiles;
    std::vector<Tile2> borders;
    std::vector<Tile2> innerBorders;
    std::vector<Tile2> exitPathsTiles;

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
    inline Tile2::Type& getType(const glm::ivec2& pos);
    inline bool isValid(const glm::ivec2& pos);
    inline bool onEdge(const glm::ivec2& pos);

    bool canPlaceOneXTwo(const glm::ivec2& pos, bool vertical);
    bool canPlaceTwoXTwo(const glm::ivec2& pos);

    bool tryPlaceTwoXTwo(const glm::ivec2& pos);
    bool tryPlaceOneXTwo(const glm::ivec2& pos, bool vertical);
public:

    bool print = false;
    RoomGen();
    ~RoomGen();

    void clear();
    void generate(bool* doors);

    const glm::ivec2* getMinMax() const;
    const glm::ivec2* getExits() const;
    const glm::ivec2& getMiddle() const;

    uint32_t getNumMainTiles() const;
    uint32_t getNumBigTiles() const;
    uint32_t getNumBorders() const;
    uint32_t getNumInnerBorders() const;
    uint32_t getNumExitTiles() const;

    const Tile2& getMainTile(uint32_t index) const;
    const Tile2& getBigTile(uint32_t index) const;
    const Tile2& getBorder(uint32_t index) const;
    const Tile2& getInnerBorder(uint32_t index) const;
    const Tile2& getExitTile(uint32_t index) const;
};