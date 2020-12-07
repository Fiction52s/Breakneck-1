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


CoralParams::CoralParams(ActorType *at, sf::Vector2i &pos,
	int p_moveFrames )
	:ActorParams(at )
{
	moveFrames = p_moveFrames;
	PlaceAerial(pos);
}

CoralParams::CoralParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at )
{
	moveFrames = 60;
	PlaceAerial(pos);
}

CoralParams::CoralParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);

	is >> moveFrames;
}


void CoralParams::WriteParamFile( std::ofstream &of )
{
	WriteMonitor(of);
	of << moveFrames << endl;
}

void CoralParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string moveFramesStr = p->textBoxes["moveframes"]->text.getString().toAnsiString();
	
	ss << moveFramesStr;

	int t_moveFrames;
	ss >> t_moveFrames;

	if( !ss.fail() )
	{
		moveFrames = t_moveFrames;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void CoralParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->checkBoxes["monitor"]->checked = hasMonitor;

	p->textBoxes["moveframes"]->text.setString( 
		boost::lexical_cast<string>( moveFrames ) );
}

ActorParams *CoralParams::Copy()
{
	CoralParams *copy = new CoralParams( *this );
	return copy;
}




TurtleParams::TurtleParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at )
{
	PlaceAerial(pos);
}

TurtleParams::TurtleParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
}

void TurtleParams::WriteParamFile( std::ofstream &of )
{
	WriteMonitor(of);
}

void TurtleParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void TurtleParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *TurtleParams::Copy()
{
	TurtleParams *copy = new TurtleParams( *this );
	return copy;
}

CheetahParams::CheetahParams(ActorType *at,
	PolyPtr p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at )
{
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

CheetahParams::CheetahParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);
}

void CheetahParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);
}

void CheetahParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void CheetahParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *CheetahParams::Copy()
{
	CheetahParams *copy = new CheetahParams( *this );
	return copy;
}



SpiderParams::SpiderParams(ActorType *at, PolyPtr p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, int p_speed )
	:ActorParams(at )
{
	speed = p_speed;

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

SpiderParams::SpiderParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);
	LoadMonitor(is);
	is >> speed;
}

SpiderParams::SpiderParams(ActorType *at,
	PolyPtr p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at )
{
	speed = 10;

	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

void SpiderParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);
	
	of << speed << endl;
}

void SpiderParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;

	stringstream ss;
	string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();
	
	ss << speedStr;

	int t_speed;
	ss >> t_speed;

	if( !ss.fail() )
	{
		speed = t_speed;
	}
}

void SpiderParams::SetPanelInfo()
{
	Panel *p = type->panel;
	
	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *SpiderParams::Copy()
{
	SpiderParams *copy = new SpiderParams( *this );
	return copy;
}

BossTigerParams::BossTigerParams(ActorType *at, Vector2i &pos )
	:ActorParams(at ), debugLines( sf::Lines, 30 * 2 )
{
	PlaceAerial(pos);

	radius1 = 800;
	radius2 = 1600;
				
	
}

BossTigerParams::BossTigerParams(ActorType *at, ifstream &is)
	:ActorParams(at), debugLines(sf::Lines, 30 * 2)
{
	LoadAerial(is);

	radius1 = 800;
	radius2 = 1600;
}

void BossTigerParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );

	target->draw( debugLines );
}

