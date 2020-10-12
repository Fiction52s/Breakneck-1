#include "Shield.h"
#include "Physics.h"
#include "Actor.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

Shield::Shield(ShieldType type, float rad, int maxH, Enemy *e, HitboxInfo *hInfo)
	:sType( type ), maxHits( maxH ), radius( rad ), parent( e ), body( CollisionBox::Hurt )
{
	body.BasicCircleSetup(radius);
	pauseFrames = 0;

	hitboxInfo = hInfo;
	if (hitboxInfo == NULL && type == T_REFLECT)
	{
		assert(0 && "needs a hitbox info");
	}
}

Shield::~Shield()
{
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
		parent->sess->Pause(5);
		pauseFrames = 0;
	}
	else
	{
		//cout << "set pause frames 5" << endl;
		pauseFrames = 5;
	}

	parent->sess->cam.SetRumble(.5, .5, 5);
}

V2d Shield::GetPosition()
{
	return body.GetBasicPos();
}


void Shield::ConfirmKill()
{
	assert(receivedHit != NULL);

	if (receivedHit->hType != HitboxInfo::COMBO)
	{
		parent->sess->Pause(7);
		pauseFrames = 0;
	}
	else
	{
		pauseFrames = 7;
		//cout << "set pause frames 7" << endl;
	}

	//parent->sess->ActivateEffect(EffectLayer::BEHIND_ENEMIES, parent->ts_killSpack, GetPosition(), true, 0, 10, 5, true);
	parent->sess->cam.SetRumble(1, 1, 7);
}

void Shield::ProcessHit()
{
	if (action != S_BREAK && ReceivedHit() && currHits > 0)
	{
		--currHits;
		if (currHits == 0)
		{
			action = S_BREAK;
			frame = 0;
			ConfirmHitNoKill();
			parent->sess->PlayerConfirmEnemyKill(parent, GetReceivedHitPlayerIndex());
		}
		else
		{
			parent->sess->PlayerConfirmEnemyNoKill(parent, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();
			if (sType == T_BLOCK)
			{
				action = S_HURT;
			}
			else if (sType == T_REFLECT && receivedHit->hType != HitboxInfo::HitboxType::COMBO)
			{
				action = S_REFLECT;
				parent->sess->PlayerApplyHit(GetReceivedHitPlayerIndex(),
					hitboxInfo, NULL, Actor::HitResult::HIT, GetPosition() );
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

HitboxInfo * Shield::IsHit(int pIndex )
{
	if (action == S_BREAK)
	{
		return NULL;
	}

	Actor *player = parent->sess->GetPlayer(pIndex);

	if (player->IntersectMyHitboxes(&body, 0))
	{
		return player->currHitboxes->hitboxInfo;
	}

	ComboObject *co = player->IntersectMyComboHitboxes(parent, &body, 0);
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
	V2d pos = body.GetBasicPos();
	sf::CircleShape test;
	test.setFillColor(Color( 255, 0, 0, 40));
	test.setRadius(radius);
	test.setOrigin(test.getLocalBounds().width / 2, test.getLocalBounds().height / 2);
	test.setPosition(pos.x, pos.y);
	target->draw(test);
}

void Shield::SetPosition(V2d &pos)
{
	body.SetBasicPos(pos);
}