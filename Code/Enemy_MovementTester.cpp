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

EnemyMover::EnemyMover()
{
	sess = Session::GetSession();

	nodeCircles = NULL;

	linearMove = linearMovementSeq.AddLineMovement(V2d(), V2d(), CubicBezier(), 0);
	quadraticMove = quadraticMovementSeq.AddQuadraticMovement(V2d(), V2d(), V2d(), CubicBezier(), 0);
	cubicMove = cubicMovementSeq.AddCubicMovement(V2d(), V2d(), V2d(), V2d(), CubicBezier(), 0);

	debugCircles = new CircleGroup(20, 40, Color::Red, 6);

	doubleQuadtraticMove0 = doubleQuadraticMovementSeq.AddQuadraticMovement(V2d(), V2d(), V2d(), CubicBezier(), 0);
	doubleQuadtraticMove1 = doubleQuadraticMovementSeq.AddQuadraticMovement(V2d(), V2d(), V2d(), CubicBezier(), 0);

	SetRectColor(swingQuad, Color::Red);

	Reset();
}

EnemyMover::~EnemyMover()
{
	if (nodeCircles != NULL)
		delete nodeCircles;

	delete debugCircles;
}

void EnemyMover::Reset()
{
	predict = false;
	linearMovementSeq.currMovement = NULL;
	quadraticMovementSeq.currMovement = NULL;
	cubicMovementSeq.currMovement = NULL;
	doubleQuadraticMovementSeq.currMovement = NULL;
	moveType = NONE;
	lastActionEndVelocity = V2d(0, 0);

	linearMove->SetFrameDuration(0);
	quadraticMove->SetFrameDuration(0);
	cubicMove->SetFrameDuration(0);

	debugCircles->HideAll();
}

void EnemyMover::UpdateSwingDebugDraw()
{
	V2d currPos = currPosInfo.GetPosition();
	V2d other = normalize( swingAnchor - currPos );
	other = V2d(other.y, -other.x);
	double width = 20;
	V2d A = currPos + other * width;
	V2d B = swingAnchor + other * width;
	V2d C = swingAnchor - other * width;
	V2d D = currPos - other * width;

	swingQuad[0].position = Vector2f(A);
	swingQuad[1].position = Vector2f(B);
	swingQuad[2].position = Vector2f(C);
	swingQuad[3].position = Vector2f(D);
}

int EnemyMover::GetLinearFrameEstimate(double attemptSpeed,
	V2d &start, V2d &end)
{
	double lengthEstimate = length(start - end);
	double frameEstimate = lengthEstimate / attemptSpeed;

	return frameEstimate;
}

int EnemyMover::GetQuadraticFrameEstimate(double attemptSpeed,
	V2d &start, V2d &cp0, V2d &end)
{
	double lengthEstimate = length(start - cp0) + length( end - cp0 );
	double frameEstimate = lengthEstimate / attemptSpeed;

	return frameEstimate;
}

int EnemyMover::GetCubicFrameEstimate(double attemptSpeed,
	V2d &start, V2d &cp0, V2d &cp1, V2d &end)
{
	double lengthEstimate = length(start - cp0) + length(cp1 - cp0)
		+ length( end - cp1 );
	double frameEstimate = lengthEstimate / attemptSpeed;

	return frameEstimate;
}

int EnemyMover::GetDoubleQuadraticFrameEstimate(
	double attemptSpeed,
	V2d &start, V2d &cp0, V2d &end,
	double spreadFactor)
{
	double lengthEstimate = length(start - cp0) + length(end - cp0);
	double frameEstimate = lengthEstimate / attemptSpeed;

	return frameEstimate;
}

void EnemyMover::InitNodeDebugDraw(int fightType,
	const std::string &str, sf::Color c )
{
	auto &a = sess->GetBossNodeVector(fightType, str);
	int numNodes = a.size();
	
	if (nodeCircles != NULL)
		delete nodeCircles;

	nodeCircles = new CircleGroup(numNodes, 20, c, 6);
	for (int i = 0; i < numNodes; ++i)
	{
		nodeCircles->SetPosition(i, Vector2f(a[i]->pos));
	}
	nodeCircles->ShowAll();
}

