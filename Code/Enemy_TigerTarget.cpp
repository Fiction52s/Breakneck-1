#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_TigerTarget.h"
#include "Actor.h"

using namespace std;
using namespace sf;

TigerTarget::TigerTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_TIGERTARGET, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	//SetEditorActions(NEUTRAL, NEUTRAL, 0);
	SetEditorActions(FLAME_LEVEL_1, FLAME_LEVEL_1, 0);

	actionLength[START_BURN] = 4;
	animFactor[START_BURN] = 3;


	actionLength[FLAME_LEVEL_1] = 15;
	animFactor[FLAME_LEVEL_1] = 3;

	actionLength[FLAME_LEVEL_2] = 14;
	animFactor[FLAME_LEVEL_2] = 3;
	//actionLength[NEUTRAL] = 2;
	//actionLength[HEAT_UP] = 60;
	//actionLength[SIMMER] = 10;
	actionLength[ATTACK_PLAYER] = 13;
	animFactor[ATTACK_PLAYER] = 3;

	actionLength[HIT_BY_PLAYER] = 13;
	animFactor[HIT_BY_PLAYER] = 3;

	actionLength[ATTACK_TIGER] = 13;
	animFactor[ATTACK_TIGER] = 3;

	//actionLength[ATTACK_TIGER] = 60;
	actionLength[EXPLODE] = 10;

	maxHitByPlayerFrames = 60;


	ts_bigFlame = GetSizedTileset("Bosses/Tiger/tiger_fire_1_192x192.png");
	ts_smallFlame = GetSizedTileset("Bosses/Tiger/tiger_fire_2_160x160.png");
	ts_attack = GetSizedTileset("Bosses/Tiger/tiger_fire_3_80x80.png");
	sprite.setTexture(*ts_bigFlame->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	maxSpeed = 40;
	baseSpeed = 2;
	hitByPlayerSpeed = 10;
	accel = .1;

	comboObj = new ComboObject(this);

	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::COMBO;

	comboObj->enemyHitBody.BasicCircleSetup(16, GetPosition());

	comboObj->enemyHitboxFrame = 0;


	//cutObject->Setup(ts, 0, 0, scale);

	ResetEnemy();
}

void TigerTarget::SetLevel(int lev)
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

//void TigerTarget::ProcessHit()
//{
//	if (!dead && ReceivedHit() && numHealth > 0)
//	{
//		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
//		ConfirmHitNoKill();
//
//		action = ATTACK_TIGER;
//		frame = 0;
//		HitboxesOff();
//		//DefaultH
//		HurtboxesOff();
//	}
//}

void TigerTarget::ResetEnemy()
{
	action = START_BURN;
	//action = NEUTRAL;
	frame = 0;

	HitboxesOff();
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();

	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;

	UpdateHitboxes();

	currHeatLevel = 0;

	speed = baseSpeed;

	currHitByPlayerFrame = 0;

	UpdateSprite();
}

void TigerTarget::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case START_BURN:
			action = FLAME_LEVEL_1;
			break;
		//case HEAT_UP:
		//	action = SIMMER;
		//	break;
		case EXPLODE:
			dead = true;
			numHealth = 0;
			sess->RemoveEnemy(this);
			break;
		case HIT_BY_PLAYER:
		{
			//action = ATTACK_TIGER;
			//frame = 0;
			break;
		}
		}
	}
}

void TigerTarget::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	/*switch (action)
	{
	case NEUTRAL:
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		break;
		break;
	}*/
}

void TigerTarget::UpdateEnemyPhysics()
{
	if (action == ATTACK_PLAYER)
	{
		velocity = PlayerDir() * speed;

		speed += accel;
		if (speed > maxSpeed)
		{
			speed = maxSpeed;
		}

		currPosInfo.position += velocity / numPhysSteps / (double)slowMultiple;
	}
	else if (action == HIT_BY_PLAYER)
	{
		V2d dir = normalize( tiger->GetPosition() - GetPosition());
		velocity += dir * .1;

		if (length(velocity) > hitByPlayerSpeed)
		{
			velocity = normalize(velocity) * hitByPlayerSpeed;
		}

		//currPosInfo.position += velocity;
		currPosInfo.position += velocity / numPhysSteps / (double)slowMultiple;
	}
	else if (action == ATTACK_TIGER)
	{
		V2d dir = normalize(tiger->GetPosition() - GetPosition());
		velocity = dir * speed;

		speed += accel;
		if (speed > maxSpeed)
		{
			speed = maxSpeed;
		}

		currPosInfo.position += velocity / numPhysSteps / (double)slowMultiple;
		//currPosInfo.position += velocity;
	}
}


