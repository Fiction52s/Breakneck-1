#include "EditorGateInfo.h"
#include "EditSession.h"
#include <fstream>
#include <SFML/Graphics.hpp>
#include "Enemy_Shard.h"


using namespace sf;
using namespace std;

GateInfo::GateInfo()
	:ISelectable(ISelectable::GATE), thickLine(sf::Quads, 4)
{
	edit = EditSession::GetSession();
	numKeysRequired = -1;
	thickLine[0].color = Color(255, 0, 0, 255);
	thickLine[1].color = Color(255, 0, 0, 255);
	thickLine[2].color = Color(255, 0, 0, 255);
	thickLine[3].color = Color(255, 0, 0, 255);
}

void GateInfo::Deactivate(EditSession *editSession, SelectPtr select)
{
	if (edit != NULL)
	{
		GateInfoPtr g = boost::dynamic_pointer_cast<GateInfo>(select);
		edit = NULL;
		editSession->gates.remove(g);

		g->point0->gate = NULL;
		g->point1->gate = NULL;
	}
}

void GateInfo::Activate(EditSession *editSession, SelectPtr select)
{
	if (edit == NULL)
	{
		GateInfoPtr g = boost::dynamic_pointer_cast<GateInfo>(select);
		edit = editSession;
		editSession->gates.push_back(g);

		g->point0->gate = g;
		g->point1->gate = g;
	}
}

void GateInfo::SetType(const std::string &gType)
{
	if (gType == "black")
	{
		type = Gate::BLACK;
	}
	else if (gType == "keygate")
	{
		type = Gate::KEYGATE;
	}
	else if (gType == "birdfight")
	{
		type = Gate::CRAWLER_UNLOCK;
	}
	else if (gType == "secret")
	{
		type = Gate::SECRET;
	}
	else if (gType == "shard")
	{
		type = Gate::SHARD;
		SetShard(0, 0);
	}
	else if (gType == "crawlerunlock")
	{
		type = Gate::CRAWLER_UNLOCK;
	}
	else if (gType == "nexus1unlock")
	{
		type = Gate::CRAWLER_UNLOCK;
	}
	else
	{
		assert(false);
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

void GateInfo::WriteFile(ofstream &of)
{
	int index0 = 0, index1 = 0;
	TerrainPoint *curr = poly0->pointStart;
	while (curr != NULL)
	{
		if (curr == point0)
		{
			break;
		}
		++index0;
		curr = curr->next;
	}

	curr = poly1->pointStart;
	while (curr != NULL)
	{
		if (curr == point1)
		{
			break;
		}
		++index1;
		curr = curr->next;
	}

	//will eventually spit out the gate value
	//but for now its just a constant to resave all the files
	of << (int)type << " " << poly0->writeIndex << " "
		<< index0 << " " << poly1->writeIndex << " " << index1 << " ";

	if (type == Gate::SHARD)
	{
		of << shardWorld << " " << shardIndex << endl;
	}
}

void GateInfo::UpdateLine()
{
	double width = 5;
	V2d dv0(point0->pos.x, point0->pos.y);
	V2d dv1(point1->pos.x, point1->pos.y);
	V2d along = normalize(dv1 - dv0);
	V2d other(along.y, -along.x);

	V2d leftv0 = dv0 - other * width;
	V2d rightv0 = dv0 + other * width;

	V2d leftv1 = dv1 - other * width;
	V2d rightv1 = dv1 + other * width;

	//cout << "a: " << dv0.x << ", " << dv0.y << ", b: " << dv1.x << ", " << dv1.y << endl;

	//Color c;
	if (type == Gate::BLACK)
	{
		color = Color(200, 200, 200);
	}
	else if (type == Gate::KEYGATE)
	{
		//if(!IsReformingType())
		//	color = Color( 100, 100, 100 );
		//else
		//{
		color = Color(200, 200, 200);
		//}
	}
	else if (type == Gate::SECRET)
	{
		color = Color(255, 0, 0);
	}
	else if (type == Gate::SHARD)
	{
		color = Color(100, 255, 10);
	}
	else if (type == Gate::CRAWLER_UNLOCK)
	{
		color = Color(0, 0, 255);
	}
	thickLine[0].color = color;
	thickLine[1].color = color;
	thickLine[2].color = color;
	thickLine[3].color = color;

	thickLine[0].position = Vector2f(leftv0.x, leftv0.y);
	thickLine[1].position = Vector2f(leftv1.x, leftv1.y);
	thickLine[2].position = Vector2f(rightv1.x, rightv1.y);
	thickLine[3].position = Vector2f(rightv0.x, rightv0.y);

	V2d center = (dv0 + dv1) / 2.0;
	shardSpr.setPosition(Vector2f(center));
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

	target->draw(thickLine);

	if (type == Gate::SHARD)
	{
		target->draw(shardSpr);
	}
}

void GateInfo::DrawPreview(sf::RenderTarget * target)
{
	sf::Vertex thickerLine[4];
	double width = 80;
	V2d dv0(point0->pos.x, point0->pos.y);
	V2d dv1(point1->pos.x, point1->pos.y);
	V2d along = normalize(dv1 - dv0);
	V2d other(along.y, -along.x);

	V2d leftv0 = dv0 - other * width;
	V2d rightv0 = dv0 + other * width;

	V2d leftv1 = dv1 - other * width;
	V2d rightv1 = dv1 + other * width;

	//cout << "a: " << dv0.x << ", " << dv0.y << ", b: " << dv1.x << ", " << dv1.y << endl;

	//Color c;
	if (type == Gate::BLACK)
	{
		color = Color::Cyan;
		//color = Color(150, 150, 150);
	}
	else if (type == Gate::KEYGATE)
	{
		color = Color::Cyan;//Color(100, 100, 100);
	}

	SetRectColor(thickerLine, color);

	thickerLine[0].position = Vector2f(leftv0.x, leftv0.y);
	thickerLine[1].position = Vector2f(leftv1.x, leftv1.y);
	thickerLine[2].position = Vector2f(rightv1.x, rightv1.y);
	thickerLine[3].position = Vector2f(rightv0.x, rightv0.y);

	target->draw(thickerLine, 4, sf::Quads);
}