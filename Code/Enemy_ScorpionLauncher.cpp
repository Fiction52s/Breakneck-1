#include "Enemy.h"
#include "Enemy_ScorpionLauncher.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "MainMenu.h"

using namespace std;
using namespace sf;

#define COLOR_BLUE Color( 0, 0x66, 0xcc )


void ScorpionLauncher::UpdateParamsSettings()
{
	SpringParams *sParams = (SpringParams*)editParams;
	speed = sParams->speed;

	stringstream ss;
	ss << speed;
	debugSpeed.setString(ss.str());
	debugSpeed.setOrigin(debugSpeed.getLocalBounds().width / 2, debugSpeed.getLocalBounds().height / 2);
}

void ScorpionLauncher::SetLevel(int lev)
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

void ScorpionLauncher::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	debugSpeed.setPosition(GetPositionF());

	UpdatePath(); //assuming that ap is editparams here
}

void ScorpionLauncher::UpdatePath()
{
	Vector2i other = Vector2i(0, -10);
	if (editParams->localPath.size() > 0)
		other = editParams->GetLocalPathPos(0);

	V2d dOther = V2d(other.x, other.y);
	V2d springVec = normalize(dOther);

	double angle = atan2(springVec.x, -springVec.y);

	double spriteAngle = angle / PI * 180.0;

	sprite.setRotation(spriteAngle);

	UpdateParamsSettings();

	dir = springVec;

	V2d dest = GetPosition() + dir * 100.0;

	debugLine[0].color = Color::Red;
	debugLine[1].color = Color::Red;
	debugLine[0].position = GetPositionF();
	debugLine[1].position = Vector2f(dest);
}

void ScorpionLauncher::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

ScorpionLauncher::ScorpionLauncher(ActorParams *ap)//SpringType sp, Vector2i &pos, Vector2i &other, int p_speed )
	:Enemy(EnemyType::EN_SCORPIONLAUNCHER, ap)//false, 2, false ), springType( sp )
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

	ts = GetSizedTileset("Enemies/Bosses/Coyote/coy_scorp_160x128.png");

	launchSoundBuf = GetSound("Enemies/spring_launch");

	sprite.setTexture(*ts->texture);

	double radius = 64;

	BasicCircleHitBodySetup(radius);

	editParams = ap;
	UpdatePath();

	ResetEnemy();
}

void ScorpionLauncher::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	target->draw(debugLine, 2, sf::Lines);
	target->draw(debugSpeed);
}

void ScorpionLauncher::DirectKill()
{
	//impossible to direct kill
}

void ScorpionLauncher::ResetEnemy()
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

void ScorpionLauncher::ActionEnded()
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

void ScorpionLauncher::Launch()
{
	assert(action == IDLE);
	action = SPRINGING;
	frame = 0;
	sess->ActivateSound(launchSoundBuf);
}

void ScorpionLauncher::ProcessState()
{
	ActionEnded();
}

void ScorpionLauncher::UpdateSprite()
{
	switch (action)
	{
	case IDLE:
		break;
	case SPRINGING:
		break;
	case RECOVERING:
		break;
	}

	ts->SetSubRect(sprite, 16);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void ScorpionLauncher::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void ScorpionLauncher::DrawMinimap(sf::RenderTarget *target)
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

int ScorpionLauncher::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void ScorpionLauncher::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void ScorpionLauncher::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}