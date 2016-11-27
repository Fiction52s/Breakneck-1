#include "Boss.h"
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


Boss_Gator::Boss_Gator( GameSession *owner, Vector2i pos )
	:Enemy( owner, EnemyType::TURTLE, false, 4 ), deathFrame( 0 ), orbVA( sf::Quads, 4 * NUM_ORBS )
{
	orbRadius = 160;

	//loop = false; //no looping on Boss_Gator for now

	//bulletSpeed = 5;

	//action = PLAN;
	showFramesPerOrb = 30;

	actionLength[PLAN] = 10;
	actionLength[SWIM] = 10;
	actionLength[SHOW] = showFramesPerOrb * NUM_ORBS;
	actionLength[SWAP] = 10;
	actionLength[WAIT] = 10;
	actionLength[ATTACK] = 10;
	actionLength[RETURN] = 10;


	animFactor[PLAN] = 1;
	animFactor[SHOW] = 1;
	animFactor[SWAP] = 1;
	animFactor[SWIM] = 1;
	animFactor[WAIT] = 1;
	animFactor[ATTACK] = 1;
	animFactor[RETURN] = 1;
	

	swimVelBez = CubicBezier( 0, 0, 1, 1 );
	

	//fireCounter = 0;
	
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	basePos = Vector2f( position.x, position.y );

	originalPos = pos;

	deathFrame = 0;

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;

	//animationFactor = 5;

	//ts = owner->GetTileset( "Boss_Gator.png", 80, 80 );
	ts = owner->GetTileset( "bat_48x48.png", 48, 48 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
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
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	//hitboxInfo->kbDir;

	

	dead = false;

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	facingRight = true;
		

	UpdateHitboxes();

	ResetEnemy();

	swimDuration = 60;

	//cout << "finish init" << endl;
}

void Boss_Gator::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Boss_Gator::ResetEnemy()
{
	dead = false;
	deathFrame = 0;
	frame = 0;
	position.x = originalPos.x;
	position.y = originalPos.y;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	showFramesPerOrb = 30;

	numSwapsThisRound = 0;
	rotateCW = false;
	rotateCCW = false;
	rotationSpeed = 1;//0.0;
	swapCounter = 0;
	currSwimIndex = 0;
	swapDuration = 60;
	SetOrbsOriginalPos();
}

void Boss_Gator::SetOrbsOriginalPos()
{
	int startingRadius = 600;
	Transform t;
	Vector2f out( 0, -startingRadius );
	for( int i = 0; i < NUM_ORBS; ++i )
	{
		orbPosRel[i] = t.transformPoint( Vector2f( 0, -startingRadius ) );
		t.rotate( 360.f / 5.f );
	}
}

void Boss_Gator::ActionEnded()
{
	int len = actionLength[action] * animFactor[action];

	if( action == SWAP )
	{
		if( swapDuration > len )
		{
			swapWaitingCounter = 0;
			swapWaitDuration = swapDuration - len;
			action = SWAPWAIT;
			frame = 0;
		}
		else
		{
			if( swapCounter == numSwapsThisRound )
			{
				action = SWIM;
				currSwimIndex = 0;
				framesSwimming = 0;
				frame = 0;
			}
			else
			{
				action = SWAP;
				swapCounter++;
			}
		}
		return;
	}
	else if( action == SWAPWAIT )
	{
		if( swapWaitingCounter == swapWaitDuration )
		{
			if( swapCounter == numSwapsThisRound )
			{
				action = SWIM;
				frame = 0;
				currSwimIndex = 0;
				framesSwimming = 0;
			}
			else
			{
				action = SWAP;
				swapCounter++;
			}
		}
	}

	
	if( frame == len )
	{
		switch( action )
		{
		case PLAN:
			action = SHOW;
			frame = 0;
			break;
		case SHOW:
			if( numSwapsThisRound > 0 )
			{
				action = SWAP;
				frame = 0;
				swapCounter = 0;
			}
			else
			{
				currSwimIndex = 0;
				action = SWIM;
				framesSwimming = 0;
				frame = 0;
				Vector2f currOrb = orbPosRel[currSwimIndex]; 
				Vector2f nextOrb = orbPosRel[currSwimIndex+1]; 
				startSwimPoint.x = basePos.x + currOrb.x;
				startSwimPoint.y = basePos.y + currOrb.y;
				endSwimPoint.x = basePos.x + nextOrb.x;
				endSwimPoint.y = basePos.y + nextOrb.y;
			}
			break;
		case SWAP:
			//nothing
			break;
		case SWAPWAIT:
			frame = 0;
			break;
		case SWIM:
			frame = 0;
			break;
		case WAIT:
			if( currSwimIndex == NUM_ORBS - 1 )
			{
				action = RETURN;
				frame = 0;
				Vector2f currOrb = orbPosRel[currSwimIndex]; 
				startSwimPoint.x = basePos.x + currOrb.x;
				startSwimPoint.y = basePos.y + currOrb.y;
				endSwimPoint.x = basePos.x;
				endSwimPoint.y = basePos.y;
			}
			else
			{
				action = SWIM;
				framesSwimming = 0;
				frame = 0;
				Vector2f currOrb = orbPosRel[currSwimIndex]; 
				Vector2f nextOrb = orbPosRel[currSwimIndex+1]; 
				startSwimPoint.x = basePos.x + currOrb.x;
				startSwimPoint.y = basePos.y + currOrb.y;
				endSwimPoint.x = basePos.x + nextOrb.x;
				endSwimPoint.y = basePos.y + nextOrb.y;
				++currSwimIndex;
			}
			break;
		case ATTACK:
			break;
		case RETURN:
			break;
		case AFTERFIGHT:
			break;
		}
	}

	if( rotateCW )
	{
		RotateOrbs( -rotationSpeed );
	}
	else if( rotateCCW )
	{
		RotateOrbs( rotationSpeed );
	}
}

void Boss_Gator::RotateOrbs( float degrees )
{
	Transform t;
	t.rotate( degrees );
	for( int i = 0; i < NUM_ORBS; ++i )
	{
		orbPosRel[i] = t.transformPoint( orbPosRel[i] );
	}
}

void Boss_Gator::UpdateOrbSprites()
{
	for( int i = 0; i < NUM_ORBS; ++i )
	{
		orbVA[i*4+0].position = basePos + orbPosRel[i] + Vector2f( -orbRadius, -orbRadius );
		orbVA[i*4+1].position = basePos + orbPosRel[i] + Vector2f( orbRadius, -orbRadius );
		orbVA[i*4+2].position = basePos + orbPosRel[i] + Vector2f( orbRadius, orbRadius );
		orbVA[i*4+3].position = basePos + orbPosRel[i] + Vector2f( -orbRadius, orbRadius );

		orbVA[i*4+0].color = Color::Red;
		orbVA[i*4+1].color = Color::Red;
		orbVA[i*4+2].color = Color::Red;
		orbVA[i*4+3].color = Color::Red;
	}
}

void Boss_Gator::SetupTravelOrder()
{
	//int possibles[NUM_ORBS];
	for( int i = 0; i < NUM_ORBS; ++i )
	{
		orbTravelOrder[i] = i;
		
	}

	//swap
	for (int i = 0; i < NUM_ORBS; ++i)
	{
		int j = rand() % NUM_ORBS;
		int temp = orbTravelOrder[i];
		orbTravelOrder[i] = orbTravelOrder[j];
		orbTravelOrder[j] = temp;
	}
}


void Boss_Gator::UpdatePrePhysics()
{
	ActionEnded();

	switch( action )
	{
	case PLAN:
		break;
	case SHOW:
		break;
	case SWAP:
		break;
	case SWAPWAIT:
		break;
	case SWIM:
		break;
	case WAIT:
		break;
	case ATTACK:
		break;
	case RETURN:
		break;
	case AFTERFIGHT:
		break;
	}

	switch( action )
	{
	case PLAN:
		{
			if( frame == 0 )
			{
				SetupTravelOrder();
			}
			break;
		}	
	case SHOW:
		{
			if( frame % showFramesPerOrb == 0 )
			{
				int orb = frame / showFramesPerOrb;
				//light up my orb, unlight the previous orb
			}
			break;
		}
	case SWAP:
		break;
	case SWAPWAIT:
		swapWaitingCounter++;
		break;
	case SWIM:
		break;
	case WAIT:
		break;
	case ATTACK:
		break;
	case RETURN:
		break;
	case AFTERFIGHT:
		break;
	}

	//switch( action )
	//{
	//case PLAN:
	//	break;
	//case SHOW:
	//	break;
	//case SWAP:
	//	break;
	//case SWIM:
	//	break;
	//case WAIT:
	//	break;
	//case ATTACK:
	//	break;
	//case RETURN:
	//	break;
	//case AFTERFIGHT:
	//	break;
	//}

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

			action = AFTERFIGHT;
			frame = 0;
			//owner->activeSequence = crawlerAfterFightSeq;
		}

		receivedHit = NULL;
	}
}

