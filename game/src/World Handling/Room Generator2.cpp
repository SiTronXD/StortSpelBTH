#include "Room Generator2.h"
#include "../deps/glm/src/glm/gtx/rotate_vector.hpp"
#include <cmath>

RoomGen::RoomGen()
	:tiles2D{}
{
	exitTilesPos[LEFT_P] = minMaxPos[LEFT_P] = glm::ivec2(-1);
	exitTilesPos[RIGHT_P] = minMaxPos[RIGHT_P] = glm::ivec2(WIDTH_HEIGHT);
	exitTilesPos[UPPER_P] = minMaxPos[UPPER_P] = glm::ivec2(-1);
	exitTilesPos[LOWER_P] = minMaxPos[LOWER_P] = glm::ivec2(WIDTH_HEIGHT);
	
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

void RoomGen::clear()
{
	exitTilesPos[LEFT_P] = minMaxPos[LEFT_P] = glm::ivec2(-1);
	exitTilesPos[RIGHT_P] = minMaxPos[RIGHT_P] = glm::ivec2(WIDTH_HEIGHT);
	exitTilesPos[UPPER_P] = minMaxPos[UPPER_P] = glm::ivec2(-1);
	exitTilesPos[LOWER_P] = minMaxPos[LOWER_P] = glm::ivec2(WIDTH_HEIGHT);

	mainTiles.clear();
	bigTiles.clear();
	borders.clear();
	innerBorders.clear();
	exitPathsTiles.clear();

	for (uint32_t i = 0; i < WIDTH_HEIGHT; i++)
	{
		memset(tiles2D[i], Tile2::Type::Unused, sizeof(Tile2::Type) * WIDTH_HEIGHT);
	}
}

void RoomGen::generate(bool* doors)
{
	const glm::ivec2 gridMid(WIDTH_HEIGHT / 2);
	drawCircle(gridMid, RADIUS);
	//branchEnds.resize(NUM_BRANCHES);

	glm::vec2 fBranch(0);
	glm::ivec2 iBranch(0);
	for (uint32_t i = 0; i < NUM_BRANCHES; i++)
	{
		fBranch.x = float(rand() % 2001) * 0.001f - 1.f;
		fBranch.y = float(rand() % 2001) * 0.001f - 1.f;
		fBranch = glm::normalize(fBranch);
		iBranch = fBranch * (float)BRANCH_DIST;
		iBranch += gridMid;

		for (uint32_t j = 0; j < BRANCH_DEPTH; j++)
		{
			drawCircle(iBranch, RADIUS);
			
			if (MAX_ANGLE != 0)
			{
				const float rotAngle = float(rand() % (int)MAX_ANGLE - (int)MAX_ANGLE / 2);
				fBranch = glm::rotate(fBranch, float(rotAngle * (M_PI / 180.f)));
			}
			iBranch += fBranch * (float)BRANCH_DIST;
		}
	}

	this->findMinMax(); // Find minMax tiles & middle

	// Reserve middle tiles for big rock
	getType(middle) = Tile2::Reserved;
	getType(middle + glm::ivec2(1, 0)) = Tile2::Reserved;
	getType(middle + glm::ivec2(0, 1)) = Tile2::Reserved;
	getType(middle + glm::ivec2(1, 1)) = Tile2::Reserved;

	this->setBigTiles();	// Select spots for 1x2/2x1/2x2
	this->setExits(doors);	// Set doors and make room for them
	this->setBorders();		// Surround the room with borders
	this->finalize();		// Finalize, fill Tile vectors and offset room to origo
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

			if (position.x > this->minMaxPos[LEFT_P].x)
			{
				this->minMaxPos[LEFT_P] = position;
			}
			if (position.x < this->minMaxPos[RIGHT_P].x)
			{
				this->minMaxPos[RIGHT_P] = position;
			}
			if (position.y > this->minMaxPos[UPPER_P].y)
			{
				this->minMaxPos[UPPER_P] = position;
			}
			if (position.y < this->minMaxPos[LOWER_P].y)
			{
				this->minMaxPos[LOWER_P] = position;
			}
		}
	}

	middle.x = (minMaxPos[LEFT_P].x + minMaxPos[RIGHT_P].x) / 2;
	middle.y = (minMaxPos[UPPER_P].y + minMaxPos[LOWER_P].y) / 2;
	size.x = minMaxPos[LEFT_P].x - minMaxPos[RIGHT_P].x;
	size.y = minMaxPos[UPPER_P].y - minMaxPos[LOWER_P].y;
}

