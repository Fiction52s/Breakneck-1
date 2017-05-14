#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Movement.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )


Narwhal::Narwhal( GameSession *owner, bool p_hasMonitor, Vector2i &startPos, 
	sf::Vector2i &endPos, int p_moveFrames )
	:Enemy( owner, EnemyType::PATROLLER, p_hasMonitor, 1 ), deathFrame( 0 ),
	pathVA( sf::Quads, 4 )
{
	
	moveFrames = p_moveFrames;
	moveFrames = 5;
	receivedHit = NULL;
	position.x = startPos.x;
	position.y = startPos.y;

	//point0 = position;
	//point1.x = endPos.x;
	//point1.y = endPos.y;

	V2d point0 = position;
	V2d point1( endPos.x, endPos.y );
	origStartPoint = point0;
	origEndPoint = point1;

	moveDistance = length( point1 - point0 );
	moveDir = normalize( point1 - point0 );

	initHealth = 40;
	health = initHealth;

	double left = min( point0.x, point1.x );
	double top = min( point0.y, point1.y );
	double right = max( point0.x, point1.x );
	double bot = max( point0.y, point1.y );

	double width = right - left;
	double height = bot - top;


	spawnRect = sf::Rect<double>( left, top, width, height );
	
	actionLength[WAITING] = 10;
	actionLength[CHARGE_START] = 10;
	actionLength[CHARGE_REPEAT] = 10;
	actionLength[TURNING] = 4;

	animFactor[WAITING] = 1;
	animFactor[CHARGE_START] = 4;
	animFactor[CHARGE_REPEAT] = 1;
	animFactor[TURNING] = 5;

	//speed = 2;
	frame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Narwhal.png", 80, 80 );
	ts = owner->GetTileset( "narwhal_256x256.png", 256, 256 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( position.x, position.y );
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
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	//hitboxInfo->kbDir;


	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	triggerBox.type = CollisionBox::Hit;
	triggerBox.isCircle = false;
	SetupWaiting();

	seq.AddLineMovement( point0, point1, CubicBezier( 0, 0, 1, 1 ), moveFrames );
	
	//seq1.AddLineMovement( point1, point0, CubicBezier( 0, 0, 1, 1 ), moveFrames );
	ResetEnemy();
	
	UpdatePath();

}

void Narwhal::SetupWaiting()
{
	//V2d diff = point1 - point0;
	V2d normDiff = moveDir;//normalize( diff );

	V2d midPoint = position + moveDir * moveDistance / 2.0;

	triggerBox.globalAngle = atan2( -normDiff.x, normDiff.y );
	triggerBox.globalPosition = midPoint;
	triggerBox.rw = 20;
	triggerBox.rh = moveDistance / 2;//length( point0-point1 ) / 2;
	
	angle = atan2( normDiff.y, normDiff.x ) / PI * 180.f;
	sprite.setRotation( angle );
	UpdateHitboxes();
}

void Narwhal::UpdatePath()
{
	V2d startPoint = position;
	V2d endPoint = position + moveDir * moveDistance;

	V2d along = normalize( endPoint - startPoint );
	V2d other( along.y, -along.x );

	double halfWidth = 10;

	V2d topLeft = startPoint + other * halfWidth;
	V2d topRight = endPoint + other * halfWidth;
	V2d botRight = endPoint - other * halfWidth;
	V2d botLeft = startPoint - other * halfWidth;

	pathVA[0].position = Vector2f( topLeft.x, topLeft.y );
	pathVA[1].position = Vector2f( topRight.x, topRight.y );
	pathVA[2].position = Vector2f( botRight.x, botRight.y );
	pathVA[3].position = Vector2f( botLeft.x, botLeft.y );

	sprite.setRotation( atan2( moveDir.y, moveDir.x ) / PI * 180.f );

	if( action != CHARGE_START )
	{
		pathVA[0].color = Color::White;
		pathVA[1].color = Color::White;
		pathVA[2].color = Color::White;
		pathVA[3].color = Color::White;
	}
	else
	{
		pathVA[0].color = Color::Magenta;
		pathVA[1].color = Color::Magenta;
		pathVA[2].color = Color::Magenta;
		pathVA[3].color = Color::Magenta;
	}
}

void Narwhal::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case WAITING:
			frame = 0;
			break;
		case CHARGE_START:
			action = CHARGE_REPEAT;
			frame = 0;
			seq.Reset();
			seq.movementList->start = position;
			seq.movementList->end = position + moveDir * moveDistance;
			break;
		case CHARGE_REPEAT:
			frame = 0;
			break;
		case TURNING:
			facingRight = !facingRight;
			action = WAITING;
			frame = 0;

			break;
		}
	}
}

