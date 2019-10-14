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


PulserParams::PulserParams(ActorType *at,
	sf::Vector2i &pos,
	std::list<sf::Vector2i> &globalPath, 
	int p_framesBetweenNodes,
	bool p_loop )
	:ActorParams(at )
{
	lines = NULL;
	PlaceAerial(pos);
	SetPath( globalPath );

	framesBetweenNodes = p_framesBetweenNodes; 

	loop = p_loop;
}

PulserParams::PulserParams(ActorType *at,
	ifstream &is)
	:ActorParams(at)
{
	lines = NULL;

	LoadAerial(is);
	LoadMonitor(is);
	LoadGlobalPath(is);

	LoadBool(is, loop);

	is >> framesBetweenNodes;
}

PulserParams::PulserParams(ActorType *at,
	sf::Vector2i &pos )
	:ActorParams(at )
{
	lines = NULL;
	PlaceAerial(pos);

	loop = false;
	//speed = 5;
	framesBetweenNodes = 60;
}

void PulserParams::WriteParamFile( std::ofstream &of )
{
	WriteMonitor(of);

	WritePath(of);
	WriteLoop(of);

	of << framesBetweenNodes << endl;
}

void PulserParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		
		VertexArray &li = *lines;
	
	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position += Vector2f( position.x, position.y );
		}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

	ActorParams::Draw( target );
}

void PulserParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string betweenStr = p->textBoxes["framesbetweennodes"]->text.getString().toAnsiString();
	bool t_loop = p->checkBoxes["loop"]->checked;

	hasMonitor = p->checkBoxes["monitor"]->checked;
	ss << betweenStr;

	int t_framesBetweenNodes;
	ss >> t_framesBetweenNodes;

	if( !ss.fail() )
	{
		framesBetweenNodes = t_framesBetweenNodes;
	}

	ss.clear();

	loop = t_loop;
}

void PulserParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}
	
	p->textBoxes["framesbetweennodes"]->text.setString( boost::lexical_cast<string>( framesBetweenNodes ) );
	p->checkBoxes["loop"]->checked = loop;
	p->checkBoxes["monitor"]->checked = hasMonitor;

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
	//p->checkBoxes["monitor"]->checked = false;
	//EditSession::SetMonitorGrid( monitorType, p->gridSelectors["monitortype"] );
}

ActorParams *PulserParams::Copy()
{
	PulserParams *copy = new PulserParams( *this );
	if( copy->lines != NULL )
	{
		int numVertices = copy->lines->getVertexCount();

		VertexArray &oldli = *copy->lines;
		copy->lines = new VertexArray( sf::LinesStrip, numVertices );
		VertexArray &li = *copy->lines;
		

		for( int i = 0; i < numVertices; ++i )
		{
			li[i] = oldli[i];
		}
	}
	return copy;
}




OwlParams::OwlParams(ActorType *at, sf::Vector2i &pos,
	int p_moveSpeed, int p_bulletSpeed, int p_rhythmFrames )
	:ActorParams( at)
{
	PlaceAerial(pos);

	moveSpeed = p_moveSpeed;
	bulletSpeed = p_bulletSpeed;
	rhythmFrames = p_rhythmFrames;
}

OwlParams::OwlParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
	
	is >> moveSpeed;
	is >> bulletSpeed;
	is >> rhythmFrames;
}

OwlParams::OwlParams(ActorType *at,
	sf::Vector2i &pos )
	:ActorParams(at )
{
	PlaceAerial(pos);

	moveSpeed = 8;
	bulletSpeed = 10;
	rhythmFrames = 60;

}

void OwlParams::WriteParamFile( std::ofstream &of )
{
	WriteMonitor(of);

	of << moveSpeed << " " << bulletSpeed << " " << rhythmFrames << endl;
}

void OwlParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string moveStr = p->textBoxes["movespeed"]->text.getString().toAnsiString();
	string bulletSpeedStr = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
	string rhythmStr = p->textBoxes["rhythmframes"]->text.getString().toAnsiString();

	hasMonitor = p->checkBoxes["monitor"]->checked;
	
	ss << moveStr;

	int t_moveSpeed;
	ss >> t_moveSpeed;

	if( !ss.fail() )
	{
		moveSpeed = t_moveSpeed;
	}

	ss.clear();

	ss << bulletSpeedStr;

	int t_bulletSpeed;
	ss >> t_bulletSpeed;

	if( !ss.fail() )
	{
		bulletSpeed = t_bulletSpeed;
	}

	ss.clear();

	ss << rhythmStr;

	int t_rhythm;
	ss >> t_rhythm;

	if( !ss.fail() )
	{
		rhythmFrames = t_rhythm;
	}
}

void OwlParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}
	
	p->textBoxes["movespeed"]->text.setString( boost::lexical_cast<string>( moveSpeed ) );
	p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( bulletSpeed ) );
	p->textBoxes["rhythmframes"]->text.setString( boost::lexical_cast<string>( rhythmFrames ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *OwlParams::Copy()
{
	OwlParams *copy = new OwlParams( *this );
	return copy;
}



BadgerParams::BadgerParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, int p_speed,
	int p_jumpStrength )
	:ActorParams(at )
{
	speed = p_speed;
	jumpStrength = p_jumpStrength;

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

BadgerParams::BadgerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	//always grounded

	LoadGrounded(is);
	LoadMonitor(is);
	
	is >> speed;
	is >> jumpStrength;
}

BadgerParams::BadgerParams(ActorType *at,
	TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at )
{
	
	speed = 10;
	jumpStrength = 5;

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );	
}

void BadgerParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);
	
	of << speed << " " << jumpStrength << endl;
}

void BadgerParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;

	stringstream ss;
	string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();
	string jumpStrengthStr = p->textBoxes["jumpstrength"]->text.getString().toAnsiString();
	
	ss << speedStr;

	int t_speed;
	ss >> t_speed;

	if( !ss.fail() )
	{
		speed = t_speed;
	}

	ss.clear();

	int t_jumpStrength;
	ss >> t_jumpStrength;

	if( !ss.fail() )
	{
		jumpStrength = t_jumpStrength;
	}
}

void BadgerParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	
	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );
	p->textBoxes["jumpstrength"]->text.setString( boost::lexical_cast<string>( jumpStrength ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *BadgerParams::Copy()
{
	BadgerParams *copy = new BadgerParams( *this );
	return copy;
}



CactusParams::CactusParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity,
	int p_bulletSpeed, int p_rhythm, int p_amplitude )
	:ActorParams( at)
{
	bulletSpeed = p_bulletSpeed;
	rhythm = p_rhythm;
	amplitude = p_amplitude;

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

CactusParams::CactusParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);
	LoadMonitor(is);

	is >> bulletSpeed;
	is >> rhythm;
	is >> amplitude;
}

CactusParams::CactusParams(ActorType *at,
	TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams( at)
{
	bulletSpeed = 5;
	rhythm = 60;
	amplitude = 10;

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}
void CactusParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);
	of << bulletSpeed << " " << rhythm << " " << amplitude << endl;
}

void CactusParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;

	stringstream ss;
	string bulletSpeedStr = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
	string rhythmStr = p->textBoxes["rhythm"]->text.getString().toAnsiString();
	string amplitudeStr = p->textBoxes["amplitude"]->text.getString().toAnsiString();
	
	ss << bulletSpeedStr;

	int t_bulletSpeed;
	ss >> t_bulletSpeed;

	if( !ss.fail() )
	{
		bulletSpeed = t_bulletSpeed;
	}

	ss.clear();

	ss << rhythmStr;

	int t_rhythm;
	ss >> t_rhythm;

	if( !ss.fail() )
	{
		rhythm = t_rhythm;
	}

	ss.clear();

	ss << amplitudeStr;

	int t_amplitude;
	ss >> t_amplitude;

	if( !ss.fail() )
	{
		amplitude = t_amplitude;
	}
}

void CactusParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	
	p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( bulletSpeed ) );
	p->textBoxes["rhythm"]->text.setString( boost::lexical_cast<string>( rhythm ) );
	p->textBoxes["amplitude"]->text.setString( boost::lexical_cast<string>( amplitude ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *CactusParams::Copy()
{
	CactusParams *copy = new CactusParams( *this );
	return copy;
}

BossCoyoteParams::BossCoyoteParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at ), debugLines( sf::Lines, 6 * 2 )
{

	PlaceAerial(pos);
	radius = 600;

	CreateFormation();
}

BossCoyoteParams::BossCoyoteParams(ActorType *at, ifstream &is)
	:ActorParams(at), debugLines(sf::Lines, 6 * 2)
{
	LoadAerial(is);

	radius = 600;
	CreateFormation();
}

//bool BossCoyoteParams::CanApply()
//{
//	CreateFormation();
//	return true;
//}

ActorParams *BossCoyoteParams::Copy()
{
	BossCoyoteParams *copy = new BossCoyoteParams( *this );
	return copy;
}

void BossCoyoteParams::CreateFormation()
{
	Vector2f op( position.x, position.y );

	Transform t;
	Vector2f offset( radius, 0 );

	debugLines[0].position = op + offset;
	t.rotate( -360.f / 6.f );
	for( int i = 1; i < 6; ++i )
	{
		Vector2f newP = t.transformPoint( offset ) + op;
		//Vector2f lastPos;
		debugLines[(i-1)*2 + 1].position = newP;
		debugLines[i*2 + 0].position = newP;

		t.rotate( -360.f / 6.f );
	}
	debugLines[5*2+1].position = debugLines[0].position;

	for( int i = 0; i < 6; ++i )
	{
		debugLines[i*2+0].color = Color::Red;
		debugLines[i*2+1].color = Color::Blue;
	}
}

void BossCoyoteParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );

	target->draw( debugLines );
}

BounceSpringParams::BounceSpringParams(ActorType *at, sf::Vector2i &pos, std::list<sf::Vector2i> &globalPath)
	:ActorParams(at)
{
	PlaceAerial(pos);

	lines = NULL;

	SetPath(globalPath);
}

BounceSpringParams::BounceSpringParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	Vector2i other;
	is >> other.x;
	is >> other.y;

	lines = NULL;

	list<Vector2i> globalPath;
	globalPath.push_back(Vector2i(position.x, position.y));
	globalPath.push_back(position + other);
	SetPath(globalPath);
}

BounceSpringParams::BounceSpringParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	PlaceAerial(pos);

	lines = NULL;
}

void BounceSpringParams::WriteParamFile(std::ofstream &of)
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

void BounceSpringParams::SetPath(std::list<sf::Vector2i> &globalPath)
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

void BounceSpringParams::SetParams()
{
	Panel *p = type->panel;

	/*string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();

	stringstream ss;
	ss << speedStr;

	int t_speed;
	ss >> t_speed;

	if (!ss.fail())
	{
		speed = t_speed;
	}*/

	hasMonitor = false;
}

void BounceSpringParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

//	p->textBoxes["speed"]->text.setString((boost::lexical_cast<string>(speed)));

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *BounceSpringParams::Copy()
{
	BounceSpringParams *copy = new BounceSpringParams(*this);
	return copy;
}

void BounceSpringParams::Draw(sf::RenderTarget *target)
{
	int localPathSize = localPath.size();

	if (localPathSize > 0)
	{
		VertexArray &li = *lines;


		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position += Vector2f(position.x, position.y);
		}


		target->draw(li);

		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position -= Vector2f(position.x, position.y);
		}
	}

	ActorParams::Draw(target);
}