#include "Enemy_QueenFloatingBomb.h"
#include "Session.h"

QueenFloatingBomb::QueenFloatingBomb(/*ActorParams *ap*/)
	:Enemy(EnemyType::EN_QUEENFLOATINGBOMB, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(FLOATING, 0, 0);
	//preload
	sess->GetTileset("Enemies/bombexplode_512x512.png", 512, 512);

	CreateSurfaceMover(startPosInfo, 32, this);

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
	//currPosInfo.position = pos;
	surfaceMover->physBody.globalPosition = pos;
	surfaceMover->velocity = vel;
	action = FLOATING;
	frame = 0;
	
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
	surfaceMover->velocity = V2d();
	action = FLOATING;
	frame = 0;
	SetHurtboxes(&hurtBody, 0);
	SetHitboxes(&hitBody, 0);
}

void QueenFloatingBomb::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOATING)
	{
		action = EXPLODING;
		frame = 0;
	}

}
