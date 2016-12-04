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


CoralParams::CoralParams( EditSession *edit, sf::Vector2i &pos, 
	int p_moveFrames )
	:ActorParams( PosType::AIR_ONLY )
{
	moveFrames = p_moveFrames;
	position = pos;	
	type = edit->types["coral"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

CoralParams::CoralParams( EditSession *edit, sf::Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY )
{
	moveFrames = 60;

	position = pos;	
	type = edit->types["coral"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}



void CoralParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;

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

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->checkBoxes["monitor"]->checked = hasMonitor;

	p->textBoxes["moveframes"]->text.setString( 
		boost::lexical_cast<string>( moveFrames ) );
}

bool CoralParams::CanApply()
{
	return true;
}

ActorParams *CoralParams::Copy()
{
	CoralParams *copy = new CoralParams( *this );
	return copy;
}




TurtleParams::TurtleParams( EditSession *edit, sf::Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY )
{
	position = pos;	
	type = edit->types["turtle"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

void TurtleParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
}

void TurtleParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void TurtleParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool TurtleParams::CanApply()
{
	return true;
}

ActorParams *TurtleParams::Copy()
{
	TurtleParams *copy = new TurtleParams( *this );
	return copy;
}

CheetahParams::CheetahParams( EditSession *edit,
	TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams( PosType::GROUND_ONLY )
{
	type = edit->types["cheetah"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

bool CheetahParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void CheetahParams::WriteParamFile( ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
}

void CheetahParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void CheetahParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *CheetahParams::Copy()
{
	CheetahParams *copy = new CheetahParams( *this );
	return copy;
}



SpiderParams::SpiderParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, int p_speed )
	:ActorParams( PosType::GROUND_ONLY )
{
	speed = p_speed;

	type = edit->types["spider"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

SpiderParams::SpiderParams( EditSession *edit,
	TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams( PosType::GROUND_ONLY )
{
	
	speed = 10;
	type = edit->types["spider"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

bool SpiderParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void SpiderParams::WriteParamFile( ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
	
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

	p->textBoxes["name"]->text.setString( "test" );
	
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	
	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *SpiderParams::Copy()
{
	SpiderParams *copy = new SpiderParams( *this );
	return copy;
}

BossTigerParams::BossTigerParams( EditSession *edit, Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY ), debugLines( sf::Lines, 30 * 2 )
{
	type = edit->types["bosstiger"];

	position = pos;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	radius1 = 800;
	radius2 = 1800;
				
	SetBoundingQuad();	
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
	Vector2f op( position.x, position.y );

	Transform t;
	Vector2f offset( 0, -radius1 );
	for( int i = 0; i < 6; ++i )
	{
		Vector2f newP = t.transformPoint( offset ) + op;
		nodePos[i+1] = newP;
		t.rotate( -360.f / 6.f );
	}

	Transform t2;
	Vector2f offset2( 0, -radius2 );
	t2.rotate( 360.f / 12.f );
	for( int i = 0; i < 6; ++i )
	{
		Vector2f newP = t2.transformPoint( offset2 ) + op;
		nodePos[i+7] = newP;
		t2.rotate( -360.f / 6.f );
	}

	for( int i = 0; i < 6; ++i )
	{
		Vector2f nextPos = nodePos[i+1];
		Vector2f thisPos = op;
		
		debugLines[i*2 + 0].position = Vector2f( thisPos.x, thisPos.y );
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

bool BossTigerParams::CanApply()
{
	CreateFormation();
	return true;
}

void BossTigerParams::WriteParamFile( ofstream &of )
{
	//no params its a boss!
}

ActorParams *BossTigerParams::Copy()
{
	BossTigerParams *copy = new BossTigerParams( *this );
	return copy;
}