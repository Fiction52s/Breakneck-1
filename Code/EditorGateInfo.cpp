#include "EditorGateInfo.h"
#include "EditSession.h"
#include <fstream>
#include <SFML/Graphics.hpp>
#include "Enemy_Shard.h"
#include "ActorParams.h"


using namespace sf;
using namespace std;

double GateInfo::lineWidth = 10;//5;
double GateInfo::selectWidth = 100;//5;

GateInfo::GateInfo()
	:ISelectable(ISelectable::GATE)
{
	edit = EditSession::GetSession();
	variation = 0;
	category = 0;
	numToOpen = 0;
	seconds = 0;
	//to its easier to track down errors
	shardWorld = -1;
	shardIndex = -1;
}

void GateInfo::SetNumToOpen(int num)
{
	if (category == Gate::NUMBER_KEY || category == Gate::PICKUP)
	{
		numToOpen = num;
	}
	else
	{
		numToOpen = 0;
	}
}

void GateInfo::SetTime(int sec)
{
	if (category == Gate::TIME_GLOBAL ||
		category == Gate::TIME_ROOM)
	{
		seconds = sec;
	}
	else
	{
		seconds = 0;
	}
}

bool GateInfo::HasSameInfo(GateInfo *other)
{
	if (other->category != category)
		return false;

	if (other->variation != variation)
		return false;

	if (category == Gate::NUMBER_KEY || category == Gate::PICKUP)
	{
		if (other->numToOpen != numToOpen)
			return false;
	}
	else if (category == Gate::SHARD)
	{
		if (shardWorld != other->shardWorld
			|| shardIndex != other->shardIndex)
		{
			return false;
		}
	}

	return true;
}

void GateInfo::Deactivate()
{
	if (edit != NULL)
	{
		active = false;
		edit->gateInfoList.remove(this);
		edit = NULL;
		point0->gate = NULL;
		point1->gate = NULL;
	}
}

void GateInfo::Activate()
{
	if (edit == NULL)
	{
		active = true;
		edit = EditSession::GetSession();
		edit->gateInfoList.push_back(this);

		point0->gate = this;
		point1->gate = this;
	}
}

void GateInfo::SetShard(int shardW, int shardI)
{
	shardWorld = shardW;
	shardIndex = shardI;

	Tileset *ts_shard = Shard::GetShardTileset(shardWorld, edit);
	shardSpr.setTexture(*ts_shard->texture);
	shardSpr.setTextureRect(ts_shard->GetSubRect(shardIndex));
	shardSpr.setOrigin(shardSpr.getLocalBounds().width / 2,
		shardSpr.getLocalBounds().height / 2);
}

bool GateInfo::IsTouchingEnemy(ActorParams * a)
{
	assert(poly0 != NULL && poly1 != NULL);
	sf::VertexArray &bva = a->boundingQuad;
	
	V2d p0(point0->pos);
	V2d p1(point1->pos);
	if( IsEdgeTouchingQuad( p0, p1, V2d(bva[0].position.x, bva[0].position.y),
		V2d(bva[1].position.x, bva[1].position.y), V2d(bva[2].position.x, bva[2].position.y),
		V2d(bva[3].position.x, bva[3].position.y)))
	{
		return true;
	}

	return false;
}

bool GateInfo::ContainsPoint( V2d &p )
{
	assert(poly0 != NULL && poly1 != NULL);

	V2d start(point0->pos);
	V2d end(point1->pos);
	V2d along = normalize(end - start);
	V2d other(along.y, -along.x);

	double halfThickness = selectWidth / 2.0;
	
	V2d a = start + other * halfThickness;
	V2d b = start - other * halfThickness;
	V2d c = end - other * halfThickness;
	V2d d = end + other * halfThickness;

	return QuadContainsPoint(a, b, c, d, p);
}

sf::IntRect GateInfo::GetAABB()
{
	int left = min(point0->pos.x, point1->pos.x);
	int right = max(point0->pos.x, point1->pos.x);

	int top = min(point0->pos.y, point1->pos.y);
	int bot = max(point0->pos.y, point1->pos.y);

	return IntRect(left, top, right - left, bot - top);
}

void GateInfo::WriteFile(ofstream &of)
{
	of << category << " ";
	of << variation << " ";

	//allkey should not export numToOpen but it needs to be like this currently to not mess up old gate maps
	if ( category == Gate::ALLKEY || category == Gate::NUMBER_KEY || category == Gate::PICKUP)
	{
		of << numToOpen << " ";
	}

	of << poly0->writeIndex << " ";
	of << point0->GetIndex() << " ";
	of << poly1->writeIndex << " ";
	of << point1->GetIndex();

	if (category == Gate::SHARD)
	{
		of << " " << shardWorld << " " << shardIndex << endl;
	}
	else if (category == Gate::TIME_GLOBAL
		|| category == Gate::TIME_ROOM)
	{
		of << " " << seconds << endl;
	}
	else
	{
		of << "\n";
	}
}

