#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CrawlerQueen.h"
#include "VisualEffects.h"
//#include "StorySequence.h"
#include "SequenceW1.h"
#include "MusicPlayer.h"
#include "MainMenu.h"
#include "MapHeader.h"
#include "Actor.h"

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

CrawlerQueen::CrawlerQueen(ActorParams *ap )//Edge *g, double q, bool cw )
	:Enemy(EnemyType::EN_CRAWLERQUEEN, ap )//, false, 1, false), clockwise(cw)
{
	SetNumActions(Count);
	SetEditorActions(DECIDE, DECIDE, 0);

	BossCrawlerParams *cParams = (BossCrawlerParams*)ap;

	SetLevel(ap->enemyLevel);

	ts_decideMarker = sess->GetTileset("Bosses/Crawler/crawler_queen_marker_64x64.png", 64, 64);

	decideVA = new Vertex[MAX_DECISIONS * 4 * 3];
	
	postFightScene = NULL;

	progressionLevel = 0;
	
	redecide = false;
	origFacingRight = false;
	facingRight = origFacingRight;

	CreateSurfaceMover(startPosInfo, 80, this);
	
	baseSpeed = 8;
	numDecisions = 3;
	bombSpeed = 2;
	currDigAttacks = 3;

	totalInvincFramesOnHit = 60;
	invincHitThresh = 3;
	notHitCap = 10;

	decMarkerPool = new EffectPool(EffectType::FX_REGULAR, MAX_DECISIONS, 1.f);
	decideIndex = 0;

	actionLength[DECIDE] = 1;
	actionLength[BOOST] = 1;
	actionLength[TURNAROUNDBOOST] = 4;
	actionLength[STOPBOOST] = 1;
	actionLength[WAIT] = 1;
	actionLength[JUMP] = 3;
	actionLength[BURROW] = 22;
	actionLength[RUMBLE] = 20;
	actionLength[POPOUT] = 27;
	actionLength[UNBURROW] = 12;

	actionLength[INITIALUNBURROW] = 9;
	actionLength[INITIALWAIT] = 1;
	actionLength[INITIALIDLE] = 1;
	actionLength[SEQ_ANGRY0] = 60;
	actionLength[SEQ_FINISHINITIALUNBURROW] = 3;
	

	animFactor[DECIDE] = 2;
	animFactor[BOOST] = 1;
	animFactor[TURNAROUNDBOOST] = 2;
	animFactor[STOPBOOST] = 2;
	animFactor[WAIT] = 1;
	animFactor[JUMP] = 2;
	animFactor[BURROW] = 2;
	animFactor[RUMBLE] = 1;
	animFactor[POPOUT] = 2;
	animFactor[UNBURROW] = 2;

	animFactor[SEQ_FINISHINITIALUNBURROW] = 3;
	animFactor[INITIALUNBURROW] = 2;
	animFactor[INITIALWAIT] = 1;
	animFactor[INITIALIDLE] = 1;
	animFactor[SEQ_ANGRY0] = 1;

	double width = 320;
	double height = 320;

	ts[WAIT] = sess->GetTileset("Bosses/Crawler/crawler_queen_stand_320x320.png", 320, 320);
	ts[DECIDE] = sess->GetTileset("Bosses/Crawler/crawler_queen_stand_320x320.png", 320, 320);
	ts[BOOST] = sess->GetTileset("Bosses/Crawler/crawler_queen_dash_320x320.png", 320, 320);
	ts[TURNAROUNDBOOST] = sess->GetTileset("Bosses/Crawler/crawler_queen_dash_320x320.png", 320, 320);
	ts[STOPBOOST] = sess->GetTileset("Bosses/Crawler/crawler_queen_dash_320x320.png", 320, 320);
	ts[JUMP] = sess->GetTileset("Bosses/Crawler/crawler_queen_jump_320x320.png", 320, 320);
	ts[BURROW] = sess->GetTileset("Bosses/Crawler/crawler_queen_dig_in_320x320.png", 320, 320);
	ts[RUMBLE] = NULL;//owner->GetTileset("Bosses/crawler_queen_dash_320x320.png", 320, 320);
	ts[POPOUT] = sess->GetTileset("Bosses/Crawler/crawler_queen_slash_320x320.png", 320, 320);
	ts[UNBURROW] = sess->GetTileset("Bosses/Crawler/crawler_queen_dig_out_320x320.png", 320, 320);
	ts[INITIALUNBURROW] = ts[UNBURROW];
	ts[INITIALIDLE] = ts[UNBURROW];
	ts[SEQ_ANGRY0] = ts[UNBURROW];
	ts[SEQ_FINISHINITIALUNBURROW] = ts[UNBURROW];

	decideDelayFrames = 20;

	multSpeed = 1;

	SetOffGroundHeight(height / 2.0);
	SetCurrPosInfo(startPosInfo);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	BasicCircleHurtBodySetup(70);

	BasicCircleHitBodySetup(70);

	hitBody.hitboxInfo = hitboxInfo;

	frame = 0;

	edgeRef = NULL;

	

	bombPool = new ObjectPool;
	
	for (int i = 0; i < 100; ++i)
	{
		FloatingBomb *fb = new FloatingBomb( ap, bombPool, i );
		bombPool->AddToInactiveList(fb);
	}

	decisionPool = new ObjectPool;
	for (int i = 0; i < MAX_DECISIONS; ++i)
	{
		DecisionMarker *d = new DecisionMarker(this, decisionPool, i * 3);
		decisionPool->AddToInactiveList(d);
	}

	decidePoints = new EdgeInfo[MAX_DECISIONS];
	decisions = new Decision[MAX_DECISIONS];


	ResetEnemy();
}

