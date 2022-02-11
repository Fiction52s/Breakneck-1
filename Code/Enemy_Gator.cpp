#include "Session.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Gator.h"
#include "Actor.h"
#include "SequenceW5.h"
#include "GameSession.h"

using namespace std;
using namespace sf;


Gator::Gator(ActorParams *ap)
	:Boss(EnemyType::EN_GATORBOSS, ap),
	swarmSummonGroup(this, new BasicAirEnemyParams(sess->types["swarm"], 1), 1, 1, 1)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	StageSetup(4, 4);

	ts_move = GetSizedTileset("Bosses/Gator/dominance_384x384.png");

	actionLength[SUMMON] = 60;
	actionLength[REDIRECT_ORBS] = 60;

	actionLength[TRIPLE_LUNGE_WAIT_1] = 30;
	actionLength[TRIPLE_LUNGE_WAIT_2] = 20;

	actionLength[ATTACK] = 10;

	hitboxStartFrame[ATTACK] = 0;
	hitboxStartFrame[TRIPLE_LUNGE_1] = 0;
	hitboxStartFrame[TRIPLE_LUNGE_2] = 0;
	hitboxStartFrame[TRIPLE_LUNGE_3] = 0;

	actionLength[TEST_ORBS] = 180;
	animFactor[TEST_ORBS] = 1;

	actionLength[CREATE_ORB_CIRCLE] = -1;
	animFactor[CREATE_ORB_CIRCLE] = 1;

	postFightScene = NULL;

	redirectRate = 20;

	orbTypePicker.AddActiveOption(0, 2);
	orbTypePicker.AddActiveOption(1, 2);

	//stageMgr.AddActiveOption(0, CREATE_ORB_CIRCLE, 2);
	stageMgr.AddActiveOption(0, CIRCLE_ORB_STUFF, 2);

	////stageMgr.AddActiveOption(0, TEST_POST, 2);
	////stageMgr.AddActiveOption(0, MOVE_CHASE, 2);
	//stageMgr.AddActiveOption(0, MOVE_NODE_LINEAR, 2);
	//stageMgr.AddActiveOption(0, MOVE_NODE_QUADRATIC, 2);
	//stageMgr.AddActiveOption(0, REDIRECT_ORBS, 2);
	////stageMgr.AddActiveOption(0, SUMMON, 2);

	//stageMgr.AddActiveOption(1, MOVE_CHASE, 2);
	//stageMgr.AddActiveOption(1, MOVE_NODE_LINEAR, 2);
	//stageMgr.AddActiveOption(1, MOVE_NODE_QUADRATIC, 2);

	//stageMgr.AddActiveOption(2, MOVE_CHASE, 2);
	//stageMgr.AddActiveOption(2, MOVE_NODE_LINEAR, 2);
	//stageMgr.AddActiveOption(2, MOVE_NODE_QUADRATIC, 2);

	//stageMgr.AddActiveOption(3, MOVE_CHASE, 2);
	//stageMgr.AddActiveOption(3, MOVE_NODE_LINEAR, 2);
	//stageMgr.AddActiveOption(3, MOVE_NODE_QUADRATIC, 2);

	myBonus = NULL;

	LoadParams();

	CreateHitboxManager("Bosses/Gator");
	SetupHitboxes(TRIPLE_LUNGE_1, "dominance");
	SetupHitboxes(TRIPLE_LUNGE_2, "dominance");
	SetupHitboxes(TRIPLE_LUNGE_3, "dominance");
	SetupHitboxes(ATTACK, "dominance");

	BasicCircleHurtBodySetup(16);
	//BasicCircleHitBodySetup(16);

	ResetEnemy();
}

Gator::~Gator()
{
	if (postFightScene != NULL)
		delete postFightScene;

	if (myBonus != NULL)
		delete myBonus;
}


