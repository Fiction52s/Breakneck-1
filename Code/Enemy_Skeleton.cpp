#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Skeleton.h"
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


Skeleton::Skeleton(ActorParams *ap)
	:Enemy(EnemyType::EN_SKELETONBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;
	reachPointOnFrame[COMBOMOVE] = 0;

	ts_move = sess->GetSizedTileset("Bosses/Gator/dominance_384x384.png");

	nodeAStr = "A";
	nodeBStr = "B";

	sprite.setColor(Color::Black);

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

void Skeleton::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Skeleton::UpdateHitboxes()
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

void Skeleton::ResetEnemy()
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

void Skeleton::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

void Skeleton::SetCommand(int index, BirdCommand &bc)
{
	actionQueue[index] = bc;
}

void Skeleton::DebugDraw(sf::RenderTarget *target)
{
	playerComboer.DebugDraw(target);
	enemyMover.DebugDraw(target);
}

void Skeleton::DirectKill()
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

void Skeleton::FrameIncrement()
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

void Skeleton::UpdatePreFrameCalculations()
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

void Skeleton::ProcessState()
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

		auto &nodeVec = sess->GetBossNodeVector(BossFightType::FT_SKELETON, nodeBStr);
		int vecSize = nodeVec.size();
		int rNode = rand() % vecSize;

		V2d nodePos = nodeVec[rNode]->pos;

		V2d pPos = sess->GetPlayerPos(0);
		V2d pDir = normalize(pPos - GetPosition());

		V2d nodeDiff = nodePos - GetPosition();

		double absNodeDiffX = abs(nodeDiff.x);

		cout << "absdiffx: " << absNodeDiffX << "\n";

		

		/*if ()
		{
			enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 2), 100);
		}
		else */
		if (true)
		{
			rcEdge = NULL;
			rayStart = GetPosition();
			rayEnd = pPos + pDir * 5000.0;
			ignorePointsCloserThanPlayer = true;
			playerDist = length(pPos - GetPosition());
			RayCast(this, sess->terrainTree->startNode, rayStart, rayEnd);

			if (rcEdge != NULL)
			{
				assert(rcEdge != NULL);

				V2d basePos = rcEdge->GetPosition(rcQuantity);

				enemyMover.currPosInfo.SetAerial();
				currPosInfo.SetAerial();

				enemyMover.SetModeNodeLinearConstantSpeed(basePos, CubicBezier(), 40);
				//enemyMover.SetModeZipAndFall(basePos, V2d(0, 2), nodePos);
			}
		}
		else if (nodeDiff.y < -600)
		{
			rcEdge = NULL;
			rayStart = nodePos + V2d( 0, -10 );
			rayEnd = nodePos + V2d(0, -1) * 5000.0;//other * 5000.0;
			ignorePointsCloserThanPlayer = false;
			RayCast(this, sess->terrainTree->startNode, rayStart, rayEnd);

			if (rcEdge != NULL)
			{
				assert(rcEdge != NULL);

				V2d basePos = rcEdge->GetPosition(rcQuantity);

				enemyMover.currPosInfo.SetAerial();
				currPosInfo.SetAerial();

				enemyMover.SetModeZipAndFall(basePos, V2d( 0, 2 ), nodePos);
				//enemyMover.SetModeRadial(basePos, speed, dest);
				//enemyMover.SetModeSwing(basePos, length(basePos - GetPosition()), 60);
			}
			//enemyMover.SetModeZipAndFall(, speed, centerPoint, nodePos);

		}
		else if (absNodeDiffX > 600 )
		//if( true )
		{
			
			//stuff
			V2d along = nodePos - GetPosition();
			V2d midPoint = GetPosition() + along / 4.0;
			V2d centerPoint = (nodePos + GetPosition()) / 2.0;
			along = normalize(along);

			V2d other(along.y, -along.x);

			double speed = -40;//dot(startVel, dir);

			if (other.y >= 0)
			{
				speed = -speed;
				other = -other;
			}


			rcEdge = NULL;
			rayStart = midPoint;
			rayEnd = midPoint + V2d(0, -1) * 5000.0;//other * 5000.0;
			RayCast(this, sess->terrainTree->startNode, rayStart, rayEnd);

			if (rcEdge != NULL)
			{
				assert(rcEdge != NULL);

				V2d basePos = rcEdge->GetPosition(rcQuantity);

				//V2d dir = normalize(basePos - GetPosition());
				//V2d along(-dir.y, dir.x);



				enemyMover.SetModeRadialDoubleJump(basePos, speed, centerPoint, nodePos);
				//enemyMover.SetModeRadial(basePos, speed, dest);
				//enemyMover.SetModeSwing(basePos, length(basePos - GetPosition()), 60);
			}
		}
		else
		{
			enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 2), 100);
		}

		
		//end stuff



		r = 4;
		if (r == 0)
		{
			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();
			//enemyMover.SetModeSwing(nodePos, length(nodePos - GetPosition()), 60);
			//enemyMover.SetModeRadial(nodePos);

			//enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 1.5), 200);
			//enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
			//orbPool.Throw(GetPosition(), nodePos, SkeletonWaterOrb::OrbType::NODE_GROW);
		}
		else if (r == 1)
		{
			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();
			enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 1.5), 200);
			//enemyMover.SetModeNodeQuadratic(pPos, nodePos, CubicBezier(), 60);
			//orbPool.Throw(GetPosition(), nodePos, SkeletonWaterOrb::OrbType::NODE_GROW);
		}
		else if (r == 2)
		{
			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();
			enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 1.5), 200);
			//enemyMover.SetModeChase(&sess->GetPlayer(0)->position, V2d(0, 0),
			//	10, .5, 60);
			//orbPool.Throw(GetPosition(), nodePos, SkeletonWaterOrb::OrbType::NODE_GROW);
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

void Skeleton::IHitPlayer(int index)
{
	hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames;
}

void Skeleton::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void Skeleton::UpdateSprite()
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

void Skeleton::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	orbPool.Draw(target);
}

void Skeleton::HandleHitAndSurvive()
{
	//fireCounter = 0;
}

int Skeleton::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Skeleton::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	//d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
}

void Skeleton::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	//fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
}

void Skeleton::HandleRayCollision(Edge *edge, double edgeQuantity,
	double rayPortion)
{
	V2d dir = normalize(rayEnd - rayStart);
	V2d pos = edge->GetPosition(edgeQuantity);
	double along = dot(dir, edge->Normal());

	double posDist = length(pos - GetPosition());

	if (along < 0 && (rcEdge == NULL || length(edge->GetPosition(edgeQuantity) - rayStart) <
		length(rcEdge->GetPosition(rcQuantity) - rayStart)))
	{
		if (!ignorePointsCloserThanPlayer || (ignorePointsCloserThanPlayer && posDist > playerDist))
		{
			rcEdge = edge;
			rcQuantity = edgeQuantity;
		}
	}
}