void EnemyMover::SetModeFall(double grav,int frames)
{
	projectileGrav = V2d(0, grav);
	actionFrames = frames;
	moveType = FALL;
}

void EnemyMover::SetModeWait(int frames)
{
	moveType = WAIT;
	actionFrames = frames;
}

void EnemyMover::SetModeSwing(V2d &p_swingAnchor, double p_wireLength,
	int frames )
{
	swingAnchor = p_swingAnchor;
	wireLength = p_wireLength;
	actionFrames = frames;
	moveType = SWING;
}

void EnemyMover::SetModeGrind( double speed, int frames)
{
	moveType = GRIND;
	actionFrames = frames;
	grindSpeed = speed;
}

void EnemyMover::SetModeNodeProjectile(
	V2d &nodePos, V2d &grav, double height)
{
	
	V2d currPos = currPosInfo.GetPosition();
	currPosInfo.SetAerial(currPos);

	double peak;
	if (currPos.y < nodePos.y)
	{
		peak = currPos.y - height;
	}
	else
	{
		peak = nodePos.y - height;
	}

	double hUp = currPos.y - peak;
	double hDown = nodePos.y - peak;
	double g = grav.y;

	double yVelStart = -sqrt(2 * hUp * g);
	double tUp = (-yVelStart) / g;
	double tDown = sqrt(2 * hDown / g);
	double totalTime = tUp + tDown;
	double xDist = nodePos.x - currPos.x;
	double xVel = xDist / totalTime;
	velocity = V2d(xVel, yVelStart);
	projectileGrav = grav;
	//debugCircles->SetPosition(0, Vector2f((currPos.x + nodePos.x)/ 2.0, peak));
	//debugCircles->SetVisible(0, true);
	actionFrames = totalTime;
	moveType = NODE_PROJECTILE;
	targetPI = NULL;
	targetPos = nodePos;
}

void EnemyMover::SetModeNodeProjectile(
	PoiInfo *pi, V2d &grav, double height)
{	
	SetModeNodeProjectile(pi->pos, grav, height);
	targetPI = pi;
}

void EnemyMover::SetModeChase(V2d *target, V2d &offset, double maxVel,
	double accel, int frameDuration )
{
	chaseTarget = target;
	chaseOffset = offset;
	chaseMaxVel = maxVel;
	chaseAccel = accel;
	velocity = V2d(0, 0);
	moveType = CHASE;
	actionFrames = frameDuration;
}

void EnemyMover::SetModeNodeJump(V2d &nodePos,double extraHeight)
{
	V2d start = currPosInfo.GetPosition();
	double xDiff = nodePos.x - start.x;

	double peakY;
	if (start.y < nodePos.y)
	{
		peakY = start.y - extraHeight;
	}
	else
	{
		peakY = nodePos.y - extraHeight;
	}

	SetModeNodeQuadraticConstantSpeed(V2d(start.x + xDiff / 2, peakY),
		nodePos, CubicBezier(), 20);
}

void EnemyMover::SetModeNodeLinear( V2d &nodePos, CubicBezier &cb, int frameDuration)
{
	moveType = NODE_LINEAR;
	linearMove->SetFrameDuration(frameDuration);
	linearMove->start = currPosInfo.GetPosition();
	linearMove->end = nodePos;
	linearMove->InitDebugDraw();
	linearMovementSeq.Reset();
}

void EnemyMover::SetModeNodeLinearConstantSpeed(
	V2d &nodePos,
	CubicBezier &cb,
	double speed)
{
	int frames = GetLinearFrameEstimate(speed, currPosInfo.GetPosition(), nodePos);

	SetModeNodeLinear(nodePos, cb, frames);
}

void EnemyMover::SetModeNodeQuadratic( V2d &controlPoint0, V2d &nodePos,
	CubicBezier &cb, int frameDuration)
{
	moveType = NODE_QUADRATIC;
	quadraticMove->SetFrameDuration(frameDuration);
	quadraticMove->A = currPosInfo.GetPosition();
	quadraticMove->B = controlPoint0;
	quadraticMove->C = nodePos;
	quadraticMove->start = quadraticMove->A;
	quadraticMove->end = quadraticMove->C;
	quadraticMove->InitDebugDraw();
	quadraticMovementSeq.Reset();
}

