#include "Gate.h"
#include "GameSession.h"
#include <iostream>
#include "Physics.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

Gate::Gate( GameSession *p_owner, GateType p_type, bool p_reformBehindYou )
	:type( p_type ), locked( true ), thickLine( sf::Quads, 4 ), zoneA( NULL ), zoneB( NULL ),owner( p_owner ),
	reformBehindYou( p_reformBehindYou )
{

	//this could just be temporary
	int t = (int)p_type;

	edgeA = NULL;
	edgeB = NULL;

	activeNext = NULL;
	ts = NULL;
	
	if( type != BLACK )
	{
		gState = HARD;
	}
	else
	{
		gState = HARD;

	}
	

	gQuads = NULL;
	frame = 0;
}

void Gate::CalcAABB()
{
	assert( edgeA != NULL && edgeB != NULL );

	double left = min( edgeA->v0.x, edgeA->v1.x );
	double top = min( edgeA->v0.y, edgeA->v1.y );
	double right = max( edgeA->v0.x, edgeA->v1.x );
	double bot = max( edgeA->v0.y, edgeA->v1.y );

	left -= 64;
	top -= 64;
	right += 64;
	bot += 64;


	aabb.left = left;
	aabb.top = top;
	aabb.width = right - left;
	aabb.height = bot - top;
}

