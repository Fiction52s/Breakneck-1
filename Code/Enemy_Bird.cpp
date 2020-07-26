#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Bird.h"
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


Bird::Bird(ActorParams *ap)
	:Enemy(EnemyType::EN_BIRDBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(PUNCH, 0, 0);

	actionLength[PUNCH] = 14;
	animFactor[PUNCH] = 3;

	actionLength[FOLLOWUPPUNCH] = 14;
	animFactor[FOLLOWUPPUNCH] = 3;

	bulletSpeed = 10;
	framesBetween = 60;

	Tileset *ts_basicBullets = sess->GetTileset("Enemies/bullet_64x64.png", 64, 64);

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::BAT, 16, 1, GetPosition(),
		V2d(1, 0), 0, 120, false, 0, 0, ts_basicBullets);
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->damage = 18;
	launchers[0]->playerIndex = 1;

	animationFactor = 5;

	ts_punch = sess->GetSizedTileset("Bosses/Bird/punch_256x256.png");
	sprite.setTexture(*ts_punch->texture);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 0;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 50;
	hitboxInfo->kbDir = normalize(V2d(1, -2));
	hitboxInfo->gravMultiplier = .5;
	hitboxInfo->invincibleFrames = 15;

	//LoadParams();

	//BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	move = ms.AddLineMovement(V2d(), V2d(), CubicBezier(), 0);


	ResetEnemy();
}

void Bird::LoadParams()
{
	ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j, punchHitboxInfo);
	HitboxInfo::SetupHitboxLevelInfo(j, kickHitboxInfo);
}

void Bird::UpdateHitboxes()
{
	BasicUpdateHitboxes();
}

void Bird::ResetEnemy()
{
	fireCounter = 0;
	facingRight = true;

	action = PUNCH;
	frame = 0;

	predict = false;
	hitPlayer = false;

	//DefaultHurtboxesOn();
	DefaultHitboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Bird::CalcTargetAfterHit()
{
	sess->ForwardSimulatePlayer(targetPlayerIndex, sess->GetPlayer(targetPlayerIndex)->hitstunFrames);
	targetPos = sess->GetPlayerPos(targetPlayerIndex);
	sess->RevertSimulatedPlayer(targetPlayerIndex);
}

void Bird::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	//V2d vel = b->velocity;
	//double angle = atan2( vel.y, vel.x );
	V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);

	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);
}

void Bird::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
	sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	b->launcher->DeactivateBullet(b);
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
}

void Bird::UpdatePreFrameCalculations()
{
	if (predict && action == FOLLOWUPPUNCH && frame == 1)
	{
		CalcTargetAfterHit();
		move->duration = (hitboxInfo->hitstunFrames - 1) * 10;
		move->start = GetPosition();
		move->end = targetPos;
		ms.Reset();
		predict = false;
	}
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
		case FOLLOWUPPUNCH:
			frame = 0;
			break;
		}
	}

	if (hitPlayer)
	{
		action = FOLLOWUPPUNCH;
		frame = 0;
	}



	V2d pDir = normalize(sess->GetPlayerPos(1) - GetPosition());
	//if( (fireCounter == 0 || fireCounter == 10 || fireCounter == 20/*framesBetween - 1*/) && slowCounter == 1 )// frame == 0 && slowCounter == 1 )
	if (slowCounter == 1)//&& action == FLY )
	{
		int f = fireCounter % 60;

		if (f % 5 == 0 && f >= 25 && f < 50)
		{
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = pDir;
			//launchers[0]->Fire();
		}
	}

	hitPlayer = false;
}

void Bird::IHitPlayer(int index)
{
	hitPlayer = true;
	predict = true;
	pauseFrames = hitboxInfo->hitlagFrames;
	/*V2d playerPos = sess->GetPlayerPos(index);
	if (playerPos.x > GetPosition().x)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}*/
}

void Bird::UpdateEnemyPhysics()
{
	if (action == FOLLOWUPPUNCH && !predict)
	{
		if (numPhysSteps == 1)
		{
			ms.Update(slowMultiple, 10);
		}
		else
		{
			ms.Update(slowMultiple);
		}

		currPosInfo.SetPosition(ms.position);
	}
	
}

void Bird::UpdateSprite()
{
	ts_punch->SetSubRect(sprite, frame / animFactor[action] + 14, !facingRight);
	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Bird::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
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