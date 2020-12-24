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

void Spring::UpdateParamsSettings()
{
	SpringParams *sParams = (SpringParams*)editParams;
	speed = sParams->speed;
	stunFrames = ceil(dist / speed);

	stringstream ss;
	ss << speed;
	debugSpeed.setString(ss.str());
	debugSpeed.setOrigin(debugSpeed.getLocalBounds().width / 2, debugSpeed.getLocalBounds().height / 2);
}

void Spring::SetLevel(int lev)
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

void Spring::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	debugSpeed.setPosition(GetPositionF());

	UpdatePath(); //assuming that ap is editparams here
}

void Spring::UpdatePath()
{
	Vector2i other = Vector2i(0, -10);
	if(editParams->localPath.size() > 0 )
		other = editParams->GetLocalPathPos(0);

	dest = GetPosition() + V2d(other);

	V2d dOther = V2d(other.x, other.y);
	V2d springVec = normalize(dOther);

	double angle = atan2(springVec.x, -springVec.y);
	sprite.setRotation(angle / PI * 180.0);

	dist = length(V2d(other));

	UpdateParamsSettings();

	dir = springVec;

	debugLine[0].color = Color::Red;
	debugLine[1].color = Color::Red;
	debugLine[0].position = GetPositionF();
	debugLine[1].position = Vector2f(dest);
}

void Spring::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

Spring::Spring(ActorParams *ap)//SpringType sp, Vector2i &pos, Vector2i &other, int p_speed )
	:Enemy(EnemyType::EN_SPRING, ap )//false, 2, false ), springType( sp )
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	SetLevel(ap->GetLevel());

	V2d position = GetPosition();
	Vector2f positionF(position);

	receivedHit = NULL;

	debugSpeed.setFont(sess->mainMenu->arial);
	debugSpeed.setFillColor(Color::White);
	debugSpeed.setCharacterSize(30);


	//potentially temporary
	string &typeName = ap->type->info.name;
	if (typeName == "spring")
	{
		springType = BLUE;
	}
	else if (typeName == "glidespring")
	{
		springType = GREEN;
	}
	else if (typeName == "bouncespring")
	{
		springType = BOUNCE;
	}
	else if (typeName == "airbouncespring")
	{
		springType = AIRBOUNCE;
	}
	


	launchSoundBuf = sess->GetSound("Enemies/spring_launch");

	switch (springType)
	{
	case BLUE:
		ts_idle = sess->GetTileset("Enemies/spring_idle_256x256.png", 256, 256);
		ts_recover = sess->GetTileset("Enemies/spring_recover_256x256.png", 256, 256);
		ts_springing = sess->GetTileset("Enemies/spring_spring_512x576.png", 512, 576);
		break;
	case GREEN:
		ts_idle = sess->GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256);
		ts_recover = sess->GetTileset("Enemies/spring_recover_2_256x256.png", 256, 256);
		ts_springing = sess->GetTileset("Enemies/spring_spring_2_512x576.png", 512, 576);
		break;
	case BOUNCE:
	case AIRBOUNCE:
		ts_idle = sess->GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256);
		ts_recover = sess->GetTileset("Enemies/spring_recover_2_256x256.png", 256, 256);
		ts_springing = sess->GetTileset("Enemies/spring_spring_2_512x576.png", 512, 576);
		sprite.setColor(Color::Yellow);
		break;
	}

	if (springType == AIRBOUNCE)
	{
		sprite.setColor(Color::Black);
	}

	double radius = 64;

	BasicCircleHitBodySetup(radius);

	actionLength[IDLE] = 12;
	actionLength[SPRINGING] = 8;
	actionLength[RECOVERING] = 8;

	animFactor[IDLE] = 4;
	animFactor[SPRINGING] = 4;
	animFactor[RECOVERING] = 4;

	sprite.setTextureRect(ts_idle->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(positionF);

	editParams = ap;
	UpdatePath();
	

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
	//impossible to direct kill
}

void Spring::ResetEnemy()
{
	dead = false;

	receivedHit = NULL;
	action = IDLE; 
	sprite.setTexture(*ts_idle->texture);

	frame = 0;
	SetHitboxes(&hitBody);
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
	sess->ActivateSound(launchSoundBuf);
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
	sprite.setPosition(GetPositionF());
}

void Spring::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}