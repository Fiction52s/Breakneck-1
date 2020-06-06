#include "Gate.h"
#include "GameSession.h"
#include <iostream>
#include "Physics.h"
#include "KeyMarker.h"
#include "MapHeader.h"
#include "Minimap.h"
#include "Enemy_Shard.h"
#include "SaveFile.h"
#include "Wire.h"
#include "Actor.h"
#include "MainMenu.h"


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

Gate::Gate( GameSession *p_owner, int p_cat, int p_var )
	:category( p_cat), variation( p_var ), locked( true ), 
	zoneA( NULL ), zoneB( NULL ),owner( p_owner )
{
	visible = true;
	gateQuads = NULL;

	edgeA = NULL;
	edgeB = NULL;

	ts = NULL;

	ts_lockedAndHardened = owner->GetTileset("Zone/gates_black_32x32.png", 32, 32);
	
	ts_orb = owner->GetSizedTileset("Zone/gate_orb_64x64.png");
	
	orbFrame = 0;
	orbState = ORB_RED;

	stateLength[HARD] = 61;
	stateLength[SOFTEN] = 61;
	stateLength[SOFT] = 11 * 3;
	stateLength[TOTALDISSOLVE] = 61;
	stateLength[REFORM] = 7 * 3;
	stateLength[LOCKFOREVER] = 1;
	stateLength[OPEN] = 1;
	
	if (!gateShader.loadFromFile("Resources/Shader/gate_shader.frag", sf::Shader::Fragment))
	{
		cout << "failed to load gate shader" << endl;
		assert(0);
	}

	if (!centerShader.loadFromFile("Resources/Shader/gatecenter_shader.frag", sf::Shader::Fragment))
	{
		cout << "failed to load gate center shader" << endl;
		assert(0);
	}

	numToOpen = 0;

	numberText.setFont(owner->mainMenu->arial);
	numberText.setCharacterSize(50);
	numberText.setFillColor(Color::Black);
}

Gate::~Gate()
{
	if (gateQuads != NULL)
		delete gateQuads;

	delete edgeA;
	delete edgeB;
}

void Gate::Reset()
{
	centerShader.setUniform("breakQuant", 0.f);
	gateShader.setUniform("fadeQuant", 0.f);
	flowFrame = 0;
	frame = 0;

	orbState = ORB_RED;
	orbFrame = 0;

	if (category == BOSS )
	{
		gState = OPEN;
		SetLocked(false);
	}
	else if (category == BLACK)
	{
		gState = LOCKFOREVER;
	}
	else if (category == SECRET)
	{
		gState = SOFT;
	}
	else
	{
		gState = HARD;
		SetLocked(true);
	}
}

void Gate::ActionEnded()
{
	if (gState == OPEN || gState == LOCKFOREVER)
		return;
	
	if (frame == stateLength[gState])
	{
		frame = 0;
		switch (gState)
		{
		case TOTALDISSOLVE:
		{
			gState = OPEN;
			break;
		}
		case SOFTEN:
		{
			gState = SOFT;
			break;
		}
		case DISSOLVE:
		{
			if (IsReformingType())
			{
				gState = REFORM;
				frame = 0;
			}
			else
			{
				gState = OPEN;
				frame = 0;
			}
			break;
		}
		case REVERSEDISSOLVE:
		{
			gState = HARD;
			frame = 0;
			break;
		}
		case REFORM:
		{
			if (IsTwoWay())
			{
				gState = SOFT;
				frame = 0;
			}
			else if (category == BOSS)
			{
				gState = HARD;
				frame = 0;
				SetLocked(true);
			}
			else
			{
				gState = LOCKFOREVER;
			}
			break;
		}
		}
	}
}

bool Gate::IsZoneType()
{
	return category != BOSS && category != BLACK;
}

void Gate::Reform()
{
	gState = Gate::REFORM;
	frame = 0;
	float aa = .5;
	centerShader.setUniform("breakPosQuant", aa);
}

void Gate::Close()
{
	gState = Gate::HARD;
	frame = 0;
	centerShader.setUniform("breakQuant", 0.f);
	gateShader.setUniform("fadeQuant", 0.f);
	flowFrame = 0;
	SetLocked(true);
}

