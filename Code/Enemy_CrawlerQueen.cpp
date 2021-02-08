#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CrawlerQueen.h"
#include "Actor.h"
#include "SequenceW1.h"
#include "SequenceW4.h"
#include "Enemy_Crawler.h"
#include "Enemy_QueenFloatingBomb.h"
#include <algorithm>

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

//boost, dig, summon

RandomPicker::RandomPicker()
{
	numMaxOptions = 0;
	options = NULL;
}

RandomPicker::~RandomPicker()
{
	if (options != NULL)
		delete[] options;
}

void RandomPicker::SetMaxOptions(int m)
{
	if (numMaxOptions != m)
	{
		numMaxOptions = m;
		if (options != NULL)
		{
			delete[] options;
		}
		else
		{
			if (numMaxOptions > 0)
			{
				options = new int[numMaxOptions];
			}
			else
			{
				options = NULL;
			}
		}
	}
}

void RandomPicker::Reset()
{
	numActiveOptions = 0;
}

void RandomPicker::AddActiveOption(int a, int reps)
{
	assert(reps > 0);
	assert(numActiveOptions + reps <= numMaxOptions);
	for (int i = 0; i < reps; ++i)
	{
		options[numActiveOptions] = a;
		++numActiveOptions;
	}
}

void RandomPicker::ShuffleActiveOptions()
{
	int val;
	int index;
	for (int i = numActiveOptions-1; i >= 1; --i)
	{
		index = rand() % (i+1);
		val = options[index];
		options[index] = options[i];
		options[i] = val;
	}
	currActiveIndex = 0;
}

int RandomPicker::GetNextOption()
{
	if (currActiveIndex >= numActiveOptions)
	{
		return -1;
	}
	else
	{
		int val = options[currActiveIndex];
		++currActiveIndex;
		return val;
	}
}

CrawlerQueen::CrawlerQueen(ActorParams *ap)
	:Enemy(EnemyType::EN_CRAWLERQUEEN, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;

	actionLength[DIG] = 30;
	actionLength[SLASH] = 26;

	actionLength[SUMMON] = 60;
	animFactor[SLASH] =4;

	crawlerParams = new BasicGroundEnemyParams(sess->types["crawler"], 1);
	for (int i = 0; i < NUM_CRAWLERS; ++i)
	{
		crawlers[i] = (Crawler*)crawlerParams->GenerateEnemy();
		crawlers[i]->queen = this;
		crawlers[i]->SetSummon(true);
	}

	for (int i = 0; i < NUM_BOMBS; ++i)
	{
		bombs[i] = new QueenFloatingBomb;
	}


	//testCrawler = new Crawler( )

	reachPointOnFrame[COMBOMOVE] = 0;

	ts_move = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_256x256.png");
	ts_slash = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_slash_320x320.png");

	postFightScene = NULL;
	postFightScene2 = NULL;

	level = ap->GetLevel();

	enemyMover.handler = this;

	nodeAStr = "A";

	CreateSurfaceMover(startPosInfo, 30, this);

	SetOffGroundHeight(128);

	hitboxInfo = new HitboxInfo;
	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 0;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 50;
	hitboxInfo->kbDir = normalize(V2d(1, -2));
	hitboxInfo->gravMultiplier = .5;
	hitboxInfo->invincibleFrames = 15;*/

	LoadParams();

	BasicCircleHurtBodySetup(70);
	BasicCircleHitBodySetup(70);


	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	ResetEnemy();
}

CrawlerQueen::~CrawlerQueen()
{
	if (postFightScene != NULL)
	{
		delete postFightScene;
	}

	if (postFightScene2 != NULL)
	{
		delete postFightScene2;
	}

	delete crawlerParams;
	for (int i = 0; i < NUM_CRAWLERS; ++i)
	{
		delete crawlers[i];
	}

	for (int i = 0; i < NUM_BOMBS; ++i)
	{
		delete bombs[i];
	}
}

void CrawlerQueen::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void CrawlerQueen::HandleCrawlerDeath()
{
	numActiveCrawlers--;

	assert(numActiveCrawlers >= 0);
}

void CrawlerQueen::UpdateHitboxes()
{
	BasicUpdateHitboxes();

	if (hitBody.hitboxInfo != NULL)
	{
		if (facingRight)
		{
			hitBody.hitboxInfo->kbDir.x = hitboxInfos[action].kbDir.x;
		}
		else
		{
			hitBody.hitboxInfo->kbDir.x = -hitboxInfos[action].kbDir.x;
		}
	}
}

