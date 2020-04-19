#include "GameSession.h"
#include "SpecialTerrain.h"

using namespace std;
using namespace sf;

SpecialTerrainPiece::SpecialTerrainPiece(GameSession *p_owner)
	:owner( p_owner )
{
	specialType = SPECIAL_TERRAIN_GLIDEWATER;
	groundva = NULL;
	slopeva = NULL;
	steepva = NULL;
	wallva = NULL;
	triva = NULL;
	tr = NULL;
	terrainVA = NULL;
}

SpecialTerrainPiece::~SpecialTerrainPiece()
{
	if (tr != NULL)
		delete tr;

	delete terrainVA;

	for (int i = 0; i < numPoints; ++i)
	{
		delete edges[i];
	}

	delete[] edges;

	delete edgeTree;
}

void SpecialTerrainPiece::GenerateCenterMesh()
{
	//vector<p2t::Point*> polyline;
	//V2d testP;
	vector<vector<Vector2i>> pointVector;
	pointVector.resize(1);
	vector<Vector2i> &currVector = pointVector[0];

	currVector.resize(numPoints);
	for (int i = 0; i < numPoints; ++i)
	{
		currVector[i] = Vector2i(GetEdge(i)->v0);
	}

	std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(pointVector);

	int vaSize = indices.size();
	int numTris = vaSize / 3;

	terrainVA = new VertexArray(sf::Triangles, vaSize);

	VertexArray & v = *terrainVA;
	Color testColor(0x75, 0x70, 0x90);
	testColor = Color(255, 0, 0, 50);//Color::White;

	for (int i = 0; i < numTris; ++i)
	{
		v[i * 3] = Vertex(Vector2f(currVector[indices[i*3]]), testColor);
		v[i * 3 + 1] = Vertex(Vector2f(currVector[indices[i * 3+1]]), testColor);
		v[i * 3 + 2] = Vertex(Vector2f(currVector[indices[i * 3+2]]), testColor);
	}
}

void SpecialTerrainPiece::Reset()
{

}

void SpecialTerrainPiece::HandleEntrant(QuadTreeEntrant *qte)
{

}

void SpecialTerrainPiece::Draw(sf::RenderTarget *target)
{
	/*sf::RectangleShape rs;
	rs.setFillColor(Color(255, 0, 0, 50));
	rs.setSize(Vector2f(aabb.width, aabb.height));
	rs.setOrigin(rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2);
	rs.setPosition(center.x, center.y);
	target->draw(rs);*/

	if (owner->usePolyShader)
	{
		target->draw(*terrainVA);
		//assert(pShader != NULL);
		//target->draw(*terrainVA, pShader);
	}
	else
	{
		target->draw(*terrainVA);
	}
}

void SpecialTerrainPiece::HandleQuery(QuadTreeCollider * qtc)
{
	qtc->HandleEntrant(this);
}

bool SpecialTerrainPiece::IsTouchingBox(const sf::Rect<double> &r)
{
	return IsBoxTouchingBox(aabb, r);
}

Edge *SpecialTerrainPiece::GetEdge(int index)
{
	return edges[index];
}

bool SpecialTerrainPiece::IsInsideArea(V2d &point)
{
	if (!aabb.contains(point))
	{
		return false;
	}

	insideQueryPoint = point;
	

	V2d closest;

	double extra = 10;
	if (insideQueryPoint.x - aabb.left > aabb.width / 2)
	{
		closest.x = aabb.left + aabb.width + extra;
	}
	else
	{
		closest.x = aabb.left - extra;
	}

	if (insideQueryPoint.y - aabb.top > aabb.height / 2)
	{
		closest.y = aabb.top + aabb.height + extra;
	}
	else
	{
		closest.y = aabb.top - extra;
	}

	rcEdge = NULL;
	numEdgesHit = 0;
	RayCast(this, edgeTree->startNode, insideQueryPoint, closest);

	if (numEdgesHit % 2 == 1)
	{
		return true;
	}

	return false;
}

void SpecialTerrainPiece::UpdateAABB()
{
	double left, top, right, bot;
	Edge *curr = GetEdge(0);
	left = curr->v0.x;
	right = curr->v0.x;
	top = curr->v0.y;
	bot = curr->v0.y;

	for (int i = 1; i < numPoints; ++i)
	{
		curr = GetEdge(i);
		left = min(left, curr->v0.x);
		right = max(right, curr->v0.x);
		top = min(top, curr->v0.y);
		bot = max(bot, curr->v0.y);
	}

	aabb.left = left;
	aabb.top = top;
	aabb.width = right - left;
	aabb.height = bot - top;
}

bool SpecialTerrainPiece::Load( ifstream &is )
{
	int matWorld;
	int matVariation;
	is >> matWorld;
	is >> matVariation;

	specialType = (SpecialTerrainType)matVariation;

	int polyPoints;
	is >> polyPoints;

	list<V2d> specialPoints;
	for (int j = 0; j < polyPoints; ++j)
	{
		int x, y, special;
		is >> x;
		is >> y;
		specialPoints.push_back(V2d(x, y));
	}

	numPoints = specialPoints.size();
	edges = new Edge*[numPoints];

	int ind = 0;
	Edge *e;
	for (auto it = specialPoints.begin(); it != specialPoints.end(); ++it)
	{
		e = new Edge;
		e->v0 = (*it);
		edges[ind] = e;
		++ind;
	}

	Edge *prevEdge = NULL;
	Edge *nextEdge = NULL;
	for (int i = 0; i < numPoints; ++i)
	{
		if (i == 0)
		{
			prevEdge = GetEdge(numPoints - 1);
		}
		else
		{
			prevEdge = GetEdge(i - 1);
		}

		if (i == numPoints - 1)
		{
			nextEdge = GetEdge(0);
		}
		else
		{
			nextEdge = GetEdge(i + 1);
		}

		e = GetEdge(i);
		e->v1 = nextEdge->v0;
		e->edge0 = prevEdge;
		e->edge1 = nextEdge;
	}

	UpdateAABB();

	double extra = 10;
	center = V2d(aabb.left + aabb.width / 2.0, aabb.top + aabb.height / 2.0);
	edgeTree = new QuadTree(aabb.width + extra, aabb.height + extra, center );

	for (int i = 0; i < numPoints; ++i)
	{
		edgeTree->Insert(GetEdge(i));
	}

	GenerateCenterMesh();

	return true;
}

void SpecialTerrainPiece::HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion)
{
	if ( rcEdge == NULL 
		|| ( length(edge->GetPosition(edgeQuantity) - insideQueryPoint) 
	> length(rcEdge->GetPosition(rcQuant) - insideQueryPoint)))
	{
		rcEdge = edge;
		rcQuant = edgeQuantity;
	}

	++numEdgesHit;
}