#include "Session.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Tiger.h"
#include "Actor.h"
#include "SequenceW4.h"
#include "SequenceW6.h"
#include "Enemy_TigerTarget.h"

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
	//spinTurretSummonGroup(this, new BasicAirEnemyParams(sess->types["tigerspinturret"], 1),
	//	4, 4, 1, true),
	targetGroup(this, new BasicAirEnemyParams(sess->types["tigertarget"], 1),
		6, 6, 1, true),
	nodeGroupA(2)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE_GRIND, 0, 0);

	StageSetup(8, 2);

	level = ap->GetLevel();

	actionLength[SUMMON] = 60;
	actionLength[THROW_SPINTURRET] = 60;
	actionLength[SUMMON_FLAME_TARGETS] = 60;//180;
	actionLength[LAUNCH_FLAME_TARGETS] = 60;//180;
	
	actionLength[GATHER_ENERGY_START] = 9;
	animFactor[GATHER_ENERGY_START] = 3;

	actionLength[GATHER_ENERGY_LOOP] = 1;

	actionLength[GATHER_ENERGY_END] = 4;
	animFactor[GATHER_ENERGY_END] = 3;


	actionLength[JUMP_LAND] = 10;
	actionLength[JUMP_SQUAT] = 10;

	actionLength[START_GRIND] = 8;
	animFactor[START_GRIND] = 3;

	actionLength[MOVE_GRIND] = 8;
	animFactor[MOVE_GRIND] = 3;

	actionLength[FLAME_STUN] = 180;
	animFactor[FLAME_STUN] = 1;

	ts_move = GetSizedTileset("Enemies/Bosses/Tiger/tiger_walk_256x160.png");
	ts_grind = GetSizedTileset("Enemies/Bosses/Tiger/tiger_grind_256x256.png");
	ts_roar = GetSizedTileset("Enemies/Bosses/Tiger/tiger_roar_256x160.png");
	//sprite.setColor(Color::Red);

	TigerTarget *target = NULL;
	for (int i = 0; i < targetGroup.numTotalEnemies; ++i)
	{
		target = (TigerTarget*)targetGroup.enemies[i];
		target->tiger = this;
	}


	stageMgr.AddActiveOptionToStages(0, START_GRIND, 2);
	stageMgr.AddActiveOptionToStages(0, JUMP_SQUAT, 2);
	stageMgr.AddActiveOptionToStages(0, GATHER_ENERGY_START, 2);
	//stageMgr.AddActiveOptionToStages(0, SUMMON, 2);

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this,
		BasicBullet::LIZARD, 32, 1, GetPosition(), V2d(0, -1), 0, 180, true);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->damage = 60;

	postFightScene = NULL;
	postFightScene2 = NULL;

	LoadParams();

	maxChargeLoopFrames = 60;
	
	defaultMoveOnlyFrames = 360;

	BasicCircleHurtBodySetup(64);
	BasicCircleHitBodySetup(64);

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

void Tiger::AddToGame()
{
	Enemy::AddToGame();

	palmSummonGroup.SetEnemyIDAndAddToAllEnemiesVec();
	targetGroup.SetEnemyIDAndAddToAllEnemiesVec();
}

void Tiger::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Enemies/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Tiger::ResetEnemy()
{
	snakePool.Reset();
	//spinTurretSummonGroup.Reset();
	palmSummonGroup.Reset();
	targetGroup.Reset();

	currChargeLoopFrame = 0;

	moveOnlyMaxFrames = defaultMoveOnlyFrames;

	if (sess->preLevelScene == NULL) //fight testing
	{
		CameraShot *cs = sess->cameraShotMap["fightcam"];
		if (cs != NULL)
		{
			sess->cam.Set(Vector2f(cs->centerPos), cs->zoom, 0);
		}
	}

	numFlamesHitBy = 0;

	lastTargetDestroyedPos = V2d(0, 0);

	BossReset();

	currMaxTargets = 3;

	facingRight = true;

	HitboxesOff();

	framesSinceRush = 0;

	moveOnlyFrames = 0;//-1;

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
	float factor = (float)stageMgr.currStage / stageMgr.numStages;
	moveOnlyMaxFrames = defaultMoveOnlyFrames - 160 * factor;

	/*20 -> 180

	if (stageMgr.currStage == 1)
	{
		moveOnlyMaxFrames = 180;
	}
	else if (stageMgr.currStage == 2)
	{
		moveOnlyMaxFrames = 20;
	}
	else if (stageMgr.currStage == 3)
	{
		moveOnlyMaxFrames = 90;
	}*/

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
	case JUMP_LAND:
	case SUMMON:
	case THROW_SPINTURRET:
	case SUMMON_FLAME_TARGETS:
	case LAUNCH_FLAME_TARGETS:
		if (moveOnlyFrames >= moveOnlyMaxFrames)
		{
			SetAction(GATHER_ENERGY_START);
		}
		else
		{
			Decide();
		}
		break;
	case COMBOMOVE:
		SetNextComboAction();
		break;
	case MOVE_RUSH:
	{
		enemyMover.currPosInfo.SetSurface(rayCastInfo.rcEdge, rayCastInfo.rcQuant);
		currPosInfo = enemyMover.currPosInfo;
		SetAction(START_GRIND);
		break;
	}
	case GATHER_ENERGY_START:
	{
		SetAction(GATHER_ENERGY_LOOP);
		currChargeLoopFrame = 0;
		break;
	}
	case GATHER_ENERGY_LOOP:
	{
		frame = 0;
		break;
	}
	case GATHER_ENERGY_END:
	{
		if (targetGroup.numActiveEnemies == 0 )
		{
			SetAction(SUMMON_FLAME_TARGETS);
		}
		else
		{
			SetAction(LAUNCH_FLAME_TARGETS);
		}
		
		break;
	}
	case MOVE_GRIND:
	{
		SetAction(MOVE_JUMP);
		break;
	}
	case MOVE_JUMP:
	{
		SetAction(JUMP_LAND);
		break;
	}
	case JUMP_SQUAT:
	{
		SetAction(MOVE_JUMP);
		break;
	}
	case START_GRIND:
	{
		SetAction(MOVE_GRIND);
		break;
	}
	case FLAME_STUN:
	{
		SetAction(MOVE_JUMP);
		//Decide();
		break;
	}
	/*case FLAME_STUN_AIR:
	{
		if( frame <= actionLength[FLAME_STUN_GROUND] * animFactor[FLAME_STUN_GROUND])
	}*/
	}
}