Gate::~Gate()
{
	delete gQuads;
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

	int tileHeight;// = 32;
	//Color c( Color::White );

	switch( type )
	{
	case BLACK:
		{
		c = Color( 150, 150, 150 );

		
		//ts = owner->GetTileset( "gateblack_64x64.png", 64, 64 );
		ts = owner->GetTileset( "gate_black_128x128.png", 128, 128 );
		tileHeight = 128;
		}
		break;
	case KEYGATE:
		{
		

		switch( owner->envType )
		{
		case 0:
			c = COLOR_BLUE;
			ts = owner->GetTileset( "gate_blue_128x128.png", 128, 128 );
			break;
		case 1:
			c = COLOR_GREEN;
			ts = owner->GetTileset( "gate_green_128x128.png", 128, 128 );
			break;
		case 2:
			c = COLOR_YELLOW;
			ts = owner->GetTileset( "gate_green_128x128.png", 128, 128 );
			break;
		case 3:
			c = COLOR_ORANGE;
			ts = owner->GetTileset( "gate_green_128x128.png", 128, 128 );
			break;
		case 4:
			c = COLOR_RED;
			ts = owner->GetTileset( "gate_green_128x128.png", 128, 128 );
			break;
		case 5:
			c = COLOR_MAGENTA;
			ts = owner->GetTileset( "gate_green_128x128.png", 128, 128 );
			break;
		case 6:
			c = COLOR_MAGENTA;
			ts = owner->GetTileset( "gate_green_128x128.png", 128, 128 );
			break;
		}
		
		tileHeight = 128;
		}
		break;
	case BIRDFIGHT:
		c = Color::Green;
		ts = owner->GetTileset( "gate_blue_128x128.png", 128, 128 );
		tileHeight = 128;
		break;
	case CRAWLER_UNLOCK:
		c = Color::Blue;
		ts = owner->GetTileset( "gate_blue_128x128.png", 128, 128 );
		tileHeight = 128;
		break;
	case NEXUS1_UNLOCK:
		c = Color::Cyan;
		ts = owner->GetTileset( "gate_blue_128x128.png", 128, 128 );
		tileHeight = 128;
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

	
	double gateLength = length(edgeA->v1 - edgeA->v0 );
	double numT = gateLength / tileHeight; //rounded down
	int numTiles = numT;
	double remainder = numT - numTiles;
	if( remainder > 0 )
	{
		numTiles += 1;
	}
	int numVertices = numTiles * 4;

	//if( type == Gate::GREY || type == Gate::BLACK || keyGate )
	//{
		if( gQuads == NULL )
		{
			gQuads = new VertexArray( sf::Quads, numVertices );
		}
	//}
	//cout << "giving gquads value!" << endl;
}

void Gate::Update()
{
	

	//gates can be timeslowed? don't worry about it just yet. 
	if( type != BLACK )
	{
		switch( gState )
		{
		case HARDEN:
			{
				if( frame == 0 )
				{
					gState = LOCKFOREVER;
					frame = 0;
				}
			}
			break;
		case HARD:
			{
				if( frame == 1 )
				{
					frame = 0;
				}
			}
			break;
		case SOFTEN:
			{
				if( frame == 0 )
				{
					gState = SOFT;
					frame = 0;
				}
			}
			break;
		case SOFT:
			{
				if( frame == 1 )
				{
					frame = 0;
				}
			}
			break;
		case DISSOLVE:
			{
				//whatever length
				if( frame == 3 * 4 )
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
				//cout << "reforming " << endl;
				//whatever length
				if( frame == 10 )
				{
					gState = LOCKFOREVER;
				}
			}
			break;
		case LOCKFOREVER:
			{
				//cout << "locked foreverrrr" << endl;
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
	}
	else
	{
		frame = 0;
	}
	double radius = 300;
	
	//if( keyGate )
	//{

	//	if( owner->player->numKeys < requiredKeys && IsEdgeTouchingCircle( edgeA->v0, edgeA->v1, owner->player->position, radius ) )
	//	{
	//		//cout << "HARDENING: " << type << endl;
	//		if( gState == SOFTEN )
	//		{
	//			gState = HARDEN;
	//			frame = 0;
	//			//frame should be the inverse so that it can get harder while from its partially softened state.
	//		}
	//		else if( gState == SOFT )
	//		{
	//			gState = HARDEN;
	//			frame = 0;
	//		}
	//	}
	//	else
	//	{
	//		//cout << "SOFTENING: " << type << endl;
	//		if( gState == HARDEN )
	//		{
	//			gState = SOFTEN;
	//			frame = 0;
	//			//inverse frame;
	//		}
	//		else if( gState == HARD )
	//		{
	//			gState = SOFTEN;
	//			frame = 0;
	//		}
	//	}
	//}

	if( type != BLACK )
	{
		Zone *currZone = owner->currentZone;
		bool enoughKeys = (owner->keyMarker->keysRequired == 0);

		if( gState == HARD && enoughKeys && 
			( currZone == NULL || ( currZone == zoneA || currZone == zoneB ) ) )
		{
			gState = SOFT; //SOFTEN
			frame = 0;

			Wire *rw = owner->player->rightWire;
			Wire *lw = owner->player->leftWire;
			if( rw != NULL && rw->anchor.e == edgeA || rw->anchor.e == edgeB )
			{
				rw->Reset();
			}
			if( lw != NULL && lw->anchor.e == edgeA || lw->anchor.e == edgeB )
			{
				lw->Reset();
			}
		}
	}

	int tileWidth = ts->tileWidth;
	int tileHeight = ts->tileHeight;
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

	
	int realFrame = 0;

	if( type != BLACK )
	{
		switch( gState )
		{
		case HARDEN:
			{
				realFrame = frame;
			}
			break;
		case HARD:
			{
				realFrame = 1;
			}
			break;
		case SOFTEN:
			{
				realFrame = 10 + frame;
			}
			break;
		case SOFT:
			{
				realFrame = 2;// + frame / 3;
			}
			break;
		case DISSOLVE:
			{
				realFrame = 3 + frame / 4;
			}
			break;
		case REFORM:
			{
				realFrame = 0;
			}
			break;
		case LOCKFOREVER:
			{
				realFrame  = 0;
			}
			break;
		case OPEN:
			{
				realFrame = 0;
			}
			break;
		}
	}
	else if( type == BLACK )
	{
	}

		
	//IntRect subRect = ts->GetSubRect( realFrame );
		

	if( realFrame < 0 )
	{
		cout << "type: " << type << endl;
		cout << "gState: " << gState << endl;
	}

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

	double fullHeight =  (double)(tileHeight * (numTiles-1));
	
	V2d lv0 = leftv0 + along * fullHeight;
	V2d lv1 = leftv1;
	V2d rv1 = rightv1;
	V2d rv0 = rightv0 + along * fullHeight;
	
	double thisHeight = length( lv1 - lv0 );

	//remainder
	
	double h = subRect.height - thisHeight;
	
	gq[(numTiles-1) * 4 + 0].texCoords = Vector2f( subRect.left, h );
	gq[(numTiles-1) * 4 + 1].texCoords = Vector2f( subRect.left + subRect.width, h  );
	gq[(numTiles-1) * 4 + 2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
	gq[(numTiles-1) * 4 + 3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );

	gq[(numTiles-1) * 4 + 3].position = Vector2f( lv0.x, lv0.y );
	gq[(numTiles-1) * 4 + 0].position = Vector2f( lv1.x, lv1.y );
	gq[(numTiles-1) * 4 + 1].position = Vector2f( rv1.x, rv1.y );
	gq[(numTiles-1) * 4 + 2].position = Vector2f( rv0.x, rv0.y );
	
	switch( type )
	{
	case BLACK:
		break;
	case KEYGATE:
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
	
	return IsBoxTouchingBox( aabb, r );//edgeA->IsTouchingBox( r );//IsBoxTouchingBox( aabb, r );//r.intersects( aabb );//edgeA->IsTouchingBox( r );
}