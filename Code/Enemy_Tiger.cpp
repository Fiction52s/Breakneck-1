#include "Session.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Tiger.h"
#include "Actor.h"
#include "SequenceW4.h"
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


Tiger::Tiger(ActorParams *ap)
	:Boss(EnemyType::EN_TIGERBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE_GRIND, 0, 0);

	StageSetup(4, 4);

	level = ap->GetLevel();

	ts_move = sess->GetSizedTileset("Bosses/Coyote/coy_stand_80x64.png");
	sprite.setColor(Color::Red);

	stageMgr.AddActiveOption(0, MOVE_GRIND, 2);
	stageMgr.AddActiveOption(0, MOVE_JUMP, 2);

	stageMgr.AddActiveOption(1, MOVE_GRIND, 2);
	stageMgr.AddActiveOption(1, MOVE_JUMP, 2);

	stageMgr.AddActiveOption(2, MOVE_GRIND, 2);
	stageMgr.AddActiveOption(2, MOVE_JUMP, 2);

	stageMgr.AddActiveOption(3, MOVE_GRIND, 2);
	stageMgr.AddActiveOption(3, MOVE_JUMP, 2);

	postFightScene = NULL;
	postFightScene2 = NULL;

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	ResetEnemy();
}

Tiger::~Tiger()
{
	if (postFightScene != NULL)
	{
		delete postFightScene;
	}

	if (postFightScene2 != NULL)
	{
		delete postFightScene2;
	}
}

void Tiger::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Tiger::ResetEnemy()
{
	snakePool.Reset();

	BossReset();

	facingRight = true;

	HitboxesOff();

	StartFight();

	UpdateSprite();
}

void Tiger::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
}

bool Tiger::TryComboMove(V2d &comboPos, int comboMoveDuration,
	int moveDurationBeforeStartNextAction,
	V2d &comboOffset)
{
	return false;
}

int Tiger::ChooseActionAfterStageChange()
{
	return Boss::ChooseActionAfterStageChange();
}

void Tiger::ActivatePostFightScene()
{
	if (level == 1)
	{
		postFightScene->Reset();
		sess->SetActiveSequence(postFightScene);
	}
	else if (level == 2)
	{
		postFightScene2->Reset();
		sess->SetActiveSequence(postFightScene2);
	}
}

void Tiger::ActionEnded()
{
	switch (action)
	{
	case WAIT:
	case MOVE_GRIND:
	case MOVE_JUMP:
		Decide();
		break;
	case COMBOMOVE:
		SetNextComboAction();
		break;
	}
}

void Tiger::HandleAction()
{

}

void Tiger::StartAction()
{
	switch (action)
	{
	case MOVE_GRIND:
	{
		int gr = rand() % 2;

		double grindSpeed = 20;
		if (gr == 0)
		{
			enemyMover.SetModeGrind(grindSpeed, 120);
		}
		else if (gr == 1)
		{
			enemyMover.SetModeGrind(-grindSpeed, 120);
		}
		break;
	}
	case MOVE_JUMP:
	{
		enemyMover.currPosInfo.SetAerial();
		currPosInfo.SetAerial();
		PoiInfo *node = nodeGroupA.AlwaysGetNextNode();
		enemyMover.SetModeNodeProjectile(node->pos, V2d(0, 1.5), 200);
		enemyMover.SetDestNode(node);
		break;
	}
		
	}
}

void Tiger::SetupPostFightScenes()
{
	if (level == 1)
	{
		if (postFightScene2 != NULL)
		{
			delete postFightScene2;
			postFightScene2 = NULL;
		}

		if (postFightScene == NULL)
		{
			postFightScene = new TigerPostFightScene;
			postFightScene->tiger = this;
			postFightScene->Init();
		}
	}
	else if (level == 2)
	{

		if (postFightScene != NULL)
		{
			delete postFightScene;
			postFightScene = NULL;
		}

		postFightScene2 = new TigerPostFight2Scene;
		postFightScene2->tiger = this;
		postFightScene2->Init();
	}
}

void Tiger::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_TIGER, "A"));
}

bool Tiger::IsDecisionValid(int d)
{
	return true;
}

bool Tiger::IsEnemyMoverAction(int a)
{
	return a == MOVE_GRIND || a == MOVE_JUMP ;
}

void Tiger::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;
	snakePool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Tiger::StartFight()
{
	Wait(10);
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	frame = 0;
	HitboxesOff();
}

void Tiger::UpdateSprite()
{
	sprite.setTexture(*ts_move->texture);
	ts_move->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Tiger::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	snakePool.Draw(target);
}

int Tiger::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Tiger::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);
}

void Tiger::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);


	bytes += sizeof(MyData);
}