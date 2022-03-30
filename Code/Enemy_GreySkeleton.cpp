#include "Session.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GreySkeleton.h"
#include "Actor.h"
#include "SequenceW7.h"


using namespace std;
using namespace sf;

GreySkeleton::GreySkeleton(ActorParams *ap)
	:Boss(EnemyType::EN_GREYSKELETONBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	StageSetup(4, 4);
	
	level = ap->GetLevel();

	ts_move = GetSizedTileset("Bosses/Gator/dominance_384x384.png");
	sprite.setColor(Color::Green);


	actionLength[THORN_TEST] = 300;
	animFactor[THORN_TEST] = 1;

	actionLength[SHAPE_TEST] = 180;
	animFactor[SHAPE_TEST] = 1;

	postFightScene = NULL;

	stageMgr.AddActiveOption(0, BOMB_TEST, 2);
	//stageMgr.AddActiveOption(0, THORN_TEST, 2);
	//stageMgr.AddActiveOption(0, SHAPE_TEST, 2);

	leftHand = NULL;

	rightHand = new Hand(true);

	/*stageMgr.AddActiveOption(0, MOVE, 2);

	stageMgr.AddActiveOption(1, MOVE, 2);

	stageMgr.AddActiveOption(2, MOVE, 2);

	stageMgr.AddActiveOption(3, MOVE, 2);*/

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	ResetEnemy();
}

GreySkeleton::~GreySkeleton()
{
	if (postFightScene != NULL)
		delete postFightScene;

	if (leftHand != NULL)
		delete leftHand;

	if (rightHand != NULL)
		delete rightHand;
}

void GreySkeleton::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void GreySkeleton::ResetEnemy()
{
	facingRight = true;

	

	HitboxesOff();

	UpdateSprite();

	thornPool.Reset();
	shapePool.Reset();
	bombPool.Reset();

	if (sess->preLevelScene == NULL) //fight testing
	{
		CameraShot *cs = sess->cameraShotMap["fightcam"];
		if (cs != NULL)
		{
			sess->cam.Set(Vector2f(cs->centerPos), cs->zoom, 0);
		}
	}

	BossReset();

	StartFight();

	UpdateSprite();

	
}

void GreySkeleton::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;
	//orbPool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void GreySkeleton::StartFight()
{
	Wait(10);
	DefaultHitboxesOn();
	DefaultHurtboxesOn();
	HitboxesOff();
}

bool GreySkeleton::TryComboMove(V2d &comboPos, int comboMoveDuration,
	int moveDurationBeforeStartNextAction,
	V2d &comboOffset)
{
	return false;
}

int GreySkeleton::ChooseActionAfterStageChange()
{
	return Boss::ChooseActionAfterStageChange();
}

void GreySkeleton::ActivatePostFightScene()
{
	postFightScene->Reset();
	sess->SetActiveSequence(postFightScene);
}

void GreySkeleton::ActionEnded()
{
	switch (action)
	{
	case WAIT:
	case MOVE:
	case THORN_TEST:
	case HAND_TEST:
	case SHAPE_TEST:
		Decide();
		break;
	}
}

void GreySkeleton::HandleAction()
{
	switch (action)
	{
	case SHAPE_TEST:
	{
		if (frame % 90 == 0 )//% 20 == 0)
		{
			int possibleOffset = 100;
			int xOffset = (rand() % possibleOffset) - possibleOffset / 2;
			int yOffset = (rand() % possibleOffset) - possibleOffset / 2;
			shapePool.Appear(0, 400, sess->GetPlayerPos(0) + V2d(xOffset, yOffset), 60, 10, 10);
		}
		//for (int i = 0; i < 3; ++i)
		{
			/*int possibleOffset = 100;
			int xOffset = (rand() % possibleOffset) - possibleOffset / 2;
			int yOffset = (rand() % possibleOffset) - possibleOffset / 2;*/

			/*float min = 200;
			if (xOffset >= 0)
			{
			xOffset += min;
			}
			else if (xOffset < 0)
			{
			xOffset -= min;
			}

			if (yOffset >= 0)
			{
			yOffset += min;
			}
			else if (yOffset  < 0)
			{
			yOffset -= min;
			}*/

			//shapePool.Appear(0, 400, sess->GetPlayerPos(0) + V2d(xOffset, yOffset), 60);
		}
		break;
	}
	case THORN_TEST:
	{
		if (frame % 90 == 0)//% 20 == 0)
		{
			int possibleOffset = 100;
			int xOffset = (rand() % possibleOffset) - possibleOffset / 2;
			int yOffset = (rand() % possibleOffset) - possibleOffset / 2;
			shapePool.Appear(0, 400, sess->GetPlayerPos(0) + V2d(xOffset, yOffset), 60, 10, 10);
		}
	}
	}
}

