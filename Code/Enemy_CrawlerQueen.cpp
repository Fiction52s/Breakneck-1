#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CrawlerQueen.h"
#include "Actor.h"
#include "SequenceW1.h"
#include "SequenceW4.h"

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


CrawlerQueen::CrawlerQueen(ActorParams *ap)
	:Enemy(EnemyType::EN_CRAWLERQUEEN, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;
	reachPointOnFrame[COMBOMOVE] = 0;

	ts_move = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_256x256.png");

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

	playerComboer.Reset();
	snakePool.Reset();
	enemyMover.Reset();

	fireCounter = 0;
	facingRight = true;

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

void CrawlerQueen::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case COMBOMOVE:
			frame = 0;
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	if (action == MOVE && moveFrames == 0)//enemyMover.IsIdle())
	{
		action = WAIT;
		waitFrames = 30;//10
					   //currPosInfo.SetGround(
					   //	targetNode->poly, targetNode->edgeIndex, targetNode->edgeQuantity);
					   //enemyMover.currPosInfo = currPosInfo;
	}
	else if (action == WAIT && waitFrames == 0)
	{
		action = MOVE;
		moveFrames = 60;

		int r = rand() % 3;

		auto &nodeVec = sess->GetBossNodeVector(BossFightType::FT_CRAWLER, nodeAStr);
		int vecSize = nodeVec.size();
		int rNode = rand() % vecSize;

		targetNode = nodeVec[rNode];

		V2d nodePos = targetNode->pos;

		V2d pPos = sess->GetPlayerPos(0);
		V2d pDir = normalize(pPos - GetPosition());

		r = 0;
		if (r == 0)
		{
			int gr = rand() % 2;

			moveFrames = 120;
			double moveSpeed = 20;
			if (gr == 0)
			{
				surfaceMover->SetSpeed(moveSpeed);
				//enemyMover.SetModeGrind(grindSpeed, 120);
			}
			else if (gr == 1)
			{
				surfaceMover->SetSpeed(-moveSpeed);
			}

			//snakePool.Throw(GetPosition(), pDir);
		}
		else if (r == 1)
		{
			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();

			enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 1.5), 200);
			//enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			enemyMover.SetDestNode(nodeVec[rNode]);

			//snakePool.Throw(GetPosition(), pDir);
		}
		else if (r == 2)
		{
			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();
			enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 1.5), 200);
			//enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			enemyMover.SetDestNode(nodeVec[rNode]);
			//snakePool.Throw(GetPosition(), pDir);

		}
		else if (r == 3)
		{

		}


		
	}
	else if (action == COMBOMOVE)
	{
		if (comboMoveFrames == 0)
		{
			//action = actionQueue[actionQueueIndex].action + 1;
			//facingRight = actionQueue[actionQueueIndex].facingRight;
			SetHitboxInfo(action);
			//only have this on if i dont turn on hitboxes at the end of the movement.
			DefaultHitboxesOn();

		}
	}

	

	hitPlayer = false;
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
	action = WAIT;
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
		if (enemyMover.grindSpeed > 0)
		{
			facingRight = true;
		}
		else if (enemyMover.grindSpeed < 0)
		{
			facingRight = false;
		}

	}

	sprite.setTexture(*ts_move->texture);
	ts_move->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());

	int extraHeight = -35;
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);

	wasAerial = false;

	float angleD = enemyMover.currPosInfo.GetGroundAngleDegrees();
	if ( !isAerial )
	{
		double distForward = 70;
		if (enemyMover.grindSpeed > 0)
		{
			angleD = enemyMover.CheckGround(distForward).GetGroundAngleDegrees();
		}
		else if (enemyMover.grindSpeed < 0)
		{
			angleD = enemyMover.CheckGround(-distForward).GetGroundAngleDegrees();
		}
		
	}

	if ( isAerial )
	{
		sprite.setRotation(0);
		wasAerial = true;
	}
	else if (wasAerial)
	{
		sprite.setRotation(enemyMover.currPosInfo.GetGroundAngleDegrees());
	}
	else
	{
		Edge *e = currPosInfo.GetEdge();
		Edge *prevEdge;
		Edge *nextEdge;
		V2d currAlong = e->Along();
		V2d nextAlong;// = currPosInfo.GetEdge()->GetNextEdge()->Along();
		V2d prevAlong;
		if (enemyMover.grindSpeed > 0)
		{
			prevEdge = e->GetPrevEdge();
			prevAlong = prevEdge->Along();

			nextEdge = e->GetNextEdge();
			nextAlong = nextEdge->Along();
			
			double distToNext = e->GetLength() - currPosInfo.GetQuant();
			double nextFactor = 0.0;
			double test = 64.0;

			V2d realAlong;

			if (distToNext <= test)
			{
				nextFactor = 1.0 - distToNext / test;
				//realAlong = currAlong * (1.0 - factor) + nextAlong * (1.0 - factor);
			}

			double distToPrev = currPosInfo.GetQuant();
			double prevFactor = 0.0;
			if (distToPrev < test)
			{
				prevFactor = distToPrev / test;
			}

			if (nextFactor > 0)
			{
				realAlong = currAlong * (1.0 - nextFactor) + nextAlong * nextFactor;
			}
			else if (prevFactor > 0)
			{
				realAlong = currAlong * (1.0 - prevFactor) + prevAlong * prevFactor;
			}
			else
			{
				realAlong = currAlong;
			}
			
			
			/*if (nextFactor > 0)
			{
				realAlong = currAlong * (1.0 - nextFactor) + nextAlong * nextFactor;
			}
			

			V2d realAlong = currAlong * factor + nextAlong * ( 1.0 - factor);*/
			realAlong = normalize(realAlong);
			double realAng = GetVectorAngleCW(realAlong);
			double realAngD = realAng / PI * 180.0;

			sprite.setRotation(realAngD);

		}
		/*float rotateVel = 2;
		float ang = sprite.getRotation();
		if (angleD > ang)
		{
			ang += rotateVel;
			if (ang > angleD)
				ang = angleD;
		}
		else if (angleD < ang)
		{
			ang += -rotateVel;
			if (ang < angleD)
				ang = angleD;
		}
		sprite.setRotation(ang);*/
	}

	sprite.setRotation(0);
	/*if (enemyMover.currPosInfo.IsAerial())
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2);
	}
	else
	{
		
	}*/
	
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

