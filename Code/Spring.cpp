#include "Enemy.h"
#include "Enemy_Spring.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

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

Spring::Spring(GameSession *owner, Vector2i &pos, Vector2i &other, int p_speed )
	:Enemy(owner, EnemyType::EN_SPRING, false, 2, false )
{
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	debugSpeed.setFont(owner->mainMenu->arial);
	debugSpeed.setFillColor(Color::White);
	debugSpeed.setCharacterSize(30);
	stringstream ss;
	ss << p_speed;
	debugSpeed.setString(ss.str());
	debugSpeed.setOrigin(debugSpeed.getLocalBounds().width / 2, debugSpeed.getLocalBounds().height / 2);
	debugSpeed.setPosition(Vector2f(position));
	

	frame = 0;

	animationFactor = 10;

	//ts = owner->GetTileset( "Spring.png", 80, 80 );
	ts = owner->GetTileset("spring_64x64.png", 64, 64);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
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

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 40;
	hurtBox.rh = 40;
	hurtBox.globalPosition = position;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBox.globalPosition = position;

	hitBody->AddCollisionBox(0, hitBox);

	spawnRect = sf::Rect<double>(position.x - 32, position.y - 32,
		64, 64);

	actionLength[IDLE] = 1;
	actionLength[SPRINGING] = 1;
	actionLength[RECOVERING] = 1;

	animFactor[IDLE] = 1;
	animFactor[SPRINGING] = 1;
	animFactor[RECOVERING] = 1;

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

void Spring::ResetEnemy()
{
	dead = false;

	frame = 0;

	receivedHit = NULL;

	SetHitboxes(hitBody, 0);
	//SetHurtboxes(hurtBody, 0);

	UpdateHitboxes();

	UpdateSprite();
}

void Spring::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
			frame = 0;
		case IDLE:
			break;
		case SPRINGING:
			action = RECOVERING;
			break;
		case RECOVERING:
			action = IDLE;
			break;
		}
	}
}

void Spring::Launch()
{
	assert(action == IDLE);
	action = SPRINGING;
	frame = 0;
}

void Spring::ProcessState()
{
}


void Spring::UpdateSprite()
{
	IntRect ir = ts->GetSubRect(0);
	sprite.setTextureRect(ir);
}

void Spring::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}