#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Coyote.h"
#include "Actor.h"
#include "SequenceW3.h"
#include "Enemy_Firefly.h"
#include "Session.h"
#include "Enemy_BabyScorpion.h"
#include "GameSession.h"
#include "Enemy_PulseAttack.h"

using namespace std;
using namespace sf;

Coyote::Coyote(ActorParams *ap)
	:Boss(EnemyType::EN_COYOTEBOSS, ap),
	nodeGroupA( 2 ),
	fireflySummonGroup( this, 
		new BasicAirEnemyParams(sess->types["firefly"], 1),
		5, 2, 1 ),
	babyScorpionGroup(this,
		new BasicAirEnemyParams(sess->types["babyscorpion"], 1),
		6, 6, 1)
{
	SetLevel(ap->GetLevel());

	BabyScorpion *bs = NULL;
	for (int i = 0; i < babyScorpionGroup.numTotalEnemies; ++i)
	{
		bs = (BabyScorpion*)babyScorpionGroup.enemies[i];
		bs->stopStartPool = &stopStartPool;
	}




	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	//StageSetup(8, 4);
	StageSetup(1, 3);

	actionLength[SUMMON] = 60;
	actionLength[DANCE_PREP] = 60;
	actionLength[DANCE] = 360;
	actionLength[PATTERN_BULLETS] = 60;
	actionLength[PATTERN_PULSE] = 60;

	patternFlickerFrames = 30;
	numPatternMoves = 3;

	
	

	ts_move = GetSizedTileset("Enemies/Bosses/Coyote/coy_stand_80x64.png");

	pulsePool = new PulseAttackPool(5);

	postFightScene = NULL;

	pattern.reserve(6);
	pattern.resize(numPatternMoves);

	patternType.reserve(6);
	patternType.resize(numPatternMoves);



	//stageMgr.AddActiveOptionToStages(0, PLAN_PATTERN, 2);
	stageMgr.AddActiveOption(0, TEST_POST, 1);




	//stageMgr.AddActiveOption(0, MOVE, 2);
	//stageMgr.AddActiveOption(0, RUSH, 2);

	//stageMgr.AddActiveOption(0, DANCE_PREP, 2);


	//stageMgr.AddActiveOptionToStages(0, PLAN_PATTERN, 2);

	//stageMgr.AddActiveOptionToStages(5, SUMMON, 2);
	//stageMgr.AddActiveOption(0, PLAN_PATTERN, 2);
	//stageMgr.AddActiveOption(0, TEST_POST, 1);

	//stageMgr.AddActiveOption(1, PLAN_PATTERN, 2);
	////stageMgr.AddActiveOption(1, MOVE, 2);
	////stageMgr.AddActiveOption(1, SUMMON, 2);

	//stageMgr.AddActiveOption(2, MOVE, 2);
	//stageMgr.AddActiveOption(2, SUMMON, 2);

	//stageMgr.AddActiveOption(3, MOVE, 2);
	//stageMgr.AddActiveOption(3, SUMMON, 2);

	

	SetNumLaunchers(2);
	launchers[0] = new Launcher(this, BasicBullet::OWL, 16, 3, GetPosition(), V2d(1, 0), PI / 12, 300);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->damage = 60;
	launchers[0]->Reset();

	launchers[1] = new Launcher(this, BasicBullet::OWL, 6*6, 6, GetPosition(), V2d(1, 0), PI / 3, 300);
	launchers[1]->SetBulletSpeed(15);
	launchers[1]->hitboxInfo->damage = 60;
	launchers[1]->Reset();

	patternPreview.setFillColor(Color::Magenta);
	patternPreview.setRadius(60);
	patternPreview.setOrigin(patternPreview.getLocalBounds().width / 2,
		patternPreview.getLocalBounds().height / 2);


	myBonus = NULL;
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

	if (myBonus != NULL)
		delete myBonus;

	delete pulsePool;
}

void Coyote::AddToGame()
{
	Enemy::AddToGame();

	fireflySummonGroup.SetEnemyIDAndAddToAllEnemiesVec();
	babyScorpionGroup.SetEnemyIDAndAddToAllEnemiesVec();
}

void Coyote::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Enemies/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}


void Coyote::ResetEnemy()
{
	if (myBonus != NULL)
	{
		myBonus->RestartLevel();
	}

	stopStartPool.Reset();
	fireflySummonGroup.Reset();
	pulsePool->Reset();

	currNode = NULL;
	patternIndex = -1;

	BossReset();
	facingRight = true;

	HitboxesOff();

	patternTypePicker.Reset();
	//patternTypePicker.AddActiveOption(PATTERN_RUSH);
	patternTypePicker.AddActiveOption(PATTERN_MOVE);
	//patternTypePicker.AddActiveOption(PATTERN_BULLETS);
	//patternTypePicker.AddActiveOption(PATTERN_PULSE);

	if (sess->preLevelScene == NULL) //fight testing
	{
		CameraShot *cs = sess->cameraShotMap["fightcam"];
		if (cs != NULL)
		{
			sess->cam.Set(Vector2f(cs->centerPos), cs->zoom, 0);
		}
	}

	StartFight();

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	rayCastInfo.tree = sess->terrainTree;

	hitPlayer = false;

	frame = 0;

	bounceCounter = -1;


	//enemyMover.currPosInfo.SetAerial();
	//currPosInfo.SetAerial();

	UpdateSprite();
}


