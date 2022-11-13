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

void RoomGen::generate()
{
	const glm::ivec2 gridMid(WIDTH_HEIGHT / 2);
	genCircle(gridMid);

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
			genCircle(iBranch);
			
			if (desc.maxAngle != 0)
			{
				const float rotAngle = float(rand() % (int)desc.maxAngle - (int)desc.maxAngle / 2);
				fBranch = glm::rotate(fBranch, float(rotAngle * (M_PI / 180.f)));
			}
			iBranch += fBranch * (float)desc.branchDist;
		}
	}

	this->setBorders();
	this->setExits();
	this->finalize();
}

const glm::ivec2* RoomGen::getMinMax() const
{
	return minMaxPos;
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

void RoomGen::setExits()
{
	glm::ivec2 position(0);
	for (position.x = 0; position.x < WIDTH_HEIGHT; position.x++)
	{
		for (position.y = 0; position.y < WIDTH_HEIGHT; position.y++)
		{
			Tile2::Type& tile = this->getType(position);

			if (tile != Tile2::OneXOne)
				continue;

			if (position.x > this->minMaxPos[0].x && desc.exits[0])
			{
				this->minMaxPos[0] = position;
			}
			if (position.x < this->minMaxPos[1].x && desc.exits[1])
			{
				this->minMaxPos[1] = position;
			}
			if (position.y > this->minMaxPos[2].y && desc.exits[2])
			{
				this->minMaxPos[2] = position;
			}
			if (position.y < this->minMaxPos[3].y && desc.exits[3])
			{
				this->minMaxPos[3] = position;
			}
		}
	}

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
		if (desc.exits[i])
		{
			this->getType(minMaxPos[i]) = Tile2::OneXOne;
			minMaxPos[i] += offsets[i];

			if (this->onEdge(minMaxPos[i]))
			{
				this->getType(minMaxPos[i]) = Tile2::Exit;
				exitTilesPos[i] = minMaxPos[i] + dirs[i];
			}
			else
			{
				for (int j = 1; j < (int)desc.borderSize + 1; j++)
				{
					adjacent = minMaxPos[i] + dirs[i] * j;
					if (this->isValid(adjacent))
					{
						this->getType(adjacent) = Tile2::Exit;
					}
				}
				exitTilesPos[i] = adjacent + dirs[i];
			}
		}
	}
}

void RoomGen::finalize()
{
	//const glm::ivec2 gridMid(WIDTH_NUM / 2);
	const glm::ivec2 roomMid((minMaxPos[0].x + minMaxPos[1].x) / 2, (minMaxPos[2].y + minMaxPos[3].y) / 2);

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
				borders.emplace_back(Tile2::Border, position - roomMid);
				break;
			case Tile2::OneXOne:
	
				tiles.emplace_back(Tile2::OneXOne, position - roomMid);
				break;
			case Tile2::OneXTwo:

				break;
			case Tile2::TwoXTwo:

				break;
			case Tile2::Exit:
				exitPathsTiles.emplace_back(Tile2::Exit, position - roomMid);
				break;
			}
		}
	}
}

void RoomGen::genCircle(const glm::ivec2& center)
{
	const glm::ivec2 start = glm::ivec2(center) - glm::ivec2(desc.radius);

	glm::ivec2 currentPoint{};
	for (currentPoint.x = start.x; currentPoint.x < start.x + (int)desc.radius * 2 + 1; currentPoint.x++)
	{
		for (currentPoint.y = start.y; currentPoint.y < start.y + (int)desc.radius * 2 + 1; currentPoint.y++)
		{
			if (this->isValid(currentPoint))
			{
				// glm::dot only accepts floating point numbers
				const glm::vec2 vec(currentPoint - center);
				if (glm::dot(vec, vec) <= ((float)desc.radius + 0.5f) * ((float)desc.radius + 0.5f))
				{
					tiles2D[currentPoint.x][currentPoint.y] = Tile2::Type::OneXOne;
				}
			}
		}
	}
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