void TigerTarget::HeatUp()
{
	if (action == HIT_BY_PLAYER || action == ATTACK_PLAYER || action == ATTACK_TIGER)
		return;

	if (action == START_BURN || action == FLAME_LEVEL_1)
	{
		action = FLAME_LEVEL_2;
		frame = 0;
	}
	else if( action == FLAME_LEVEL_2 )
	{
		action = ATTACK_PLAYER;
		speed = baseSpeed;
		frame = 0;
		DefaultHitboxesOn();
		HurtboxesOff();
	}
	/*if (action == NEUTRAL)
	{
		currHeatLevel = 0;
		action = HEAT_UP;
		frame = 0;
	}
	else
	{
		assert(action == HEAT_UP || action == SIMMER );
		++currHeatLevel;

		if (currHeatLevel == 1)
		{
			action = ATTACK_PLAYER;
			speed = baseSpeed;
			frame = 0;
			DefaultHitboxesOn();
			HurtboxesOff();
		}
		else
		{
			action = HEAT_UP;
			frame = 0;
		}
	}*/
}

void TigerTarget::UpdateSprite()
{
	int trueFrame;
	switch (action)
	{
	case START_BURN:
	{
		ts_bigFlame->SetSpriteTexture(sprite);
		ts_bigFlame->SetSubRect(sprite, frame / animFactor[START_BURN]);
		break;
	}
	case FLAME_LEVEL_1:
	{
		ts_bigFlame->SetSpriteTexture(sprite);
		ts_bigFlame->SetSubRect(sprite, frame / animFactor[FLAME_LEVEL_1] 
			+ actionLength[START_BURN]);
		break;
	}
	case FLAME_LEVEL_2:
	{
		ts_smallFlame->SetSpriteTexture(sprite);
		ts_smallFlame->SetSubRect(sprite, frame / animFactor[FLAME_LEVEL_2]);
		break;
	}
	case ATTACK_PLAYER:
	{
		ts_attack->SetSpriteTexture(sprite);
		ts_attack->SetSubRect(sprite, frame / animFactor[ATTACK_PLAYER]);
		break;
	}
	case ATTACK_TIGER:
	{
		ts_attack->SetSpriteTexture(sprite);
		ts_attack->SetSubRect(sprite, frame / animFactor[ATTACK_PLAYER]);
		break;
	}
	case HIT_BY_PLAYER:
	{
		ts_attack->SetSpriteTexture(sprite);
		ts_attack->SetSubRect(sprite, frame / animFactor[ATTACK_PLAYER]);
		break;
	}
	//case NEUTRAL:
	//{
	//	ts_bigFlame->SetSpriteTexture(sprite);
	//	ts_bigFlame->SetSubRect( sprite, )
	//	//sprite.setColor(Color::White);
	//	break;
	//}
	//	
	//case HEAT_UP:
	//case SIMMER:
	//{
	//	if (currHeatLevel == 0)
	//	{

	//	}
	//	/*if (currHeatLevel == 0)
	//	{
	//		sprite.setColor(Color::Red);
	//	}
	//	else if (currHeatLevel == 1)
	//	{
	//		sprite.setColor(Color::Blue);
	//	}
	//	else if (currHeatLevel == 2)
	//	{
	//		sprite.setColor(Color::Cyan);
	//	}
	//	else
	//	{
	//		sprite.setColor(Color::Magenta);
	//	}*/
	//	break;
	//}
	/*case EXPLODE:
		sprite.setColor(Color::Magenta);
		break;*/

	}

	//ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void TigerTarget::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void TigerTarget::IHitPlayer(int index)
{
	if (action == ATTACK_PLAYER)
	{
		action = EXPLODE;
		frame = 0;
		HitboxesOff();
	}
}

void TigerTarget::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		ConfirmHitNoKill();
		//shootFrames = 0;
		//action = S_SHOT;
		action = HIT_BY_PLAYER;
		frame = 0;

		speed = hitByPlayerSpeed;
		//SetHitboxes(hitBody, 0);
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		V2d dir;

		comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;

		dir = normalize(receivedHit->hDir);

		/*if (dir.x != 0 && dir.y != 0)
		{
			double absX = abs(dir.x);
			double absY = abs(dir.y);
			if (absX - absY > -.01)
			{
				dir.y = 0;
			}
			else
			{
				dir.x = 0;
			}
		}

		dir = normalize(dir);*/



		velocity = dir * baseSpeed;

		//IntRect ir;

		//sprite.setTextureRect(ir);
		sess->PlayerAddActiveComboObj(comboObj, GetReceivedHitPlayerIndex());
	}
}

void TigerTarget::ComboHit()
{
	pauseFrames = 5;
	//++currHits;
	//if (currHits >= hitLimit)
	{
		action = EXPLODE;
		frame = 0;
		sess->PlayerRemoveActiveComboer(comboObj);
		//sess->PlayerAddActiveComboObj(comboObj, GetReceivedHitPlayerIndex());
	}
}

bool TigerTarget::CanComboHit( Enemy *e )
{
	if (e->type == EnemyType::EN_TIGERBOSS)
		return true;

	return false;
}

void TigerTarget::FrameIncrement()
{
	if (action == HIT_BY_PLAYER)
	{
		++currHitByPlayerFrame;

		if (currHitByPlayerFrame == maxHitByPlayerFrames)
		{
			action = ATTACK_TIGER;
			frame = 0;
		}
	}
}