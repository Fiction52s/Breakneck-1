#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Airdasher.h"
#include "Eye.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


Airdasher::Airdasher(GameSession *owner, bool p_hasMonitor, Vector2i pos, int p_level )
	:Enemy(owner, EnemyType::EN_AIRDASHER, p_hasMonitor, 1)
{
	level = p_level;

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

	hitLimit = 5;
	action = S_FLOAT;
	//receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;
	origPos = position;

	dashRadius = 600;//500;
	dashFrames = 36;
	returnFrames = 30;

	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	frame = 0;

	//ts = owner->GetTileset( "Airdasher.png", 80, 80 );
	ts = owner->GetTileset("Enemies/dasher_208x144.png", 208, 144);
	ts_aura = owner->GetTileset("Enemies/dasher_aura_208x144.png", 208, 144);

	auraSprite.setTexture(*ts_aura->texture);

	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos.x, pos.y);

	auraSprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	SetupBodies(1, 1);
	AddBasicHurtCircle(48);
	AddBasicHitCircle(48);
	hitBody->hitboxInfo = hitboxInfo;

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

	comboObj->enemyHitBody = new CollisionBody(2);

	comboObj->enemyHitBody->AddCollisionBox(0, hitBody->GetCollisionBoxes(0)->front());

	CollisionBox exBox;
	exBox.type = CollisionBox::Hit;
	exBox.isCircle = true;
	exBox.globalAngle = 0;
	exBox.offset.x = 0;
	exBox.offset.y = 0;
	exBox.rw = 48 * scale;
	exBox.rh = 48 * scale;

	comboObj->enemyHitBody->AddCollisionBox(1, exBox);

	comboObj->enemyHitboxFrame = 0;

	//hitboxInfo->kbDir;

	targetNode = 1;
	forward = true;

	dead = false;

	
	facingRight = true;

	UpdateHitboxes();

	actionLength[S_FLOAT] = 11;
	actionLength[S_DASH] = 30;
	actionLength[S_RETURN] = 60;
	actionLength[S_OUT] = 20;
	actionLength[S_COMBO] = 40;

	animFactor[S_FLOAT] = 4;
	animFactor[S_DASH] = 1;
	animFactor[S_RETURN] = 1;
	animFactor[S_OUT] = 1;
	animFactor[S_COMBO] = 1;

	ResetEnemy();

	maxCharge = 15;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(15);
	cutObject->SetSubRectBack(14);
	cutObject->SetScale(scale);
}

Airdasher::~Airdasher()
{
	delete comboObj;
}

void Airdasher::ComboHit()
{
	pauseFrames = 5;
	++currHits;
	if (currHits >= hitLimit)
	{
		dead = true;
		owner->PlayerRemoveActiveComboer(comboObj);
		velocity = V2d(0, 0);
	}
}

void Airdasher::ResetEnemy()
{
	hitFrame = -1;

	currHits = 0;
	currOrig = origPos;
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	dead = false;
	action = S_FLOAT;
	frame = 0;
	position = origPos;
	receivedHit = NULL;
	comboObj->Reset();

	sprite.setRotation(0);

	UpdateHitboxes();

	UpdateSprite();
}

void Airdasher::ProcessHit()
{
	if ( action == S_DASH && !dead && ReceivedHit() && numHealth > 0)
	{
		owner->PlayerConfirmEnemyNoKill(this);
		ConfirmHitNoKill();
		action = S_COMBO;
		frame = 0;
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		V2d dir;
		double speed = 20;
		facingRight = !facingRight;

		comboObj->enemyHitboxInfo->hDir = -playerDir;//receivedHit->hDir;
		dir = -playerDir;
		velocity = dir * speed;

		owner->PlayerAddActiveComboObj(comboObj);
	}
	else
	{
		Enemy::ProcessHit();
	}

	//might add more later to return
}

