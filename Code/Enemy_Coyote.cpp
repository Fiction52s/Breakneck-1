#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Coyote.h"
#include "Actor.h"
#include "SequenceW3.h"
#include "Enemy_Firefly.h"
#include "Session.h"

using namespace std;
using namespace sf;

Coyote::Coyote(ActorParams *ap)
	:Boss(EnemyType::EN_COYOTEBOSS, ap),
	fireflySummonGroup( this, 
		new BasicAirEnemyParams(sess->types["firefly"], 1),
		5, 2, 1 )
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	StageSetup(4, 4);

	actionLength[SUMMON] = 60;

	ts_move = sess->GetSizedTileset("Bosses/Coyote/coy_stand_80x64.png");

	postFightScene = NULL;

	stageMgr.AddActiveOption(0, MOVE, 2);
	stageMgr.AddActiveOption(0, SUMMON, 2);

	stageMgr.AddActiveOption(1, MOVE, 2);
	stageMgr.AddActiveOption(1, SUMMON, 2);

	stageMgr.AddActiveOption(2, MOVE, 2);
	stageMgr.AddActiveOption(2, SUMMON, 2);

	stageMgr.AddActiveOption(3, MOVE, 2);
	stageMgr.AddActiveOption(3, SUMMON, 2);

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

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	ResetEnemy();
}

Coyote::~Coyote()
{
	if (postFightScene != NULL)
	{
		delete postFightScene;
	}
}

void Coyote::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Coyote::ResetEnemy()
{
	stopStartPool.Reset();
	fireflySummonGroup.Reset();

	BossReset();
	facingRight = true;

	HitboxesOff();

	StartFight();

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	hitPlayer = false;

	frame = 0;

	UpdateSprite();
}


void Coyote::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;
	stopStartPool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	facingRight = false;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Coyote::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
}

void Coyote::StartFight()
{
	Wait(20);
	DefaultHitboxesOn();
	DefaultHurtboxesOn();
}

bool Coyote::TryComboMove(V2d &comboPos, int comboMoveDuration,
	int moveDurationBeforeStartNextAction,
	V2d &comboOffset)
{
	return false;
}

int Coyote::ChooseActionAfterStageChange()
{
	return Boss::ChooseActionAfterStageChange();
}

void Coyote::ActivatePostFightScene()
{
	if (level == 1)
	{
		postFightScene->Reset();
		sess->SetActiveSequence(postFightScene);
	}
}

void Coyote::ActionEnded()
{	
	switch (action)
	{
	case WAIT:
	case MOVE:
	case SUMMON:
		Decide();
		break;
	case SEQ_WAIT:
		frame = 0;
		break;
	case COMBOMOVE:
		frame = 0;
		break;
	}
}

void Coyote::HandleAction()
{
	switch (action)
	{
	case SUMMON:
	{
		if( frame == 20 && slowCounter == 1)
		{
			fireflySummonGroup.Summon();
		}
		break;
	}
		
	}
}

void Coyote::StartAction()
{
	switch (action)
	{
	case MOVE:
		enemyMover.currPosInfo.SetAerial();
		currPosInfo.SetAerial();
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
		stopStartPool.Throw(GetPosition(), PlayerDir());
		break;
	}
}

void Coyote::SetupPostFightScenes()
{
	if (postFightScene != NULL)
	{
		postFightScene = new CoyotePostFightScene;
		postFightScene->coy = this;
		postFightScene->Init();
	}
}

void Coyote::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_COYOTE, "A"));
}

bool Coyote::IsEnemyMoverAction(int a)
{
	return a == MOVE;
}

bool Coyote::IsDecisionValid(int d)
{
	if (d == SUMMON && !fireflySummonGroup.CanSummon())
	{
		return false;
	}

	return true;
}

void Coyote::UpdateSprite()
{
	/*switch (action)
	{
	case WAIT:
	case MOVE:
	case COMBOMOVE:
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 2, !facingRight);
		break;
	case PUNCH:
		sprite.setTexture(*ts_punch->texture);
		ts_punch->SetSubRect(sprite, frame / animFactor[action] + 14, !facingRight);
		break;
	case KICK:
		sprite.setTexture(*ts_kick->texture);
		ts_kick->SetSubRect(sprite, frame / animFactor[action] + 6, !facingRight);
		break;
	}*/

	sprite.setTexture(*ts_move->texture);
	ts_move->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Coyote::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	stopStartPool.Draw(target);
}

void Coyote::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &fireflySummonGroup)
	{
		PoiInfo *summonNode;

		summonNode = nodeGroupA.AlwaysGetNextNode();

		e->startPosInfo.SetAerial(summonNode->pos);
	}
}

//rollback
int Coyote::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Coyote::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	//d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);
}

void Coyote::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	//fireCounter = d.fireCounter;

	bytes += sizeof(MyData);
}

