#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Bird.h"
#include "Actor.h"
#include "SequenceW2.h"
#include "SequenceW5.h"
#include "SequenceW7.h"
#include "Enemy_Bat.h"

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



Bird::Bird(ActorParams *ap)
	:Boss(EnemyType::EN_BIRDBOSS, ap),
	shurPool(this),
	batSummonGroup(this, new BasicAirEnemyParams(sess->types["bat"], 1), 2, 1, 1),
	nodeGroupA(Color::Magenta),
	nodeGroupB(Color::Yellow)
{
	StageSetup(4, 4);

	SetNumActions(A_Count);
	SetEditorActions(PUNCH, 0, 0);

	level = ap->GetLevel();

	actionLength[PUNCH] = 14;
	actionLength[SUMMON] = 60;
	actionLength[SHURIKEN_SHOTGUN] = 60;
	actionLength[UNDODGEABLE_SHURIKEN] = 60;
	actionLength[KICK] = 10;

	animFactor[PUNCH] = 3;
	animFactor[KICK] = 3;

	actionTargetOffsets[PUNCH] = V2d(100, 0);

	hitboxStartFrame[PUNCH] = 7;

	stageMgr.AddActiveOption(0, MOVE_CHASE, 2);
	//stageMgr.AddActiveOption(0, MOVE_NODE_LINEAR, 2);
	//stageMgr.AddActiveOption(0, MOVE_NODE_QUADRATIC, 2);

	stageMgr.AddActiveOption(1, MOVE_CHASE, 2);
	stageMgr.AddActiveOption(1, MOVE_NODE_LINEAR, 2);
	stageMgr.AddActiveOption(1, MOVE_NODE_QUADRATIC, 2);
	stageMgr.AddActiveOption(1, SUMMON, 2);

	stageMgr.AddActiveOption(2, MOVE_CHASE, 2);
	stageMgr.AddActiveOption(2, MOVE_NODE_LINEAR, 2);
	stageMgr.AddActiveOption(2, MOVE_NODE_QUADRATIC, 2);
	stageMgr.AddActiveOption(2, SUMMON, 2);
	stageMgr.AddActiveOption(2, SHURIKEN_SHOTGUN, 2);

	stageMgr.AddActiveOption(3, MOVE_CHASE, 2);
	stageMgr.AddActiveOption(3, MOVE_NODE_LINEAR, 2);
	stageMgr.AddActiveOption(3, MOVE_NODE_QUADRATIC, 2);
	stageMgr.AddActiveOption(3, SUMMON, 2);
	stageMgr.AddActiveOption(3, SHURIKEN_SHOTGUN, 2);
	stageMgr.AddActiveOption(3, UNDODGEABLE_SHURIKEN, 2);

	ts_move = sess->GetSizedTileset("Bosses/Bird/intro_256x256.png");
	ts_punch = sess->GetSizedTileset("Bosses/Bird/punch_256x256.png");
	ts_kick = sess->GetSizedTileset("Bosses/Bird/kick_256x256.png");
	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(32, 0, V2d(100, 0), V2d());

	CreateHitboxManager("Bosses/Bird");
	punchBody = hitboxManager->CreateBody("punch", &hitboxInfos[PUNCH]);

	LoadParams();

	//punchBody->hitboxInfo->hitsThroughInvincibility = true;

	postFightScene = NULL;
	postFightScene2 = NULL;
	postFightScene3 = NULL;

	ResetEnemy();
}

Bird::~Bird()
{
	if (postFightScene != NULL)
	{
		delete postFightScene;
	}
	if (postFightScene2 != NULL)
	{
		delete postFightScene2;
	}
	if (postFightScene3 != NULL)
	{
		delete postFightScene3;
	}

	delete punchBody;
}

void Bird::LoadParams()
{
	ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);
}

void Bird::UpdateHitboxes()
{
	if (action == PUNCH)
	{
		BasicUpdateHitboxes(); //for hurtboxes
		if (currHitboxes != NULL)
		{
			vector<CollisionBox> *cList = &(currHitboxes->GetCollisionBoxes(currHitboxFrame));
			if (cList != NULL)
				for (auto it = cList->begin(); it != cList->end(); ++it)
				{
					/*if (ground != NULL)
					{
						(*it).globalAngle = angle;
					}
					else
					{
						(*it).globalAngle = 0;
					}*/

					(*it).flipHorizontal = !facingRight;

					V2d pos = GetPosition();

					(*it).globalPosition = pos;
				}
		}

		hitboxInfos[action].flipHorizontalKB = !facingRight;
		//hitboxInfo->flipHorizontalKB = !facingRight;
	}
	else
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
}

