#include "Room Generator2.h"

RoomGen::RoomGen()
{
	tiles2D = new Tile2::Type[WIDTH_NUM * WIDTH_NUM];
}

RoomGen::~RoomGen()
{
}

void RoomGen::generate()
{
	const glm::ivec2 middle(WIDTH_NUM / 2, WIDTH_NUM / 2);
	genCircle(middle);
	
	std::vector<glm::ivec2> branches(NUM_BRANCHES * BRANCH_LENGTH);
	int c = 0;
	for (uint32_t i = 0; i < NUM_BRANCHES; i++)
	{
		
	}
}

Tile2::Type RoomGen::getType(int xPos, int yPos) const
{
#ifdef _CONSOLE
	int idx = POS_IDX(xPos, yPos);
	if (idx > WIDTH_NUM * WIDTH_NUM || idx < 0)
	{
		Log::error("RoomGen invalid index: " + std::to_string(idx) + 
			". Pos: " + std::to_string(xPos) + ", " + std::to_string(yPos));
	}
#endif

    return tiles2D[POS_IDX(xPos, yPos)];
}

void RoomGen::genCircle(const glm::ivec2& center)
{
	glm::ivec2 start = center - glm::ivec2(CIRCLE_RADIUS);
	glm::ivec2 currentPoint{};
	int idx = -1;

	for (currentPoint.x = start.x; currentPoint.x < start.x + CIRCLE_RADIUS; currentPoint.x++)
	{
		for (currentPoint.y = start.y; currentPoint.y < start.y + CIRCLE_RADIUS; currentPoint.y++)
		{
			idx = POS_IDX(currentPoint.x, currentPoint.y);
			if (idx < WIDTH_NUM * WIDTH_NUM || idx >= -1)
			{
				if (glm::dot(currentPoint, currentPoint) < CIRCLE_RADIUS * CIRCLE_RADIUS)
				{
					tiles2D[idx] = Tile2::Type::OneXOne;
				}
			}
		}
	}
}

int RoomGen::getNrTiles() const
{
	return (int)tiles.size();
}

int RoomGen::getNrBorders() const
{
	return (int)borders.size();
}

int RoomGen::getNrExitTiles() const
{
	return (int)exitPathsTiles.size();
}

Tile2::Type RoomGen::getRoomTile(int index) const
{
	return tiles2D[index];
}

const Tile2& RoomGen::getTile(int index) const
{
	return tiles[index];
}

const Tile2& RoomGen::getBorder(int index) const
{
	return borders[index];
}

const Tile2& RoomGen::getExitTiles(int index) const
{
	return exitPathsTiles[index];
}
