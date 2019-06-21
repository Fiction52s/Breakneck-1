#include "Gate.h"
#include "GameSession.h"
#include <iostream>
#include "Physics.h"
#include "KeyMarker.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;


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
	visible = true;
	blackGate = NULL;
	reformBehindYou = true;
	//breakFrame = 0;
	flowFrame = 0;
	//this could just be temporary
	int t = (int)p_type;

	edgeA = NULL;
	edgeB = NULL;

	activeNext = NULL;
	ts = NULL;
	ts_black = owner->GetTileset("Zone/gates_black_32x32.png", 32, 32);
	ts_lightning = owner->GetTileset("Zone/gates_lightning_1_64x64.png", 64, 64);
	
	
	if( type != BLACK )
	{
		gState = HARD;
	}
	else
	{
		gState = LOCKFOREVER;

	}
	
	if (!gateShader.loadFromFile("Resources/Shader/gate_shader.frag", sf::Shader::Fragment))
	{
		cout << "failed to load gate shader" << endl;
		assert(0);
	}
	gateShader.setUniform("fadeQuant", 0.f);

	if (!centerShader.loadFromFile("Resources/Shader/gatecenter_shader.frag", sf::Shader::Fragment))
	{
		cout << "failed to load gate center shader" << endl;
		assert(0);
	}
	centerShader.setUniform("breakQuant", 0.f);

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
	if( gQuads != NULL )
		delete gQuads;

	if (blackGate != NULL)
		delete blackGate;

	delete edgeA;
	delete edgeB;
}

void Gate::Reset()
{
	centerShader.setUniform("breakQuant", 0.f);
	gateShader.setUniform("fadeQuant", 0.f);
	flowFrame = 0;
	frame = 0;

	SetLocked(true);
	if (type != Gate::BLACK)
		gState = Gate::HARD;
}

void Gate::Draw( sf::RenderTarget *target )
{
	if (!visible)
		return;

	if( gQuads != NULL )
	{
		if( gState != OPEN )
		{
			target->draw( *gQuads, ts->texture );
		}
	}
	else
	{
		if( gState != OPEN )
		{
			if (gState == REFORM || gState == LOCKFOREVER)
			{
				if (gState == REFORM)
				{
					target->draw(centerLine, 4, sf::Quads, &centerShader);
				}
				target->draw(blackGate, numBlackQuads * 4, sf::Quads, ts_black->texture );
			}
			else
			{
				if (gState == SOFT)
				{
					target->draw(blackGate, numBlackQuads * 4, sf::Quads, ts_lightning->texture);
				}
				target->draw(centerLine, 4, sf::Quads, &centerShader);

				if (gState != SOFT)
				{
					target->draw(testLine, 4, sf::Quads, &gateShader);
				}
				else
				{
					//target->draw(blackGate, numBlackQuads * 4, sf::Quads, ts_lightning->texture);
				}
			}
		}

		target->draw(nodes, 8, sf::Quads, ts_node->texture);
	}

}

