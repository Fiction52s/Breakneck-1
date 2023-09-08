#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Hand.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Hand::Hand( bool right )
	:Enemy(EnemyType::EN_HAND, NULL)
{
	isRight = right;

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 4;
	animFactor[NEUTRAL] = 5;

	actionLength[APPEAR] = 60;
	animFactor[APPEAR] = 1;

	accel = .1;

	maxSpeed = 5;

	ts = GetSizedTileset("Enemies/W7/hand_256x256.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	/*cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(21);
	cutObject->SetSubRectBack(20);
	cutObject->SetScale(scale);*/

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	CreateSurfaceMover(startPosInfo, 32, this);

	Reset();
	//ResetEnemy();
}

void Hand::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Hand::SetLevel(int lev)
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

void Hand::ResetEnemy()
{
	/*if (GetPosition().x < sess->playerOrigPos[0].x)
	facingRight = false;
	else
	facingRight = true;*/

	action = NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Hand::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case APPEAR:
			action = NEUTRAL;
			frame = 0;
			break;
		}
	}
}

void Hand::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();
}

void Hand::UpdateEnemyPhysics()
{
	Enemy::UpdateEnemyPhysics();
	//V2d movementVec = velocity;
	//movementVec /= slowMultiple * (double)numPhysSteps;

	//currPosInfo.position += movementVec;
}

void Hand::UpdateSprite()
{
	int trueFrame = 0;

	ts->SetSubRect(sprite, trueFrame, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());


	sprite.setColor(Color::White);
	switch (action)
	{
	case APPEAR:
		sprite.setColor(Color::Red);
		break;
	}
}

void Hand::HitTerrainAerial(Edge *e, double quant)
{

}

void Hand::Appear(V2d &pos)
{
	if (!active)
	{
		Reset();
		sess->AddEnemy(this);

		action = APPEAR;
		frame = 0;

		currPosInfo.position = pos;
		currPosInfo.ground = NULL;

		surfaceMover->Set(currPosInfo);

		UpdateHitboxes();
	}
}