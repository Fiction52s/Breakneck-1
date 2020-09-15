#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CoyoteHelper.h"
#include "Actor.h"

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


CoyoteHelper::CoyoteHelper(ActorParams *ap)
	:Enemy(EnemyType::EN_COYOTEHELPER, ap)
{
	/*boost::filesystem::path p("Resources/Maps//W2//gateblank9.brknk");
	sess->bonusGame = new GameSession(saveFile, p);
	sess->bonusGame->SetParentGame(this);
	bonusGame->Load();

	currSession = this;
	pauseMenu->owner = this;*/

	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;
	reachPointOnFrame[COMBOMOVE] = 0;

	ts_move = sess->GetSizedTileset("Bosses/Coyote/coy_stand_80x64.png");

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

	LoadParams();

	//BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	sprite.setColor(Color::Black);

	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	ResetEnemy();
}

void CoyoteHelper::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void CoyoteHelper::UpdateHitboxes()
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

void CoyoteHelper::ResetEnemy()
{
	playerComboer.Reset();
	stopStartPool.Reset();
	enemyMover.Reset();

	fireCounter = 0;
	facingRight = true;

	action = WAIT;
	DefaultHitboxesOn();
	//SetHitboxes(NULL);
	waitFrames = 10;

	//action = PUNCH;
	//SetHitboxInfo(PUNCH);
	//DefaultHitboxesOn();

	hitPlayer = false;
	comboMoveFrames = 0;

	//actionQueueIndex = 0;




	frame = 0;

	UpdateSprite();
}

void CoyoteHelper::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

//void Coyote::SetCommand(int index, BirdCommand &bc)
//{
//	actionQueue[index] = bc;
//}

void CoyoteHelper::DebugDraw(sf::RenderTarget *target)
{
	playerComboer.DebugDraw(target);
	enemyMover.DebugDraw(target);
	Enemy::DebugDraw(target);
}

void CoyoteHelper::DirectKill()
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

void CoyoteHelper::FrameIncrement()
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

void CoyoteHelper::UpdatePreFrameCalculations()
{
	Actor *targetPlayer = sess->GetPlayer(targetPlayerIndex);

	if (playerComboer.CanPredict(targetPlayerIndex))
	{
		/*if (actionQueueIndex == 3)
		{
		dead = true;
		sess->RemoveEnemy(this);
		return;
		}*/

		playerComboer.UpdatePreFrameCalculations(targetPlayerIndex);
		targetPos = playerComboer.GetTargetPos();

		comboMoveFrames = targetPlayer->hitstunFrames - 1;//(hitBody.hitboxInfo->hitstunFrames - 1);
		counterTillAttack = comboMoveFrames - 10;

		//enemyMover.SetModeNodeJump(targetPos, 200);
		enemyMover.SetModeNodeProjectile(targetPos, V2d(0, 1.0), 200);
		//enemyMover.SetModeNodeLinear(targetPos, CubicBezier(), comboMoveFrames);

		//int nextAction = //actionQueue[actionQueueIndex].action + 1;
		//comboMoveFrames -= actionLength[nextAction] * animFactor[nextAction] - 10;

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

void CoyoteHelper::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case COMBOMOVE:
			frame = 0;
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	if (action == MOVE && enemyMover.IsIdle())
	{
		action = WAIT;
		waitFrames = 1;//10
	}
	else if (action == WAIT && waitFrames == 0)
	{
		int r = rand() % 3;

		auto &nodeVec = sess->GetBossNodeVector(BossFightType::FT_COYOTE, nodeAStr);
		int vecSize = nodeVec.size();
		int rNode = rand() % vecSize;

		V2d nodePos = nodeVec[rNode]->pos;

		V2d pPos = sess->GetPlayerPos(0);
		V2d pDir = normalize(pPos - GetPosition());

		enemyMover.currPosInfo.SetAerial();
		currPosInfo.SetAerial();

		if (r == 0)
		{
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			//enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);
			//stopStartPool.Throw(GetPosition(), pDir);
		}
		else if (r == 1)
		{
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			//enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);
			//enemyMover.SetModeNodeQuadratic(pPos, nodePos, CubicBezier(), 60);
			//stopStartPool.Throw(GetPosition(), pDir);
		}
		else if (r == 2)
		{
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			//enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);
			//enemyMover.SetModeChase(&sess->GetPlayer(0)->position, V2d(0, 0),
			//	10, .5, 60);
			//stopStartPool.Throw(GetPosition(), pDir);
		}
		else if (r == 3)
		{

		}


		action = MOVE;
		moveFrames = 60;
	}
	else if (action == COMBOMOVE)
	{
		if (comboMoveFrames == 0)
		{
			//action = actionQueue[actionQueueIndex].action + 1;
			//facingRight = actionQueue[actionQueueIndex].facingRight;
			SetHitboxInfo(action);
			//only have this on if i dont turn on hitboxes at the end of the movement.
			DefaultHitboxesOn();

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
		//if (!comboInterrupted)
		//	++actionQueueIndex;
		SetHitboxes(NULL, 0);

		//if (actionQueueIndex == 3)
		{

		}
	}

	hitPlayer = false;
}

void CoyoteHelper::IHitPlayer(int index)
{
	//hitPlayer = true;
	if (hitBody.hitboxInfo != NULL)
	{
		pauseFrames = hitBody.hitboxInfo->hitlagFrames;
	}
	else
	{
		pauseFrames = 0;
	}
}

void CoyoteHelper::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void CoyoteHelper::UpdateSprite()
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

void CoyoteHelper::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	stopStartPool.Draw(target);
}

void CoyoteHelper::HandleHitAndSurvive()
{
	fireCounter = 0;
}

int CoyoteHelper::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void CoyoteHelper::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
}

void CoyoteHelper::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
}

bool CoyoteHelper::CheckHitPlayer(int index)
{
	Actor *player = sess->GetPlayer(index);

	if (player == NULL)
		return false;
	

	if (currHitboxes != NULL )//&& currHitboxes->hitboxInfo != NULL)
	{
		Actor::HitResult hitResult = player->CheckIfImHit(currHitboxes, currHitboxFrame, HitboxInfo::AIRUP,//currHitboxes->hitboxInfo->hitPosType,
			GetPosition(), facingRight, false, false);
			//currHitboxes->hitboxInfo->canBeParried,
			//currHitboxes->hitboxInfo->canBeBlocked);

		if (hitResult != Actor::HitResult::MISS)
		{
			IHitPlayer(index);
			if (currHitboxes != NULL) //needs a second check in case ihitplayer changes the hitboxes
			{
				player->touchedCoyoteHelper = true;
				return true;
				//player->RestoreAirDash();
				//player->RestoreDoubleJump();
				//ClearRect(quad);
				//spawned = false;
				//sess->RemoveEnemy(this);
				/*player->ApplyHit(currHitboxes->hitboxInfo,
				NULL, hitResult, GetPosition());*/
			}
		}
	}


	return false;
}