void EnemyMover::SetModeNodeQuadraticConstantSpeed(
	V2d &controlPoint0,
	V2d &nodePos,
	CubicBezier &cb,
	double speed)
{
	int frames = GetQuadraticFrameEstimate(speed, currPosInfo.GetPosition(),
		controlPoint0, nodePos);

	SetModeNodeQuadratic(controlPoint0, nodePos, cb, frames);
}

void EnemyMover::SetModeNodeCubic(V2d &controlPoint0, V2d &controlPoint1,
	V2d &nodePos, CubicBezier &cb, int frameDuration)
{
	moveType = NODE_CUBIC;
	cubicMove->SetFrameDuration(frameDuration);
	cubicMove->A = currPosInfo.GetPosition();
	cubicMove->B = controlPoint0;
	cubicMove->C = controlPoint1;
	cubicMove->D = nodePos;
	cubicMove->start = cubicMove->A;
	cubicMove->end = cubicMove->D;
	cubicMove->InitDebugDraw();
	cubicMovementSeq.Reset();
}

void EnemyMover::SetModeNodeCubicConstantSpeed(V2d &controlPoint0,V2d &controlPoint1,
	V2d &nodePos, CubicBezier &cb, double speed)
{
	int frames = GetCubicFrameEstimate(speed, currPosInfo.GetPosition(),
		controlPoint0, controlPoint1, nodePos);

	SetModeNodeCubic(controlPoint0, controlPoint1, nodePos, cb, frames);
}

void EnemyMover::SetModeNodeDoubleQuadratic(
	V2d &controlPoint0,
	V2d &nodePos,
	CubicBezier &cb,
	int frameDuration,
	double spreadFactor)
{
	moveType = NODE_DOUBLE_QUADRATIC;
	doubleQuadtraticMove0->SetFrameDuration(frameDuration / 2);
	doubleQuadtraticMove0->A = currPosInfo.GetPosition();
	doubleQuadtraticMove0->C = controlPoint0;

	doubleQuadtraticMove0->start = doubleQuadtraticMove0->A;
	doubleQuadtraticMove0->end = doubleQuadtraticMove0->C;

	doubleQuadtraticMove1->SetFrameDuration(frameDuration / 2);
	doubleQuadtraticMove1->A = controlPoint0;

	doubleQuadtraticMove1->C = nodePos;
	doubleQuadtraticMove1->start = doubleQuadtraticMove1->A;
	doubleQuadtraticMove1->end = doubleQuadtraticMove1->C;

	V2d dir0 = normalize(doubleQuadtraticMove0->end - doubleQuadtraticMove0->start);
	V2d dir1 = normalize(doubleQuadtraticMove0->end - doubleQuadtraticMove1->end);
	V2d bisector = normalize(dir0 + dir1);
	V2d nb(bisector.y, -bisector.x);

	if (GetVectorAngleDiffCCW(dir0, dir1) < PI)
	{
		nb = -nb;
	}

	doubleQuadtraticMove0->B = controlPoint0 - nb * spreadFactor;
	doubleQuadtraticMove1->B = controlPoint0 + nb * spreadFactor;

	doubleQuadtraticMove0->InitDebugDraw();
	doubleQuadtraticMove1->InitDebugDraw();

	doubleQuadraticMovementSeq.Reset();
}

void EnemyMover::SetModeNodeDoubleQuadraticConstantSpeed(
	V2d &controlPoint0,
	V2d &nodePos,
	CubicBezier &cb,
	double speed,
	double spreadFactor)
{
	SetModeNodeDoubleQuadratic(controlPoint0, nodePos, cb, 0, spreadFactor);

	int f0 = GetQuadraticFrameEstimate(speed, doubleQuadtraticMove0->A,
		doubleQuadtraticMove0->B, doubleQuadtraticMove0->C);
	doubleQuadtraticMove0->SetFrameDuration(f0);
	int f1 = GetQuadraticFrameEstimate(speed, doubleQuadtraticMove1->A,
		doubleQuadtraticMove1->B, doubleQuadtraticMove1->C);
	doubleQuadtraticMove1->SetFrameDuration(f1);
	doubleQuadtraticMove0->InitDebugDraw();
	doubleQuadtraticMove1->InitDebugDraw();
}

