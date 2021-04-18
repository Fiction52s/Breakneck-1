#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Comboer.h"
#include "Eye.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

Comboer::Comboer(ActorParams *ap )
	:Enemy(EnemyType::EN_COMBOER, ap )//, false, 1, false)
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, S_FLOAT, 0);

	actionLength[S_FLOAT] = 18;
	actionLength[S_SHOT] = 3;
	actionLength[S_EXPLODE] = 20;
	actionLength[S_RETURN] = 30;

	animFactor[S_FLOAT] = 2;
	animFactor[S_SHOT] = 6;
	animFactor[S_EXPLODE] = 1;


	SetLevel(ap->GetLevel());

	pathFollower.SetParams(ap);

	SetCurrPosInfo(startPosInfo);
	
	speed = 15;
	shootLimit = 120;
	hitLimit = 6;
	facingRight = true;

	ts = sess->GetTileset("Enemies/comboers_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);
	hitBody.hitboxInfo = hitboxInfo;

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

	comboObj->enemyHitBody.BasicCircleSetup(48, GetPosition());

	comboObj->enemyHitboxFrame = 0;

	ResetEnemy();
}

Comboer::~Comboer()
{
}

void Comboer::SetLevel(int lev)
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

void Comboer::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	pathFollower.SetParams(ap);
}

void Comboer::UpdatePath()
{
	pathFollower.SetParams(editParams);
}

void Comboer::Return()
{
	sess->PlayerRemoveActiveComboer(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	numHealth = maxHealth;
}

void Comboer::ResetEnemy()
{	
	shootFrames = 0;
	currHits = 0;
	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;
	velocity = V2d(0, 0);
	
	DefaultHurtboxesOn();
	DefaultHitboxesOn();
	action = S_FLOAT;
	frame = 0;
	pathFollower.Reset();

	UpdateHitboxes();

	UpdateSprite();
}

void Comboer::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		ConfirmHitNoKill();
		shootFrames = 0;
		action = S_SHOT;
		frame = 0;
		//SetHitboxes(hitBody, 0);
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		V2d dir;

		comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;

		dir = normalize(receivedHit->hDir);
		/*switch (receivedHit->hDir)
		{
		case HitboxInfo::LEFT:
			dir = V2d(-1, 0);
			break;
		case HitboxInfo::RIGHT:
			dir = V2d(1, 0);
			break;
		case HitboxInfo::UP:
			dir = V2d(0, -1);
			break;
		case HitboxInfo::DOWN:
			dir = V2d(0, 1);
			break;
		case HitboxInfo::UPLEFT:
			dir = V2d(-1, -1);
			break;
		case HitboxInfo::UPRIGHT:
			dir = V2d(1, -1);
			break;
		case HitboxInfo::DOWNLEFT:
			dir = V2d(-1, 1);
			break;
		case HitboxInfo::DOWNRIGHT:
			dir = V2d(1, 1);
			break;
		default: 
			assert(0);
		
		}*/


		if (dir.x != 0 && dir.y != 0 )
		{
			double absX = abs(dir.x);
			double absY = abs(dir.y);
			if (absX - absY > -.01 )
			{
				dir.y = 0;
			}
			else
			{
				dir.x = 0;
			}
		}

		dir = normalize(dir);



		velocity = dir * speed;

		IntRect ir;
		
		

		if (velocity.x == 0 || velocity.y == 0)
		{
			ir = ts->GetSubRect(1);
			sprite.setTextureRect(ts->GetSubRect(1));
		}
		else
		{
			ir = ts->GetSubRect(2);
			
		}

		if( velocity.x < 0 )
		{
			ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
		}
		if (velocity.x == 0)
		{
			if (velocity.y > 0)
			{
				sprite.setRotation(90);
			}
			else
			{
				sprite.setRotation(-90);
			}	
		}
		else
		{
			if (velocity.x > 0 && velocity.y > 0)
			{
				sprite.setRotation(90);
			}
			else if( velocity.x < 0 && velocity.y > 0 )
			{
				sprite.setRotation(-90);
			}
		}

		sprite.setTextureRect(ir);

		/*if (velocity.y > 0 && velocity.x < 0)
		{
			sprite.setTextureRect(sf::IntRect(ir.left, ir.top, -ir.width, ir.height));
		}*/


		sess->PlayerAddActiveComboObj(comboObj, GetReceivedHitPlayerIndex());
	}
}

void Comboer::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_EXPLODE:
			action = S_RETURN;
			frame = 0;
			Return();

			/*numHealth = 0;
			dead = true;
			sess->PlayerRemoveActiveComboer(comboObj);*/
			break;
		case S_RETURN:
			action = S_FLOAT;
			frame = 0;
			SetCurrPosInfo(startPosInfo);
			DefaultHurtboxesOn();
			DefaultHitboxesOn();
			break;
		}
	}

	//V2d playerPos = owner->GetPlayer(0)->position;
}

void Comboer::HandleNoHealth()
{
	
}

void Comboer::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLOAT:
	{
		double movement = speed / numPhysSteps;
		movement /= (double)slowMultiple;
		pathFollower.Move(movement, currPosInfo.position );
		break;
	}
	case S_SHOT:
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(velocity);
}

void Comboer::FrameIncrement()
{
	if (action == S_SHOT)
	{
		if (shootFrames == shootLimit)
		{
			action = S_EXPLODE;
			frame = 0;
		}
		else
		{
			++shootFrames;
		}
	}
}

void Comboer::ComboHit()
{
	pauseFrames = 5;
	++currHits;
	if (currHits >= hitLimit)
	{
		action = S_EXPLODE;
		frame = 0;
	}
}

void Comboer::UpdateSprite()
{
	sprite.setPosition(GetPositionF());
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setRotation(0);
	/*int tIndex = 0;
	switch (action)
	{
	case S_FLOAT:
		tIndex = 0;
		break;
	case S_SHOT:
		
		break;
	case S_EXPLODE:
		break;
	}
	sprite.setTextureRect(ts->GetSubRect(frame));*/
}

void Comboer::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}