void Gator::LoadParams()
{
	ifstream is;
	is.open("Resources/Bosses/Gator/gatorparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["dominance"], hitboxInfos[ATTACK]);
	HitboxInfo::SetupHitboxLevelInfo(j["dominance"], hitboxInfos[TRIPLE_LUNGE_1]);
	HitboxInfo::SetupHitboxLevelInfo(j["dominance"], hitboxInfos[TRIPLE_LUNGE_2]);
	HitboxInfo::SetupHitboxLevelInfo(j["dominance"], hitboxInfos[TRIPLE_LUNGE_3]);
	//HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);
}

void Gator::ResetEnemy()
{
	if (myBonus != NULL)
	{
		myBonus->RestartLevel();
	}

	rayCastInfo.tree = sess->terrainTree;

	for (int i = 0; i < NUM_ORB_POOLS; ++i)
	{
		orbPool[i].Reset();
	}
	swarmSummonGroup.Reset();

	BossReset();

	if (sess->preLevelScene == NULL) //fight testing
	{
		CameraShot *cs = sess->cameraShotMap["fightcam"];
		if (cs != NULL)
		{
			sess->cam.Set(Vector2f(cs->centerPos), cs->zoom, 0);
		}
	}

	facingRight = true;

	HitboxesOff();

	StartFight();
	redirectingOrbs = false;

	UpdateSprite();
}

void Gator::InitBonus()
{
	//sess->GetPlayer(0)->position = oldPlayerPos;
}

bool Gator::TryComboMove(V2d &comboPos, int comboMoveDuration,
	int moveDurationBeforeStartNextAction, int framesRemaining,
	V2d &comboOffset)
{
	SetAction(COMBOMOVE);
	//nextAction = COMBOMOVE;
	actionLength[COMBOMOVE] = moveDurationBeforeStartNextAction;

	facingRight = GetCurrCommand().facingRight;


	enemyMover.SetModeNodeLinear(comboPos + comboOffset, CubicBezier(), comboMoveDuration);

	return true;
}
int Gator::ChooseActionAfterStageChange()
{
	return Boss::ChooseActionAfterStageChange();
}

void Gator::ActivatePostFightScene()
{
	postFightScene->Reset();
	sess->SetActiveSequence(postFightScene);
}

void Gator::ActionEnded()
{
	switch (action)
	{
	case MOVE_NODE_LINEAR:
	case MOVE_NODE_QUADRATIC:
		if (!redirectingOrbs)
		{
			V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
			//orbPool.Throw(GetPosition(), nodePos, GatorWaterOrb::NODE_GROW_HIT);//orbTypePicker.AlwaysGetNextOption());
		}
		Wait(10);
		//Decide();
		break;
	case ATTACK:
	{
		if (TrySetActionToNextAction())
		{
			facingRight = GetCurrCommand().facingRight;
		}
		else
		{
			Decide();
		}
		break;
	}
	case WAIT:
	case SUMMON:
	case REDIRECT_ORBS:
	case TRIPLE_LUNGE_3:
		if (TrySetActionToNextAction())
		{
			facingRight = GetCurrCommand().facingRight;
		}
		else
		{
			Decide();
		}
		
		break;
	case COMBOMOVE:
		SetNextComboAction();
		break;
	case MOVE_CHASE:
		SetAction(TRIPLE_LUNGE_1);
		break;
	case TRIPLE_LUNGE_1:
		if (TrySetActionToNextAction())
		{
			facingRight = GetCurrCommand().facingRight;
		}
		else
		{
			SetAction(TRIPLE_LUNGE_WAIT_1);
		}
		
		break;
	case TRIPLE_LUNGE_WAIT_1:
		SetAction(TRIPLE_LUNGE_2);
		break;
	case TRIPLE_LUNGE_2:
		if (TrySetActionToNextAction())
		{
			facingRight = GetCurrCommand().facingRight;
		}
		else
		{
			SetAction(TRIPLE_LUNGE_WAIT_2);
		}
		break;
	case TRIPLE_LUNGE_WAIT_2:
		SetAction(TRIPLE_LUNGE_3);
		break;
	case TEST_ORBS:
	{
		//orbPool.StopChase();
		if (TrySetActionToNextAction())
		{
			facingRight = GetCurrCommand().facingRight;
		}
		else
		{
			Decide();
		}
		break;
	}
	case CREATE_ORB_CIRCLE:
	{
		//orbPool.Reset();
		break;
	}
	}
}

void Gator::HandleAction() 
{
	HitboxesOff();
	if (redirectingOrbs)
	{
		if (redirectFrame % redirectRate == 0)
		{
			if (!orbPool[0].RedirectOldestAtPlayer(targetPlayer, 20))
			{
				redirectingOrbs = false;
			}
		}
	}

	switch (action)
	{
	case SUMMON:
	{
		if (frame == 20 && slowCounter == 1)
		{
			swarmSummonGroup.Summon();
		}
		break;
	}
	case REDIRECT_ORBS:
	{
		/*if (frame % 40 == 0 && slowCounter == 1)
		{
			orbPool.RedirectOldestAtPlayer(targetPlayer, 20);
		}*/
		break;
	}
	case TRIPLE_LUNGE_1:
	case TRIPLE_LUNGE_2:
	case TRIPLE_LUNGE_3:
	case ATTACK:
		if (!actionHitPlayer)
		{
			SetHitboxes(hitBodies[action], 0);
		}
		break;
	case CREATE_ORB_CIRCLE:
	{
		//OrbAttack1_1();
		//OrbAttack1_2();

		OrbAttack2();






		break;
	}
	case CIRCLE_ORB_STUFF:
	{
		if (enemyMover.IsIdle())
		{
			V2d nodePos;// = nodeGroupA.AlwaysGetNextNode()->pos;

			//V2d playerPos = sess->GetPlayerPos();
			//sort(nodePosAVec.begin(), nodePosAVec.end(),[](const V2d &a, 
			//	const V2d &b{return length( a - playerPos) 
			SortNodePosAVec();

			bool needsToMove = true;
			for (int i = 0; i < nodePosAVec.size(); ++i)
			{
				nodePos = nodePosAVec[i];
				if (nodePos == GetPosition())
				{
					needsToMove = false;
					break;
				}

				if (nodePos != GetPosition() && !ExecuteRayCast(GetPosition(), nodePos))
				{
					break;
				}
			}

			/*while (nodePos == GetPosition() || ExecuteRayCast(GetPosition(), nodePos))
			{
				nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
			}*/

			//cout << "moving to: " << nodePos.x << ", " << nodePos.y << " gator: " << GetPosition().x << ", " << GetPosition().y << endl;
			//enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
			//enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 20);
			if (needsToMove)
			{
				enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);//20);
			}
			
		}

		if (orbPool[0].GetNumActive() == 0)
		{
			frame = 0;
		}

		OrbAttack2();
		break;
	}

	
	}


	for (int i = 0; i < NUM_ORB_POOLS; ++i)
	{
		orbPool[i].Update();
	}
}

