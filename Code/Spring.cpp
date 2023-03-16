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

	double spriteAngle = angle / PI * 180.0;

	sprite.setRotation(spriteAngle);
	recoverSprite.setRotation(spriteAngle);
	boostSprite.setRotation(spriteAngle);
	particleSprite.setRotation(spriteAngle);

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

	receivedHit.SetEmpty();

	debugSpeed.setFont(sess->mainMenu->arial);
	debugSpeed.setFillColor(Color::White);
	debugSpeed.setCharacterSize(30);

	//defaults
	actionLength[IDLE] = 12;
	actionLength[SPRINGING] = 8;
	actionLength[RECOVERING] = 8;

	animFactor[IDLE] = 5;
	animFactor[SPRINGING] = 5;
	animFactor[RECOVERING] = 4;

	//potentially temporary
	string &typeName = ap->type->info.name;
	if (typeName == "spring")
	{
		springType = TYPE_REGULAR;
		tilesetChoice = 0;
		recoverTileseChoice = 0;
		recoverStartFrame = 0;
		startFrame = 0;
	}
	else if (typeName == "glidespring")
	{
		springType = TYPE_GLIDE;
		tilesetChoice = 0;
		recoverTileseChoice = 0;
		recoverStartFrame = 8;
		startFrame = 5;
	}
	else if (typeName == "annihilationspring")
	{
		springType = TYPE_ANNIHILATION_GLIDE;
		tilesetChoice = 1;
		recoverTileseChoice = 2;
		recoverStartFrame = 16;
		startFrame = 20;
	}
	/*else if (typeName == "hominglauncher")
	{
		springType = TYPE_HOMING;
		tilesetChoice = 1;
		recoverTileseChoice = 2;
		recoverStartFrame = 0;
		startFrame = 10;
	}*/
	
	if (tilesetChoice == 0)
	{
		ts = GetSizedTileset("Enemies/Launchers/launcher_1_384x384.png");
	}
	else if (tilesetChoice == 1)
	{
		ts = GetSizedTileset("Enemies/Launchers/launcher_2_384x384.png");
	}

	if (recoverTileseChoice == 0)
	{
		ts_recover = GetSizedTileset("Enemies/Launchers/launcher_recover_1_384x384.png");
	}
	else if (recoverTileseChoice == 1)
	{
		ts_recover = GetSizedTileset("Enemies/Launchers/launcher_recover_2_384x384.png");
	}
	else if (recoverTileseChoice == 2)
	{
		ts_recover = GetSizedTileset("Enemies/Launchers/launcher_recover_3_384x384.png");
	}

	launchSoundBuf = GetSound("Enemies/spring_launch");
	ts_particles = GetSizedTileset("Enemies/Launchers/launcher_particles_256x256.png");
	ts_boost = GetSizedTileset("Enemies/Launchers/launcher_explode_512x512.png");
	
	
	particleSprite.setTexture(*ts_particles->texture);
	boostSprite.setTexture(*ts_boost->texture);
	recoverSprite.setTexture(*ts_recover->texture);
	sprite.setTexture(*ts->texture);

	double radius = 64;

	BasicCircleHitBodySetup(radius);

	//IDLE = 12 * 4
	//SPRINGING = 8 * 4
	//RECOVERING = 8 * 4

	

	/*sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(positionF);*/

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

	receivedHit.SetEmpty();
	action = IDLE; 
	//sprite.setTexture(*ts_idle->texture);

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
		sprite.setTextureRect(ts->GetSubRect((frame / animFactor[action])/3 + startFrame));
		particleSprite.setTextureRect(ts_particles->GetSubRect(frame / animFactor[action]));
		break;
	case SPRINGING:
		boostSprite.setTextureRect(ts_boost->GetSubRect(frame / animFactor[action]));
		sprite.setTextureRect(ts->GetSubRect(4 + startFrame));
		break;
	case RECOVERING:
		recoverSprite.setTextureRect(ts->GetSubRect(frame / animFactor[action] + recoverStartFrame));
		sprite.setTextureRect(ts->GetSubRect(4 + startFrame));
		break;
	}

	particleSprite.setOrigin(particleSprite.getLocalBounds().width / 2,
		particleSprite.getLocalBounds().height / 2);
	particleSprite.setPosition(GetPositionF());

	boostSprite.setOrigin(boostSprite.getLocalBounds().width / 2,
		boostSprite.getLocalBounds().height / 2);
	boostSprite.setPosition(GetPositionF());

	recoverSprite.setOrigin(recoverSprite.getLocalBounds().width / 2, recoverSprite.getLocalBounds().height / 2);
	recoverSprite.setPosition(GetPositionF());

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void Spring::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);

	if (action == RECOVERING)
	{
		target->draw(recoverSprite);
	}
	else if (action == IDLE)
	{
		target->draw(particleSprite);
	}

	if (action == SPRINGING)
	{
		target->draw(boostSprite);
	}
}

void Spring::DrawMinimap(sf::RenderTarget *target)
{
	if (!dead)
	{
		CircleShape enemyCircle;
		enemyCircle.setFillColor(COLOR_BLUE);
		enemyCircle.setRadius(50);
		enemyCircle.setOrigin(enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2);
		enemyCircle.setPosition(GetPositionF());
		target->draw(enemyCircle);
	}
}

int Spring::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Spring::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Spring::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}