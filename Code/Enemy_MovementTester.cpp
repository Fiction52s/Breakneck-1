#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_MovementTester.h"
#include "Actor.h"
#include "EditorTerrain.h"

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





MovementTester::MovementTester(ActorParams *ap)
	:Enemy(EnemyType::EN_MOVEMENTTESTER, ap), shurPool( this )
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	actionLength[MOVE] = 1;
	animFactor[MOVE] = 1;

	actionLength[WAIT] = 1;
	animFactor[WAIT] = 1;

	actionLength[MOVE] = 2;
	animFactor[MOVE] = 1;

	maxWaitFrames = 60;

	hitboxInfo = new HitboxInfo;

	//BasicCircleHurtBodySetup(16);
	//BasicCircleHitBodySetup(16);

	move = ms.AddLineMovement(V2d(), V2d(), CubicBezier(), 0);
	qCurve = curveMovement.AddQuadraticMovement(V2d(), V2d(), V2d(), CubicBezier(), 0);

	

	predictCircle.setFillColor(Color::Red);
	predictCircle.setRadius(20);
	predictCircle.setOrigin(predictCircle.getLocalBounds().width / 2,
		predictCircle.getLocalBounds().height / 2);

	myCircle.setFillColor(Color::Magenta);
	myCircle.setRadius(20);
	myCircle.setOrigin(myCircle.getLocalBounds().width / 2,
		myCircle.getLocalBounds().height / 2);

	targetPlayerIndex = 0;
	accel = .8;
	maxSpeed = 15;

	ResetEnemy();
}

MovementTester::~MovementTester()
{
}

void MovementTester::ResetEnemy()
{
	testCounter = 0;
	facingRight = true;

	action = WAIT;
	waitFrames = maxWaitFrames;
	frame = 0;

	predict = false;

	moveFrames = 0;

	ms.currMovement = NULL;

	curveMovement.currMovement = NULL;

	//DefaultHitboxesOn();

	velocity = V2d();

	enemyMover.Reset();
	//SetModeChase(&(sess->GetPlayer(targetPlayerIndex)->position), V2d(50, 0), 30, 3);
	//moveType = NODE_QUADRATIC;
	//enemyMover.moveType = EnemyMover::MoveType::NODE_QUADRATIC;
	

	qCurve->SetFrameDuration(0);

	approachStartDist = -1;

	UpdateSprite();

	shurPool.Reset();
	coyBulletPool.Reset();
	tigerBulletPool.Reset();
	gatorOrbPool.Reset();
	coyShockPool.Reset();
	tigerSpinTurretPool.Reset();

	testCheck = 0;
}

void MovementTester::UpdateHitboxes()
{
	BasicUpdateHitboxes();

	/*if (facingRight)
	{
		hitBody.hitboxInfo->kbDir.x = hitboxInfos[action].kbDir.x;
	}
	else
	{
		hitBody.hitboxInfo->kbDir.x = -hitboxInfos[action].kbDir.x;
	}*/
}

void MovementTester::CalcMovement()
{
	targetPos = sess->GetPlayerPos(targetPlayerIndex);
	moveFrames = 30;
	startMovePlayerPos = targetPos;

	if (false)
	{
		move->SetFrameDuration(moveFrames);
		move->start = GetPosition();
		move->end = targetPos;
		ms.Reset();
	}
	else
	{
		
		V2d dir = normalize(targetPos - GetPosition());
		V2d other(dir.y, -dir.x);

		//V2d startMove(10, 0);

		qCurve->A = GetPosition();
		//qCurve->B = GetPosition() + startMove * 40.0;//other * 200.0;
		/*curve->C = targetPos + other * 200.0;*/
		qCurve->C = targetPos;

		double arcLength = qCurve->GetArcLength();
		moveFrames = arcLength / 10.0;
		startMoveFrames = moveFrames;

		qCurve->SetFrameDuration(moveFrames);
		
		curveMovement.Reset();
		curveMovement.InitMovementDebug();
	}
}

