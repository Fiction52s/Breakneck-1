#include "Gate.h"
#include "GameSession.h"
#include <iostream>
#include "Physics.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Gate::Gate( GameSession *p_owner, GateType p_type, bool p_reformBehindYou )
	:type( p_type ), locked( true ), thickLine( sf::Quads, 4 ), zoneA( NULL ), zoneB( NULL ),owner( p_owner ),
	reformBehindYou( p_reformBehindYou )
{
	edgeA = NULL;
	edgeB = NULL;

	activeNext = NULL;
	ts = NULL;
	gState = SOFT;

	gQuads = NULL;
	frame = 0;
}

void Gate::Draw( sf::RenderTarget *target )
{
	//draw quads
	CircleShape cs( 5 );
	cs.setFillColor( c );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

	cs.setPosition( edgeA->v0.x, edgeA->v0.y );
	target->draw( cs );

	cs.setPosition( edgeA->v1.x, edgeA->v1.y );
	target->draw( cs );

	if( locked )
	{
	//	target->draw( thickLine );
	}

	if( gQuads != NULL )
	{
		if( gState != OPEN )
		{
			target->draw( *gQuads, ts->texture );
		}
		//if( locked )
		//{
			
		//}
	}
	else
	{
		if( gState != OPEN )
		{
			target->draw( thickLine );
		}
	}

}

void Gate::UpdateLine()
{
	Color c( Color::White );
	switch( type )
	{
	case GREY:
		c = Color( 100, 100, 100 );
		ts = owner->GetTileset( "greygate.png", 32, 32 );
		break;
	case BLACK:
		c = Color( 0, 0, 0 );
		//ts = owner->GetTileset( "blackgate_64x64.png", 64, 64 );
		break;
	case BLUE:
		c =  Color( 77, 150, 249);
		break;
	case GREEN:
		c = Color::Green;
		break;
	case RED:
		c = Color::Red;
		break;
	}
	thickLine[0].color = c;
	thickLine[1].color = c;
	thickLine[2].color = c;
	thickLine[3].color = c;

	double width = 5;
	V2d dv0( edgeA->v0.x, edgeA->v0.y );
	V2d dv1( edgeA->v1.x, edgeA->v1.y );
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

	int tileHeight = 32;
	double gateLength = length(edgeA->v1 - edgeA->v0 );
	double numT = gateLength / tileHeight; //rounded down
	int numTiles = numT;
	double remainder = numT - numTiles;
	if( remainder > 0 )
	{
		numTiles += 1;
	}
	int numVertices = numTiles * 4;

	if( type == Gate::GREY )// || type == Gate::BLACK )
	{
		if( gQuads == NULL )
		{
			gQuads = new VertexArray( sf::Quads, numVertices );
		}
	}
	//cout << "giving gquads value!" << endl;
}

