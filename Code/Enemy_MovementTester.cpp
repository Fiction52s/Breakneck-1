#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_MovementTester.h"
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

//PlayerComboer::PlayerComboer(Enemy *e)
//{
//	myEnemy = e;
//}
//
//void PlayerComboer::CalcTargetAfterHit(int pIndex)
//{
//	Session *sess = myEnemy->sess;
//	sess->ForwardSimulatePlayer(pIndex, sess->GetPlayer(pIndex)->hitstunFrames);
//	targetPos = sess->GetPlayerPos(pIndex);
//	sess->RevertSimulatedPlayer(pIndex);
//	predictCircle.setPosition(Vector2f(targetPos));
//}
//
//void PlayerComboer::PredictNextFrame()
//{
//	predict = true;
//}
//
//void PlayerComboer::UpdatePreFrameCalculations(int pIndex)
//{
//	Actor *targetPlayer = myEnemy->sess->GetPlayer(pIndex);
//	if (predict || targetPlayer->hitOutOfHitstunLastFrame)
//	{
//		CalcTargetAfterHit();
//		moveFrames = targetPlayer->hitstunFrames - 1;
//		move->duration = moveFrames * NUM_MAX_STEPS * 5;
//		move->start = GetPosition();
//		move->end = targetPos;
//		ms.Reset();
//		predict = false;
//		if (moveFrames < 0)
//		{
//			moveFrames = 0;
//		}
//
//		SetHitboxes(NULL, 0);
//
//		action = MOVE;
//		frame = 0;
//		//++moveFrames;
//
//	}
//}

EnemyMover::EnemyMover(Enemy *e)
{
	myEnemy = e;

	linearMove = linearMovementSeq.AddLineMovement(V2d(), V2d(), CubicBezier(), 0);
	quadraticMove = quadraticMovementSeq.AddQuadraticMovement(V2d(), V2d(), V2d(), CubicBezier(), 0);
	cubicMove = cubicMovementSeq.AddCubicMovement(V2d(), V2d(), V2d(), V2d(), CubicBezier(), 0);

	Reset();
}

void EnemyMover::Reset()
{
	predict = false;
	linearMovementSeq.currMovement = NULL;
	quadraticMovementSeq.currMovement = NULL;
	cubicMovementSeq.currMovement = NULL;
	moveType = NONE;

	linearMove->SetFrameDuration(0);
	quadraticMove->SetFrameDuration(0);
	cubicMove->SetFrameDuration(0);
}

void EnemyMover::SetModeChase(V2d *target, V2d &offset, double maxVel,
	double accel)
{
	chaseTarget = target;
	chaseOffset = offset;
	chaseMaxVel = maxVel;
	chaseAccel = accel;
	moveType = CHASE;
}

void EnemyMover::SetModeNodeLinear( V2d &nodePos, CubicBezier &cb, int frameDuration)
{
	moveType = NODE_LINEAR;
	linearMove->SetFrameDuration(frameDuration);
	linearMove->start = myEnemy->GetPosition();
	linearMove->end = nodePos;
	linearMove->InitDebugDraw();
	linearMovementSeq.Reset();
}

void EnemyMover::SetModeNodeQuadratic( V2d &controlPoint0, V2d &nodePos,
	CubicBezier &cb, int frameDuration)
{
	moveType = NODE_QUADRATIC;
	quadraticMove->SetFrameDuration(frameDuration);
	quadraticMove->A = myEnemy->GetPosition();
	quadraticMove->B = controlPoint0;
	quadraticMove->C = nodePos;
	quadraticMove->start = quadraticMove->A;
	quadraticMove->end = quadraticMove->C;
	quadraticMove->InitDebugDraw();
	quadraticMovementSeq.Reset();
}

void EnemyMover::SetModeNodeCubic(V2d &controlPoint0, V2d &controlPoint1,
	V2d &nodePos, CubicBezier &cb, int frameDuration)
{
	moveType = NODE_CUBIC;
	cubicMove->SetFrameDuration(frameDuration);
	cubicMove->A = myEnemy->GetPosition();
	cubicMove->B = controlPoint0;
	cubicMove->C = controlPoint1;
	cubicMove->D = nodePos;
	cubicMove->start = cubicMove->A;
	cubicMove->end = cubicMove->D;
	cubicMove->InitDebugDraw();
	cubicMovementSeq.Reset();
}

