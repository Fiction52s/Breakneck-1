#include "EnemyMover.h"
#include "GameSession.h"
#include "EditorTerrain.h"

using namespace sf;




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
	V2d other = normalize(swingAnchor - currPos);
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
	double lengthEstimate = length(start - cp0) + length(end - cp0);
	double frameEstimate = lengthEstimate / attemptSpeed;

	return frameEstimate;
}

int EnemyMover::GetCubicFrameEstimate(double attemptSpeed,
	V2d &start, V2d &cp0, V2d &cp1, V2d &end)
{
	double lengthEstimate = length(start - cp0) + length(cp1 - cp0)
		+ length(end - cp1);
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
	const std::string &str, sf::Color c)
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

void EnemyMover::SetModeFall(double grav, int frames)
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
	int frames)
{
	swingAnchor = p_swingAnchor;
	wireLength = p_wireLength;
	actionFrames = frames;
	moveType = SWING;
}

void EnemyMover::SetModeGrind(double speed, int frames)
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
	double accel, int frameDuration)
{
	chaseTarget = target;
	chaseOffset = offset;
	chaseMaxVel = maxVel;
	chaseAccel = accel;
	velocity = V2d(0, 0);
	moveType = CHASE;
	actionFrames = frameDuration;
}

void EnemyMover::SetModeNodeJump(V2d &nodePos, double extraHeight)
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

void EnemyMover::SetModeNodeLinear(V2d &nodePos, CubicBezier &cb, int frameDuration)
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

void EnemyMover::SetModeNodeQuadratic(V2d &controlPoint0, V2d &nodePos,
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

void EnemyMover::SetModeNodeCubicConstantSpeed(V2d &controlPoint0, V2d &controlPoint1,
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