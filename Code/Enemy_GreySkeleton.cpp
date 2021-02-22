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

	ts_move = sess->GetSizedTileset("Bosses/Gator/dominance_384x384.png");
	sprite.setColor(Color::Green);

	postFightScene = NULL;

	stageMgr.AddActiveOption(0, MOVE, 2);

	stageMgr.AddActiveOption(1, MOVE, 2);

	stageMgr.AddActiveOption(2, MOVE, 2);

	stageMgr.AddActiveOption(3, MOVE, 2);

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	ResetEnemy();
}

GreySkeleton::~GreySkeleton()
{
	if (postFightScene != NULL)
		delete postFightScene;
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

	BossReset();

	StartFight();

	HitboxesOff();

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
		Decide();
		break;
	}
}

void GreySkeleton::HandleAction()
{

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