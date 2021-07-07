#include "Enemy_QueenFloatingBomb.h"
#include "Session.h"

QueenFloatingBomb::QueenFloatingBomb(ActorParams *ap)
	:Enemy(EnemyType::EN_QUEENFLOATINGBOMB, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(FLOATING, 0, 0);
	//preload
	ts_explosion = GetSizedTileset(
		"Bosses/Crawler/bombexplode_512x512.png");

	CreateSurfaceMover(startPosInfo, 32, this);

	ts = GetSizedTileset("Bosses/Crawler/bomb_128x160.png");
	sprite.setTexture(*ts->texture);

	action = FLOATING;

	actionLength[FLOATING] = 9;
	actionLength[EXPLODING] = 4;

	animFactor[FLOATING] = 3;
	animFactor[EXPLODING] = 3;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;
	hitboxInfo->hType = HitboxInfo::BLUE;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;


	ResetEnemy();
}

void QueenFloatingBomb::Init(V2d &pos, V2d &vel)
{
	startPosInfo.position = pos;
	initVel = vel;
}

QueenFloatingBomb::~QueenFloatingBomb()
{
}

void QueenFloatingBomb::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case FLOATING:
			frame = 0;
			break;
		case EXPLODING:
			numHealth = 0;
			dead = true;
			spawned = false;
			sess->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
				ts_explosion, GetPosition(), false, 0, 10, 3, true);
			//cout << "deactivating " << this << " . currently : " << myPool->numActiveMembers << endl;
			//myPool->DeactivatePoolMember(this);
			break;
		}
	}

	switch (action)
	{
	case FLOATING:
		break;
	case EXPLODING:
		break;
	}

	switch (action)
	{
	case FLOATING:
		break;
	case EXPLODING:
		break;
	}
}

void QueenFloatingBomb::HandleNoHealth()
{

}

void QueenFloatingBomb::FrameIncrement()
{

}

void QueenFloatingBomb::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void QueenFloatingBomb::IHitPlayer(int index)
{
	if (action == FLOATING)
	{
		action = EXPLODING;
		frame = 0;
	}
}

void QueenFloatingBomb::UpdateSprite()
{
	switch (action)
	{
	case FLOATING:
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[FLOATING]));
		break;
	case EXPLODING:
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[EXPLODING]));
		break;
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void QueenFloatingBomb::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	if (!dead)
		surfaceMover->physBody.DebugDraw(CollisionBox::Physics, target);
}

void QueenFloatingBomb::HitTerrainAerial(Edge * e, double q)
{
	if (action == FLOATING)
	{
		surfaceMover->velocity = V2d(0, 0);
		action = EXPLODING;
		frame = 0;
	}
}

void QueenFloatingBomb::ResetEnemy()
{
	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->velocity = initVel;
	action = FLOATING;
	frame = 0;
	DefaultHurtboxesOn();
	DefaultHitboxesOn();
}

void QueenFloatingBomb::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOATING)
	{
		action = EXPLODING;
		frame = 0;
	}

}
