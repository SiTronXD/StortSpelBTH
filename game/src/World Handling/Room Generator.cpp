#include "Room Generator.h"
#include <iostream> //TODO: ONLY FOR VISUALIZING ROOM LAYOUT. REMOVE LATER

/*
    Limit num objects (atleast houses)
    Sometimes generates:
        holes (something to do with different size pieces I suspect)
        piece on edge
        border piece not on border

*/

const float RoomGenerator::DEFAULT_TILE_SCALE = 0.04f;

RoomGenerator::RoomGenerator()
{
    //create 2d array representing room and set all room pieces to 0
    this->tiles = std::vector<Tile>();
}

RoomGenerator::~RoomGenerator()
{
    delete[] room;
}

void RoomGenerator::init(int roomSize, int tileTypes)
{
    this->ROOM_SIZE = roomSize;
    this->TILE_TYPES = tileTypes;
    this->HALF_ROOM = this->ROOM_SIZE / 2;
    this->room = new Tile::Type[ROOM_SIZE * ROOM_SIZE];

    this->reset();
}

void RoomGenerator::generateRoom()
{
    //generate first room piece in middle (0,0) and depth 0
    this->addPiece(glm::vec2(0, 0), 0);
}

void RoomGenerator::generateBorders(const bool* hasDoors)
{
    // 4 doors, each with a row & column index (i & j) (in the next for loop)
    int doorGridIndex[4][2]{};
    for (int i = 0; i < 4; i++)
    {
        if (hasDoors[i])
        {
            getIJIndex(getArrayIndexFromPosition(this->minMaxPos[i].x, this->minMaxPos[i].y), doorGridIndex[i]);
        }

#ifdef _DEBUG
        else
        {
            doorGridIndex[i][0] = doorGridIndex[i][1] = -1;
        }
#endif
    }

    int index = -1;
    for (int i = 0; i < this->ROOM_SIZE; i++)
    {
        for (int j = 0; j < this->ROOM_SIZE; j++)
        {
            index = -1;

            // Check that we're behind the door and
            // save index of which door we're currently behind
            if      (hasDoors[0] && j > doorGridIndex[0][0] && i == doorGridIndex[0][1]) { index = 0; }
            else if (hasDoors[1] && j < doorGridIndex[1][0] && i == doorGridIndex[1][1]) { index = 1; }
            else if (hasDoors[2] && j == doorGridIndex[2][0] && i > doorGridIndex[2][1]) { index = 2; }
            else if (hasDoors[3] && j == doorGridIndex[3][0] && i < doorGridIndex[3][1]) { index = 3; }

            // Behind a door? Place tile..
            if (index != -1)
            {
                this->room[i * this->ROOM_SIZE + j] = Tile::OneXOne;

                Tile tile{};
                tile.type = Tile::OneXOne;
                tile.position = glm::vec2(j - this->HALF_ROOM, i - this->HALF_ROOM);
                this->exitPathsTiles.emplace_back(tile);

                // save the positions of the tile if it's at an edge
                if (i == this->ROOM_SIZE - 1 || i == 0 || j == this->ROOM_SIZE - 1 || j == 0)
                {
                    this->exitTilesPos[index] = this->exitPathsTiles.back().position;
                }
            }


            // Place a border-tile on empty tiles
            else if (this->room[i * this->ROOM_SIZE + j] == Tile::Invalid)
            {
                Tile tile{};
                tile.type = Tile::Border;
                tile.position = glm::vec2(j - this->HALF_ROOM, i - this->HALF_ROOM);
                this->borders.push_back(tile);
            }
        }
    }
}

