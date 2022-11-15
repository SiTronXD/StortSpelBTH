#include "Room Generator2.h"
#include "../deps/glm/src/glm/gtx/rotate_vector.hpp"
#include <cmath>

RoomGen::RoomGen()
	:tiles2D{}
{
	exitTilesPos[0] = minMaxPos[0] = glm::ivec2(-1);
	exitTilesPos[1] = minMaxPos[1] = glm::ivec2(WIDTH_HEIGHT);
	exitTilesPos[2] = minMaxPos[2] = glm::ivec2(-1);
	exitTilesPos[3] = minMaxPos[3] = glm::ivec2(WIDTH_HEIGHT);
	
	tiles2D = new Tile2::Type * [WIDTH_HEIGHT];
	for (uint32_t i = 0; i < WIDTH_HEIGHT; i++)
	{
		tiles2D[i] = new Tile2::Type[WIDTH_HEIGHT];
		memset(tiles2D[i], Tile2::Type::Unused, sizeof(Tile2::Type) * WIDTH_HEIGHT);
	}
}

/*
	TODO: (Not in order)
	* Add support for 1x2 & 2x2 tiles & make sure doors aren't blocked by them
		* Make sure the rock can spawn in the middle
	* Generate multiple rooms
		* Generate paths
		* Generate borders around paths
	* Smoothen out borders
	* Separate the current tile-meshes from their floor
		* Fill the rooms with stuff
	* Only place colliders on first border layer (Fog?)
*/

RoomGen::~RoomGen()
{
	for (uint32_t i = 0; i < WIDTH_HEIGHT; i++)
	{
		delete[] tiles2D[i];
	}
	delete[]tiles2D;
	tiles2D = nullptr;
}

void RoomGen::setDesc(const RoomGenDescription& desc)
{
	this->desc = desc;
}

void RoomGen::clear()
{
	exitTilesPos[0] = minMaxPos[0] = glm::ivec2(-1);
	exitTilesPos[1] = minMaxPos[1] = glm::ivec2(WIDTH_HEIGHT);
	exitTilesPos[2] = minMaxPos[2] = glm::ivec2(-1);
	exitTilesPos[3] = minMaxPos[3] = glm::ivec2(WIDTH_HEIGHT);

	tiles.clear();
	borders.clear();
	exitPathsTiles.clear();

	for (uint32_t i = 0; i < WIDTH_HEIGHT; i++)
	{
		memset(tiles2D[i], Tile2::Type::Unused, sizeof(Tile2::Type) * WIDTH_HEIGHT);
	}
}

void RoomGen::generate(bool* doors)
{
	const glm::ivec2 gridMid(WIDTH_HEIGHT / 2);
	genCircle(gridMid, desc.radius);

	glm::vec2 fBranch(0);
	glm::ivec2 iBranch(0);
	for (uint32_t i = 0; i < desc.numBranches; i++)
	{
		fBranch.x = float(rand() % 2001) * 0.001f - 1.f;
		fBranch.y = float(rand() % 2001) * 0.001f - 1.f;	
		fBranch = glm::normalize(fBranch);
		iBranch = fBranch * (float)desc.branchDist;
		iBranch += gridMid;

		for (uint32_t j = 0; j < desc.branchDepth; j++)
		{
			genCircle(iBranch, desc.radius);
			
			if (desc.maxAngle != 0)
			{
				const float rotAngle = float(rand() % (int)desc.maxAngle - (int)desc.maxAngle / 2);
				fBranch = glm::rotate(fBranch, float(rotAngle * (M_PI / 180.f)));
			}
			iBranch += fBranch * (float)desc.branchDist;
		}
	}

	this->findMinMax();
	this->setExits(doors);
	this->setBorders();
	this->finalize();
}

