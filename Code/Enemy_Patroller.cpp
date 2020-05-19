#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Patroller.h"
#include "Eye.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

//static Enemy *Patroller::Create(ActorParams *ap)
//{
//
//}

void Patroller::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	pathFollower.SetParams(ap);
}

void Patroller::UpdatePath()
{
	pathFollower.SetParams(editParams);
}

void Patroller::SetLevel(int lev)
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

Patroller::Patroller(ActorParams *ap)//bool p_hasMonitor, Vector2i pos, list<Vector2i> &pathParam, bool loopP, int p_level )
	:Enemy( EnemyType::EN_PATROLLER, ap )//, p_hasMonitor, 1 )
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLAP, S_FLAP, 0);

	SetLevel(ap->GetLevel());

	pathFollower.SetParams(ap);

	ts = sess->GetTileset("Enemies/patroller_256x256.png", 256, 256);
	ts_aura = sess->GetTileset("Enemies/patroller_aura_256x256.png", 256, 256);
	shootSound = sess->GetSound("Enemies/patroller_shoot");

	eye = new PatrollerEye(this);
	eye->SetPosition(GetPositionF());
	
	
	BasicRectHurtBodySetup(32, 72, 0, V2d(0, 30));
	BasicRectHitBodySetup(32, 72, 0, V2d(0, 30));

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3*60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitBody.hitboxInfo = hitboxInfo;

	cutObject->Setup(ts, 37, 38, scale);

	origFacingRight = true;

	actionLength[S_FLAP] = 18;
	actionLength[S_BEAKOPEN] = 3;
	actionLength[S_BEAKHOLDOPEN] = 20;
	actionLength[S_BEAKCLOSE] = 3;

	animFactor[S_FLAP] = 2;
	animFactor[S_BEAKOPEN] = 6;
	animFactor[S_BEAKHOLDOPEN] = 1;
	animFactor[S_BEAKCLOSE] = 6;

	turnAnimFactor = 4;
	maxAimingFrames = 35;
	speed = 3;
	beakTurnSpeed = .13;

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::PATROLLER, 16, 1, GetPosition(), V2d(1, 0), 0, 200, false);
	launchers[0]->SetBulletSpeed(5);//70);
	launchers[0]->hitboxInfo->damage = 18;
	

	ResetEnemy();

	SetSpawnRect();
}

Patroller::~Patroller()
{
	delete eye;
}

void Patroller::ResetEnemy()
{
	SetCurrPosInfo(startPosInfo);
	pathFollower.Reset();
	facingRight = origFacingRight;
	aimingFrames = 0;
	if (facingRight)
	{
		turnFrame = (6 * turnAnimFactor) - 1;
	}
	else
	{
		turnFrame = 0;
	}
	targetAngle = 0;
	currentAngle = targetAngle;

	eye->Reset();
	fireCounter = 0;
	action = S_FLAP;
	frame = 0;

	SetHitboxes(&hitBody, 0);
	SetHurtboxes(&hurtBody, 0);

	UpdateHitboxes();
	UpdateSprite();

	targetAngle = 0;
	currentAngle = targetAngle;
}