V2d EnemyMover::UpdatePhysics()
{

	V2d result(0, 0);

	int numPhysSteps = myEnemy->numPhysSteps;
	int slowMultiple = myEnemy->slowMultiple;

	switch (moveType)
	{
	case CHASE:
	{
		V2d movementVec = chaseVelocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		result = V2d(myEnemy->GetPosition() + movementVec);
		break;
	}
	case NODE_LINEAR:
	case NODE_QUADRATIC:
	case NODE_CUBIC:
	{
		MovementSequence *currSeq = NULL;
		if (linearMovementSeq.currMovement != NULL)
		{
			currSeq = &linearMovementSeq;
		}
		else if (quadraticMovementSeq.currMovement != NULL)
		{
			currSeq = &quadraticMovementSeq;
		}
		else if (cubicMovementSeq.currMovement != NULL)
		{
			currSeq = &cubicMovementSeq;
		}

		if (currSeq != NULL)
		{
			if (numPhysSteps == 1)
			{
				currSeq->Update(slowMultiple, 10);
			}
			else
			{
				currSeq->Update(slowMultiple);
			}

			result = currSeq->position;

			if (currSeq->currMovement == NULL)
			{
				moveType = NONE;
			}
		}

		break;
	}

	}

	return result;
}

void EnemyMover::DebugDraw(sf::RenderTarget *target)
{
	switch (moveType)
	{
	case CHASE:
	{
		break;
	}
	case NODE_LINEAR:
	case NODE_QUADRATIC:
	case NODE_CUBIC:
	{
		MovementSequence *currSeq = NULL;
		if (linearMovementSeq.currMovement != NULL)
		{
			currSeq = &linearMovementSeq;
		}
		else if (quadraticMovementSeq.currMovement != NULL)
		{
			currSeq = &quadraticMovementSeq;
		}
		else if (cubicMovementSeq.currMovement != NULL)
		{
			currSeq = &cubicMovementSeq;
		}

		currSeq->MovementDebugDraw(target);
		break;
	}

	}
}


MovementTester::MovementTester(ActorParams *ap)
	:Enemy(EnemyType::EN_MOVEMENTTESTER, ap),
	enemyMover(this)
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
	moveType = NODE_QUADRATIC;
	//enemyMover.moveType = EnemyMover::MoveType::NODE_QUADRATIC;
	

	qCurve->SetFrameDuration(0);

	approachStartDist = -1;

	UpdateSprite();

	shurPool.Reset();

	testCheck = 0;
}

void MovementTester::SetModeChase(V2d *target, V2d &offset, double maxVel,
	double accel)
{
	chaseTarget = target;
	chaseOffset = offset;
	chaseMaxVel = maxVel;
	chaseAccel = accel;
	moveType = CHASE;
}

void MovementTester::SetModeNodeLinear(V2d &nodePos, CubicBezier &cb, int frameDuration)
{
	move->SetFrameDuration(frameDuration);
	move->start = GetPosition();
	move->end = nodePos;
	move->InitDebugDraw();
	ms.Reset();
}