void CrawlerQueen::ClearDecisionMarkers()
{
	for (int i = 0; i < MAX_DECISIONS * 4 * 3; ++i)
	{
		decideVA[i].position = Vector2f(0, 0);
	}
}

CrawlerQueen::~CrawlerQueen()
{
	if (postFightScene != NULL)
	{
		delete postFightScene;
	}

	delete decMarkerPool;

	delete[] decideVA;

	delete[] edgeRef;
	delete[] decidePoints;
	delete[] decisions;

	bombPool->DestroyAllMembers();
	decisionPool->DestroyAllMembers();

	delete decisionPool;
	delete bombPool;
}

void CrawlerQueen::InitOnRespawn()
{
	if (game != NULL && zone != NULL)
		zone->action = Zone::OPEN;
}

void CrawlerQueen::ResetEnemy()
{
	ClearDecisionMarkers();
	decisionPool->DeactivateAll();
	invinc = false;
	currInvincFramesOnHit = 0;
	invincHitCount = 0;
	redecide = false;
	SetDifficulty(0);
	
	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	
	decideIndex = 0;

	frame = 0;

	UpdateHitboxes();

	action = INITIALWAIT;
	frame = 0;

	DeactivateAllBombs();


	UpdateSprite();
}

void CrawlerQueen::UpdateHitboxes()
{
	BasicUpdateHitboxes();
	
	if (surfaceMover->ground != NULL)
	{
		V2d gn = surfaceMover->ground->Normal();
		double angle = 0;


		angle = atan2(gn.x, -gn.y);

		V2d knockbackDir(1, -1);
		knockbackDir = normalize(knockbackDir);
		double maxExtraKB = 15.0;
		double baseKB = 8;
		if (surfaceMover->groundSpeed > 0)
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = baseKB + max(abs(surfaceMover->groundSpeed), maxExtraKB);
		}
		else
		{
			hitboxInfo->kbDir = V2d(-knockbackDir.x, knockbackDir.y);
			hitboxInfo->knockback = baseKB + max(abs(surfaceMover->groundSpeed), maxExtraKB);
		}
	}
	else
	{
	}
}

HitboxInfo *CrawlerQueen::IsHit(int pIndex)
{
	Actor *player = sess->GetPlayer(pIndex);

	if (player->IntersectMyHitboxes(currHurtboxes, currHurtboxFrame))
	{
		if (!invinc)
		{
			hitThisFrame = true;
			++invincHitCount;
			if (invincHitCount == invincHitThresh)
			{
				invinc = true;
				invincHitCount = 0;
				currInvincFramesOnHit = 0;
			}
			return player->currHitboxes->hitboxInfo;
		}
	}

	return NULL;
}

void CrawlerQueen::StartAngryYelling()
{
	assert(action == INITIALIDLE);
	action = SEQ_ANGRY0;
	frame = 0;
}

void CrawlerQueen::StartFight()
{
	assert(action == SEQ_ANGRY0);
	action = SEQ_FINISHINITIALUNBURROW;
	frame = 0;
}


void CrawlerQueen::StartInitialUnburrow()
{
	assert(action == INITIALWAIT);
	action = INITIALUNBURROW;
	frame = 0;
}

