#include "Enemy.h"
#include "Enemy_Spring.h"
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

Spring::Spring(GameSession *owner, SpringType sp, Vector2i &pos, Vector2i &other, int p_speed )
	:Enemy(owner, EnemyType::EN_SPRING, false, 2, false ), springType( sp )
{
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	launchSoundBuf = owner->soundManager->GetSound("Enemies/spring_launch");

	debugSpeed.setFont(owner->mainMenu->arial);
	debugSpeed.setFillColor(Color::White);
	debugSpeed.setCharacterSize(30);
	stringstream ss;
	ss << p_speed;
	debugSpeed.setString(ss.str());
	debugSpeed.setOrigin(debugSpeed.getLocalBounds().width / 2, debugSpeed.getLocalBounds().height / 2);
	debugSpeed.setPosition(Vector2f(position));
	
	switch (springType)
	{
	case BLUE:
		ts_idle = owner->GetTileset("Enemies/spring_idle_256x256.png", 256, 256);
		ts_recover = owner->GetTileset("Enemies/spring_recover_256x256.png", 256, 256);
		ts_springing = owner->GetTileset("Enemies/spring_spring_512x576.png", 512, 576);
		break;
	case GREEN:
		ts_idle = owner->GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256);
		ts_recover = owner->GetTileset("Enemies/spring_recover_2_256x256.png", 256, 256);
		ts_springing = owner->GetTileset("Enemies/spring_spring_2_512x576.png", 512, 576);
		break;
	case REDIRECT:
	case REFLECT:
	case BOUNCE:
		ts_idle = owner->GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256);
		ts_recover = owner->GetTileset("Enemies/spring_recover_2_256x256.png", 256, 256);
		ts_springing = owner->GetTileset("Enemies/spring_spring_2_512x576.png", 512, 576);
		sprite.setColor(Color::Yellow);
		break;
	}
	

	frame = 0;

	animationFactor = 10;

	sprite.setTexture(*ts_idle->texture);
	sprite.setTextureRect(ts_idle->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(pos.x, pos.y);
	
	V2d dOther = V2d(other.x, other.y);
	V2d springVec = normalize(dOther);

	double angle = atan2(springVec.x, -springVec.y);//atan2(-springVec.x, springVec.y);
	sprite.setRotation(angle / PI * 180.0 );

	speed = p_speed;//length( dOther ) / (double)p_moveFrames;
	double dist = length( V2d(other));
	stunFrames =  ceil(dist / speed);
	dir = springVec;
	
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
		hurtboxRadius *2 + 10, hurtboxRadius*2 + 10);

	actionLength[IDLE] = 12;
	actionLength[SPRINGING] = 8;
	actionLength[RECOVERING] = 8;

	animFactor[IDLE] = 4;
	animFactor[SPRINGING] = 4;
	animFactor[RECOVERING] = 4;

	debugLine[0].color = Color::Red;
	debugLine[1].color = Color::Red;
	debugLine[0].position = Vector2f(pos);
	debugLine[1].position = Vector2f(pos + other);

	ResetEnemy();
}
void Spring::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	target->draw(debugLine, 2, sf::Lines);
	target->draw(debugSpeed);
}

void Spring::DirectKill()
{

}

void Spring::ResetEnemy()
{
	dead = false;

	
	receivedHit = NULL;
	action = IDLE; 
	sprite.setTexture(*ts_idle->texture);
	frame = 0;
	SetHitboxes(hitBody, 0);
	//SetHurtboxes(hurtBody, 0);

	UpdateHitboxes();

	UpdateSprite();
}

void Spring::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case SPRINGING:
			action = RECOVERING;
			sprite.setTexture(*ts_recover->texture);
			break;
		case RECOVERING:
			action = IDLE;
			sprite.setTexture(*ts_idle->texture);
			break;
		}
	}
}

void Spring::Launch()
{
	assert(action == IDLE);
	action = SPRINGING;
	sprite.setTexture(*ts_springing->texture);
	frame = 0;
	owner->soundNodeList->ActivateSound(launchSoundBuf);
}

void Spring::ProcessState()
{
	ActionEnded();
}


void Spring::UpdateSprite()
{
	switch (action)
	{
	case IDLE:
		sprite.setTextureRect(ts_idle->GetSubRect(frame / animFactor[action]));
		break;
	case SPRINGING:
		sprite.setTextureRect(ts_springing->GetSubRect(frame / animFactor[action]));
		break;
	case RECOVERING:
		sprite.setTextureRect(ts_recover->GetSubRect(frame / animFactor[action]));
		break;
	}
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Spring::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}