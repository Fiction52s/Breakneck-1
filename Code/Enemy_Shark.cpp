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
#define COLOR_RED Color( 0xff, 0x22, 0 )

Shark::Shark( GameSession *owner, bool p_hasMonitor, Vector2i pos, float pspeed )
	:Enemy( owner, EnemyType::SHARK, p_hasMonitor, 5 ), deathFrame( 0 ), approachAccelBez( 1,.01,.86,.32 ) 
{

	//attackCounter = 0;
	actionLength[WAKEUP] = 30;
	actionLength[APPROACH] = 2;
	actionLength[CIRCLE] = 2;
	actionLength[RUSH] = 7;
	actionLength[FINALCIRCLE] = 2;

	animFactor[WAKEUP] = 1;
	animFactor[FINALCIRCLE] = 1;
	animFactor[APPROACH] = 1;
	animFactor[CIRCLE] = 1;
	animFactor[RUSH] = 5;
	latchedOn = false;
	//offsetPlayer 
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	origPosition = position;

	//approachFrames = 180 * 3;
	//totalFrame = 0;

	//latchedOn = true; 

	circleFrames = 120;
	V2d dirFromPlayer = normalize( owner->player->position - position );
	double fromPlayerAngle =  atan2( dirFromPlayer.y, dirFromPlayer.x ) + PI;
	//cout << "dirfrom: " << dirFromPlayer.x << ", " << dirFromPlayer.y << endl;
	//cout << "from player angle: " << fromPlayerAngle << endl;
	circleSeq.AddRadialMovement( 1, 0, 2 * PI, 
		true, V2d( 1, 1 ), 0, CubicBezier( .44,.79,.77,.1), circleFrames );
	
	circleSeq.InitMovementDebug();


	rushSeq.AddLineMovement( V2d( 0, 0 ), 
		V2d( 1, 0 ), CubicBezier( 0, 0, 1, 1 ), actionLength[RUSH] * animFactor[RUSH] );


	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );

	basePos = position;
	
	speed = pspeed;

	//speed = 2;
	action = WAKEUP;
	frame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Shark.png", 80, 80 );
	//ts = owner->GetTileset( "bat_48x48.png", 48, 48 );
	ts_circle = owner->GetTileset( "shark_circle_256x256.png", 256, 256 );
	ts_bite = owner->GetTileset( "shark_bite_256x256.png", 256, 256 );
	ts_death = owner->GetTileset( "shark_death_256x256.png", 256, 256 );
	sprite.setTexture( *ts_circle->texture );
	sprite.setTextureRect( ts_circle->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	//position.x = 0;
	//position.y = 0;
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

	
	latchStartAngle = 0;
	dead = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	facingRight = true;
	 
	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	wakeCounter = 0;
	wakeCap = 20;

	botDeathSprite.setTexture( *ts_death->texture );
	botDeathSprite.setTextureRect( ts_death->GetSubRect( 1 ) );
	botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
	topDeathSprite.setTexture( *ts_death->texture );
	topDeathSprite.setTextureRect( ts_death->GetSubRect( 0 ) );
	topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
}

void Shark::HandleEntrant( QuadTreeEntrant *qte )
{

}