void RoomGen::setBorders()
{
	const int NUM = 8;
	const glm::ivec2 DIRS[NUM] =
	{
		{ 1,  0},
		{-1,  0},
		{ 0,  1},
		{ 0, -1},

		{ 1,  1},
		{-1,  1},
		{ 1, -1},
		{-1, -1},
	};

	glm::ivec2 position(0);
	glm::ivec2 adjacent(0);
	for (position.x = 0; position.x < WIDTH_HEIGHT; position.x++)
	{
		for (position.y = 0; position.y < WIDTH_HEIGHT; position.y++)
		{
			Tile2::Type& currTile = this->getType(position);
			if (currTile == Tile2::Unused)
			{
				for (uint32_t i = 0; i < NUM; i++)
				{
					for (int j = 1; j < (int)desc.borderSize + 1; j++)
					{
						adjacent = position + DIRS[i] * j;
						if (this->isValid(adjacent))
						{
							const Tile2::Type& adj = this->getType(adjacent);
							if (adj == Tile2::OneXOne)
								currTile = Tile2::Border;
						}
					}
				}
			}
		}
	}
}

void RoomGen::findMinMax()
{
	glm::ivec2 position(0);
	for (position.x = 0; position.x < WIDTH_HEIGHT; position.x++)
	{
		for (position.y = 0; position.y < WIDTH_HEIGHT; position.y++)
		{
			Tile2::Type& tile = this->getType(position);

			if (tile != Tile2::OneXOne)
				continue;

			if (position.x > this->minMaxPos[0].x)
			{
				this->minMaxPos[0] = position;
			}
			if (position.x < this->minMaxPos[1].x)
			{
				this->minMaxPos[1] = position;
			}
			if (position.y > this->minMaxPos[2].y)
			{
				this->minMaxPos[2] = position;
			}
			if (position.y < this->minMaxPos[3].y)
			{
				this->minMaxPos[3] = position;
			}
		}
	}

	middle.x = (minMaxPos[0].x + minMaxPos[1].x) / 2;
	middle.y = (minMaxPos[2].y + minMaxPos[3].y) / 2;
	size.x = minMaxPos[0].x - minMaxPos[1].x;
	size.y = minMaxPos[2].y - minMaxPos[3].y;
}

void RoomGen::setExits(bool* doors)
{
	// TEMP, FIX REAL FUNC
#define rRand2(value) rand() % value - value / 2

	glm::ivec2 doorsPos[4] = 
	{
		{ minMaxPos[0].x, middle.y + rRand2(size.y / 3) },
		{ minMaxPos[1].x, middle.y + rRand2(size.y / 3) },

		{ middle.x + rRand2(size.x / 3), minMaxPos[2].y },
		{ middle.x + rRand2(size.x / 3), minMaxPos[3].y }
	};

	const glm::ivec2 offsets[4] =
	{
		{0, desc.radius / 2},
		{0, desc.radius / 2},
		{desc.radius / 2, 0},
		{desc.radius / 2, 0}
	};

	const glm::ivec2 dirs[4] =
	{
		{ 1,  0},
		{-1,  0},
		{ 0,  1},
		{ 0, -1}
	};

	glm::ivec2 adjacent(0);
	for (int i = 0; i < 4; i++)
	{
		if (doors[i])
		{
			doorsPos[i] += offsets[i];

			genCircle(doorsPos[i], desc.radius);
			genCircle(doorsPos[i] + -dirs[i] * (int)desc.radius, desc.radius);

			doorsPos[i] += dirs[i] * (int)desc.radius;

			if (this->onEdge(doorsPos[i]))
			{
				this->getType(doorsPos[i]) = Tile2::Exit;
				exitTilesPos[i] = doorsPos[i];
			}
			else
			{
				glm::ivec2 lastValid = doorsPos[i];
				for (int j = 1; j < (int)desc.borderSize + 1; j++)
				{
					adjacent = doorsPos[i] + dirs[i] * j;
					if (this->isValid(adjacent))
					{
						this->getType(adjacent) = Tile2::Exit;
						lastValid = adjacent;
					}
				}
				
				exitTilesPos[i] = lastValid + dirs[i];

			}
		}
	}
}