void RoomGenerator::addPiece(glm::vec2 position, int depth)
{
    int x = position.x;
    int y = position.y;
    int index = getArrayIndexFromPosition(x, y);

    //add piece only if tile is within room bounds
    if (abs(x) < this->HALF_ROOM - 1 && abs(y) < this->HALF_ROOM - 1)
    {
        //std::random_device rd; //obtain random number from hardware
        std::mt19937 gen(rand());  //seed generator

        if (this->room[index] < 1) 
        {
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
                this->placeTile(tileType, position, position);
                break;
            }
            case Tile::OneXTwo: //1x2 piece, need to check if adjecent tile is free
            {
                int dY = this->getFreeAdjacent(position, glm::vec2(0, 1)).y;
                if (dY != 0) 
                {
                    this->placeTile(tileType, position, position + glm::vec2(0, 0.5 * dY));
                    this->room[this->getArrayIndexFromPosition(x, y + dY)] = tileType;
                }
                break;
            }
            case Tile::TwoXOne: //2x1
            {
                int dX = this->getFreeAdjacent(position, glm::vec2(1, 0)).x;
                if (dX != 0)
                {
                    this->placeTile(tileType, position, position + glm::vec2(0.5 * dX, 0));
                    this->room[this->getArrayIndexFromPosition(x + dX, y)] = tileType;
                }
                break;
            }
            case Tile::TwoXTwo: //2x2
            {
                glm::vec2 dir = getFreeLarge(position);
                if (dir.x != 0) 
                {
                    this->placeTile(tileType, position, position + glm::vec2(0.5 * dir.x, 0.5 * dir.y));
                    this->room[this->getArrayIndexFromPosition(x + dir.x, y)] = tileType;
                    this->room[this->getArrayIndexFromPosition(x, y + dir.y)] = tileType;
                    this->room[this->getArrayIndexFromPosition(x + dir.x, y + dir.y)] = tileType;
                }
                break;
            }
            default:
                this->placeTile(Tile::OneXOne, position, position);
                break;
            }
        }

        std::uniform_int_distribution<> distr(0, depth * 1); //smaller chance of creating new piece the deeper we are in the tree

        //check if should place tiles above, below, right and left
        for (int i = 0; i < 4; i++)
        {
            //use sin and cos to check adjacent tiles in clockwise order
            int dirX = sin(i * M_PI / 2);
            int dirY = cos(i * M_PI / 2);

            int       newPiece = (distr(gen));
            glm::vec2 nextPos = position + glm::vec2(dirX, dirY);
            if (newPiece <= 1)
            {
                this->addPiece(nextPos, depth + 1);
            }
        }
    }
}

void RoomGenerator::placeTile(Tile::Type tileType, glm::vec2 gridPosition, glm::vec2 worldPosition)
{
    this->room[this->getArrayIndexFromPosition(gridPosition.x, gridPosition.y)] = tileType;

    Tile t;
    t.type = tileType;
    t.position = worldPosition;
    this->tiles.push_back(t);

    if (tileType == Tile::TwoXTwo)
        return;

    // Find extents of room
    if (worldPosition.x >= this->minMaxPos[0].x)
    {
        if (this->room[this->getArrayIndexFromPosition(gridPosition.x + 1.f, gridPosition.y)])
        {
            this->minMaxPos[0] = worldPosition;
        }
    }
    if (worldPosition.x <= this->minMaxPos[1].x)
    {
        if (this->room[this->getArrayIndexFromPosition(gridPosition.x - 1.f, gridPosition.y)])
        {
            this->minMaxPos[1] = worldPosition;
        }
    }
    if (worldPosition.y >= minMaxPos[2].y)
    {
        if (this->room[this->getArrayIndexFromPosition(gridPosition.x, gridPosition.y + 1.f)])
        {
            this->minMaxPos[2] = worldPosition;
        }
    }
    if (worldPosition.y <= minMaxPos[3].y)
    {
        if (this->room[this->getArrayIndexFromPosition(gridPosition.x, gridPosition.y - 1.f)])
        {
            this->minMaxPos[3] = worldPosition;
        }
    }
}