void RoomGen::setBigTiles()
{
	//for (auto& pos : branchEnds)
	//{
	//	getType(pos) = Tile2::TwoXTwo;
	//}

	int placedCount2x2 = 0;
	int placedCount1x2 = 0;

	glm::ivec2 position(0);
	for (position.x = minMaxPos[RIGHT_P].x; position.x < minMaxPos[LEFT_P].x; position.x++)
	{
		for (position.y = minMaxPos[LOWER_P].y; position.y < minMaxPos[UPPER_P].y; position.y++)
		{
			if (rand() % 100 < TWO_X_TWO_CHANCE && getType(position) == Tile2::OneXOne) // can remove OneXOne check ?
			{
				if (placedCount2x2 < MAX_TWO_X_TWO)
					placedCount2x2 += tryPlaceTwoXTwo(position);
			}
			else if (rand() % 100 < ONE_X_TWO_CHANCE && getType(position) == Tile2::OneXOne)
			{
				if (placedCount1x2 < MAX_ONE_X_TWO)
					placedCount1x2 += tryPlaceOneXTwo(position, bool(rand() % 2));
			}
		}
	}
}

void RoomGen::setExits(bool* doors)
{
	// TEMP, FIX REAL FUNC
#define rRand2(value) (rand() % value - value / 2)

	glm::ivec2 doorsPos[4] = 
	{
		{ minMaxPos[LEFT_P].x, middle.y + rRand2(size.y / 3) },
		{ minMaxPos[RIGHT_P].x, middle.y + rRand2(size.y / 3) },

		{ middle.x + rRand2(size.x / 3), minMaxPos[UPPER_P].y },
		{ middle.x + rRand2(size.x / 3), minMaxPos[LOWER_P].y }
	};

	const glm::ivec2 offsets[4] =
	{
		{0, RADIUS / 2},
		{0, RADIUS / 2},
		{RADIUS / 2, 0},
		{RADIUS / 2, 0}
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

			drawCircle(doorsPos[i], RADIUS);
			drawCircle(doorsPos[i] + -dirs[i] * (int)RADIUS, RADIUS - 1u);

			doorsPos[i] += dirs[i] * (int)RADIUS;

			if (this->onEdge(doorsPos[i]))
			{
				this->getType(doorsPos[i]) = Tile2::Exit;
				exitTilesPos[i] = doorsPos[i];
			}
			else
			{
				glm::ivec2 lastValid = doorsPos[i];
				for (int j = 1; j < (int)BORDER_SIZE + 1; j++)
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
					for (int j = 1; j < (int)BORDER_SIZE + 1; j++)
					{
						adjacent = position + DIRS[i] * j;
						if (this->isValid(adjacent))
						{
							const Tile2::Type& adj = this->getType(adjacent);
							if (adj == Tile2::OneXOne || adj == Tile2::TwoXTwo ||
								currTile == Tile2::Exit || currTile == Tile2::OneXTwo || currTile == Tile2::TwoXOne)
							{
								currTile = Tile2::Border; 
								i = NUM;
								j = BORDER_SIZE + 1;
							}
						}
					}
				}
			}
		}
	}

	for (position.x = 0; position.x < WIDTH_HEIGHT; position.x++)
	{
		for (position.y = 0; position.y < WIDTH_HEIGHT; position.y++)
		{
			Tile2::Type& currTile = this->getType(position);
			if (currTile == Tile2::OneXOne || currTile == Tile2::Exit || 
				currTile == Tile2::OneXTwo || currTile == Tile2::TwoXOne || currTile == Tile2::TwoXTwo)
			{
				for (int i = 0; i < 4; i++)
				{
					adjacent = position + DIRS[i];
					if (this->isValid(adjacent))
					{
						if (getType(adjacent) == Tile2::Border)
							getType(adjacent) = Tile2::InnerBorder;
					}
				}
			}
		}
	}
}

void RoomGen::finalize()
{
	const glm::vec2 fMiddle = middle;
	glm::vec2 fPosition(0.f);

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
			
			fPosition = position;

			switch (curTile)
			{
			default:
				break;
			case Tile2::Border:
				borders.emplace_back(Tile2::Border, fPosition - fMiddle);
				break;
			case Tile2::InnerBorder:
				innerBorders.emplace_back(Tile2::InnerBorder, fPosition - fMiddle);
				break;

			case Tile2::OneXOne:
				mainTiles.emplace_back(Tile2::OneXOne, fPosition - fMiddle);
				break;

			case Tile2::TwoXOne:
				bigTiles.emplace_back(Tile2::TwoXOne, (fPosition + glm::vec2(0.5f, 0.f)) - fMiddle);
				getType(position + glm::ivec2(1, 0)) = Tile2::Reserved;
				break;

			case Tile2::OneXTwo:
				bigTiles.emplace_back(Tile2::OneXTwo, (fPosition + glm::vec2(0.f, 0.5f)) - fMiddle);
				getType(position + glm::ivec2(0, 1)) = Tile2::Reserved;
				break;

			case Tile2::TwoXTwo:
				bigTiles.emplace_back(Tile2::TwoXTwo, (fPosition + glm::vec2(0.5f)) - fMiddle);
				getType(position + glm::ivec2(1, 0)) = Tile2::Reserved;
				getType(position + glm::ivec2(0, 1)) = Tile2::Reserved;
				getType(position + glm::ivec2(1, 1)) = Tile2::Reserved;
				break;

			case Tile2::Exit:
				exitPathsTiles.emplace_back(Tile2::Exit, fPosition - fMiddle);
				break;
			}
		}
	}

	for (int i = 0; i < 4; i++)
		exitTilesPos[i] -= middle;
}