void Shark::ResetEnemy()
{
	//attackCounter = 0;
	wakeCounter = 0;
	action = WAKEUP;
	latchStartAngle = 0;
	latchedOn = false;
//	totalFrame = 0;
	circleSeq.Reset();
	//circleSeq.Update();
	dead = false;
	deathFrame = 0;
	frame = 0;
	basePos = origPosition;
	position = basePos;

	receivedHit = NULL;
	

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Shark::UpdatePrePhysics()
{
	if( !dead && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dead = true;
		}

		receivedHit = NULL;
	}



	if( !dead )
	{

		double cs = cos( latchStartAngle );
		double sn = sin( latchStartAngle );

		V2d truePosOffset( circleSeq.position.x * cs - 
			circleSeq.position.y * sn, 
			circleSeq.position.x * sn + circleSeq.position.y * cs );
		truePosOffset *= length( offsetPlayer );

		if( latchedOn )
		{
			basePos = owner->player->position;// + offsetPlayer;
		}
		
		if( action == WAKEUP )
		{
			if( length( basePos - owner->player->position ) < 400 )
			{
				wakeCounter++;
				if( wakeCounter == wakeCap )
				{
					action = CIRCLE;
					frame = 0;
					latchedOn = true;
					offsetPlayer = basePos - owner->player->position;//owner->player->position - basePos;
					origOffset = offsetPlayer;//length( offsetPlayer );
					V2d offsetDir = normalize( offsetPlayer );
					latchStartAngle = atan2( offsetDir.y, offsetDir.x );
					//cout << "latchStart: " << latchStartAngle << endl;
					circleSeq.Update();
					basePos = owner->player->position;
				}
			}
			else
			{
				wakeCounter--;
				if( wakeCounter < 0 )
					wakeCounter = 0;
			}	
		}
		else if( action == CIRCLE )
		{
			
			if( owner->player->hitstunFrames > 0  )
			{
				cout << "final circle" << endl;
				//got hit!
				action = FINALCIRCLE;

			}
			frame = 0;
		}
		else if( action == FINALCIRCLE && frame == circleFrames )
		{
			cout << "RUSHING: " << frame << endl;
			//offsetPlayer = 
			action = RUSH;
			frame = 0;
			rushSeq.Reset();
			rushSeq.currMovement->start = truePosOffset;
			rushSeq.currMovement->end = -truePosOffset;//V2d( 0, 0 );//position + 2.0 * ( basePos - position );
			rushSeq.Update();

			//cout << "true pos offset: " << truePosOffset.x << ", " << truePosOffset.y << endl;

			//cout << "up: " << rushSeq.position.x << ", " << rushSeq.position.y << endl;
			//rushSeq.currMovement-=
		}
		else if( action == RUSH )
		{
			if( rushSeq.currMovement == NULL )
			{
				cout << "back to normal: " << offsetPlayer.x << ", " << offsetPlayer.y << endl;
				action = CIRCLE;
				truePosOffset = -truePosOffset;
				//offsetPlayer = -offsetPlayer;
				offsetPlayer = truePosOffset;
				V2d offsetDir = normalize( truePosOffset );
				latchStartAngle = atan2( offsetDir.y, offsetDir.x );
				circleSeq.Reset();
				circleSeq.Update();
					//basePos = owner->player->position;
			}
		}
		
	}
	
}

void Shark::UpdatePhysics()
{
	specterProtected = false;

	//V2d offsetDir = normalize( offsetPlayer );
	//double newAngle = atan2( offsetDir.y, offsetDir.x ) + PI;
	//", new: " <<
		//newAngle << endl;
	

	double cs = cos( latchStartAngle );
	double sn = sin( latchStartAngle );

	V2d truePosOffset( circleSeq.position.x * cs - 
		circleSeq.position.y * sn, 
		circleSeq.position.x * sn + circleSeq.position.y * cs );
	//position = basePos + truePosOffset * length( offsetPlayer );// * 2.0;
	if( (action == CIRCLE || action == FINALCIRCLE) && latchedOn )
	{
		
		//cout << "circleSeq: " << circleSeq.position.x << ", " 
		//	<< circleSeq.position.y << endl;// ",  new: " <<
			//truePosOffset.x << ", " << truePosOffset.y << endl;
		position = basePos + truePosOffset * length( offsetPlayer );
	/*	theta = deg2rad(angle);

		cs = cos(theta);
		sn = sin(theta);

		x = x * cs - y * sn;
		y = x * sn + y * cs;*/

		circleSeq.Update();
		if( circleSeq.currMovement == NULL )
		{
			circleSeq.Reset();
			circleSeq.Update();
			//cout << "resetting sequence" << endl;
		}
		//offsetPlayer =  origOffset - origOffset * approachAccelBez.GetValue( ( (double)totalFrame / approachFrames) );
	}
	else if( action == RUSH )
	{
		//double acs = cos( attackAngle );
		//double asn = sin( attackAngle );
		//2.0 * offsetPlayer;
		position = basePos + rushSeq.position;// - truePosOffset * length( offsetPlayer );
		//cout << "rushSeq: " << rushSeq.position.x << ", " << rushSeq.position.y << endl;
		rushSeq.Update();

		
		//rushSeq.currMovement->start = 
		//position = 
	}

	//return;

	//double movement = speed / NUM_STEPS;
	
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

	if( dead )
		return;

	PhysicsResponse();
}