void Gate::UpdateOrb()
{
	if (category == KEY)
	{


		if (owner->GetPlayer(0)->numKeysHeld >= numToOpen)
		{
			bool currZone = (owner->currentZone == zoneA ||
				owner->currentZone == zoneB);
			if (orbState != ORB_GO && currZone)
			{
				orbState = ORB_GO;
				orbFrame = 0;
			}
			else if (!currZone)
			{
				orbState = ORB_GREEN;
				orbFrame = 0;
			}

			if (orbState == ORB_GO)
			{
				ts_orb->SetQuadSubRect(orbQuad, 2 + orbFrame / 2);

				orbFrame++;
				if (orbFrame == 10 * 2)
				{
					orbFrame = 0;
				}
			}
			else
			{
				ts_orb->SetQuadSubRect(orbQuad, 1);
			}
		}
		else
		{
			ts_orb->SetQuadSubRect(orbQuad, 0);
		}
	}
}

void Gate::Update()
{
	if (category == BLACK)
	{
		return;
	}

	ActionEnded();
	CheckSoften();
	UpdateSprite();
	UpdateShaders();
	UpdateOrb();

	if( gState != LOCKFOREVER && gState != OPEN )
		++frame;
}

void Gate::UpdateShaders()
{
	if (flowFrame > 60)
	{
		flowFrame = 0;
	}

	int tileWidth = ts->tileWidth;
	int tileHeight = ts->tileHeight;
	double gateLength = length(edgeA->v1 - edgeA->v0);
	double numT = gateLength / tileHeight; //rounded down
	int numTiles = numT;
	double remainder = numT - numTiles;
	if (remainder > 0)
		numTiles += 1;

	int numVertices = numTiles * 4;

	double width = tileWidth / 2;
	V2d dv0(edgeA->v0.x, edgeA->v0.y);
	V2d dv1(edgeA->v1.x, edgeA->v1.y);
	V2d along = normalize(dv1 - dv0);
	V2d other(along.y, -along.x);

	V2d leftv0 = dv0 - other * width;
	V2d rightv0 = dv0 + other * width;

	V2d leftv1 = dv1 - other * width;
	V2d rightv1 = dv1 + other * width;

	Vector2f leftv0f(leftv0.x, leftv0.y);
	Vector2f leftv1f(leftv1.x, leftv1.y);
	Vector2f rightv1f(rightv1.x, rightv1.y);
	Vector2f rightv0f(rightv0.x, rightv0.y);
	int f = frame / 3;

	float ff = flowFrame / 60.f;
	gateShader.setUniform("quant", ff);
	centerShader.setUniform("quant", ff);

	float dLen = stateLength[DISSOLVE] - 1;

	if (gState == DISSOLVE || gState == TOTALDISSOLVE )
	{
		centerShader.setUniform("breakQuant", (frame / dLen));
	}
	else if (gState == REVERSEDISSOLVE)
	{
		centerShader.setUniform("breakQuant", 1.f - (frame / dLen));
	}
	else if (gState == REFORM)
	{
		if (frame <= dLen)
		{
			centerShader.setUniform("breakQuant", ((float)frame / (7 * 3)));
		}
	}

	if (gState == SOFTEN || gState == TOTALDISSOLVE)
	{
		float gg = (frame / 60.f);
		gateShader.setUniform("fadeQuant", gg);
	}
	else if( gState == REVERSEDISSOLVE)
	{
		float gg = 1.f - (frame / 60.f);
		gateShader.setUniform("fadeQuant", gg);
	}

	++flowFrame;
}

void Gate::TotalDissolve()
{
	gState = TOTALDISSOLVE;
	frame = 0;
	SetLocked(false);
	centerShader.setUniform("breakPosQuant", .5f);
	ResetAttachedWires();
}

void Gate::ReverseDissolve()
{
	gState = REVERSEDISSOLVE;
	frame = 0;
	SetLocked(true);
	centerShader.setUniform("breakPosQuant", .5f);
}

void Gate::Soften()
{
	assert(gState == HARD);

	gState = SOFTEN;
	frame = 0;
	ResetAttachedWires();
}

void Gate::ResetAttachedWires()
{
	Wire *rw = owner->GetPlayer(0)->rightWire;
	Wire *lw = owner->GetPlayer(0)->leftWire;
	if (rw != NULL && rw->anchor.e == edgeA || rw->anchor.e == edgeB)
	{
		rw->Reset();
	}
	if (lw != NULL && lw->anchor.e == edgeA || lw->anchor.e == edgeB)
	{
		lw->Reset();
	}
}

bool Gate::CanSoften()
{
	if (category == BLACK || category == BOSS || category == SECRET )
	{
		return false;
	}

	Zone *currZone = owner->currentZone;
	Actor * player = owner->GetPlayer(0);

	bool correctStateAndZones = gState == HARD && (currZone == NULL
		|| (currZone == zoneA || currZone == zoneB));

	bool okayToSoften = false;

	if (correctStateAndZones)
	{
		switch (category)
		{
		case KEY:
		{
			if ((player->numKeysHeld >= numToOpen))
				okayToSoften = true;
			break;
		}
		case SHARD:
		{
			double len = length(owner->GetPlayer(0)->position - GetCenter());
			if (owner->IsShardCaptured(shardType) && len < 300)
			{
				okayToSoften = true;
			}
			break;
		}
		case PICKUP:
		{
			//TODO
			break;
		}
		}
	}

	return okayToSoften;
}