void GateInfo::UpdateLine()
{
	double halfWidth = lineWidth / 2.0;
	V2d dv0(point0->pos.x, point0->pos.y);
	V2d dv1(point1->pos.x, point1->pos.y);
	V2d along = normalize(dv1 - dv0);
	V2d other(along.y, -along.x);

	V2d leftv0 = dv0 - other * halfWidth;
	V2d rightv0 = dv0 + other * halfWidth;

	V2d leftv1 = dv1 - other * halfWidth;
	V2d rightv1 = dv1 + other * halfWidth;

	

	/*if (category == Gate::BLACK)
	{
		color = Color(200, 200, 200);
	}
	else if (category == Gate::ALLKEY || category == Gate::NUMBER_KEY)
	{
		color = Color(200, 200, 200);
	}
	else if (category == Gate::ENEMY)
	{
		color = Color::Magenta;
	}
	else if (category == Gate::TIME_GLOBAL
		|| category == Gate::TIME_ROOM)
	{
		color == Color::Yellow;
	}
	else if (category == Gate::SECRET)
	{
		color = Color(255, 0, 0);
	}
	else if (category == Gate::SHARD)
	{
		color = Color(100, 255, 10);
	}
	else if (category == Gate::BOSS)
	{
		color = Color(0, 0, 255);
	}
	thickLine[0].color = color;
	thickLine[1].color = color;
	thickLine[2].color = color;
	thickLine[3].color = color;*/

	SetRectSubRect(thickLine, edit->createGatesModeUI->ts_gateCategories->GetSubRect(category));

	thickLine[0].position = Vector2f(leftv0.x, leftv0.y);
	thickLine[1].position = Vector2f(leftv1.x, leftv1.y);
	thickLine[2].position = Vector2f(rightv1.x, rightv1.y);
	thickLine[3].position = Vector2f(rightv0.x, rightv0.y);

	V2d center = (dv0 + dv1) / 2.0;
	shardSpr.setPosition(Vector2f(center));

	line[0].position = Vector2f(point0->pos.x, point0->pos.y);
	line[1].position = Vector2f(point1->pos.x, point1->pos.y);
	
	TerrainPolygon::EdgeAngleType eat = TerrainPolygon::GetEdgeAngleType(other);
	Color edgeColor;
	switch (eat)
	{
	case TerrainPolygon::EDGE_FLAT:
		edgeColor = Color::Red;
		break;
	case TerrainPolygon::EDGE_SLOPED:
		edgeColor = Color::Green;
		break;
	case TerrainPolygon::EDGE_STEEPSLOPE:
		edgeColor = Color::White;
		break;
	case TerrainPolygon::EDGE_WALL:
		edgeColor = Color::Magenta;
		break;
	case TerrainPolygon::EDGE_STEEPCEILING:
		edgeColor = Color::Yellow;
		break;
	case TerrainPolygon::EDGE_SLOPEDCEILING:
		edgeColor = Color::Cyan;
		break;
	case TerrainPolygon::EDGE_FLATCEILING:
		edgeColor = Color::Red;
		break;
	}

	line[0].color = edgeColor;
	line[1].color = edgeColor;
}

void GateInfo::Draw(sf::RenderTarget *target)
{

	CircleShape cs(5);
	cs.setFillColor(color);
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);

	cs.setPosition(point0->pos.x, point0->pos.y);
	target->draw(cs);

	cs.setPosition(point1->pos.x, point1->pos.y);
	target->draw(cs);

	target->draw(thickLine, 4, sf::Quads, edit->createGatesModeUI->ts_gateCategories->texture);

	if (category == Gate::SHARD)
	{
		target->draw(shardSpr);
	}
}

void GateInfo::DrawPreview(sf::RenderTarget * target)
{
	sf::Vertex thickerLine[4];
	double width = 30;
	V2d dv0(point0->pos.x, point0->pos.y);
	V2d dv1(point1->pos.x, point1->pos.y);
	V2d along = normalize(dv1 - dv0);
	V2d other(along.y, -along.x);

	V2d leftv0 = dv0 - other * width;
	V2d rightv0 = dv0 + other * width;

	V2d leftv1 = dv1 - other * width;
	V2d rightv1 = dv1 + other * width;

	SetRectSubRect(thickerLine, edit->createGatesModeUI->ts_gateCategories->GetSubRect(category));
	//cout << "a: " << dv0.x << ", " << dv0.y << ", b: " << dv1.x << ", " << dv1.y << endl;

	//Color c;
	if (category == Gate::BLACK)
	{
		color = Color::Cyan;
		//color = Color(150, 150, 150);
	}
	else if (category == Gate::NUMBER_KEY || category == Gate::ALLKEY)
	{
		color = Color::Cyan;//Color(100, 100, 100);
	}
	
	
	/*SetRectColor(thickerLine, color);*/

	thickerLine[0].position = Vector2f(leftv0.x, leftv0.y);
	thickerLine[1].position = Vector2f(leftv1.x, leftv1.y);
	thickerLine[2].position = Vector2f(rightv1.x, rightv1.y);
	thickerLine[3].position = Vector2f(rightv0.x, rightv0.y);

	target->draw(thickerLine, 4, sf::Quads, edit->createGatesModeUI->ts_gateCategories->texture);
}

void GateInfo::DrawSecretPreview(sf::RenderTarget *target)
{
	Vector2f p0(point0->pos.x, point0->pos.y);
	Vector2f p1(point1->pos.x, point1->pos.y);

	line[0].position = p0;
	line[1].position = p1;

	target->draw(line, 2, sf::Lines);
}