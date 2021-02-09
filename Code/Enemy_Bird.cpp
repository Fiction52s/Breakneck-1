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
	:Enemy(EnemyType::EN_BIRDBOSS, ap), shurPool( this )
{
	SetNumActions(A_Count);
	SetEditorActions(PUNCH, 0, 0);

	level = ap->GetLevel();

	nodeDebugCircles = NULL;
	nodeAVec = NULL;
	targetPlayerIndex = 0;

	actionLength[PUNCH] = 14;
	animFactor[PUNCH] = 3;
	reachPointOnFrame[PUNCH] = 0;

	actionLength[SUMMON] = 60;

	actionLength[KICK] = 10;
	animFactor[KICK] = 3;
	reachPointOnFrame[KICK] = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;
	reachPointOnFrame[COMBOMOVE] = 0;

	

	ts_move = sess->GetSizedTileset("Bosses/Bird/intro_256x256.png");

	ts_punch = sess->GetSizedTileset("Bosses/Bird/punch_256x256.png");

	ts_kick = sess->GetSizedTileset("Bosses/Bird/kick_256x256.png");

	nodeAStr = "A";

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

	postFightScene = NULL;
	postFightScene2 = NULL;
	postFightScene3 = NULL;

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);


	batParams = new BasicAirEnemyParams(sess->types["bat"], 1);
	for (int i = 0; i < NUM_BATS; ++i)
	{
		bats[i] = (Bat*)batParams->GenerateEnemy();
		bats[i]->SetSummoner(this);
	}

	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

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

	if (nodeDebugCircles != NULL)
	{
		delete nodeDebugCircles;
	}

	for (int i = 0; i < NUM_BATS; ++i)
	{
		delete bats[i];
	}
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

void Bird::ResetEnemy()
{
	playerComboer.Reset();
	shurPool.Reset();
	enemyMover.Reset();

	fireCounter = 0;
	facingRight = true;

	currMaxActiveBats = NUM_BATS;
	numBatsToSummonAtOnce = 1;
	numActiveBats = 0;

	StartFight();

	hitPlayer = false;
	comboMoveFrames = 0;

	actionQueueIndex = 0;
	
	for (int i = 0; i < NUM_BATS; ++i)
	{
		bats[i]->Reset();
	}

	UpdateSprite();
}

void Bird::Setup()
{
	Enemy::Setup();

	if (level == 1 )
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

	if (nodeAVec == NULL)
	{
		nodeAVec = sess->GetBossNodeVector(BossFightType::FT_BIRD, nodeAStr);
		assert(nodeAVec != NULL);

		int numNodes = nodeAVec->size();
		nodeDebugCircles = new CircleGroup(numNodes, 10, Color::Magenta, 6);

		nodePicker.ReserveNumOptions(numNodes);
		for (int i = 0; i < numNodes; ++i)
		{
			nodePicker.AddActiveOption(i);
			nodeDebugCircles->SetPosition(i, Vector2f(nodeAVec->at(i)->pos));
			
		}

		nodeDebugCircles->ShowAll();
		
	}


}

void Bird::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

void Bird::SetCommand(int index, BirdCommand &bc)
{
	actionQueue[index] = bc;
}

int Bird::SetLaunchersStartIndex(int ind)
{
	int currIndex = Enemy::SetLaunchersStartIndex( ind );

	for (int i = 0; i < NUM_BATS; ++i)
	{
		currIndex = bats[i]->SetLaunchersStartIndex(currIndex);
	}
	return currIndex;
}

void Bird::DebugDraw(sf::RenderTarget *target)
{
	playerComboer.DebugDraw(target);
	enemyMover.DebugDraw(target);
	nodeDebugCircles->Draw(target);
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
	++fireCounter;

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

void Bird::HandleSummonedChildRemoval(Enemy *e)
{
	numActiveBats--;

	assert(numActiveBats >= 0);
}

void Bird::UpdatePreFrameCalculations()
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

		comboMoveFrames = targetPlayer->hitstunFrames-1;//(hitBody.hitboxInfo->hitstunFrames - 1);
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

void Bird::Wait()
{
	action = SEQ_WAIT;
	frame = 0;
	shurPool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
//	facingRight = false;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Bird::StartFight()
{
	action = WAIT;
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	frame = 0;
	SetHitboxes(NULL);
	waitFrames = 10;
}

void Bird::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case PUNCH:
			frame = 0;
			break;
		case COMBOMOVE:
			frame = 0;
			break;
		case KICK:
			frame = 0;
			break;
		case SUMMON:
			action = WAIT;
			waitFrames = 0;
			break;
		case SEQ_WAIT:
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
		int r = rand() % 4;

		auto *nodeVec = sess->GetBossNodeVector(BossFightType::FT_BIRD, nodeAStr);
		int vecSize = nodeVec->size();
		int rNode = rand() % vecSize;

		V2d nodePos = nodeVec->at(rNode)->pos;

		V2d pPos = sess->GetPlayerPos(0);
		V2d pDir = normalize( pPos - GetPosition());

		

		if (r == 0)
		{
			action = MOVE;
			moveFrames = 60;
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
			shurPool.Throw(GetPosition(), pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
		}
		else if (r == 1)
		{
			action = MOVE;
			moveFrames = 60;
			enemyMover.SetModeNodeQuadratic( pPos, nodePos, CubicBezier(), 60);
			shurPool.Throw(GetPosition(), pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
		}
		else if (r == 2)
		{
			action = MOVE;
			moveFrames = 60;
			enemyMover.SetModeChase(&sess->GetPlayer(0)->position, V2d(0, 0),
				10, .5, 60);
			shurPool.Throw(GetPosition(), pDir, BirdShuriken::ShurikenType::SLIGHTHOMING);
		}
		else if (r == 3)
		{
			action = SUMMON;
			frame = 0;
		}

		
		
	}
	else if (action == COMBOMOVE )
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

	if (action == SUMMON && frame == 20 && slowCounter == 1)
	{
		if (frame == 20 && slowCounter == 1)
		{
			int currSummoned = 0;
			PoiInfo *summonNode;
			for (int i = 0; i < NUM_BATS; ++i)
			{
				if (!bats[i]->active)
				{
					summonNode = nodeAVec->at(nodePicker.AlwaysGetNextOption());

					bats[i]->spawned = false;
					bats[i]->startPosInfo.SetAerial(summonNode->pos);

					sess->AddEnemy(bats[i]);
					++numActiveBats;
					++currSummoned;

					if (!CanSummonBat())
					{
						break;
					}
					else if (currSummoned == numBatsToSummonAtOnce)
					{
						break;
					}
				}
			}
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
		if( !comboInterrupted )
			++actionQueueIndex;
		SetHitboxes(NULL, 0);

		if (actionQueueIndex == 3)
		{

		}
	}

	hitPlayer = false;
}

bool Bird::CanSummonBat()
{
	return numActiveBats < currMaxActiveBats;
}

void Bird::IHitPlayer(int index)
{
	hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames;
}

void Bird::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void Bird::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 1)
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			if (hasMonitor && !suppressMonitor)
			{
				//sess->CollectKey();
			}

			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			ConfirmKill();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();
		}

		if (numHealth == 1)
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

		receivedHit = NULL;
	}
}

void Bird::UpdateSprite()
{
	switch (action)
	{
	case WAIT:
	case MOVE:
	case SUMMON:
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