void Bird::ResetEnemy()
{
	shurPool.Reset();
	batSummonGroup.Reset();

	fireCounter = 1000; //fire a shot when first starting
	facingRight = true;

	BossReset();

	StartFight();

	actionQueueIndex = 0;
	
	UpdateSprite();
}

void Bird::SetupPostFightScenes()
{
	if (level == 1)
	{
		if (postFightScene2 != NULL)
		{
			delete postFightScene2;
			postFightScene2 = NULL;
		}

		if (postFightScene3 != NULL)
		{
			delete postFightScene3;
			postFightScene3 = NULL;
		}

		if (postFightScene == NULL)
		{
			postFightScene = new BirdPostFightScene;
			postFightScene->bird = this;
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

		if (postFightScene3 != NULL)
		{
			delete postFightScene3;
			postFightScene3 = NULL;
		}

		if (postFightScene2 == NULL)
		{
			postFightScene2 = new BirdPostFight2Scene;
			postFightScene2->bird = this;
			postFightScene2->Init();
		}

	}
	else if (level == 3)
	{
		if (postFightScene != NULL)
		{
			delete postFightScene;
			postFightScene = NULL;
		}

		if (postFightScene2 != NULL)
		{
			delete postFightScene2;
			postFightScene2 = NULL;
		}

		if (postFightScene3 == NULL)
		{
			postFightScene3 = new BirdPostFight3Scene;
			postFightScene3->bird = this;
			postFightScene3->Init();
		}
	}
}

void Bird::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_BIRD, "A"));
	nodeGroupB.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_BIRD, "B"));
}

void Bird::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitboxInfo->hitsThroughInvincibility = true;

	//hitboxInfo->flipHorizontalKB = !facingRight;
	//hitBody.hitboxInfo = hitboxInfo;
}

void Bird::SetCommand(int index, BirdCommand &bc)
{
	actionQueue[index] = bc;
}

int Bird::SetLaunchersStartIndex(int ind)
{
	int currIndex = Enemy::SetLaunchersStartIndex( ind );

	currIndex = batSummonGroup.SetLaunchersStartIndex(currIndex);

	return currIndex;
}

void Bird::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	enemyMover.DebugDraw(target);
	nodeGroupA.Draw(target);
}

void Bird::DirectKill()
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

void Bird::FrameIncrement()
{
	Boss::FrameIncrement();

	++fireCounter;
}

void Bird::SequenceWait()
{
	SetAction(SEQ_WAIT);
	shurPool.Reset();
	batSummonGroup.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Bird::Wait(int numFrames)
{
	SetAction(WAIT);
	assert(numFrames > 0);
	actionLength[WAIT] = numFrames;
}

void Bird::StartFight()
{
	Wait(30);
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	HitboxesOff();
}

bool Bird::IsDecisionValid(int d)
{
	if (d == SUMMON && !batSummonGroup.CanSummon())
	{
		return false;
	}

	return true;
}

int Bird::ChooseActionAfterStageChange()
{
	switch (stageMgr.currStage)
	{
	case 1:
		return SUMMON;
	case 2:
		return SHURIKEN_SHOTGUN;
	case 3:
		return UNDODGEABLE_SHURIKEN;
	}

	return -1;
}

void Bird::StartAction()
{
	switch (action)
	{
	case MOVE_NODE_LINEAR:
	{
		int nodeIndex = nodeGroupA.picker.AlwaysGetNextOption();
		V2d nodePos = nodeGroupA.nodeVec->at(nodeIndex)->pos;

		//enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
		enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);
		break;
	}
	case MOVE_NODE_QUADRATIC:
	{
		int nodeIndex = nodeGroupA.picker.AlwaysGetNextOption();
		V2d nodePos = nodeGroupA.nodeVec->at(nodeIndex)->pos;
		enemyMover.SetModeNodeQuadratic(sess->GetPlayerPos(0), nodePos, CubicBezier(), 60);
		break;
	}

	case MOVE_CHASE:
	{
		enemyMover.SetModeChase(&sess->GetPlayer(0)->position, V2d(0, 0),
			10, .5, 60);
		break;
	}
	/*case RUSH:
	{
		enemyMover.SetModeNodeLinearConstantSpeed(GetPosition() + PlayerDir() * 600.0, CubicBezier(), 20);
		break;
	}*/
	}

	if (IsEnemyMoverAction(action))
	{
		int currStage = stageMgr.GetCurrStage();
		if (fireCounter >= 60
			&& ((IsEnemyMoverAction(prevAction) && currStage > 0)
				|| currStage == 0))
		{
			fireCounter = 0;
			//shurPool.Throw(GetPosition(), PlayerDir(), BirdShuriken::ShurikenType::SLIGHTHOMING);
		}
	}
}