void BossTigerParams::CreateFormation()
{
	sf::Vector2f nodePos[13];
	//allNodes[0]->position = V2d( originalPos.x, originalPos.y );
	Vector2f fPos = GetFloatPos();

	Transform t;
	Vector2f offset( 0, -radius1 );
	for( int i = 0; i < 6; ++i )
	{
		Vector2f newP = t.transformPoint( offset ) + fPos;
		nodePos[i+1] = newP;
		t.rotate( -360.f / 6.f );
	}

	Transform t2;
	Vector2f offset2( 0, -radius2 );
	t2.rotate( 360.f / 12.f );
	for( int i = 0; i < 6; ++i )
	{
		Vector2f newP = t2.transformPoint( offset2 ) + fPos;
		nodePos[i+7] = newP;
		t2.rotate( -360.f / 6.f );
	}

	for( int i = 0; i < 6; ++i )
	{
		Vector2f nextPos = nodePos[i+1];
		
		debugLines[i * 2 + 0].position = fPos;
		debugLines[i*2 + 1].position = Vector2f( nextPos.x, nextPos.y );
	}

	//layer 1 and the connections to layer 2
	int debugIndex = 6;
	for( int i = 0; i < 6; ++i )
	{
		int prev = i - 1;
		if( prev < 0 )
			prev += 6;
		int next = i + 1;
		if( next > 5 )
		{
			next -= 6;
		}

		int prev2 = i;
		int next2 = i + 1;
		if( next2 > 5 )
			next2 -= 6;


		Vector2f a = nodePos[prev+1];
		Vector2f b = nodePos[i+1];
		Vector2f c = nodePos[next+1];
		Vector2f d = nodePos[i+1];
		Vector2f e = nodePos[prev2+7];
		Vector2f f = nodePos[i+1];
		Vector2f g = nodePos[next2+7];
		Vector2f h = nodePos[i+1];

		debugLines[debugIndex * 2 + 0].position = Vector2f( a.x, a.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( b.x, b.y );
		debugIndex++;

		debugLines[debugIndex * 2 + 0].position = Vector2f( e.x, e.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( f.x, f.y );
		debugIndex++;		

		debugLines[debugIndex * 2 + 0].position = Vector2f( g.x, g.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( h.x, h.y );
		debugIndex++;
	}

	//layer 2 connections to layer 2
	int layer2Start = 7;
	for( int i = 0; i < 6; ++i )
	{
		int next = i + 1;
		if( next > 5 )
			next -= 6;

		Vector2f a = nodePos[layer2Start + i];
		Vector2f b = nodePos[layer2Start + next];

		debugLines[debugIndex * 2 + 0].position = Vector2f( b.x, b.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( a.x, a.y );
		debugIndex++;
	}

	for( int i = 0; i < 30; ++i )
	{
		debugLines[i*2+0].color = Color::Red;
		debugLines[i*2+1].color = Color::Blue;
	}
}

//bool BossTigerParams::CanApply()
//{
//	CreateFormation();
//	return true;
//}


ActorParams *BossTigerParams::Copy()
{
	BossTigerParams *copy = new BossTigerParams( *this );
	return copy;
}

RailParams::RailParams(ActorType *at, sf::Vector2i pos, vector<sf::Vector2i> &globalPath,
	bool p_accelerate, int p_level )
	:ActorParams(at)
{
	lines = NULL;
	
	PlaceAerial(pos);

	SetPath(globalPath);

	accelerate = p_accelerate;
	
	enemyLevel = p_level;
}

RailParams::RailParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	lines = NULL;
	LoadAerial(is);
	LoadGlobalPath(is);

	LoadBool(is, accelerate);

	LoadEnemyLevel(is);
}

RailParams::RailParams(ActorType *at,
	sf::Vector2i &pos)
	:ActorParams(at)
{
	lines = NULL;
	PlaceAerial(pos);
	accelerate = false;
}

void RailParams::SetParams()
{
	Panel *p = type->panel;

	//accelerate = p->checkBoxes["accelerate"]->checked;

	hasMonitor = false;
}

void RailParams::SetPanelInfo()
{
	Panel *p = type->panel;
	//p->checkBoxes["accelerate"]->checked = accelerate;

	EditSession *edit = EditSession::GetSession();
	MakeGlobalPath(edit->patrolPath);
}

void RailParams::Draw(sf::RenderTarget *target)
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

void RailParams::WriteParamFile(ofstream &of)
{
	WritePath(of);
	WriteBool(of, accelerate);
	WriteLevel(of);
}

ActorParams *RailParams::Copy()
{
	RailParams *rp = new RailParams(*this);
	return rp;
}

TeleporterParams::TeleporterParams(ActorType *at, sf::Vector2i &pos, std::vector<sf::Vector2i> &globalPath )
	:ActorParams(at)
{
	PlaceAerial(pos);

	lines = NULL;

	SetPath(globalPath);
}

TeleporterParams::TeleporterParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	Vector2i other;
	is >> other.x;
	is >> other.y;

	lines = NULL;

	vector<Vector2i> globalPath;

	Vector2i intPos = GetIntPos();

	globalPath.reserve(2);
	globalPath.push_back(intPos);
	globalPath.push_back(intPos + other);
	SetPath(globalPath);
}

TeleporterParams::TeleporterParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	PlaceAerial(pos);

	lines = NULL;
}

void TeleporterParams::WriteParamFile(std::ofstream &of)
{
	if (localPath.size() == 0)
	{
		of << 0 << " " << 0 << endl;
	}
	else
	{
		of << localPath.front().x << " " << localPath.front().y << endl;
	}

}

void TeleporterParams::SetPath(std::vector<sf::Vector2i> &globalPath)
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

void TeleporterParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = false;
}

void TeleporterParams::SetPanelInfo()
{
	Panel *p = type->panel;

	EditSession *edit = EditSession::GetSession();
	MakeGlobalPath(edit->patrolPath);
}

ActorParams *TeleporterParams::Copy()
{
	TeleporterParams *copy = new TeleporterParams(*this);
	return copy;
}

void TeleporterParams::Draw(sf::RenderTarget *target)
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