void Gator::SortNodePosAVec()
{
	int numNodes = nodePosAVec.size();

	for (int i = 0; i < numNodes; ++i)
	{
		nodePosAVec[i] = nodeGroupA.nodeVec->at(i)->pos;
	}

	V2d playerPos = sess->GetPlayerPos(0);
	V2d temp;
	for (int i = 1; i < numNodes; ++i)
	{
		for (int j = i; j >= 1; --j)
		{
			if (length(nodePosAVec[j] - playerPos)
				< length(nodePosAVec[j - 1] - playerPos))
			{
				temp = nodePosAVec[j - 1];
				nodePosAVec[j - 1] = nodePosAVec[j];
				nodePosAVec[j] = temp;
			}
		}
	}
}


void Gator::OrbAttack1_1()
{
	//first circle

	GatorWaterOrbPool &pool = orbPool[0];

	if (frame == 0)
	{
		pool.CreateCircle(GetPosition(), 5, 200, 1, 0);
		pool.ChangeAllCircleOrbsRadiusOverTime(1.0, 32);
	}
	if (frame == 30)
	{
		pool.RotateCircle(0, .005, .02 * PI);
	}
	if (frame == 60)
	{
		/*V2d velDir = normalize(sess->GetPlayerPos(0)
			- orbPool[0].GetActiveCenter());
		orbPool[0].SetCircleVelocity(velDir * 10.0);*/
	}
	if (frame == 90)
	{
		//orbPool.RotateCircle(0);
		pool.ExpandCircle(10.0);//2.0, .1, 10.0 );
	}
	else if (frame == 120)
	{
		pool.SetCircleTimeToLive(180);
		pool.EndCircle();
		pool.Chase(&targetPlayer->position, 1.0, 5);//15);
	}
}

