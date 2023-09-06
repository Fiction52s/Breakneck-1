#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Patroller.h"
#include "Eye.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )




Patroller::Patroller(ActorParams *ap)//bool p_hasMonitor, Vector2i pos, list<Vector2i> &pathParam, bool loopP, int p_level )
	:Enemy( EnemyType::EN_PATROLLER, ap )//, p_hasMonitor, 1 )
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLAP, S_FLAP, 0);

	SetLevel(ap->GetLevel());

	pathFollower.SetParams(ap);

	ts = GetSizedTileset("Enemies/W1/patroller_256x256.png");
	shootSound = GetSound("Enemies/patroller_shoot");

	ts_bulletExplode = GetSizedTileset("FX/bullet_explode1_64x64.png");

	eye = new PatrollerEye(this);
	eye->SetPosition(GetPositionF());
	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = normalize(V2d(1, -.3));
	hitboxInfo->hType = HitboxInfo::BLUE;
	
	BasicRectHurtBodySetup(32, 60, 0, V2d(0, 30));//72, 0, V2d(0, 30));
	BasicRectHitBodySetup(32, 60, 0, V2d(0, 30));//72, 0, V2d(0, 30));
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
	launchers[0]->hitboxInfo->damage = 60;
	launchers[0]->hitboxInfo->hitstunFrames = 10;
	launchers[0]->hitboxInfo->hType = HitboxInfo::BLUE;


	
	ResetEnemy();
}

Patroller::~Patroller()
{
	delete eye;
}

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