void Patroller::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);

	eye->ProcessState(Vector2f(playerPos));

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case S_FLAP:
			break;
		case S_BEAKOPEN:
		{
			action = S_BEAKHOLDOPEN;
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = normalize(targetPos - GetPosition());
			launchers[0]->Fire();
			sess->ActivateSoundAtPos( GetPosition(), shootSound);
			break;
		}
		case S_BEAKHOLDOPEN:
			action = S_BEAKCLOSE;
			break;
		case S_BEAKCLOSE:
			action = S_FLAP;
			aimingFrames = 0;
			break;
		}
	}

	switch (action)
	{
	case S_FLAP:
	{
		sf::Vector2f dir = Vector2f(normalize(playerPos - GetPosition()));
		float dist = length(playerPos - GetPosition());
		targetPos = playerPos;
		targetAngle = -atan2(dir.y, -dir.x) + PI / 2;
		if (targetAngle < 0)
			targetAngle += 2 * PI;
		else if (targetAngle > 2 * PI)
		{
			targetAngle -= 2 * PI;
		}

		if (playerPos.x < GetPosition().x)
		{
			if (turnFrame > 0)
				--turnFrame;
		}
		else if (playerPos.x > GetPosition().x)
		{
			if (turnFrame < 6 * turnAnimFactor)
				++turnFrame;
		}

		if (eye->IsEyeActivated())
		{
			if (aimingFrames == maxAimingFrames)
			{
				if (turnFrame == 0 || turnFrame == 6 * turnAnimFactor
					&& abs( targetAngle - currentAngle ) < .3)
				{
					action = S_BEAKOPEN;
					frame = 0;
				}
			}
			else
			{
				aimingFrames++;
			}
		}
		break;
	}
	case S_BEAKOPEN:
		break;
	case S_BEAKHOLDOPEN:
		
		break;
	case S_BEAKCLOSE:
		break;

	}


	if (action != S_BEAKHOLDOPEN)
	{
		if (currentAngle > targetAngle)
		{
			float diff = currentAngle - targetAngle;
			float revDiff = (2 * PI - currentAngle) + targetAngle;
			if (diff >= revDiff)
			{
				currentAngle += beakTurnSpeed;
				if (currentAngle >= 2 * PI)
				{
					currentAngle -= 2 * PI;
				}
			}
			else
			{
				currentAngle -= beakTurnSpeed;
				if (currentAngle < 0)
					currentAngle += 2 * PI;
			}
		}
		else if (currentAngle < targetAngle)
		{
			float diff = targetAngle - currentAngle;
			float revDiff = (2 * PI - targetAngle) + currentAngle;

			if (diff >= revDiff)
			{
				currentAngle -= beakTurnSpeed;
				if (currentAngle < 0)
					currentAngle += 2 * PI;
			}
			else
			{
				currentAngle += beakTurnSpeed;
				if (currentAngle >= 2 * PI)
				{
					currentAngle -= 2 * PI;
				}
			}
		}

		if (abs(targetAngle - currentAngle) < beakTurnSpeed * 2)
		{
			currentAngle = targetAngle;
		}
	}
}

void Patroller::HandleHitAndSurvive()
{
	fireCounter = 0;
}

void Patroller::HandleNoHealth()
{
	cutObject->SetFlipHoriz( sin( currentAngle ) < 0 );
	//cutObject->SetCutRootPos(Vector2f( position ));
}

void Patroller::UpdateEnemyPhysics()
{
	double movement = speed / numPhysSteps;
	movement /= (double)slowMultiple;
	pathFollower.Move(movement, currPosInfo.position);
}

void Patroller::FrameIncrement()
{
	/*if (action == CHARGEDFLAP)
	{
		++fireCounter;
	}*/
}

sf::FloatRect Patroller::GetAABB()
{
	return GetQuadAABB(bodyVA);
}