void CrawlerQueen::ResetEnemy()
{
	currPosInfo = startPosInfo;

	wasAerial = false;

	playerComboer.Reset();
	snakePool.Reset();
	enemyMover.Reset();

	fireCounter = 0;
	facingRight = true;

	currMaxActiveCrawlers = 4;//NUM_CRAWLERS;
	currMaxActiveBombs = NUM_BOMBS;
	numCrawlersToSummonAtOnce = 3;

	numActiveCrawlers = 0;

	currDashSpeed = 20;
	currDashAccel = 1.0;

	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);

	if (sess->preLevelScene == NULL) //fight testing
	{
		CameraShot *cs = sess->cameraShotMap["fightcam"];
		if (cs != NULL)
		{
			sess->cam.Set(Vector2f(cs->centerPos), cs->zoom, 1);
		}
	}
	

	StartFight();

	hitPlayer = false;
	comboMoveFrames = 0;

	UpdateSprite();

	for (int i = 0; i < NUM_CRAWLERS; ++i)
	{
		crawlers[i]->Reset();
	}

	for (int i = 0; i < NUM_BOMBS; ++i)
	{
		bombs[i]->Reset();
	}
}

void CrawlerQueen::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

void CrawlerQueen::HandleFinishTargetedMovement()
{
	surfaceMover->Set(enemyMover.currPosInfo);
}

//void CrawlerQueen::SetCommand(int index, BirdCommand &bc)
//{
//	actionQueue[index] = bc;
//}

void CrawlerQueen::DebugDraw(sf::RenderTarget *target)
{
	//playerComboer.DebugDraw(target);
	//enemyMover.DebugDraw(target);
	//Enemy::DebugDraw(target);

	surfaceMover->physBody.DebugDraw( CollisionBox::Physics, target);
}

void CrawlerQueen::DirectKill()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[0]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}
	receivedHit = NULL;
}

void CrawlerQueen::FrameIncrement()
{
	++fireCounter;

	if (comboMoveFrames > 0)
	{
		--comboMoveFrames;
	}

	if (moveFrames > 0)
	{
		--moveFrames;
	}

	if (waitFrames > 0)
	{
		--waitFrames;
	}

	enemyMover.FrameIncrement();
	currPosInfo = enemyMover.currPosInfo;
}

void CrawlerQueen::Decide(int numFrames)
{
	action = DECIDE;
	frame = 0;
	actionLength[DECIDE] = numFrames;
}

void CrawlerQueen::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case COMBOMOVE:
			frame = 0;
			break;
		case SUMMON:
			Decide(0);
			break;
		case SLASH:
			Decide(0);
			break;
		case MOVE:
			Decide(0);
			break;
		}
	}

	if (action == MOVE && moveFrames == 0)
	{
		Decide(0);
	}

	switch (action)
	{
	case DECIDE:
	{
		if (frame == actionLength[DECIDE] * animFactor[DECIDE])
		{
			int r = rand() % 3;

			auto &nodeVec = sess->GetBossNodeVector(BossFightType::FT_CRAWLER, nodeAStr);
			int vecSize = nodeVec.size();
			int rNode = rand() % vecSize;

			targetNode = nodeVec[rNode];

			V2d nodePos = targetNode->pos;

			V2d pPos = sess->GetPlayerPos(0);
			V2d pDir = normalize(pPos - GetPosition());

			if (r == 0)
			{
				int gr = rand() % 2;
				action = MOVE;
				moveFrames = 120;

				if (gr == 0)
				{
					if (surfaceMover->groundSpeed == 0)
					{
						surfaceMover->SetSpeed(currDashSpeed);
					}
					
					facingRight = true;
					//enemyMover.SetModeGrind(grindSpeed, 120);
				}
				else if (gr == 1)
				{
					if (surfaceMover->groundSpeed == 0)
					{
						surfaceMover->SetSpeed(-currDashSpeed);
					}
					
					facingRight = false;
				}

				//snakePool.Throw(GetPosition(), pDir);
			}
			else if (r == 1)
			{
				action = SLASH;
				frame = 0;
				surfaceMover->SetSpeed(0);
			}
			else if (r == 2)
			{
				action = SUMMON;
				frame = 0;
				surfaceMover->SetSpeed(0);
			}
		}
		break;
	}
		
	case MOVE:
		break;
	case SUMMON:
		break;
	case DIG:
		break;
	case SLASH:
		break;
	}

	switch (action)
	{
	case DECIDE:
		break;
	case MOVE:
		if (facingRight && surfaceMover->groundSpeed < currDashSpeed)
		{
			surfaceMover->groundSpeed += currDashAccel;
			if (surfaceMover->groundSpeed > currDashSpeed)
			{
				surfaceMover->groundSpeed = currDashSpeed;
			}
		}
		else if (!facingRight && surfaceMover->groundSpeed > -currDashSpeed)
		{
			surfaceMover->groundSpeed += -currDashAccel;
			if (surfaceMover->groundSpeed < -currDashSpeed)
			{
				surfaceMover->groundSpeed = -currDashSpeed;
			}
		}
		break;
	case SUMMON:
		if (frame == 20 && slowCounter == 1)
		{
			int currSummoned = 0;
			for (int i = 0; i < NUM_CRAWLERS; ++i)
			{
				if (!crawlers[i]->spawned || !crawlers[i]->dead )
				{
					crawlers[i]->spawned = false;
					crawlers[i]->startPosInfo.SetGround(targetNode->poly,
						targetNode->edgeIndex, targetNode->edgeQuantity);

					sess->AddEnemy(crawlers[i]);
					++numActiveCrawlers;
					++currSummoned;

					if (!CanSummonCrawler())
					{
						break;
					}
					else if (currSummoned == numCrawlersToSummonAtOnce)
					{
						break;
					}
				}
			}
		}
		break;
	case DIG:
		break;
	case SLASH:
		if (frame == 11 * animFactor[SLASH] && slowCounter == 1)
		{
			for (int i = 0; i < NUM_BOMBS; ++i)
			{
				if (!bombs[i]->spawned)
				{
					V2d gn = surfaceMover->ground->Normal();
					sess->AddEnemy(bombs[i]);
					bombs[i]->Init(GetPosition() + gn * 80.0, gn * 5.0);
					break;
				}
			}
		}
		break;
	}

	//enemyMover.currPosInfo = currPosInfo;

	

	//hitPlayer = false;
}