void Tiger::HandleAction()
{
	switch (action)
	{
	case SUMMON:
	{
		if (frame == 20 && slowCounter == 1)
		{
			palmSummonGroup.Summon();
		}
	}
	case MOVE_GRIND:
	{
		if (moveOnlyFrames >= moveOnlyMaxFrames)
		{
			SetAction(MOVE_JUMP);
		}
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
	case MOVE_JUMP:
	{
		if (enemyMover.actionFrame == enemyMover.actionTotalDuration/2)
		{
			snakePool.Throw(TigerGrindBullet::GB_REGULAR_CW, GetPosition(), PlayerDir());
		}


		
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
		int grindFrames = 120;

		int gf = rand() % 2;

		if (gf == 0)
		{
			grindFrames = 60;
		}
		else
		{
			grindFrames = 120;
		}

		if (gr == 0)
		{
			enemyMover.SetModeGrind(grindSpeed, grindFrames);
			facingRight = true;
		}
		else if (gr == 1)
		{
			enemyMover.SetModeGrind(-grindSpeed, grindFrames);
			facingRight = false;
		}
		break;
	}
	case MOVE_JUMP:
	{
		double jumpHeight = 200;

		/*if (enemyMover.currPosInfo.ground != NULL
			&& enemyMover.currPosInfo.GetEdge()->Normal().y > 0)
		{
			jumpHeight = 1;
		}*/


		enemyMover.currPosInfo.SetAerial();
		currPosInfo.SetAerial();
		PoiInfo *node = nodeGroupA.AlwaysGetNextNode();

		

		enemyMover.SetModeNodeProjectile(node->pos, V2d(0, 1.5), jumpHeight);
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
		PoiInfo *node = nodeGroupGrind.AlwaysGetNextNode();//nodeGroupGrind.AlwaysGetNextNode();
		//snakePool.ThrowAt(TigerGrindBullet::GB_REGULAR_CW, GetPosition(), node);
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
	case SUMMON_FLAME_TARGETS:
	{
		moveOnlyFrames = 0;
		nodeGroupC.pickers[0].ShuffleActiveOptions();
		for (int i = 0; i < currMaxTargets; ++i)
		{
			targetGroup.Summon();
		}

		numFlamesHitBy = 0;
		break;
	}
	case LAUNCH_FLAME_TARGETS:
	{
		moveOnlyFrames = 0;
		Enemy *e;
		TigerTarget *tt = NULL;
		for (int i = 0; i < targetGroup.numTotalEnemies; ++i)
		{
			e = targetGroup.enemies[i];
			tt = (TigerTarget*)e;
			if (tt->IsReadyToThrow())
			{
				tt->AttackPlayer();
			}
			
		}
	}
	case FLAME_STUN:
	{
		enemyMover.Stop();
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
	nodeGroupGrind.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_TIGER, "A"));
}

void Tiger::DrawMinimap( sf::RenderTarget *target )
{
	Enemy::DrawMinimap(target);
	palmSummonGroup.DrawMinimap(target);
	targetGroup.DrawMinimap(target);
	snakePool.DrawMinimap(target);
	//spinTurretSummonGroup.DrawMinimap(target);
}

void Tiger::FrameIncrement()
{
	Boss::FrameIncrement();
	if (action != MOVE_RUSH)
	{
		++framesSinceRush;
	}

	if ((action == MOVE_GRIND || action == MOVE_JUMP) && moveOnlyFrames >= 0)
	{
		++moveOnlyFrames;
	}

	if (action == GATHER_ENERGY_LOOP)
	{
		++currChargeLoopFrame;
		if (currChargeLoopFrame == maxChargeLoopFrames)
		{
			SetAction(GATHER_ENERGY_END);
		}
	}
}

bool Tiger::IsDecisionValid(int d)
{
	if (d == SUMMON && !palmSummonGroup.CanSummon())
	{
		return false;
	}
	//else if ( (action == SUMMON_FLAME_TARGETS || action == HEAT_FLAME_TARGETS)
	else if ( d == GATHER_ENERGY_START && moveOnlyFrames >= 0 && moveOnlyFrames < moveOnlyMaxFrames )
	{
		return false;
	}
	else if (action == MOVE_GRIND && d == MOVE_GRIND)
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
	//spinTurretSummonGroup.Reset();
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
	sprite.setColor(Color::White);

	if (action == START_GRIND)
	{
		sprite.setTexture(*ts_grind->texture);
		ts_grind->SetSubRect(sprite, frame / animFactor[START_GRIND], !facingRight);
	}
	else if (action == MOVE_GRIND)
	{
		int f = frame % (actionLength[MOVE_GRIND] * animFactor[MOVE_GRIND]);
		sprite.setTexture(*ts_grind->texture);
		ts_grind->SetSubRect(sprite, f / animFactor[MOVE_GRIND] + 8, !facingRight);
	}
	else if (action == GATHER_ENERGY_START)
	{
		sprite.setTexture(*ts_roar->texture);
		ts_roar->SetSubRect(sprite, frame / animFactor[GATHER_ENERGY_START], !facingRight);
	}
	else if (action == GATHER_ENERGY_LOOP)
	{
		sprite.setTexture(*ts_roar->texture);
		ts_roar->SetSubRect(sprite, frame / animFactor[GATHER_ENERGY_LOOP] + 8, !facingRight);
	}
	else if (action == GATHER_ENERGY_END)
	{
		sprite.setTexture(*ts_roar->texture);
		ts_roar->SetSubRect(sprite, frame / animFactor[GATHER_ENERGY_END] + 18, !facingRight);
	}
	else if (action == FLAME_STUN )
	{
		sprite.setTexture(*ts_roar->texture);
		ts_roar->SetSubRect(sprite, 0, !facingRight);
		sprite.setColor(Color::Red);
	}
	else
	{
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 0, !facingRight);
	}
	

	if (action == GATHER_ENERGY_START)
	{
		//sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
		//sprite.setColor(Color::Red);
	}
	else
	{
		//sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
		//sprite.setColor(Color::White);
	}

	if (action == MOVE_GRIND)
	{
		sprite.setRotation(0);
	}
	else
	{
		sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
	}
	

	double extra = 64;
	if (action == MOVE_GRIND)
	{
		extra = 0;
	}
	else if (action == START_GRIND)
	{
		if (frame / animFactor[START_GRIND] == 6)
		{
			extra = extra * .7;
		}
		else if (frame / animFactor[START_GRIND] == 7)
		{
			extra = extra * .3;
		}
		//double fac = ((double)frame) / (actionLength[START_GRIND] * animFactor[START_GRIND]);
		//extra = extra * (1.f - fac);
	}
	sprite.setPosition(GetPositionF() + Vector2f( 0, -extra ));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Tiger::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	snakePool.Draw(target);
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
	/*else if (group == &spinTurretSummonGroup)
	{
		TigerSpinTurret *t = (TigerSpinTurret*)e;
		t->Init(GetPosition(), PlayerDir());
	}*/
	else if (group == &targetGroup)
	{
		PoiInfo *summonNode;
		summonNode = nodeGroupC.AlwaysGetNextNode();//nodeGroupA.AlwaysGetNextNode(1);
		if (summonNode->pos == lastTargetDestroyedPos)
		{
			summonNode = nodeGroupC.AlwaysGetNextNode();
		}
		e->startPosInfo.SetAerial(summonNode->pos);

		TigerTarget *tt = (TigerTarget*)e;

		tt->SetBurnFrames(moveOnlyMaxFrames);

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

int Tiger::GetNumSimulationFramesRequired()
{
	return 0;
}

void Tiger::HitWithFlame()
{
	numFlamesHitBy++;

	if (numFlamesHitBy == currMaxTargets)
	{
		SetAction(FLAME_STUN);
		/*if (action == MOVE_JUMP)
		{
			SetAction(FLAME_STUN_AIR);
		}
		else
		{
			SetAction(FLAME_STUN_GROUND);
		}*/
	}
}