void Gate::UpdateLine()
{
	dissolveLength = 20 * max( 1.0, length(edgeA->v1 - edgeA->v0) / 400.0 );
	float tileHeight = 64;

	switch( type )
	{
	case BLACK:
		{
		c = Color( 150, 150, 150 );
		ts = owner->GetTileset( "Zone/gate_black_128x128.png", 128, 128 );
		tileHeight = 128;
		}
		break;
	case CRAWLER_UNLOCK:
	case SECRET:
	case KEYGATE:
		{
		switch( owner->mh->envWorldType ) 
		{
		case 0:
			c = COLOR_BLUE;
			ts = owner->GetTileset( "Zone/gate_blue_128x128.png", 128, 128 );
			break;
		case 1:
			c = COLOR_GREEN;
			ts = owner->GetTileset( "Zone/gate_green_128x128.png", 128, 128 );
			break;
		case 2:
			c = COLOR_YELLOW;
			ts = owner->GetTileset( "Zone/gate_green_128x128.png", 128, 128 );
			break;
		case 3:
			c = COLOR_ORANGE;
			ts = owner->GetTileset( "Zone/gate_green_128x128.png", 128, 128 );
			break;
		case 4:
			c = COLOR_RED;
			ts = owner->GetTileset( "Zone/gate_green_128x128.png", 128, 128 );
			break;
		case 5:
			c = COLOR_MAGENTA;
			ts = owner->GetTileset( "Zone/gate_green_128x128.png", 128, 128 );
			break;
		case 6:
			c = COLOR_MAGENTA;
			ts = owner->GetTileset( "Zone/gate_green_128x128.png", 128, 128 );
			break;
		}
		
		//ts = owner->GetTileset("gate_32x64.png", 32, 64);

		Tileset *tts = owner->GetTileset("Zone/gates_32x64.png", 32, 64);
		gateShader.setUniform("u_texture", *tts->texture );
		gateShader.setUniform("tile", 1.f);
		//gateShader.setUniform("fadeQuant", 1.f);

		centerShader.setUniform("u_texture", *tts->texture);
		frame = 0;
		

		tileHeight = 128;
		}
		break;
	/*case BIRDFIGHT:
		c = Color::Green;
		ts = owner->GetTileset( "gate_blue_128x128.png", 128, 128 );
		tileHeight = 128;
		break;*/
	//case CRAWLER_UNLOCK:
		/*c = Color::Blue;
		ts = owner->GetTileset( "gate_blue_128x128.png", 128, 128 );
		tileHeight = 128;*/
	//	break;
	/*case NEXUS1_UNLOCK:
		c = Color::Cyan;
		ts = owner->GetTileset( "gate_blue_128x128.png", 128, 128 );
		tileHeight = 128;
		break;*/
	}
	/*thickLine[0].color = c;
	thickLine[1].color = c;
	thickLine[2].color = c;
	thickLine[3].color = c;*/

	tileHeight = 64;

	double width = 16; //5
	V2d dv0( edgeA->v0.x, edgeA->v0.y );
	V2d dv1( edgeA->v1.x, edgeA->v1.y );
	V2d along = normalize( dv1 - dv0 );
	V2d other( along.y, -along.x );
	
	V2d leftv0 = dv0 - other * width;
	V2d rightv0 = dv0 + other * width;

	V2d leftv1 = dv1 - other * width;
	V2d rightv1 = dv1 + other * width;

	V2d nodeALeftv0 = leftv0 - along * 32.0;
	V2d nodeARightv0 = rightv0 - along * 32.0;
	V2d nodeALeftv1 = leftv0 + along * 32.0;
	V2d nodeARightv1 = rightv0 + along * 32.0;
	
	V2d nodeBLeftv0 = leftv1 + along * 32.0;
	V2d nodeBRightv0 = rightv1 + along * 32.0;
	V2d nodeBLeftv1 = leftv1 - along * 32.0;
	V2d nodeBRightv1 = rightv1 - along * 32.0;

	thickLine[0].position = Vector2f( leftv0.x, leftv0.y );
	thickLine[1].position = Vector2f( leftv1.x, leftv1.y );
	thickLine[2].position = Vector2f( rightv1.x, rightv1.y );
	thickLine[3].position = Vector2f( rightv0.x, rightv0.y );

	testLine[0].position = Vector2f(rightv0.x, rightv0.y); 
	testLine[1].position = Vector2f(leftv0.x, leftv0.y); 
	testLine[2].position = Vector2f(leftv1.x, leftv1.y); 
	testLine[3].position = Vector2f(rightv1.x, rightv1.y);

	centerLine[0].position = Vector2f(rightv0.x, rightv0.y);
	centerLine[1].position = Vector2f(leftv0.x, leftv0.y);
	centerLine[2].position = Vector2f(leftv1.x, leftv1.y);
	centerLine[3].position = Vector2f(rightv1.x, rightv1.y);

	nodes[0].position = Vector2f(nodeALeftv0);
	nodes[1].position = Vector2f(nodeARightv0);
	nodes[2].position = Vector2f(nodeARightv1);
	nodes[3].position = Vector2f(nodeALeftv1);

	nodes[4].position = Vector2f(nodeBRightv0);
	nodes[5].position = Vector2f(nodeBLeftv0);
	nodes[6].position = Vector2f(nodeBLeftv1);
	nodes[7].position = Vector2f(nodeBRightv1);

	ts_node = owner->GetTileset("Zone/gatenode_32x64.png", 32, 64);

	SetRectSubRect(nodes, ts_node->GetSubRect(0) );
	SetRectSubRect((nodes+4), ts_node->GetSubRect(0));

	FloatRect ir;
	ir.left = 0;
	ir.top = 0;
	ir.width = 1.0;
	double edgeLen = length(leftv0 - leftv1);
	ir.height = edgeLen / tileHeight;
	centerShader.setUniform("numReps", ir.height);
	gateShader.setUniform("numReps", ir.height);

	SetRectSubRect(testLine, ir);
	SetRectSubRect(centerLine, ir);
	
	double gateLength = length(edgeA->v1 - edgeA->v0 );
	double numT = gateLength / tileHeight; //rounded down
	int numTiles = numT;
	double remainder = numT - numTiles;
	if( remainder > 0 )
	{
		numTiles += 1;
	}
	int numVertices = numTiles * 4;

	blackGate = new Vertex[numVertices];
	numBlackQuads = numTiles;
	

	double angle = atan2(-along.x, along.y);
	//angle *= 180 / PI;
	//cout << "along: " << along.x << ", " << along.y << endl;
	//cout << "angle: " << angle << endl;


	for (int i = 0; i < numTiles; ++i)
	{

		if (i == numTiles - 1 && remainder > 0)
		{
			V2d start = edgeA->GetPoint(64 * i);
			V2d end = edgeA->v1;
			double h = length(end - start);
			SetRectRotation(blackGate + i * 4, angle, 64, h, Vector2f(edgeA->GetPoint(64 * i + (32 - (64 - h) / 2))));
		}
		else
		{
			SetRectRotation(blackGate + i * 4, angle, 64, 64, Vector2f(edgeA->GetPoint(64 * i + 32 )));
		}
		//SetRectSubRect(blackGate + i * 4, ts_black->GetSubRect(6));
		
	}

	VertexArray & gq = *gQuads;
	//if( type == Gate::GREY || type == Gate::BLACK || keyGate )
	//{
		if( gQuads == NULL )
		{
			//gQuads = new VertexArray( sf::Quads, numVertices );
		}
	//}
	//cout << "giving gquads value!" << endl;
}

