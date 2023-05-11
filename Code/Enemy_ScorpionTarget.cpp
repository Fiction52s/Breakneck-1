#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_ScorpionTarget.h"

using namespace std;
using namespace sf;

ScorpionTarget::ScorpionTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_SCORPIONTARGET, ap)
{
	SetNumActions(S_Count);
	SetEditorActions(S_NEUTRAL, S_NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	ts = GetSizedTileset("Enemies/healthfly_64x64.png");

	BasicCircleHitBodySetup(32);

	sprite.setColor(Color::Yellow);
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	actionLength[S_NEUTRAL] = 5;
	animFactor[S_NEUTRAL] = 5;

	cutObject->Setup(ts, 0, 1, scale, 0, false, false);

	ResetEnemy();
}

void ScorpionTarget::SetLevel(int lev)
{
	level = lev;
	switch (level)
	{
	case 1:
		scale = 4.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 4.0;
		maxHealth += 5;
		break;
	}
}

void ScorpionTarget::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void ScorpionTarget::ResetEnemy()
{
	action = S_NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void ScorpionTarget::Collect()
{
	if (numHealth > 0)
	{
		numHealth = 0;
		sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		ConfirmKill();
	}
}

//void ScorpionTarget::IHitPlayer(int index)
//{
//	Actor *p = sess->GetPlayer(index);
//
//	if (p->action == Actor::SPRINGSTUNGLIDE)
//	{
//		Collect();
//	}
//}

void ScorpionTarget::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
	}
}

void ScorpionTarget::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(frame / animFactor[S_NEUTRAL]));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void ScorpionTarget::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

int ScorpionTarget::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void ScorpionTarget::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void ScorpionTarget::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}