#include "Room Generator.h"

/*
    Limit num objects (atleast houses)
    Sometimes generates:
        holes (something to do with different size pieces I suspect)
        piece on edge
        border piece not on border
        
*/
const float RoomGenerator::DEFUALT_TILE_SCALE = 0.04f;

RoomGenerator::RoomGenerator()
{
    //create 2d array representing room and set all room pieces to 0
    tiles = std::vector<Tile>();
}

RoomGenerator::~RoomGenerator() 
{
    delete[] room;
}

void RoomGenerator::init(int roomSize, int tileTypes)
{
    ROOM_SIZE = roomSize;
    TILE_TYPES = tileTypes;
    HALF_ROOM = ROOM_SIZE / 2;
    room = new Tile::Type[ROOM_SIZE * ROOM_SIZE];

    reset();
}

void RoomGenerator::generateRoom()
{
    //generate first room piece in middle (0,0) and depth 0
    addPiece(glm::vec2(0, 0), 0);    
}

void RoomGenerator::generateBorders(const bool* hasDoors)
{
    // 4 doors, each with a row & column index (i & j) (in the next for loop)
    int doorGridIndex[4][2]{};
    for (int i = 0; i < 4; i++)
    {
        if (hasDoors[i])
            getIJIndex(getArrayIndexFromPosition(minMaxPos[i].x, minMaxPos[i].y), doorGridIndex[i]);

#ifdef _DEBUG
        else
            doorGridIndex[i][0] = doorGridIndex[i][1] = -1;
#endif
    }
    
    int index = -1;
    for (int i = 0; i < ROOM_SIZE; i++) 
    {
        for (int j = 0; j < ROOM_SIZE; j++) 
        {

            index = -1;

            // Check that we're behind the door and
            // save index of which door we're currently behind
            if      (hasDoors[0] && j >  doorGridIndex[0][0] && i == doorGridIndex[0][1]) index = 0;
            else if (hasDoors[1] && j <  doorGridIndex[1][0] && i == doorGridIndex[1][1]) index = 1;
            else if (hasDoors[2] && j == doorGridIndex[2][0] && i >  doorGridIndex[2][1]) index = 2;
            else if (hasDoors[3] && j == doorGridIndex[3][0] && i <  doorGridIndex[3][1]) index = 3;
            
            // Behind a door? Place tile..
            if (index != -1)
            {
                room[i * ROOM_SIZE + j] = Tile::OneXOne;

                Tile tile{};
                tile.type     = Tile::OneXOne;
                tile.position = glm::vec2(j - HALF_ROOM, i - HALF_ROOM);
                tiles.push_back(tile);

                // save the positions of the tile if it's at an edge
                if (i == ROOM_SIZE -1 || i == 0 || j == ROOM_SIZE - 1 || j == 0)
                    exitTilesPos[index] = tiles.back().position;
            }


            // Place a border-tile on empty tiles
            else if (room[i * ROOM_SIZE + j] == -1)
            {
                Tile tile{};
                tile.type     = Tile::Border;
                tile.position = glm::vec2(j - HALF_ROOM, i - HALF_ROOM);
                tiles.push_back(tile);
            }
        }
    }
}

void RoomGenerator::addPiece(glm::vec2 position, int depth)
{
    int x     = position.x;
    int y     = position.y;
    int index = getArrayIndexFromPosition(x, y);

    //add piece only if tile is within room bounds
    if (abs(x) < HALF_ROOM - 1 && abs(y) < HALF_ROOM - 1) 
    {
        std::random_device rd; //obtain random number from hardware
        std::mt19937       gen(rd()); //seed generator
        if (room[index] < 1) {
            std::uniform_int_distribution<> tileTypeRange(1, 10); //TODO: Update when more pieces exists
            Tile::Type tileType = Tile::Type(tileTypeRange(gen));

            // Temp
            //if (tileType == Tile::TwoXTwo)
            tileType = Tile::OneXOne;


            //TODO: clean up code
            switch (tileType)
            {
            case Tile::OneXOne: //Simple 1x1 piece
            {
                placeTile(tileType, position, position);
                break;
            }
            case Tile::OneXTwo: //1x2 piece, need to check if adjecent tile is free
            {
                int dY = getFreeAdjacent(position, glm::vec2(0, 1)).y;
                if (dY != 0) {
                    placeTile(tileType, position, position + glm::vec2(0, 0.5 * dY));
                    room[getArrayIndexFromPosition(x, y + dY)] = tileType;
                    break;
                }
            }
            case Tile::TwoXOne: //2x1
            {
                int dX = getFreeAdjacent(position, glm::vec2(1, 0)).x;
                if (dX != 0) {
                    placeTile(tileType, position, position + glm::vec2(0.5 * dX, 0));
                    room[getArrayIndexFromPosition(x + dX, y)] = tileType;
                    break;
                }
            }
            case Tile::TwoXTwo: //2x2
            {
                glm::vec2 dir = getFreeLarge(position);
                if (dir.x != 0) {
                    placeTile(tileType, position,
                              position + glm::vec2(0.5 * dir.x, 0.5 * dir.y));
                    room[getArrayIndexFromPosition(x + dir.x, y)]         = tileType;
                    room[getArrayIndexFromPosition(x, y + dir.y)]         = tileType;
                    room[getArrayIndexFromPosition(x + dir.x, y + dir.y)] = tileType;
                    break;
                }
            }
            default:
                placeTile(Tile::OneXOne, position, position);
                break;
            }
        }

        std::uniform_int_distribution<> distr( 0, depth * 3); //smaller chance of creating new piece the deeper we are in the tree

        //check if should place tiles above, below, right and left
        for (int i = 0; i < 4; i++) 
        {
            //use sin and cos to check adjacent tiles in clockwise order
            int dirX = sin(i * M_PI / 2);
            int dirY = cos(i * M_PI / 2);

            int       newPiece = (distr(gen));
            glm::vec2 nextPos  = position + glm::vec2(dirX, dirY);
            if (newPiece <= 1) 
            {
                addPiece(nextPos, depth + 1);
            }
        }
    }
}