void Gate::SetNodeSprite(bool active)
{
	//SetRectSubRect( nodes, ts)
}

void Gate::Update()
{
	//gates can be timeslowed? don't worry about it just yet. 
	if( type != BLACK )
	{
		switch( gState )
		{
		case HARD:
			{
			if (frame == 60)
			{
				
				frame = 0;
			}
				//frame = 0;
				break;
			}
		case TOTALDISSOLVE:
		{
			if (frame == 60)
			{
				gState = OPEN;
				frame = 0;
			}
			break;
		}
			
		case SOFTEN:
			{
				if( frame == 60 )
				{
					gState = SOFT;
					frame = 0;
				}
				break;
			}
			
		case SOFT:
			{
			if( frame == 11 * 3)
				frame = 0;
				break;
			}
			
		case DISSOLVE:
			{
				//whatever length
				if(frame == dissolveLength)
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
				break;
			}
			
		case REFORM:
			{	
				if( frame == 7 * 3 )
				{
					/*for (int i = 0; i < numBlackQuads; ++i)
					{
						SetRectSubRect(blackGate + i * 4, ts_black->GetSubRect(6));
					}*/
					if (type == SECRET)
					{
						gState = HARD;
						frame = 0;
					}
					else
					{
						gState = LOCKFOREVER;
					}
				}
				break;
			}
			
		case LOCKFOREVER:
			{
				frame = 0;
				break;
			}
			
		case OPEN:
			{
				frame = 0;
				break;
			}
		}
	}
	else
	{
		frame = 0;
	}

	if (gState == REFORM)
	{
		for (int i = 0; i < numBlackQuads; ++i)
		{
			//IntRect
			SetRectSubRect(blackGate + i * 4, ts_black->GetSubRect(frame / 3));
		}
	}
	else if (gState == SOFT)
	{
		for (int i = 0; i < numBlackQuads; ++i)
		{
			//IntRect
			SetRectSubRect(blackGate + i * 4, ts_lightning->GetSubRect(frame / 3));
		}
	}

	double radius = 300;

	if (flowFrame > 60)
	{
		flowFrame = 0;
	}

	if( type != BLACK && type != CRAWLER_UNLOCK )
	{
		Zone *currZone = owner->currentZone;
		bool enoughKeys = (owner->keyMarker->keysRequired == 0);

		if( gState == HARD && enoughKeys && 
			( currZone == NULL || ( currZone == zoneA || currZone == zoneB ) ))
		{
			if ( zoneA == zoneB && zoneA == currZone )
			{
				gState = TOTALDISSOLVE;
				SetLocked(false);
				centerShader.setUniform("breakPosQuant", .5f);
			}
			else
			{
				gState = SOFTEN; //SOFTEN
			}
			
			frame = 0;

			Wire *rw = owner->GetPlayer( 0 )->rightWire;
			Wire *lw = owner->GetPlayer( 0 )->leftWire;
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


	
	float ff = flowFrame / 60.f;
	gateShader.setUniform("quant", ff );
	centerShader.setUniform("quant", ff);
	
	float dLen = dissolveLength;

	if (gState == DISSOLVE || gState == TOTALDISSOLVE )
	{
		centerShader.setUniform("breakQuant", (frame / dLen));
	}
	else if (gState == REFORM)
	{
		if (frame <= dLen)
		{
			centerShader.setUniform("breakQuant", ((float)frame / (7*3)));
		}
	}
	if (gState == SOFTEN || gState == TOTALDISSOLVE)
	{
		float gg = (frame / 60.f);
		gateShader.setUniform("fadeQuant", gg);
		//(frame / 60.f));
	}
	++frame;
	++flowFrame;
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
	}
}

void Gate::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool Gate::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsBoxTouchingBox(aabb, r);
}