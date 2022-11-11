#include "Room Generator2.h"
#include "vengine/VengineMath.hpp"

RoomGen::RoomGen()
{
	tiles2D = new Tile2::Type[WIDTH_NUM * WIDTH_NUM];
}

RoomGen::~RoomGen()
{
	delete[] tiles2D;
}

void RoomGen::generate()
{
	const glm::ivec2 ;
	std::vector<glm::ivec2> points(NUM_BRANCH_MAIN * NUM_BRANCH_ITER + 1);
	const glm::ivec2& middle = points[0] = glm::ivec2(WIDTH_NUM / 2, WIDTH_NUM / 2);
	
	glm::vec2 fbranch;
	glm::ivec2 ibranch;

	int c = 0;
	for (uint32_t i = 1; i < NUM_BRANCH_MAIN; i++)
	{
		fbranch.x = rand() % 2001 * 0.001f - 1.f;
		fbranch.y = rand() % 2001 * 0.001f - 1.f;
		ibranch = glm::normalize(fbranch) * (float)BRANCH_LENGTH;

		points[i] = middle + ibranch;
	}
	uint32_t count = 0;
	for (uint32_t i = NUM_BRANCH_MAIN; i < NUM_BRANCH_MAIN * NUM_BRANCH_ITER + 1; i++)
	{
		fbranch.x = rand() % 2001 * 0.001f - 1.f;
		fbranch.y = rand() % 2001 * 0.001f - 1.f;
		ibranch = glm::normalize(fbranch) * (float)BRANCH_LENGTH;

		points[i] = points[count++] + ibranch;
	}
	//genCircle(middle);

	for (glm::ivec2& point : points)
	{
		genCircle(point);
	}


	const glm::ivec2 OFFSET(WIDTH_NUM / 2);
	for (int i = 0; i < WIDTH_NUM * WIDTH_NUM; i++)
	{
		switch (tiles2D[i])
		{
		default:
			break;
		case Tile2::Type::Border:
			
			break;
		case Tile2::Type::OneXOne:
			tiles.emplace_back(Tile2::Type::OneXOne, IDX_TO_POS(i) - OFFSET);
			break;
		case Tile2::Type::OneXTwo:
			
			break;
		case Tile2::Type::TwoXTwo:

			break;
		}
	}
}

Tile2::Type RoomGen::getType(int xPos, int yPos) const
{
#ifdef _CONSOLE
	int idx = POS_TO_IDX(xPos, yPos);
	if (idx > WIDTH_NUM * WIDTH_NUM || idx < 0)
	{
		Log::error("RoomGen invalid index: " + std::to_string(idx) + 
			". Pos: " + std::to_string(xPos) + ", " + std::to_string(yPos));
	}
#endif

    return tiles2D[POS_TO_IDX(xPos, yPos)];
}

void RoomGen::genCircle(const glm::ivec2& center)
{
	glm::vec2 start = glm::vec2(center) - glm::vec2(CIRCLE_RADIUS);
	glm::ivec2 currentPoint{};
	int idx = -1;

	for (currentPoint.x = start.x; currentPoint.x < start.x + CIRCLE_RADIUS * 2 + 1; currentPoint.x++)
	{
		for (currentPoint.y = start.y; currentPoint.y < start.y + CIRCLE_RADIUS * 2 + 1; currentPoint.y++)
		{
			idx = POS_TO_IDX(currentPoint.x, currentPoint.y);
			if (idx < WIDTH_NUM * WIDTH_NUM || idx > -1)
			{
				const glm::vec2 vec(glm::vec2(currentPoint) - glm::vec2(center));
				if (glm::dot(vec, vec) <= ((float)CIRCLE_RADIUS + 0.5f) * ((float)CIRCLE_RADIUS + 0.5f))
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