void EnemyMover::UpdatePhysics(int numPhysSteps,
	int slowMultiple)
{
	switch (moveType)
	{
	case NODE_PROJECTILE:
	{
		double factor = slowMultiple * (double)numPhysSteps;
		V2d movementVec = velocity;
		movementVec /= factor;

		currPosInfo.position += movementVec;
		velocity += projectileGrav / factor;
		break;
	}
	case CHASE:
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
		break;
	}
	case NODE_LINEAR:
	case NODE_QUADRATIC:
	case NODE_CUBIC:
	case NODE_DOUBLE_QUADRATIC:
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
		else if (doubleQuadraticMovementSeq.currMovement != NULL)
		{
			currSeq = &doubleQuadraticMovementSeq;
		}

		if (currSeq != NULL)
		{
			Movement *currMovement = currSeq->currMovement;
			if (numPhysSteps == 1)
			{
				currSeq->Update(slowMultiple, 10);
			}
			else
			{
				currSeq->Update(slowMultiple);
			}

			currPosInfo.position = currSeq->position;

			if (currSeq->currMovement == NULL)
			{
				moveType = NONE;
				lastActionEndVelocity = currMovement->GetEndVelocity();
			}
		}

		break;
	}
	case GRIND:
	{
		double factor = slowMultiple * (double)numPhysSteps;
		double movement = grindSpeed / factor;

		double quant = currPosInfo.GetQuant();

		Edge *grindEdge = currPosInfo.GetEdge();
		int grindEdgeIndex = currPosInfo.GetEdgeIndex();
		PolyPtr groundPoly = currPosInfo.ground;
		int numPoints = groundPoly->GetNumPoints();

		while (!approxEquals(movement, 0))
		{
			double gLen = grindEdge->GetLength();

			if (movement > 0)
			{
				double extra = quant + movement - gLen;

				if (extra > 0)
				{
					movement -= gLen - quant;
					grindEdge = grindEdge->GetNextEdge();
					++grindEdgeIndex;
					if (grindEdgeIndex == numPoints)
					{
						grindEdgeIndex = 0;
					}

					quant = 0;
				}
				else
				{
					quant += movement;
					movement = 0;
				}
			}
			else
			{
				double extra = quant + movement;

				if (extra < 0)
				{
					movement -= movement - extra;
					grindEdge = grindEdge->GetPrevEdge();
					--grindEdgeIndex;
					if (grindEdgeIndex < 0)
					{
						grindEdgeIndex = numPoints - 1;
					}
					quant = grindEdge->GetLength();
				}
				else
				{
					quant += movement;
					movement = 0;
				}
			}
		}
		currPosInfo.SetGround(groundPoly, grindEdgeIndex, quant);
		break;
	}
	case SWING:
	{
		double factor = slowMultiple * numPhysSteps;
		//velocity += V2d( 0, 2.0 ) / factor;

		V2d wPos = currPosInfo.GetPosition();

		V2d tes = normalize(wPos - swingAnchor);
		tes = V2d(tes.y, -tes.x);

		double val = dot(velocity, normalize(swingAnchor - wPos));

		V2d otherTes = val * normalize(swingAnchor - wPos);

		double speed = dot(velocity, tes);

		/*if (speed > 0 && speed < 10)
		{
			speed = 10;
		}
		else if( speed < 0 && speed > -10 )
		{
			speed = -10;
		}*/
		//speed = 20.0;

		velocity = speed * tes;
		velocity += otherTes;

		V2d future = wPos + velocity;

		V2d diff = swingAnchor - future;

		if (length(diff) > wireLength)
		{
			double pullVel = length(diff) - wireLength;
			V2d pullDir = normalize(diff);
			future += pullDir * pullVel;
			velocity = future - wPos;
		}

		V2d movementVec = velocity;
		movementVec /= factor;

		currPosInfo.position += movementVec;
		break;
	}
	case FALL:
	{
		double factor = slowMultiple * (double)numPhysSteps;
		V2d movementVec = velocity;
		movementVec /= factor;

		currPosInfo.position += movementVec;
		velocity += projectileGrav / factor;
		break;
	}
		
	}
}

