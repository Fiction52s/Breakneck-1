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

	StageSetup(8, 2);

	ts_move = GetSizedTileset("Bosses/Gator/dominance_384x384.png");
	ts_bite = GetSizedTileset("Bosses/Gator/gator_dash_512x320.png");

	actionLength[SUMMON] = 60;
	actionLength[REDIRECT_ORBS] = 60;

	actionLength[TRIPLE_LUNGE_WAIT_1] = 30;
	actionLength[TRIPLE_LUNGE_WAIT_2] = 20;

	actionLength[BITE_ATTACK] = 20;
	animFactor[BITE_ATTACK] = 1;

	actionLength[BITE_STUCK] = 30;
	animFactor[BITE_STUCK] = 1;

	actionLength[ATTACK] = 10;

	hitboxStartFrame[ATTACK] = 0;
	hitboxStartFrame[TRIPLE_LUNGE_1] = 0;
	hitboxStartFrame[TRIPLE_LUNGE_2] = 0;
	hitboxStartFrame[TRIPLE_LUNGE_3] = 0;

	actionLength[TEST_ORBS] = 180;
	animFactor[TEST_ORBS] = 1;

	actionLength[TIME_ORB_ATTACK] = 90;
	animFactor[TIME_ORB_ATTACK] = 1;

	postFightScene = NULL;

	redirectRate = 20;

	orbTypePicker.AddActiveOption(0, 2);
	orbTypePicker.AddActiveOption(1, 2);

	stageMgr.AddActiveOptionToStages(0, MOVE_WANTS_TO_BITE, 2);
	stageMgr.AddActiveOptionToStages(0, TIME_ORB_ATTACK, 2);
	stageMgr.AddActiveOptionToStages(0, MOVE_TO_ORB_ATTACK_1, 2);
	stageMgr.AddActiveOptionToStages(0, MOVE_TO_ORB_ATTACK_2, 2);
	stageMgr.AddActiveOptionToStages(0, MOVE_TO_ORB_ATTACK_3, 2);

	//stageMgr.AddActiveOption(0, MOVE_TO_ORB_ATTACK, 2);
	//stageMgr.AddActiveOption(0, CIRCLE_ORB_STUFF, 2);
	//stageMgr.AddActiveOption(0, MOVE_WANTS_TO_BITE, 2);


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
	SetupHitboxes(CHASE_ATTACK, "dominance");
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

	HitboxInfo::SetupHitboxLevelInfo(j["dominance"], hitboxInfos[CHASE_ATTACK]);
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

	currMoveSpeed = 10;

	rayCastInfo.tree = sess->terrainTree;

	for (int i = 0; i < NUM_ORB_POOLS; ++i)
	{
		orbPool[i].Reset();
	}
	timeOrbPool.Reset();
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

	moveMode = MM_STOP;

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

void Gator::MoveRandomly()
{
	if (enemyMover.IsIdle())
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;

		while (nodePos == GetPosition() || ExecuteRayCast(GetPosition(), nodePos))
		{
			nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		}

		double xDiff = nodePos.x - GetPosition().x;
		if (xDiff > 0)
		{
			facingRight = true;
		}
		else if (xDiff < 0)
		{
			facingRight = false;
		}

		enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), currMoveSpeed);
	}
}

void Gator::MoveTowardsPlayer()
{
	if (enemyMover.IsIdle())
	{
		V2d nodePos;

		SortNodePosAVec(sess->GetPlayerPos(0));

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

		if (needsToMove)
		{
			double xDiff = nodePos.x - GetPosition().x;
			if ( xDiff > 0)
			{
				facingRight = true;
			}
			else if (xDiff < 0)
			{
				facingRight = false;
			}
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), currMoveSpeed);//10);//20);
		}
	}
}