glm::vec2 RoomGenerator::getFreeLarge(glm::vec2 position)
{
    int x = position.x;
    int y = position.y;

    int dY = 1;
    if (this->room[this->getArrayIndexFromPosition(x, y + dY)] < 1) //first check tile above
    {
        int dX = 1;
        if (this->room[this->getArrayIndexFromPosition(x + dX, y)] < 1 && this->room[this->getArrayIndexFromPosition(x + dX, y + dY)] < 1)
        {
            return glm::vec2(dX, dY);
        }

        dX = -1;
        if (this->room[this->getArrayIndexFromPosition(x + dX, y)] < 1 && this->room[this->getArrayIndexFromPosition(x + dX, y + dY)] < 1)
        {
            return glm::vec2(dX, dY);
        }
    }

    dY = -1;
    if (this->room[this->getArrayIndexFromPosition(x, y + dY)] < 1) //then below
    {
        int dX = 1;
        if (this->room[this->getArrayIndexFromPosition(x + dX, y)] < 1 && this->room[this->getArrayIndexFromPosition(x + dX, y + dY)] < 1)
        {
            return glm::vec2(dX, dY);
        }

        dX = -1;
        if (this->room[this->getArrayIndexFromPosition(x + dX, y)] < 1 && this->room[this->getArrayIndexFromPosition(x + dX, y + dY)] < 1)
        {
            return glm::vec2(dX, dY);
        }
    }
    return glm::vec2(0); //if no direction has free space, return 0,0
}

glm::vec2 RoomGenerator::getFreeAdjacent(glm::vec2 position, glm::vec2 dir)
{
    if (this->room[this->getArrayIndexFromPosition(position.x + dir.x, position.y + dir.y)] < 1)
    {
        return dir;
    }

    dir *= -1;

    if (this->room[this->getArrayIndexFromPosition(position.x + dir.x, position.y + dir.y)] < 1)
    {
        return dir;
    }

    return glm::vec2(0);
}

int RoomGenerator::getArrayIndexFromPosition(int x, int y) const
{
    return (y + this->HALF_ROOM) * this->ROOM_SIZE + (x + this->HALF_ROOM);
}

void RoomGenerator::getIJIndex(int index, int* output) const
{
    output[1] = (index / this->ROOM_SIZE);
    output[0] = (index - this->ROOM_SIZE * output[1]);
}

const glm::vec2* RoomGenerator::getExitTilesPos() const
{
    return this->exitTilesPos;
}

int RoomGenerator::getRoomTile(int index) const
{
    return this->room[index];
}

int RoomGenerator::getRoomSize() const
{
    return ROOM_SIZE;
}

int RoomGenerator::getNrTiles() const
{
    return (int)this->tiles.size();
}

int RoomGenerator::getNrBorders() const
{
    return (int)this->borders.size();
}

int RoomGenerator::getNrExitTiles() const
{
    return (int)this->exitPathsTiles.size();
}

const Tile& RoomGenerator::getTile(int index) const
{
    return this->tiles[index];
}

const Tile& RoomGenerator::getBorder(int index) const
{
    return this->borders[index];
}

const Tile& RoomGenerator::getExitTiles(int index) const
{
    return this->exitPathsTiles[index];
}

const glm::vec2* RoomGenerator::getMinMaxPos() const
{
    return this->minMaxPos;
}

void RoomGenerator::reset()
{
    memset(this->room, Tile::Invalid, sizeof(int) * this->ROOM_SIZE * this->ROOM_SIZE);
    this->tiles.clear();
    this->borders.clear();
    this->exitPathsTiles.clear();

    this->minMaxPos[0] = glm::vec2((float)-this->ROOM_SIZE);
    this->minMaxPos[1] = glm::vec2((float)this->ROOM_SIZE);
    this->minMaxPos[2] = glm::vec2((float)-this->ROOM_SIZE);
    this->minMaxPos[3] = glm::vec2((float)this->ROOM_SIZE);

    for (int i = 0; i < 4; i++)
    {
        this->exitTilesPos[i] = glm::vec2(0.f);
    }
}