void MovementTester::SetModeNodeQuadratic(V2d &controlPoint0, V2d &nodePos,
	CubicBezier &cb, int frameDuration)
{
	qCurve->SetFrameDuration(frameDuration);
	qCurve->A = GetPosition();
	qCurve->B = controlPoint0;
	qCurve->C = nodePos;
	qCurve->start = qCurve->A;
	qCurve->end = qCurve->C;
	qCurve->InitDebugDraw();
	curveMovement.Reset();
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

	switch (moveType)
	{
	case CURVE:
	{
		if (action == MOVE)
		{
			double testLen = length(playerPos - startMovePlayerPos);
			if (moveFrames == 0 )
			{
				action = MOVE;
				V2d vel = qCurve->GetFrameVelocity(startMoveFrames - moveFrames);
				cout << "vel: " << vel.x << ", " << vel.y << "\n";
				vel = normalize(vel);
				vel *= 10.0;
				qCurve->B = GetPosition() + vel * 30.0;
				CalcMovement();
			}

			//if (moveFrames == 0)
			//{
			//	action = MOVE;

			//	//V2d vel = normalize( qCurve->GetPosition(1.0) - qCurve->GetPosition(.99) ) * 10.0;
			//	//moveFrames = 60;
			//	V2d vel = qCurve->GetEndVelocity();
			//	cout << "vel: " << vel.x << ", " << vel.y << "\n";
			//	vel = normalize(vel);
			//	vel *= 10.0;
			//	qCurve->B = GetPosition() + vel * 30.0;
			//	CalcMovement();
			//	//action = WAIT;
			//	//waitFrames = maxWaitFrames;
			//}
		}
		else if (action == WAIT)
		{
			if (waitFrames == 0)
			{
				action = MOVE;
				qCurve->B = (GetPosition() + playerPos) / 2.0;
				CalcMovement();
			}
		}
		break;
	}
	case CHASE:
	{
		if (sess->totalGameFrames % 60 == 0)
		{
			V2d pDir = normalize(playerPos - GetPosition() );
			shurPool.Throw(GetPosition(), pDir);
			int r = rand() % 4;
			switch (r)
			{
			case 0:
				chaseOffset = V2d(200, 0);
				break;
			case 1:
				chaseOffset = V2d(0, -200);
				break;
			case 2:
				chaseOffset = V2d(-200, 0);
				break;
			case 3:
				chaseOffset = V2d(0, 200);
				break;
			}
		}
		//chaseOffset = V2d((rand() % 200) -100 , (rand() % 200) - 100);
		targetPos = *chaseTarget + chaseOffset;//playerPos + V2d(50, 0);
		V2d diff = targetPos - GetPosition();
		V2d pDir = normalize(diff);

		velocity += pDir * chaseAccel;
		double velLen = length(velocity);
		if (velLen > chaseMaxVel )
		{
			velocity = normalize(velocity) * chaseMaxVel;
		}

		if (length(diff) < chaseMaxVel)
		{
			//currPosInfo.position = targetPos;
			//velocity = V2d(0, 0);
		}
		break;
	}
	case APPROACH:
	{
		targetPos = playerPos + V2d(50, 0);
		V2d diff = targetPos - GetPosition();
		double dist = length(diff);

		if (action == WAIT)
		{
			if (waitFrames == 0)
			{
				action = MOVE;
				approachStartDist = dist;
				moveFrames = 60;
			}
			currPosInfo.position = targetPos;
		}
		else if( action == MOVE )
		{
			if (moveFrames == 0)
			{
				action = WAIT;
				waitFrames = 60;
				velocity = V2d(0, 0);
				currPosInfo.position = targetPos;
			}
			else
			{


				V2d pDir = normalize(diff);

				double f = moveFrames / 60.0;

				V2d destPos = targetPos - pDir * approachStartDist * f;
				velocity = destPos - GetPosition();
				//int f = (60 - moveFrames) / 60;


				//double f = approachBez.GetValue( 1.0 - (moveFrames / 60.0));

				//double distPortion = approachStartDist * f;
			}
		}
		
		break;
	}
	case NODE_LINEAR:
	{
		if ( action == WAIT && waitFrames == 0)
		{
			action = MOVE;
			int r = testCheck;
			++testCheck;
			if (testCheck == 3)
				testCheck = 0;

			string checkStr = "A";
			if (r == 1)
			{
				checkStr = "B";
			}
			else if (r == 2)
			{
				checkStr = "C";
			}
			//SetModeNodeLinear(sess->GetBossNode(2, checkStr )->pos,
			//	CubicBezier(), 60);
		}
		else if (action == MOVE && ms.currMovement == NULL)
		{
			action = WAIT;
			waitFrames = maxWaitFrames;
		}
		break;
	}
	case NODE_QUADRATIC:
	{
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

			enemyMover.SetModeNodeQuadratic(controlPos1, nodePos,
				CubicBezier(), 60);

			/*if (enemyMover.cubicMove->duration !=  )
			{
				
			}
			else
			{
				enemyMover.SetModeNodeQuadratic(controlPos, nodePos,
					CubicBezier(), 60);

			}*/
			
			
		}
		else if (action == MOVE && enemyMover.quadraticMovementSeq.currMovement == NULL)
		{
			action = WAIT;
			waitFrames = 10;
		}
		break;
	}
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
}

void MovementTester::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	enemyMover.DebugDraw(target);
	curveMovement.MovementDebugDraw(target);
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

	if (moveType == CHASE)
	{
		//CalcPlayerFuturePos(30);
	}
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
		currPosInfo.SetPosition(enemyMover.UpdatePhysics());
	}
	
	return;
	switch (moveType)
	{
	case CURVE:
	{
		MovementSequence *currSeq = NULL;
		if (ms.currMovement != NULL)
		{
			currSeq = &ms;
		}
		else if (curveMovement.currMovement != NULL)
		{
			currSeq = &curveMovement;
		}


		if (currSeq != NULL)
		{
			if (numPhysSteps == 1)
			{
				currSeq->Update(slowMultiple, 10);
			}
			else
			{
				currSeq->Update(slowMultiple);
			}

			currPosInfo.SetPosition(currSeq->position);

			/*if (ms.currMovement == NULL)
			{
			DefaultHitboxesOn();
			}*/
		}
		break;
	}
	case CHASE:
	{
		/*V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;*/
		break;
	}
	case APPROACH:
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
		break;
	}
	case NODE_LINEAR:
	case NODE_QUADRATIC:
	{
		MovementSequence *currSeq = NULL;
		if (ms.currMovement != NULL)
		{
			currSeq = &ms;
		}
		else if (curveMovement.currMovement != NULL)
		{
			currSeq = &curveMovement;
		}


		if (currSeq != NULL)
		{
			if (numPhysSteps == 1)
			{
				currSeq->Update(slowMultiple, 10);
			}
			else
			{
				currSeq->Update(slowMultiple);
			}

			currPosInfo.SetPosition(currSeq->position);
		}
		break;
	}
		
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
	//target->draw(predictCircle);
}