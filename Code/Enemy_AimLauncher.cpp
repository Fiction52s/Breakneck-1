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
	}
	else if (typeName == "hominglauncher")
	{
		aimLauncherType = TYPE_HOMING;
		sprite.setColor(Color::Red);
	}

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

	ts_idle = sess->GetTileset("Enemies/spring_idle_256x256.png", 256, 256);
	ts_recover = sess->GetTileset("Enemies/spring_recover_256x256.png", 256, 256);
	ts_springing = sess->GetTileset("Enemies/spring_spring_512x576.png", 512, 576);

	double radius = 64;

	BasicCircleHitBodySetup(radius);

	actionLength[IDLE] = 12;
	actionLength[AIMING] = 2;
	actionLength[LAUNCHING] = 8;
	actionLength[RECOVERING] = 8;

	animFactor[IDLE] = 4;
	animFactor[AIMING] = 1;
	animFactor[LAUNCHING] = 4;
	animFactor[RECOVERING] = 4;

	animationFactor = 10;

	sprite.setTextureRect(ts_idle->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(positionF);

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
	sprite.setTexture(*ts_idle->texture);

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
			sprite.setTexture(*ts_recover->texture);
			break;
		case RECOVERING:
			action = IDLE;
			sprite.setTexture(*ts_idle->texture);
			break;
		}
	}
}

void AimLauncher::SetCurrDir(V2d &newDir)
{
	currDir = newDir;
	double angle = atan2(currDir.x, -currDir.y);
	sprite.setRotation(angle / PI * 180.0);
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
	sprite.setTexture(*ts_springing->texture);
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
	case IDLE:
		sprite.setTextureRect(ts_idle->GetSubRect(frame / animFactor[action]));
		break;
	case LAUNCHING:
		sprite.setTextureRect(ts_springing->GetSubRect(frame / animFactor[action]));
		break;
	case RECOVERING:
		sprite.setTextureRect(ts_recover->GetSubRect(frame / animFactor[action]));
		break;
	}
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void AimLauncher::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}