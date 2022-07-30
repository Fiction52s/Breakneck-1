#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GlideTarget.h"

using namespace std;
using namespace sf;

GlideTarget::GlideTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_GLIDETARGET, ap)
{
	SetNumActions(S_Count);
	SetEditorActions(S_NEUTRAL, S_NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	ts = GetSizedTileset("Enemies/healthfly_64x64.png");

	BasicCircleHitBodySetup(32);

	sprite.setColor(Color::Green);
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	actionLength[S_NEUTRAL] = 5;
	animFactor[S_NEUTRAL] = 5;

	cutObject->Setup(ts, 0, 1, scale, 0, false, false);

	ResetEnemy();
}

void GlideTarget::SetLevel(int lev)
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

void GlideTarget::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void GlideTarget::ResetEnemy()
{
	action = S_NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void GlideTarget::Collect()
{
	if (numHealth > 0)
	{
		numHealth = 0;
		sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		ConfirmKill();
	}
}

//void GlideTarget::IHitPlayer(int index)
//{
//	Actor *p = sess->GetPlayer(index);
//
//	if (p->action == Actor::SPRINGSTUNGLIDE)
//	{
//		Collect();
//	}
//}

void GlideTarget::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
	}
}

void GlideTarget::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(frame / animFactor[S_NEUTRAL]));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void GlideTarget::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

int GlideTarget::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void GlideTarget::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void GlideTarget::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}