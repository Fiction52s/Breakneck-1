#include "Rail.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Rail::Rail(GameSession *p_owner, sf::Vector2i &pos,
	std::list<sf::Vector2i> &path, bool energized)
	:owner( p_owner )
{
	path.push_back(Vector2i(0, 0));
	for (auto it = path.begin(); it != path.end(); ++it)
	{
		(*it) += pos;
	}
	int pathSize = path.size();

	ts_rail = owner->GetTileset("rail_32x32.png", 32, 32);
	

	if (pathSize == 1)
	{
		edges = NULL;
		va = NULL;
	}
	else
	{
		numEdges = pathSize - 1;
		va = new Vertex[numEdges * 4];
		edges = new Edge*[numEdges];
		for (int i = 0; i < numEdges; ++i)
		{
			edges[i] = new Edge;
		}

		int ind = 0;
		auto it = path.begin();
		Edge *curr = NULL;
		double width = 32;
		double hw = width / 2;
		for (int i = 0; i < numEdges; ++it)
		{
			curr = edges[i];

			curr->v0 = V2d((*it).x, (*it).y);
			++it;
			curr->v1 = V2d((*it).x, (*it).y);
			assert(it != path.end());
			owner->railTree->Insert(curr);

			V2d norm = curr->Normal();

			V2d A = curr->v0 + norm * hw;
			V2d B = curr->v1 + norm * hw;
			V2d C = curr->v1 - norm * hw;
			V2d D = curr->v0 - norm * hw;
			
			va[i * 4 + 0].position = Vector2f(A.x, A.y);
			va[i * 4 + 1].position = Vector2f(B.x, B.y);
			va[i * 4 + 2].position = Vector2f(C.x, C.y);
			va[i * 4 + 3].position = Vector2f(D.x, D.y);
		}
	}
}

void Rail::UpdateSprite()
{
	//and different if a player is attached
	int ind = 0;
	if (energized)
		ind = 1;
	IntRect sub = ts_rail->GetSubRect(ind);

	for (int i = 0; i < numEdges; ++i)
	{
		va[i * 4 + 0].texCoords = Vector2f(sub.left, sub.top);
		va[i * 4 + 1].texCoords = Vector2f(sub.left + sub.width, sub.top);
		va[i * 4 + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
		va[i * 4 + 3].texCoords = Vector2f(sub.left, sub.top + sub.height);
	}
}

void Rail::Draw(sf::RenderTarget *target)
{
	target->draw(va, numEdges * 4, sf::Quads, ts_rail->texture);
}