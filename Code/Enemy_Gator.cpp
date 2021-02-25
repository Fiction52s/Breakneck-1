#include "Session.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Gator.h"
#include "Actor.h"
#include "SequenceW5.h"

using namespace std;
using namespace sf;


Gator::Gator(ActorParams *ap)
	:Boss(EnemyType::EN_GATORBOSS, ap),
	swarmSummonGroup(this, new BasicAirEnemyParams(sess->types["swarm"], 1), 2, 2, 1)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	StageSetup(4, 4);

	ts_move = sess->GetSizedTileset("Bosses/Gator/dominance_384x384.png");

	actionLength[SUMMON] = 60;

	postFightScene = NULL;

	stageMgr.AddActiveOption(0, MOVE_CHASE, 2);
	stageMgr.AddActiveOption(0, MOVE_NODE_LINEAR, 2);
	stageMgr.AddActiveOption(0, MOVE_NODE_QUADRATIC, 2);
	stageMgr.AddActiveOption(0, SUMMON, 2);

	stageMgr.AddActiveOption(1, MOVE_CHASE, 2);
	stageMgr.AddActiveOption(1, MOVE_NODE_LINEAR, 2);
	stageMgr.AddActiveOption(1, MOVE_NODE_QUADRATIC, 2);

	stageMgr.AddActiveOption(2, MOVE_CHASE, 2);
	stageMgr.AddActiveOption(2, MOVE_NODE_LINEAR, 2);
	stageMgr.AddActiveOption(2, MOVE_NODE_QUADRATIC, 2);

	stageMgr.AddActiveOption(3, MOVE_CHASE, 2);
	stageMgr.AddActiveOption(3, MOVE_NODE_LINEAR, 2);
	stageMgr.AddActiveOption(3, MOVE_NODE_QUADRATIC, 2);


	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	ResetEnemy();
}

Gator::~Gator()
{
	if (postFightScene != NULL)
		delete postFightScene;
}

void Gator::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Gator::ResetEnemy()
{
	orbPool.Reset();
	swarmSummonGroup.Reset();

	BossReset();

	facingRight = true;

	HitboxesOff();

	StartFight();

	UpdateSprite();
}

bool Gator::TryComboMove(V2d &comboPos, int comboMoveDuration,
	int moveDurationBeforeStartNextAction,
	V2d &comboOffset)
{
	return false;
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

void Gator::ActionEnded()
{
	switch (action)
	{
	case WAIT:
	case MOVE_NODE_LINEAR:
	case MOVE_NODE_QUADRATIC:
	case MOVE_CHASE:
		Decide();
		break;
	case COMBOMOVE:
		SetNextComboAction();
		break;
	}
}

void Gator::HandleAction() 
{
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
		
	}
}

void Gator::StartAction()
{
	switch (action)
	{
	case MOVE_NODE_LINEAR:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
		orbPool.Throw(GetPosition(), nodePos, GatorWaterOrb::OrbType::NODE_GROW);
		break;

	}
	case MOVE_NODE_QUADRATIC:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeNodeQuadratic(targetPlayer->position, nodePos, CubicBezier(), 60);
		orbPool.Throw(GetPosition(), nodePos, GatorWaterOrb::OrbType::NODE_GROW);
		break;
	}
	case MOVE_CHASE:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeChase(&targetPlayer->position, V2d(0, 0),
			10, .5, 60);
		orbPool.Throw(GetPosition(), nodePos, GatorWaterOrb::OrbType::NODE_GROW);
		break;
	}
	}
}
void Gator::SetupPostFightScenes()
{
	if (postFightScene == NULL)
	{
		postFightScene = new GatorPostFightScene;
		postFightScene->gator = this;
		postFightScene->Init();
	}
}

void Gator::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_GATOR, "A"));
}

bool Gator::IsDecisionValid(int d)
{
	return true;
}

bool Gator::IsEnemyMoverAction(int a)
{
	return a == MOVE_NODE_LINEAR || a == MOVE_NODE_QUADRATIC
		|| a == MOVE_CHASE;
}

void Gator::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;
	orbPool.Reset();
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

void Gator::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
}

void Gator::UpdateSprite()
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

void Gator::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	orbPool.Draw(target);
}


void Gator::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &swarmSummonGroup)
	{
		PoiInfo *summonNode;

		summonNode = nodeGroupA.AlwaysGetNextNode();
		e->startPosInfo.SetGround(summonNode->poly,
			summonNode->edgeIndex, summonNode->edgeQuantity);
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