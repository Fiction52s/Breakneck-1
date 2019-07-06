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


PatrollerParams::PatrollerParams(  sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop )
	:ActorParams()
{	
	lines = NULL;
	position = pos;	
	type = EditSession::GetSession()->types["patroller"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	SetPath( globalPath );

	loop = p_loop;
	speed = p_speed;

	SetBoundingQuad();
	//ss << localPath.size();
	//params.push_back( ss.str() );
	//ss.str( "" );

	/*for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );*/
}

PatrollerParams::PatrollerParams( 
	sf::Vector2i &pos )
	:ActorParams( )
{	
	lines = NULL;
	position = pos;	
	type = EditSession::GetSession()->types["patroller"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	loop = false;
	speed = 10;

	SetBoundingQuad();

	//image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	//SetPath( globalPath );

	//loop = p_loop;
	//speed = p_speed;

	//SetBoundingQuad();

	//ss << localPath.size();
	//params.push_back( ss.str() );
	//ss.str( "" );

	/*for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );*/
}

void PatrollerParams::SetParams()
{
	Panel *p = type->panel;

	bool loop = p->checkBoxes["loop"]->checked;
	

	string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();

	stringstream ss;
	ss << speedStr;

	int t_speed; 
	ss >> t_speed;

	if( !ss.fail() )
	{
		speed = t_speed;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
	//try
	//{
	//	speed = boost::lexical_cast<int>( p->textBoxes["speed"]->text.getString().toAnsiString() );
	//}
	//catch(boost::bad_lexical_cast &)
	//{
	//	//error
	//}
}

void PatrollerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );
	p->checkBoxes["loop"]->checked = loop;
	p->checkBoxes["monitor"]->checked = hasMonitor;

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
}

bool PatrollerParams::CanApply()
{
	return true;
	//see note for keyparams
}

void PatrollerParams::SetPath(std::list<sf::Vector2i> &globalPath)
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

void PatrollerParams::Draw( sf::RenderTarget *target )
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
	//target->draw( image );

	//DrawBoundar
}

std::list<sf::Vector2i> PatrollerParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
}

void PatrollerParams::WriteParamFile( ofstream &of )
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

	//of.precision( 5 );
	of << speed << endl;
	//of << fixed << speed << endl;
}

ActorParams *PatrollerParams::Copy()
{
	PatrollerParams *copy = new PatrollerParams( *this );
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

CrawlerParams::CrawlerParams(  TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, bool p_clockwise, float p_speed )
	:ActorParams()
{
	clockwise = p_clockwise;
	speed = p_speed;

	type = EditSession::GetSession()->types["crawler"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

CrawlerParams::CrawlerParams(  
		TerrainPolygon *p_edgePolygon,
		int p_edgeIndex, double p_edgeQuantity )
		:ActorParams( ), clockwise( true ), speed( 5 )
{
	type = EditSession::GetSession()->types["crawler"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

CrawlerParams::CrawlerParams( EditSession *edit )
	:ActorParams( ), clockwise( true ), speed( 0 )
{
	
	type = EditSession::GetSession()->types["crawler"];
}

void CrawlerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );
	
	p->checkBoxes["monitor"]->checked = hasMonitor;
}

void CrawlerParams::SetParams()
{
	Panel *p = type->panel;

	clockwise = p->checkBoxes["clockwise"]->checked;
	double sp;

	stringstream ss;
	string s = p->textBoxes["speed"]->text.getString().toAnsiString();
	ss << s;


	ss >> sp;

	if( !ss.fail() )
	{
		speed = sp;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;

	//s = p->textBoxes["dist"]->text.getString().toAnsiString();
	//ss
	dist = 1000;
}

bool CrawlerParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void CrawlerParams::WriteParamFile( ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
	if( clockwise )
		of << "+clockwise" << endl;
	else
		of << "-clockwise" << endl;
	
	of << speed << endl;

	of << dist << endl;
}

ActorParams *CrawlerParams::Copy()
{
	CrawlerParams *copy = new CrawlerParams( *this );
	return copy;
}





BossCrawlerParams::BossCrawlerParams(  TerrainPolygon *p_edgePolygon, 
	int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( )
{
	type = EditSession::GetSession()->types["bosscrawler"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

bool BossCrawlerParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void BossCrawlerParams::WriteParamFile( ofstream &of )
{
	//no params its a boss!
}

ActorParams *BossCrawlerParams::Copy()
{
	BossCrawlerParams *copy = new BossCrawlerParams( *this );
	return copy;
}




BasicTurretParams::BasicTurretParams(  TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, double p_bulletSpeed, int p_framesWait )
	:ActorParams( )
{
	bulletSpeed = p_bulletSpeed;
	framesWait = p_framesWait;

	type = EditSession::GetSession()->types["basicturret"];
	
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

BasicTurretParams::BasicTurretParams(  TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( )
{
	bulletSpeed = 10;
	framesWait = 60;

	type = EditSession::GetSession()->types["basicturret"];
	
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

bool BasicTurretParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void BasicTurretParams::WriteParamFile( ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
	of << bulletSpeed << endl;
	of << framesWait << endl;
}

void BasicTurretParams::SetParams()
{
	Panel *p = type->panel;
	stringstream ss;
	string bulletSpeedString = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
	string framesWaitString = p->textBoxes["waitframes"]->text.getString().toAnsiString();
	ss << bulletSpeedString;
			
	
	double bSpeed;
	ss >> bSpeed;

	if( !ss.fail() )
	{
		bulletSpeed = bSpeed;
		//assert( false );
	}

	ss.clear();

	ss << framesWaitString;

	int fWait;
	ss >> fWait;

	if( !ss.fail() )
	{
		framesWait = fWait;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void BasicTurretParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( bulletSpeed ) );
	p->textBoxes["waitframes"]->text.setString( boost::lexical_cast<string>( framesWait ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *BasicTurretParams::Copy()
{
	BasicTurretParams *copy = new BasicTurretParams( *this );
	return copy;
}




FootTrapParams::FootTrapParams(  TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( )	
{
	type = EditSession::GetSession()->types["foottrap"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

bool FootTrapParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void FootTrapParams::SetParams()
{
	Panel *p = type->panel;
	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void FootTrapParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

void FootTrapParams::WriteParamFile( ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
}

ActorParams *FootTrapParams::Copy()
{
	FootTrapParams *copy = new FootTrapParams( *this );
	return copy;
}

AirdasherParams::AirdasherParams( sf::Vector2i &pos)
	:ActorParams()
{
	position = pos;
	type = EditSession::GetSession()->types["airdasher"];

	image = type->GetSprite(false);
	image.setPosition(pos.x, pos.y);

	SetBoundingQuad();
}

void AirdasherParams::WriteParamFile(std::ofstream &of)
{
	int hMon;
	if (hasMonitor)
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
}

void AirdasherParams::SetParams()
{
	Panel *p = type->panel;
	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void AirdasherParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool AirdasherParams::CanApply()
{
	return true;
}

ActorParams *AirdasherParams::Copy()
{
	AirdasherParams *copy = new AirdasherParams(*this);
	return copy;
}