//returns true if comboing
bool Bird::TryComboMove(V2d &comboPos, int comboMoveDuration)
{
	if (actionQueueIndex < 3)
	{
		BirdCommand nextAction = actionQueue[actionQueueIndex];

		V2d offset(-100, 0);
		if (!nextAction.facingRight)
		{
			offset.x = -offset.x;
		}

		SetAction(COMBOMOVE);
		HitboxesOff();

		actionLength[COMBOMOVE] = comboMoveDuration - (hitboxStartFrame[nextAction.action] * animFactor[nextAction.action] - 1);

		facingRight = nextAction.facingRight;

		enemyMover.SetModeNodeLinear(comboPos + offset, CubicBezier(), comboMoveDuration);

		return true;
	}
	else
	{
		HitboxesOff();
		Wait(60);

		return false;
	}
}

void Bird::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case SUMMON:
		case WAIT:
		case UNDODGEABLE_SHURIKEN:
		case SHURIKEN_SHOTGUN:
			Decide();
			break;
		case PUNCH:
			HitboxesOff();
			Decide();
			break;
		case COMBOMOVE:
		{
			BirdCommand nextAction = actionQueue[actionQueueIndex];
			SetAction(nextAction.action);
			++actionQueueIndex;
			break;
		}
		case KICK:
			frame = 0;
			break;
		case SEQ_WAIT:
			break;
		}
	}
}

void Bird::HandleAction()
{
	HitboxesOff();
	switch (action)
	{
	case MOVE_CHASE:
		if (enemyMover.GetActionProgress() > .5 && PlayerDist() < 400)
		{
			if (PlayerDir().x > 0)
			{
				facingRight = true;
			}
			else
			{
				facingRight = false;
			}

			V2d offset(-100, 0);
			if (!facingRight)
			{
				offset.x = -offset.x;
			}

			SetAction(PUNCH);

			BirdCommand bc;
			bc.action = PUNCH;
			bc.facingRight = true;

			actionQueueIndex = 0;

			SetCommand(0, bc);

			bc.facingRight = false;
			SetCommand(1, bc);
			SetCommand(2, bc);

			double chaseSpeed = 15;
			double accel = 2.0;//.8;

			enemyMover.SetModeChase(&sess->GetPlayer(0)->position, offset,
				chaseSpeed, accel, actionLength[PUNCH] * animFactor[PUNCH] - 10);
			enemyMover.velocity = PlayerDir() * chaseSpeed;
		}
		break;
	case SUMMON:
		if (frame == 20 && slowCounter == 1)
		{
			batSummonGroup.Summon();
		}
		break;
	case SHURIKEN_SHOTGUN:
	{
		if ((frame == 20 || frame == 50) && slowCounter == 1)
		{
			V2d pDir = PlayerDir();
			double spread = PI / 8.0;
			shurPool.Throw(GetPosition(), pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
			RotateCW(pDir, spread);
			shurPool.Throw(GetPosition(), pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
			RotateCCW(pDir, spread * 2);
			shurPool.Throw(GetPosition(), pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
		}
		break;
	}
	case UNDODGEABLE_SHURIKEN:
	{
		if (frame == 30 && slowCounter == 1)
		{
			shurPool.Throw(GetPosition(), PlayerDir(), BirdShuriken::ShurikenType::UNDODGEABLE);
		}
		break;
	}
	case COMBOMOVE:
	{
		break;
	}
	case PUNCH:
	{
		if (!actionHitPlayer)
		{
			SetHitboxes(punchBody, frame / animFactor[PUNCH]);
		}
		
		break;
	}
	}
}

bool Bird::IsEnemyMoverAction( int a)
{
	return a == MOVE_NODE_LINEAR || a == MOVE_NODE_QUADRATIC
		|| a == MOVE_CHASE;
}

void Bird::ActivatePostFightScene()
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
	else if (level == 3)
	{
		postFightScene3->Reset();
		sess->SetActiveSequence(postFightScene3);
	}
}

void Bird::UpdateSprite()
{
	switch (action)
	{
	case MOVE:
	case SUMMON:
	case COMBOMOVE:
	case MOVE_NODE_LINEAR:
	case MOVE_NODE_QUADRATIC:
	case MOVE_CHASE:
	case WAIT:
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
	}

	if (invincibleFrames == 0)
	{
		sprite.setColor(Color::White);
	}
	else
	{
		sprite.setColor(Color::Red);
	}

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Bird::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	shurPool.Draw(target);
}

void Bird::HandleHitAndSurvive()
{
	fireCounter = 0;
}

void Bird::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &batSummonGroup)
	{
		PoiInfo *summonNode;

		summonNode = nodeGroupB.AlwaysGetNextNode();

		e->startPosInfo.SetAerial(summonNode->pos);
	}
}


//Rollback functions
int Bird::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Bird::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
}

void Bird::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
}