void CrawlerQueen::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case INITIALWAIT:
			frame = 0;
			break;
		case INITIALIDLE:
			frame = 0;
			break;
		case INITIALUNBURROW:
			action = INITIALIDLE;
			frame = 0;
			break;
		case HURT:
			break;
		case WAIT:
			break;
		case BOOST:
		{
			break;
		}
		case DECIDE:
			//cout << "decide: " << frame << endl;
			break;
		case TURNAROUNDBOOST:
			break;
		case STOPBOOST:
			break;
		case JUMP:
			break;
		case BURROW:
			action = RUMBLE;
			frame = 0;
			digAttackCounter = 0;
			break;
		case RUMBLE:
			action = POPOUT;
			//mover->ground = decidePoints[digAttackCounter].edge;
			//mover->edgeQuantity = decidePoints[digAttackCounter].quantity;
			//mover->UpdateGroundPos();
			frame = 0;
			break;
		case POPOUT:
			++digAttackCounter;
			if (digAttackCounter == numDecisions)
			{
				//mover->ground = digInfo.edge;
				//mover->edgeQuantity = digInfo.quantity;
				//mover->UpdateGroundPos();
				action = UNBURROW;
				frame = 0;
			}
			else
			{
				action = RUMBLE;
				frame = 0;
			}
			break;
		case UNBURROW:
			Boost();
			break;
		case SEQ_ANGRY0:
			action = SEQ_ANGRY0;
			frame = 0;
			//alert the game UI/kin/player that the fight has started
			break;
		case SEQ_FINISHINITIALUNBURROW:
			action = DECIDE;
			frame = 0;
			SetHurtboxes(&hurtBody, 0);
			SetHitboxes(&hitBody, 0);
			SetDecisions();
			DecidePoints();
			break;
		}
	}
	
	if (redecide)
	{
		action = DECIDE;
		SetDecisions();
		DecidePoints();
		SetDifficulty(0);
		frame = 0;
		decideIndex = 0;
		surfaceMover->SetSpeed(0);
		redecide = false;
	}

//	cout << "action: " << action << endl;
	switch (action)
	{
	case WAIT:
		break;
	case BOOST:
	{
		assert(surfaceMover->ground != NULL);
		break;
	}
	case DECIDE:
		if (frame == 0)
		{
			//DecidePoints();
			//SetDecisions();
		}
		if (frame % decideDelayFrames == 0 && frame > 0)
		{
			if (decideIndex == numDecisions - 1)
			{
				startTravelPoint.edge = surfaceMover->ground;
				startTravelPoint.quantity = surfaceMover->edgeQuantity;
				startTravelPoint.index = edgeIndexMap[surfaceMover->ground];
				travelIndex = 0;
				redecide = false;
				DecideAction();
			}
			else
			{
				decideIndex++;
			}
		}
		break;
	case TURNAROUNDBOOST:
		break;
	case STOPBOOST:
		break;
	case JUMP:
		break;
	case BURROW:
		break;
	case RUMBLE:
		break;
	case POPOUT:

		break;
	case UNBURROW:
		break;
	}

	switch (action)
	{
	case WAIT:
		break;
	case DECIDE:
		break;
	case TURNAROUNDBOOST:
		break;
	case STOPBOOST:
		break;
	case JUMP:
		break;
	case BURROW:
		break;
	case RUMBLE:
		break;
	case POPOUT:
		if (frame == 25)
		{
			FloatingBomb *fb = (FloatingBomb*)bombPool->ActivatePoolMember();
			assert(fb != NULL);
			fb->Init(surfaceMover->ground->GetPosition(surfaceMover->edgeQuantity), surfaceMover->ground->Normal() * bombSpeed);
			sess->AddEnemy(fb);
			cout << "spawning bomb: " << fb << ": " << fb->GetPosition().x << ", " << fb->GetPosition().y << endl;
		}
		break;
	case UNBURROW:
		break;
	}

	hitThisFrame = false;
}