void Airdasher::ProcessState()
{
	V2d playerPos = owner->GetPlayerPos();
	

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_DASH:
		{
			//action = S_RETURN;
			break;
		}
		case S_RETURN:
			//action = S_FLOAT;
			break;
		case S_OUT:
		{
			action = S_RETURN;
			sprite.setRotation(0);
			V2d pDir = normalize(playerPos - position);
			SetFacingSide(pDir);
			break;
		}
		case S_COMBO:
			dead = true;
			owner->PlayerRemoveActiveComboer(comboObj);
			break;
		}
	}

	bool withinRange = length(playerPos - position) < ( dashRadius + 10.0 );
	if (withinRange)
	{
		if (action == S_FLOAT)
		{
			action = S_CHARGE;
			frame = 0;
			chargeFrames = 0;
			playerDir = normalize(playerPos - position);
			SetFacingPlayerAngle();
		}
		else if (action == S_CHARGE)
		{
			playerDir = normalize(playerPos - position);
			SetFacingPlayerAngle();
			if (chargeFrames == maxCharge)
			{
				action = S_DASH;
				frame = 0;
				physStepIndex = 0;
				
				//cout << "angle : " << angle << endl;
			}
			else
			{
				chargeFrames++;
			}
		}
	}
	else
	{
		if (action == S_CHARGE)
		{
			action = S_FLOAT;
			frame = 0;
			sprite.setRotation(0);
		}
	}
	
}

void Airdasher::IHitPlayer(int index)
{

}

void Airdasher::SetFacingSide( V2d pDir )
{
	if (pDir.x < 0)
	{
		facingRight = false;
	}
	else if (pDir.x > 0)
	{
		facingRight = true;
	}
	else if (pDir.y == 1)
	{
		facingRight = true;
	}
	else if (pDir.y == -1)
	{
		facingRight = false;
	}
	else
	{
		assert(0);
	}
}

double Airdasher::SetFacingPlayerAngle()
{
	double angle = -atan2(playerDir.y, -playerDir.x) / PI * 180.0;
	

	if (playerDir.x < 0)
	{
		facingRight = false;
	}
	else if (playerDir.x > 0)
	{
		facingRight = true;
		angle += 180;
	}
	else if (playerDir.y == 1)
	{
		facingRight = true;
	}
	else if (playerDir.y == -1)
	{
		facingRight = false;
	}
	else
	{
		assert(0);
	}

	sprite.setRotation(angle);

	return angle;
}

void Airdasher::UpdateEnemyPhysics()
{
	V2d playerPos = owner->GetPlayerPos();
	V2d dest = currOrig + playerDir * dashRadius;
	switch (action)
	{
	case S_DASH:
	{
		double a = (double)physStepIndex / (dashFrames * NUM_MAX_STEPS * 5);
		if (a > 1.0)
		{
			action = S_OUT;
			frame = 0;
			physStepIndex = 0;
			break;
		}
		double f = dashBez.GetValue(a);
		double rf = 1.0 - f;

		position = currOrig * rf + dest * f;


		int steps = (5 / slowMultiple) * NUM_MAX_STEPS / numPhysSteps;

		physStepIndex += steps;
		break;
	}
	case S_RETURN:
	{
		V2d d = currOrig + playerDir * ( dashRadius / 2.0 );
		double a = (double)physStepIndex / (returnFrames * NUM_MAX_STEPS * 5);
		if (a > 1.0)
		{
			action = S_CHARGE;
			playerDir = normalize(playerPos - position);
			SetFacingPlayerAngle();
			chargeFrames = maxCharge - 5;
			frame = 0;
			currOrig = position;
			break;
		}
		double f = returnBez.GetValue(a);
		double rf = 1.0 - f;

		position = dest * rf + d * f;
		

		int steps = (5 / slowMultiple) * NUM_MAX_STEPS / numPhysSteps;

		physStepIndex += steps;
		break;
	}
	case S_COMBO:
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		position += movementVec;
		break;
	}
		
	}
}

void Airdasher::FrameIncrement()
{
	/*if (action == CHARGEDFLAP)
	{
	++fireCounter;
	}*/
}

void Airdasher::UpdateSprite()
{
	sprite.setPosition(position.x, position.y);

	int tIndex = 0;

	
	switch (action)
	{
	case S_FLOAT:
		tIndex = frame / animFactor[S_FLOAT];
		break;
	case S_CHARGE:
		//sprite.setRotation(angle);
		tIndex = 11;
		break;
	case S_DASH:
		tIndex = 12;
		break;
	case S_OUT:
		tIndex = 11;
		break;
	case S_RETURN:
		//tIndex = 0;
		tIndex = (frame / animFactor[S_FLOAT]) % actionLength[S_FLOAT];
		break;
	case S_COMBO:
		tIndex = 13;
		break;
	}

	IntRect ir = ts->GetSubRect(tIndex);
	if (!facingRight)
	{
		ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
	}
	sprite.setTextureRect(ir);

	SyncSpriteInfo(auraSprite, sprite);

}

void Airdasher::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(auraSprite);
	DrawSpriteIfExists(target, sprite);
}