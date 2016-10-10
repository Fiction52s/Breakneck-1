#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )


Owl::Owl( GameSession *owner, bool p_hasMonitor, Vector2i &pos, int p_bulletSpeed, int p_framesBetweenFiring, bool p_facingRight )
	:Enemy( owner, EnemyType::OWL, p_hasMonitor, 3 ), deathFrame( 0 ),flyingBez( 0, 0, 1, 1 )
{
	ts_death = owner->GetTileset( "owl_death_160x160.png", 160, 160 );
	ts_flap = owner->GetTileset( "owl_flap_160x160.png", 160, 160 );
	ts_spin = owner->GetTileset( "owl_spin_160x160.png", 160, 160 );
	ts_throw = owner->GetTileset( "owl_throw_160x160.png", 160, 160 );


	//movementRadius = 300;
	retreatRadius = 400;
	chaseRadius = 600;
	shotRadius = 800;
	flySpeed = 5.0;
	velocity = V2d( 0, 0 );
	action = REST;
	frame = 0;
	//actionLength[NEUTRAL] = 30;
	actionLength[FIRE] = 10 * 6;
	//actionLength[RETREAT] = 30;
	//actionLength[CHASE] = 30;
	actionLength[GUARD] = 120;
	actionLength[REST] = 60;
	actionLength[SPIN] = 60;

	hasGuard = true;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	originalPos = pos;

	facingRight = p_facingRight;

	bulletSpeed = p_bulletSpeed;
	framesBetween = p_framesBetweenFiring;

	deathFrame = 0;
	
	launcher = new Launcher( this, BasicBullet::OWL, owner, 16, 1, position, V2d( 1, 0 ), 0, 300 );
	launcher->SetBulletSpeed( bulletSpeed );
	launcher->hitboxInfo->damage = 18;

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );

	//ts = owner->GetTileset( "Owl.png", 80, 80 );
	ts = owner->GetTileset( "bat_48x48.png", 48, 48 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 16;
	hurtBody.rh = 16;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 16;
	hitBody.rh = 16;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	//hitboxInfo->kbDir;

	

	dead = false;
	dying = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	facingRight = true;
	 
	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//ts_blood = 
	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	ts_bulletExplode = owner->GetTileset( "bullet_explode3_64x64.png", 64, 64 );

	//cout << "finish init" << endl;
}

void Owl::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Owl::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	if( b->bounceCount == 2 )
	{
		V2d norm = edge->Normal();
		double angle = atan2( norm.y, -norm.x );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
		b->launcher->DeactivateBullet( b );
	}
	else
	{
		V2d en = edge->Normal();
		if( pos == edge->v0 )
		{
			en = normalize( b->position - pos );
		}
		else if( pos == edge->v1 )
		{
			en = normalize( b->position - pos );
		}
		double d = dot( b->velocity, en );
		V2d ref = b->velocity - (2.0 * d * en);
		b->velocity = ref;
		cout << "ref: " << ref.x << ", " << ref.y << endl;
		//b->velocity = -b->velocity;
		b->bounceCount++;
		b->framesToLive = b->launcher->maxFramesToLive;
	}
}

void Owl::BulletHitPlayer(BasicBullet *b )
{
	//owner->player->ApplyHit( b->launcher->hitboxInfo );
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	owner->player->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
}

