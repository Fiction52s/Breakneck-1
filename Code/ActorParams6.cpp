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

SpecterParams::SpecterParams(  sf::Vector2i &pos )
	:ActorParams( )
{
	position = pos;	
	type = EditSession::GetSession()->types["specter"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

void SpecterParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
}

void SpecterParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void SpecterParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool SpecterParams::CanApply()
{
	return true;
}

ActorParams *SpecterParams::Copy()
{
	SpecterParams *copy = new SpecterParams( *this );
	return copy;
}


CopycatParams::CopycatParams(  sf::Vector2i &pos )
	:ActorParams( )
{
	position = pos;	
	type = EditSession::GetSession()->types["copycat"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

void CopycatParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
}

void CopycatParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void CopycatParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool CopycatParams::CanApply()
{
	return true;
}

ActorParams *CopycatParams::Copy()
{
	CopycatParams *copy = new CopycatParams( *this );
	return copy;
}

GorillaParams::GorillaParams( 
		sf::Vector2i &pos, int p_wallWidth,
		int p_followFrames )
		:ActorParams( )
{
	wallWidth = p_wallWidth;
	followFrames = p_followFrames;
	position = pos;	
	type = EditSession::GetSession()->types["gorilla"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

GorillaParams::GorillaParams(  sf::Vector2i &pos )
	:ActorParams( )
{
	wallWidth = 400;
	followFrames = 60;
	position = pos;	
	type = EditSession::GetSession()->types["gorilla"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

void GorillaParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;

	of << wallWidth << endl;
	of << followFrames << endl;
}

void GorillaParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string wallWidthStr = p->textBoxes["wallwidth"]->text.getString().toAnsiString();
	string followFramesStr = p->textBoxes["followframes"]->text.getString().toAnsiString();

	hasMonitor = p->checkBoxes["monitor"]->checked;
	
	ss << wallWidthStr;

	int t_wallWidth;
	ss >> t_wallWidth;

	if( !ss.fail() )
	{
		wallWidth = t_wallWidth;
	}

	int t_followFrames;
	ss >> t_followFrames;

	if( !ss.fail() )
	{
		followFrames = t_followFrames;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void GorillaParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->textBoxes["wallwidth"]->text.setString( boost::lexical_cast<string>( wallWidth ) );
	p->textBoxes["followframes"]->text.setString( boost::lexical_cast<string>( followFrames ) );
	

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool GorillaParams::CanApply()
{
	return true;
}

ActorParams *GorillaParams::Copy()
{
	GorillaParams *copy = new GorillaParams( *this );
	return copy;
}

NarwhalParams::NarwhalParams( 
		sf::Vector2i &pos, sf::Vector2i & p_dest,
		int p_moveFrames )
		:ActorParams( )
{
	dest = p_dest;
	moveFrames = p_moveFrames;
	position = pos;	
	type = EditSession::GetSession()->types["narwhal"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

NarwhalParams::NarwhalParams(  sf::Vector2i &pos )
	:ActorParams( )
{
	dest = pos + Vector2i( 500, 0 );
	moveFrames = 60;
	position = pos;	
	type = EditSession::GetSession()->types["narwhal"];

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

void NarwhalParams::SetPath( std::list<sf::Vector2i> &globalPath )
{
	if( globalPath.size() > 1 )
	{
		dest = globalPath.back();
	}
}

void NarwhalParams::Draw( RenderTarget *target )
{
	ActorParams::Draw( target );

	//if( position != dest )
	//{
	sf::Vertex line[] = { 
		sf::Vertex( Vector2f( position.x, position.y ), Color::Green ),
		sf::Vertex( Vector2f( dest.x, dest.y ), Color::Green ) };
	target->draw( line, 2, sf::Lines );
	//}
}

void NarwhalParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;

	of << dest.x << " " << dest.y << endl;
	
	of << moveFrames << endl;
}

void NarwhalParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string moveFramesStr = p->textBoxes["moveframes"]->text.getString().toAnsiString();
	

	hasMonitor = p->checkBoxes["monitor"]->checked;
	
	ss << moveFramesStr;

	int t_moveFrames;
	ss >> t_moveFrames;

	if( !ss.fail() )
	{
		moveFrames = t_moveFrames;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void NarwhalParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->textBoxes["moveframes"]->text.setString( boost::lexical_cast<string>( moveFrames ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath.clear();
	edit->patrolPath.push_back(position);
	edit->patrolPath.push_back(dest);
}

bool NarwhalParams::CanApply()
{
	return true;
}

ActorParams *NarwhalParams::Copy()
{
	NarwhalParams *copy = new NarwhalParams( *this );
	return copy;
}

BossSkeletonParams::BossSkeletonParams(  Vector2i &pos )
	:ActorParams( )
{
	type = EditSession::GetSession()->types["bossskeleton"];

	position = pos;

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

				
	SetBoundingQuad();	
}

bool BossSkeletonParams::CanApply()
{
	return true;
}

void BossSkeletonParams::WriteParamFile( ofstream &of )
{
	//no params its a boss!
}

ActorParams *BossSkeletonParams::Copy()
{
	BossSkeletonParams *copy = new BossSkeletonParams( *this );
	return copy;
}

