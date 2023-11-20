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
	nodeGroupA(1, Color::Magenta),
	nodeGroupB(1, Color::Yellow)
{
	SetNumActions(A_Count);
	SetEditorActions(PUNCH, 0, 0);

	StageSetup(8, 4);

	level = ap->GetLevel();

	//actionLength[PUNCH] = 14;
	actionLength[PUNCH] = 28;
	animFactor[PUNCH] = 1;//2;

	actionLength[PUNCH2] = 18;
	animFactor[PUNCH2] = 3;

	actionLength[SUMMON] = 60;
	actionLength[SHURIKEN_SHOTGUN_1] = 10;
	animFactor[SHURIKEN_SHOTGUN_1] = 3;

	actionLength[SHURIKEN_SHOTGUN_2] = 10;
	animFactor[SHURIKEN_SHOTGUN_2] = 3;

	actionLength[UNDODGEABLE_SHURIKEN] = 60;
	actionLength[KICK] = 10;

	actionLength[BASIC_SHURIKEN] = 10;
	animFactor[BASIC_SHURIKEN] = 3;

	//animFactor[PUNCH] = 3;
	
	animFactor[KICK] = 3;

	actionLength[GATHER_ENERGY_START] = 1;
	animFactor[GATHER_ENERGY_START] = 1;

	actionLength[GATHER_ENERGY_LOOP] = 1;

	actionLength[GATHER_ENERGY_END] = 1;
	animFactor[GATHER_ENERGY_END] = 1;

	hitOffsetMap[PUNCH] = V2d(100, 20);
	hitOffsetMap[PUNCH2] = V2d(62, -35);

	maxChargeLoopFrames = 20;

	idleAnimFactor = 8;

	chaseStartStage = 3;
	shotgunStartStage = 5;
	summonStartStage = 6;
	
	
	//stageMgr.AddActiveOptionToStages(0, SHURIKEN_SHOTGUN_1, 2);
	//stageMgr.AddActiveOptionToStages(0, GATHER_ENERGY_START, 2);

	stageMgr.AddActiveOptionToStages(0, MOVE_CHASE, 2);


	/*stageMgr.AddActiveOptionToStages(0, MOVE_NODE_LINEAR, 2);
	stageMgr.AddActiveOptionToStages(0, GATHER_ENERGY_START, 2);

	stageMgr.AddActiveOptionToStages(2, MOVE_NODE_QUADRATIC, 2);

	stageMgr.AddActiveOptionToStages(chaseStartStage, MOVE_CHASE, 2);

	stageMgr.AddActiveOptionToStages(shotgunStartStage, SHURIKEN_SHOTGUN_1, 2);

	stageMgr.AddActiveOptionToStages(summonStartStage, SUMMON, 2);*/

	waitFrames.resize(stageMgr.numStages);
	chargeFrames.resize(stageMgr.numStages);
	
	int j = 0;
	for (int i = 0; i < stageMgr.numStages; ++i)
	{
		j = (stageMgr.numStages -1)- i;
		waitFrames[i] = 5 + j * 3;
		chargeFrames[i] = 20 + j * 5;
	}
	/*waitFrames[0] = 30;
	waitFrames[1] = 25;
	waitFrames[2] = 20;
	waitFrames[3] = 15;
	waitFrames[4] = 10;
	waitFrames[5] = 20;
	waitFrames[6] = 20;
	waitFrames[7] = 20;*/

	
	


	/*stageMgr.AddActiveOption(0, MOVE_CHASE, 2);
	stageMgr.AddActiveOption(0, MOVE_NODE_LINEAR, 2);
	stageMgr.AddActiveOption(0, MOVE_NODE_QUADRATIC, 2);
	stageMgr.AddActiveOption(0, GATHER_ENERGY_START, 2);*/


	/*stageMgr.AddActiveOption(1, MOVE_CHASE, 2);
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
	stageMgr.AddActiveOption(3, UNDODGEABLE_SHURIKEN, 2);*/

	ts_idle = GetSizedTileset("Enemies/Bosses/Bird/bird_idle_160x160.png");
	ts_punch = GetSizedTileset("Enemies/Bosses/Bird/bird_punch_256x256.png");
	ts_punch2 = GetSizedTileset("Enemies/Bosses/Bird/bird_punch_2_256x256.png");
	ts_kick = GetSizedTileset("Enemies/Bosses/Bird/kick_256x256.png");
	ts_charge = GetSizedTileset("Enemies/Bosses/Bird/bird_charge_160x256.png");
	ts_throw = GetSizedTileset("Enemies/Bosses/Bird/bird_throw_256x256.png");

	BasicCircleHurtBodySetup(16);
	//BasicCircleHitBodySetup(32, 0, V2d(100, 0), V2d());

	LoadParams();

	CreateHitboxManager("Enemies/Bosses/Bird");
	SetupHitboxes(PUNCH, "punch1");
	SetupHitboxes(PUNCH2, "punch2");

	attackPicker.AddActiveOption(PUNCH);
	attackPicker.AddActiveOption(PUNCH2);
	//attackPicker.AddActiveOption(PUNCH);

	myBonus = NULL;

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

	if (myBonus != NULL)
		delete myBonus;
}

