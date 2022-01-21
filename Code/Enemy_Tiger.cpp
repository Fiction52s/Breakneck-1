#include "Session.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Tiger.h"
#include "Actor.h"
#include "SequenceW4.h"
#include "SequenceW6.h"

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


Tiger::Tiger(ActorParams *ap)
	:Boss(EnemyType::EN_TIGERBOSS, ap),
	palmSummonGroup(this,
		new BasicGroundEnemyParams(sess->types["palmturret"], 1),
		2, 2, 1),
	spinTurretSummonGroup(this, new BasicAirEnemyParams(sess->types["tigerspinturret"], 1),
		4, 4, 1, true),
	targetGroup(this, new BasicAirEnemyParams(sess->types["tigertarget"], 1),
		3, 3, 1, true),
	nodeGroupA(2)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE_GRIND, 0, 0);

	StageSetup(4, 4);

	level = ap->GetLevel();

	actionLength[SUMMON] = 60;
	actionLength[THROW_SPINTURRET] = 60;
	actionLength[TEST] = 60;//10000;

	ts_move = GetSizedTileset("Bosses/Coyote/coy_stand_80x64.png");
	ts_bulletExplode = GetSizedTileset("FX/bullet_explode2_64x64.png");
	sprite.setColor(Color::Red);

	stageMgr.AddActiveOption(0, TEST, 2);
	stageMgr.AddActiveOption(0, MOVE_GRIND, 2);
	stageMgr.AddActiveOption(0, MOVE_JUMP, 2);
	//stageMgr.AddActiveOption(0, MOVE_RUSH, 2);
	stageMgr.AddActiveOption(0, SUMMON, 2);
	//stageMgr.AddActiveOption(0, THROW_SPINTURRET, 2);

	stageMgr.AddActiveOption(1, MOVE_GRIND, 2);
	stageMgr.AddActiveOption(1, MOVE_JUMP, 2);

	stageMgr.AddActiveOption(2, MOVE_GRIND, 2);
	stageMgr.AddActiveOption(2, MOVE_JUMP, 2);

	stageMgr.AddActiveOption(3, MOVE_GRIND, 2);
	stageMgr.AddActiveOption(3, MOVE_JUMP, 2);

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this,
		BasicBullet::LIZARD, 32, 1, GetPosition(), V2d(0, -1), 0, 180, true);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->damage = 18;

	postFightScene = NULL;
	postFightScene2 = NULL;

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	ResetEnemy();
}

Tiger::~Tiger()
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

void Tiger::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Tiger::ResetEnemy()
{
	snakePool.Reset();
	spinTurretSummonGroup.Reset();
	palmSummonGroup.Reset();
	targetGroup.Reset();

	lastTargetDestroyedPos = V2d(0, 0);

	BossReset();

	facingRight = true;

	HitboxesOff();

	framesSinceRush = 0;

	StartFight();

	UpdateSprite();
}

void Tiger::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
}

bool Tiger::TryComboMove(V2d &comboPos, int comboMoveDuration,
	int moveDurationBeforeStartNextAction,
	V2d &comboOffset)
{
	return false;
}

int Tiger::ChooseActionAfterStageChange()
{
	return Boss::ChooseActionAfterStageChange();
}

void Tiger::ActivatePostFightScene()
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

void Tiger::ActionEnded()
{
	switch (action)
	{
	case WAIT:
	case MOVE_GRIND:
	case MOVE_JUMP:
	case SUMMON:
	case THROW_SPINTURRET:
	case TEST:
		Decide();
		break;
	case COMBOMOVE:
		SetNextComboAction();
		break;
	case MOVE_RUSH:
	{
		enemyMover.currPosInfo.SetSurface(rayCastInfo.rcEdge, rayCastInfo.rcQuant);
		currPosInfo = enemyMover.currPosInfo;
		SetAction(MOVE_GRIND);
		break;
	}
	}
}

