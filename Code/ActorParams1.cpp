#include "EditSession.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Physics.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "ActorParams.h"

using namespace std;
using namespace sf;

#define cout std::cout

#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )


BossCrawlerParams::BossCrawlerParams(ActorType *at, PolyPtr p_edgePolygon,
	int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( at)
{
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

BossCrawlerParams::BossCrawlerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);
}

ActorParams *BossCrawlerParams::Copy()
{
	BossCrawlerParams *copy = new BossCrawlerParams( *this );
	return copy;
}

BoosterParams::BoosterParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> strength;
}

BoosterParams::BoosterParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0,0));

	strength = 10;
}

void BoosterParams::WriteParamFile(std::ofstream &of)
{
	of << strength << "\n";
}

void BoosterParams::SetParams()
{
	Panel *p = type->panel;

	string strengthStr = p->textBoxes["strength"]->text.getString().toAnsiString();

	stringstream ss;
	ss << strengthStr;

	int t_strength;
	ss >> t_strength;

	if (!ss.fail())
	{
		strength = t_strength;
	}
	//hasMonitor = p->checkBoxes["monitor"]->checked;
}

void BoosterParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

	p->textBoxes["strength"]->text.setString(boost::lexical_cast<string>(strength));
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *BoosterParams::Copy()
{
	BoosterParams *copy = new BoosterParams(*this);
	return copy;
}

SpringParams::SpringParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> speed;

	lines = NULL;

	Vector2i other;
	is >> other.x;
	is >> other.y;

	vector<Vector2i> globalPath;
	globalPath.reserve(2);
	Vector2i intPos = GetIntPos();
	globalPath.push_back(intPos);
	globalPath.push_back(intPos + other);
	SetPath(globalPath);

	
}

SpringParams::SpringParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0,0));

	speed = 60;

	lines = NULL;


}

void SpringParams::WriteParamFile(std::ofstream &of)
{
	of << speed << "\n";
	of << localPath.front().x << " " << localPath.front().y << endl;
}

void SpringParams::SetPath(std::vector<sf::Vector2i> &globalPath)
{
	ActorParams::SetPath(globalPath);
	if (globalPath.size() > 1)
	{
		VertexArray &li = *lines;
		Vector2f diff = li[1].position - li[0].position;
		float f = GetVectorAngleCW(diff);
		float rot = f / PI * 180.f + 90;
		image.setRotation(rot);
	}
}

void SpringParams::SetParams()
{
	Panel *p = type->panel;

	string moveFrameStr = p->textBoxes["speed"]->text.getString().toAnsiString();

	stringstream ss;
	ss << moveFrameStr;

	int t_speed;
	ss >> t_speed;

	if (!ss.fail())
	{
		speed = t_speed;
	}


	hasMonitor = false;

}

void SpringParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

	p->textBoxes["speed"]->text.setString((boost::lexical_cast<string>(speed)));

	EditSession *edit = EditSession::GetSession();
	MakeGlobalPath(edit->patrolPath);
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}
ActorParams *SpringParams::Copy()
{
	SpringParams *copy = new SpringParams(*this);
	return copy;
}

void SpringParams::Draw(sf::RenderTarget *target)
{
	int localPathSize = localPath.size();

	if (localPathSize > 0)
	{
		VertexArray &li = *lines;

		Vector2f fPos = GetFloatPos();
		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position += fPos;
		}


		target->draw(li);

		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position -= fPos;
		}
	}

	ActorParams::Draw(target);
}