void Gator::OrbAttack1_2()
{
	GatorWaterOrbPool &pool = orbPool[1];

	//first circle
	if (frame == 0)
	{
		pool.CreateCircle(GetPosition(), 5, 400, 1, PI / 4);
		pool.ChangeAllCircleOrbsRadiusOverTime(2.0, 64);
	}
	if (frame == 30)
	{
		pool.RotateCircle(0, -.005, -.02 * PI);
	}
	if (frame == 60)
	{
		/*V2d velDir = normalize(sess->GetPlayerPos(0)
			- orbPool[0].GetActiveCenter());
		orbPool[1].SetCircleVelocity(velDir * 10.0);*/
	}
	if (frame == 90)
	{
		//orbPool.RotateCircle(0);
		pool.ExpandCircle(10.0);//2.0, .1, 10.0 );
	}
	else if (frame == 120)
	{
		pool.SetCircleTimeToLive(180);
		pool.EndCircle();
		pool.Chase(&targetPlayer->position, 1.0, 3);//10);
	}
}

void Gator::OrbAttack2()
{
	GatorWaterOrbPool &pool = orbPool[0];
	//first circle
	if (frame == 0)
	{
		pool.CreateCircle(GetPosition(), 5, 200, 1, 0);
		pool.SetCircleFollowPos(&currPosInfo.position);
		pool.ChangeAllCircleOrbsRadiusOverTime(1.0, 32);
	}
	if (frame == 30)
	{
		pool.RotateCircle(0, .005, .02 * PI);
	}
	if (frame == 60)
	{
		//pool.StopCircleFollow();
		//V2d velDir = normalize(sess->GetPlayerPos(0)
		//- pool.GetActiveCenter());
		//pool.SetCircleVelocity(velDir * 10.0);
	}
	if (frame == 90)
	{
		pool.RotateCircle(0);
		pool.ExpandCircle(10.0);//2.0, .1, 10.0 );
	}
	else if (frame == 120)
	{
		pool.SetCircleTimeToLive(150);
		pool.EndCircle();
		pool.Chase(&targetPlayer->position, 1.0, 15);//15);
	}
}

void Gator::StartAction()
{
	switch (action)
	{
	case MOVE_NODE_LINEAR:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;

		while (ExecuteRayCast(GetPosition(), nodePos) || nodePos == GetPosition())
		{
			nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
			
		}

		//cout << "moving to: " << nodePos.x << ", " << nodePos.y << " gator: " << GetPosition().x << ", " << GetPosition().y << endl;
		//enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
		enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);//CubicBezier(.76, .3, .83, .67), 60);
		if (!redirectingOrbs)
		{
			//orbPool.Throw(GetPosition(), nodePos, orbTypePicker.AlwaysGetNextOption());
		}
		break;
	}
	case MOVE_NODE_QUADRATIC:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeNodeQuadratic(targetPlayer->position, nodePos, CubicBezier(.76, .3, .83, .67), 60);
		if (!redirectingOrbs)
		{
			//orbPool.Throw(GetPosition(), nodePos, orbTypePicker.AlwaysGetNextOption());
		}
		
		break;
	}
	case MOVE_CHASE:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeChase(&targetPlayer->position, V2d(0, 0),
			8, .9, 90);
		//orbPool.Throw(GetPosition(), nodePos, GatorWaterOrb::OrbType::NODE_GROW);
		break;
	}
	case TRIPLE_LUNGE_1:
	case TRIPLE_LUNGE_2:
	case TRIPLE_LUNGE_3:
	{
		BossCommand bc;
		bc.action = ATTACK;
		bc.facingRight = true;

		ResetCommands();


		QueueCommand(bc);

		bc.facingRight = false;
		QueueCommand(bc);
		QueueCommand(bc);

		enemyMover.SetModeNodeLinearConstantSpeed(GetPosition() + PlayerDir() * (PlayerDist() + 200), CubicBezier(), 30);
		/*if (PlayerDist() < 400)
		{
			enemyMover.SetModeNodeLinearConstantSpeed(GetPosition() + PlayerDir() * (PlayerDist() + 200 ), CubicBezier(), 20);
		}
		else
		{
			enemyMover.SetModeNodeLinearConstantSpeed(targetPlayer->position, CubicBezier(), 40);
		}*/
		//enemyMover.SetModeNodeLinearConstantSpeed(GetPosition() + PlayerDir() * 300.0, CubicBezier(), 20);
		
		break;
	}
	case REDIRECT_ORBS:
	{
		redirectingOrbs = true;
		redirectFrame = 0;
		//actionLength[REDIRECT_ORBS] = orbPool.GetNumGrowingOrbs() * 40;
		//orbPool.Redirect(PlayerDir() * 20.0);
		break;
	}
	case TEST_ORBS:
	{
		//orbPool.GroupChase(&targetPlayer->position);
	}
	case CREATE_ORB_CIRCLE:
	{
		
		break;
	}
	case ATTACK:
	{
		break;
	}
	case TEST_POST:
	{
		GameSession *game = GameSession::GetSession();

		if (game != NULL)
		{
			oldPlayerPos = sess->GetPlayerPos(0);
			game->SetBonus(myBonus, GetPosition(), this);
		}
		break;
	}

	}
}

