#include "Enemy.h"
#include "Enemy_SwingLauncher.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "MainMenu.h"

using namespace std;
using namespace sf;


void SwingLauncher::UpdateParamsSettings()
{
	SpringParams *sParams = (SpringParams*)editParams;
	speed = sParams->speed;
	stunFrames = 600;//ceil(swingRadius / speed);
}

void SwingLauncher::UpdatePath()
{
	Vector2i other = Vector2i(0, -10);
	if (editParams->localPath.size() > 0)
		other = editParams->GetLocalPathPos(0);

	V2d dOther = V2d(other.y, -other.x); //perpendicular because swinglauncher
	V2d springVec = normalize(dOther);

	double angle = atan2(springVec.x, -springVec.y);
	sprite.setRotation(angle / PI * 180.0);

	swingRadius = length(V2d(other));

	UpdateParamsSettings();

	dir = springVec;
	if( !clockwiseLaunch)
		dir = -dir;

	anchor = V2d(other) + GetPosition();

	debugLine[0].color = Color::Red;
	debugLine[1].color = Color::Red;
	debugLine[0].position = GetPositionF();
	debugLine[1].position = Vector2f(anchor);

	stringstream ss;
	ss << speed;
	debugSpeed.setString(ss.str());
	debugSpeed.setOrigin(debugSpeed.getLocalBounds().width / 2, debugSpeed.getLocalBounds().height / 2);
	debugSpeed.setPosition(GetPositionF());
}

void SwingLauncher::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

SwingLauncher::SwingLauncher(ActorParams *ap)
	:Enemy(EnemyType::EN_SWINGLAUNCHER, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	launchSoundBuf = GetSound("Enemies/spring_launch");

	debugSpeed.setFont(sess->mainMenu->arial);
	debugSpeed.setFillColor(Color::White);
	debugSpeed.setCharacterSize(30);

	if (ap->GetTypeName() == "swinglaunchercw")
	{
		clockwiseLaunch = true;
	}
	else
	{
		clockwiseLaunch = false;
		sprite.setColor(Color::Red);
	}

	ts_idle = GetSizedTileset("Enemies/spring_idle_256x256.png");
	ts_recover = GetSizedTileset("Enemies/spring_recover_256x256.png");
	ts_springing = GetSizedTileset("Enemies/spring_spring_512x576.png");

	animationFactor = 10;

	BasicCircleHitBodySetup(64);

	actionLength[IDLE] = 12;
	actionLength[SWINGING] = 8;
	actionLength[RECOVERING] = 8;

	animFactor[IDLE] = 4;
	animFactor[SWINGING] = 4;
	animFactor[RECOVERING] = 4;

	editParams = ap;
	UpdatePath();

	ResetEnemy();
}
void SwingLauncher::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	target->draw(debugLine, 2, sf::Lines);
	target->draw(debugSpeed);
}

void SwingLauncher::DirectKill()
{

}

void SwingLauncher::ResetEnemy()
{
	action = IDLE;
	frame = 0;

	sprite.setTexture(*ts_idle->texture);
	
	DefaultHitboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void SwingLauncher::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case SWINGING:
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

void SwingLauncher::Launch()
{
	assert(action == IDLE);
	action = SWINGING;
	sprite.setTexture(*ts_springing->texture);
	frame = 0;
	sess->ActivateSound(launchSoundBuf);
}

void SwingLauncher::ProcessState()
{
	ActionEnded();
}


void SwingLauncher::UpdateSprite()
{
	switch (action)
	{
	case IDLE:
		sprite.setTextureRect(ts_idle->GetSubRect(frame / animFactor[action]));
		break;
	case SWINGING:
		sprite.setTextureRect(ts_springing->GetSubRect(frame / animFactor[action]));
		break;
	case RECOVERING:
		sprite.setTextureRect(ts_recover->GetSubRect(frame / animFactor[action]));
		break;
	}
	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void SwingLauncher::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}