void Tiger::HandleAction()
{
	switch (action)
	{
	case SUMMON:
	{
		if( frame == 20 && slowCounter == 1 )
		{
			palmSummonGroup.Summon();
		}
	}
	case MOVE_GRIND:
	{
		/*double dist = PlayerDist();
		if (framesSinceRush > 60 && dist > 500 && dist < 2000)
		{
			SetAction(MOVE_RUSH);
		}*/
		break;
	}
	case TEST:
	{
		//if (targetGroup.numActiveEnemies == 0)
		//{
		//	nodeGroupC.pickers[0].ShuffleActiveOptions();
		//	for (int i = 0; i < 3; ++i)
		//	{
		//		targetGroup.Summon();
		//	}
		//	
		//	//snakePool.Throw( TigerGrindBullet::GB_REGULAR_CW, GetPosition(), PlayerDir());
		//}

		
		break;
	}
	}
	/*if (action == MOVE_JUMP && enemyMover.actionFrame % 4 == 0)
	{
		launchers[0]->position = GetPosition();
		launchers[0]->facingDir = PlayerDir();
		launchers[0]->Fire();
	}*/
}

void Tiger::StartAction()
{
	switch (action)
	{
	case MOVE_GRIND:
	{
		int gr = rand() % 2;

		if (facingRight)
		{
			gr = 0;
		}
		else
		{
			gr = 1;
		}

		double grindSpeed = 20;
		if (gr == 0)
		{
			enemyMover.SetModeGrind(grindSpeed, 120);
			facingRight = true;
		}
		else if (gr == 1)
		{
			enemyMover.SetModeGrind(-grindSpeed, 120);
			facingRight = false;
		}
		break;
	}
	case MOVE_JUMP:
	{
		enemyMover.currPosInfo.SetAerial();
		currPosInfo.SetAerial();
		PoiInfo *node = nodeGroupA.AlwaysGetNextNode();
		enemyMover.SetModeNodeProjectile(node->pos, V2d(0, 1.5), 200);
		enemyMover.SetDestNode(node);

		if (node->pos.x >= GetPosition().x)
		{
			facingRight = true;
		}
		else
		{
			facingRight = false;
		}
		
		//spinPool.Throw(GetPosition(), PlayerDir());
		break;
	}
	case THROW_SPINTURRET:
	{
		PoiInfo *node = nodeGroupA.AlwaysGetNextNode();//nodeGroupGrind.AlwaysGetNextNode();
		snakePool.ThrowAt(TigerGrindBullet::GB_REGULAR_CW, GetPosition(), node);
		//spinTurretSummonGroup.Summon();
		break;
	}
	case MOVE_LUNGE:
	{

		break;
	}
	case MOVE_RUSH:
	{
		framesSinceRush = 0;
		V2d pPos = targetPlayer->position;
		rayCastInfo.rcEdge = NULL;
		rayCastInfo.rayStart = GetPosition();
		rayCastInfo.rayEnd = pPos + PlayerDir(targetPlayerIndex) * 5000.0;
		ignorePointsCloserThanPlayer = true;
		playerDist = PlayerDist(targetPlayerIndex);
		RayCast(this, sess->terrainTree->startNode, rayCastInfo.rayStart, rayCastInfo.rayEnd);

		if (rayCastInfo.rcEdge != NULL)
		{
			assert(rayCastInfo.rcEdge != NULL);

			V2d basePos = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant);

			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();

			enemyMover.SetModeNodeLinearConstantSpeed(basePos, CubicBezier(), 40);
			//enemyMover.SetModeNodeProjectile(basePos, V2d(0, 1.5), 200);

			if (basePos.x >= GetPosition().x)
			{
				facingRight = true;
			}
			else
			{
				facingRight = false;
			}
			//enemyMover.SetModeZipAndFall(basePos, V2d(0, 2), nodePos);
		}
		else
		{
			Wait(1);
			assert(0);
		}
		break;
	}
	case TEST:
	{
		nodeGroupC.pickers[0].ShuffleActiveOptions();
		for (int i = 0; i < 3; ++i)
		{
			targetGroup.Summon();
		}

		//snakePool.Throw( TigerGrindBullet::GB_REGULAR_CW, GetPosition(), PlayerDir());
		break;
	}

	}
}

