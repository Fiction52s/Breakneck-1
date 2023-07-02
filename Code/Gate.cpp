#include "Gate.h"
#include "Session.h"
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
#include "EditorGateInfo.h"
#include "EditorTerrain.h"
#include "Zone.h"

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

Gate::Gate( Session *p_sess, int p_cat, int p_var )
	:category( p_cat), variation( p_var ),
	zoneA( NULL ), zoneB( NULL ),sess( p_sess )
{
	data.locked = true;

	gateIndex = -1;
	visible = true;
	gateQuads = NULL;

	edgeA = NULL;
	edgeB = NULL;

	ts = NULL;

	ts_lockedAndHardened = sess->GetTileset("Zone/gates_black_32x32.png", 32, 32);
	ts_glitch = NULL;
	
	ts_orb = sess->GetSizedTileset("Zone/gate_orb_64x64.png");
	
	data.orbFrame = 0;
	data.orbState = ORB_RED;

	timeGateIsSecret = false;
	data.secretTimeGateIsOpened = false;
	data.timeLocked = false;

	seconds = 0;

	stateLength[HARD] = 61;
	stateLength[SOFTEN] = 61;
	stateLength[GLITCH] = 7 * 3;
	
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

	numberText.setFont(sess->mainMenu->arial);
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

void Gate::Setup(GateInfoPtr gi)
{
	Edge *edge0 = gi->poly0->GetEdge(gi->point0->index);//allPolysVec[poly0Index]->GetEdge(vertexIndex0);
	Edge *edge1 = gi->poly1->GetEdge(gi->point1->index);//allPolysVec[poly1Index]->GetEdge(vertexIndex1);

	V2d point0 = edge0->v0;
	V2d point1 = edge1->v0;

	if (category == Gate::SHARD)
	{
		SetShard(gi->shardWorld, gi->shardIndex);
	}
	else if (category == Gate::NUMBER_KEY || category == Gate::PICKUP || category == Gate::ALLKEY )
	{
		SetNumToOpen(gi->numToOpen);
	}
	else if (category == Gate::TIME_GLOBAL || category == Gate::TIME_ROOM)
	{
		SetTime(gi->seconds);
	}

	temp0prev = edge0->edge0;
	temp0next = edge0;
	temp1prev = edge1->edge0;
	temp1next = edge1;

	edgeA = new Edge;
	edgeA->edgeType = Edge::CLOSED_GATE;
	edgeA->info = this;
	edgeB = new Edge;
	edgeB->edgeType = Edge::CLOSED_GATE;
	edgeB->info = this;

	edgeA->v0 = point0;
	edgeA->v1 = point1;

	edgeB->v0 = point1;
	edgeB->v1 = point0;

	edgeA->CalcAABB();
	edgeB->CalcAABB();

	next = NULL;
	prev = NULL;

	CalcAABB();

	gateIndex = sess->gates.size();
	sess->gates.push_back(this);

	SetLocked(true);

	Init();

	sess->terrainTree->Insert(edgeA);
	sess->terrainTree->Insert(edgeB);

	//cout << "inserting gate: " << gate->edgeA << endl;
	sess->gateTree->Insert(this);

	Reset();
}

void Gate::Reset()
{
	centerShader.setUniform("breakQuant", 0.f);
	gateShader.setUniform("fadeQuant", 0.f);
	data.flowFrame = 0;
	data.frame = 0;

	data.secretTimeGateIsOpened = false;
	data.timeLocked = false;

	data.orbState = ORB_RED;
	data.orbFrame = 0;

	if (category == BOSS )
	{
		data.gState = OPEN;
		SetLocked(false);
	}
	else if (category == BLACK)
	{
		data.gState = LOCKFOREVER;
	}
	else if (category == SECRET)
	{
		data.gState = SOFT;
	}
	else if (category == TIME_GLOBAL
		|| category == TIME_ROOM)
	{
		data.gState = SOFT;
	}
	else
	{
		data.gState = HARD;
		SetLocked(true);
	}
}

void Gate::PassThrough(double alongAmount)
{
	if (category == SECRET || timeGateIsSecret )
	{
		data.gState = Gate::GLITCH;
		data.frame = 0;
		sess->LockGate(this);
	}
	else if (IsReformingType())
	{
		sess->LockGate(this);

		data.gState = Gate::REFORM;
		data.frame = 0;
		float aa = alongAmount;
		centerShader.setUniform("breakPosQuant", aa);
	}
	else
	{
		data.gState = Gate::DISSOLVE;
		data.frame = 0;
		float aa = alongAmount;
		centerShader.setUniform("breakPosQuant", aa);
	}
}

void Gate::ActionEnded()
{
	if (data.gState == OPEN || data.gState == LOCKFOREVER)
		return;
	
	if (data.frame == stateLength[data.gState])
	{
		data.frame = 0;
		switch (data.gState)
		{
		case TOTALDISSOLVE:
		{
			data.gState = OPEN;
			break;
		}
		case SOFTEN:
		{
			data.gState = SOFT;
			break;
		}
		case DISSOLVE:
		{
			if (IsReformingType())
			{
				data.gState = REFORM;
				data.frame = 0;
			}
			else
			{
				data.gState = OPEN;
				data.frame = 0;
			}
			break;
		}
		case REVERSEDISSOLVE:
		{
			data.gState = HARD;
			data.frame = 0;
			break;
		}
		case REFORM:
		{
			if (IsTwoWay())
			{
				data.gState = SOFT;
				data.frame = 0;
			}
			else if (category == BOSS)
			{
				data.gState = HARD;
				data.frame = 0;
				SetLocked(true);
			}
			else
			{
				data.gState = LOCKFOREVER;
			}
			break;
		}
		case GLITCH:
			data.gState = SOFT;
			break;
		}
	}
}

bool Gate::IsZoneType()
{
	return category != BOSS && category != BLACK;
}

void Gate::Reform()
{
	data.gState = Gate::REFORM;
	data.frame = 0;
	float aa = .5;
	centerShader.setUniform("breakPosQuant", aa);
}

void Gate::Close()
{
	data.gState = Gate::HARD;
	data.frame = 0;
	centerShader.setUniform("breakQuant", 0.f);
	gateShader.setUniform("fadeQuant", 0.f);
	data.flowFrame = 0;
	SetLocked(true);
}

void Gate::UpdateOrb()
{
	if (category == NUMBER_KEY || category == ALLKEY)
	{
		if (data.gState == LOCKFOREVER || data.gState == REFORM)
		{
			SetRectColor(mapLine, mapLineColor);
			return;
		}

		int numKeysHeld = sess->GetPlayer(0)->numKeysHeld;

		if ( numKeysHeld >= numToOpen)
		{
			bool currZone = (sess->currentZone == zoneA ||
				sess->currentZone == zoneB);
			if (data.orbState != ORB_GO && currZone)
			{
				data.orbState = ORB_GO;
				data.orbFrame = 0;
			}
			else if (!currZone)
			{
				data.orbState = ORB_GREEN;
				data.orbFrame = 0;
			}

			if (data.orbState == ORB_GO)
			{
				ts_orb->SetQuadSubRect(orbQuad, 2 + data.orbFrame / 2);

				data.orbFrame++;
				if (data.orbFrame == 10 * 2)
				{
					data.orbFrame = 0;
				}

				int mapLineFrame = (data.orbFrame / 2) % 3;
				switch (mapLineFrame)
				{
				case 0:
					SetRectColor(mapLine, mapLineColor);
					break;
				case 1:
					SetRectColor(mapLine, Color::Red);
					break;
				case 2:
					SetRectColor(mapLine, Color::Yellow);
					break;
				}
			}
			else
			{
				ts_orb->SetQuadSubRect(orbQuad, 1);
				SetRectColor(mapLine, mapLineColor);
			}
		}
		else
		{
			numberText.setString(to_string(numToOpen - numKeysHeld));
			auto &bounds = numberText.getLocalBounds();
			numberText.setOrigin(bounds.left + bounds.width / 2,
				bounds.top + bounds.height / 2);
			ts_orb->SetQuadSubRect(orbQuad, 0);
			SetRectColor(mapLine, mapLineColor);
		}
	}
	else if (category == ENEMY )
	{
		if (data.gState == LOCKFOREVER || data.gState == REFORM)
		{
			SetRectColor(mapLine, mapLineColor);
			return;
		}

		int numRemainingEnemies = sess->currentZone->GetNumRemainingKillableEnemies();

		numberText.setString(to_string(numRemainingEnemies));
		auto &bounds = numberText.getLocalBounds();
		numberText.setOrigin(bounds.left + bounds.width / 2,
			bounds.top + bounds.height / 2);
		if ( sess->currentZone != NULL 
			&& sess->currentZone->GetNumRemainingKillableEnemies() == 0 )
		{
			bool currZone = (sess->currentZone == zoneA ||
				sess->currentZone == zoneB);
			if (data.orbState != ORB_GO && currZone)
			{
				data.orbState = ORB_GO;
				data.orbFrame = 0;
			}
			else if (!currZone)
			{
				data.orbState = ORB_GREEN;
				data.orbFrame = 0;
			}

			if (data.orbState == ORB_GO)
			{
				ts_orb->SetQuadSubRect(orbQuad, 2 + data.orbFrame / 2);

				data.orbFrame++;
				if (data.orbFrame == 10 * 2)
				{
					data.orbFrame = 0;
				}

				int mapLineFrame = (data.orbFrame / 2) % 3;
				switch (mapLineFrame)
				{
				case 0:
					SetRectColor(mapLine, mapLineColor);
					break;
				case 1:
					SetRectColor(mapLine, Color::Red);
					break;
				case 2:
					SetRectColor(mapLine, Color::Yellow);
					break;
				}
			}
			else
			{
				ts_orb->SetQuadSubRect(orbQuad, 1);
				SetRectColor(mapLine, mapLineColor);
			}
		}
		else
		{
			ts_orb->SetQuadSubRect(orbQuad, 0);
			SetRectColor(mapLine, mapLineColor);
		}
	}
	else if (category == TIME_GLOBAL || category == TIME_ROOM)
	{
		if (data.gState == LOCKFOREVER || data.gState == REFORM)
		{
			SetRectColor(mapLine, mapLineColor);
		}

		if (!data.timeLocked)
		{
			data.orbState = ORB_GREEN;
			data.orbFrame = 0;
			ts_orb->SetQuadSubRect(orbQuad, 1);
			SetRectColor(mapLine, mapLineColor);
		}
		else
		{
			ts_orb->SetQuadSubRect(orbQuad, 0);
			SetRectColor(mapLine, mapLineColor);
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
	CheckTimeLock();
	UpdateSprite();
	UpdateShaders();
	UpdateOrb();

	

	if(data.gState != LOCKFOREVER && data.gState != OPEN )
		++data.frame;
}

void Gate::UpdateShaders()
{
	if (data.flowFrame > 60)
	{
		data.flowFrame = 0;
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
	int f = data.frame / 3;

	float ff = data.flowFrame / 60.f;
	gateShader.setUniform("quant", ff);
	centerShader.setUniform("quant", ff);

	float dLen = stateLength[DISSOLVE] - 1;

	if (data.gState == DISSOLVE || data.gState == TOTALDISSOLVE )
	{
		centerShader.setUniform("breakQuant", (data.frame / dLen));
	}
	else if (data.gState == REVERSEDISSOLVE)
	{
		centerShader.setUniform("breakQuant", 1.f - (data.frame / dLen));
	}
	else if (data.gState == REFORM)
	{
		if (data.frame <= dLen)
		{
			centerShader.setUniform("breakQuant", ((float)data.frame / (7 * 3)));
		}
	}

	if (data.gState == SOFTEN || data.gState == TOTALDISSOLVE)
	{
		float gg = (data.frame / 60.f);
		gateShader.setUniform("fadeQuant", gg);
	}
	else if(data.gState == REVERSEDISSOLVE)
	{
		float gg = 1.f - (data.frame / 60.f);
		gateShader.setUniform("fadeQuant", gg);
	}

	++data.flowFrame;
}

void Gate::TotalDissolve()
{
	data.gState = TOTALDISSOLVE;
	data.frame = 0;
	SetLocked(false);
	centerShader.setUniform("breakPosQuant", .5f);
	ResetAttachedWires();
}

void Gate::ReverseDissolve()
{
	data.gState = REVERSEDISSOLVE;
	data.frame = 0;
	SetLocked(true);
	centerShader.setUniform("breakPosQuant", .5f);
}

void Gate::Soften()
{
	assert(data.gState == HARD);

	SetLocked(false);
	data.gState = SOFTEN;
	data.frame = 0;
	ResetAttachedWires();
}

void Gate::ResetAttachedWires()
{
	Wire *rw = sess->GetPlayer(0)->rightWire;
	Wire *lw = sess->GetPlayer(0)->leftWire;
	if (rw != NULL )
	{
		Edge *e = sess->GetEdge(&rw->data.anchor.edgeInfo);
		if (e == edgeA || e == edgeB)
		{
			rw->Reset();
		}
	}

	if (lw != NULL )
	{
		Edge *e = sess->GetEdge(&lw->data.anchor.edgeInfo);
		if (e == edgeA || e == edgeB)
		{
			lw->Reset();
		}
	}
}

bool Gate::CanSoften()
{
	if (category == BLACK || category == BOSS || category == SECRET )
	{
		return false;
	}

	Zone *currZone = sess->currentZone;
	Actor * player = sess->GetPlayer(0);

	bool correctStateAndZones = data.gState == HARD && (currZone == NULL
		|| (currZone == zoneA || currZone == zoneB));

	bool okayToSoften = false;

	if (correctStateAndZones)
	{
		switch (category)
		{
		case ALLKEY:
		case NUMBER_KEY:
		{
			if ((player->numKeysHeld >= numToOpen))
				okayToSoften = true;
			break;
		}
		case ENEMY:
		{
			if (currZone != NULL)
			{
				if (currZone->GetNumRemainingKillableEnemies() == 0)
				{
					okayToSoften = true;
				}
			}
			else
			{
				okayToSoften = false;
			}
			
			break;
		}
		case SHARD:
		{
			double len = length(sess->GetPlayer(0)->position - GetCenter());
			if (sess->IsShardCaptured(shardType) && len < 300)
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
		case TIME_GLOBAL:
		case TIME_ROOM:
		{
			okayToSoften = true;
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

	Zone *currZone = sess->currentZone;

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

void Gate::SetToTwoWay()
{
	if (category == TIME_GLOBAL || category == TIME_ROOM)
	{
		timeGateIsSecret = true;
	}
	else
	{
		category = SECRET;
	}
}

void Gate::CheckTimeLock()
{
	if (category != TIME_GLOBAL && category != TIME_ROOM )
	{
		return;
	}

	if (data.gState == SOFT && !data.secretTimeGateIsOpened)
	{
		int totalTime = -1;
		if (category == TIME_GLOBAL)
		{
			totalTime = sess->totalGameFrames;
		}
		else
		{
			totalTime = sess->currentZone->GetFramesSinceActivation();
		}

		if (totalTime >= seconds * 60)
		{
			data.timeLocked = true;
			Reform();
			numberText.setString(to_string(seconds));
			auto &bounds = numberText.getLocalBounds();
			numberText.setOrigin(bounds.left + bounds.width / 2,
				bounds.top + bounds.height / 2);
		}
		else
		{
			if (category == TIME_GLOBAL || category == TIME_ROOM)
			{
				if (data.gState == SOFT )
				{
					numberText.setString(to_string(seconds - totalTime / 60));
					auto &bounds = numberText.getLocalBounds();
					numberText.setOrigin(bounds.left + bounds.width / 2,
						bounds.top + bounds.height / 2);
				}
			}
		}
	}

	

	bool canSoften = CanSoften();

	Zone *currZone = sess->currentZone;

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
	if (data.gState == REFORM)
	{
		for (int i = 0; i < numGateQuads; ++i)
		{
			ts_lockedAndHardened->SetQuadSubRect(gateQuads + i * 4, data.frame / 3);
		}
	}
	else if (data.gState == SOFT)
	{
		for (int i = 0; i < numGateQuads; ++i)
		{
			ts_wiggle->SetQuadSubRect(gateQuads + i * 4, data.frame / 3);
		}
	}
	else if (data.gState == GLITCH)
	{
		for (int i = 0; i < numGateQuads; ++i)
		{
			ts_glitch->SetQuadSubRect(gateQuads + i * 4, data.frame / 3);
		}
	}
}

bool Gate::IsTwoWay()
{
	return category == SECRET || category == SHARD
		|| ( (category == TIME_GLOBAL || category == TIME_ROOM) && timeGateIsSecret && !data.timeLocked);
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
	return data.gState == SOFTEN || data.gState == SOFT || data.gState == DISSOLVE || data.gState == TOTALDISSOLVE;
}

bool Gate::CanUnlock()
{
	if (data.gState == Gate::OPEN)
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
		case ALLKEY:
		case NUMBER_KEY:
		{
			return true;
		}
		case TIME_GLOBAL:
		case TIME_ROOM:
		{
			return true;
		}
		case ENEMY:
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



void Gate::Draw(sf::RenderTarget *target)
{
	if (!visible)
		return;

	bool isTimeGate = category == TIME_GLOBAL || category == TIME_ROOM;

	if(data.gState != OPEN )
	{
		if (data.gState == REFORM || data.gState == LOCKFOREVER)
		{
			if (data.gState == REFORM)
			{
				if (category == SECRET || timeGateIsSecret)
				{

				}
				else
				{
					target->draw(centerLine, 4, sf::Quads, &centerShader);
				}

			}
			target->draw(gateQuads, numGateQuads * 4, sf::Quads, ts_lockedAndHardened->texture);

			//&& !((gState == REFORM || gState == LOCKFOREVER) && !timeLocked))
			if (isTimeGate && data.timeLocked)
			{
				target->draw(orbQuad, 4, sf::Quads, ts_orb->texture);
				if (!data.secretTimeGateIsOpened)
				{
					target->draw(numberText);
				}
			}
		}
		else if (data.gState == GLITCH)
		{
			target->draw(gateQuads, numGateQuads * 4, sf::Quads, ts_glitch->texture);
		}
		else
		{
			if (data.gState == SOFT)
			{
				target->draw(gateQuads, numGateQuads * 4, sf::Quads, ts_wiggle->texture);
			}

			if (category != SECRET)
			{
				if (!(isTimeGate && timeGateIsSecret))
				{
					target->draw(centerLine, 4, sf::Quads, &centerShader);
				}
				
				

				if (data.gState != SOFT)
				{
					target->draw(hardLine, 4, sf::Quads, &gateShader);
				}

				if (data.gState != TOTALDISSOLVE)
				{
					if (category == NUMBER_KEY || category == ALLKEY || category == PICKUP
						|| category == ENEMY || (isTimeGate && !(timeGateIsSecret && data.secretTimeGateIsOpened)))
					{
						target->draw(orbQuad, 4, sf::Quads, ts_orb->texture);

						if (data.orbState != ORB_GO)
						{
							target->draw(numberText);
						}
					}
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
	if(data.locked && visible && category != SECRET )
		target->draw(mapLine, 4, sf::Quads);
}

void Gate::OpenSecretTimeGate()
{
	if (category == TIME_GLOBAL || category == TIME_ROOM)
	{
		if (timeGateIsSecret)
		{
			data.secretTimeGateIsOpened = true;
		}
	}
}

void Gate::SetNumToOpen(int num)
{
	assert(category == NUMBER_KEY || category == PICKUP || category == ALLKEY );

	numToOpen = num;
	numberText.setString(to_string(numToOpen));
	auto &bounds = numberText.getLocalBounds();
	numberText.setOrigin(bounds.left + bounds.width / 2,
		bounds.top + bounds.height / 2);
}

void Gate::SetTime(int sec)
{
	assert(category == TIME_GLOBAL || category == TIME_ROOM);

	seconds = sec;
	numberText.setString(to_string(seconds));
	auto &bounds = numberText.getLocalBounds();
	numberText.setOrigin(bounds.left + bounds.width / 2,
		bounds.top + bounds.height / 2);
}

void Gate::SetShard(int w, int li)
{
	assert(category == SHARD);
	
	shardWorld = w;
	shardIndex = li;
	ts_shard = Shard::GetShardTileset(shardWorld, sess);
	//shardSprite.setColor(Color::Black);
	shardSprite.setTexture(*ts_shard->texture);
	shardSprite.setTextureRect(ts_shard->GetSubRect(shardIndex));
	shardSprite.setOrigin(shardSprite.getLocalBounds().width / 2, shardSprite.getLocalBounds().height / 2);

	shardType = Shard::GetShardTypeFromWorldAndIndex(shardWorld, shardIndex);
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
	case ALLKEY:
	case NUMBER_KEY:
	{
		mapLineColor = Color::Cyan;
		/*switch (sess->mapHeader->envWorldType)
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
		}*/
	}
	case ENEMY:
	{
		mapLineColor = Color::Magenta;
		break;
	}
	case TIME_GLOBAL:
	case TIME_ROOM:
	{
		mapLineColor = Color::Green;
		break;
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

	data.frame = 0;


	

	if (category == SHARD)
	{
		V2d center = (edgeA->v0 + edgeA->v1) / 2.0;
		shardSprite.setPosition(Vector2f(center));
	}
	else if (category == SECRET || category == TIME_GLOBAL || category == TIME_ROOM )
	{
		ts_glitch = sess->GetSizedTileset("Zone/gate_glitch_64x64.png");
		//tileHeight = 256;
		//width = 128;
	}

	if (category == SECRET)
	{
		ts_wiggle = sess->GetSizedTileset("Zone/gate_glitch_loop_64x64.png");
		stateLength[SOFT] = 5 * 3;
	}
	else if( category == ENEMY )
	{
		ts_wiggle = sess->GetSizedTileset("Zone/gates_lightning_5_64x64.png");
		stateLength[SOFT] = 11 * 3;
	}
	else if (category == ALLKEY || category == NUMBER_KEY)
	{
		ts_wiggle = sess->GetSizedTileset("Zone/gates_lightning_1_64x64.png");
		stateLength[SOFT] = 11 * 3;
	}
	else if (category == SHARD)
	{
		ts_wiggle = sess->GetSizedTileset("Zone/gates_lightning_2_64x64.png");
		stateLength[SOFT] = 11 * 3;
	}
	else if (category == TIME_GLOBAL || category == TIME_ROOM)
	{
		ts_wiggle = sess->GetSizedTileset("Zone/gates_lightning_3_64x64.png");
		stateLength[SOFT] = 11 * 3;
	}
	else
	{
		ts_wiggle = sess->GetSizedTileset("Zone/gates_lightning_1_64x64.png");
		stateLength[SOFT] = 11 * 3;
	}

	ts = sess->GetTileset("Zone/gates_32x64.png", 32, 64);
	gateShader.setUniform("u_texture", *ts->texture);

	if (category == ALLKEY || category == NUMBER_KEY)
	{
		gateShader.setUniform("tile", 7.f);
	}
	else if (category == ENEMY)
	{
		gateShader.setUniform("tile", 11.f);
	}
	else if (category == SHARD)
	{
		gateShader.setUniform("tile", 8.f);
	}
	else if (category == TIME_GLOBAL || category == TIME_ROOM)
	{
		gateShader.setUniform("tile", 9.f);
	}
	else
	{
		gateShader.setUniform("tile", 7.f);
	}

	
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

	 ts_node = sess->GetTileset("Zone/gatenode_32x64.png", 32, 64);

	SetRectSubRect(nodes, ts_node->GetSubRect(0) );
	SetRectSubRect((nodes+4), ts_node->GetSubRect(0));

	FloatRect ir;
	

	if (category == ALLKEY || category == NUMBER_KEY)
	{
		centerShader.setUniform("tile", 0.f);
	}
	else if (category == SHARD)
	{
		centerShader.setUniform("tile", 1.f);
	}
	else if (category == TIME_GLOBAL || category == TIME_ROOM)
	{
		centerShader.setUniform("tile", 2.f);
	}
	else if (category == ENEMY)
	{
		centerShader.setUniform("tile", 4.f);
	}
	else
	{
		centerShader.setUniform("tile", 0.f);
	}

	

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

	Reset();
}

void Gate::SetNodeSprite(bool active)
{
	//SetRectSubRect( nodes, ts)
}

void Gate::SetLocked( bool on )
{
	if( on )
	{
		data.locked = true;

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
		data.locked = false;

		//edgeA->edgeType = Edge::OPEN_GATE;
		//edgeB->edgeType = Edge::OPEN_GATE;

		edgeA->edgeType = Edge::CLOSED_GATE;
		edgeB->edgeType = Edge::CLOSED_GATE;

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

bool Gate::IsSoft()
{
	return data.gState == SOFTEN || data.gState == SOFT;
}

bool Gate::IsLocked()
{
	return data.locked;
}

bool Gate::IsLockedForever()
{
	return data.gState == REFORM || data.gState == LOCKFOREVER;
}

void Gate::Open()
{
	data.gState = Gate::OPEN;
	data.frame = 0;
	sess->UnlockGate(this);
}

bool Gate::IsReformable()
{
	return data.gState == Gate::HARD || data.gState == Gate::SOFT || data.gState == Gate::SOFTEN;
}

bool Gate::CanBeHitByWire()
{
	return category == Gate::BLACK || IsLockedForever() || data.gState == HARD;
}


bool Gate::IsSecret()
{
	return category == SECRET;
}

int Gate::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Gate::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &data, sizeof(MyData));
}

void Gate::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetLocked(data.locked);
}

