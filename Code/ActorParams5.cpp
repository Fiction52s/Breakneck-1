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

SharkParams::SharkParams(ActorType *at, sf::Vector2i &pos,
	int p_circleFrames )
	:ActorParams( at)
{
	circleFrames = p_circleFrames;

	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

SharkParams::SharkParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
	is >> circleFrames;
}

SharkParams::SharkParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at )
{
	circleFrames = 60;

	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

void SharkParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;

	of << circleFrames << endl;
}

void SharkParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string circleStr = p->textBoxes["circleframes"]->text.getString().toAnsiString();

	ss << circleStr;

	int t_circleFrames;
	ss >> t_circleFrames;

	if( !ss.fail() )
	{
		circleFrames = circleFrames;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void SharkParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->textBoxes["circleframes"]->text.setString( boost::lexical_cast<string>( circleFrames ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool SharkParams::CanApply()
{
	return true;
}

ActorParams *SharkParams::Copy()
{
	SharkParams *copy = new SharkParams( *this );
	return copy;
}




SwarmParams::SwarmParams(ActorType *at, sf::Vector2i &pos,
	int p_liveFrames )
	:ActorParams(at )
{
	liveFrames = p_liveFrames;

	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

SwarmParams::SwarmParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
	is >> liveFrames;
}

SwarmParams::SwarmParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at )
{
	liveFrames = 420;

	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

void SwarmParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;

	of << liveFrames << endl;
}

void SwarmParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string circleStr = p->textBoxes["liveframes"]->text.getString().toAnsiString();

	ss << circleStr;

	int t_liveFrames;
	ss >> t_liveFrames;

	if( !ss.fail() )
	{
		liveFrames = t_liveFrames;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void SwarmParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->textBoxes["liveframes"]->text.setString( boost::lexical_cast<string>( liveFrames ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool SwarmParams::CanApply()
{
	return true;
}

ActorParams *SwarmParams::Copy()
{
	SwarmParams *copy = new SwarmParams( *this );
	return copy;
}

GhostParams::GhostParams(ActorType *at, sf::Vector2i &pos,
	int p_speed )
	:ActorParams(at )
{
	speed = p_speed;

	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

GhostParams::GhostParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
	is >> speed;
}

GhostParams::GhostParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams( at)
{
	speed = 1;

	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

void GhostParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;

	of << speed << endl;
}

void GhostParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();

	ss << speedStr;

	int t_speed;
	ss >> t_speed;

	if( !ss.fail() )
	{
		speed = t_speed;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void GhostParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool GhostParams::CanApply()
{
	return true;
}

ActorParams *GhostParams::Copy()
{
	GhostParams *copy = new GhostParams( *this );
	return copy;
}




OvergrowthParams::OvergrowthParams(ActorType *at,
	TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at )
{

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

OvergrowthParams::OvergrowthParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);
	LoadMonitor(is);
}

bool OvergrowthParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void OvergrowthParams::WriteParamFile( ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
}

void OvergrowthParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void OvergrowthParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *OvergrowthParams::Copy()
{
	OvergrowthParams *copy = new OvergrowthParams( *this );
	return copy;
}

BossGatorParams::BossGatorParams(ActorType *at, Vector2i &pos )
	:ActorParams(at )
{
	radius = 600;
	orbRadius = 160;

	position = pos;

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	
	for( int i = 0; i < 5; ++i )
	{
		CircleShape &cs = circles[i];
		cs.setFillColor( Color::Red );
		cs.setRadius( orbRadius );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	}


	Transform t;
	Vector2f offset( 0, -radius );
	for( int i = 0; i < 5; ++i )
	{
		CircleShape &cs = circles[i];
		Vector2f truePos = t.transformPoint( offset ) + Vector2f( position.x, position.y );
		cs.setPosition( truePos );
		t.rotate( 360.f / 5.f );
	}
	
		
				
	SetBoundingQuad();	
}

BossGatorParams::BossGatorParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	radius = 600;
	orbRadius = 160;

	for (int i = 0; i < 5; ++i)
	{
		CircleShape &cs = circles[i];
		cs.setFillColor(Color::Red);
		cs.setRadius(orbRadius);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
	}


	Transform t;
	Vector2f offset(0, -radius);
	for (int i = 0; i < 5; ++i)
	{
		CircleShape &cs = circles[i];
		Vector2f truePos = t.transformPoint(offset) + Vector2f(position.x, position.y);
		cs.setPosition(truePos);
		t.rotate(360.f / 5.f);
	}
}

bool BossGatorParams::CanApply()
{
	Transform t;
	Vector2f offset( 0, -radius );
	for( int i = 0; i < 5; ++i )
	{
		CircleShape &cs = circles[i];
		Vector2f truePos = t.transformPoint( offset ) + Vector2f( position.x, position.y );
		cs.setPosition( truePos );
		t.rotate( 360.f / 5.f );
	}
	return true;
}

void BossGatorParams::WriteParamFile( ofstream &of )
{
	//no params its a boss!
}

ActorParams *BossGatorParams::Copy()
{
	BossGatorParams *copy = new BossGatorParams( *this );
	return copy;
}

void BossGatorParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );

	for( int i = 0; i < 5; ++i )
	{
		target->draw( circles[i] );
	}
}

