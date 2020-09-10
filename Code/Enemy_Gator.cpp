#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Gator.h"
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


Gator::Gator(ActorParams *ap)
	:Enemy(EnemyType::EN_GATORBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;
	reachPointOnFrame[COMBOMOVE] = 0;

	ts_move = sess->GetSizedTileset("Bosses/Gator/dominance_384x384.png");

	nodeAStr = "A";

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

	//BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);


	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	ResetEnemy();
}

void Gator::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Gator::UpdateHitboxes()
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

void Gator::ResetEnemy()
{
	playerComboer.Reset();
	orbPool.Reset();
	enemyMover.Reset();

	facingRight = true;

	action = WAIT;
	SetHitboxes(NULL);
	waitFrames = 10;

	//action = PUNCH;
	//SetHitboxInfo(PUNCH);
	//DefaultHitboxesOn();

	hitPlayer = false;
	comboMoveFrames = 0;

	actionQueueIndex = 0;

	frame = 0;

	UpdateSprite();
}

void Gator::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

void Gator::SetCommand(int index, BirdCommand &bc)
{
	actionQueue[index] = bc;
}

void Gator::DebugDraw(sf::RenderTarget *target)
{
	playerComboer.DebugDraw(target);
	enemyMover.DebugDraw(target);
}

void Gator::DirectKill()
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

void Gator::FrameIncrement()
{
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

void Gator::UpdatePreFrameCalculations()
{
	Actor *targetPlayer = sess->GetPlayer(targetPlayerIndex);

	if (playerComboer.CanPredict(targetPlayerIndex))
	{
		if (actionQueueIndex == 3)
		{
			dead = true;
			sess->RemoveEnemy(this);
			return;
		}

		playerComboer.UpdatePreFrameCalculations(targetPlayerIndex);
		targetPos = playerComboer.GetTargetPos();

		comboMoveFrames = targetPlayer->hitstunFrames - 1;//(hitBody.hitboxInfo->hitstunFrames - 1);
		counterTillAttack = comboMoveFrames - 10;

		//enemyMover.SetModeNodeJump(targetPos, 200);
		enemyMover.SetModeNodeProjectile(targetPos, V2d(0, 1.0), 200);
		//enemyMover.SetModeNodeLinear(targetPos, CubicBezier(), comboMoveFrames);

		int nextAction = actionQueue[actionQueueIndex].action + 1;
		comboMoveFrames -= actionLength[nextAction] * animFactor[nextAction] - 10;

		if (comboMoveFrames < 0)
		{
			comboMoveFrames = 0;
		}

		SetHitboxes(NULL, 0);

		action = COMBOMOVE;
		frame = 0;
		hitPlayer = false;
	}
}

void Gator::ProcessState()
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

	if (action == MOVE && enemyMover.IsIdle())
	{
		action = WAIT;
		waitFrames = 10;
	}
	else if (action == WAIT && waitFrames == 0)
	{
		int r = rand() % 3;

		auto &nodeVec = sess->GetBossNodeVector(BossFightType::FT_GATOR, nodeAStr);
		int vecSize = nodeVec.size();
		int rNode = rand() % vecSize;

		V2d nodePos = nodeVec[rNode]->pos;

		V2d pPos = sess->GetPlayerPos(0);
		V2d pDir = normalize(pPos - GetPosition());



		if (r == 0)
		{
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
			orbPool.Throw(GetPosition(), nodePos, GatorWaterOrb::OrbType::NODE_GROW);
		}
		else if (r == 1)
		{
			enemyMover.SetModeNodeQuadratic(pPos, nodePos, CubicBezier(), 60);
			orbPool.Throw(GetPosition(), nodePos, GatorWaterOrb::OrbType::NODE_GROW);
		}
		else if (r == 2)
		{
			enemyMover.SetModeChase(&sess->GetPlayer(0)->position, V2d(0, 0),
				10, .5, 60);
			orbPool.Throw(GetPosition(), nodePos, GatorWaterOrb::OrbType::NODE_GROW);
		}
		else if (r == 3)
		{

		}


		action = MOVE;
		moveFrames = 60;
	}
	else if (action == COMBOMOVE)
	{
		if (comboMoveFrames == 0)
		{
			action = actionQueue[actionQueueIndex].action + 1;
			facingRight = actionQueue[actionQueueIndex].facingRight;
			SetHitboxInfo(action);
			//only have this on if i dont turn on hitboxes at the end of the movement.
			DefaultHitboxesOn();

		}
	}

	bool comboInterrupted = sess->GetPlayer(targetPlayerIndex)->hitOutOfHitstunLastFrame
		&& comboMoveFrames > 0;
	//added this combo counter thing
	if (hitPlayer || comboInterrupted)
	{
		action = COMBOMOVE;
		frame = 0;
		playerComboer.PredictNextFrame();
		if (!comboInterrupted)
			++actionQueueIndex;
		SetHitboxes(NULL, 0);

		if (actionQueueIndex == 3)
		{

		}
	}

	hitPlayer = false;
}

void Gator::IHitPlayer(int index)
{
	hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames;
}

void Gator::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void Gator::UpdateSprite()
{
	switch (action)
	{
	case WAIT:
	case MOVE:
	case COMBOMOVE:
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 0, !facingRight);
		break;
	}

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Gator::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	orbPool.Draw(target);
}

void Gator::HandleHitAndSurvive()
{
	//fireCounter = 0;
}

int Gator::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Gator::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	//d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
}

void Gator::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	//fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
}

