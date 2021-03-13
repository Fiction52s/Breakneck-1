#include "Session.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Skeleton.h"
#include "Actor.h"
#include "SequenceW6.h"

using namespace std;
using namespace sf;


Skeleton::Skeleton(ActorParams *ap)
	:Boss(EnemyType::EN_SKELETONBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	StageSetup(4, 4);

	level = ap->GetLevel();

	ts_move = sess->GetSizedTileset("Bosses/Gator/dominance_384x384.png");


	stageMgr.AddActiveOption(0, PLAN_PATTERN, 2);

	//stageMgr.AddActiveOption(0, MOVE_OTHER, 2);

	stageMgr.AddActiveOption(1, MOVE_WIRE_DASH, 2);

	stageMgr.AddActiveOption(2, MOVE_WIRE_DASH, 2);

	stageMgr.AddActiveOption(3, MOVE_WIRE_DASH, 2);
	

	postFightScene = NULL;

	sprite.setColor(Color::Black);

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	patternFlickerFrames = 30;
	numPatternMoves = 3;

	patternTypePicker.Reset();
	patternTypePicker.AddActiveOption(PATTERN_MOVE);

	pattern.reserve(6);
	pattern.resize(numPatternMoves);

	patternType.reserve(6);
	patternType.resize(numPatternMoves);

	patternPreview.setFillColor(Color::Magenta);
	patternPreview.setRadius(60);
	patternPreview.setOrigin(patternPreview.getLocalBounds().width / 2,
		patternPreview.getLocalBounds().height / 2);


	ResetEnemy();
}

Skeleton::~Skeleton()
{
	if (postFightScene != NULL)
		delete postFightScene;
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

void Skeleton::ResetEnemy()
{
	orbPool.Reset();

	currNode = NULL;
	patternIndex = -1;

	facingRight = true;

	rayCastInfo.tree = sess->terrainTree;

	BossReset();

	StartFight();

	UpdateSprite();
}

bool Skeleton::TryComboMove(V2d &comboPos, int comboMoveDuration,
	int moveDurationBeforeStartNextAction,
	V2d &comboOffset)
{
	return false;
}

int Skeleton::ChooseActionAfterStageChange()
{
	return Boss::ChooseActionAfterStageChange();
}

void Skeleton::ActivatePostFightScene()
{
	postFightScene->Reset();
	sess->SetActiveSequence(postFightScene);
}

void Skeleton::ActionEnded()
{
	switch (action)
	{
	case WAIT:
	case MOVE_WIRE_DASH:
	case MOVE_OTHER:
		Decide();
		break;
	case PLAN_PATTERN:
	{
		patternIndex = 0;

		SetAction(PATTERN_MOVE);
		break;
	}
	case PATTERN_MOVE:
	{
		if (patternIndex == numPatternMoves - 1)
		{
			/*if (stageMgr.GetCurrStage() > 0)
			{
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = PlayerDir();
			launchers[0]->Fire();
			}*/

			if (patternType[patternIndex] == PATTERN_MOVE)
			{
				Wait(30);
			}
			else
			{
				++patternIndex;
				SetAction(patternType[patternIndex - 1]);
			}
		}
		else
		{
			++patternIndex;
			SetAction(patternType[patternIndex - 1]);

		}

		break;
	}
	}
}

void Skeleton::HandleAction()
{
	switch (action)
	{
	case PLAN_PATTERN:
	{
		if (frame % patternFlickerFrames == 0)
		{
			int mult = frame / patternFlickerFrames;
			if (mult < numPatternMoves)
			{
				//currBabyScorpPos = pattern[mult]->pos;
				patternPreview.setPosition(Vector2f(pattern[mult]->pos));

				if (patternType[mult] == PATTERN_MOVE)
				{
					patternPreview.setFillColor(Color::Magenta);
				}
				/*else if (patternType[mult] == PATTERN_RUSH)
				{
					patternPreview.setFillColor(Color::Red);
				}*/
			}
		}
		break;
	}
	}
}

void Skeleton::StartAction()
{
	switch (action)
	{
	case MOVE_WIRE_DASH:
	{
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
			//enemyMover.SetModeNodeLinear(basePos, CubicBezier(), 60);
			enemyMover.SetModeNodeLinearConstantSpeed(basePos, CubicBezier(), 40);
			//enemyMover.SetModeZipAndFall(basePos, V2d(0, 2), nodePos);
		}
		else
		{
			Wait(1);
			assert(0);
		}
		break;
	}
	case MOVE_OTHER:
	{
		V2d nodePos = nodeGroupB.AlwaysGetNextNode()->pos;
		StartMovement(nodePos);
		break;
	}
	case PATTERN_MOVE:
	{
		currNode = pattern[patternIndex];
		StartMovement(currNode->pos);
		break;
	}
	case PLAN_PATTERN:
	{
		//babyScorpionGroup.Reset();
		PoiInfo *testNode;
		for (int i = 0; i < numPatternMoves; ++i)
		{
			do
			{
				testNode = nodeGroupB.AlwaysGetNextNode();
			} while (testNode == currNode);

			pattern[i] = testNode;
		}

		for (int i = 0; i < numPatternMoves; ++i)
		{
			patternType[i] = patternTypePicker.AlwaysGetNextOption();
		}

		actionLength[PLAN_PATTERN] = patternFlickerFrames * numPatternMoves;
		break;
	}
		
	}
}