void RoomGen::finalize()
{
	glm::ivec2 position(0);
	for (position.x = 0; position.x < WIDTH_HEIGHT; position.x++)
	{
		for (position.y = 0; position.y < WIDTH_HEIGHT; position.y++)
		{
			Tile2::Type& curTile = getType(position);

			if (this->onEdge(position) && curTile == Tile2::OneXOne)
			{
				curTile = Tile2::Border;
			}

			switch (curTile)
			{
			default:
				break;
			case Tile2::Border:
				borders.emplace_back(Tile2::Border, position - middle);
				break;
			case Tile2::OneXOne:
	
				tiles.emplace_back(Tile2::OneXOne, position - middle);
				break;
			case Tile2::OneXTwo:

				break;
			case Tile2::TwoXTwo:

				break;
			case Tile2::Exit:
				exitPathsTiles.emplace_back(Tile2::Exit, position - middle);
				break;
			}
		}
	}

	for (int i = 0; i < 4; i++)
	{
		exitTilesPos[i] -= middle;
	}
}

void RoomGen::genCircle(const glm::ivec2& center, uint32_t radius)
{
	const glm::ivec2 start = glm::ivec2(center) - glm::ivec2(radius);

	glm::ivec2 currentPoint{};
	for (currentPoint.x = start.x; currentPoint.x < start.x + (int)radius * 2 + 1; currentPoint.x++)
	{
		for (currentPoint.y = start.y; currentPoint.y < start.y + (int)radius * 2 + 1; currentPoint.y++)
		{
			if (this->isValid(currentPoint))
			{
				// glm::dot only accepts floating point numbers
				const glm::vec2 vec(currentPoint - center);
				if (glm::dot(vec, vec) <= ((float)radius + 0.5f) * ((float)radius + 0.5f))
				{
					tiles2D[currentPoint.x][currentPoint.y] = Tile2::Type::OneXOne;
				}
			}
		}
	}
}

bool RoomGen::canPlaceOneXTwo(const glm::ivec2& pos, bool vertical)
{
	const glm::ivec2 adjacent(pos.x + 1 * !vertical, pos.y + 1 * vertical);

	if (!isValid(pos) || !isValid(adjacent))
	{
		return false;
	}

	if (getType(pos) != Tile2::OneXOne || getType(adjacent) != Tile2::OneXOne)
	{
		return false;
	}

	return true;
}

bool RoomGen::canPlaceTwoXTwo(const glm::ivec2& pos)
{
	if (!canPlaceOneXTwo(pos, true) || !canPlaceOneXTwo(pos, false))
	{
		return false;
	}

	if (getType(pos + glm::ivec2(1)) != Tile2::OneXOne)
	{
		return false;
	}

	return true;
}

const glm::ivec2* RoomGen::getMinMax() const
{
	return minMaxPos;
}

const glm::ivec2* RoomGen::getExits() const
{
	return exitTilesPos;
}

glm::ivec2 RoomGen::getCenter() const
{
	return middle - glm::ivec2(WIDTH_HEIGHT / 2);
}

inline Tile2::Type& RoomGen::getType(const glm::ivec2& pos)
{
	return tiles2D[pos.x][pos.y];
}

inline bool RoomGen::isValid(const glm::ivec2& pos)
{
	return (pos.x >= 0 && pos.x < WIDTH_HEIGHT && pos.y >= 0 && pos.y < WIDTH_HEIGHT);
}

inline bool RoomGen::onEdge(const glm::ivec2& pos)
{
	return (pos.x == 0 || pos.x == WIDTH_HEIGHT - 1 ||
			pos.y == 0 || pos.y == WIDTH_HEIGHT - 1);
}

uint32_t RoomGen::getNumTiles() const
{
	return (int)tiles.size();
}

uint32_t RoomGen::getNumBorders() const
{
	return (int)borders.size();
}

uint32_t RoomGen::getNumExitTiles() const
{
	return (int)exitPathsTiles.size();
}

const Tile2& RoomGen::getTile(uint32_t index) const
{
	return tiles[index];
}

const Tile2& RoomGen::getBorder(uint32_t index) const
{
	return borders[index];
}

const Tile2& RoomGen::getExitTile(uint32_t index) const
{
	return exitPathsTiles[index];
}
