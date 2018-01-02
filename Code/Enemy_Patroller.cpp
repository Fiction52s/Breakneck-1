#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Patroller.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


Patroller::Patroller( GameSession *owner, bool p_hasMonitor, Vector2i pos, list<Vector2i> &pathParam, bool loopP, int pspeed )
	:Enemy( owner, EnemyType::EN_PATROLLER, p_hasMonitor, 1 )
{
	action = FLAP;
	//receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

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

	animationFactor = 2;

	//ts = owner->GetTileset( "patroller.png", 80, 80 );
	ts = owner->GetTileset( "patroller_224x272.png", 224, 272 );
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
	cutObject->SetSubRectFront(12);
	cutObject->SetSubRectBack(13);

	facingRight = true;
	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	
	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	actionLength[FLAP] = 12 * animationFactor;
	actionLength[TRANSFORM] = 240;
	actionLength[CHARGEDFLAP] = 12 * animationFactor;

	fireCounter = 0;

	launchers = new Launcher*[1];
	launchers[0] = new Launcher(this, BasicBullet::BAT, owner, 16, 1, position, V2d(1, 0), 0, 300);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->damage = 18;
}

void Patroller::ResetEnemy()
{
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	fireCounter = 0;
	//cout << "resetting enemy" << endl;
	//spawned = false;
	targetNode = 1;
	forward = true;
	dead = false;
	action = FLAP;
	frame = 0;
	position.x = path[0].x;
	position.y = path[0].y;
	receivedHit = NULL;
	

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Patroller::ProcessState()
{
	if (frame == actionLength[action])
	{
		switch (action)
		{
		case FLAP:
			frame = 0;
			break;
		case TRANSFORM:
			action = CHARGEDFLAP;
			frame = 0;
			fireCounter = 0;
			break;
		case CHARGEDFLAP:
			frame = 0;
			break;
		}
	}

	//launcher->UpdatePrePhysics();

	switch (action)
	{
	case FLAP:
		if (length(position - owner->GetPlayer(0)->position) < 1000)
		{
			action = TRANSFORM;
			frame = 0;
		}
		break;
	case TRANSFORM:

		break;
	case CHARGEDFLAP:
		break;
	}

	if (action == CHARGEDFLAP && fireCounter == 60)// frame == 0 && slowCounter == 1 )
	{
		launchers[0]->position = position;
		V2d targetPoint = V2d(path[targetNode].x, path[targetNode].y);
		launchers[0]->facingDir = normalize(owner->GetPlayer(0)->position - position);//normalize(position - targetPoint);//normalize(owner->GetPlayer(0)->position - position);
																				  //cout << "shooting bullet at: " << launcher->facingDir.x <<", " <<
																				  //	launcher->facingDir.y << endl;
		launchers[0]->Fire();
		fireCounter = 0;
		//testLauncher->Fire();
	}
}

void Patroller::HandleHitAndSurvive()
{
	fireCounter = 0;
}

void Patroller::HandleNoHealth()
{
	cutObject->SetFlipHoriz(!facingRight);
	cutObject->SetCutRootPos(Vector2f( position ));
}

void Patroller::UpdateEnemyPhysics()
{
	double movement = speed / NUM_STEPS;

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
	if (action == CHARGEDFLAP)
	{
		++fireCounter;
	}
}

void Patroller::UpdateSprite()
{
	sprite.setPosition( position.x, position.y );

	if (action == FLAP || action == CHARGEDFLAP)
	{
		sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));
	}
	else
	{
		int f = frame / (actionLength[TRANSFORM] / 5);
		sprite.setTextureRect(ts->GetSubRect( 0 ) );
	}
}

void Patroller::EnemyDraw( sf::RenderTarget *target )
{
	if( hasMonitor && !suppressMonitor )
	{
		if( owner->pauseFrames < 2 || receivedHit == NULL )
		{
			target->draw( sprite, keyShader );
		}
		else
		{
			target->draw( sprite, hurtShader );
		}
		target->draw( *keySprite );
	}
	else
	{
		if( owner->pauseFrames < 2 || receivedHit == NULL )
		{
			target->draw( sprite );
		}
		else
		{
			target->draw( sprite, hurtShader );
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