void Skeleton::StartMovement(V2d &pos)
{
	V2d nodePos = pos;
	V2d nodeDiff = nodePos - GetPosition();
	double absNodeDiffX = abs(nodeDiff.x);

	if (nodeDiff.y < -600)
	{
		rayCastInfo.rcEdge = NULL;
		rayCastInfo.rayStart = nodePos + V2d(0, -10);
		rayCastInfo.rayEnd = nodePos + V2d(0, -1) * 5000.0;//other * 5000.0;
		ignorePointsCloserThanPlayer = false;
		RayCast(this, sess->terrainTree->startNode, rayCastInfo.rayStart, rayCastInfo.rayEnd);

		if (rayCastInfo.rcEdge != NULL)
		{
			assert(rayCastInfo.rcEdge != NULL);

			V2d basePos = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant);

			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();

			enemyMover.SetModeZipAndFall(basePos, V2d(0, 2), nodePos);
			//enemyMover.SetModeRadial(basePos, speed, dest);
			//enemyMover.SetModeSwing(basePos, length(basePos - GetPosition()), 60);
		}
		else
		{
		}
		//enemyMover.SetModeZipAndFall(, speed, centerPoint, nodePos);

	}
	else if (absNodeDiffX > 600)
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


		rayCastInfo.rcEdge = NULL;
		rayCastInfo.rayStart = midPoint;
		rayCastInfo.rayEnd = midPoint + V2d(0, -1) * 5000.0;//other * 5000.0;
		RayCast(this, sess->terrainTree->startNode, rayCastInfo.rayStart, rayCastInfo.rayEnd);

		if (rayCastInfo.rcEdge != NULL)
		{
			assert(rayCastInfo.rcEdge != NULL);

			V2d basePos = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant);

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
}

void Skeleton::SetupPostFightScenes()
{
	if (postFightScene == NULL)
	{
		postFightScene = new SkeletonPostFightScene;
		postFightScene->skeleton = this;
		postFightScene->coyHelper = coyHelper;
		postFightScene->Init();
	}
}

void Skeleton::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_SKELETON, "A"));
	nodeGroupB.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_SKELETON, "B"));
}

bool Skeleton::IsDecisionValid(int d)
{
	return true;
}

bool Skeleton::IsEnemyMoverAction(int a)
{
	return a == MOVE_WIRE_DASH || a == MOVE_OTHER || a == PATTERN_MOVE;
}

void Skeleton::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
}

void Skeleton::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Skeleton::StartFight()
{
	Wait(10);
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	frame = 0;
	HitboxesOff();
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
	if (action == PLAN_PATTERN)
	{
		target->draw(patternPreview);
	}

	DrawSprite(target, sprite);
	orbPool.Draw(target);
}

int Skeleton::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Skeleton::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	//d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);
}

void Skeleton::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	//fireCounter = d.fireCounter;

	bytes += sizeof(MyData);
}

void Skeleton::HandleRayCollision(Edge *edge, double edgeQuantity,
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