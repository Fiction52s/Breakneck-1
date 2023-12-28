#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SwordProjectile.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

SwordProjectile::SwordProjectile(Actor *player)
	:Enemy(EnemyType::EN_SWORDPROJECTILE, NULL)//, false, 1, false)
{
	SetNumActions(S_Count);
	SetEditorActions(S_SHOT, S_SHOT, 0);

	actionLength[S_SHOT] = 3;
	actionLength[S_EXPLODE] = 1;

	animFactor[S_SHOT] = 6;
	animFactor[S_EXPLODE] = 1;


	//SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	speed = 15;
	shootLimit = 40;
	hitLimit = 6;
	facingRight = true;

	ts = player->ts_swordProjectile;//GetSizedTileset("Enemies/General/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);


	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;*/

	//BasicCircleHurtBodySetup(48);
	//BasicCircleHitBodySetup(48);
	//hitBody.hitboxInfo = hitboxInfo;

	comboObj = new ComboObject(this);

	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->comboer = true;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::GREY;

	comboObj->enemyHitBody.BasicCircleSetup(48, GetPosition());

	ResetEnemy();
}

SwordProjectile::~SwordProjectile()
{
}

//sess->PlayerRestoreDoubleJump(0);
//sess->PlayerRestoreAirDash(0);

void SwordProjectile::SetLevel(int lev)
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

void SwordProjectile::ComboKill(Enemy *e)
{
	//if (detachOnKill)
	//{
	//	action = S_WAIT;
	//	frame = 0;
	//	latchedOn = false;
	//	DefaultHurtboxesOn();
	//	//sprite.setColor(Color::Blue);
	//}

	//++numKilled;

	//if (limitedKills && numKilled == maxKilled)
	//{
	//	if (hasMonitor && !suppressMonitor)
	//	{
	//		sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
	//			sess->GetPlayer(0), 1, GetPosition());
	//		suppressMonitor = true;
	//	}

	//	action = S_RETURN;
	//	frame = 0;

	//	Return();

	//	return;
	//}
	//UpdateKilledNumberText(maxKilled - numKilled);
	sess->PlayerRestoreAirOptions(0);
}

void SwordProjectile::ResetEnemy()
{
	data.shootFrames = 0;
	data.currHits = 0;
	comboObj->Reset();
	data.velocity = V2d(0, 0);

	//DefaultHurtboxesOn();
	//DefaultHitboxesOn();
	action = S_SHOT;
	frame = 0;

	UpdateHitboxes();

	UpdateSprite();
}

bool SwordProjectile::IsActive()
{
	return spawned && !dead;//comboObj->active || !dead;
}

void SwordProjectile::Throw( int playerIndex, V2d &pos, V2d &dir)
{
	sess->AddEnemy(this);

	//ResetEnemy();
	V2d normalizedDir = normalize(dir);

	currPosInfo.position = pos;
	action = S_SHOT;
	frame = 0;
	comboObj->enemyHitboxInfo->hDir = normalizedDir;
	data.velocity = normalizedDir * speed;
	sess->PlayerAddActiveComboObj(comboObj,playerIndex );
}

void SwordProjectile::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_EXPLODE:
			numHealth = 0;
			dead = true;
			spawned = false;
			sess->PlayerRemoveActiveComboer(comboObj);
			//sess->RemoveEnemy(this);
			break;
		}
	}

	//V2d playerPos = owner->GetPlayer(0)->position;
}

void SwordProjectile::HandleNoHealth()
{

}

void SwordProjectile::UpdateEnemyPhysics()
{
	switch (action)
	{
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

void SwordProjectile::FrameIncrement()
{
	if (action == S_SHOT)
	{
		if (data.shootFrames == shootLimit)
		{
			action = S_EXPLODE;
			frame = 0;
		}
		else
		{
			++data.shootFrames;
		}
	}
}

void SwordProjectile::ComboHit()
{
	pauseFrames = 6;
	++data.currHits;
	if (data.currHits >= hitLimit)
	{
		action = S_EXPLODE;
		frame = 0;
	}
}

void SwordProjectile::UpdateSprite()
{
	sprite.setPosition(GetPositionF());
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setRotation(0);
}

int SwordProjectile::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void SwordProjectile::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void SwordProjectile::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}