void Bird::LoadParams()
{
	ifstream is;
	is.open("Resources/Enemies/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["punch2"], hitboxInfos[PUNCH2]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);
}

void Bird::ResetEnemy()
{
	if (myBonus != NULL)
	{
		myBonus->RestartLevel();
	}

	shurPool.Reset();
	batSummonGroup.Reset();

	fireCounter = 1000; //fire a shot when first starting
	facingRight = true;

	BossReset();

	StartFight();

	idleCounter = 0;
	
	UpdateSprite();
}

void Bird::SetupPostFightScenes()
{
	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();
		assert(myBonus == NULL);
		myBonus = game->CreateBonus("FinishedScenes/W2/birdpost");
	}
	else
	{
		myBonus = NULL;
	}


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
			//postFightScene->bird = this;
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
			//postFightScene2->bird = this;
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
			//postFightScene3->bird = this;
			postFightScene3->Init();
		}
	}
}

void Bird::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_BIRD, "A"));
	nodeGroupB.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_BIRD, "B"));
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

void Bird::FrameIncrement()
{
	Boss::FrameIncrement();


	if (action == GATHER_ENERGY_LOOP)
	{
		//cout << "currchargeframe: " << currChargeLoopFrame << endl;
		++currChargeLoopFrame;
		if (currChargeLoopFrame == chargeFrames[stageMgr.currStage])
		{
			SetAction(GATHER_ENERGY_END);
		}
	}

	++fireCounter;
}

void Bird::SeqWait()
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
	if (prevDecision == GATHER_ENERGY_START && d == GATHER_ENERGY_START)
	{
		return false;
	}
	if (prevDecision == SHURIKEN_SHOTGUN_1 && d == SHURIKEN_SHOTGUN_1)
	{
		return false;
	}

	return true;
}

int Bird::ChooseActionAfterStageChange()
{
	if (stageMgr.currStage == summonStartStage)
	{
		return SUMMON;
	}
	else if (stageMgr.currStage == shotgunStartStage)
	{
		return SHURIKEN_SHOTGUN_1;
	}
	else if (stageMgr.currStage == chaseStartStage)
	{
		return MOVE_CHASE;
	}

	return -1;
}