void RoomGen::drawCircle(const glm::ivec2& center, uint32_t radius)
{
	const glm::ivec2 start = glm::ivec2(center) - glm::ivec2(radius);

	glm::ivec2 currentPoint{};
	for (currentPoint.x = start.x; currentPoint.x < start.x + (int)radius * 2 + 1; currentPoint.x++)
	{
		for (currentPoint.y = start.y; currentPoint.y < start.y + (int)radius * 2 + 1; currentPoint.y++)
		{
			if (this->isValid(currentPoint))
			{
				Tile2::Type& ref = getType(currentPoint);
				if (ref == Tile2::Unused && ref != Tile2::Reserved)
				{
					// check if tile is within RADIUS to center
					// glm::dot only accepts floating point numbers
					const glm::vec2 vec(currentPoint - center);
					if (glm::dot(vec, vec) <= ((float)radius + 0.5f) * ((float)radius + 0.5f))
					{
						ref = Tile2::Type::OneXOne;
					}
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

	glm::ivec2 curPos(0);
	const int thicc = (int)BIG_TILE_MIN_DIST;
	for (curPos.x = pos.x - thicc; curPos.x <= (pos.x + (int)!vertical + thicc); curPos.x++)
	{
		for (curPos.y = pos.y - thicc; curPos.y <= (pos.y + (int)vertical + thicc); curPos.y++)
		{
			if (isValid(curPos))
			{
				const Tile2::Type& ref = getType(curPos);
				if (ref != Tile2::OneXOne && ref != Tile2::Unused)
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool RoomGen::canPlaceTwoXTwo(const glm::ivec2& pos)
{
	if (!isValid(pos) || !isValid(pos + glm::ivec2(1, 0)) || 
		!isValid(pos + glm::ivec2(0, 1)) || !isValid(pos + glm::ivec2(1, 1)))
	{
		return false;
	}

	// can remove ?
	if (getType(pos) != Tile2::OneXOne || getType(pos + glm::ivec2(1, 0)) != Tile2::OneXOne ||
		getType(pos + glm::ivec2(0, 1)) != Tile2::OneXOne || getType(pos + glm::ivec2(1, 1)) != Tile2::OneXOne)
	{
		return false;
	}

	glm::ivec2 curPos(0);
	const int thicc = (int)BIG_TILE_MIN_DIST;
	for (curPos.x = pos.x - thicc; curPos.x <= (pos.x + 1 + thicc); curPos.x++)
	{
		for (curPos.y = pos.y - thicc; curPos.y <= (pos.y + 1 + thicc); curPos.y++)
		{
			if (isValid(curPos))
			{
				const Tile2::Type& ref = getType(curPos);
				if (ref != Tile2::OneXOne && ref != Tile2::Unused)
				{
					return false;
				}
			}
		}
	}
	
	return true;
}

bool RoomGen::tryPlaceTwoXTwo(const glm::ivec2& pos)
{
	if (!canPlaceTwoXTwo(pos)) 
	{
		return false; 
	}

	getType(pos) = Tile2::TwoXTwo;
	getType(pos + glm::ivec2(1, 0)) = Tile2::TwoXTwo;
	getType(pos + glm::ivec2(0, 1)) = Tile2::TwoXTwo;
	getType(pos + glm::ivec2(1, 1)) = Tile2::TwoXTwo;

	return true;
}

bool RoomGen::tryPlaceOneXTwo(const glm::ivec2& pos, bool vertical)
{
	if (!canPlaceOneXTwo(pos, vertical))
	{
		return false;
	}

	const glm::ivec2 adjacent(pos.x + 1 * !vertical, pos.y + 1 * vertical);

	getType(pos)	  = vertical ? Tile2::OneXTwo : Tile2::TwoXOne;
	getType(adjacent) = vertical ? Tile2::OneXTwo : Tile2::TwoXOne;

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

const glm::ivec2& RoomGen::getMiddle() const
{
	return middle;
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

uint32_t RoomGen::getNumMainTiles() const
{
	return (int)mainTiles.size();
}

uint32_t RoomGen::getNumBigTiles() const
{
	return (int)bigTiles.size();
}

uint32_t RoomGen::getNumBorders() const
{
	return (int)borders.size();
}

uint32_t RoomGen::getNumInnerBorders() const
{
	return (int)innerBorders.size();
}

uint32_t RoomGen::getNumExitTiles() const
{
	return (int)exitPathsTiles.size();
}

const Tile2& RoomGen::getMainTile(uint32_t index) const
{
	return mainTiles[index];
}

const Tile2& RoomGen::getBigTile(uint32_t index) const
{
	return bigTiles[index];
}

const Tile2& RoomGen::getBorder(uint32_t index) const
{
	return borders[index];
}

const Tile2& RoomGen::getInnerBorder(uint32_t index) const
{
	return innerBorders[index];
}

const Tile2& RoomGen::getExitTile(uint32_t index) const
{
	return exitPathsTiles[index];
}