void Coyote::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;
	stopStartPool.Reset();
	pulsePool->Reset();
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
	if (stageMgr.currStage == 2)
	{
		patternTypePicker.Reset();
		patternTypePicker.AddActiveOption(PATTERN_RUSH, 2);
		patternTypePicker.AddActiveOption(PATTERN_MOVE, 2);
		//patternTypePicker.AddActiveOption(PATTERN_BULLETS);
		//patternTypePicker.AddActiveOption(PATTERN_PULSE);
	}
	else if( stageMgr.currStage == 4 )
	{
		patternTypePicker.Reset();
		patternTypePicker.AddActiveOption(PATTERN_RUSH, 2);
		patternTypePicker.AddActiveOption(PATTERN_MOVE, 2);
		patternTypePicker.AddActiveOption(PATTERN_BULLETS,2);
	}
	else if (stageMgr.currStage == 6)
	{
		patternTypePicker.Reset();
		patternTypePicker.AddActiveOption(PATTERN_RUSH, 2);
		patternTypePicker.AddActiveOption(PATTERN_MOVE, 2);
		patternTypePicker.AddActiveOption(PATTERN_BULLETS, 2);
		patternTypePicker.AddActiveOption(PATTERN_PULSE,2);
	}

	
	//patternTypePicker.AddActiveOption(PATTERN_PULSE);


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
	case MOVE:
	{
		if (bounceCounter == 3)
		{
			Wait(30);
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = PlayerDir();
			launchers[0]->Fire();
		}
		else
		{
			++bounceCounter;
			SetAction(MOVE);
		}
		break;
	}
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
	case PATTERN_RUSH:
	{
		//SetAction(PATTERN_MOVE);
		if (patternIndex == numPatternMoves)
		{
			//SetAction(PATTERN_MOVE);
			//++patternIndex;
			Wait(30);
		}
		else
		{
			SetAction(PATTERN_MOVE);
		}

		break;
	}
	case PATTERN_BULLETS:
	{
		if (patternIndex == numPatternMoves)
		{
			//SetAction(PATTERN_MOVE);
			//++patternIndex;
			Wait(30);
		}
		else
		{
			SetAction(PATTERN_MOVE);
		}
		break;
	}
	case PATTERN_PULSE:
	{
		if (patternIndex == numPatternMoves)
		{
			//SetAction(PATTERN_MOVE);
			//++patternIndex;
			Wait(30);
		}
		else
		{
			SetAction(PATTERN_MOVE);
		}
		break;
	}
	case WAIT:
	case SUMMON:
	case RUSH:
		Decide();
		break;
	case SEQ_WAIT:
		frame = 0;
		break;
	case COMBOMOVE:
		frame = 0;
		break;
	case DANCE_PREP:
		SetAction(DANCE);
		break;
	case DANCE:
		babyScorpionGroup.Reset();
		Decide();
		break;
	}
}

void Coyote::HandleAction()
{
	switch (action)
	{
	case SUMMON:
	{
		if (frame == 20 && slowCounter == 1)
		{
			fireflySummonGroup.Summon();
		}
		break;
	}
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
				else if (patternType[mult] == PATTERN_RUSH)
				{
					patternPreview.setFillColor(Color::Red);
				}
				else if (patternType[mult] == PATTERN_BULLETS)
				{
					patternPreview.setFillColor(Color::Green);
				}
				else if (patternType[mult] == PATTERN_PULSE)
				{
					patternPreview.setFillColor(Color::Cyan);
				}
			}

			//babyScorpionGroup.Summon();
		}
		break;
	}
		
	}
}