//Gator::Gator(ActorParams *ap)
//	:Enemy(EnemyType::EN_GATORBOSS, ap)
//{
//	SetNumActions(A_Count);
//	SetEditorActions(DOMINATION, 0, 0);
//
//	actionLength[DOMINATION] = 60;
//	animFactor[DOMINATION] = 1;
//
//	bulletSpeed = 10;
//	framesBetween = 60;
//
//	Tileset *ts_basicBullets = sess->GetTileset("Enemies/bullet_64x64.png", 64, 64);
//
//	SetNumLaunchers(1);
//	launchers[0] = new Launcher(this, BasicBullet::BAT, 16, 1, GetPosition(),
//		V2d(1, 0), 0, 120, false, 0, 0, ts_basicBullets );
//	launchers[0]->SetBulletSpeed(bulletSpeed);
//	launchers[0]->hitboxInfo->damage = 18;
//	launchers[0]->playerIndex = 1;
//
//	animationFactor = 5;
//
//	ts = sess->GetSizedTileset("Bosses/Gator/dominance_384x384.png");
//	sprite.setTexture(*ts->texture);
//
//	hitboxInfo = new HitboxInfo;
//	hitboxInfo->damage = 18;
//	hitboxInfo->drainX = 0;
//	hitboxInfo->drainY = 0;
//	hitboxInfo->hitlagFrames = 0;
//	hitboxInfo->hitstunFrames = 10;
//	hitboxInfo->knockback = 4;
//
//	//BasicCircleHurtBodySetup(16);
//	BasicCircleHitBodySetup(16);
//	hitBody.hitboxInfo = hitboxInfo;
//
//	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);
//
//	ResetEnemy();
//}
//
//int Gator::GetNumStoredBytes()
//{
//	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
//}
//
//void Gator::StoreBytes(unsigned char *bytes)
//{
//	MyData d;
//	memset(&d, 0, sizeof(MyData));
//	StoreBasicEnemyData(d);
//	d.fireCounter = fireCounter;
//
//	memcpy(bytes, &d, sizeof(MyData));
//
//	bytes += sizeof(MyData);
//
//	launchers[0]->StoreBytes(bytes);
//}
//
//void Gator::SetFromBytes(unsigned char *bytes)
//{
//	MyData d;
//	memcpy(&d, bytes, sizeof(MyData));
//
//	SetBasicEnemyData(d);
//
//	fireCounter = d.fireCounter;
//
//	bytes += sizeof(MyData);
//
//	launchers[0]->SetFromBytes(bytes);
//}
//
//void Gator::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
//{
//	//V2d vel = b->velocity;
//	//double angle = atan2( vel.y, vel.x );
//	V2d norm = edge->Normal();
//	double angle = atan2(norm.y, -norm.x);
//
//	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
//	b->launcher->DeactivateBullet(b);
//}
//
//void Gator::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
//{
//	//if you dont deactivate the bullet it will hit constantly and make weird fx
//
//	//cout << "hit player??" << endl;
//	V2d vel = b->velocity;
//	double angle = atan2(vel.y, vel.x);
//	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
//
//	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
//	{
//		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
//	}
//	
//	b->launcher->DeactivateBullet(b);
//}
//
//
//void Gator::ResetEnemy()
//{
//	fireCounter = 0;
//	facingRight = true;
//
//	action = DOMINATION;
//	frame = 0;
//
//	//DefaultHurtboxesOn();
//	DefaultHitboxesOn();
//
//	UpdateHitboxes();
//
//	UpdateSprite();
//}
//
//void Gator::DirectKill()
//{
//	for (int i = 0; i < numLaunchers; ++i)
//	{
//		BasicBullet *b = launchers[0]->activeBullets;
//		while (b != NULL)
//		{
//			BasicBullet *next = b->next;
//			double angle = atan2(b->velocity.y, -b->velocity.x);
//			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
//			b->launcher->DeactivateBullet(b);
//
//			b = next;
//		}
//	}
//	receivedHit = NULL;
//}
//
//void Gator::FrameIncrement()
//{
//	++fireCounter;
//}
//
//void Gator::ProcessState()
//{
//
//	V2d pDir = normalize(sess->GetPlayerPos(1) - GetPosition());
//	//if( (fireCounter == 0 || fireCounter == 10 || fireCounter == 20/*framesBetween - 1*/) && slowCounter == 1 )// frame == 0 && slowCounter == 1 )
//	if (slowCounter == 1)//&& action == FLY )
//	{
//		int f = fireCounter % 60;
//
//		if (f % 5 == 0 && f >= 25 && f < 50)
//		{
//			launchers[0]->position = GetPosition();
//			launchers[0]->facingDir = pDir;
//			launchers[0]->Fire();
//		}
//	}
//}
//
//void Gator::IHitPlayer(int index)
//{
//	V2d playerPos = sess->GetPlayerPos(index);
//	if (playerPos.x > GetPosition().x)
//	{
//		facingRight = true;
//	}
//	else
//	{
//		facingRight = false;
//	}
//}
//
//void Gator::UpdateEnemyPhysics()
//{
//}
//
//void Gator::UpdateSprite()
//{
//	sprite.setTextureRect(ts->GetSubRect(0));
//	sprite.setPosition(GetPositionF());
//	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
//}
//
//void Gator::EnemyDraw(sf::RenderTarget *target)
//{
//	DrawSprite(target, sprite);
//}
//
//void Gator::HandleHitAndSurvive()
//{
//	fireCounter = 0;
//}