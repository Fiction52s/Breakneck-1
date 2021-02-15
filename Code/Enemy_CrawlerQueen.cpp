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



CrawlerQueen::CrawlerQueen(ActorParams *ap)
	:Enemy(EnemyType::EN_CRAWLERQUEEN, ap),
	crawlerSummonGroup(this, 
		new BasicGroundEnemyParams(sess->types["crawler"], 1),
		5, 5, 1)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	
	

	actionLength[DIG_IN] = 21;
	actionLength[DIG_OUT] = 12;
	actionLength[SLASH] = 26;
	actionLength[UNDERGROUND] = 60;

	actionLength[SUMMON] = 60;

	animFactor[COMBOMOVE] = 1;
	animFactor[SLASH] =4;
	animFactor[DIG_IN] = 4;
	animFactor[DIG_OUT] = 4;

	nodeVecA = NULL;

	lungeSpeed = 20;

	for (int i = 0; i < NUM_BOMBS; ++i)
	{
		bombs[i] = new QueenFloatingBomb;
	}

	decidePicker.AddActiveOption(MOVE, 2);
	decidePicker.AddActiveOption(SUMMON, 2);
	decidePicker.AddActiveOption(DIG_IN, 2);

	nodePicker.ReserveNumOptions(8);
	for (int i = 0; i < 8; ++i)
	{
		nodePicker.AddActiveOption(i);
	}

	undergroundNodePicker.ReserveNumOptions(8);

	digDecidePicker.AddActiveOption(DIG_OUT, 2);
	digDecidePicker.AddActiveOption(LUNGE, 2);
	digDecidePicker.AddActiveOption(SLASH, 2);
	

	//testCrawler = new Crawler( )

	reachPointOnFrame[COMBOMOVE] = 0;

	ts_move = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_dash_320x320.png");
	ts_slash = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_slash_320x320.png");
	ts_dig_in = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_dig_in_320x320.png");
	ts_dig_out = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_dig_out_320x320.png");
	ts_jump = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_jump_320x320.png");

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

	

	invincibleFrames = 0;
	
	playerComboer.Reset();
	enemyMover.Reset();
	crawlerSummonGroup.Reset();

	fireCounter = 0;
	facingRight = true;

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

	if (invincibleFrames > 0)
	{
		--invincibleFrames;
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

void CrawlerQueen::GoUnderground(int numFrames)
{
	action = UNDERGROUND;
	frame = 0;
	actionLength[UNDERGROUND] = numFrames;
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
			GoUnderground(15);
			break;
		case MOVE:
			Decide(0);
			break;
		case DIG_IN:
			GoUnderground(60);
			break;
		case DIG_OUT:
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
			int r = decidePicker.AlwaysGetNextOption();//rand() % 3;

			int vecSize = nodeVecA->size();
			int rNode = rand() % vecSize;

			targetNode = nodeVecA->at(rNode);

			V2d nodePos = targetNode->pos;

			V2d pPos = sess->GetPlayerPos(0);
			V2d pDir = normalize(pPos - GetPosition());

			if (r == MOVE)
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
			else if (r == SUMMON)
			{
				action = SUMMON;
				frame = 0;
				surfaceMover->SetSpeed(0);
			}
			else if (r == SLASH)
			{
				action = SLASH;
				frame = 0;
				surfaceMover->SetSpeed(0);
			}
			else if (r == DIG_IN)
			{
				action = DIG_IN;
				frame = 0;
				surfaceMover->groundSpeed = 0;
			}
		}
		break;
	}
	case UNDERGROUND:
		if (frame == actionLength[UNDERGROUND] * animFactor[UNDERGROUND])
		{
			int nodeChoice = nodePicker.AlwaysGetNextOption();
			PositionInfo nodePosInfo;
			PoiInfo *node = nodeVecA->at(nodeChoice);
			nodePosInfo.SetGround(node->poly, node->edgeIndex, node->edgeQuantity);
			surfaceMover->Set(nodePosInfo);

			int digChoice = digDecidePicker.AlwaysGetNextOption();
			action = digChoice;
			frame = 0;
		}
		break;
	case MOVE:
		break;
	case SUMMON:
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
			crawlerSummonGroup.Summon();
		}
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
					bombs[i]->Init(GetPosition() + gn * 80.0, gn * 2.0);
					break;
				}
			}
		}
		break;
	case LUNGE:
	{
		if (frame == 0 && slowCounter == 1)
		{
			V2d gn = surfaceMover->ground->Normal();
			V2d lungeVel = gn * lungeSpeed;
			surfaceMover->Jump(lungeVel);
			if (lungeVel.x > 0)
			{
				facingRight = true;
			}
			else if (lungeVel.x < 0)
			{
				facingRight = false;
			}


		}
	}
	}

	//enemyMover.currPosInfo = currPosInfo;

	

	//hitPlayer = false;
}

bool CrawlerQueen::CanBeHitByPlayer()
{
	return invincibleFrames == 0;
}

void CrawlerQueen::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 1)
	{
		numHealth -= 1;

		if (numHealth <= 0) //useful later for editor
		{
			if (hasMonitor && !suppressMonitor)
			{
				//sess->CollectKey();
			}

			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			ConfirmKill();
		}
		else if (numHealth == 1)
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
		else
		{
			if (numHealth % 3 == 0)
			{
				invincibleFrames = 60;
			}


			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();
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

	if (nodeVecA == NULL)
	{
		nodeVecA = sess->GetBossNodeVector(BossFightType::FT_CRAWLER, nodeAStr);
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
	SetHitboxes(NULL);
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

	int extraHeight = -80;

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
	case DIG_IN:
	{
		sprite.setTexture(*ts_dig_in->texture);
		ts_dig_in->SetSubRect(sprite, frame / animFactor[DIG_IN], !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - 80);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		break;
	}
	case DIG_OUT:
	{
		sprite.setTexture(*ts_dig_out->texture);
		ts_dig_out->SetSubRect(sprite, frame / animFactor[DIG_OUT], !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		break;
	}
	case LUNGE:
	{
		V2d vel = surfaceMover->velocity;
		double ang = RadiansToDegrees(GetVectorAngleCCW(normalize(vel)));
		sprite.setTexture(*ts_jump->texture);
		ts_jump->SetSubRect(sprite, 1, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(ang);
		break;
	}
		
		
	}
	
	

	sprite.setPosition(GetPositionF());

	
	if (invincibleFrames > 0)
	{
		sprite.setColor(Color::Red);
	}
	else
	{
		sprite.setColor(Color::White);
	}

	

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
	if (action != UNDERGROUND)
	{
		DrawSprite(target, sprite);
	}
}

void CrawlerQueen::HandleHitAndSurvive()
{
	fireCounter = 0;
}

bool CrawlerQueen::IsDecisionValid(int d)
{
	/*if (d == SUMMON && !crawlerSummonGroup.CanSummon())
	{
		return false;
	}*/

	return true;
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


void CrawlerQueen::HitTerrainAerial(Edge *e, double quant)
{
	Decide(0);
}

void CrawlerQueen::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &crawlerSummonGroup)
	{
		PoiInfo *summonNode;

		summonNode = nodeVecA->at(nodePicker.AlwaysGetNextOption());
		e->startPosInfo.SetGround(summonNode->poly,
			summonNode->edgeIndex, summonNode->edgeQuantity);
	}
}