bool CrawlerQueen::CanSummonCrawler()
{
	return numActiveCrawlers < currMaxActiveCrawlers;
}

void CrawlerQueen::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 1)
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			if (hasMonitor && !suppressMonitor)
			{
				//sess->CollectKey();
			}

			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			ConfirmKill();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();
		}

		if (numHealth == 1)
		{
			if (level == 1)
			{
				postFightScene->Reset();
				sess->SetActiveSequence(postFightScene);
			}
			else if (level == 2)
			{
				postFightScene2->Reset();
				sess->SetActiveSequence(postFightScene2);
			}
		}

		receivedHit = NULL;
	}
}

void CrawlerQueen::Setup()
{
	Enemy::Setup();

	if (level == 1)
	{
		if (postFightScene2 != NULL)
		{
			delete postFightScene2;
			postFightScene2 = NULL;
		}

		if (postFightScene == NULL)
		{
			postFightScene = new CrawlerPostFightScene;
			postFightScene->queen = this;
			postFightScene->Init();
		}
	}
	else if (level == 2)
	{

		if (postFightScene != NULL)
		{
			delete postFightScene;
			postFightScene = NULL;
		}

		if (postFightScene2 == NULL)
		{
			postFightScene2 = new CrawlerPostFight2Scene;
			postFightScene2->queen = this;
			postFightScene2->Init();
		}
		
	}


}

void CrawlerQueen::StartAngryYelling()
{

}

void CrawlerQueen::StartInitialUnburrow()
{

}

void CrawlerQueen::Wait()
{
	action = SEQ_WAIT;
	frame = 0;
	snakePool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void CrawlerQueen::StartFight()
{
	Decide(30);
	//action = WAIT;
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	frame = 0;
	SetHitboxes(NULL);
	waitFrames = 10;
}

void CrawlerQueen::IHitPlayer(int index)
{
	//hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames;
}

void CrawlerQueen::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		//currPosInfo = enemyMover.currPosInfo;
	}
	else
	{
		Enemy::UpdateEnemyPhysics();
	}
}