void Boss_Gator::UpdatePhysics()
{	
	if( action == SWIM || action == RETURN )
	{
		double a = (double)framesSwimming / swimDuration;
		double f = swimVelBez.GetValue( a );
		position = startSwimPoint * ( 1.0 - f ) + endSwimPoint * ( f );
	}

	specterProtected = false;
	if( !dead )
	{
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
	}

	if( !dead )
	{
		PhysicsResponse();
	}
	return;
}

void Boss_Gator::PhysicsResponse()
{
	if( !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( 5, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}	
		}

		if( IHitPlayer() )
		{
		}
	}
}

void Boss_Gator::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	if( slowCounter == slowMultiple )
	{
		//cout << "fireCounter: " << fireCounter << endl;
		++frame;
		slowCounter = 1;
	
		//if( dead )
		//{
			//cout << "deathFrame: " << deathFrame << endl;
		//	deathFrame++;
		//}

	}
	else
	{
		slowCounter++;
	}

	

	if( dead )
	{
		//cout << "REMOVING" << endl;
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
	UpdateOrbSprites();
}

void Boss_Gator::UpdateSprite()
{
	if( !dead )
	{
		sprite.setTextureRect( ts->GetSubRect( 0 ) );
		sprite.setPosition( position.x, position.y );
	}
	/*if( dying )
	{

		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 0 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 1 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
	}*/
}

void Boss_Gator::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{
		target->draw( orbVA );
		target->draw( sprite );
	}
	else if( !dead )
	{
		target->draw( botDeathSprite );
		
		target->draw( topDeathSprite );
	}



}

void Boss_Gator::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead )
	{
		/*CircleShape enemyCircle;
		enemyCircle.setFillColor( COLOR_BLUE );
		enemyCircle.setRadius( 50 );
		enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
		enemyCircle.setPosition( position.x, position.y );
		target->draw( enemyCircle );*/

		/*if( hasMonitor && !suppressMonitor )
		{
			monitor->miniSprite.setPosition( position.x, position.y );
			target->draw( monitor->miniSprite );
		}*/
	}
}

bool Boss_Gator::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Boss_Gator::UpdateHitboxes()
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
pair<bool,bool> Boss_Gator::PlayerHitMe()
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

bool Boss_Gator::PlayerSlowingMe()
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

void Boss_Gator::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Boss_Gator::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Boss_Gator::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}