void Gator::MoveAwayFromPlayer()
{
	if (enemyMover.IsIdle())
	{
		V2d nodePos;

		SortNodePosAVec(sess->GetPlayerPos(0));

		bool needsToMove = true;
		for (int i = nodePosAVec.size() - 1; i >= 0; --i)
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

		if (needsToMove)
		{
			double xDiff = nodePos.x - GetPosition().x;
			if (xDiff > 0)
			{
				facingRight = true;
			}
			else if (xDiff < 0)
			{
				facingRight = false;
			}
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), currMoveSpeed);//20);
		}

	}
}

void Gator::ThrowTimeOrbTowardsPlayer()
{

}

void Gator::ThrowTimeOrbRandomly()
{

}

void Gator::UpdateMove()
{
	switch (moveMode)
	{
	case MM_STOP:
	{
		break;
	}
	case MM_APPROACH:
	{
		MoveTowardsPlayer();
		break;
	}
	case MM_FLEE:
	{
		MoveAwayFromPlayer();
		break;
	}
	case MM_RANDOM:
	{
		MoveRandomly();
		break;
	}

	}
}

void Gator::ActionEnded()
{
	switch (action)
	{
	case TIME_ORB_ATTACK:
	{
		SetAction(CHASE_ATTACK);
		//Decide();
		break;
	}
	case MOVE_TO_ORB_ATTACK_1:
	{
		SetAction(ORB_ATTACK_1);
		break;
	}
	case MOVE_TO_ORB_ATTACK_2:
	{
		SetAction(ORB_ATTACK_2);
		break;
	}
	case MOVE_TO_ORB_ATTACK_3:
	{
		SetAction(ORB_ATTACK_3);
		break;
	}
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
	case CHASE_ATTACK:
		Decide();
		//SetAction(BITE_ATTACK);
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
	case MOVE_WANTS_TO_BITE:
	{
		SetAction(MOVE_WANTS_TO_BITE);
		//Decide();
		//Wait(10);
		break;
	}
	case MOVE_WANTS_TO_TRIPLE_RUSH:
	{
		SetAction(MOVE_WANTS_TO_TRIPLE_RUSH);
		break;
	}
	case BITE_ATTACK:
	{
		SetAction(BITE_STUCK);
		break;
	}
	case BITE_STUCK:
	{
		SetAction(BITE_RECOVER);
		break;
	}
	case BITE_RECOVER:
	{
		Wait(10);
		break;
	}
	}
}