void Patroller::ResetEnemy()
{
	SetCurrPosInfo(startPosInfo);
	pathFollower.Reset();
	facingRight = origFacingRight;
	data.aimingFrames = 0;
	if (facingRight)
	{
		data.turnFrame = (6 * turnAnimFactor) - 1;
	}
	else
	{
		data.turnFrame = 0;
	}
	data.targetAngle = 0;
	data.currentAngle = data.targetAngle;

	eye->Reset();
	data.fireCounter = 0;
	action = S_FLAP;
	frame = 0;

	SetHitboxes(&hitBody, 0);
	SetHurtboxes(&hurtBody, 0);

	UpdateHitboxes();
	UpdateSprite();

	data.targetAngle = 0;
	data.currentAngle = data.targetAngle;
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
			launchers[0]->facingDir = normalize(data.targetPos - GetPosition());
			launchers[0]->Fire();
			sess->ActivateSoundAtPos( GetPosition(), shootSound);
			break;
		}
		case S_BEAKHOLDOPEN:
			action = S_BEAKCLOSE;
			break;
		case S_BEAKCLOSE:
			action = S_FLAP;
			data.aimingFrames = 0;
			break;
		}
	}

	switch (action)
	{
	case S_FLAP:
	{
		sf::Vector2f dir = Vector2f(normalize(playerPos - GetPosition()));
		float dist = length(playerPos - GetPosition());
		data.targetPos = playerPos;
		data.targetAngle = -atan2(dir.y, -dir.x) + PI / 2;
		if (data.targetAngle < 0)
			data.targetAngle += 2 * PI;
		else if (data.targetAngle > 2 * PI)
		{
			data.targetAngle -= 2 * PI;
		}

		if (playerPos.x < GetPosition().x)
		{
			if (data.turnFrame > 0)
				--data.turnFrame;
		}
		else if (playerPos.x > GetPosition().x)
		{
			if (data.turnFrame < 6 * turnAnimFactor)
				++data.turnFrame;
		}

		if (eye->IsEyeActivated())
		{
			if (data.aimingFrames == maxAimingFrames)
			{
				if (data.turnFrame == 0 || data.turnFrame == 6 * turnAnimFactor
					&& abs(data.targetAngle - data.currentAngle ) < .3)
				{
					action = S_BEAKOPEN;
					frame = 0;
				}
			}
			else
			{
				data.aimingFrames++;
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
		if (data.currentAngle > data.targetAngle)
		{
			float diff = data.currentAngle - data.targetAngle;
			float revDiff = (2 * PI - data.currentAngle) + data.targetAngle;
			if (diff >= revDiff)
			{
				data.currentAngle += beakTurnSpeed;
				if (data.currentAngle >= 2 * PI)
				{
					data.currentAngle -= 2 * PI;
				}
			}
			else
			{
				data.currentAngle -= beakTurnSpeed;
				if (data.currentAngle < 0)
					data.currentAngle += 2 * PI;
			}
		}
		else if (data.currentAngle < data.targetAngle)
		{
			float diff = data.targetAngle - data.currentAngle;
			float revDiff = (2 * PI - data.targetAngle) + data.currentAngle;

			if (diff >= revDiff)
			{
				data.currentAngle -= beakTurnSpeed;
				if (data.currentAngle < 0)
					data.currentAngle += 2 * PI;
			}
			else
			{
				data.currentAngle += beakTurnSpeed;
				if (data.currentAngle >= 2 * PI)
				{
					data.currentAngle -= 2 * PI;
				}
			}
		}

		if (abs(data.targetAngle - data.currentAngle) < beakTurnSpeed * 2)
		{
			data.currentAngle = data.targetAngle;
		}
	}
}

void Patroller::HandleHitAndSurvive()
{
	data.fireCounter = 0;
}

void Patroller::HandleNoHealth()
{
	cutObject->SetFlipHoriz( sin(data.currentAngle ) < 0 );
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

void Patroller::DirectKill()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[i]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}

	Enemy::DirectKill();
}

void Patroller::UpdateSprite()
{
	Vector2f posF = GetPositionF();
	eye->SetPosition(posF);
	eye->UpdateSprite();

	SetRectCenter(bodyVA, ts->tileWidth * scale, ts->tileHeight * scale, posF );
	SetRectCenter(bodyVA + 4, ts->tileWidth * scale, ts->tileHeight * scale, posF);
	
	bool turnedLeft;
	if (data.turnFrame == 0)
		turnedLeft = true;
	else
		turnedLeft = false;
	

	if (action == S_FLAP)
	{
		SetRectRotation(bodyVA + 4, data.currentAngle, ts->tileWidth * scale, ts->tileHeight * scale,
			posF);
		SetRectSubRect(bodyVA, ts->GetSubRect(frame / animFactor[S_FLAP]));
		SetRectSubRect(bodyVA + 4, ts->GetSubRect(27 + data.turnFrame / turnAnimFactor));
	}
	else if (action == S_BEAKOPEN)
	{
		SetRectRotation(bodyVA, 0, ts->tileWidth * scale, ts->tileHeight * scale,
			posF, !turnedLeft);
		SetRectRotation(bodyVA + 4, data.currentAngle, ts->tileWidth * scale, ts->tileHeight * scale,
			posF, !turnedLeft);
		SetRectSubRect(bodyVA, ts->GetSubRect(24 + frame / animFactor[S_BEAKOPEN]));
		SetRectSubRect(bodyVA + 4, ts->GetSubRect(34 + frame / animFactor[S_BEAKOPEN]));
	}
	else if (action == S_BEAKCLOSE)
	{
		SetRectRotation(bodyVA, 0, ts->tileWidth * scale, ts->tileHeight * scale,
			posF, !turnedLeft);
		SetRectRotation(bodyVA + 4, data.currentAngle, ts->tileWidth * scale, ts->tileHeight * scale,
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
		SetRectRotation(bodyVA + 4, data.currentAngle, ts->tileWidth * scale, ts->tileHeight * scale,
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
	//bool b = (sess->GetPauseFrames() < 2 && pauseFrames < 2 && !pauseFramesFromAttacking) || (receivedHit == NULL && pauseFrames < 2);

	bool drawHurtShader = (pauseFrames >= 2 && !pauseFramesFromAttacking) && currShield == NULL;

	RenderStates rs;
	rs.texture = ts->texture;

	if( hasMonitor && !suppressMonitor )
	{			
		if(drawHurtShader)
		{
			rs.shader = &hurtShader;
			//target->draw( sprite, hurtShader );

			target->draw(bodyVA, 8, sf::Quads, rs);
			eye->Draw(target, &hurtShader);
		}
		else
		{
			rs.shader = &keyShader;
			//target->draw( sprite, keyShader );

			target->draw(bodyVA, 8, sf::Quads, rs);
			eye->Draw(target, &keyShader);
			
		}
		target->draw( keySprite );
	}
	else
	{
		if(drawHurtShader)
		{
			//target->draw( sprite );
			
			rs.shader = &hurtShader;
			//target->draw( sprite, hurtShader );

			target->draw(bodyVA, 8, sf::Quads, rs);
			eye->Draw(target, &hurtShader);
			
		}
		else
		{

			target->draw(bodyVA, 8, sf::Quads, rs);
			eye->Draw(target);

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

void Patroller::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);

	//launchers[0]->hitboxInfo->kbDir = normalize( b->velocity );
	//launchers[0]->hitboxInfo->knockback = 10;
	//owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
	
	b->launcher->DeactivateBullet(b);
}

void Patroller::UpdateHitboxes()
{
	hurtBody.SetBasicPos(GetPosition(), data.currentAngle);
	hitBody.SetBasicPos(GetPosition(), data.currentAngle);

	BasicUpdateHitboxInfo();
}

int Patroller::GetNumStoredBytes()
{
	//return 0;
	return sizeof(MyData) +eye->GetNumStoredBytes() + pathFollower.GetNumStoredBytes() + launchers[0]->GetNumStoredBytes();
}

void Patroller::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
	
	eye->StoreBytes(bytes);

	bytes += eye->GetNumStoredBytes();

	pathFollower.StoreBytes(bytes);

	bytes += pathFollower.GetNumStoredBytes();

	launchers[0]->StoreBytes(bytes);

	bytes += launchers[0]->GetNumStoredBytes();
}

void Patroller::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);

	bytes += sizeof(MyData);

	eye->SetFromBytes(bytes);

	bytes += eye->GetNumStoredBytes();

	pathFollower.SetFromBytes(bytes);

	bytes += pathFollower.GetNumStoredBytes();

	launchers[0]->SetFromBytes(bytes);

	bytes += launchers[0]->GetNumStoredBytes();
}