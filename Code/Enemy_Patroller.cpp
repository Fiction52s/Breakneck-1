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


Patroller::Patroller( GameSession *owner, bool p_hasMonitor, Vector2i pos, list<Vector2i> &pathParam, bool loopP, int pspeed )
	:Enemy( owner, EnemyType::EN_PATROLLER, p_hasMonitor, 1 )
{
	eye = new PatrollerEye(owner);
	
	action = S_FLAP;
	//receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;
	eye->SetPosition(Vector2f(pos));

	initHealth = 80;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	pathLength = pathParam.size() + 1;
	//cout << "pathLength: " << pathLength << endl;
	path = new Vector2i[pathLength];
	path[0] = pos;

	int index = 1;
	for( list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it )
	{
		path[index] = (*it) + pos;
		++index;
		//path.push_back( (*it) );

	}

	loop = loopP;
	
	//eventually maybe put this on a multiplier for more variation?
	//doubt ill need it though
	speed = pspeed;



	//speed = 2;
	frame = 0;
	beakTurnSpeed = .1;
	//ts = owner->GetTileset( "patroller.png", 80, 80 );
	ts = owner->GetTileset( "Enemies/patroller_256x256.png", 256, 256 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	//position.x = 0;
	//position.y = 0;
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 16;
	hurtBox.rh = 16;
	hurtBody = new CollisionBody(1);
	hurtBody->AddCollisionBox(0, hurtBox);

	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 16;
	hitBox.rh = 16;
	hitBody = new CollisionBody(1);
	hitBody->AddCollisionBox(0, hitBox);
	

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	hitBody->hitboxInfo = hitboxInfo;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	//hitboxInfo->kbDir;

	targetNode = 1;
	forward = true;

	dead = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(38);
	cutObject->SetSubRectBack(37);

	facingRight = true;
	
	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	
	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	actionLength[S_FLAP] = 18;
	actionLength[S_BEAKOPEN] = 3;
	actionLength[S_BEAKHOLDOPEN] = 20;
	actionLength[S_BEAKCLOSE] = 3;

	animFactor[S_FLAP] = 2;
	animFactor[S_BEAKOPEN] = 6;
	animFactor[S_BEAKHOLDOPEN] = 1;
	animFactor[S_BEAKCLOSE] = 6;

	fireCounter = 0;
	turnAnimFactor = 4;
	numLaunchers = 1;
	launchers = new Launcher*[numLaunchers];
	launchers[0] = new Launcher(this, BasicBullet::BAT, owner, 16, 1, position, V2d(1, 0), 0, 300);
	launchers[0]->SetBulletSpeed(20);//70);
	launchers[0]->hitboxInfo->damage = 18;
	maxAimingFrames = 60;

	ResetEnemy();
}

void Patroller::ResetEnemy()
{
	currFacingRight = facingRight;
	aimingFrames = 0;
	if (currFacingRight)
	{
		turnFrame = (7 * turnAnimFactor) - 1;
	}
	targetAngle = 0;
	currentAngle = targetAngle;

	eye->Reset();
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	fireCounter = 0;
	//cout << "resetting enemy" << endl;
	//spawned = false;
	targetNode = 1;
	forward = true;
	dead = false;
	action = S_FLAP;
	frame = 0;
	position.x = path[0].x;
	position.y = path[0].y;
	receivedHit = NULL;
	

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;

	targetAngle = 0;
	currentAngle = targetAngle;
}

void Patroller::ProcessState()
{
	eye->ProcessState(Vector2f(owner->GetPlayer(0)->position));

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
			launchers[0]->position = position;
			V2d targetPoint = V2d(path[targetNode].x, path[targetNode].y);
			launchers[0]->facingDir = normalize(targetPos - position);
			launchers[0]->Fire();
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
	V2d playerPos = owner->GetPlayer(0)->position;

	

	switch (action)
	{
	case S_FLAP:
	{
		sf::Vector2f dir = Vector2f(normalize(playerPos - position));
		float dist = length(playerPos - position);
		targetPos = playerPos;
		targetAngle = -atan2(dir.y, -dir.x) + PI / 2;
		if (targetAngle < 0)
			targetAngle += 2 * PI;
		else if (targetAngle > 2 * PI)
		{
			targetAngle -= 2 * PI;
		}

		if (playerPos.x < position.x)
		{
			if (turnFrame > 0)
				--turnFrame;
		}
		else if (playerPos.x > position.x)
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
		//case FLAP:
	/*		if (eye->IsEyeActivated())
			{
				action = TRANSFORM;
				frame = 0;
			}
			break;
		case TRANSFORM:
			if (!eye->IsEyeActivated())
			{
				action = FLAP;
				frame = 0;
			}
			break;
		case CHARGEDFLAP:
			if (!eye->IsEyeActivated())
			{
				action = FLAP;
				frame = 0;
			}
			break;*/

	//if (action == CHARGEDFLAP && fireCounter == 60)// frame == 0 && slowCounter == 1 )
	//{
	//	launchers[0]->position = position;
	//	V2d targetPoint = V2d(path[targetNode].x, path[targetNode].y);
	//	launchers[0]->facingDir = normalize(owner->GetPlayer(0)->position - position);//normalize(position - targetPoint);//normalize(owner->GetPlayer(0)->position - position);
	//																			  //cout << "shooting bullet at: " << launcher->facingDir.x <<", " <<
	//																			  //	launcher->facingDir.y << endl;
	//	launchers[0]->Fire();
	//	fireCounter = 0;
	//	//testLauncher->Fire();
	//}
}

void Patroller::HandleHitAndSurvive()
{
	fireCounter = 0;
}

void Patroller::HandleNoHealth()
{
	cutObject->SetFlipHoriz( sin( currentAngle ) < 0 );
	cutObject->SetCutRootPos(Vector2f( position ));
}

void Patroller::UpdateEnemyPhysics()
{
	double movement = speed / numPhysSteps;

	if( pathLength > 1 )
	{
		movement /= (double)slowMultiple;

		while( movement != 0 )
		{
			//cout << "movement loop? "<< endl;
			V2d targetPoint = V2d( path[targetNode].x, path[targetNode].y );
			V2d diff = targetPoint - position;
			double len = length( diff );
			if( len >= abs( movement ) )
			{
				position += normalize( diff ) * movement;
				movement = 0;
			}
			else
			{
				position += diff;
				movement -= length( diff );
				AdvanceTargetNode();	
			}
		}
	}
}

void Patroller::AdvanceTargetNode()
{
	if( loop )
	{
		++targetNode;
		if( targetNode == pathLength )
			targetNode = 0;
	}
	else
	{
		if( forward )
		{
			++targetNode;
			if( targetNode == pathLength )
			{
				targetNode -= 2;
				forward = false;
			}
		}
		else
		{
			--targetNode;
			if( targetNode < 0 )
			{
				targetNode = 1;
				forward = true;
			}
		}
	}
}

void Patroller::FrameIncrement()
{
	/*if (action == CHARGEDFLAP)
	{
		++fireCounter;
	}*/
}

void Patroller::UpdateSprite()
{
	eye->SetPosition(Vector2f(position));
	eye->UpdateSprite();

	SetRectCenter(bodyVA, ts->tileWidth, ts->tileHeight, Vector2f( position ) );
	SetRectCenter(bodyVA + 4, ts->tileWidth, ts->tileHeight, Vector2f(position));
	
	bool turnedLeft;
	if (turnFrame == 0)
		turnedLeft = true;
	else
		turnedLeft = false;
	

	if (action == S_FLAP)
	{
		SetRectRotation(bodyVA + 4, currentAngle, ts->tileWidth, ts->tileHeight,
			Vector2f(position));
		

		SetRectSubRect(bodyVA, ts->GetSubRect(frame / animFactor[S_FLAP]));
		SetRectSubRect(bodyVA + 4, ts->GetSubRect(27 + turnFrame / turnAnimFactor));
	}
	else if (action == S_BEAKOPEN)
	{
		SetRectRotation(bodyVA, 0, ts->tileWidth, ts->tileHeight,
			Vector2f(position), !turnedLeft);
		SetRectRotation(bodyVA + 4, currentAngle, ts->tileWidth, ts->tileHeight,
			Vector2f(position), !turnedLeft);
		SetRectSubRect(bodyVA, ts->GetSubRect(24 + frame / animFactor[S_BEAKOPEN]));
		SetRectSubRect(bodyVA + 4, ts->GetSubRect(34 + frame / animFactor[S_BEAKOPEN]));
	}
	else if (action == S_BEAKCLOSE)
	{
		SetRectRotation(bodyVA, 0, ts->tileWidth, ts->tileHeight,
			Vector2f(position), !turnedLeft);
		SetRectRotation(bodyVA + 4, currentAngle, ts->tileWidth, ts->tileHeight,
			Vector2f(position), !turnedLeft);
		SetRectSubRect(bodyVA, ts->GetSubRect(24 + ( (actionLength[S_BEAKCLOSE]-1)
			- frame / animFactor[S_BEAKOPEN]) ) );
		SetRectSubRect(bodyVA + 4, ts->GetSubRect(
			34 + ( (actionLength[S_BEAKCLOSE]-1) - frame / animFactor[S_BEAKOPEN]) ));
	}
	else if (action == S_BEAKHOLDOPEN)
	{
		SetRectRotation(bodyVA, 0, ts->tileWidth, ts->tileHeight,
			Vector2f(position), !turnedLeft);
		SetRectRotation(bodyVA + 4, currentAngle, ts->tileWidth, ts->tileHeight,
			Vector2f(position), !turnedLeft);
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
	
	RenderStates rs;
	rs.texture = ts->texture;
	if( hasMonitor && !suppressMonitor )
	{			
		if( owner->pauseFrames < 2 || receivedHit == NULL )
		{
			rs.shader = keyShader;
			//target->draw( sprite, keyShader );
			eye->Draw(target, keyShader);
			target->draw(bodyVA, 8, sf::Quads, rs);
			
		}
		else
		{
			rs.shader = hurtShader;
			//target->draw( sprite, hurtShader );
			eye->Draw(target, hurtShader);
			target->draw(bodyVA, 8, sf::Quads, rs);
			
		}
		target->draw( *keySprite );
	}
	else
	{
		if( owner->pauseFrames < 2 || receivedHit == NULL )
		{
			//target->draw( sprite );
			eye->Draw(target);
			target->draw(bodyVA, 8, sf::Quads, rs);
			
		}
		else
		{
			rs.shader = hurtShader;
			//target->draw( sprite, hurtShader );
			eye->Draw(target, hurtShader);
			target->draw(bodyVA, 8, sf::Quads, rs);

		}
			
	}
	

	
}

void Patroller::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	if( owner->GetPlayer( 0 )->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->groundSpeed * ( owner->GetPlayer( 0 )->ground->v1 - owner->GetPlayer( 0 )->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->velocity );
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
	owner->GetPlayer(0)->ApplyHit(b->launcher->hitboxInfo);
	b->launcher->DeactivateBullet(b);
}