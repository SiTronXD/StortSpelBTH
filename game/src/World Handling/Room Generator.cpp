#include "Room Generator.h"
#include <iostream> //TODO: ONLY FOR VISUALIZING ROOM LAYOUT. REMOVE LATER

/*
    Limit num objects (atleast houses)
    Sometimes generates:
        holes (something to do with different size pieces I suspect)
        piece on edge
        border piece not on border
        
*/

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
    room = new int[ROOM_SIZE * ROOM_SIZE];

    reset();
}

void RoomGenerator::generateRoom()
{
    //generate first room piece in middle (0,0) and depth 0
    addPiece(glm::vec2(0, 0), 0);    
}

void RoomGenerator::generateBorders(const bool* hasDoors)
{
    //Add border pieces

    int doorsIndex[4] = 
    {
        hasDoors[0] ? getArrayIndexFromPosition(minMaxPos[0].x, minMaxPos[0].y) : -1,
        hasDoors[1] ? getArrayIndexFromPosition(minMaxPos[1].x, minMaxPos[1].y) : -1,
        hasDoors[2] ? getArrayIndexFromPosition(minMaxPos[2].x, minMaxPos[2].y) : -1,
        hasDoors[3] ? getArrayIndexFromPosition(minMaxPos[3].x, minMaxPos[3].y) : -1
    };

    /*
        
        i-index = index / ROOM_SIZE
        j-index = index - (ROOM_SIZE * i-index)
    */

    int paths[4][2];

    paths[0][0] = hasDoors[0] ? (doorsIndex[0] - ROOM_SIZE * (doorsIndex[0] / ROOM_SIZE)) - 1 : -1;
    paths[0][1] = hasDoors[0] ? (doorsIndex[0] / ROOM_SIZE) : -1;

    paths[1][0] = hasDoors[1] ? (doorsIndex[1] - ROOM_SIZE * (doorsIndex[1] / ROOM_SIZE)) : -1;
    paths[1][1] = hasDoors[1] ? (doorsIndex[1] / ROOM_SIZE) : -1;
    
    paths[2][0] = hasDoors[2] ? (doorsIndex[2] - ROOM_SIZE * (doorsIndex[2] / ROOM_SIZE)) : -1;
    paths[2][1] = hasDoors[2] ? (doorsIndex[2] / ROOM_SIZE) : -1;
    
    paths[3][0] = hasDoors[3] ? (doorsIndex[3] - ROOM_SIZE * (doorsIndex[3] / ROOM_SIZE)) : -1;
    paths[3][1] = hasDoors[3] ? (doorsIndex[3] / ROOM_SIZE) : -1;
    
    printf("paths +x: %d, %d\n", paths[0][0],paths[0][1]);
    printf("paths -x: %d, %d\n", paths[1][0],paths[1][1]);
    printf("paths +z: %d, %d\n", paths[2][0],paths[2][1]);
    printf("paths -z: %d, %d\n", paths[3][0],paths[3][1]);


    for (int i = 0; i < ROOM_SIZE; i++) 
    {
        for (int j = 0; j < ROOM_SIZE; j++) 
        {
            
            if (j >= paths[0][0] && i == paths[0][1] && hasDoors[0])
            {
                room[i * ROOM_SIZE + j] = 1;
                Tile t;
                t.type     = Tile::OneXOne;
                t.position = glm::vec2(j - HALF_ROOM, i - HALF_ROOM);
                tiles.push_back(t);
            }
            if (j <= paths[1][0] && i == paths[1][1] && hasDoors[1])
            {
                room[i * ROOM_SIZE + j] = 1;
                Tile t;
                t.type     = Tile::OneXOne;
                t.position = glm::vec2(j - HALF_ROOM, i - HALF_ROOM);
                tiles.push_back(t);
            }
            if (j == paths[2][0] && i > paths[2][1] && hasDoors[2])
            {
                room[i * ROOM_SIZE + j] = 1;
                Tile t;
                t.type     = Tile::OneXOne;
                t.position = glm::vec2(j - HALF_ROOM, i - HALF_ROOM);
                tiles.push_back(t);
            }
            if (j == paths[3][0] && i < paths[3][1] && hasDoors[3])
            {
                room[i * ROOM_SIZE + j] = 1;
                Tile t;
                t.type     = Tile::OneXOne;
                t.position = glm::vec2(j - HALF_ROOM, i - HALF_ROOM);
                tiles.push_back(t);
            } 
        }
    }


    for (int i = 0; i < ROOM_SIZE; i++) 
    {
        for (int j = 0; j < ROOM_SIZE; j++) 
        {
            if (room[i * ROOM_SIZE + j] == 0)
            {
                Tile t;
                t.type     = Tile::Border;
                t.position = glm::vec2(j - HALF_ROOM, i - HALF_ROOM);
                tiles.push_back(t);
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
    if (abs(x) < HALF_ROOM && abs(y) < HALF_ROOM) 
    {
        std::random_device rd; //obtain random number from hardware
        std::mt19937       gen(rd()); //seed generator
        if (room[index] < 1) {
            std::uniform_int_distribution<> tileTypeRange(1, 10); //TODO: Update when more pieces exists
            Tile::Type tileType = Tile::Type(tileTypeRange(gen));
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
        if (room[getArrayIndexFromPosition(gridPosition.x + 1.f, gridPosition.y)] != Tile::TwoXTwo)
            minMaxPos[0] = worldPosition; 
    }
    if (worldPosition.x <= minMaxPos[1].x) 
    {
        if (room[getArrayIndexFromPosition(gridPosition.x - 1.f, gridPosition.y)] != Tile::TwoXTwo)
            minMaxPos[1] = worldPosition; 
    }
    if (worldPosition.y >= minMaxPos[2].y) 
    {
        if (room[getArrayIndexFromPosition(gridPosition.x, gridPosition.y + 1.f)] != Tile::TwoXTwo)
            minMaxPos[2] = worldPosition; 
    }
    if (worldPosition.y <= minMaxPos[3].y) 
    {
        if (room[getArrayIndexFromPosition(gridPosition.x, gridPosition.y - 1.f)] != Tile::TwoXTwo)
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