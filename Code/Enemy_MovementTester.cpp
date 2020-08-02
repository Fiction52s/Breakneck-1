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





MovementTester::MovementTester(ActorParams *ap)
	:Enemy(EnemyType::EN_MOVEMENTTESTER, ap)
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

	moveType = MoveType::HOMING;

	UpdateSprite();
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
			if (testLen > 100 && moveFrames < startMoveFrames - 10)
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
	case HOMING:
	{
		targetPos = playerPos + V2d(50, 0);
		V2d diff = targetPos - GetPosition();
		V2d pDir = normalize(diff);

		

		velocity += pDir * 10.0;
		double velLen = length(velocity);
		if (velLen > maxSpeed)
		{
			velocity = normalize(velocity) * maxSpeed;
		}

		if (length(diff) < maxSpeed)
		{
			currPosInfo.position = targetPos;
			velocity = V2d(0, 0);
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
	curveMovement.MovementDebugDraw(target);
}

void MovementTester::UpdatePreFrameCalculations()
{
	Actor *targetPlayer = sess->GetPlayer(targetPlayerIndex);
	if( false )
//	if (predict || targetPlayer->hitOutOfHitstunLastFrame)
	{
		CalcTargetAfterHit();
		moveFrames = targetPlayer->hitstunFrames - 1;//(hitBody.hitboxInfo->hitstunFrames - 1);
		move->duration = moveFrames * NUM_MAX_STEPS * 5;
		move->start = GetPosition();
		move->end = targetPos;
		ms.Reset();
		predict = false;
		//int nextAction = actionQueue[actionQueueIndex].action + 1;
		//moveFrames -= actionLength[nextAction] * animFactor[nextAction] - 10;
		if (moveFrames < 0)
		{
			moveFrames = 0;
		}

		SetHitboxes(NULL, 0);

		action = MOVE;
		frame = 0;
		//++moveFrames;

	}

	if (moveType == HOMING)
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
	case HOMING:
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
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
	//target->draw(predictCircle);
}