void Gate::CheckSoften()
{
	if (category == BLACK || category == BOSS || category == SECRET)
	{
		return;
	}

	bool canSoften = CanSoften();

	Zone *currZone = owner->currentZone;

	if (canSoften)
	{
		if (category == SHARD)
		{
			Soften();
		}
		else
		{
			if (zoneA == zoneB && zoneA == currZone)
			{
				TotalDissolve();
			}
			else
			{
				Soften();
			}
		}
	}
}

void Gate::UpdateSprite()
{
	if (gState == REFORM)
	{
		for (int i = 0; i < numGateQuads; ++i)
		{
			ts_lockedAndHardened->SetQuadSubRect(gateQuads + i * 4, frame / 3);
		}
	}
	else if (gState == SOFT)
	{
		for (int i = 0; i < numGateQuads; ++i)
		{
			ts_wiggle->SetQuadSubRect(gateQuads + i * 4, frame / 3);
		}
	}
}

bool Gate::IsTwoWay()
{
	return category == SECRET || category == SHARD;
}

bool Gate::IsAlwaysUnlocked()
{
	return category == SECRET;
}

bool Gate::IsReformingType()
{
	return true;
}

bool Gate::IsInUnlockableState()
{
	return gState == SOFTEN || gState == SOFT || gState == DISSOLVE || gState == TOTALDISSOLVE;
}

bool Gate::CanUnlock()
{
	if (gState == Gate::OPEN)
	{
		return false;
	}

	//bool unlockableState = IsInUnlockableState();
	/*if (gState != Gate::LOCKFOREVER && gState != Gate::REFORM && gState != Gate::HARD)
	{
		unlockableState = true;
	}*/

	if (IsInUnlockableState())
	{
		if (IsAlwaysUnlocked())
			return true;

		switch (category)
		{
		case BLACK:
			return false;
		case KEY:
		{
			return true;
		}
		case SHARD:
		{
			return true;
			break;
		}
		case BOSS:
		{
			return true;
			break;
		}
		}

	}

	return false;
}