void RoomGenerator::placeTile(Tile::Type tileType, glm::vec2 gridPosition, glm::vec2 worldPosition)
{
    room[getArrayIndexFromPosition(gridPosition.x, gridPosition.y)] = tileType;

    Tile t;
    t.type = tileType;
    t.position = worldPosition;
    tiles.push_back(t);

    if (tileType == Tile::TwoXTwo)
        return;

    // Find extents of room
    if (worldPosition.x >= minMaxPos[0].x) 
    {
        if (room[getArrayIndexFromPosition(gridPosition.x + 1.f, gridPosition.y)])
            minMaxPos[0] = worldPosition; 
    }
    if (worldPosition.x <= minMaxPos[1].x) 
    {
        if (room[getArrayIndexFromPosition(gridPosition.x - 1.f, gridPosition.y)])
            minMaxPos[1] = worldPosition; 
    }
    if (worldPosition.y >= minMaxPos[2].y) 
    {
        if (room[getArrayIndexFromPosition(gridPosition.x, gridPosition.y + 1.f)])
            minMaxPos[2] = worldPosition; 
    }
    if (worldPosition.y <= minMaxPos[3].y) 
    {
        if (room[getArrayIndexFromPosition(gridPosition.x, gridPosition.y - 1.f)])
            minMaxPos[3] = worldPosition; 
    }
}

glm::vec2 RoomGenerator::getFreeLarge(glm::vec2 position)
{
    int x = position.x;
    int y = position.y;

    int dY = 1;
    if (room[getArrayIndexFromPosition(x, y + dY)] < 1) //first check tile above
    {
        int dX = 1;
        if (room[getArrayIndexFromPosition(x + dX, y)] < 1 &&
            room[getArrayIndexFromPosition(x + dX, y + dY)] < 1)
            return glm::vec2(dX, dY);
        dX = -1;
        if (room[getArrayIndexFromPosition(x + dX, y)] < 1 &&
            room[getArrayIndexFromPosition(x + dX, y + dY)] < 1)
            return glm::vec2(dX, dY);
    }
    dY = -1;
    if (room[getArrayIndexFromPosition(x, y + dY)] < 1) //then below
    {
        int dX = 1;
        if (room[getArrayIndexFromPosition(x + dX, y)] < 1 &&
            room[getArrayIndexFromPosition(x + dX, y + dY)] < 1)
            return glm::vec2(dX, dY);
        dX = -1;
        if (room[getArrayIndexFromPosition(x + dX, y)] < 1 &&
            room[getArrayIndexFromPosition(x + dX, y + dY)] < 1)
            return glm::vec2(dX, dY);
    }
    return glm::vec2(0); //if no direction has free space, return 0,0
}

glm::vec2 RoomGenerator::getFreeAdjacent(glm::vec2 position, glm::vec2 dir)
{
    if (room[getArrayIndexFromPosition(position.x + dir.x, position.y + dir.y)] < 1) return dir;
    dir *= -1;
    if (room[getArrayIndexFromPosition(position.x + dir.x, position.y + dir.y)] < 1) return dir;
    return glm::vec2(0);
}

const glm::vec2* RoomGenerator::getExitTilesPos() const
{
    return exitTilesPos;
}