void Bird::StartAction()
{
	switch (action)
	{
	case MOVE_NODE_LINEAR:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;

		

		//enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
		enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);

		SetAction(BASIC_SHURIKEN);
		break;
	}
	/*case MOVE_NODE_LINEAR_BASIC_SHURIKEN:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);
		break;
	}*/
	case MOVE_NODE_QUADRATIC:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeNodeQuadratic(sess->GetPlayerPos(0), nodePos, CubicBezier(), 60);

		SetAction(BASIC_SHURIKEN);
		//shurPool.Throw(GetPosition(), PlayerDir(), BirdShuriken::ShurikenType::SLIGHTHOMING);
		break;
	}
	/*case MOVE_NODE_QUADRATIC_BASIC_SHURIKEN:
	{
		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);
		break;
	}*/
	case MOVE_CHASE:
	{
		enemyMover.SetModeChase(&sess->GetPlayer(0)->position, V2d(0, 0),
			10, .5, 60);
		break;
	}
	case TEST_POST:
	{
		GameSession *game = GameSession::GetSession();

		if (game != NULL)
		{
			sess->RemoveBoss(this);
			game->SetBonus(myBonus, GetPosition());
		}
		break;
	}
	case SHURIKEN_SHOTGUN_1:
	case SHURIKEN_SHOTGUN_2:
	{
		if (PlayerDir().x > 0)
		{
			facingRight = true;
		}
		else if (PlayerDir().x < 0)
		{
			facingRight = false;
		}
		break;
	}
	/*case RUSH:
	{
		enemyMover.SetModeNodeLinearConstantSpeed(GetPosition() + PlayerDir() * 600.0, CubicBezier(), 20);
		break;
	}*/
	//case MOVE_NODE_QUADRATIC_BASIC_SHURIKEN:
	//case MOVE_NODE_LINEAR_BASIC_SHURIKEN:
	case BASIC_SHURIKEN:
	{
		if (PlayerDir().x > 0)
		{
			facingRight = true;
		}
		else if (PlayerDir().x < 0)
		{
			facingRight = false;
		}
		
		break;
	}
	}

	/*if (IsEnemyMoverAction(action))
	{
		int currStage = stageMgr.GetCurrStage();
		if (fireCounter >= 60
			&& ((IsEnemyMoverAction(prevAction) && currStage > 0)
				|| currStage == 0))
		{
			fireCounter = 0;
			shurPool.Throw(GetPosition(), PlayerDir(), BirdShuriken::ShurikenType::SLIGHTHOMING);
		}
	}*/
}

//returns true if comboing
bool Bird::TryComboMove(V2d &comboPos, int comboMoveDuration, int moveDurationBeforeStartNextAction, int framesRemaining, V2d &comboOffset )
{
	if (moveDurationBeforeStartNextAction < 0)
	{
		//TODO: make the bird combo off of this. usually means hitstun is interrupted.
		return false;
	}
	nextAction = COMBOMOVE;
	actionLength[COMBOMOVE] = moveDurationBeforeStartNextAction - framesRemaining;

	if (actionLength[COMBOMOVE] < 0)
	{
		assert(actionLength[COMBOMOVE] > 0);
	}


	enemyMover.SetModeNodeLinear(comboPos + comboOffset, CubicBezier(), comboMoveDuration);

	return true;
}

