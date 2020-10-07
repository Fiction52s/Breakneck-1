#include "Enemy_QueenFloatingBomb.h"
#include "Session.h"

QueenFloatingBomb::QueenFloatingBomb(ActorParams *ap, ObjectPool *p_myPool, int index)
	:Enemy(EnemyType::EN_QUEENFLOATINGBOMB, ap),//, false, 1, false),
	PoolMember(index), myPool(p_myPool)
{
	//preload
	sess->GetTileset("Enemies/bombexplode_512x512.png", 512, 512);

	mover = new SurfaceMover(NULL, 0, 32);
	mover->surfaceHandler = this;
	mover->SetSpeed(0);

	ts = sess->GetTileset("Enemies/bomb_128x160.png", 128, 160);
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

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;


	ResetEnemy();
}

void QueenFloatingBomb::Init(V2d pos, V2d vel)
{
	currPosInfo.position = pos;
	mover->velocity = vel;
	action = FLOATING;
	frame = 0;
	mover->physBody.globalPosition = GetPosition();
}

QueenFloatingBomb::~QueenFloatingBomb()
{
	delete mover;
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
			sess->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
				sess->GetTileset("Enemies/bombexplode_512x512.png", 512, 512),
				GetPosition(), false, 0, 10, 3, true);
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
		mover->physBody.DebugDraw(CollisionBox::Physics, target);
}

void QueenFloatingBomb::HitTerrainAerial(Edge * e, double q)
{
	if (action == FLOATING)
	{
		mover->velocity = V2d(0, 0);
		action = EXPLODING;
		frame = 0;
	}
}

void QueenFloatingBomb::ResetEnemy()
{
	mover->ground = NULL;
	mover->edgeQuantity = 0;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);
	action = FLOATING;
	frame = 0;
	SetHurtboxes(&hurtBody, 0);
	SetHitboxes(&hitBody, 0);
}

void QueenFloatingBomb::UpdateEnemyPhysics()
{
	if (!dead)
	{
		mover->Move(slowMultiple, numPhysSteps);
		//position = mover->physBody.globalPosition;
	}
}

void QueenFloatingBomb::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOATING)
	{
		action = EXPLODING;
		frame = 0;
	}

}