void Gator::StartAction()
{
	switch (action)
	{
	case TIME_ORB_ATTACK:
	{
		//TimeOrbAttack1();
		break;
	}
	case MOVE_TO_ORB_ATTACK_1:
	{
		currMoveSpeed = 20;
		MoveRandomly();
		//moveMode = MM_RANDOM;
		break;
	}
	case MOVE_TO_ORB_ATTACK_2:
	{
		currMoveSpeed = 20;
		MoveRandomly();
		//moveMode = MM_RANDOM;
		break;
	}
	case MOVE_TO_ORB_ATTACK_3:
	{
		currMoveSpeed = 20;
		MoveRandomly();
		//moveMode = MM_RANDOM;
		break;
	}
	case MOVE_NODE_LINEAR:
	{
		MoveTowardsPlayer();
		break;

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
			//V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
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
	case CHASE_ATTACK:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeChase(&targetPlayer->position, V2d(0, 0),
			12, 1.5, 180);
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

		//+200
		enemyMover.SetModeNodeLinearConstantSpeed(GetPosition() + PlayerDir() * (PlayerDist() + 0), CubicBezier(), 30);
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
	case ORB_ATTACK_2:
	{
		//MoveTowardsPlayer();
		//moveMode = MM_RANDOM;
		//moveMode = MM_APPROACH;
		break;
	}
	case BITE_ATTACK:
	{
		double dist = 4000;
		bool hit = ExecuteRayCast(GetPosition(), GetPosition() + PlayerDir() * dist);
		assert(hit);
		V2d rayHitPos = rayCastInfo.GetRayHitPos();
		biteRushDir = normalize(rayHitPos - GetPosition());
		V2d endMovePos = GetPosition() + biteRushDir
			* (length(rayHitPos - GetPosition()) - 150.0);

		if (biteRushDir.x < 0)
		{
			facingRight = false;
		}
		else if (biteRushDir.x > 0)
		{
			facingRight = true;
		}
		else if (biteRushDir.y == 1)
		{
			facingRight = true;
		}
		else if (biteRushDir.y == -1)
		{
			facingRight = false;
		}

		enemyMover.SetModeNodeLinearConstantSpeed(endMovePos, CubicBezier(), 30);
		
		break;
	}
	case MOVE_WANTS_TO_BITE:
	{
		SortNodePosBVec(sess->GetPlayerPos(0));
		GatorWaterOrb *orb = timeOrbPool.Throw(GetPosition(), nodePosBVec[0], 1);
		orb->ChangeRadiusOverTime(.5, 150);
		orb->SetTimeToLive(360);
		MoveTowardsPlayer();
		break;
	}
	case MOVE_WANTS_TO_TRIPLE_RUSH:
	{
		MoveTowardsPlayer();
		break;
	}
	case BITE_RECOVER:
	{
		if (enemyMover.IsIdle())
		{
			V2d nodePos;

			SortNodePosAVec( GetPosition() );

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

			if (needsToMove)
			{
				double xDiff = nodePos.x - GetPosition().x;
				if (xDiff > 0)
				{
					facingRight = true;
				}
				else if (xDiff < 0)
				{
					facingRight = false;
				}
				enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 20);//10);//20);
			}
		}
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
	case CHASE_ATTACK:
		if (!actionHitPlayer)
		{
			SetHitboxes(hitBodies[action], 0);
		}
		break;
	case TIME_ORB_ATTACK:
	{
		/*if (frame > 0 && orbPool[0].GetNumActive() == 0
			&& orbPool[1].GetNumActive() == 0
			&& orbPool[2].GetNumActive() == 0)
		{
			Decide();
			break;
		}*/
		TimeOrbAttack1();
		break;
	}
	case ORB_ATTACK_1:
	{
		if (frame > 0 && orbPool[0].GetNumActive() == 0
			&& orbPool[1].GetNumActive() == 0
			&& orbPool[2].GetNumActive() == 0)
		{
			Decide();
			break;
		}
		OrbAttack1();
		break;
	}
	case ORB_ATTACK_2:
	{
		if ( frame > 0 && orbPool[0].GetNumActive() == 0
			&& orbPool[1].GetNumActive() == 0
			&& orbPool[2].GetNumActive() == 0)
		{
			Decide();
			break;
		}
		OrbAttack2();
		break;
	}
	case ORB_ATTACK_3:
	{
		if (frame > 0 && orbPool[0].GetNumActive() == 0
			&& orbPool[1].GetNumActive() == 0
			&& orbPool[2].GetNumActive() == 0)
		{
			Decide();
			break;
		}
		OrbAttack3();
		break;
	}
	case MOVE_NODE_LINEAR:
	{
		if (PlayerDist() < 600)
		{
			if (!ExecuteRayCast(GetPosition(), sess->GetPlayerPos(0)))
			{
				SetAction(BITE_ATTACK);
			}
		}
		break;
	}
	case MOVE_WANTS_TO_BITE:
	{
		if (PlayerDist() < 900)
		{
			if (!ExecuteRayCast(GetPosition(), sess->GetPlayerPos(0)))
			{
				SetAction(BITE_ATTACK);
			}
		}
		break;
	}
	case MOVE_WANTS_TO_TRIPLE_RUSH:
	{
		if (PlayerDist() < 600)
		{
			if (!ExecuteRayCast(GetPosition(), sess->GetPlayerPos(0)))
			{
				SetAction(TRIPLE_LUNGE_1);
			}
		}
		break;
	}

	
	}


	//UpdateMove();

	for (int i = 0; i < NUM_ORB_POOLS; ++i)
	{
		orbPool[i].Update();
	}

	timeOrbPool.Update();
}

void Gator::SortNodePosAVec( V2d &sortPos)
{
	int numNodes = nodePosAVec.size();

	for (int i = 0; i < numNodes; ++i)
	{
		nodePosAVec[i] = nodeGroupA.nodeVec->at(i)->pos;
	}

	//V2d playerPos = sess->GetPlayerPos(0);
	V2d temp;
	for (int i = 1; i < numNodes; ++i)
	{
		for (int j = i; j >= 1; --j)
		{
			if (length(nodePosAVec[j] - sortPos)
				< length(nodePosAVec[j - 1] - sortPos))
			{
				temp = nodePosAVec[j - 1];
				nodePosAVec[j - 1] = nodePosAVec[j];
				nodePosAVec[j] = temp;
			}
		}
	}
}

void Gator::SortNodePosBVec(V2d &sortPos)
{
	int numNodes = nodePosBVec.size();

	for (int i = 0; i < numNodes; ++i)
	{
		nodePosBVec[i] = nodeGroupB.nodeVec->at(i)->pos;
	}

	//V2d playerPos = sess->GetPlayerPos(0);
	V2d temp;
	for (int i = 1; i < numNodes; ++i)
	{
		for (int j = i; j >= 1; --j)
		{
			if (length(nodePosBVec[j] - sortPos)
				< length(nodePosBVec[j - 1] - sortPos))
			{
				temp = nodePosBVec[j - 1];
				nodePosBVec[j - 1] = nodePosBVec[j];
				nodePosBVec[j] = temp;
			}
		}
	}
}

void Gator::OrbAttack1()
{
	if (frame == 0)
	{
		int r = rand() % 360;
		orbAttack1Angle = r / 180.0 * PI;
	}
	OrbAttack1_1();
	OrbAttack1_2();
	//OrbAttack1_3();
}

void Gator::OrbAttack1_1()
{
	//first circle

	GatorWaterOrbPool &pool = orbPool[0];

	if (frame == 0)
	{
		//int r = rand() % 360;
		//double startAngle = r / 180.0 * PI;
		pool.CreateCircle(GetPosition(), 5, 20, 1, orbAttack1Angle + (PI *2) / 5);
		pool.ExpandCircleToRadius(200, 10);
		pool.SetCircleFollowPos(&currPosInfo.position);
		pool.ChangeAllCircleOrbsRadiusOverTime(1.0, 24);
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
		//moveMode = MM_STOP;
		//enemyMover.SetMoveType(EnemyMover::NONE);
		//orbPool.RotateCircle(0);
		pool.ExpandCircle(10);//2.0, .1, 10.0 );
	}
	/*else if (frame == 119)
	{
		pool.ExpandCircle(.0);
	}*/
	else if (frame == 120)
	{
		pool.SetCircleTimeToLive(360);
		pool.EndCircle();
		pool.Chase(&targetPlayer->position, 1.0, 2);//5);//15);
	}
}

void Gator::OrbAttack1_2()
{
	GatorWaterOrbPool &pool = orbPool[1];

	//first circle
	if (frame == 0)
	{
		pool.CreateCircle(GetPosition(), 5, 50, 1, orbAttack1Angle);
		pool.ExpandCircleToRadius(400, 20);
		pool.SetCircleFollowPos(&currPosInfo.position);
		pool.ChangeAllCircleOrbsRadiusOverTime(2.0, 48);
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
		pool.ExpandCircle(14);//10.0);//2.0, .1, 10.0 );
	}
	else if (frame == 120)
	{
		pool.SetCircleTimeToLive(360);
		pool.EndCircle();
		pool.Chase(&targetPlayer->position, 1.0, 1);//10);
	}
}

void Gator::OrbAttack1_3()
{
	//first circle

	GatorWaterOrbPool &pool = orbPool[2];

	if (frame == 0)
	{
		int r = rand() % 360;
		double startAngle = r / 180.0 * PI;
		pool.CreateCircle(GetPosition(), 5, 100, 1, startAngle);
		pool.SetCircleFollowPos(&currPosInfo.position);
		pool.ChangeAllCircleOrbsRadiusOverTime(1.0, 16);
	}
	if (frame == 30)
	{
		pool.RotateCircle(0, -.005, -.04 * PI);
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
		//pool.ExpandCircle(1.0, 200);//10.0);//2.0, .1, 10.0 );
		pool.ChangeAllCircleOrbsRadiusOverTime(.2, 48);
	}
	else if (frame == 120)
	{
		pool.StopCircleFollow();
		pool.SetCircleTimeToLive(180);
		pool.RotateCircle(pool.circleRotateSpeed, .1, .06 * PI);
		//pool.EndCircle();
		V2d dir = normalize(sess->GetPlayerPos(0) - pool.circleCenter);
		//pool.SetCircleVelocity(dir * 15.0);
		//pool.CircleChase(&sess->GetPlayer(0)->position, .2, 15);
		//pool.Chase(&targetPlayer->position, 1.0, 12);//15);
	}
}

void Gator::OrbAttack2()
{
	GatorWaterOrbPool &pool = orbPool[0];

	//first circle
	if (frame == 0)
	{
		pool.CreateCircle(GetPosition(), 5, 50, 1, 0);
		pool.ExpandCircleToRadius(250, 10);
		pool.SetCircleFollowPos(&currPosInfo.position);
		pool.ChangeAllCircleOrbsRadiusOverTime(1.0, 32);
		//pool.RotateCircle(0, .001, .008 * PI);

		double rotateDir = 1.0;
		/*int cw = rand() % 2;
		if( cw == 0 )
		{
			rotateDir = -rotateDir;
		}*/

		pool.RotateCircle(0, .005 * rotateDir, .02 * PI * rotateDir);
	}
	if (frame == 30)
	{
		
	}
	if (frame == 60)
	{
		//pool.StopCircleFollow();
		//V2d velDir = normalize(sess->GetPlayerPos(0)
		//- pool.GetActiveCenter());
		//pool.SetCircleVelocity(velDir * 10.0);
	}
	if (frame == 100)
	{
		pool.RotateCircle(0);
		pool.ExpandCircle(20.0);//2.0, .1, 10.0 );
	}
	else if (frame == 120)
	{
		pool.SetCircleTimeToLive(180);
		pool.EndCircle();
		pool.Chase(&targetPlayer->position, .7/*1.0*/, 20);//15);
	}
	//else if (frame == 120 + 90)
	//{
	//	pool.Chase(&targetPlayer->position, .2, 20);//15);
	//}
}

void Gator::OrbAttack3()
{
	GatorWaterOrbPool &pool = orbPool[0];
	//first circle

	int throwFrame = 60;

	double expandRate = 12;
	if (frame == 0)
	{
		int r = rand() % 360;
		double startAngle = r / 180.0 * PI;
		pool.CreateCircle(GetPosition(), 5, 50, 1, startAngle);
		pool.ExpandCircleToRadius(200, 10);
		pool.SetCircleFollowPos(&currPosInfo.position);
		pool.ChangeAllCircleOrbsRadiusOverTime(1.0, 32);
		pool.RotateCircle(0, .0002, .005 * PI);
	}
	if (frame == throwFrame)
	{
		pool.StopCircleFollow();
		V2d velDir = normalize(sess->GetPlayerPos(0)
			- pool.circleCenter);
		double speed = 15;//15
		pool.SetCircleVelocity(velDir * speed);
		pool.CircleChase(&sess->GetPlayer(0)->position, .5, speed);
		
		//pool.
		
	}
	if (frame == throwFrame + 30)
	{
		pool.ExpandCircleToRadius(500, expandRate);//2, .1, 10);
	}
	else if (frame == throwFrame + 60)
	{
		pool.ExpandCircleToRadius(200, -expandRate);//-2, -.1, -10);
	}
	else if (frame == throwFrame + 90)
	{
		pool.ExpandCircleToRadius(500, expandRate);
	}
	else if (frame == throwFrame + 120)
	{
		pool.ExpandCircleToRadius(50, -expandRate);//-2, -.1, -10);
		pool.SetCircleVelocity(V2d());
		pool.StopCircleChase();
		pool.SetCircleTimeToLive(90);//60);
	}
}

void Gator::OrbAttack4()
{
	GatorWaterOrbPool &pool = orbPool[0];
	//first circle

	int throwFrame = 90;

	if (frame == 0)
	{
		int r = rand() % 360;
		double startAngle = r / 180.0 * PI;
		pool.CreateCircle(GetPosition(), 5, 50, 1, startAngle);
		pool.ExpandCircleToRadius(150, 10);
		pool.SetCircleFollowPos(&currPosInfo.position);
		pool.ChangeAllCircleOrbsRadiusOverTime(1.0, 32);
		pool.RotateCircle(0, .004, .06 * PI);
	}
	if (frame == throwFrame)
	{
		pool.StopCircleFollow();
		V2d velDir = normalize(sess->GetPlayerPos(0)
			- pool.circleCenter);
		pool.SetCircleVelocity(velDir * 15.0);
		pool.RotateCircle( pool.circleRotateSpeed, -.01, 0 );
		//pool.CircleChase(&sess->GetPlayer(0)->position, 1.0, 15);

		//pool.

	}
	if (frame == throwFrame + 30)
	{
		//pool.SetCircleVelocity(V2d());
		pool.ExpandCircle(20);
		pool.SetCircleTimeToLive(40);
		//pool.ExpandCircleToRadius(500, 10);//2, .1, 10);
	}
	if (frame == throwFrame + 60)
	{
		//pool.ExpandCircleToRadius(50, -10);//-2, -.1, -10);
		//pool.SetCircleVelocity(V2d());
		//pool.StopCircleChase();
		//pool.RotateCircle(0);
		//pool.ExpandCircle(10.0);//2.0, .1, 10.0 );
	}
	else if (frame == throwFrame + 90)
	{


		


		//pool.EndCircle();
		//pool.Chase(&targetPlayer->position, 1.0, 15);//15);
	}
}

void Gator::OrbAttack5()
{
	GatorWaterOrbPool &pool = orbPool[0];
	//first circle

	int throwFrame = 90;

	if (frame == 0)
	{
		int r = rand() % 360;
		double startAngle = r / 180.0 * PI;
		pool.CreateCircle(GetPosition(), 5, 50, 1, startAngle);
		pool.ExpandCircleToRadius(150, 10);
		pool.SetCircleFollowPos(&currPosInfo.position);
		pool.ChangeAllCircleOrbsRadiusOverTime(1.0, 32);
		pool.RotateCircle(0, .004, .06 * PI);
	}
	if (frame == throwFrame)
	{
		pool.StopCircleFollow();
		V2d velDir = normalize(sess->GetPlayerPos(0)
			- pool.circleCenter);
		pool.SetCircleVelocity(velDir * 15.0);
		pool.RotateCircle(pool.circleRotateSpeed, -.01, 0);
		//pool.CircleChase(&sess->GetPlayer(0)->position, 1.0, 15);

		//pool.

	}
	if (frame == throwFrame + 30)
	{
		//pool.SetCircleVelocity(V2d());
		pool.ExpandCircle(20);
		pool.SetCircleTimeToLive(40);
		//pool.ExpandCircleToRadius(500, 10);//2, .1, 10);
	}
	if (frame == throwFrame + 60)
	{
		//pool.ExpandCircleToRadius(50, -10);//-2, -.1, -10);
		//pool.SetCircleVelocity(V2d());
		//pool.StopCircleChase();
		//pool.RotateCircle(0);
		//pool.ExpandCircle(10.0);//2.0, .1, 10.0 );
	}
	else if (frame == throwFrame + 90)
	{





		//pool.EndCircle();
		//pool.Chase(&targetPlayer->position, 1.0, 15);//15);
	}
}

void Gator::TimeOrbAttack1()
{
	GatorWaterOrbPool &pool = orbPool[0];
	//first circle

	int throwFrame = 90;


	//orbPool[0].Throw(GetPosition(), sess->GetPlayerPos(0), GatorWaterOrb::NODE_GROW_SLOW);

	if (frame == 0)
	{
		int r = rand() % 360;
		double startAngle = 0;//r / 180.0 * PI;
		pool.Reset();
		pool.CreateCircle(GetPosition(), 1, 0, 1, startAngle, 
			GatorWaterOrb::OrbType::NODE_GROW_SLOW);
		//pool.ExpandCircleToRadius(800, 10);
		//pool.SetCircleFollowPos(&currPosInfo.position);
		pool.ChangeAllCircleOrbsRadiusOverTime(20.0, 800);
		pool.SetCircleTimeToLive(160);
		//pool.RotateCircle(0, .0002, .01 * PI);
		//pool.RotateCircle(0, .004, .06 * PI);
	}
	if (frame == throwFrame)
	{
		//pool.StopCircleFollow();
		//V2d velDir = normalize(sess->GetPlayerPos(0)
		//	- pool.circleCenter);
		//pool.SetCircleVelocity(velDir * 15.0);
		//pool.RotateCircle(pool.circleRotateSpeed, -.01, 0);
		//pool.CircleChase(&sess->GetPlayer(0)->position, 1.0, 15);

		//pool.

	}
	if (frame == throwFrame + 30)
	{
		//pool.SetCircleVelocity(V2d());
		//pool.ExpandCircle(20);
		pool.SetCircleTimeToLive(40);
		//pool.ExpandCircleToRadius(500, 10);//2, .1, 10);
	}
	if (frame == throwFrame + 60)
	{
		//pool.ExpandCircleToRadius(50, -10);//-2, -.1, -10);
		//pool.SetCircleVelocity(V2d());
		//pool.StopCircleChase();
		//pool.RotateCircle(0);
		//pool.ExpandCircle(10.0);//2.0, .1, 10.0 );
	}
	else if (frame == throwFrame + 90)
	{





		//pool.EndCircle();
		//pool.Chase(&targetPlayer->position, 1.0, 15);//15);
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
	nodeGroupB.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_GATOR, "B"));
	int numANodes = nodeGroupA.nodeVec->size();
	nodePosAVec.resize(numANodes);

	int numBNodes = nodeGroupB.nodeVec->size();
	nodePosBVec.resize(numBNodes);
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
		|| a == CHASE_ATTACK || a == TRIPLE_LUNGE_1
		|| a == TRIPLE_LUNGE_2
		|| a == TRIPLE_LUNGE_3
		|| a == BITE_ATTACK
		|| a == MOVE_WANTS_TO_BITE
		|| a == MOVE_WANTS_TO_TRIPLE_RUSH
		|| a == BITE_RECOVER
		|| a == MOVE_TO_ORB_ATTACK_1
		|| a == MOVE_TO_ORB_ATTACK_2
		|| a == MOVE_TO_ORB_ATTACK_3; //bite attack temp here
}

void Gator::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;

	for (int i = 0; i < NUM_ORB_POOLS; ++i)
	{
		orbPool[i].Reset();
	}
	timeOrbPool.Reset();
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
	sprite.setRotation(0);
	switch (action)
	{
	case BITE_STUCK:
	case BITE_ATTACK:
	{
		ts_bite->SetSpriteTexture(sprite);
		ts_bite->SetSubRect(sprite, 0, !facingRight);

		double angle = GetVectorAngleCW(biteRushDir);
		if (facingRight)
		{
			sprite.setRotation(angle / PI * 180.0);
		}
		else
		{
			sprite.setRotation(angle / PI * 180.0 + 180);
		}
		//GetVectorAngleCCW(biteRushDir);
		//angle = angle / PI * 180.0;
		
		break;
	}
	case WAIT:
	case MOVE:
	case COMBOMOVE:
	default:
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

	timeOrbPool.Draw(target);
	
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

int Gator::GetNumSimulationFramesRequired()
{
	return 0;
}