void CrawlerQueen::UpdateEnemyPhysics()
{
	if (!dead)
	{
		surfaceMover->Move(slowMultiple, numPhysSteps);
		//position = mover->physBody.globalPosition;

		switch (action)
		{
		case BOOST:
		{
			if (travelIndex < numDecisions)
			{
				int startIndex = edgeIndexMap[startTravelPoint.edge];
				int destIndex = edgeIndexMap[decidePoints[travelIndex].edge];
				int groundIndex = edgeIndexMap[surfaceMover->ground];
				EdgeInfo &dest = decidePoints[travelIndex];
				int maxEdge = edgeIndexMap.size() - 1;

				if (!leftInitialEdge && surfaceMover->ground != startTravelPoint.edge)
				{
					leftInitialEdge = true;
				}

				if (!completedLoop && leftInitialEdge && groundIndex == startIndex)
				{
					completedLoop = true;
				}

				if (facingRight)
				{
					if (!partLoop
						&& (groundIndex < startIndex
							|| (leftInitialEdge && (groundIndex == destIndex || groundIndex == startIndex))))
					{
						partLoop = true;
					}

					if (!completedLoop && leftInitialEdge && groundIndex == startIndex)
					{
						completedLoop = true;
					}


					bool a = groundIndex > destIndex;
					bool b = (destIndex == maxEdge && groundIndex == 0 && completedLoop);
					bool c = (groundIndex == destIndex && surfaceMover->edgeQuantity >= dest.quantity);
					if (partLoop && ( a || b || c ) )
					{
						//cout << a << " " << b << " " << c << ", start: " << startIndex << ", dest: " << destIndex << endl;
						DecideNextAction();
						break;
					}
				}
				else
				{
					if (!partLoop
						&& (groundIndex > startIndex
							|| (leftInitialEdge && (groundIndex == destIndex || groundIndex == startIndex))))
					{
						partLoop = true;
					}

					bool a = groundIndex < destIndex;
					bool b = (destIndex == 0 && groundIndex == maxEdge && completedLoop );
					bool c = (groundIndex == destIndex && surfaceMover->edgeQuantity <= dest.quantity);
					if (partLoop && (a || b || c ) )
					{
						//cout << a << " " << b << " " << c << ", start: " << startIndex << ", dest: " << destIndex << endl;
						DecideNextAction();
						break;
					}
				}
			}
			break;
		}
		}
	}
}

void CrawlerQueen::FrameIncrement()
{
	if (invinc)
	{
		currInvincFramesOnHit++;
		if (currInvincFramesOnHit == totalInvincFramesOnHit)
		{
			invinc = false;
		}
	}
	else
	{
		if (hitThisFrame)
		{
			notHitFrames = 0;
		}
		else
		{
			notHitFrames++;
			if (notHitFrames == notHitCap)
			{
				invincHitCount = 0;
			}
		}
	}
}

void CrawlerQueen::EnemyDraw(sf::RenderTarget *target)
{
	if (action == INITIALWAIT)
		return;
	if (action == RUMBLE )
	{
		target->draw(decideVA, MAX_DECISIONS * 4*3, sf::Quads, ts_decideMarker->texture);
		return;
	}
		

	if (sess->GetPauseFrames() < 2 || receivedHit == NULL)
	{
		target->draw(sprite);
	}
	else
	{
		target->draw(sprite, &hurtShader);
	}

	target->draw(decideVA, MAX_DECISIONS * 4 * 3, sf::Quads, ts_decideMarker->texture);
}

void CrawlerQueen::IHitPlayer(int index)
{

}


void CrawlerQueen::HandleNoHealth()
{
}

void CrawlerQueen::DeactivateAllBombs()
{
	FloatingBomb *fb = (FloatingBomb*)bombPool->activeListStart;
	while (fb != NULL)
	{
		FloatingBomb *fbNext = (FloatingBomb*)fb->pmnext;
		fb->Reset();
		sess->RemoveEnemy(fb);
		fb = fbNext;
	}

	bombPool->DeactivateAll();
}

void CrawlerQueen::HitTerrainAerial(Edge *e, double q)
{
	surfaceMover->ground = e;
	surfaceMover->edgeQuantity = q;
	surfaceMover->UpdateGroundPos();

	startTravelPoint.edge = surfaceMover->ground;
	startTravelPoint.index = surfaceMover->edgeQuantity;
	startTravelPoint.index = edgeIndexMap[surfaceMover->ground];

	Boost();
}

