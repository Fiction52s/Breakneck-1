#include "Shield.h"
#include "Physics.h"
#include "Actor.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

Shield::Shield(ShieldType type, float rad, int maxH, Enemy *e)
	:sType( type ), maxHits( maxH ), radius( rad ), parent( e )
{
	CollisionBox box;
	box.type = CollisionBox::Hurt;
	box.isCircle = true;
	box.globalAngle = 0;
	box.offset.x = 0;
	box.offset.y = 0;
	box.rw = radius;
	box.rh = radius;
	body = new CollisionBody(1);
	body->AddCollisionBox(0, box);
	pauseFrames = 0;
}

Shield::~Shield()
{
	delete body;
}

void Shield::Reset()
{
	currHits = maxHits;
	action = S_WAIT;
	frame = 0;
	pauseFrames = 0;
}

//returns false when the shield is dead
bool Shield::ProcessState()
{
	receivedHit = NULL;

	if (pauseFrames > 0)
		return true;

	
	switch (action)
	{
	case S_WAIT:
		break;
	case S_HURT:

		break;
	case S_BREAK:
		if (frame == 20)
		{
			return false;
		}
		break;
	case S_REFLECT:
		break;
	}

	switch (action)
	{
	case S_WAIT:
		break;
	case S_HURT:
		break;
	case S_BREAK:
		break;
	case S_REFLECT:
		break;
	}

	return true;
}

void Shield::ConfirmHitNoKill()
{
	assert(receivedHit != NULL);

	if (receivedHit->hType != HitboxInfo::COMBO)
	{
		parent->owner->Pause(5);
		pauseFrames = 0;
	}
	else
	{
		//cout << "set pause frames 5" << endl;
		pauseFrames = 5;
	}

	parent->owner->cam.SetRumble(.5, .5, 5);
}

V2d Shield::GetPosition()
{
	return body->GetCollisionBoxes(0)->front().globalPosition;
}

void Shield::ConfirmKill()
{
	assert(receivedHit != NULL);

	if (receivedHit->hType != HitboxInfo::COMBO)
	{
		parent->owner->Pause(7);
		pauseFrames = 0;
	}
	else
	{
		pauseFrames = 7;
		//cout << "set pause frames 7" << endl;
	}

	parent->owner->ActivateEffect(EffectLayer::BEHIND_ENEMIES, parent->ts_killSpack, GetPosition(), true, 0, 10, 5, true);
	parent->owner->cam.SetRumble(1, 1, 7);
}

void Shield::ProcessHit()
{
	if (action != S_BREAK && ReceivedHit() && currHits > 0)
	{
		cout << "hits before: " << currHits << endl;
		--currHits;
		if (currHits == 0)
		{
			action = S_BREAK;
			frame = 0;
			ConfirmHitNoKill();
			parent->owner->GetPlayer(0)->ConfirmEnemyKill(parent);
		}
		else
		{
			parent->owner->GetPlayer(0)->ConfirmEnemyNoKill(parent);
			ConfirmHitNoKill();
			if (sType == T_BLOCK)
			{
				action = S_HURT;
			}
			else if (sType == T_REFLECT)
			{
				action = S_REFLECT;
			}
			frame = 0;
		}
	}
	/*else if (action == S_BREAK && ReceivedHit())
	{
		parent->owner->GetPlayer(0)->ConfirmEnemyNoKill(parent);
		ConfirmHitNoKill();
	}*/
}

HitboxInfo * Shield::IsHit(Actor *player)
{
	if (action == S_BREAK)
	{
		return NULL;
	}

	if (player->IntersectMyHitboxes(body, 0))
	{
		return player->currHitboxes->hitboxInfo;
	}

	ComboObject *co = player->IntersectMyComboHitboxes(body, 0);
	if (co != NULL)
	{
		HitboxInfo *hi = co->enemyHitboxInfo;

		co->enemy->ComboHit();
		return hi;
	}

	return NULL;
}

void Shield::FrameIncrement()
{
		++frame;
}

void Shield::UpdateSprite()
{
	switch (action)
	{
	case S_WAIT:
		break;
	case S_HURT:
		break;
	case S_BREAK:
		break;
	case S_REFLECT:
		break;
	}
}

void Shield::Draw(sf::RenderTarget *target)
{
	V2d pos = body->GetCollisionBoxes(0)->front().globalPosition;
	sf::CircleShape test;
	test.setFillColor(Color( 255, 0, 0, 40));
	test.setRadius(radius);
	test.setOrigin(test.getLocalBounds().width / 2, test.getLocalBounds().height / 2);
	test.setPosition(pos.x, pos.y);
	target->draw(test);
}

void Shield::SetPosition(V2d &pos)
{
	body->GetCollisionBoxes(0)->front().globalPosition = pos;
}