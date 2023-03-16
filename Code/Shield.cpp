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

	hitboxInfo = hInfo;
	if (hitboxInfo == NULL && type == T_REFLECT)
	{
		assert(0 && "needs a hitbox info");
	}

	Reset();
}

Shield::~Shield()
{
}

void Shield::Reset()
{
	data.currHits = maxHits;
	data.action = S_WAIT;
	data.frame = 0;
	data.pauseFrames = 0;
}

//returns false when the shield is dead
bool Shield::ProcessState()
{
	receivedHit.SetEmpty();

	if (data.pauseFrames > 0)
		return true;

	
	switch (data.action)
	{
	case S_WAIT:
		break;
	case S_HURT:

		break;
	case S_BREAK:
		if (data.frame == 20)
		{
			return false;
		}
		break;
	case S_REFLECT:
		break;
	}

	switch (data.action)
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
	assert(!receivedHit.IsEmpty());

	if (receivedHit.hType != HitboxInfo::COMBO)
	{
		//parent->sess->Pause(5);
		data.pauseFrames = receivedHit.hitlagFrames;
	}
	else
	{
		//cout << "set pause frames 5" << endl;
		data.pauseFrames = receivedHit.hitlagFrames;
	}

	parent->pauseFrames = data.pauseFrames;

	parent->sess->cam.SetRumble(.5, .5, data.pauseFrames);
}

V2d Shield::GetPosition()
{
	return body.GetBasicPos();
}


void Shield::ConfirmKill()
{
	assert(!receivedHit.IsEmpty());

	if (receivedHit.hType != HitboxInfo::COMBO)
	{
		//parent->sess->Pause(7);
		data.pauseFrames = 7;
	}
	else
	{
		data.pauseFrames = 7;
		//cout << "set pause frames 7" << endl;
	}

	parent->pauseFrames = data.pauseFrames;

	//parent->sess->ActivateEffect(EffectLayer::BEHIND_ENEMIES, parent->ts_killSpack, GetPosition(), true, 0, 10, 5, true);
	parent->sess->cam.SetRumble(1, 1, 7);
}

void Shield::ProcessHit()
{
	if (data.action != S_BREAK && HasReceivedHit() && data.currHits > 0)
	{
		--data.currHits;
		if (data.currHits == 0)
		{
			data.action = S_BREAK;
			data.frame = 0;
			ConfirmHitNoKill();
			parent->sess->PlayerConfirmEnemyKill(parent, GetReceivedHitPlayerIndex());
		}
		else
		{
			parent->sess->PlayerConfirmEnemyNoKill(parent, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();
			if (sType == T_BLOCK)
			{
				data.action = S_HURT;
			}
			else if (sType == T_REFLECT && receivedHit.hType != HitboxInfo::HitboxType::COMBO)
			{
				data.action = S_REFLECT;
				parent->sess->PlayerApplyHit(GetReceivedHitPlayerIndex(),
					hitboxInfo, NULL, Actor::HitResult::HIT, GetPosition() );
			}
			data.frame = 0;
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
	if (data.action == S_BREAK)
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
	++data.frame;
}

void Shield::UpdateSprite()
{
	switch (data.action)
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

	if (data.pauseFrames > 0)
	{
		test.setFillColor(Color( 255, 255, 255, 80 ));
	}
	else
	{
		test.setFillColor(Color(255, 0, 0, 40));
	}
	
	test.setRadius(radius);
	test.setOrigin(test.getLocalBounds().width / 2, test.getLocalBounds().height / 2);
	test.setPosition(pos.x, pos.y);

	target->draw(test);
}

void Shield::SetPosition(V2d &pos)
{
	body.SetBasicPos(pos);
}

int Shield::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Shield::StoreBytes(unsigned char *bytes)
{
	data.hittableObjectData.receivedHit = receivedHit;
	data.hittableObjectData.receivedHitPlayerIndex = receivedHitPlayerIndex;
	data.hittableObjectData.comboHitEnemy = comboHitEnemy;
	data.hittableObjectData.numHealth = numHealth;
	data.hittableObjectData.specterProtected = specterProtected;

	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Shield::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	
	receivedHit = data.hittableObjectData.receivedHit;
	receivedHitPlayerIndex = data.hittableObjectData.receivedHitPlayerIndex;
	comboHitEnemy = data.hittableObjectData.comboHitEnemy;
	numHealth = data.hittableObjectData.numHealth;
	specterProtected = data.hittableObjectData.specterProtected;

	bytes += sizeof(MyData);
}