void Narwhal::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Narwhal::ResetEnemy()
{
	if( origEndPoint.x < origStartPoint.x )
		facingRight = false;
	else
	{
		facingRight = true;
	}
	

	seq.Reset();
	//seq1.Reset();
	start0 = false;
	triggered = false;
	dead = false;
	deathFrame = 0;
	position = origStartPoint;
	receivedHit = NULL;
	action = WAITING;
	frame = 0;
	slowMultiple = 1;
	slowCounter = 1;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Narwhal::UpdatePrePhysics()
{
	ActionEnded();
	switch( action )
	{
	case WAITING:
		{
			
		}
		break;
	case CHARGE_START:
		break;
	case CHARGE_REPEAT:
		break;
	case TURNING:
		break;
	}

	if( !dead && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		

		//cout << "health now: " << health << endl;
		//owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->GetPlayer( 0 )->position + position ) / 2.0, true, 0, 10, 2, true );
		//owner->Pause( 5 );
		health -= 20;
		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			//AttemptSpawnMonitor();
			dead = true;
			owner->GetPlayer( 0 )->ConfirmEnemyKill( this );
		}
		else
		{
			owner->GetPlayer( 0 )->ConfirmEnemyNoKill( this );
		}
		receivedHit = NULL;
		
		
	}

	triggered = false;
}

