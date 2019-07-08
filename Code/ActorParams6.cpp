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

SpecterParams::SpecterParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at )
{
	PlaceAerial(pos);
}

SpecterParams::SpecterParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
}

void SpecterParams::WriteParamFile(std::ofstream &of )
{
	WriteMonitor(of);
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

ActorParams *SpecterParams::Copy()
{
	SpecterParams *copy = new SpecterParams( *this );
	return copy;
}


CopycatParams::CopycatParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams( at)
{
	PlaceAerial(pos);
}

CopycatParams::CopycatParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
}

void CopycatParams::WriteParamFile( std::ofstream &of )
{
	WriteMonitor(of);
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

ActorParams *CopycatParams::Copy()
{
	CopycatParams *copy = new CopycatParams( *this );
	return copy;
}

GorillaParams::GorillaParams(ActorType *at,
		sf::Vector2i &pos, int p_wallWidth,
		int p_followFrames )
		:ActorParams( at)
{
	wallWidth = p_wallWidth;
	followFrames = p_followFrames;
	PlaceAerial(pos);
}

GorillaParams::GorillaParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
	is >> wallWidth;
	is >> followFrames;
}

GorillaParams::GorillaParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	wallWidth = 400;
	followFrames = 60;
	PlaceAerial(pos);
}

void GorillaParams::WriteParamFile( std::ofstream &of )
{
	WriteMonitor(of);
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
ActorParams *GorillaParams::Copy()
{
	GorillaParams *copy = new GorillaParams( *this );
	return copy;
}

NarwhalParams::NarwhalParams(ActorType *at,
		sf::Vector2i &pos, sf::Vector2i & p_dest,
		int p_moveFrames )
		:ActorParams( at)
{
	dest = p_dest;
	moveFrames = p_moveFrames;
	PlaceAerial(pos);
}

NarwhalParams::NarwhalParams(ActorType *at,ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
	is >> dest.x;
	is >> dest.y;
	is >> moveFrames;
}

NarwhalParams::NarwhalParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at )
{
	dest = pos + Vector2i( 500, 0 );
	moveFrames = 60;
	PlaceAerial(pos);
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
	WriteMonitor(of);
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

ActorParams *NarwhalParams::Copy()
{
	NarwhalParams *copy = new NarwhalParams( *this );
	return copy;
}

BossSkeletonParams::BossSkeletonParams(ActorType *at, Vector2i &pos )
	:ActorParams(at )
{
	PlaceAerial(pos);
}

BossSkeletonParams::BossSkeletonParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
}

ActorParams *BossSkeletonParams::Copy()
{
	BossSkeletonParams *copy = new BossSkeletonParams( *this );
	return copy;
}