void Shark::PhysicsResponse()
{
	if( !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( COLOR_RED, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
//			owner->ActivateEffect( EffectLayer::IN_FRONT, ts_testBlood, position, true, 0, 6, 3, facingRight );
			
		//	cout << "Shark received damage of: " << receivedHit->damage << endl;
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
			//cout << "Shark hit player Shark pos: " <<
			//	position.x << ", " << position.y << ", playerpos: "
			//	<< owner->player->position.x << ", " << owner->player->position.y << endl;
		//	cout << "Shark just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Shark::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	

	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
	
		if( dead )
		{
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	if( deathFrame == 60 )
	{
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
}

void Shark::UpdateSprite()
{
	if( !dead )
	{
		switch( action )
		{
		case WAKEUP:
			{
			sprite.setTexture( *ts_circle->texture );
			IntRect ir = ts_circle->GetSubRect( 0 );
			//if( 
			sprite.setTextureRect( ir );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, 
					sprite.getLocalBounds().height / 2 );
			sprite.setRotation( 0 );
			
				//testColor = Color::White;
			}
			break;
		case APPROACH:
			{
			sprite.setTexture( *ts_circle->texture );
			IntRect ir = ts_circle->GetSubRect( 0 );
			//if( 
			sprite.setTextureRect( ir );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, 
					sprite.getLocalBounds().height / 2 );
			sprite.setRotation( 0 );
			//testColor = Color::Green;
			}
			break;
		case FINALCIRCLE:
			//sprite.setColor( Color::Blue );
			//break;
		case CIRCLE:
			{
			//sprite.setColor( Color::Blue );
			sprite.setTexture( *ts_circle->texture );
			int trueFrame = 0;
			
			
			

			double div = 2 * PI / 12.0;

			
			double cs = cos( latchStartAngle );
			double sn = sin( latchStartAngle );
			V2d truePosOffset( circleSeq.position.x * cs - 
				circleSeq.position.y * sn, 
				circleSeq.position.x * sn + circleSeq.position.y * cs );
			V2d normOffset = normalize( truePosOffset );
			double angle = atan2( normOffset.x, -normOffset.y );
			//angle -= PI / 4;
			if( angle < 0 )
				angle += PI * 2;
			
			int mults = angle / div;
			//cout << "mults: " << mults << ", angle: " << angle << endl;
			
			IntRect ir = ts_circle->GetSubRect( mults );
			sprite.setTextureRect( ir );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, 
					sprite.getLocalBounds().height / 2 );
			sprite.setRotation( 0 );
			}
			break;
		case RUSH:
			{
				sprite.setTexture( *ts_bite->texture );
				sprite.setTextureRect( ts_bite->GetSubRect( frame / animFactor[RUSH] ) );
				
				V2d normOffset = normalize( offsetPlayer );
				double angle = atan2( normOffset.y, normOffset.x );
				if( angle < 0 )
					angle += PI * 2;
				sprite.setOrigin( sprite.getLocalBounds().width / 2, 
					sprite.getLocalBounds().height / 2 );
				sprite.setRotation( angle / PI * 180.0 );


			}
			//testColor = Color::Red;
			break;
		}

		sprite.setPosition( position.x, position.y );
		

		//sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
		//sprite.setPosition( position.x, position.y );
	}
	else
	{
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
	}
}

void Shark::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{
		if( hasMonitor && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			/*CircleShape cs;
			cs.setRadius( 40 );
			cs.setFillColor( COLOR_BLUE );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );*/
		}
		/*CircleShape cs;
		cs.setFillColor( testColor );
		cs.setRadius( 40 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( position.x, position.y );
		target->draw( cs );*/
		target->draw( sprite );
	}
	else
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			
		}
		
		target->draw( topDeathSprite );
	}



}

void Shark::DrawMinimap( sf::RenderTarget *target )
{
	CircleShape enemyCircle;
	enemyCircle.setFillColor( COLOR_BLUE );
	enemyCircle.setRadius( 50 );
	enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
	enemyCircle.setPosition( position.x, position.y );
	target->draw( enemyCircle );

	/*if( hasMonitor && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/
}

bool Shark::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Shark::UpdateHitboxes()
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
pair<bool,bool> Shark::PlayerHitMe()
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
				
				for( list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); 
					it != player->ghosts[i]->currHitboxes->end(); ++it )
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
			//player->Sharks[i]->curhi
		}
	}

	return pair<bool, bool>(false,false);
}

bool Shark::PlayerSlowingMe()
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

void Shark::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		if( circleSeq.currMovement != NULL )
		{
			if( circleSeq.currMovement->vertices != NULL )
			{
				circleSeq.currMovement->DebugDraw( target );
			}
		}
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Shark::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Shark::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}