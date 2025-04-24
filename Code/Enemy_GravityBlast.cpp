#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GravityBlast.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

GravityBlast::GravityBlast(Actor *player)
	:Enemy(EnemyType::EN_GRAVITYBLAST, NULL)//, false, 1, false)
{
	SetNumActions(S_Count);
	SetEditorActions(S_EXPLODE, S_EXPLODE, 0);

	actionLength[S_EXPLODE] = 30;

	animFactor[S_EXPLODE] = 1;

	testCircle.setFillColor(Color(0, 255, 0, 60));
	

	//SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

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

	int blastRadius = 180;

	comboObj->enemyHitBody.BasicCircleSetup(blastRadius, GetPosition());

	testCircle.setRadius(blastRadius);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2, testCircle.getLocalBounds().height / 2);

	ResetEnemy();
}

GravityBlast::~GravityBlast()
{
}

//sess->PlayerRestoreDoubleJump(0);
//sess->PlayerRestoreAirDash(0);

void GravityBlast::SetLevel(int lev)
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

void GravityBlast::ComboKill(Enemy *e)
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
	//sess->PlayerRestoreAirOptions(0);
}

void GravityBlast::ResetEnemy()
{
	data.currHits = 0;
	comboObj->Reset();
	action = S_EXPLODE;
	frame = 0;

	UpdateHitboxes();

	UpdateSprite();
}

bool GravityBlast::IsActive()
{
	return spawned && !dead;//comboObj->active || !dead;
}

void GravityBlast::DirectKill()
{
	if (!dead)
	{
		//sess->ActivateEffect(DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_killSpack, GetPosition(), true, 0, 10, 4, true);

		dead = true;
		spawned = false;

		numHealth = 0;
		HandleNoHealth();
		receivedHit.SetEmpty();

		if (cutObject != NULL)
		{
			SyncCutObject();
			cutObject->SetCutRootPos(GetPositionF());
		}
	}

	if (comboObj != NULL)
		sess->PlayerRemoveActiveComboer(comboObj);
}

void GravityBlast::Activate(int playerIndex, V2d pos, V2d dir)
{
	sess->AddEnemy(this);

	//ResetEnemy();
	V2d normalizedDir = normalize(dir);

	currPosInfo.position = pos;
	action = S_EXPLODE;
	frame = 0;
	comboObj->enemyHitboxInfo->hitPosType = HitboxInfo::HitPosType::OMNI;
	comboObj->enemyHitboxInfo->hDir = V2d(0, 0);//normalizedDir;
	sess->PlayerAddActiveComboObj(comboObj, playerIndex);

	UpdateSprite();
}

void GravityBlast::ProcessState()
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
			sess->RemoveEnemy(this);
			break;
		}
	}

	//V2d playerPos = owner->GetPlayer(0)->position;
}

void GravityBlast::HandleNoHealth()
{

}

void GravityBlast::UpdateEnemyPhysics()
{
	/*switch (action)
	{
	case S_SHOT:
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(data.velocity);*/
}

void GravityBlast::FrameIncrement()
{
	/*if (action == S_SHOT)
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
	}*/
}

void GravityBlast::ComboHit()
{
	pauseFrames = 6;
	++data.currHits;
	/*if (data.currHits >= hitLimit)
	{
		action = S_EXPLODE;
		frame = 0;
	}*/
}

void GravityBlast::UpdateSprite()
{
	sprite.setPosition(GetPositionF());
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setRotation(0);

	testCircle.setPosition(GetPositionF());
}

void GravityBlast::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	target->draw(testCircle); 
}

int GravityBlast::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void GravityBlast::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void GravityBlast::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}