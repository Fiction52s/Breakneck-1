#include "Rail.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Rail::Rail(GameSession *p_owner, sf::Vector2i &pos,
	std::list<sf::Vector2i> &path, bool energized)
	:owner( p_owner )
{
	path.push_front(Vector2i(0, 0));
	for (auto it = path.begin(); it != path.end(); ++it)
	{
		(*it) += pos;
	}
	int pathSize = path.size();

	ts_rail = owner->GetTileset("rail_32x32.png", 32, 32);
	
	drawNext = NULL;

	if (pathSize == 1)
	{
		edges = NULL;
		va = NULL;
	}
	else
	{
		double left, right, top, bottom;
		numEdges = pathSize - 1;
		va = new Vertex[numEdges * 4];
		edges = new Edge*[numEdges];
		for (int i = 0; i < numEdges; ++i)
		{
			edges[i] = new Edge;
			edges[i]->info = (void*)this;
		}

		int ind = 0;
		auto it = path.begin();
		Edge *curr = NULL;
		double width = 32;
		double hw = width / 2;
		int subIndex = 0;
		if (energized)
		{
			subIndex = 1;
		}
		IntRect sub = ts_rail->GetSubRect(subIndex);
		for (int i = 0; i < numEdges; ++i)
		{

			curr = edges[i];

			if (i > 0)
				curr->edge0 = edges[i - 1];
			if (i < numEdges - 1)
			{
				curr->edge1 = edges[i + 1];
			}

			curr->v0 = V2d((*it).x, (*it).y);
			++it;

			curr->v1 = V2d((*it).x, (*it).y);
			assert(it != path.end());
			owner->railEdgeTree->Insert(curr);

			V2d norm = curr->Normal();

			V2d A = curr->v0 - norm * hw;
			V2d B = curr->v0 + norm * hw;
			V2d C = curr->v1 + norm * hw;
			V2d D = curr->v1 - norm * hw;
			
			va[i * 4 + 0].position = Vector2f(A.x, A.y);
			va[i * 4 + 1].position = Vector2f(B.x, B.y);
			va[i * 4 + 2].position = Vector2f(C.x, C.y);
			va[i * 4 + 3].position = Vector2f(D.x, D.y);

			/*va[i * 4 + 0].texCoords = Vector2f(sub.left, sub.top);
			va[i * 4 + 1].texCoords = Vector2f(sub.left + sub.width, sub.top);
			va[i * 4 + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
			va[i * 4 + 3].texCoords = Vector2f(sub.left, sub.top + sub.height);*/
		}

		Edge *tEdge = edges[0];
		left = min( tEdge->v0.x, tEdge->v1.x );
		right = max(tEdge->v0.x, tEdge->v1.x);
		top = min( tEdge->v0.y, tEdge->v1.y );
		bottom = max(tEdge->v0.y, tEdge->v1.y );

		for (int i = 1; i < numEdges; ++i)
		{
			tEdge = edges[i];
			left = min(left, min(tEdge->v0.x, tEdge->v1.x));
			right = max(right, max(tEdge->v0.x, tEdge->v1.x));
			top = min( top, min(tEdge->v0.y, tEdge->v1.y) );
			bottom = max( bottom, max(tEdge->v0.y, tEdge->v1.y) );
		}

		aabb.left = left;
		aabb.top = top;
		aabb.width = right - left;
		aabb.height = bottom - top;
	}

	owner->railDrawTree->Insert(this);
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

void Rail::HandleQuery(QuadTreeCollider * qtc)
{
	qtc->HandleEntrant(this);
}

bool Rail::IsTouchingBox(const sf::Rect<double> &r)
{
	return r.intersects(aabb);
}