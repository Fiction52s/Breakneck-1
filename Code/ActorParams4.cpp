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

CoralParams::CoralParams( EditSession *edit, sf::Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY )
{
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
}

void CoralParams::SetParams()
{
	Panel *p = type->panel;
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