void Tiger::SetupPostFightScenes()
{
	if (level == 1)
	{
		if (postFightScene2 != NULL)
		{
			delete postFightScene2;
			postFightScene2 = NULL;
		}

		if (postFightScene == NULL)
		{
			postFightScene = new TigerPostFightScene;
			//postFightScene->tiger = this;
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

		postFightScene2 = new TigerPostFight2Scene;
		postFightScene2->tiger = this;
		postFightScene2->Init();
	}
}

void Tiger::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_TIGER, "A"));
	nodeGroupB.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_TIGER, "B"));
	nodeGroupC.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_TIGER, "C"));
	//nodeGroupGrind.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_TIGER, "A"));
}

void Tiger::DrawMinimap( sf::RenderTarget *target )
{
	Enemy::DrawMinimap(target);
	palmSummonGroup.DrawMinimap(target);
	targetGroup.DrawMinimap(target);
	spinTurretSummonGroup.DrawMinimap(target);
}

void Tiger::FrameIncrement()
{
	Boss::FrameIncrement();
	if (action != MOVE_RUSH)
	{
		++framesSinceRush;
	}
	
}

bool Tiger::IsDecisionValid(int d)
{
	if (d == SUMMON && !palmSummonGroup.CanSummon())
	{
		return false;
	}

	return true;
}

bool Tiger::IsEnemyMoverAction(int a)
{
	return a == MOVE_GRIND || a == MOVE_JUMP || a == MOVE_RUSH;
}

void Tiger::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;
	snakePool.Reset();
	spinTurretSummonGroup.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Tiger::StartFight()
{
	Wait(10);
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	frame = 0;
	HitboxesOff();
}

void Tiger::UpdateSprite()
{
	sprite.setTexture(*ts_move->texture);
	ts_move->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Tiger::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	snakePool.Draw(target);
}

void Tiger::BulletHitPlayer(
	int playerIndex,
	BasicBullet *b,
	int hitResult)
{
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}

	b->launcher->DeactivateBullet(b);
}

void Tiger::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &palmSummonGroup)
	{
		PoiInfo *summonNode;

		summonNode = nodeGroupB.AlwaysGetNextNode();
		e->startPosInfo.SetGround(summonNode->poly,
			summonNode->edgeIndex, summonNode->edgeQuantity);
	}
	else if (group == &spinTurretSummonGroup)
	{
		TigerSpinTurret *t = (TigerSpinTurret*)e;
		t->Init(GetPosition(), PlayerDir());
	}
	else if (group == &targetGroup)
	{
		PoiInfo *summonNode;
		summonNode = nodeGroupC.AlwaysGetNextNode();//nodeGroupA.AlwaysGetNextNode(1);
		if (summonNode->pos == lastTargetDestroyedPos)
		{
			summonNode = nodeGroupC.AlwaysGetNextNode();
		}
		e->startPosInfo.SetAerial(summonNode->pos);
		//cout << "summon pos:" << summonNode->pos.x << ", " << summonNode->pos.y << endl;
	}
}

void Tiger::HandleSummonedChildRemoval(Enemy *e)
{
	if (e->type == EnemyType::EN_TIGERTARGET)
	{
		lastTargetDestroyedPos = e->GetPosition();
	}
}

void Tiger::HandleRayCollision(Edge *edge, double edgeQuantity,
	double rayPortion)
{
	if (edge->edgeType == Edge::BORDER)
	{
		return;
	}

	V2d dir = normalize(rayCastInfo.rayEnd - rayCastInfo.rayStart);
	V2d pos = edge->GetPosition(edgeQuantity);

	//if (!sess->IsWithinBounds(pos))
	//{
	//	return; //prevents it from hitting the birdtransform area currently
	//}

	double along = dot(dir, edge->Normal());

	double posDist = length(pos - GetPosition());

	if (along < 0 && (rayCastInfo.rcEdge == NULL || length(edge->GetPosition(edgeQuantity) - rayCastInfo.rayStart) <
		length(rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcPortion) - rayCastInfo.rayStart)))
	{
		if (!ignorePointsCloserThanPlayer || (ignorePointsCloserThanPlayer && posDist > playerDist))
		{
			rayCastInfo.rcEdge = edge;
			rayCastInfo.rcQuant = edgeQuantity;
		}
	}
}


//Rollback

int Tiger::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Tiger::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);
}

void Tiger::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);


	bytes += sizeof(MyData);
}