void MovementTester::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
	}

	V2d playerPos = sess->GetPlayerPos(targetPlayerIndex);

	if (action == MOVE && enemyMover.moveType == EnemyMover::NONE)
	{
		action = WAIT;
		waitFrames = 2;
	}

	enemyMover.currPosInfo = currPosInfo;

	if (action == WAIT && waitFrames == 0)
	{
		
		action = MOVE;
		string checkStr = "A";
		auto &nodeVec = sess->GetBossNodeVector( BossFightType::FT_BIRD, checkStr);
		int vecSize = nodeVec.size();

		int r = rand() % vecSize;

		V2d nodePos = nodeVec[r]->pos;
		V2d controlPos = normalize(nodePos - GetPosition());
		controlPos = V2d(controlPos.y, -controlPos.x);
		double len = length(nodePos - GetPosition());
		V2d playerPos = sess->GetPlayerPos(targetPlayerIndex);
		V2d dir = normalize(playerPos - GetPosition());
		double lenn = length(playerPos - GetPosition());
				
		controlPos = V2d();
		if (enemyMover.cubicMove->duration != 0)
		{
			controlPos = enemyMover.cubicMove->GetEndVelocity() * 40.0;//normalize(enemyMover.cubicMove->GetEndVelocity()) * 20.0;
		}

		controlPos = GetPosition() + controlPos;

		V2d controlPos1 = playerPos + dir * 300.0;
		/*enemyMover.SetModeNodeCubic(controlPos, controlPos1, nodePos,
			CubicBezier(), 60);*/

		/*enemyMover.SetModeNodeQuadratic(controlPos1, nodePos,
			CubicBezier(), 60);*/

		/*enemyMover.SetModeNodeLinear(nodePos,
			CubicBezier(), 60);*/

		//enemyMover.SetModeNodeJump(nodePos, 200);
		enemyMover.InitNodeDebugDraw(FT_BIRD, "A", Color::Cyan);

		/*if (!enemyMover.currPosInfo.IsAerial() && testCounter == 0 )
		{
			enemyMover.SetModeGrind(20, 60);
			testCounter = 1;
		}
		else*/
		//if( testCounter == 2 )
		{
			tigerSpinTurretPool.Throw(GetPosition(), dir);
			//coyShockPool.Throw(GetPosition(), 100, 400, 50, 60 );
			//auto *f = shurPool.Throw(GetPosition(), dir, BirdShuriken::UNBLOCKABLE_STICK);
			//if (f == NULL)
			//{
			//	shurPool.RethrowAll();
			//	enemyMover.SetModeWait(120);
			//}
			//else
			//{
			enemyMover.SetModeNodeProjectile(nodeVec[r], V2d(0, 2.0), 200);//300);
			//}
			//gatorOrbPool.Throw(GetPosition(), dir);
			
			testCounter = 0;
		}
		/*
		{
			V2d pos = nodeVec[r]->pos;
			double dist = length(pos - currPosInfo.GetPosition());
			enemyMover.SetModeSwing(pos, dist, 300);
			testCounter = 2;
		}*/


		//else if( testCounter == 0 )
		//{
		//	//enemyMover.velocity = V2d(0, 0);
		//	enemyMover.SetModeFall( 2.0, 60 );
		//	testCounter = 1;
		//}


		/*else if (testCounter == 0)
		
		

		//if (testCounter == 0)
		//{
		//	enemyMover.SetModeChase(&sess->GetPlayer(targetPlayerIndex)->position,
		//		V2d(), 20, .8, 60);
		//	enemyMover.chaseVelocity = enemyMover.lastActionEndVelocity;
		//}
		//else
		//{
		//	int r1 = rand() % 2;
		//	if (r1 == 0)
		//	{
		//		double lengthEstimate = length(nodePos - controlPos1)
		//			+ length(controlPos1 - GetPosition());
		//		double attemptVel = 20.0;
		//		double frameEstimate = lengthEstimate / attemptVel;

		//		enemyMover.SetModeNodeQuadratic(controlPos1, nodePos,
		//			CubicBezier(), frameEstimate);
		//		/*enemyMover.SetModeNodeLinear(nodePos,
		//		CubicBezier(), 60);*/
		//	}
		//	else if (r1 == 1)
		//	{
		//		/*enemyMover.SetModeNodeDoubleQuadratic(playerPos,
		//		nodePos, CubicBezier(), 60, 300.0);*/

		//		enemyMover.SetModeNodeDoubleQuadraticConstantSpeed(playerPos,
		//			nodePos, CubicBezier(), 20.0, 300.0);
		//	}
		//}		
	}
	else if (action == MOVE && enemyMover.moveType == EnemyMover::NONE )
	{
		action = WAIT;
		waitFrames = 10;
	}
}

sf::FloatRect MovementTester::GetAABB()
{
	return myCircle.getGlobalBounds();
}

void MovementTester::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

void MovementTester::CalcTargetAfterHit()
{
	sess->ForwardSimulatePlayer(targetPlayerIndex, sess->GetPlayer(targetPlayerIndex)->hitstunFrames);
	targetPos = sess->GetPlayerPos(targetPlayerIndex);
	sess->RevertSimulatedPlayer(targetPlayerIndex);
	predictCircle.setPosition(Vector2f(targetPos));
}

void MovementTester::CalcPlayerFuturePos(int frames)
{
	sess->ForwardSimulatePlayer(targetPlayerIndex, frames);
	targetPos = sess->GetPlayerPos(targetPlayerIndex);
	sess->RevertSimulatedPlayer(targetPlayerIndex);
	predictCircle.setPosition(Vector2f(targetPos));
}

void MovementTester::FrameIncrement()
{
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

void MovementTester::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	enemyMover.DebugDraw(target);
	
	//curveMovement.MovementDebugDraw(target);
}

void MovementTester::UpdatePreFrameCalculations()
{
	Actor *targetPlayer = sess->GetPlayer(targetPlayerIndex);
	if( false )
//	if (predict || targetPlayer->hitOutOfHitstunLastFrame)
	{
		CalcTargetAfterHit();
		moveFrames = targetPlayer->hitstunFrames - 1;
		move->duration = moveFrames * NUM_MAX_STEPS * 5;
		move->start = GetPosition();
		move->end = targetPos;
		ms.Reset();
		predict = false;
		if (moveFrames < 0)
		{
			moveFrames = 0;
		}

		SetHitboxes(NULL, 0);

		action = MOVE;
		frame = 0;
		//++moveFrames;

	}

	//if (moveType == CHASE)
	//{
	//	//CalcPlayerFuturePos(30);
	//}
}



void MovementTester::IHitPlayer(int index)
{
	//hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames;
}

void MovementTester::UpdateEnemyPhysics()
{
	if (enemyMover.moveType != EnemyMover::NONE)
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void MovementTester::UpdateSprite()
{
	myCircle.setPosition(GetPositionF());
}

void MovementTester::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(myCircle);
	shurPool.Draw(target);
	coyBulletPool.Draw(target);
	tigerBulletPool.Draw(target);
	gatorOrbPool.Draw(target);
	coyShockPool.Draw(target);
	tigerSpinTurretPool.Draw(target);
	//target->draw(predictCircle);
}