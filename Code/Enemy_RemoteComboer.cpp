#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_RemoteComboer.h"
#include "Eye.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

RemoteComboer::RemoteComboer(ActorParams *ap)
	:Enemy(EnemyType::EN_REMOTECOMBOER, ap)//, false, 1, false)
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, S_FLOAT, 0);

	actionLength[S_FLOAT] = 18;
	actionLength[S_SHOT] = 3;
	actionLength[S_EXPLODE] = 20;
	actionLength[S_RETURN] = 10;

	animFactor[S_FLOAT] = 2;
	animFactor[S_SHOT] = 6;
	animFactor[S_EXPLODE] = 1;
	animFactor[S_RETURN] = 1;


	SetLevel(ap->GetLevel());


	SetCurrPosInfo(startPosInfo);

	speed = 17;
	shootLimit = 300;
	hitLimit = 21;
	facingRight = true;

	ts = GetSizedTileset("Enemies/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);
	sprite.setColor(Color::Red);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(48);
	//BasicCircleHitBodySetup(48);
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

	ResetEnemy();
}

RemoteComboer::~RemoteComboer()
{
}

void RemoteComboer::SetLevel(int lev)
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

void RemoteComboer::ResetEnemy()
{
	data.shootFrames = 0;
	data.currHits = 0;
	comboObj->Reset();
	data.velocity = V2d(0, 0);

	DefaultHurtboxesOn();
	//DefaultHitboxesOn();
	action = S_FLOAT;
	frame = 0;

	UpdateHitboxes();

	UpdateSprite();
}

void RemoteComboer::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		ConfirmHitNoKill();
		action = S_SHOT;
		data.shootFrames = 0;
		frame = 0;
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		controlPlayer = sess->GetPlayer(GetReceivedHitPlayerIndex());

		V2d dir;

		comboObj->enemyHitboxInfo->hDir = receivedHit.hDir;

		dir = normalize(receivedHit.hDir);

		if (dir.x != 0 && dir.y != 0)
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

		dir = normalize(dir);



		data.velocity = dir * speed;

		IntRect ir;



		if (data.velocity.x == 0 || data.velocity.y == 0)
		{
			ir = ts->GetSubRect(1);
			sprite.setTextureRect(ts->GetSubRect(1));
		}
		else
		{
			ir = ts->GetSubRect(2);

		}

		if (data.velocity.x < 0)
		{
			ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
		}
		if (data.velocity.x == 0)
		{
			if (data.velocity.y > 0)
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
			if (data.velocity.x > 0 && data.velocity.y > 0)
			{
				sprite.setRotation(90);
			}
			else if (data.velocity.x < 0 && data.velocity.y > 0)
			{
				sprite.setRotation(-90);
			}
		}

		sprite.setTextureRect(ir);

		sess->PlayerAddActiveComboObj(comboObj, GetReceivedHitPlayerIndex());

		receivedHit.SetEmpty();
	}
}

void RemoteComboer::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_EXPLODE:
			numHealth = 0;
			dead = true;
			sess->PlayerRemoveActiveComboer(comboObj);
			break;
		case S_RETURN:
		{
			action = S_FLOAT;
			frame = 0;
			SetCurrPosInfo(startPosInfo);
			DefaultHurtboxesOn();
		}
		}
	}

	if (action == S_SHOT)
	{
		double accel = 1.5;
		V2d dir8 = controlPlayer->currInput.GetLeft8Dir();
		data.velocity += dir8 * accel;
		if (length(data.velocity) > speed)
		{
			data.velocity = normalize(data.velocity) * speed;
		}
	}
}

void RemoteComboer::HandleNoHealth()
{

}

void RemoteComboer::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLOAT:
	{
		break;
	}
	case S_SHOT:
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(data.velocity);
}

void RemoteComboer::FrameIncrement()
{
	if (action == S_SHOT)
	{
		if (data.shootFrames == shootLimit)
		{
			data.velocity = V2d(0, 0);
			sess->PlayerRemoveActiveComboer(comboObj);
			action = S_RETURN;
			//action = S_EXPLODE;
			frame = 0;
		}
		else
		{
			++data.shootFrames;
		}
	}
}

void RemoteComboer::ComboHit()
{
	pauseFrames = 5;
	++data.currHits;
	if (data.currHits >= hitLimit)
	{
		action = S_EXPLODE;
		frame = 0;
	}
}

void RemoteComboer::UpdateSprite()
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

void RemoteComboer::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

int RemoteComboer::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void RemoteComboer::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void RemoteComboer::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}