void CrawlerQueen::UpdateSprite()
{
	if (action == HURT)
		return;
	if (action == RUMBLE || action == INITIALWAIT)
	{
		return;
	}
	//V2d gPoint = mover->ground->GetPoint(mover->edgeQuantity);

	//return;
	float extraVert = 64;

	double angle = 0;

	IntRect ir;

	Tileset *currTS = NULL;

	currTS = ts[action];
	sprite.setTexture(*currTS->texture);

	int currTile = 0;


	switch (action)
	{
	case INITIALIDLE:
		currTile = 8;
		break;
	case WAIT:
		currTile = 0;
		break;
	case DECIDE:
		currTile = 0;
		break;
	case SEQ_ANGRY0:
		currTile = 8;
		break;
	case BOOST:
		currTile = 0;
		break;
	case TURNAROUNDBOOST:
		currTile = frame / animFactor[action] + 1;
		break;
	case STOPBOOST:
		currTile = 10;
		break;
	case JUMP:
		currTile = 1;
		break;
	case BURROW:
		currTile = frame / animFactor[action];
		break;
	case POPOUT:
		currTile = frame / animFactor[action];
		break;
	case INITIALUNBURROW:
		currTile = frame / animFactor[action];
		break;
	case UNBURROW:
		currTile = frame / animFactor[action];
		break;
	case SEQ_FINISHINITIALUNBURROW:
		currTile = frame / animFactor[action] + 9;
		break;
	}

	ir = currTS->GetSubRect(currTile);

	bool extraFlipCW = facingRight && action == POPOUT && surfaceMover->ground->Normal().x < 0;
	bool extraFlipCCW = !facingRight && action == POPOUT && surfaceMover->ground->Normal().x > 0;
	bool flip = !facingRight;
	if (extraFlipCW || extraFlipCCW)
	{
		flip = !flip;
	}
	if (flip)
	{
		sprite.setTextureRect(sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height));
	}
	else
	{
		sprite.setTextureRect(ir);
	}
	

	if (surfaceMover->ground != NULL)
	{
		V2d gn = surfaceMover->ground->Normal();

		if (!surfaceMover->roll)
		{
			angle = atan2(gn.x, -gn.y);

			V2d pp = surfaceMover->ground->GetPosition(surfaceMover->edgeQuantity);//ground->GetPosition( edgeQuantity );
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);

			sprite.setRotation(angle / PI * 180);
			sprite.setPosition(pp.x, pp.y);
		}
		else
		{
			if (facingRight)
			{
				V2d vec = normalize(GetPosition() - surfaceMover->ground->v1);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
				V2d pp = surfaceMover->ground->GetPosition(surfaceMover->edgeQuantity);//ground->GetPosition( edgeQuantity );
				sprite.setPosition(pp.x, pp.y);
			}
			else
			{
				V2d vec = normalize(GetPosition() - surfaceMover->ground->v0);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
				V2d pp = surfaceMover->ground->GetPosition(surfaceMover->edgeQuantity);
				sprite.setPosition(pp.x, pp.y);
			}
		}
	}
	else
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);// -extraVert);
		sprite.setRotation(angle / PI * 180);
		sprite.setPosition(Vector2f(surfaceMover->physBody.globalPosition));
	}

	if (DecideShownAction())
	{
		for (int i = 0; i < numDecisions; ++i)
		{
			if (i == 0)
				decDebugDraw[i].setFillColor(Color::Red);
			else if (i == 1)
				decDebugDraw[i].setFillColor(Color::Blue);
			else
			{
				decDebugDraw[i].setFillColor(Color::Green);
			}

			decDebugDraw[i].setRadius(32);
			decDebugDraw[i].setOrigin(Vector2f(decDebugDraw[i].getLocalBounds().width / 2,
				decDebugDraw[i].getLocalBounds().height / 2));
			decDebugDraw[i].setPosition(
				Vector2f(decidePoints[i].edge->GetPosition(decidePoints[i].quantity))
				+ Vector2f(decidePoints[i].edge->Normal() * surfaceMover->physBody.rw));
		}
	}

	if (invinc)
	{
		if (currInvincFramesOnHit % 3 == 0)
		{
			if (sprite.getColor().a == 60)
			{
				sprite.setColor(Color(255, 255, 255, 255));
			}
			else
			{
				sprite.setColor(Color(255, 255, 255, 60));
			}
		}
	}
	else
	{
		sprite.setColor(Color(255, 255, 255, 255));
	}
	/*sprite.setColor(Color(255, 255, 255, 60));
	sprite.setColor(Color(255, 255, 255, 255));*/

	PoolMember *pm = decisionPool->activeListStart;
	while (pm != NULL)
	{
		DecisionMarker *dm = (DecisionMarker*)pm;
		dm->Update();
		pm = pm->pmnext;
	}
}

bool CrawlerQueen::DecideShownAction()
{
	if (action == UNBURROW || action == POPOUT || action == RUMBLE || action == BURROW
		|| action == JUMP || action == STOPBOOST || action == TURNAROUNDBOOST
		|| action == WAIT)
		return true;

	return false;
}

void CrawlerQueen::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	if (!dead)
		surfaceMover->physBody.DebugDraw(CollisionBox::Physics, target);
	
	for (int i = 0; i < numDecisions; ++i)
	{
		target->draw(decDebugDraw[i]);
	}
}

void CrawlerQueen::TransferEdge(Edge *e)
{
	leftInitialEdge = true;
}

