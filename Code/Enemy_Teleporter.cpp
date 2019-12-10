#include "Enemy.h"
#include "Enemy_Teleporter.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "MainMenu.h"

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

Teleporter::Teleporter(GameSession *owner, Vector2i &pos, Vector2i &other, bool bothWays, bool p_secondary )
	:Enemy(owner, EnemyType::EN_TELEPORTER, false, 2, false)
{
	secondary = p_secondary;
	otherTele = NULL;

	goesBothWays = bothWays;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;


	ts_idle = owner->GetTileset("Enemies/spring_idle_256x256.png", 256, 256);
	ts_recover = owner->GetTileset("Enemies/spring_recover_256x256.png", 256, 256);
	ts_springing = owner->GetTileset("Enemies/spring_spring_512x576.png", 512, 576);

	frame = 0;

	animationFactor = 10;

	sprite.setTexture(*ts_idle->texture);
	sprite.setTextureRect(ts_idle->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(pos.x, pos.y);

	if (secondary)
	{
		if (goesBothWays)
		{
			sprite.setColor(Color::Red);
		}
		else
		{
			sprite.setColor(Color::Magenta);
		}
		
	}

	dest = V2d(other + pos);

	V2d dOther = V2d(other.x, other.y);
	V2d TeleporterVec = normalize(dOther);

	double angle = atan2(TeleporterVec.x, -TeleporterVec.y);//atan2(-TeleporterVec.x, TeleporterVec.y);
	sprite.setRotation(angle / PI * 180.0);

	double dist = length(V2d(other));

	speed = 200;
	stunFrames = floor(dist / speed);

	dir = TeleporterVec;

	float hurtboxRadius = 64;

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = hurtboxRadius;
	hurtBox.rh = hurtboxRadius;
	hurtBox.globalPosition = position;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = hurtboxRadius;
	hitBox.rh = hurtboxRadius;
	hitBox.globalPosition = position;

	hitBody->AddCollisionBox(0, hitBox);

	spawnRect = sf::Rect<double>(position.x - hurtboxRadius - 10, position.y - hurtboxRadius - 10,
		hurtboxRadius * 2 + 10, hurtboxRadius * 2 + 10);

	actionLength[IDLE] = 12;
	actionLength[TELEPORTING] = 8;
	actionLength[RECEIVING] = 8;
	actionLength[RECEIVERECOVERING] = 8;
	actionLength[RECOVERING] = 8;

	animFactor[IDLE] = 4;
	animFactor[TELEPORTING] = 4;
	animFactor[RECEIVING] = 4;
	animFactor[RECEIVERECOVERING] = 4;
	animFactor[RECOVERING] = 8;

	ResetEnemy();
}


Teleporter *Teleporter::CreateSecondary()
{
	assert(otherTele == NULL);
	if (!secondary)
	{
		otherTele = new Teleporter(owner, Vector2i(dest), Vector2i(position - dest), goesBothWays, true);
		otherTele->otherTele = this;
		return otherTele;
	}
	else
	{
		return NULL;
	}
}

void Teleporter::DirectKill()
{

}

void Teleporter::ReceiveRecover()
{
	Teleporter *curr = this;
	if (!teleportedPlayer)
	{
		curr = otherTele;
	}

	curr->action = RECOVERING;
	curr->frame = 0;
	
}

void Teleporter::ResetEnemy()
{
	dead = false;
	teleportedPlayer = false;

	receivedHit = NULL;
	action = IDLE;
	sprite.setTexture(*ts_idle->texture);
	frame = 0;
	SetHitboxes(hitBody, 0);

	UpdateHitboxes();

	UpdateSprite();
}

void Teleporter::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case TELEPORTING:
			action = RECOVERING;
			sprite.setTexture(*ts_recover->texture);
			break;
		case RECEIVING:
			break;
		case RECEIVERECOVERING:
			break;
		case RECOVERING:
			action = IDLE;
			sprite.setTexture(*ts_idle->texture);
			teleportedPlayer = false;
			break;
		}
	}
}

bool Teleporter::TryTeleport()
{
	if (otherTele->teleportedPlayer || ( !goesBothWays && secondary ))
	{
		return false;
	}

	assert(action == IDLE);
	action = TELEPORTING;
	sprite.setTexture(*ts_springing->texture);
	frame = 0;
	teleportedPlayer = true;

	return true;
	//owner->soundNodeList->ActivateSound(launchSoundBuf);
}

void Teleporter::ProcessState()
{
	ActionEnded();
}


void Teleporter::UpdateSprite()
{
	switch (action)
	{
	case IDLE:
		sprite.setTextureRect(ts_idle->GetSubRect(frame / animFactor[action]));
		break;
	case TELEPORTING:
		sprite.setTextureRect(ts_springing->GetSubRect(frame / animFactor[action]));
		break;
	case RECOVERING:
		sprite.setTextureRect(ts_recover->GetSubRect(frame / animFactor[action]));
		break;
	case RECEIVING:
		sprite.setTextureRect(ts_recover->GetSubRect(frame / animFactor[action]));
		break;
	case RECEIVERECOVERING:
		sprite.setTextureRect(ts_recover->GetSubRect(frame / animFactor[action]));
		break;
	}
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Teleporter::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}