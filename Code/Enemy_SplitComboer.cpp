#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SplitComboer.h"
#include "Eye.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

void SplitPiece::SetLevel(int lev)
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

SplitPiece::SplitPiece( SplitComboer *splitComb )
	:Enemy( EnemyType::EN_SPLITCOMBOER, NULL )//, false, 1, false)
{
	SetNumActions(S_Count);

	actionLength[S_FLY] = 120;
	actionLength[S_EXPLODE] = 20;

	animFactor[S_FLY] = 1;
	animFactor[S_EXPLODE] = 1;

	sc = splitComb;
	SetLevel(sc->level);
	
	Tileset *ts = sc->ts;
	sprite.setTexture(*ts->texture);
	//sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setScale(scale, scale);
	

	comboObj = new ComboObject(this);

	double rad = 48;

	comboObj->enemyHitBody.BasicCircleSetup(rad * scale);

	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::COMBO;

	facingRight = true;

	hitLimit = 6;

	ResetEnemy();
}

SplitPiece::~SplitPiece()
{
	//delete comboObj;
}

void SplitPiece::ResetEnemy()
{
	SetCurrPosInfo(sc->currPosInfo);

	sprite.setTextureRect(sc->ts->GetSubRect(0));
	sprite.setRotation(0);
	comboObj->Reset();
	data.velocity = V2d(0, 0);
	//SetHitboxes(&hitBody, 0);
	action = S_FLY;
	frame = 0;
	
	data.currHits = 0;
	UpdateHitboxes();

	UpdateSprite();
}

void SplitPiece::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_FLY:
			action = S_EXPLODE;
			frame = 0;
			break;
		case S_EXPLODE:
			numHealth = 0;
			dead = true;
			sess->PlayerRemoveActiveComboer(comboObj);
			break;
		}
	}
}

void SplitPiece::UpdateEnemyPhysics()
{
	if( action == S_FLY )
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}
}

void SplitPiece::ComboHit()
{
	pauseFrames = 5;
	++data.currHits;
	if (data.currHits >= hitLimit)
	{
		action = S_EXPLODE;
		frame = 0;
	}
}

void SplitPiece::UpdateSprite()
{
	sprite.setTextureRect(sc->ts->GetSubRect(1));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
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

void SplitPiece::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void SplitPiece::Shoot(V2d dir)
{
	comboObj->enemyHitboxInfo->hDir = dir;
	data.velocity = dir * sc->shootSpeed;
	sess->PlayerAddActiveComboObj(comboObj);
}

int SplitPiece::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void SplitPiece::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void SplitPiece::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void SplitComboer::SetLevel(int lev)
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

void SplitComboer::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	pathFollower.SetParams(ap);
}

void SplitComboer::UpdatePath()
{
	pathFollower.SetParams(editParams);
}

SplitComboer::SplitComboer( ActorParams *ap )//Vector2i pos, list<Vector2i> &pathParam,
	//bool p_loop,
	//int p_level)
	:Enemy(EnemyType::EN_SPLITCOMBOER, ap )//false, 1, false)
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, S_FLOAT, 0);

	SetLevel(ap->GetLevel());

	pathFollower.SetParams(ap);

	speed = 15;

	ts = GetSizedTileset("Enemies/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::BLUE;

	hitBody.hitboxInfo = hitboxInfo;

	facingRight = true;

	actionLength[S_FLOAT] = 18;
	actionLength[S_SPLIT] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_SPLIT] = 1;

	shootSpeed = 10;

	pieces[0] = new SplitPiece( this);
	pieces[1] = new SplitPiece( this);

	ResetEnemy();

	SetSpawnRect();
}

SplitComboer::~SplitComboer()
{
	delete pieces[0];
	delete pieces[1];
}


void SplitComboer::ResetEnemy()
{
	SetCurrPosInfo(startPosInfo);
	pieces[0]->Reset();
	pieces[1]->Reset();
	sprite.setTextureRect(ts->GetSubRect(1));
	sprite.setRotation(0);
	SetHitboxes(&hitBody, 0);
	SetHurtboxes(&hurtBody, 0);
	pathFollower.Reset();
	action = S_FLOAT;
	frame = 0;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
}

void SplitComboer::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		ConfirmHitNoKill();
		action = S_SPLIT;
		frame = 0;
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);
		//comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;

		V2d dir = normalize(receivedHit->hDir);
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
		}*/

		dir = normalize(dir);

		V2d dir0 = dir;
		V2d dir1 = dir;

		RotateCW(dir0, PI / 5);
		RotateCCW(dir1, PI / 5);

		sess->AddEnemy(pieces[0]);
		sess->AddEnemy(pieces[1]);

		pieces[0]->Shoot(dir0);
		pieces[1]->Shoot(dir1);
	}
}

void SplitComboer::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_SPLIT:
			numHealth = 0;
			dead = true;
			break;
		}
	}
}

void SplitComboer::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLOAT:
	{
		double movement = speed / numPhysSteps;
		movement /= (double)slowMultiple;
		pathFollower.Move(movement, currPosInfo.position);
		break;
	}
	}


}

void SplitComboer::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(1));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	
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

void SplitComboer::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void SplitComboer::CheckedMiniDraw(sf::RenderTarget *target,
	sf::FloatRect &rect)
{
	pieces[0]->CheckedMiniDraw(target, rect);
	pieces[1]->CheckedMiniDraw(target, rect);
	Enemy::CheckedMiniDraw(target, rect);
}

void SplitComboer::SetZoneSpritePosition()
{
	pieces[0]->SetZoneSpritePosition();
	pieces[1]->SetZoneSpritePosition();
	Enemy::SetZoneSpritePosition();
}

int SplitComboer::GetNumStoredBytes()
{
	return sizeof(MyData) + pieces[0]->GetNumStoredBytes() * NUM_PIECES + pathFollower.GetNumStoredBytes();
}

void SplitComboer::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	for (int i = 0; i < NUM_PIECES; ++i)
	{
		pieces[i]->StoreBytes(bytes);
		bytes += pieces[i]->GetNumStoredBytes();
	}

	pathFollower.StoreBytes(bytes);
	bytes += pathFollower.GetNumStoredBytes();
}

void SplitComboer::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	for (int i = 0; i < NUM_PIECES; ++i)
	{
		pieces[i]->SetFromBytes(bytes);
		bytes += pieces[i]->GetNumStoredBytes();
	}

	pathFollower.SetFromBytes(bytes);
	bytes += pathFollower.GetNumStoredBytes();
}