void CrawlerQueen::UpdateSprite()
{
	bool isAerial = enemyMover.currPosInfo.IsAerial();
	if (!isAerial)
	{
		

	}

	int extraHeight = -35;

	switch (action)
	{
	case MOVE:
	{
		sprite.setTexture(*ts_move->texture);
		
		

		ts_move->SetSubRect(sprite, 0, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight );
		break;
	}
	case DECIDE:
	{
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 0, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		break;
	}
	case SLASH:
	{
		sprite.setTexture(*ts_slash->texture);
		ts_slash->SetSubRect(sprite, frame / animFactor[SLASH], !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - 80);
		sprite.setRotation(surfaceMover->GetAngleDegrees());

		V2d gn = surfaceMover->ground->Normal();

		if (gn.y <= 0)
		{
			if (gn.x < 0)
			{
				facingRight = false;
			}
			else if (gn.x > 0)
			{
				facingRight = true;
			}
			else
			{
				//fine either way
			}
		}
		else if (gn.y > 0)
		{
			if (gn.x < 0)
			{
				facingRight = false;
			}
			else if (gn.x > 0)
			{
				facingRight = true;
			}
			else
			{
				//fine either way
			}
		}
		/*else
		{
			if (gn.x < 0)
			{
				facingRight = false;
			}
			else if (gn.x > 0)
			{
				facingRight = true;
			}
		}*/

		break;
	}
	case SUMMON:
	{
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 0, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		
		break;
	}
		
		
	}
	
	

	sprite.setPosition(GetPositionF());

	
	

	

	{
		//float angleD = enemyMover.currPosInfo.GetGroundAngleDegrees();
		//if ( !isAerial )
		//{
		//	double distForward = 70;
		//	if (enemyMover.grindSpeed > 0)
		//	{
		//		angleD = enemyMover.CheckGround(distForward).GetGroundAngleDegrees();
		//	}
		//	else if (enemyMover.grindSpeed < 0)
		//	{
		//		angleD = enemyMover.CheckGround(-distForward).GetGroundAngleDegrees();
		//	}
		//	
		//}

		//if ( isAerial )
		//{
		//	sprite.setRotation(0);
		//	wasAerial = true;
		//}
		//else if (wasAerial)
		//{
		//	sprite.setRotation(enemyMover.currPosInfo.GetGroundAngleDegrees());
		//}
		//else
		//{
		//	Edge *e = currPosInfo.GetEdge();
		//	Edge *prevEdge;
		//	Edge *nextEdge;
		//	V2d currAlong = e->Along();
		//	V2d nextAlong;// = currPosInfo.GetEdge()->GetNextEdge()->Along();
		//	V2d prevAlong;
		//	if (enemyMover.grindSpeed > 0)
		//	{
		//		prevEdge = e->GetPrevEdge();
		//		prevAlong = prevEdge->Along();

		//		nextEdge = e->GetNextEdge();
		//		nextAlong = nextEdge->Along();
		//		
		//		double distToNext = e->GetLength() - currPosInfo.GetQuant();
		//		double nextFactor = 0.0;
		//		double test = 64.0;

		//		V2d realAlong;

		//		if (distToNext <= test)
		//		{
		//			nextFactor = 1.0 - distToNext / test;
		//			//realAlong = currAlong * (1.0 - factor) + nextAlong * (1.0 - factor);
		//		}

		//		double distToPrev = currPosInfo.GetQuant();
		//		double prevFactor = 0.0;
		//		if (distToPrev < test)
		//		{
		//			prevFactor = distToPrev / test;
		//		}

		//		if (nextFactor > 0)
		//		{
		//			realAlong = currAlong * (1.0 - nextFactor) + nextAlong * nextFactor;
		//		}
		//		else if (prevFactor > 0)
		//		{
		//			realAlong = currAlong * (1.0 - prevFactor) + prevAlong * prevFactor;
		//		}
		//		else
		//		{
		//			realAlong = currAlong;
		//		}
		//		
		//		
		//		/*if (nextFactor > 0)
		//		{
		//			realAlong = currAlong * (1.0 - nextFactor) + nextAlong * nextFactor;
		//		}
		//		

		//		V2d realAlong = currAlong * factor + nextAlong * ( 1.0 - factor);*/
		//		realAlong = normalize(realAlong);
		//		double realAng = GetVectorAngleCW(realAlong);
		//		double realAngD = realAng / PI * 180.0;

		//		sprite.setRotation(realAngD);

		//	}
		//	/*float rotateVel = 2;
		//	float ang = sprite.getRotation();
		//	if (angleD > ang)
		//	{
		//		ang += rotateVel;
		//		if (ang > angleD)
		//			ang = angleD;
		//	}
		//	else if (angleD < ang)
		//	{
		//		ang += -rotateVel;
		//		if (ang < angleD)
		//			ang = angleD;
		//	}
		//	sprite.setRotation(ang);*/
		//}

		////sprite.setRotation(0);
		///*if (enemyMover.currPosInfo.IsAerial())
		//{
		//	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2);
		//}
		//else
		//{
		//	
		//}*/
	}
	
}

void CrawlerQueen::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	snakePool.Draw(target);
}

void CrawlerQueen::HandleHitAndSurvive()
{
	fireCounter = 0;
}

int CrawlerQueen::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void CrawlerQueen::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
}

void CrawlerQueen::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
}

