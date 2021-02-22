#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Skeleton.h"
#include "Actor.h"
#include "SequenceW6.h"

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
	:Boss(EnemyType::EN_SKELETONBOSS, ap),
	testGroup(2, 20, Color::Red, 6)
	//testGroup2(1, 18, Color::Cyan, 6)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	StageSetup(4, 4);

	level = ap->GetLevel();

	ts_move = sess->GetSizedTileset("Bosses/Gator/dominance_384x384.png");


	stageMgr.AddActiveOption(0, MOVE_OTHER, 2);

	stageMgr.AddActiveOption(1, MOVE_WIRE_DASH, 2);

	stageMgr.AddActiveOption(2, MOVE_WIRE_DASH, 2);

	stageMgr.AddActiveOption(3, MOVE_WIRE_DASH, 2);
	

	postFightScene = NULL;

	sprite.setColor(Color::Black);

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

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

	facingRight = true;

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
	}
}

void Skeleton::HandleAction()
{

}

void Skeleton::StartAction()
{
	switch (action)
	{
	case MOVE_WIRE_DASH:
	{
		V2d pPos = targetPlayer->position;
		rcEdge = NULL;
		rayStart = GetPosition();
		rayEnd = pPos + PlayerDir(targetPlayerIndex) * 5000.0;
		ignorePointsCloserThanPlayer = true;
		playerDist = PlayerDist(targetPlayerIndex);
		RayCast(this, sess->terrainTree->startNode, rayStart, rayEnd);

		//testGroup.HideAll();
		//testGroup2.HideAll();
		
		
		

		if (rcEdge != NULL)
		{
			assert(rcEdge != NULL);

			V2d basePos = rcEdge->GetPosition(rcQuantity);

			//testGroup.SetPosition(0, Vector2f(rcEdge->v0));
			//testGroup.SetPosition(1, Vector2f(rcEdge->v1));

			//testGroup2.SetPosition(0, Vector2f(basePos));

			//testGroup.ShowAll();
			//testGroup2.ShowAll();

			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();

			enemyMover.SetModeNodeLinear(basePos, CubicBezier(), 60);
			//enemyMover.SetModeNodeLinearConstantSpeed(basePos, CubicBezier(), 40);
			//enemyMover.SetModeZipAndFall(basePos, V2d(0, 2), nodePos);
		}
		break;
	}
	case MOVE_OTHER:
	{
		V2d nodePos = nodeGroupB.AlwaysGetNextNode()->pos;

		
		testGroup.SetPosition(0, Vector2f(nodePos));
		testGroup.ShowAll();

		V2d nodeDiff = nodePos - GetPosition();
		if (nodeDiff.y < -600)
		{
			rcEdge = NULL;
			rayStart = nodePos + V2d(0, -10);
			rayEnd = nodePos + V2d(0, -1) * 5000.0;//other * 5000.0;
			ignorePointsCloserThanPlayer = false;
			RayCast(this, sess->terrainTree->startNode, rayStart, rayEnd);

			if (rcEdge != NULL)
			{
				assert(rcEdge != NULL);

				V2d basePos = rcEdge->GetPosition(rcQuantity);

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
		else
		{
			cout << "cancel: " << nodeDiff.y << endl;
		}
		break;
	}
		
	}

	{
		//if (true)
		//{
		//	rcEdge = NULL;
		//	rayStart = GetPosition();
		//	rayEnd = pPos + pDir * 5000.0;
		//	ignorePointsCloserThanPlayer = true;
		//	playerDist = length(pPos - GetPosition());
		//	RayCast(this, sess->terrainTree->startNode, rayStart, rayEnd);

		//	if (rcEdge != NULL)
		//	{
		//		assert(rcEdge != NULL);

		//		V2d basePos = rcEdge->GetPosition(rcQuantity);

		//		enemyMover.currPosInfo.SetAerial();
		//		currPosInfo.SetAerial();

		//		enemyMover.SetModeNodeLinearConstantSpeed(basePos, CubicBezier(), 40);
		//		//enemyMover.SetModeZipAndFall(basePos, V2d(0, 2), nodePos);
		//	}
		//}
		//else if (nodeDiff.y < -600)
		//{
		//	rcEdge = NULL;
		//	rayStart = nodePos + V2d(0, -10);
		//	rayEnd = nodePos + V2d(0, -1) * 5000.0;//other * 5000.0;
		//	ignorePointsCloserThanPlayer = false;
		//	RayCast(this, sess->terrainTree->startNode, rayStart, rayEnd);

		//	if (rcEdge != NULL)
		//	{
		//		assert(rcEdge != NULL);

		//		V2d basePos = rcEdge->GetPosition(rcQuantity);

		//		enemyMover.currPosInfo.SetAerial();
		//		currPosInfo.SetAerial();

		//		enemyMover.SetModeZipAndFall(basePos, V2d(0, 2), nodePos);
		//		//enemyMover.SetModeRadial(basePos, speed, dest);
		//		//enemyMover.SetModeSwing(basePos, length(basePos - GetPosition()), 60);
		//	}
		//	//enemyMover.SetModeZipAndFall(, speed, centerPoint, nodePos);

		//}
		//else if (absNodeDiffX > 600)
		//	//if( true )
		//{

		//	//stuff
		//	V2d along = nodePos - GetPosition();
		//	V2d midPoint = GetPosition() + along / 4.0;
		//	V2d centerPoint = (nodePos + GetPosition()) / 2.0;
		//	along = normalize(along);

		//	V2d other(along.y, -along.x);

		//	double speed = -40;//dot(startVel, dir);

		//	if (other.y >= 0)
		//	{
		//		speed = -speed;
		//		other = -other;
		//	}


		//	rcEdge = NULL;
		//	rayStart = midPoint;
		//	rayEnd = midPoint + V2d(0, -1) * 5000.0;//other * 5000.0;
		//	RayCast(this, sess->terrainTree->startNode, rayStart, rayEnd);

		//	if (rcEdge != NULL)
		//	{
		//		assert(rcEdge != NULL);

		//		V2d basePos = rcEdge->GetPosition(rcQuantity);

		//		//V2d dir = normalize(basePos - GetPosition());
		//		//V2d along(-dir.y, dir.x);



		//		enemyMover.SetModeRadialDoubleJump(basePos, speed, centerPoint, nodePos);
		//		//enemyMover.SetModeRadial(basePos, speed, dest);
		//		//enemyMover.SetModeSwing(basePos, length(basePos - GetPosition()), 60);
		//	}
		//}
		//else
		//{
		//	enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 2), 100);
		//}
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
	return a == MOVE_WIRE_DASH || a == MOVE_OTHER;
}

void Skeleton::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
	testGroup.Draw(target);
	//testGroup.Draw(target);
	//testGroup2.Draw(target);
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

	V2d dir = normalize(rayEnd - rayStart);
	V2d pos = edge->GetPosition(edgeQuantity);

	//if (!sess->IsWithinBounds(pos))
	//{
	//	return; //prevents it from hitting the birdtransform area currently
	//}

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