void GreySkeleton::StartAction()
{
	switch (action)
	{
	case MOVE:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
		break;
	}
	case THORN_TEST:
	{
		PoiInfo *node;

		int thornType = rand() % 2;
		for (int i = 0; i < 3; ++i)
		{
			node = nodeGroupB.AlwaysGetNextNode();
			thornPool.Throw(thornType, node->pos, node->edge->Normal() );
		}
		
		
		//thornPool.Throw(0, nodeGroupB.AlwaysGetNextNode()->pos, PlayerDir());
		//thornPool.Throw(0, nodeGroupB.AlwaysGetNextNode()->pos, PlayerDir());
		//thornPool.Throw(0, GetPosition(), PlayerDir());
		break;
	}
	case HAND_TEST:
	{
		rightHand->Appear(GetPosition() + V2d(200, 0));
		break;
	}
	case SHAPE_TEST:
	{
		/*if (frame % 20 == 0)
		{
			int possibleOffset = 100;
			int xOffset = (rand() % possibleOffset) - possibleOffset / 2;
			int yOffset = (rand() % possibleOffset) - possibleOffset / 2;
			shapePool.Appear(0, 200, sess->GetPlayerPos(0) + V2d(xOffset, yOffset), 60);
		}*/
		//for (int i = 0; i < 3; ++i)
		{
			/*int possibleOffset = 100;
			int xOffset = (rand() % possibleOffset) - possibleOffset / 2;
			int yOffset = (rand() % possibleOffset) - possibleOffset / 2;*/

			/*float min = 200;
			if (xOffset >= 0)
			{
				xOffset += min;
			}
			else if (xOffset < 0)
			{
				xOffset -= min;
			}

			if (yOffset >= 0)
			{
				yOffset += min;
			}
			else if (yOffset  < 0)
			{
				yOffset -= min;
			}*/

			//shapePool.Appear(0, 400, sess->GetPlayerPos(0) + V2d(xOffset, yOffset), 60);
		}
		break;
	}
	case BOMB_TEST:
	{
		bombPool.Throw(BeamBomb::BOMB_SINGLE, GetPosition() + V2d(200, 0), V2d(1, 1));
		break;
	}
	}
}

void GreySkeleton::SetupPostFightScenes()
{
	if (postFightScene == NULL)
	{
		postFightScene = new FinalSkeletonPostFightScene;
		postFightScene->greySkeleton = this;
		postFightScene->Init();
	}
}

void GreySkeleton::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_SKELETON2, "A"));
	nodeGroupB.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_SKELETON2, "B"));
}

bool GreySkeleton::IsDecisionValid(int d)
{
	return true;
}

bool GreySkeleton::IsEnemyMoverAction(int a)
{
	return a == MOVE;
}

void GreySkeleton::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
}

void GreySkeleton::UpdateSprite()
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

void GreySkeleton::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	//orbPool.Draw(target);
}

int GreySkeleton::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void GreySkeleton::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	//d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);
}

void GreySkeleton::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	//fireCounter = d.fireCounter;

	bytes += sizeof(MyData);
}

int GreySkeleton::GetNumSimulationFramesRequired()
{
	return 0;
}