void Owl::ResetEnemy()
{
	velocity = V2d( 0, 0 );
	//action = NEUTRAL;
	//testSeq.Reset();
	launcher->Reset();
	//cout << "resetting enemy" << endl;
	//spawned = false;
	//targetNode = 1;
	//forward = true;
	dead = false;
	dying = false;
	deathFrame = 0;
	frame = 0;
	position.x = originalPos.x;
	position.y = originalPos.y;
	receivedHit = NULL;
	

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Owl::DirectKill()
{
	BasicBullet *b = launcher->activeBullets;
	while( b != NULL )
	{
		BasicBullet *next = b->next;
		double angle = atan2( b->velocity.y, -b->velocity.x );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
		b->launcher->DeactivateBullet( b );

		b = next;
	}

	dying = true;
	health = 0;
	receivedHit = NULL;
}

void Owl::ActionEnded()
{
	double dist = 800;
	Actor *player = owner->player;
	double len = length( player->position - position );
	if( frame == actionLength[action] )
	{
		switch( action )
		{
		case REST:
			break;
		case SPIN:
			{
				action = FIRE;

				fireDir = normalize( owner->player->position - position );
				ang = atan2( -fireDir.y, fireDir.x );
			}
			break;
		case GUARD:
			action = REST;
			break;
		case FIRE:
			if( length( player->position - position ) >= dist )
			{
				action = REST;
				frame = 0;
			}

			action = SPIN;
			//action = FIRE;
			break;
		}
		//if( action == FIRE )
		//{
		//	if( len > shotRadius )
		//	{
		//		action = REST;
		//	}
		//	else if( len > chaseRadius )
		//	{
		//		action = CHASE;
		//		velocity = normalize( player->position - position ) * 2.5;
		//	}
		//	else if( len < retreatRadius )
		//	{
		//		action = RETREAT;
		//		velocity = normalize( player->position - position ) * -2.5;
		//	}
		//	else
		//	{
		//		action = NEUTRAL;
		//	}	
		//}
		//else if( action == REST )
		//{
		//	if( len > shotRadius )
		//	{
		//		//stay the same
		//	}
		//	else if( len > chaseRadius )
		//	{
		//		action = CHASE;
		//		velocity = normalize( player->position - position ) * 2.5;
		//	}
		//	else if( len < retreatRadius )
		//	{
		//		action = RETREAT;
		//		velocity = normalize( player->position - position ) * -2.5;
		//	}
		//	else
		//	{
		//		action = NEUTRAL;
		//		velocity = V2d( 0, 0 );
		//	}	
		//}
		//else
		//{
		//	action = FIRE;
		//	velocity = V2d( 0, 0 );
		//}
		frame = 0;
	}
}


void Owl::UpdatePrePhysics()
{
	ActionEnded();

	Actor *player = owner->player;
	double dist = 800;
	bool lessThanSize = length( player->position - position ) < dist;
	
	switch( action )
	{
	case REST:
		{
			if( lessThanSize )
			{
				action = FIRE;
				frame = 0;
			}
		}
		break;
	case GUARD:
		{
			//action = REST;
		}
		break;
	case SPIN:
		{
			if( !lessThanSize )
			{
				action = REST;
				frame = 0;
			}
		}
		break;
	case FIRE:
		{
			
		}
		break;
	}


	//switch( action )
	//{
	//case NEUTRAL:
	//	cout << "neutral: " << frame << endl;
	//	break;
	//case FIRE:
	//	cout << "fire: " << frame << endl;
	//	break;
	//case RETREAT:
	//	cout << "retreat: " << frame << endl;
	//	
	//	break;
	//case CHASE:
	//	cout << "chase" << endl;
	//	break;
	//case REST:
	//	cout << "rest" << endl;
	//	break;
	//default:
	//	cout << "what" << endl;
	//}


	/*if( action == RETREAT )
	{
		velocity = normalize( player->position - position ) * -2.5;
	}
	else if( action == NEUTRAL )
	{
		velocity = normalize( player->position - position ) * 2.5;
	}*/

	launcher->UpdatePrePhysics();

	if( !dead && !dying && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dying = true;

			owner->player->ConfirmEnemyKill( this );
			//cout << "dying" << endl;
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}

		receivedHit = NULL;
	}

	if( !dying && !dead && action == FIRE && frame == 3 * 6 - 1  )// frame == 0 && slowCounter == 1 )
	{
		launcher->position = position;
		launcher->facingDir = fireDir;//normalize( owner->player->position - position );
		launcher->Fire();
	}
}

void Owl::UpdatePhysics()
{	
	specterProtected = false;
	if( !dead && !dying )
	{
		//position += velocity / NUM_STEPS / (double)slowMultiple;

		PhysicsResponse();
	}

	launcher->UpdatePhysics();

	if( PlayerSlowingMe() )
	{
		if( slowMultiple == 1 )
		{
			slowCounter = 1;
			slowMultiple = 5;
		}
	}
	else
	{
		slowMultiple = 1;
		slowCounter = 1;
	}

	return;
}