void Gate::Update()
{


	//gates can be timeslowed? don't worry about it just yet. 
	switch( gState )
	{
	case HARDEN:
		{
			if( frame == 9 )
			{
				gState = HARD;
				frame = 0;
			}
		}
		break;
	case HARD:
		{
			if( frame == 3 * 3 )
			{
				frame = 0;
			}
		}
		break;
	case SOFTEN:
		{
			if( frame == 10 )
			{
				gState = SOFT;
				frame = 0;
			}
		}
		break;
	case SOFT:
		{
			if( frame == 12 * 3 )
			{
				frame = 0;
			}
		}
		break;
	case DISSOLVE:
		{
			//whatever length
			if( frame == 9 * 4 )
			{
				if( reformBehindYou )
				{
					gState = REFORM;
					frame = 0;
				}
				else
				{
					gState = OPEN;
					frame = 0;
				}
			}
		}
		break;
	case REFORM:
		{
			//whatever length
			if( frame == 10 )
			{
				gState = LOCKFOREVER;
			}
		}
		break;
	case LOCKFOREVER:
		{
			//whatever the last frame of lockforever is
			frame = 10;
		}
		break;
	case OPEN:
		{
			frame = 0;
		}
		break;
	}

	double radius = 200;
	//double dist = length( owner->player.position
	if( IsEdgeTouchingCircle( edgeA->v0, edgeA->v1, owner->player.position, radius ) )
	{
		if( gState == SOFTEN )
		{
			gState = HARDEN;
			//frame should be the inverse so that it can get harder while from its partially softened state.
		}
		else if( gState == SOFT )
		{
			gState = HARDEN;
			frame = 0;
		}
	}
	else
	{
		if( gState == HARDEN )
		{
			gState = SOFTEN;
			//inverse frame;
		}
		else if( gState == HARD )
		{
			gState = SOFTEN;
			frame = 0;
		}
	}

	int tileWidth = 32;
	int tileHeight = 32;
	double gateLength = length(edgeA->v1 - edgeA->v0 );
	double numT = gateLength / tileHeight; //rounded down
	int numTiles = numT;
	double remainder = numT - numTiles;
	if( remainder > 0 )
		numTiles += 1;

	int numVertices = numTiles * 4;

	double width = tileWidth / 2;
	V2d dv0( edgeA->v0.x, edgeA->v0.y );
	V2d dv1( edgeA->v1.x, edgeA->v1.y );
	V2d along = normalize( dv1 - dv0 );
	V2d other( along.y, -along.x );
	
	V2d leftv0 = dv0 - other * width;
	V2d rightv0 = dv0 + other * width;

	V2d leftv1 = dv1 - other * width;
	V2d rightv1 = dv1 + other * width;
	
	VertexArray & gq = *gQuads;

	Vector2f leftv0f( leftv0.x, leftv0.y );
	Vector2f leftv1f( leftv1.x, leftv1.y );
	Vector2f rightv1f( rightv1.x, rightv1.y );
	Vector2f rightv0f( rightv0.x, rightv0.y );
	int f = frame / 3;
	//cout << "gq: " << gq.getVertexCount() << endl;
	if( type == GREY )
	{
		int realFrame = -1;
		switch( gState )
		{
		case HARDEN:
			{
				realFrame = frame;
			}
			break;
		case HARD:
			{
				realFrame = 10;
			}
			break;
		case SOFTEN:
			{
				realFrame = 10 + frame;
			}
			break;
		case SOFT:
			{
				realFrame = 21 + frame / 3;
			}
			break;
		case DISSOLVE:
			{
				realFrame = 34 + frame / 4;
			}
			break;
		case REFORM:
			{
				realFrame = 0;
			}
			break;
		case LOCKFOREVER:
			{
				realFrame  = 1;
			}
			break;
		case OPEN:
			{
				realFrame = 0;
			}
			break;
		}
		//IntRect subRect = ts->GetSubRect( realFrame );
		assert( realFrame >= 0 );

		IntRect subRect = ts->GetSubRect( realFrame );
		for( int i = 0; i < numTiles - 1; ++i )
		{
			
			gq[i*4+0].texCoords = Vector2f( subRect.left, subRect.top );//Vector2f( 0, frame * tileHeight + 0 );
			gq[i*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );//Vector2f( tileWidth, frame * tileHeight + 0 );
			gq[i*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );//Vector2f( tileWidth, frame * tileHeight + tileHeight );
			gq[i*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );//Vector2f( 0, frame * tileHeight + tileHeight );

			V2d lv0 = leftv0 + along * (double)(tileHeight * i);
			V2d lv1 = leftv0 + along * (double)(tileHeight * (i+1));
			V2d rv1 = rightv0 + along * (double)(tileHeight * (i+1));
			V2d rv0 = rightv0 + along * (double)(tileHeight * i);
			gq[i*4+3].position = Vector2f( lv0.x, lv0.y );
			gq[i*4+0].position = Vector2f( lv1.x, lv1.y );
			gq[i*4+1].position = Vector2f( rv1.x, rv1.y );
			gq[i*4+2].position = Vector2f( rv0.x, rv0.y );
		}

		V2d lv0 = leftv0 + along * (double)(tileHeight * (numTiles-1));
		V2d lv1 = leftv1;
		V2d rv1 = rightv1;
		V2d rv0 = rightv0 + along * (double)(tileHeight * (numTiles-1));
		//remainder

	
		gq[(numTiles-1) * 4 + 0].texCoords = Vector2f( subRect.left, subRect.top );
		gq[(numTiles-1) * 4 + 1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		gq[(numTiles-1) * 4 + 2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		gq[(numTiles-1) * 4 + 3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );

		gq[(numTiles-1) * 4 + 3].position = Vector2f( lv0.x, lv0.y );
		gq[(numTiles-1) * 4 + 0].position = Vector2f( lv1.x, lv1.y );
		gq[(numTiles-1) * 4 + 1].position = Vector2f( rv1.x, rv1.y );
		gq[(numTiles-1) * 4 + 2].position = Vector2f( rv0.x, rv0.y );
	}
	switch( type )
	{
	case GREY:
		{
			
		}
		break;
	case BLACK:
		break;
	case BLUE:
		break;
	case GREEN:
		break;
	case RED:
		break;
	}


	
	++frame;
}

void Gate::SetLocked( bool on )
{
	if( on )
	{
		locked = true;

		edgeA->edgeType = Edge::CLOSED_GATE;
		edgeB->edgeType = Edge::CLOSED_GATE;

		edgeA->edge0 = temp0prev;
		temp0prev->edge1 = edgeA;

		edgeA->edge1 = temp1next;
		temp1next->edge0 = edgeA;

		//edgeB->edge0 = temp0prev;
		//temp0prev->edge1 = edgeB;

	//	edgeB->edge1 = temp1next;
	//	temp1next->edge0 = edgeB;

		edgeB->edge0 = temp1prev;
		temp1prev->edge1 = edgeB;

		edgeB->edge1 = temp0next;
		temp0next->edge0 = edgeB;
	}
	else
	{
		locked = false;

		edgeA->edgeType = Edge::OPEN_GATE;
		edgeB->edgeType = Edge::OPEN_GATE;

		temp0next->edge0 = temp0prev;
		temp0prev->edge1 = temp0next;

		temp1next->edge0 = temp1prev;
		temp1prev->edge1 = temp1next;
		//locked = false;
		//edge0->edge1 = edge1;
		//edge1->edge0 = edge0;
	}
}

void Gate::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
	//edgeA->HandleQuery( qtc );
}

bool Gate::IsTouchingBox( const sf::Rect<double> &r )
{
	return edgeA->IsTouchingBox( r );
}