bool CrawlerQueen::PlayerInFront()
{
	V2d dir;
	if (facingRight)
	{
		dir = normalize(surfaceMover->ground->v1 - surfaceMover->ground->v0);
	}
	else if (!facingRight)
	{
		dir = normalize(surfaceMover->ground->v0 - surfaceMover->ground->v1);
	}
	double alongDist = dot(sess->GetPlayer(0)->position - surfaceMover->physBody.globalPosition, dir);
	if (alongDist > -60)
		return true;
	else
		return false;
}

void CrawlerQueen::Accelerate(double amount)
{
	if (facingRight)
	{
		amount = abs(amount);
	}
	else
	{
		amount = -abs(amount);
	}

	double max = 35.0;
	double am = surfaceMover->groundSpeed + amount;
	if (am > max)
		am = max;
	if (am < -max)
		am = -max;
	cout << "speed: " << am << endl;
	surfaceMover->SetSpeed(am);
}

void CrawlerQueen::SetForwardSpeed(double speed)
{
	double aSpeed = abs(speed);
	if (facingRight)
	{
		surfaceMover->SetSpeed(aSpeed);
	}
	else
	{
		surfaceMover->SetSpeed(-aSpeed);
	}
}

void CrawlerQueen::SetDecisions()
{
	for (int i = 0; i < numDecisions; ++i)
	{
		int r = rand() % (maxDecision);
		//r = D_JUMP;
		decisions[i] = (Decision)r;//D_DIG;//(Decision)r;
	}
}
void CrawlerQueen::ConfirmKill()
{
	action = HURT;
	frame = 0;

	if (game != NULL)
	{
		game->SoftenGates(Gate::BOSS);
		game->SetActiveSequence(postFightScene);
	}
	
	surfaceMover->groundSpeed = 0;

	dead = true;

	DeactivateAllBombs();
}

void CrawlerQueen::SetDifficulty( int lev)
{
	//max health is 120 
	if (numHealth > 100)
	{
		numDecisions = 1;
		maxDecision = 1;
	}
	else if (numHealth > 80)
	{
		numDecisions = 2;
		maxDecision = 1;
	}
	else if (numHealth > 60)
	{
		numDecisions = 2;
		maxDecision = 2;
	}
	else if (numHealth > 40)
	{
		numDecisions = 2;
		maxDecision = 3;
	}
	else// if (numHealth > 20)
	{
		numDecisions = 3;
		maxDecision = 3;
	}
}

#include <math.h>
void CrawlerQueen::DecidePoints()
{
	//add more to this algorithm later to balance it
	ClearDecisionMarkers();
	decisionPool->DeactivateAll();
	for (int i = 0; i < numDecisions; ++i)
	{
		int r = rand() % numTotalEdges; 

		int quant = rand() % (int)edgeRef[r]->GetLength();

		decidePoints[i].edge = edgeRef[r];
		decidePoints[i].index = r;
		decidePoints[i].quantity = quant;

		Edge *e = decidePoints[i].edge;

		V2d pos = e->GetPosition(decidePoints[i].quantity) + e->Normal() * 32.0;

		PoolMember *pm = decisionPool->ActivatePoolMember();
		DecisionMarker *dm = (DecisionMarker*)pm;
		dm->Reset(Vector2f(pos), decisions[i]);
	}
}

double CrawlerQueen::GetDistanceCCW(int index)
{
	Edge *e = decidePoints[index].edge;
	double q = decidePoints[index].quantity;


	double sum = 0;
	Edge *curr = surfaceMover->ground;
	double currQ = surfaceMover->edgeQuantity;

	sum += currQ;
	curr = curr->edge0;

	while (curr != e)
	{
		sum += length(curr->v1 - curr->v0);
		curr = curr->edge0;
	}

	sum += length(curr->v1 - curr->v0) - q;

	return sum;
}

void CrawlerQueen::Jump()
{
	action = JUMP;
	frame = 0;
	surfaceMover->velocity = surfaceMover->ground->Normal() * 10.0;
	surfaceMover->ground = NULL;
}

void CrawlerQueen::Boost()
{
	leftInitialEdge = false;
	partLoop = false;
	completedLoop = false;
	action = BOOST;
	frame = 0;
	if (GetClockwise(travelIndex))
	{
		facingRight = true;
		surfaceMover->SetSpeed(baseSpeed + travelIndex * multSpeed);
		//cout << "travel: " << travelIndex << ", clockwise: " << startTravelPoint.index << " to " << decidePoints[travelIndex].index << endl;
	}
	else
	{
		facingRight = false;
		surfaceMover->SetSpeed(-baseSpeed - travelIndex * multSpeed);
		//cout << "travel: " << travelIndex << ", CCW: " << startTravelPoint.index << " to " << decidePoints[travelIndex].index << endl;
	}
}