void Bird::ActionEnded()
{
	switch (action)
	{
	case BASIC_SHURIKEN:
		action = prevAction;
		break;
	case MOVE_CHASE:
	case MOVE_NODE_LINEAR:
	case MOVE_NODE_QUADRATIC:
		Wait(waitFrames[stageMgr.currStage]);
		break;
	case SUMMON:
	case WAIT:
	case UNDODGEABLE_SHURIKEN:
	case SHURIKEN_SHOTGUN_2:
	//case MOVE_CHASE:
	//case MOVE_NODE_LINEAR:
	//case MOVE_NODE_QUADRATIC:
		Decide();
		break;
	case SHURIKEN_SHOTGUN_1:
		SetAction(SHURIKEN_SHOTGUN_2);
		break;
	case PUNCH:
		if (TrySetActionToNextAction())
		{
			facingRight = GetCurrCommand().facingRight;
		}
		else
		{
			Decide();
		}
		break;
	case PUNCH2:
		if (TrySetActionToNextAction())
		{
			facingRight = GetCurrCommand().facingRight;
		}
		else
		{
			Decide();
		}
		break;
	case COMBOMOVE:
	{
		SetNextComboAction();
		break;
	}
	case KICK:
		frame = 0;
		break;
	case SEQ_WAIT:
		break;
	case GATHER_ENERGY_START:
	{
		//cout << "looping" << endl;
		SetAction(GATHER_ENERGY_LOOP);
		currChargeLoopFrame = 0;
		break;
	}
	case GATHER_ENERGY_LOOP:
	{
		frame = 0;
		break;
	}
	case GATHER_ENERGY_END:
	{
		Wait(waitFrames[stageMgr.currStage]);
		//Decide();
		/*if (targetGroup.numActiveEnemies == 0)
		{
			SetAction(SUMMON_FLAME_TARGETS);
		}
		else
		{
			SetAction(HEAT_FLAME_TARGETS);
		}*/

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

			int attackAction = attackPicker.AlwaysGetNextOption();//PUNCH2;
			SetAction(attackAction);

			int attackActionTotalLength = actionLength[attackAction] 
				* animFactor[attackAction];

			BossCommand bc;
			bc.action = attackPicker.AlwaysGetNextOption();;
			bc.facingRight = true;

			ResetCommands();
			
			
			QueueCommand(bc);

			bc.action = attackPicker.AlwaysGetNextOption();

			bc.facingRight = false;
			QueueCommand(bc);

			bc.action = attackPicker.AlwaysGetNextOption();

			QueueCommand(bc);

			double chaseSpeed = 15;
			double accel = 2.0;//.8;

			enemyMover.SetModeChase(&sess->GetPlayer(0)->position, offset,
				chaseSpeed, accel, attackActionTotalLength - 10);
			enemyMover.velocity = PlayerDir() * chaseSpeed;
		}
		break;
	case SUMMON:
		if (frame == 20 && slowCounter == 1)
		{
			batSummonGroup.Summon();
		}
		break;
	case SHURIKEN_SHOTGUN_2:
	case SHURIKEN_SHOTGUN_1:
	{
		//if ((frame == 20 || frame == 50) && slowCounter == 1)
		if (frame == 3 * animFactor[action] && slowCounter == 1)
		{
			V2d shurikenOffset(65, -41);
			if (!facingRight)
			{
				shurikenOffset.x = -shurikenOffset.x;
			}

			V2d pDir = PlayerDir( shurikenOffset, V2d());
			V2d myPos = GetPosition() + shurikenOffset;
			double spread = PI / 8.0;
			shurPool.Throw(myPos, pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
			RotateCW(pDir, spread);
			shurPool.Throw(myPos, pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
			RotateCCW(pDir, spread * 2);
			shurPool.Throw(myPos, pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
		}
		break;
	}
	case BASIC_SHURIKEN:
	{
		if (frame == 3 * animFactor[action] && slowCounter == 1)
		{
			V2d shurikenOffset(65, -41);
			if (!facingRight)
			{
				shurikenOffset.x = -shurikenOffset.x;
			}

			V2d pDir = PlayerDir(shurikenOffset, V2d());
			V2d myPos = GetPosition() + shurikenOffset;
			shurPool.Throw(myPos, pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
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
		SetBasicActiveHitbox();
		break;
	}
	case PUNCH2:
	{
		SetBasicActiveHitbox();
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
		sprite.setTexture(*ts_idle->texture);
		ts_idle->SetSubRect(sprite, idleCounter / idleAnimFactor, !facingRight);
		//idle is facing left by default. will be changed soon

		++idleCounter;
		if (idleCounter == 9 * idleAnimFactor)
		{
			idleCounter = 0;
		}
		break;
	case PUNCH:
		sprite.setTexture(*ts_punch->texture);
		ts_punch->SetSubRect(sprite, frame / animFactor[action], !facingRight);
		break;
	case PUNCH2:
		ts_punch2->SetSpriteTexture(sprite);
		ts_punch2->SetSubRect(sprite, frame / animFactor[action], !facingRight);
		break;
	case KICK:
		sprite.setTexture(*ts_kick->texture);
		ts_kick->SetSubRect(sprite, frame / animFactor[action] + 6, !facingRight);
		break;
	case GATHER_ENERGY_START:
	case GATHER_ENERGY_LOOP:
	case GATHER_ENERGY_END:
	{
		ts_charge->SetSpriteTexture(sprite);
		ts_charge->SetSubRect(sprite, 0, !facingRight);
		break;
	}
	case SHURIKEN_SHOTGUN_2:
	case SHURIKEN_SHOTGUN_1:
	case BASIC_SHURIKEN:
	{
		int f = min(frame / animFactor[action], 7);
		ts_throw->SetSpriteTexture(sprite);
		ts_throw->SetSubRect(sprite, f, !facingRight);
		break;
	}
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
	return sizeof(MyData);// +launchers[0]->GetNumStoredBytes();
}

void Bird::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	//launchers[0]->StoreBytes(bytes);
}

void Bird::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	//launchers[0]->SetFromBytes(bytes);
}

