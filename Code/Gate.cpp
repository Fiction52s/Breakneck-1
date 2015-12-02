#include "Gate.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Gate::Gate( GateType p_type )
	:Edge(), type( p_type ), locked( true ), thickLine( sf::Quads, 4 )
{
	switch( type )
	{
	case RED:
		c = Color::Red;
		break;
	case GREEN:
		c = Color::Green;
		break;
	case BLUE:
		c =  Color( 77, 150, 249);
		break;
	}
	thickLine[0].color = c;
	thickLine[1].color = c;
	thickLine[2].color = c;
	thickLine[3].color = c;
}

void Gate::Draw( sf::RenderTarget *target )
{
	CircleShape cs( 5 );
	cs.setFillColor( c );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

	cs.setPosition( v0.x, v0.y );
	target->draw( cs );

	cs.setPosition( v1.x, v1.y );
	target->draw( cs );

	if( locked )
	{
		target->draw( thickLine );
	}
}

void Gate::UpdateLine()
{
	double width = 5;
	V2d dv0( v0.x, v0.y );
	V2d dv1( v1.x, v1.y );
	V2d along = normalize( dv1 - dv0 );
	V2d other( along.y, -along.x );
	
	V2d leftv0 = dv0 - other * width;
	V2d rightv0 = dv0 + other * width;

	V2d leftv1 = dv1 - other * width;
	V2d rightv1 = dv1 + other * width;

	
	thickLine[0].position = Vector2f( leftv0.x, leftv0.y );
	thickLine[1].position = Vector2f( leftv1.x, leftv1.y );
	thickLine[2].position = Vector2f( rightv1.x, rightv1.y );
	thickLine[3].position = Vector2f( rightv0.x, rightv0.y );
}