void CrawlerQueen::DecideAction()
{
	if (travelIndex == numDecisions)
	{
		redecide = true;
		//action = DECIDE;
		//frame = 0;
		//decideIndex = 0;
		return;
	}

	switch (decisions[travelIndex])
	{
	case D_BOOST:
		{
		Boost();	
		break;
		}
	case D_JUMP:
	{
		Jump();
		break;
	}
	case D_DIG:
	{
		surfaceMover->SetSpeed(0);
		digInfo.edge = surfaceMover->ground;
		digInfo.quantity = surfaceMover->edgeQuantity;
		action = BURROW;
		frame = 0;
		digAttackCounter = 0;
		break;
	}
		
	}
}

double CrawlerQueen::GetDistanceClockwise(int index)
{
	Edge *e = decidePoints[index].edge;
	double q = decidePoints[index].quantity;


	double sum = 0;
	Edge *curr = surfaceMover->ground;
	double currQ = surfaceMover->edgeQuantity;

	sum += length(curr->v1 - curr->v0) - currQ;
	curr = curr->edge1;

	Edge *sEdge = curr;

	//for( int i = 0; i < numTotalEdges; ++i )
	int counter = 0;
	while (curr != e)
	{
		sum += length(curr->v1 - curr->v0);
		curr = curr->edge1;
		++counter;

		if (counter > numTotalEdges)
		{
			int f = 5;
		}
	}

	sum += q;


	return sum;
}

void CrawlerQueen::DecideNextAction()
{
	++travelIndex;
	startTravelPoint.edge = surfaceMover->ground;
	startTravelPoint.index = surfaceMover->edgeQuantity;
	startTravelPoint.index = edgeIndexMap[surfaceMover->ground];
	DecideAction();
}

bool CrawlerQueen::GetClockwise(int index)
{
	Edge *e = decidePoints[index].edge;
	double q = decidePoints[index].quantity;

	double sum = 0;
	Edge *curr = surfaceMover->ground;
	double currQ = surfaceMover->edgeQuantity;

	if (curr == e)
	{
		if (q > currQ)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		double distanceClockwise = GetDistanceClockwise(index);
		double ccw = GetDistanceCCW(index);
		if (distanceClockwise >= ccw)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

void CrawlerQueen::Init()
{
	
	if (sess->mapHeader->bossFightType == 1)
	{
		//PoiInfo *pi = owner->poiMap["crawlercam"];
		//assert(pi != NULL);
		//owner->cam.manual = true;
		//owner->cam.Ease(Vector2f(pi->pos), 1.75, 60, CubicBezier());
	}
}

void CrawlerQueen::Setup()
{
	Enemy::Setup();

	postFightScene = new CrawlerPostFightScene;
	postFightScene->Init();

	ResetEnemy();
}

FloatingBomb::FloatingBomb( ActorParams *ap, ObjectPool *p_myPool, int index)
	:Enemy(EnemyType::EN_FLOATINGBOMB, ap ),//, false, 1, false),
	PoolMember(index), myPool(p_myPool)
{
	//preload
	sess->GetTileset("Enemies/bombexplode_512x512.png", 512, 512);

	mover = new SurfaceMover(NULL, 0, 32);
	mover->surfaceHandler = this;
	mover->SetSpeed(0);

	ts = sess->GetTileset("Enemies/bomb_128x160.png", 128, 160);
	sprite.setTexture(*ts->texture);

	action = FLOATING;

	actionLength[FLOATING] = 9;
	actionLength[EXPLODING] = 4;

	animFactor[FLOATING] = 3;
	animFactor[EXPLODING] = 3;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;


	ResetEnemy();
}

void FloatingBomb::Init(V2d pos, V2d vel)
{
	currPosInfo.position = pos;
	mover->velocity = vel;
	action = FLOATING;
	frame = 0;
	mover->physBody.globalPosition = GetPosition();
}

FloatingBomb::~FloatingBomb()
{
	delete mover;
}

void FloatingBomb::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case FLOATING:
			frame = 0;
			break;
		case EXPLODING:
			numHealth = 0;
			dead = true;
			sess->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
				sess->GetTileset("Enemies/bombexplode_512x512.png", 512, 512),
				GetPosition(), false, 0, 10, 3, true);
			//cout << "deactivating " << this << " . currently : " << myPool->numActiveMembers << endl;
			//myPool->DeactivatePoolMember(this);
			break;
		}
	}

	switch (action)
	{
	case FLOATING:
		break;
	case EXPLODING:
		break;
	}

	switch (action)
	{
	case FLOATING:
		break;
	case EXPLODING:
		break;
	}
}

