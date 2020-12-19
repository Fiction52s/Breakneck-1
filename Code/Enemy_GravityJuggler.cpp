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
#include "AbsorbParticles.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

void GravityJuggler::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();
	if (limitedJuggles)
	{
		JugglerParams *jParams = (JugglerParams*)editParams;
		juggleReps = jParams->numJuggles;
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
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	actionLength[S_FLOAT] = 18;
	actionLength[S_POP] = 10;
	actionLength[S_JUGGLE] = 10;
	actionLength[S_RETURN] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_POP] = 1;
	animFactor[S_JUGGLE] = 1;
	animFactor[S_RETURN] = 6;

	SetLevel(ap->GetLevel());
	
	numJugglesText.setFont(sess->mainMenu->arial);
	numJugglesText.setFillColor(Color::White);
	numJugglesText.setOutlineColor(Color::Black);
	numJugglesText.setOutlineThickness(3);
	numJugglesText.setCharacterSize(32);
	

	string &typeName = ap->type->info.name;
	if (typeName == "downgravityjuggler" )
	{
		reversedGrav = false;
		limitedJuggles = false;
	}
	else if (typeName == "limiteddowngravityjuggler")
	{
		reversedGrav = false;
		limitedJuggles = true;
	}
	else if (typeName == "upgravityjuggler")
	{
		reversedGrav = true;
		limitedJuggles = false;
	}
	else if (typeName == "limitedupgravityjuggler")
	{
		reversedGrav = true;
		limitedJuggles = true;
	}

	
	UpdateParamsSettings();
	
	maxWaitFrames = 180;

	guidedDir = NULL;
	/*juggleReps = path.size();

	guidedDir = NULL;
	if (juggleReps > 0)
	{
		Vector2i prev(0, 0);
		guidedDir = new V2d[juggleReps];
		int ind = 0;
		for (auto it = path.begin(); it != path.end(); ++it)
		{
			if (it != path.begin())
			{
				it--;
				prev = (*it);
				it++;
			}

			guidedDir[ind] = normalize(V2d((*it) - prev));

			++ind;
		}
	}

	if (guidedDir == NULL)*/
	//juggleReps = jReps;



	//position.x = pos.x;
	//position.y = pos.y;
	//origPos = position;
	//spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	

	gravFactor = 1.0;

	gDir = V2d(0, 1);
	if (reversedGrav)
	{
		gDir.y = -gDir.y;
		sprite.setColor(Color::Green);
	}
	else
	{
		sprite.setColor(Color::Red);
	}
	maxFallSpeed = 15;

	reversed = true;

	ts = sess->GetTileset("Enemies/Comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(0));
	
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
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::COMBO;

	comboObj->enemyHitBody.BasicCircleSetup(48, GetPosition());
	comboObj->enemyHitboxFrame = 0;

	

	ResetEnemy();
}

GravityJuggler::~GravityJuggler()
{
	if (guidedDir != NULL)
		delete[] guidedDir;
}

void GravityJuggler::UpdateJuggleRepsText( int reps )
{
	if (limitedJuggles)
	{
		numJugglesText.setString(to_string(reps));
		numJugglesText.setOrigin(numJugglesText.getLocalBounds().left
			+ numJugglesText.getLocalBounds().width / 2,
			numJugglesText.getLocalBounds().top
			+ numJugglesText.getLocalBounds().height / 2);
	}
}

void GravityJuggler::ResetEnemy()
{
	hitLimit = -1;
	action = S_FLOAT;

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	currHits = 0;
	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;
	velocity = V2d(0, 0);

	DefaultHurtboxesOn();
	//DefaultHitboxesOn();

	dead = false;
	action = S_FLOAT;
	frame = 0;
	receivedHit = NULL;
	UpdateHitboxes();

	currJuggle = 0;

	UpdateJuggleRepsText(juggleReps);

	UpdateSprite();
}

void GravityJuggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	velocity = vel;
}

void GravityJuggler::Throw(V2d vel)
{
	velocity = vel;
}

void GravityJuggler::Return()
{
	sess->PlayerRemoveActiveComboer(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	currJuggle = 0;

	UpdateJuggleRepsText(0);

	numHealth = maxHealth;
}

void GravityJuggler::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++currJuggle;
	
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	waitFrame = 0;

	UpdateJuggleRepsText(juggleReps - currJuggle);
}

void GravityJuggler::PopThrow()
{
	V2d dir; 

	if (guidedDir == NULL)
	{
		dir = receivedHit->hDir;//normalize(receivedHit->hDir);
	}
	else
	{
		dir = guidedDir[currJuggle];
	}


	V2d hit(0, -20);

	double extraX = 8;

	if (dir.x != 0)
	{
		hit.x += dir.x * extraX;
	}

	bool pFacingRight = sess->PlayerIsFacingRight(0);

	if (((dir.y == 1 && !reversedGrav) || (dir.y == -1 && reversedGrav))
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

	}

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
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		//Actor *player = owner->GetPlayer(0);
		if (numHealth <= 0)
		{
			if ( limitedJuggles && currJuggle == juggleReps - 1)
			{
				if (hasMonitor && !suppressMonitor)
				{
					sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
						sess->GetPlayer(0), 1, GetPosition());
					suppressMonitor = true;
				}

				sess->PlayerConfirmEnemyNoKill(this);
				ConfirmHitNoKill();

				action = S_RETURN;
				frame = 0;

				Return();
			}
			else
			{
				action = S_POP;
				frame = 0;
				PopThrow();
			}
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}
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


	if (action == S_POP && ( (velocity.y >= 0 && !reversedGrav) || ( velocity.y <= 0 && reversedGrav ) ) )
	{
		action = S_JUGGLE;
		frame = 0;
		DefaultHurtboxesOn();
	}
}

void GravityJuggler::HandleNoHealth()
{	
	
}

void GravityJuggler::Move()
{
	double numStep = numPhysSteps;
	V2d movementVec = velocity;
	movementVec /= slowMultiple * numStep;

	currPosInfo.position += movementVec;

	velocity += gDir * (gravFactor / numStep / slowMultiple);

	if (reversedGrav)
	{
		if (velocity.y < -maxFallSpeed)
		{
			velocity.y = -maxFallSpeed;
		}
	}
	else
	{
		if (velocity.y > maxFallSpeed)
		{
			velocity.y = maxFallSpeed;
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

	comboObj->enemyHitboxInfo->hDir = normalize(velocity);
}

void GravityJuggler::FrameIncrement()
{
	if (action == S_POP || action == S_JUGGLE)
	{
		if (waitFrame == maxWaitFrames)
		{
			action = S_RETURN;
			frame = 0;
			Return();
		}
		else
		{
			waitFrame++;
		}
		
	}
	
}

void GravityJuggler::ComboHit()
{
	pauseFrames = 5;
	++currHits;
	if ( hitLimit > 0 && currHits >= hitLimit)
	{
		action = S_RETURN;
		frame = 0;
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
		sprite.setTextureRect(ts->GetSubRect(tile));
		break;
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