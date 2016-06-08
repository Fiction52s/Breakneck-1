#include "EditSession.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Physics.h"
#include <sstream>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace sf;

#define cout std::cout
#define V2d sf::Vector2<double>

#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )


PulserParams::PulserParams( EditSession *edit,
	sf::Vector2i &pos,
	std::list<sf::Vector2i> &globalPath, 
	int p_framesBetweenNodes,
	bool p_loop )
	:ActorParams( PosType::AIR_ONLY )
{
	lines = NULL;
	position = pos;	
	type = edit->types["pulser"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	SetPath( globalPath );

	framesBetweenNodes = p_framesBetweenNodes; 

	loop = p_loop;

	SetBoundingQuad();
}

PulserParams::PulserParams( EditSession *edit,
	sf::Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY )
{
	lines = NULL;
	position = pos;	
	type = edit->types["pulser"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
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



OwlParams::OwlParams( EditSession *edit, sf::Vector2i &pos,
	int moveSpeed, int bulletSpeed, int rhythmFrames )
	:ActorParams( PosType::AIR_ONLY )
{
	position = pos;	
	type = edit->types["owl"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	framesBetweenNodes = p_framesBetweenNodes; 

	loop = p_loop;

	SetBoundingQuad();
}

OwlParams::OwlParams( EditSession *edit,
	sf::Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY )
{
	lines = NULL;
	position = pos;	
	type = edit->types["pulser"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	loop = false;
	//speed = 5;
	framesBetweenNodes = 60;

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

	of << moveSpeed << " " << bulletSpeed << " " << rhythm << endl;
}

void OwlParams::Draw( sf::RenderTarget *target )
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
		rhythm = t_rhythm;
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
	p->textBoxes["rhythmframes"]->text.setString( boost::lexical_cast<string>( rhythm ) );

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