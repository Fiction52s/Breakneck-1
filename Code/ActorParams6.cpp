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

SpecterParams::SpecterParams( EditSession *edit, sf::Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY )
{
	position = pos;	
	type = edit->types["specter"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
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