void FloatingBomb::HandleNoHealth()
{

}

void FloatingBomb::FrameIncrement()
{

}

void FloatingBomb::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void FloatingBomb::IHitPlayer(int index)
{
	if (action == FLOATING)
	{
		action = EXPLODING;
		frame = 0;
	}
}

void FloatingBomb::UpdateSprite()
{
	switch (action)
	{
	case FLOATING:
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[FLOATING]));
		break;
	case EXPLODING:
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[EXPLODING]));
		break;
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void FloatingBomb::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	if (!dead)
		mover->physBody.DebugDraw(CollisionBox::Physics, target);
}

void FloatingBomb::HitTerrainAerial(Edge * e, double q )
{
	if (action == FLOATING)
	{
		mover->velocity = V2d(0, 0);
		action = EXPLODING;
		frame = 0;
	}
}

void FloatingBomb::ResetEnemy()
{
	mover->ground = NULL;
	mover->edgeQuantity = 0;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);
	action = FLOATING;
	frame = 0;
	SetHurtboxes(&hurtBody, 0);
	SetHitboxes(&hitBody, 0);
}

void FloatingBomb::UpdateEnemyPhysics()
{
	if (!dead)
	{
		mover->Move(slowMultiple, numPhysSteps);
		//position = mover->physBody.globalPosition;
	}
}

void FloatingBomb::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOATING )
	{
		action = EXPLODING;
		frame = 0;
	}
		
}

DecisionMarker::DecisionMarker(CrawlerQueen *p_parent,
	ObjectPool *mPool,
	int p_index)
	:PoolMember(index), parent( p_parent ), myPool( mPool ), index( p_index )
{
}

void DecisionMarker::Update()
{
	switch (action)
	{
	case START:
		if (frame == 10)
		{
			action = LOOP;
		}
		break;
	case LOOP:
		//if (frame == 10)
		//	frame = 0;
		break;
	case DISSIPATE:
		if (frame == 10)
		{
			myPool->DeactivatePoolMember(this);
			Clear();
			return;
		}
		break;
	}

	int f = frame % 60;
	float prop = f / 60.f;
	float a = 0;
	
	if (action == LOOP)
	{
		a = 2.0 * PI * prop;
		//if( index == 0 )
		//cout << "a: " << a << endl;
	}

	SetRectRotation(parent->decideVA + index * 4, a, parent->ts_decideMarker->tileWidth,
		parent->ts_decideMarker->tileHeight, pos);
	SetRectRotation(parent->decideVA + (index+1) * 4, -a, parent->ts_decideMarker->tileWidth,
		parent->ts_decideMarker->tileHeight, pos);
	SetRectRotation(parent->decideVA + (index+2) * 4, 0, parent->ts_decideMarker->tileWidth,
		parent->ts_decideMarker->tileHeight, pos);

	int tRow;
	switch (dec)
	{
	case CrawlerQueen::D_BOOST:
		tRow = 1;
		break;
	case CrawlerQueen::D_JUMP:
		tRow = 2;
		break;
	case CrawlerQueen::D_DIG:
		tRow = 0;
		break;
	default:
		assert(0);
		break;
	}

	SetRectSubRect(parent->decideVA + index * 4, parent->ts_decideMarker->GetSubRect(tRow * 3));
	SetRectSubRect(parent->decideVA + (index+1) * 4, parent->ts_decideMarker->GetSubRect(tRow * 3 + 1));
	SetRectSubRect(parent->decideVA + (index+2) * 4, parent->ts_decideMarker->GetSubRect(tRow * 3 + 2));
	//SetRectColor( parent->decideVA + index * 4, Color(Color::Red));

	++frame;
}

void DecisionMarker::Clear()
{
	for (int i = 0; i < 3; ++i)
	{
		parent->decideVA[(index+i) * 4 + 0].position = Vector2f(0, 0);
		parent->decideVA[(index + i) * 4 + 1].position = Vector2f(0, 0);
		parent->decideVA[(index + i) * 4 + 2].position = Vector2f(0, 0);
		parent->decideVA[(index + i) * 4 + 3].position = Vector2f(0, 0);
	}
	
}

void DecisionMarker::Reset(sf::Vector2f &p_pos, CrawlerQueen::Decision p_dec )
{
	pos = p_pos;
	action = START;
	frame = 0;
	dec = p_dec;
}