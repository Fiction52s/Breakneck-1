#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BabyScorpion.h"
#include "Enemy_CoyoteBullet.h"
#include "Actor.h"

using namespace std;
using namespace sf;

BabyScorpion::BabyScorpion(ActorParams *ap)
	:Enemy(EnemyType::EN_BABYSCORPION, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 2;
	actionLength[DISSIPATE] = 30;
	actionLength[DANCE] = 60;

	ts = GetSizedTileset("Bosses/Coyote/babyscorpion_64x64.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	ts_bulletExplode = GetSizedTileset("FX/bullet_explode3_64x64.png");

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::OWL, 16, 1, GetPosition(), V2d(1, 0),0/* PI / 12*/, 300);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->damage = 60;
	launchers[0]->Reset();

	stopStartPool = NULL;

	ResetEnemy();
}

void BabyScorpion::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

void BabyScorpion::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		ConfirmHitNoKill();

		if (receivedHit->hType == HitboxInfo::COMBO)
		{
			comboHitEnemy->ComboKill(this);
		}

		action = DISSIPATE;
		frame = 0;
		HitboxesOff();
		HurtboxesOff();
	}
}

void BabyScorpion::ResetEnemy()
{
	//action = NEUTRAL;
	action = DANCE;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void BabyScorpion::Dance()
{
	action = DANCE;
	frame = 0;
}

void BabyScorpion::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case DISSIPATE:
			dead = true;
			break;
		case DANCE:
			break;
		}
	}
}

void BabyScorpion::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case NEUTRAL:
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		break;
	case DANCE:
		if (frame == 0 && slowCounter == 1)
		{
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = PlayerDir();//normalize( owner->GetPlayer( 0 )->position - position );
			launchers[0]->Fire();
			//stopStartPool->Throw(GetPosition(), PlayerDir());
		}
		break;
	}
}

void BabyScorpion::UpdateEnemyPhysics()
{
	/*V2d movementVec = velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;*/
}

void BabyScorpion::UpdateSprite()
{
	int trueFrame;
	switch (action)
	{
	case NEUTRAL:
		//sprite.setColor(Color::White);
		break;

		break;
	}

	ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void BabyScorpion::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void BabyScorpion::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	if (b->bounceCount == 0)
	{
		V2d norm = edge->Normal();
		double angle = atan2(norm.y, -norm.x);
		sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
		b->launcher->DeactivateBullet(b);
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


void BabyScorpion::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}

	b->launcher->DeactivateBullet(b);
}