void Gator::SetupPostFightScenes()
{
	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();
		myBonus = game->CreateBonus("BossTest/gatorfightpost");
	}
	else
	{
		myBonus = NULL;
	}

	if (postFightScene == NULL)
	{
		postFightScene = new GatorPostFightScene;
		//postFightScene->gator = this;
		postFightScene->Init();
	}
}

void Gator::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_GATOR, "A"));
	int numNodes = nodeGroupA.nodeVec->size();
	nodePosAVec.resize(numNodes);
	nodeDistances.resize(numNodes);
	nodePosAVecSorted.resize(numNodes);
}

bool Gator::IsDecisionValid(int d)
{
	if (d == SUMMON && !swarmSummonGroup.CanSummon())
	{
		return false;
	}
	else if (d == REDIRECT_ORBS && orbPool[0].GetNumGrowingOrbs() == 0)
	{
		return false;
	}
	else if (d == TEST_ORBS && orbPool[0].GetNumGrowingOrbs() < 3)
	{
		return false;
	}

	return true;
}

bool Gator::IsEnemyMoverAction(int a)
{
	return a == MOVE_NODE_LINEAR || a == MOVE_NODE_QUADRATIC
		|| a == MOVE_CHASE || a == TRIPLE_LUNGE_1
		|| a == TRIPLE_LUNGE_2
		|| a == TRIPLE_LUNGE_3;
}

void Gator::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;

	for (int i = 0; i < NUM_ORB_POOLS; ++i)
	{
		orbPool[i].Reset();
	}
	superOrbPool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Gator::StartFight()
{
	Wait(10);
	DefaultHurtboxesOn();
	
	HitboxesOff();
	
}

void Gator::FrameIncrement()
{
	Boss::FrameIncrement();
	if (redirectingOrbs)
	{
		++redirectFrame;
	}
}

void Gator::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
	nodeGroupA.Draw(target);
	Enemy::DebugDraw(target);
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
	for (int i = 0; i < NUM_ORB_POOLS; ++i)
	{
		orbPool[i].Draw(target);
	}
	
	superOrbPool.Draw(target);
	DrawSprite(target, sprite);

	
}


void Gator::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &swarmSummonGroup)
	{
		PoiInfo *summonNode;

		summonNode = nodeGroupA.AlwaysGetNextNode();
		e->startPosInfo.SetAerial(summonNode->pos);
	}
}



//rollback

int Gator::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Gator::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	//d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

}

void Gator::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	//fireCounter = d.fireCounter;

	bytes += sizeof(MyData);
}