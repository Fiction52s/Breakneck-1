#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Gator.h"

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


Gator::Gator(ActorParams *ap)
	:Enemy(EnemyType::EN_GATOR, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(DOMINATION, 0, 0);

	actionLength[DOMINATION] = 60;
	animFactor[DOMINATION] = 1;

	bulletSpeed = 10;
	framesBetween = 60;

	Tileset *ts_basicBullets = sess->GetTileset("Enemies/bullet_64x64.png", 64, 64);

	numLaunchers = 1;
	launchers = new Launcher*[numLaunchers];
	launchers[0] = new Launcher(this, BasicBullet::BAT, 16, 1, GetPosition(),
		V2d(1, 0), 0, 120, false, 0, 0, ts_basicBullets );
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->damage = 18;
	launchers[0]->playerIndex = 1;

	animationFactor = 5;

	ts = sess->GetSizedTileset("Bosses/Gator/dominance_384x384.png");
	sprite.setTexture(*ts->texture);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	ResetEnemy();
}

void Gator::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	//V2d vel = b->velocity;
	//double angle = atan2( vel.y, vel.x );
	V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);

	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);
}

void Gator::BulletHitPlayer(BasicBullet *b)
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
	sess->PlayerApplyHit(b->launcher->hitboxInfo, b->launcher->playerIndex );
	b->launcher->DeactivateBullet(b);
}


void Gator::ResetEnemy()
{
	fireCounter = 0;
	facingRight = true;

	action = DOMINATION;
	frame = 0;

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Gator::DirectKill()
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

void Gator::FrameIncrement()
{
	++fireCounter;
}

void Gator::ProcessState()
{

	V2d pDir = normalize(sess->GetPlayerPos(1) - GetPosition());
	//if( (fireCounter == 0 || fireCounter == 10 || fireCounter == 20/*framesBetween - 1*/) && slowCounter == 1 )// frame == 0 && slowCounter == 1 )
	if (slowCounter == 1)//&& action == FLY )
	{
		int f = fireCounter % 60;

		if (f % 5 == 0 && f >= 25 && f < 50)
		{
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = pDir;
			launchers[0]->Fire();
		}
	}
}

void Gator::IHitPlayer(int index)
{
	V2d playerPos = sess->GetPlayerPos(index);
	if (playerPos.x > GetPosition().x)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}
}

void Gator::UpdateEnemyPhysics()
{
}

void Gator::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Gator::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void Gator::HandleHitAndSurvive()
{
	fireCounter = 0;
}