void Owl::PhysicsResponse()
{
	if( !dead && !dying && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( COLOR_YELLOW, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
			owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 6, 3, facingRight );
			
		//	cout << "Owl received damage of: " << receivedHit->damage << endl;
			/*if( !result.second )
			{
				owner->Pause( 8 );
			}
		
			health -= 20;

			if( health <= 0 )
				dead = true;

			receivedHit = NULL;*/
			//dead = true;
			//receivedHit = NULL;
		}

		if( IHitPlayer() )
		{
		//	cout << "Owl just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Owl::UpdatePostPhysics()
{
	launcher->UpdatePostPhysics();
	if( receivedHit != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	if( deathFrame == 0 && dying )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}

	if( deathFrame == 60 && dying )
	{
		//cout << "switching dead" << endl;
		dying = false;
		dead = true;
		//cout << "REMOVING" << endl;
		//testLauncher->Reset();
		//owner->RemoveEnemy( this );
		//return;
	}

	if( dead && launcher->GetActiveCount() == 0 )
	{
		//cout << "REMOVING" << endl;
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
	launcher->UpdateSprites();

	if( slowCounter == slowMultiple )
	{
		//cout << "fireCounter: " << fireCounter << endl;
		++frame;
		slowCounter = 1;
	
		if( dying )
		{
			//cout << "deathFrame: " << deathFrame << endl;
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}
}

void Owl::UpdateSprite()
{
	if( !dying && !dead )
	{
		switch( action )
		{
		case REST:
			{
				sprite.setRotation( 0 );
				sprite.setTexture( *ts_flap->texture );
				sprite.setTextureRect( ts_flap->GetSubRect( (frame / 5) % 7 ) );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
			}
			break;
		case SPIN:
			{
				sprite.setRotation( 0 );
				sprite.setTexture( *ts_spin->texture );
				sprite.setTextureRect( ts_spin->GetSubRect( (frame / 5) % 8 ) );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
			}
			break;
		case GUARD:
			{
				sprite.setRotation( 0 );
				sprite.setTexture( *ts_spin->texture );
				sprite.setTextureRect( ts_spin->GetSubRect( 0 ) );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
			}
			break;
		case FIRE:
			{
				sprite.setRotation( ang / PI * 180.f );
				sprite.setTexture( *ts_throw->texture );
				sprite.setTextureRect( ts_throw->GetSubRect( frame / 6 ) );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
			}
			break;
		}
		//sprite.setTextureRect( ts->GetSubRect( 0 ) );
		sprite.setPosition( position.x, position.y );

		if( hasMonitor && !suppressMonitor )
		{
			//keySprite.setTexture( *ts_key->texture );
			keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
			keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
				keySprite->getLocalBounds().height / 2 );
			keySprite->setPosition( position.x, position.y );

		}
	}
	if( dying )
	{

		botDeathSprite.setTexture( *ts_death->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 0 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );

		topDeathSprite.setTexture( *ts_death->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 1 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
	}
}

void Owl::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead && !dying )
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
	else if( !dead )
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			/*bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );*/
		}
		
		target->draw( topDeathSprite );
	}



}

void Owl::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying )
	{
		if( hasMonitor && !suppressMonitor )
		{
			CircleShape cs;
			cs.setRadius( 50 );
			cs.setFillColor( Color::White );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		else
		{
			CircleShape cs;
			cs.setRadius( 40 );
			cs.setFillColor( Color::Red );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
	}
}

bool Owl::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Owl::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;

	if( owner->player->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->player->groundSpeed * ( owner->player->ground->v1 - owner->player->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->player->velocity );
	}
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Owl::PlayerHitMe()
{
	Actor *player = owner->player;
	if( player->currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			sf::Rect<double> qRect( position.x - hurtBody.rw,
			position.y - hurtBody.rw, hurtBody.rw * 2, 
			hurtBody.rw * 2 );
			owner->specterTree->Query( this, qRect );

			if( !specterProtected )
			{
				receivedHit = player->currHitboxInfo;
				return pair<bool, bool>(true,false);
			}
			else
			{
				return pair<bool, bool>(false,false);
			}
			
		}
		
	}

	for( int i = 0; i < player->recordedGhosts; ++i )
	{
		if( player->ghostFrame < player->ghosts[i]->totalRecorded )
		{
			if( player->ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player->currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player->ghosts[i]->curhi
		}
	}

	return pair<bool, bool>(false,false);
}

bool Owl::PlayerSlowingMe()
{
	Actor *player = owner->player;
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

void Owl::DebugDraw( RenderTarget *target )
{
	if( !dying )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Owl::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Owl::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}