void Narwhal::UpdatePhysics()
{
	//cout << "setting to targetnode: " << targetNode << endl;
	//position = V2d( path[targetNode].x, path[targetNode].y );
	specterProtected = false;

	if( !dead )
	{
		//if( 
		if( action == CHARGE_REPEAT )
		{
			if( seq.currMovement == NULL )
			{
				//cout << "turning 0" << endl;
				action = TURNING;
				
				frame = 0;
				V2d oldStart = seq.movementList->start;
				V2d oldEnd = seq.movementList->end;
				moveDir = normalize( oldStart - oldEnd ); 
				UpdatePath();
				SetupWaiting();
			}
			else
			{
				seq.Update( slowMultiple );
				position = seq.position;
			}
		}
		
		//cout << "position: " << position.x << ", " << position.y << 
		//	", newpos: " << testSeq.position.x 
		//	<< ", " << testSeq.position.y << endl;
		
		//PhysicsResponse();
	}
	
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

void Narwhal::PhysicsResponse()
{
	if( !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->GetPlayer( 0 )->ConfirmHit( 6, 5, .8, 6 );


			if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
			{
				owner->GetPlayer( 0 )->velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->GetPlayer( 0 )->frame << endl;

			//owner->GetPlayer( 0 )->frame--;
			//owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 6, 3, true );
			

		//	cout << "Narwhal received damage of: " << receivedHit->damage << endl;
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
		//	cout << "Narwhal just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		if( action == WAITING && !triggered )
		{
			if( triggerBox.Intersects( owner->GetPlayer( 0 )->hurtBody ) )
			{
				triggered = true;
			}
		}
	}
}

void Narwhal::UpdatePostPhysics()
{
	if( triggered )
	{
		action = CHARGE_START;
		frame = 0;
		currMoveFrame = 0;

		
		//start0 = !start0;

		//seq.Reset();

		if( start0 )
		{
			
		}
		else
		{
		//	seq1.Reset();
		}

	}

	if( !dead && receivedHit != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->GetPlayer( 0 )->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	/*if( receivedHit != NULL && !dead )
	{
		
		
	}*/

	if( deathFrame == 0 && dead )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}

	if( dead && deathFrame == 60 )
	{
		//owner->ActivateEffect( ts_testBlood, position, true, 0, 15, 2, true );
		owner->RemoveEnemy( this );
		//return;
	}

	if( action == CHARGE_START )
	{
		V2d playerDir = normalize( owner->GetPlayer( 0 )->position - position );
		moveDir = playerDir;
		UpdatePath();
	}

	UpdateSprite();

	
	


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
}

void Narwhal::UpdateSprite()
{
	//ts->GetSubRect( frame / animationFactor ) );
	
	IntRect ir;// = ts->GetSubRect( 0 );
	switch( action )
	{
		case WAITING:
			ir = ts->GetSubRect( 0 );
			break;
		case CHARGE_START:
			ir = ts->GetSubRect( 1 );
			break;
		case CHARGE_REPEAT:
			ir = ts->GetSubRect( 2 );
			break;
		case TURNING:
			ir = ts->GetSubRect( frame / animFactor[TURNING] + 4 );
			break;
	}

	
	if( facingRight )
	{
		sprite.setTextureRect( ir );
	}
	else
	{
		ir.left += ir.width;
		ir.width = -ir.width;
		//ir.top += ir.height;
		//ir.height = -ir.height;
		sprite.setTextureRect( ir );
		//sprite.setRotation( -angle );
	}
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( position.x, position.y );


	if( dead )
	{
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 9 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 8 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
	}
	else
	{
		if( hasMonitor && !suppressMonitor )
		{
			//keySprite.setTexture( *ts_key->texture );
			keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
			keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
				keySprite->getLocalBounds().height / 2 );
			keySprite->setPosition( position.x, position.y );

		}
	}
}

void Narwhal::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{
		target->draw( pathVA );
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
	else
	{
		target->draw( botDeathSprite );

		
		target->draw( topDeathSprite );
	}



}

void Narwhal::DrawMinimap( sf::RenderTarget *target )
{
	/*CircleShape enemyCircle;
	enemyCircle.setFillColor( COLOR_BLUE );
	enemyCircle.setRadius( 50 );
	enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
	enemyCircle.setPosition( position.x, position.y );
	target->draw( enemyCircle );

	if( hasMonitor && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/

	if( !dead )
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

bool Narwhal::IHitPlayer( int index )
{
	Actor *player = owner->GetPlayer( 0 );
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Narwhal::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;

	if( owner->GetPlayer( 0 )->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->groundSpeed * ( owner->GetPlayer( 0 )->ground->v1 - owner->GetPlayer( 0 )->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->velocity );
	}
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Narwhal::PlayerHitMe( int index )
{
	Actor *player = owner->GetPlayer( 0 );
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
			receivedHit = player->currHitboxInfo;
			return pair<bool, bool>(true,false);
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

bool Narwhal::PlayerSlowingMe()
{
	Actor *player = owner->GetPlayer( 0 );
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

void Narwhal::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
		triggerBox.DebugDraw( target );
	}
}

void Narwhal::SaveEnemyState()
{
	//stored.dead = dead;
	///stored.deathFrame = deathFrame;
	//stored.forward = forward;
//	stored.frame = frame;
	//stored.hitlagFrames = hitlagFrames;
	///stored.hitstunFrames = hitstunFrames;
	//stored.position = position;
//	stored.targetNode = targetNode;
}

void Narwhal::LoadEnemyState()
{
//	dead = stored.dead;
//	deathFrame = stored.deathFrame;
//	forward = stored.forward;
	//frame = stored.frame;
	//hitlagFrames = stored.hitlagFrames;
	//hitstunFrames = stored.hitstunFrames;
//	position = stored.position;
///	targetNode = stored.targetNode;
}