void Patroller::UpdateSprite()
{
	Vector2f posF = GetPositionF();
	eye->SetPosition(posF);
	eye->UpdateSprite();

	SetRectCenter(bodyVA, ts->tileWidth * scale, ts->tileHeight * scale, posF );
	SetRectCenter(bodyVA + 4, ts->tileWidth * scale, ts->tileHeight * scale, posF);
	
	bool turnedLeft;
	if (turnFrame == 0)
		turnedLeft = true;
	else
		turnedLeft = false;
	

	if (action == S_FLAP)
	{
		SetRectRotation(bodyVA + 4, currentAngle, ts->tileWidth * scale, ts->tileHeight * scale,
			posF);
		SetRectSubRect(bodyVA, ts->GetSubRect(frame / animFactor[S_FLAP]));
		SetRectSubRect(bodyVA + 4, ts->GetSubRect(27 + turnFrame / turnAnimFactor));
	}
	else if (action == S_BEAKOPEN)
	{
		SetRectRotation(bodyVA, 0, ts->tileWidth * scale, ts->tileHeight * scale,
			posF, !turnedLeft);
		SetRectRotation(bodyVA + 4, currentAngle, ts->tileWidth * scale, ts->tileHeight * scale,
			posF, !turnedLeft);
		SetRectSubRect(bodyVA, ts->GetSubRect(24 + frame / animFactor[S_BEAKOPEN]));
		SetRectSubRect(bodyVA + 4, ts->GetSubRect(34 + frame / animFactor[S_BEAKOPEN]));
	}
	else if (action == S_BEAKCLOSE)
	{
		SetRectRotation(bodyVA, 0, ts->tileWidth * scale, ts->tileHeight * scale,
			posF, !turnedLeft);
		SetRectRotation(bodyVA + 4, currentAngle, ts->tileWidth * scale, ts->tileHeight * scale,
			posF, !turnedLeft);
		SetRectSubRect(bodyVA, ts->GetSubRect(24 + ( (actionLength[S_BEAKCLOSE]-1)
			- frame / animFactor[S_BEAKOPEN]) ) );
		SetRectSubRect(bodyVA + 4, ts->GetSubRect(
			34 + ( (actionLength[S_BEAKCLOSE]-1) - frame / animFactor[S_BEAKOPEN]) ));
	}
	else if (action == S_BEAKHOLDOPEN)
	{
		SetRectRotation(bodyVA, 0, ts->tileWidth * scale, ts->tileHeight * scale,
			posF, !turnedLeft);
		SetRectRotation(bodyVA + 4, currentAngle, ts->tileWidth * scale, ts->tileHeight * scale,
			posF, !turnedLeft);
		SetRectSubRect(bodyVA, ts->GetSubRect(26));
		SetRectSubRect(bodyVA + 4, ts->GetSubRect(36));
	}
	
	

	//sprite.setPosition( position.x, position.y );

	/*if (action == FLAP || action == CHARGEDFLAP)
	{
		sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));
	}
	else
	{
		int f = frame / (actionLength[TRANSFORM] / 5);
		sprite.setTextureRect(ts->GetSubRect( 0 ) );
	}*/
}

void Patroller::EnemyDraw( sf::RenderTarget *target )
{
	bool b = (sess->GetPauseFrames() < 2 && pauseFrames < 2) || (receivedHit == NULL && pauseFrames < 2);

	RenderStates rs;
	rs.texture = ts->texture;

	RenderStates rsAura;
	rsAura.texture = ts_aura->texture;
	if( hasMonitor && !suppressMonitor )
	{			
		if( b )
		{
			rs.shader = &keyShader;
			//target->draw( sprite, keyShader );
			target->draw(bodyVA, 8, sf::Quads, rsAura);
			eye->Draw(target, &keyShader);
			target->draw(bodyVA, 8, sf::Quads, rs);
			
		}
		else
		{
			rs.shader = &hurtShader;
			//target->draw( sprite, hurtShader );
			target->draw(bodyVA, 8, sf::Quads, rsAura);
			eye->Draw(target, &hurtShader);
			target->draw(bodyVA, 8, sf::Quads, rs);
			
		}
		target->draw( keySprite );
	}
	else
	{
		if( b )
		{
			//target->draw( sprite );
			target->draw(bodyVA, 8, sf::Quads, rsAura);
			eye->Draw(target);
			target->draw(bodyVA, 8, sf::Quads, rs);
			
		}
		else
		{
			rs.shader = &hurtShader;
			target->draw(bodyVA, 8, sf::Quads, rsAura);
			//target->draw( sprite, hurtShader );
			eye->Draw(target, &hurtShader);
			target->draw(bodyVA, 8, sf::Quads, rs);

		}		
	}	
}

void Patroller::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	//V2d vel = b->velocity;
	//double angle = atan2( vel.y, vel.x );
	V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);

	//owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);
}

void Patroller::BulletHitPlayer(BasicBullet *b)
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	//owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
	sess->PlayerApplyHit(b->launcher->hitboxInfo);
	b->launcher->DeactivateBullet(b);
}

void Patroller::UpdateHitboxes()
{
	hurtBody.SetBasicPos(GetPosition(), currentAngle);
	hitBody.SetBasicPos(GetPosition(), currentAngle);

	BasicUpdateHitboxInfo();
}