void Coyote::StartAction()
{
	if (prevAction == MOVE && action != MOVE)
	{
		bounceCounter = -1;
	}

	switch (action)
	{
	case MOVE:
	{
		if (bounceCounter == -1)
		{
			bounceCounter = 0;
		}
		enemyMover.currPosInfo.SetAerial();
		currPosInfo.SetAerial();

		V2d nodePos = nodeGroupA.AlwaysGetNextNode()->pos;
		if (nodePos == GetPosition())
		{
			int xxx = 5;
		}
		

		enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
		//enemyMover.SetDestNode(currNode);
		/*if (length(nodePos - GetPosition()) < 100)
		{
			enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 20);
		}
		else
		{
			
		}*/
		stopStartPool.Throw(GetPosition(), PlayerDir());
		break;
	}
	case PATTERN_RUSH:
	case RUSH:
	{
		enemyMover.currPosInfo.SetAerial();
		currPosInfo.SetAerial();
		if (ExecuteRayCast(GetPosition() + PlayerDir() * 1.0 , GetPosition() + PlayerDir() * 3000.0));
		{
			V2d rayPos = rayCastInfo.GetRayHitPos();
			rushNode.SetGrounded(rayCastInfo.rcEdge->poly, rayCastInfo.rcEdge->edgeIndex, rayCastInfo.rcQuant);

			enemyMover.SetModeNodeLinearConstantSpeed(rayPos, CubicBezier(), 60);
			enemyMover.SetDestNode(&rushNode);
		}
		//stopStartPool.Throw(GetPosition(), PlayerDir());
		break;
	}
	case PATTERN_BULLETS:
	{
		launchers[1]->position = GetPosition();
		launchers[1]->facingDir = PlayerDir();
		launchers[1]->Fire();
		break;
	}
	case PATTERN_PULSE:
	{
		pulsePool->Pulse(0, GetPosition(), 20, 100, 2000, 60, Color::Yellow, Color::Yellow, NULL);
		break;
	}
	case PATTERN_MOVE:
	{
		enemyMover.currPosInfo.SetAerial();
		currPosInfo.SetAerial();
		currNode = pattern[patternIndex];
		enemyMover.SetModeNodeLinearConstantSpeed(currNode->pos, CubicBezier(), 60);
		enemyMover.SetDestNode(currNode);
		//stopStartPool.Throw(GetPosition(), PlayerDir());
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
				testNode = nodeGroupA.AlwaysGetNextNode();
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
	case DANCE_PREP:
	{
		babyScorpionGroup.Reset();
		enemyMover.SetModeNodeLinear(danceNode->pos, CubicBezier(), 30);
		for (int i = 0; i < babyScorpionGroup.currMaxActiveEnemies; ++i)
		{
			currBabyScorpPos = nodeGroupA.nodeVec->at(i)->pos;
			babyScorpionGroup.Summon();
		}
		//fireflySummonGroup.Summon();
		//fireflySummonGroup.Summon();
		break;
	}
	case TEST_POST:
	{
		GameSession *game = GameSession::GetSession();

		if (game != NULL)
		{
			sess->RemoveBoss(this);
			//oldPlayerPos = sess->GetPlayerPos(0);
			game->SetBonus(myBonus, GetPosition());
		}
		break;
	}
	}
}

void Coyote::SetupPostFightScenes()
{
	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();
		assert(myBonus == NULL);
		myBonus = game->CreateBonus("NewScenes/postcoyotefight");
	}
	else
	{
		myBonus = NULL;
	}


	if (postFightScene == NULL)
	{
		postFightScene = new CoyotePostFightScene;
		//postFightScene->coy = this;
		postFightScene->Init();
	}
}

void Coyote::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_COYOTE, "A"));

	danceNode = NULL;// sess->GetBossNodeVector(BossFightType::FT_COYOTE, "B")->at(0);
}

bool Coyote::IsEnemyMoverAction(int a)
{
	return a == MOVE || a == RUSH || a == PATTERN_MOVE || a == PATTERN_RUSH;
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


	/*if (action == MOVE_GRIND)
	{
		sprite.setRotation(0);
	}
	else*/
	{
		sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
	}

	sprite.setPosition(GetPositionF());

	float extra = 32;
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 + extra);
}

void Coyote::EnemyDraw(sf::RenderTarget *target)
{
	if (action == PLAN_PATTERN)
	{
		target->draw(patternPreview);
	}

	DrawSprite(target, sprite);
	stopStartPool.Draw(target);
}

void Coyote::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &fireflySummonGroup)
	{
		PoiInfo *summonNode;

		summonNode = nodeGroupA.AlwaysGetNextNode(1);

		e->startPosInfo.SetAerial(summonNode->pos);
	}
	else if (group == &babyScorpionGroup)
	{
		e->startPosInfo.SetAerial(currBabyScorpPos);
	}
}

void Coyote::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	if (b->bounceCount == 1)
	{
		b->Kill(-edge->Normal());
	}
	else
	{
		V2d en = edge->Normal();
		if (pos == edge->v0)
		{
			en = normalize(b->position - pos);
		}
		else if (pos == edge->v1)
		{
			en = normalize(b->position - pos);
		}
		double d = dot(b->velocity, en);
		V2d ref = b->velocity - (2.0 * d * en);
		b->velocity = ref;
		//cout << "ref: " << ref.x << ", " << ref.y << endl;
		//b->velocity = -b->velocity;
		b->bounceCount++;
		b->framesToLive = b->launcher->maxFramesToLive;
	}
}

int Coyote::SetLaunchersStartIndex(int ind)
{
	int currIndex = Enemy::SetLaunchersStartIndex(ind);

	currIndex = babyScorpionGroup.SetLaunchersStartIndex(currIndex);

	return currIndex;
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