V2d Gate::GetCenter()
{
	return (edgeA->v0 + edgeA->v1) / 2.0;
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



void Gate::Draw( sf::RenderTarget *target )
{
	if (!visible)
		return;
	
	if( gState != OPEN )
	{
		if (gState == REFORM || gState == LOCKFOREVER)
		{
			if (gState == REFORM)
			{
				target->draw(centerLine, 4, sf::Quads, &centerShader);
			}
			target->draw(gateQuads, numGateQuads * 4, sf::Quads, ts_lockedAndHardened->texture );
		}
		else
		{
			if (gState == SOFT)
			{
				target->draw(gateQuads, numGateQuads * 4, sf::Quads, ts_wiggle->texture);
			}

			target->draw(centerLine, 4, sf::Quads, &centerShader);

			if (gState != SOFT)
			{
				target->draw(hardLine, 4, sf::Quads, &gateShader);
			}

			if ( category == KEY || category == PICKUP)
			{
				target->draw(orbQuad, 4, sf::Quads, ts_orb->texture );

				if( orbState != ORB_GO )
				{
					target->draw(numberText);
				}
			}
		}
	}

	target->draw(nodes, 8, sf::Quads, ts_node->texture);

	if (category == Gate::SHARD)
	{
		target->draw(shardSprite);
	}

}

void Gate::MapDraw(sf::RenderTarget *target)
{
	if( locked && visible)
		target->draw(mapLine, 4, sf::Quads);
}

void Gate::SetNumToOpen(int num)
{
	assert(category == KEY || category == PICKUP);

	numToOpen = num;
	numberText.setString(to_string(numToOpen));
	auto &bounds = numberText.getLocalBounds();
	numberText.setOrigin(bounds.left + bounds.width / 2,
		bounds.top + bounds.height / 2);
}

void Gate::SetShard(int w, int li)
{
	assert(category == SHARD);
	
	shardWorld = w;
	shardIndex = li;
	ts_shard = Shard::GetShardTileset(shardWorld, owner);
	//shardSprite.setColor(Color::Black);
	shardSprite.setTexture(*ts_shard->texture);
	shardSprite.setTextureRect(ts_shard->GetSubRect(shardIndex));
	shardSprite.setOrigin(shardSprite.getLocalBounds().width / 2, shardSprite.getLocalBounds().height / 2);

	shardType = Shard::GetShardType(shardWorld, shardIndex);
}

void Gate::SetMapLineColor()
{
	switch (category)
	{
	case BLACK:
		mapLineColor = Color(150, 150, 150);
		break;
	case SHARD:
		mapLineColor = Color::Transparent;
		break;
	case SECRET:
		mapLineColor = Color::Transparent;
		break;
	case BOSS:
		mapLineColor = Color::Blue;
		break;
	case KEY:
	{
		switch (owner->mapHeader->envWorldType)
		{
		case 0:
			mapLineColor = COLOR_BLUE;
			break;
		case 1:
			mapLineColor = COLOR_GREEN;
			break;
		case 2:
			mapLineColor = COLOR_YELLOW;
			break;
		case 3:
			mapLineColor = COLOR_ORANGE;
			break;
		case 4:
			mapLineColor = COLOR_RED;
			break;
		case 5:
			mapLineColor = COLOR_MAGENTA;
			break;
		case 6:
			mapLineColor = COLOR_MAGENTA;
			break;
		}
	}
	case PICKUP:
		//todo
		break;
	}
}

void Gate::Init()
{
	stateLength[DISSOLVE] = 20 * max( 1.0, length(edgeA->v1 - edgeA->v0) / 400.0 );
	stateLength[REVERSEDISSOLVE] = stateLength[DISSOLVE];

	Vector2f centerPos = Vector2f(edgeA->v1 + edgeA->v0) / 2.f;
	SetRectCenter(orbQuad, ts_orb->tileWidth, ts_orb->tileHeight, centerPos);
	numberText.setPosition(centerPos);
	
	double width = 16;
	float tileHeight = 64;

	SetMapLineColor();

	frame = 0;

	if (category == SHARD)
	{
		V2d center = (edgeA->v0 + edgeA->v1) / 2.0;
		shardSprite.setPosition(Vector2f(center));
	}
	else if (category == SECRET)
	{
		//tileHeight = 256;
		//width = 128;
	}

	/*if (category == SECRET)
	{
		ts_wiggle = owner->GetSizedTileset("Zone/secret_gate_loop_256x256.png");
	}
	else*/
	{
		ts_wiggle = owner->GetSizedTileset("Zone/gates_lightning_1_64x64.png");
	}

	ts = owner->GetTileset("Zone/gates_32x64.png", 32, 64);
	gateShader.setUniform("u_texture", *ts->texture);
	gateShader.setUniform("tile", 1.f);
	centerShader.setUniform("u_texture", *ts->texture);
	 //5
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

	mapLine[0].position = Vector2f( leftv0.x, leftv0.y );
	mapLine[1].position = Vector2f( leftv1.x, leftv1.y );
	mapLine[2].position = Vector2f( rightv1.x, rightv1.y );
	mapLine[3].position = Vector2f( rightv0.x, rightv0.y );

	hardLine[0].position = Vector2f(rightv0.x, rightv0.y);
	hardLine[1].position = Vector2f(leftv0.x, leftv0.y);
	hardLine[2].position = Vector2f(leftv1.x, leftv1.y);
	hardLine[3].position = Vector2f(rightv1.x, rightv1.y);

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

	SetRectSubRect(hardLine, ir);
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

	if (gateQuads != NULL)
	{
		delete[] gateQuads;
	}

	gateQuads = new Vertex[numVertices];
	numGateQuads = numTiles;
	

	double angle = atan2(-along.x, along.y);

	float wiggleWidth = ts_wiggle->tileWidth;
	float wiggleHeight = ts_wiggle->tileHeight;
	float wiggleHalfHeight = wiggleHeight / 2;

	for (int i = 0; i < numTiles; ++i)
	{

		if (i == numTiles - 1 && remainder > 0)
		{
			V2d start = edgeA->GetPosition(wiggleHeight * i);
			V2d end = edgeA->v1;
			double h = length(end - start);
			SetRectRotation(gateQuads + i * 4, angle, wiggleWidth, h, 
				Vector2f(edgeA->GetPosition(wiggleHeight * i 
					+ (wiggleHalfHeight - (wiggleHeight - h) / 2))));
		}
		else
		{
			SetRectRotation(gateQuads + i * 4, angle, wiggleWidth, wiggleHeight, 
				Vector2f(edgeA->GetPosition(wiggleHeight * i + wiggleHalfHeight)));
		}	
	}
}

void Gate::SetNodeSprite(bool active)
{
	//SetRectSubRect( nodes, ts)
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