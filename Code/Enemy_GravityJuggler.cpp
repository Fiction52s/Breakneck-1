#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GravityJuggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

void GravityJuggler::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();

	JugglerParams *jParams = (JugglerParams*)editParams;
	juggleReps = jParams->numJuggles;
	
	if (juggleReps == 0)
	{
		limitedJuggles = false;
	}
	else
	{
		limitedJuggles = true;
		UpdateJuggleRepsText(juggleReps);
	}
}

void GravityJuggler::SetLevel(int lev)
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

GravityJuggler::GravityJuggler(ActorParams *ap)
	:Enemy(EnemyType::EN_GRAVITYJUGGLER, ap)
{
	enemyDrawLayer = ENEMYDRAWLAYER_COMBOER;

	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	actionLength[S_FLOAT] = 18;
	actionLength[S_POP] = 60;
	actionLength[S_JUGGLE] = 10;
	
	actionLength[S_RETURN] = 30;

	animFactor[S_FLOAT] = 2;
	animFactor[S_POP] = 1;
	animFactor[S_JUGGLE] = 1;
	
	animFactor[S_RETURN] = 1;

	SetLevel(ap->GetLevel());
	
	numJugglesText.setFont(sess->mainMenu->arial);
	numJugglesText.setFillColor(Color::White);
	numJugglesText.setOutlineColor(Color::Black);
	numJugglesText.setOutlineThickness(3);
	numJugglesText.setCharacterSize(32);
	

	const string &typeName = ap->GetTypeName();
	if (typeName == "downgravityjuggler" )
	{
		reversedGrav = false;
		
	}
	else if (typeName == "upgravityjuggler")
	{
		reversedGrav = true;
	}

	UpdateParamsSettings();
	
	maxWaitFrames = 90;

	gravFactor = 1.0;

	gDir = V2d(0, 1);
	if (reversedGrav)
	{
		gDir.y = -gDir.y;
		//sprite.setColor(Color::Green);
	}
	else
	{
		//sprite.setColor(Color::Red);
	}
	maxFallSpeed = 15;

	ts = GetSizedTileset("Enemies/General/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);
	


	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;*/

	BasicCircleHurtBodySetup(48);
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
	comboObj->enemyHitboxInfo->hType = HitboxInfo::GREEN;
	

	comboObj->enemyHitBody.BasicCircleSetup(48, GetPosition());

	

	ResetEnemy();
}

GravityJuggler::~GravityJuggler()
{
}

void GravityJuggler::UpdateJuggleRepsText( int reps )
{
	if (limitedJuggles)
	{
		data.juggleTextNumber = reps;
		numJugglesText.setString(to_string(reps));
		numJugglesText.setOrigin(numJugglesText.getLocalBounds().left
			+ numJugglesText.getLocalBounds().width / 2,
			numJugglesText.getLocalBounds().top
			+ numJugglesText.getLocalBounds().height / 2);
		numJugglesText.setPosition(sprite.getPosition());
	}
}

void GravityJuggler::ResetEnemy()
{
	data.hitLimit = -1;
	data.currHits = 0;
	data.currJuggle = 0;
	data.velocity = V2d(0, 0);
	data.doneBeingHittable = false;

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	
	comboObj->Reset();
	

	DefaultHurtboxesOn();

	dead = false;
	action = S_FLOAT;
	frame = 0;
	receivedHit.SetEmpty();
	UpdateHitboxes();

	UpdateJuggleRepsText(juggleReps);

	UpdateSprite();
}

void GravityJuggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	data.velocity = vel;
}

void GravityJuggler::Throw(V2d vel)
{
	data.velocity = vel;
}

void GravityJuggler::Return()
{
	action = S_RETURN;
	frame = 0;

	sess->PlayerRemoveActiveComboer(comboObj);

	HurtboxesOff();

	data.doneBeingHittable = true;

	data.currJuggle = 0;

	UpdateJuggleRepsText(0);

	numHealth = maxHealth;
}

void GravityJuggler::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++data.currJuggle;
	
	HurtboxesOff();
	data.waitFrame = 0;

	UpdateJuggleRepsText(juggleReps - data.currJuggle);
}

void GravityJuggler::PopThrow()
{
	action = S_POP;
	frame = 0;

	V2d dir; 

	dir = receivedHit.hDir;//normalize(receivedHit->hDir);

	if (!HitboxInfo::IsAirType(receivedHit.hitPosType))
	{
		if ((dir.y > 0 && !reversedGrav ) || (dir.y < 0 && reversedGrav))
		{
			dir.y = 0;
		}
	}

	V2d hit(0, -25);//-20);

	double extraX = 10;//8;

	if (dir.x > 0)
	{
		hit.x = extraX;
	}
	else if( dir.x < 0 )
	{
		hit.x = -extraX;
	}

	

	if ((dir.y > 0 && !reversedGrav) || (dir.y < 0 && reversedGrav))
	{
		hit.y = 10;
	}

	bool pFacingRight = sess->PlayerIsFacingRight(0);

	/*if (((dir.y == 1 && !reversedGrav) || (dir.y == -1 && reversedGrav))
		&& dir.x == 0)
	{
		hit.y += 3;
		if (pFacingRight)
		{
			hit.x -= extraX / 2.0;
		}
		else
		{
			hit.x += extraX / 2.0;
		}

	}*/

	if (reversedGrav)
	{
		hit.y = -hit.y;
	}

	Pop();

	Throw(hit);

	sess->PlayerAddActiveComboObj(comboObj);
}

