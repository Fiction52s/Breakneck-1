#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_TetheredRusher.h"
#include "Actor.h"

using namespace std;
using namespace sf;

TetheredRusher::TetheredRusher(ActorParams *ap)
	:Enemy(EnemyType::EN_TETHEREDRUSHER, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 1;
	actionLength[RUSH] = 14;
	actionLength[RECOVER] = 40;

	animFactor[NEUTRAL] = 1;
	animFactor[RUSH] = 1;
	animFactor[RECOVER] = 1;

	accel = 10.0;

	maxSpeed = 50;

	chainRadius = 500;

	ts = GetSizedTileset("Enemies/W6/rusher_160x160.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(0);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 8;
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	anchorRadius = 30;

	BasicCircleHitBodySetup(48);
	BasicCircleHurtBodySetup(anchorRadius);

	hitBody.hitboxInfo = hitboxInfo;

	

	Color circleColor = Color::Red;
	circleColor.a = 40;
	testCircle.setFillColor(circleColor);
	testCircle.setRadius(chainRadius);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2,
		testCircle.getLocalBounds().height / 2);

	circleColor = Color::Black;
	anchorCircle.setOutlineColor(Color::Red);
	anchorCircle.setOutlineThickness(5);
	anchorCircle.setFillColor(circleColor);
	anchorCircle.setRadius(anchorRadius);
	anchorCircle.setOrigin(anchorCircle.getLocalBounds().width / 2,
		anchorCircle.getLocalBounds().height / 2);


	for (int i = 0; i < NUM_SEGMENTS; ++i)
	{
		ts->SetQuadSubRect(segmentQuads + i * 4, 0);
	}

	attackMovement= data.ms.AddLineMovement(V2d(), V2d(), CubicBezier(), actionLength[RUSH] * animFactor[RUSH]);

	//needs 1 less frame of movement than the length of the animation for some reason. Otherwise it slowly moves
	//off of its base. works with this fix though.
	retreatMovement = data.ms.AddLineMovement(V2d(), V2d(), CubicBezier(), actionLength[RECOVER] * animFactor[RECOVER] - 1);
	

	ResetEnemy();
}

void TetheredRusher::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void TetheredRusher::SetLevel(int lev)
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

bool TetheredRusher::IsSlowed(int index)
{
	//needs to use the hitboxes because of the head,
	//the anchor should not slow the head down.
	Actor *player = sess->GetPlayer(index);
	return (player->IntersectMySlowboxes(currHitboxes, currHitboxFrame));
}

void TetheredRusher::UpdateHitboxes()
{
	V2d position = GetPosition();

	double ang = GetGroundedAngleRadians();
	//can update this with a universal angle at some point
	if (!hurtBody.IsEmpty())
	{
		hurtBody.SetBasicPos(anchorPos, ang);
	}

	if (!hitBody.IsEmpty())
	{
		hitBody.SetBasicPos(position, ang);
	}

	auto comboBoxes = GetComboHitboxes();
	if (comboBoxes != NULL)
	{
		for (auto it = comboBoxes->begin(); it != comboBoxes->end(); ++it)
		{
			(*it).globalPosition = position;
		}
	}

	BasicUpdateHitboxInfo();
}

void TetheredRusher::ResetEnemy()
{
	/*if (GetPosition().x < sess->playerOrigPos[0].x)
	facingRight = false;
	else
	facingRight = true;*/

	anchorPos = GetPosition();

	anchorCircle.setPosition(Vector2f(anchorPos));

	action = NEUTRAL;
	frame = 0;

	testCircle.setPosition(Vector2f(anchorPos));

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void TetheredRusher::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case RUSH:
			action = RECOVER;
			break;
		case RECOVER:
			action = NEUTRAL;
			break;
		}
	}
}

void TetheredRusher::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case NEUTRAL:
		if (dist < DEFAULT_DETECT_RADIUS)
		{
			action = RUSH;
			frame = 0;

			attackMovement->start = GetPosition();
			attackMovement->end = GetPosition() + dir * chainRadius;

			retreatMovement->start = attackMovement->end;
			retreatMovement->end = attackMovement->start;

			if (attackMovement->start.x - attackMovement->end.x <= 0)
			{
				facingRight = true;
			}
			else
			{
				facingRight = false;
			}
			data.ms.Reset();
		}
		break;
	case RUSH:
		break;
	case RECOVER:
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		data.velocity = V2d(0, 0);
		break;
	case RUSH:
		break;
	case RECOVER:
		break;
	}
}

void TetheredRusher::UpdateEnemyPhysics()
{
	if (action == RUSH || action == RECOVER)
	{
		data.ms.Update(slowMultiple, NUM_MAX_STEPS / numPhysSteps);
		currPosInfo.position = data.ms.GetPos();
	}
}

void TetheredRusher::UpdateSprite()
{
	int trueFrame;
	/*switch (action)
	{
	case NEUTRAL:
		sprite.setColor(Color::White);
		break;
	case RUSH:
		sprite.setColor(Color::Green);
		break;
	case RECOVER:
		sprite.setColor(Color::Blue);
		break;
	}*/

	if (action == RUSH || action == RECOVER)
	{
		if (data.ms.IsMovementActive())
		{
			int t = 0;
			for (int i = 0; i < NUM_SEGMENTS; ++i)
			{
				int currTime = data.ms.data.currTime - data.ms.data.currMovementStartTime;

				if (data.ms.data.currMovement == attackMovement)
				{
					t = currTime * (i / (double)(NUM_SEGMENTS));
				}
				else if (data.ms.data.currMovement == retreatMovement )
				{
					t = data.ms.data.currMovement->duration -
						(data.ms.data.currMovement->duration - currTime)
						* (i / (double)(NUM_SEGMENTS));
				}

				SetRectCenter(segmentQuads + i * 4,
					ts->tileWidth, ts->tileHeight,
					Vector2f(data.ms.data.currMovement->GetPosition(t)));
			}
		}
		else
		{
			//assert(false);
		}

	}
	

	int tile = 1;

	switch (action)
	{
	case NEUTRAL:
		tile = 1;
		break;
	case RUSH:
		tile = frame / 2 + 2;
		break;
	case RECOVER:
		if (frame < 15)
		{
			tile = 8;
		}
		else
		{
			tile = 9;
		}
		
		break;
	}


	ts->SetSubRect(sprite, tile, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	if (action == RUSH)
	{
		V2d attackDir = normalize(attackMovement->end - attackMovement->start);
		double ang = GetVectorAngleCW(attackDir);
		ang = ang / PI * 180.0;
		if (!facingRight)
		{
			ang = -ang + 90;
		}
		sprite.setRotation(ang);
	}

}

void TetheredRusher::EnemyDraw(sf::RenderTarget *target)
{
	//target->draw(testCircle);


	target->draw(anchorCircle);

	if (action == RUSH || action == RECOVER)
	{
		target->draw(segmentQuads, NUM_SEGMENTS * 4, sf::Quads, ts->texture);
	}

	DrawSprite(target, sprite);

	
}


int TetheredRusher::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void TetheredRusher::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void TetheredRusher::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}