void EnemyMover::DebugDraw(sf::RenderTarget *target)
{
	if (nodeCircles != NULL)
	{
		nodeCircles->Draw(target);
	}

	debugCircles->Draw(target);


	switch (moveType)
	{
	case CHASE:
	{
		break;
	}
	case NODE_LINEAR:
	case NODE_QUADRATIC:
	case NODE_CUBIC:
	case NODE_DOUBLE_QUADRATIC:
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
		else if (doubleQuadraticMovementSeq.currMovement != NULL)
		{
			currSeq = &doubleQuadraticMovementSeq;
		}

		currSeq->MovementDebugDraw(target);
		break;
	}
	case SWING:
	{
		UpdateSwingDebugDraw();
		target->draw(swingQuad, 4, sf::Quads);
		break;
	}
	}
}

void EnemyMover::FrameIncrement()
{
	if (moveType == CHASE)
	{
		if (actionFrames > 0)
		{
			--actionFrames;
			if (actionFrames == 0)
			{
				moveType = NONE;
				lastActionEndVelocity = velocity;
				return;
			}
		}

		V2d tPos = *chaseTarget + chaseOffset;//playerPos + V2d(50, 0);
		V2d diff = tPos - currPosInfo.GetPosition();
		V2d pDir = normalize(diff);

		velocity += pDir * chaseAccel;
		double velLen = length(velocity);
		if (velLen > chaseMaxVel)
		{
			velocity = normalize(velocity) * chaseMaxVel;
		}
	}
	else if (moveType == NODE_PROJECTILE)
	{
		if (actionFrames > 0)
		{
			--actionFrames;
			if (actionFrames == 0)
			{
				moveType = NONE;
				if (targetPI != NULL)
				{
					if (targetPI->edge != NULL)
					{
						currPosInfo.SetGround(targetPI->edge->poly, targetPI->edgeIndex,
							targetPI->edgeQuantity);
						currPosInfo.position = targetPI->pos;
					}
					else
					{
						currPosInfo.SetAerial(targetPI->pos);
					}
				}
				else
				{
					currPosInfo.SetAerial(targetPos);
				}
				lastActionEndVelocity = velocity;
			}
		}
	}
	else if (moveType == GRIND)
	{
		if (actionFrames > 0)
		{
			--actionFrames;
			if (actionFrames == 0)
			{
				moveType = NONE;
				lastActionEndVelocity = currPosInfo.GetEdge()->Along() * grindSpeed;
			}
		}
	}
	else if (moveType == SWING)
	{
		if (actionFrames > 0)
		{
			--actionFrames;
			if (actionFrames == 0)
			{
				moveType = NONE;
				lastActionEndVelocity = velocity;
			}
		}
	}
	else if (moveType == FALL)
	{
		if (actionFrames > 0)
		{
			--actionFrames;
			if (actionFrames == 0)
			{
				moveType = NONE;
				lastActionEndVelocity = velocity;
			}
		}
	}
	else if (moveType == WAIT)
	{
		if (actionFrames > 0)
		{
			--actionFrames;
			if (actionFrames == 0)
			{
				moveType = NONE;
				lastActionEndVelocity = V2d(0, 0);
				return;
			}
		}
	}
}

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
			coyShockPool.Throw(GetPosition(), 100, 400, 50, 60 );
			//auto *f = shurPool.Throw(GetPosition(), dir, BirdShuriken::UNBLOCKABLE_STICK);
			//if (f == NULL)
			//{
			//	shurPool.RethrowAll();
			//	enemyMover.SetModeWait(120);
			//}
			//else
			//{
			//	enemyMover.SetModeNodeProjectile(nodeVec[r], V2d(0, 2.0), 200);//300);
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
	coyShockPool.Draw(target);
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
	//target->draw(predictCircle);
}