void GravityJuggler::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		//Actor *player = owner->GetPlayer(0);
		if (numHealth <= 0)
		{
			if ( limitedJuggles && data.currJuggle == juggleReps - 1)
			{
				if (hasMonitor && !suppressMonitor)
				{
					ActivateDarkAbsorbParticles();
					suppressMonitor = true;
					PlayKeyDeathSound();
				}

				sess->PlayerConfirmEnemyNoKill(this);
				ConfirmHitNoKill();

				data.doneBeingHittable = true;

				PopThrow();
			}
			else
			{
				if (!limitedJuggles)
				{
					if (hasMonitor && !suppressMonitor)
					{
						ActivateDarkAbsorbParticles();
						suppressMonitor = true;
						PlayKeyDeathSound();
					}

					sess->PlayerConfirmEnemyNoKill(this);
					ConfirmHitNoKill();
				}

				PopThrow();
			}
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}

		receivedHit.SetEmpty();
	}
}

void GravityJuggler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_RETURN:
			UpdateJuggleRepsText(juggleReps);
			SetCurrPosInfo(startPosInfo);
			DefaultHurtboxesOn();
			action = S_FLOAT;
			data.doneBeingHittable = false;
			frame = 0;
			//DefaultHitboxesOn();
			break;
		/*case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->PlayerRemoveActiveComboer(comboObj);
			break;*/
		}
	}

	/*if (action != S_FLOAT && action != S_EXPLODE && action != S_RETURN)
	{
	sf::Rect<double> r(position.x - 50, position.y - 50, 100, 100);
	owner->activeEnemyItemTree->Query(this, r);
	}*/

	double verticalLimitToBeHittable = -6;
	if (action == S_POP && frame >= 20)//( (data.velocity.y >= verticalLimitToBeHittable && !reversedGrav) || (data.velocity.y <= -verticalLimitToBeHittable && reversedGrav ) ) )
	{
		action = S_JUGGLE;
		frame = 0;

		if (!data.doneBeingHittable)
		{
			DefaultHurtboxesOn();
		}
		
	}
}

void GravityJuggler::HandleNoHealth()
{	
	
}

void GravityJuggler::Move()
{
	double numStep = numPhysSteps;
	V2d movementVec = data.velocity;
	movementVec /= slowMultiple * numStep;

	currPosInfo.position += movementVec;

	data.velocity += gDir * (gravFactor / numStep / slowMultiple);

	if (reversedGrav)
	{
		if (data.velocity.y < -maxFallSpeed)
		{
			data.velocity.y = -maxFallSpeed;
		}
	}
	else
	{
		if (data.velocity.y > maxFallSpeed)
		{
			data.velocity.y = maxFallSpeed;
		}
	}
}

void GravityJuggler::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_POP:
	case S_JUGGLE:
	{
		Move();
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(data.velocity);
}

void GravityJuggler::FrameIncrement()
{
	if (action == S_POP || action == S_JUGGLE )
	{
		if (data.waitFrame == maxWaitFrames)
		{
			Return();
		}
		else
		{
			data.waitFrame++;
		}
		
	}
	
}

bool GravityJuggler::CanComboHit(Enemy *e)
{
	if (e->type == EN_GRAVITYJUGGLER)
	{
		return false;
	}

	return true;
}

void GravityJuggler::ComboHit()
{
	pauseFrames = 6;
	++data.currHits;
	if (data.hitLimit > 0 && data.currHits >= data.hitLimit)
	{
		Return();
	}
}

void GravityJuggler::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		//sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	if (reversedGrav)
	{
		sprite.setTextureRect(ts->GetSubRect(2));
	}
	else
	{
		sprite.setTextureRect(ts->GetSubRect(3));
	}

	if (data.doneBeingHittable)
	{
		sprite.setColor(Color::Blue);
	}
	else
	{
		sprite.setColor(Color::White);
	}
	
	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

	if (limitedJuggles)
	{
		numJugglesText.setPosition(sprite.getPosition());
	}
}

void GravityJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (limitedJuggles)
	{
		target->draw(numJugglesText);
	}
}

int GravityJuggler::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void GravityJuggler::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void GravityJuggler::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);

	UpdateJuggleRepsText(data.juggleTextNumber);
	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}