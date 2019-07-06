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
	:ActorParams( )
{
	lines = NULL;
	position = pos;	
	type = EditSession::GetSession()->types["pulser"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	SetPath( globalPath );

	framesBetweenNodes = p_framesBetweenNodes; 

	loop = p_loop;

	SetBoundingQuad();
}

PulserParams::PulserParams(ActorType *at,
	sf::Vector2i &pos )
	:ActorParams( )
{
	lines = NULL;
	position = pos;	
	type = EditSession::GetSession()->types["pulser"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	loop = false;
	//speed = 5;
	framesBetweenNodes = 60;

	SetBoundingQuad();
}

void PulserParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;

	of << localPath.size() << endl;

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		of << (*it).x  << " " << (*it).y << endl;
	}

	if( loop )
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}

	of << framesBetweenNodes << endl;
}

void PulserParams::SetPath( std::list<sf::Vector2i> &globalPath )
{
	if( lines != NULL )
	{
		delete lines;
		lines = NULL;
	}
	
	localPath.clear();
	if( globalPath.size() > 1 )
	{

		int numLines = globalPath.size();
	
		lines = new VertexArray( sf::LinesStrip, numLines );
		VertexArray &li = *lines;
		li[0].position = Vector2f( 0, 0 );
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f( temp.x, temp.y );
			li[index].color = Color::Magenta;
			++index;
		}
	}
}

std::list<sf::Vector2i> PulserParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
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

bool PulserParams::CanApply()
{
	return true;
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
	:ActorParams( )
{
	position = pos;	
	type = EditSession::GetSession()->types["owl"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	moveSpeed = p_moveSpeed;
	bulletSpeed = p_bulletSpeed;
	rhythmFrames = p_rhythmFrames;

	SetBoundingQuad();
}

OwlParams::OwlParams(ActorType *at,
	sf::Vector2i &pos )
	:ActorParams( )
{
	position = pos;	
	type = EditSession::GetSession()->types["owl"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	moveSpeed = 8;
	bulletSpeed = 10;
	rhythmFrames = 60;

	SetBoundingQuad();
}

void OwlParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;

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

bool OwlParams::CanApply()
{
	return true;
}

ActorParams *OwlParams::Copy()
{
	OwlParams *copy = new OwlParams( *this );
	return copy;
}



BadgerParams::BadgerParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, int p_speed,
	int p_jumpStrength )
	:ActorParams( )
{
	speed = p_speed;
	jumpStrength = p_jumpStrength;

	type = EditSession::GetSession()->types["badger"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

BadgerParams::BadgerParams(ActorType *at,
	TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams( )
{
	
	speed = 10;
	jumpStrength = 5;
	type = EditSession::GetSession()->types["badger"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

bool BadgerParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void BadgerParams::WriteParamFile( ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
	
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
	:ActorParams( )
{
	bulletSpeed = p_bulletSpeed;
	rhythm = p_rhythm;
	amplitude = p_amplitude;

	type = EditSession::GetSession()->types["cactus"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

CactusParams::CactusParams(ActorType *at,
	TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams( )
{
	bulletSpeed = 5;
	rhythm = 60;
	amplitude = 10;
	
	type = EditSession::GetSession()->types["cactus"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

bool CactusParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void CactusParams::WriteParamFile( ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
	
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
	:ActorParams( ), debugLines( sf::Lines, 6 * 2 )
{
	type = EditSession::GetSession()->types["bosscoyote"];

	position = pos;

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	radius = 600;

	//AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	

	CreateFormation();
}

bool BossCoyoteParams::CanApply()
{
	CreateFormation();
	return true;
}

void BossCoyoteParams::WriteParamFile( ofstream &of )
{
	//no params its a boss!
}

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