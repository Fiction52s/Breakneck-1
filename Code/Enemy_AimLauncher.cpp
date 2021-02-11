#include "Enemy.h"
#include "Enemy_AimLauncher.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "MainMenu.h"

using namespace std;
using namespace sf;

void AimLauncher::UpdateParamsSettings()
{
	SpringParams *sParams = (SpringParams*)editParams;
	speed = sParams->speed;
	stunFrames = ceil(dist / speed);
}

void AimLauncher::SetLevel(int lev)
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

void AimLauncher::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	UpdatePath(); //assuming that ap is editparams here
}

void AimLauncher::UpdatePath()
{
	Vector2i other = Vector2i(0, -10);
	if (editParams->localPath.size() > 0)
		other = editParams->GetLocalPathPos(0);

	dest = GetPosition() + V2d(other);

	V2d dOther = V2d(other.x, other.y);
	V2d springVec = normalize(dOther);

	origDir = springVec;

	dist = length(V2d(other));

	UpdateParamsSettings();

	SetCurrDir(origDir);

	debugLine[0].color = Color::Red;
	debugLine[1].color = Color::Red;
	debugLine[0].position = GetPositionF();
	debugLine[1].position = Vector2f(dest);
}

void AimLauncher::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

AimLauncher::AimLauncher(ActorParams *ap)//SpringType sp, Vector2i &pos, Vector2i &other, int p_speed )
	:Enemy(EnemyType::EN_AIMLAUNCHER, ap)//false, 2, false ), springType( sp )
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	SetLevel(ap->GetLevel());

	const string &typeName = ap->GetTypeName();

	if (typeName == "aimlauncher")
	{
		aimLauncherType = TYPE_BOUNCE;
		tilesetChoice = 0;
		recoverTileseChoice = 0;
		recoverStartFrame = 16;
		startFrame = 10;
	}
	else if (typeName == "airbouncelauncher")
	{
		aimLauncherType = TYPE_AIRBOUNCE;
		
		tilesetChoice = 0;
		recoverTileseChoice = 1;
		recoverStartFrame = 0;
		startFrame = 15;
	}
	else if (typeName == "grindlauncher")
	{
		aimLauncherType = TYPE_GRIND;
		
		tilesetChoice = 1;
		recoverTileseChoice = 1;
		recoverStartFrame = 8;
		startFrame = 0;
	}
	else if (typeName == "hominglauncher")
	{
		aimLauncherType = TYPE_HOMING;
		
		tilesetChoice = 1;
		recoverTileseChoice = 2;
		recoverStartFrame = 0;
		startFrame = 10;
	}

	if (tilesetChoice == 0)
	{
		ts = sess->GetSizedTileset("Enemies/Launchers/launcher_1_384x384.png");
	}
	else if (tilesetChoice == 1)
	{
		ts = sess->GetSizedTileset("Enemies/Launchers/launcher_2_384x384.png");
	}

	if (recoverTileseChoice == 0)
	{
		ts_recover = sess->GetSizedTileset("Enemies/Launchers/launcher_recover_1_384x384.png");
	}
	else if (recoverTileseChoice == 1)
	{
		ts_recover = sess->GetSizedTileset("Enemies/Launchers/launcher_recover_2_384x384.png");
	}
	else if (recoverTileseChoice == 2)
	{
		ts_recover = sess->GetSizedTileset("Enemies/Launchers/launcher_recover_3_384x384.png");
	}

	ts_particles = sess->GetSizedTileset("Enemies/Launchers/launcher_particles_256x256.png");
	ts_boost = sess->GetSizedTileset("Enemies/Launchers/launcher_explode_512x512.png");

	particleSprite.setTexture(*ts_particles->texture);
	boostSprite.setTexture(*ts_boost->texture);
	recoverSprite.setTexture(*ts_recover->texture);
	sprite.setTexture(*ts->texture);

	V2d position = GetPosition();
	Vector2f positionF(position);

	receivedHit = NULL;

	debugSpeed.setFont(sess->mainMenu->arial);
	debugSpeed.setFillColor(Color::White);
	debugSpeed.setCharacterSize(30);

	stringstream ss;
	ss << speed;
	debugSpeed.setString(ss.str());
	debugSpeed.setOrigin(debugSpeed.getLocalBounds().width / 2, debugSpeed.getLocalBounds().height / 2);
	debugSpeed.setPosition(positionF);

	launchSoundBuf = sess->GetSound("Enemies/spring_launch");

	double radius = 64;

	BasicCircleHitBodySetup(radius);

	actionLength[IDLE] = 12;
	actionLength[AIMING] = 12;
	actionLength[LAUNCHING] = 8;
	actionLength[RECOVERING] = 8;

	animFactor[IDLE] = 5;
	animFactor[AIMING] = 5;
	animFactor[LAUNCHING] = 4;
	animFactor[RECOVERING] = 4;

	animationFactor = 10;

	/*sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(positionF);*/

	editParams = ap;
	UpdatePath();

	ResetEnemy();
}
void AimLauncher::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	target->draw(debugLine, 2, sf::Lines);
	target->draw(debugSpeed);
}

void AimLauncher::DirectKill()
{
	//impossible to direct kill
}

void AimLauncher::ResetEnemy()
{
	dead = false;

	receivedHit = NULL;
	action = IDLE;
	

	frame = 0;
	SetHitboxes(&hitBody);
	//SetHurtboxes(hurtBody, 0);

	SetCurrDir(origDir);

	UpdateHitboxes();

	UpdateSprite();
}

void AimLauncher::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case AIMING:
			break;
		case LAUNCHING:
			action = RECOVERING;
			break;
		case RECOVERING:
			action = IDLE;
			break;
		}
	}
}

void AimLauncher::SetCurrDir(V2d &newDir)
{
	currDir = newDir;
	double angle = atan2(currDir.x, -currDir.y);

	float spriteAngle = angle / PI * 180.0;

	sprite.setRotation(spriteAngle);
	recoverSprite.setRotation(spriteAngle);
	boostSprite.setRotation(spriteAngle);
	particleSprite.setRotation(spriteAngle);
}

void AimLauncher::StartAiming()
{
	action = AIMING;
	frame = 0;
}

void AimLauncher::Launch()
{
	assert(action == AIMING);
	action = LAUNCHING;
	frame = 0;
	sess->ActivateSound(launchSoundBuf);
}

void AimLauncher::ProcessState()
{
	ActionEnded();
}


void AimLauncher::UpdateSprite()
{
	switch (action)
	{
	case AIMING:
	case IDLE:
		sprite.setTextureRect(ts->GetSubRect((frame / animFactor[action]) / 3 + startFrame));
		particleSprite.setTextureRect(ts_particles->GetSubRect(frame / animFactor[action]));
		break;
	case LAUNCHING:
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

void AimLauncher::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);

	if (action == RECOVERING)
	{
		target->draw(recoverSprite);
	}
	else if (action == IDLE || action == AIMING)
	{
		target->draw(particleSprite);
	}

	if (action == LAUNCHING)
	{
		target->draw(boostSprite);
	}
}