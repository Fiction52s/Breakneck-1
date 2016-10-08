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

Copycat::PlayerAttack::PlayerAttack()
			:nextAttack( NULL )
{
	a = UAIR;
	facingRight = true;
	reversed = false;
	speedLevel = 0;
	position = V2d( 0, 0 );
	delayFrames = 0;
}

Copycat::PlayerAttack *Copycat::GetAttack()
{
	if( inactiveAttacks == NULL )
	{
		cout << "out of possible attacks copycat" << endl;
		return NULL;
	}
	else
	{
		PlayerAttack *p = inactiveAttacks;
		PlayerAttack *next = inactiveAttacks->nextAttack;
		p->nextAttack = NULL;
		p->prevAttack = NULL;
		inactiveAttacks = next;
		return p;
	}
}

Copycat::PlayerAttack * Copycat::PopAttack()
{
	if( activeAttacksBack == NULL )
	{
		cout << "popping but im out of active attacks" << endl;
		return NULL;
	}
	else if( activeAttacksBack->prevAttack == NULL )
	{
		PlayerAttack *p = activeAttacksBack;
		p->prevAttack = NULL;

		activeAttacksFront = NULL;
		activeAttacksBack = NULL;

		return p;
	}
	else
	{
		PlayerAttack *p = activeAttacksBack;
		p->prevAttack->nextAttack = NULL;
		activeAttacksBack = p->prevAttack;
		p->prevAttack = NULL;

		return p;
	}
}

void Copycat::QueueAttack( Action a,
		bool facingRight,
		bool reversed, int speedLevel,
		sf::Vector2<double> &pos,
		int delayFrames )
{
	PlayerAttack *p = GetAttack();
	if( p == NULL )
	{
		assert( 0 );
	}
	else
	{
		p->a = a;
		p->facingRight = facingRight;
		p->reversed = reversed;
		p->speedLevel = speedLevel;
		p->delayFrames = delayFrames;

		if( activeAttacksFront == NULL )
		{
			activeAttacksFront = p;
			activeAttacksBack = p;
		}
		else
		{
			p->nextAttack = activeAttacksFront;
			activeAttacksFront->prevAttack = p;
			activeAttacksFront = p;
		}
	}
}

void Copycat::ResetAttacks()
{
	PlayerAttack *next = NULL;
	PlayerAttack *prev = NULL;
	
	activeAttacksFront = NULL;
	activeAttacksBack = NULL;
	inactiveAttacks = NULL;
	for( int i = 0; i < attackBufferSize; ++i )
	{
		PlayerAttack &curr = *allAttacks[i];
		curr.prevAttack = NULL;
		curr.nextAttack = NULL;
		if( i == 0 )
		{
			curr.prevAttack = NULL;
		}
		else
		{
			curr.prevAttack = allAttacks[i-1];
		}
		
		if( i == attackBufferSize - 1 )
		{
			curr.nextAttack = NULL;
		}
		else
		{
			curr.nextAttack = allAttacks[i+1];
		}
	}

	inactiveAttacks = allAttacks[0];
}

Copycat::Copycat( GameSession *owner, bool p_hasMonitor, Vector2i &pos )
	:Enemy( owner, EnemyType::COPYCAT, p_hasMonitor, 4 ), deathFrame( 0 )
{
	attackBufferSize = 16;
	allAttacks = new PlayerAttack*[attackBufferSize];
	PlayerAttack *prev = NULL;
	for( int i = 0; i < attackBufferSize; ++i )
	{
		PlayerAttack *p = new PlayerAttack();
		allAttacks[i] = p;
	}

	action = NEUTRAL;

	animFactor[NEUTRAL] = 1;
	animFactor[MOVE] = 1;
	animFactor[RETURN] = 1;
	animFactor[FAIR] = 1;
	animFactor[DAIR] = 1;
	animFactor[UAIR] = 1;
	animFactor[STANDN] = 1;
	animFactor[WALLATTACK] = 1;
	animFactor[CLIMBATTACK] = 1;
	animFactor[SLIDEATTACK] = 1;


	actionLength[NEUTRAL] = 10;
	actionLength[MOVE] = 60;
	actionLength[RETURN] = 60;
	actionLength[FAIR] = 40;
	actionLength[DAIR] = 40;
	actionLength[UAIR] = 40;
	actionLength[STANDN] = 40;
	actionLength[WALLATTACK] = 40;
	actionLength[CLIMBATTACK] = 40;
	actionLength[SLIDEATTACK] = 40;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	originalPos = pos;

	deathFrame = 0;

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;

	//animationFactor = 5;

	//ts = owner->GetTileset( "Copycat.png", 80, 80 );
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
	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	//cout << "finish init" << endl;
}

void Copycat::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Copycat::ResetEnemy()
{
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

void Copycat::ActionEnded()
{
	if( frame == actionLength[action] )
	{
		switch( action )
		{
		case NEUTRAL:
			frame = 0;
			break;
		case MOVE:
			frame = 0;
			break;
		case RETURN:
			break;
		case FAIR:
			break;
		case DAIR:
			break;
		case STANDN:
			break;
		case WALLATTACK:
			break;
		case CLIMBATTACK:
			break;
		case SLIDEATTACK:
			break;
		}
	}
}

void Copycat::UpdatePrePhysics()
{
	ActionEnded();

	switch( action )
	{
	case NEUTRAL:
		//cout << "NEUTRAL";
		break;
	case FIRE:
		//cout << "FIRE";
		break;
	case INVISIBLE:
		//cout << "INVISIBLE";
		break;
	case FADEIN:
		//cout << "FADEIN";
		break;
	case FADEOUT: 
		//cout << "FADEOUT";
		break;
	}

	//cout << " " << frame << endl;

	switch( action )
	{
	case NEUTRAL:
		break;
	case FIRE:
		break;
	case INVISIBLE:
		break;
	case FADEIN:
		break;
	case FADEOUT: 
		break;
	}



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
			//cout << "dying" << endl;
		}

		receivedHit = NULL;
	}
}

void Copycat::UpdatePhysics()
{	
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

	if( !dead && !dying )
	{
		if( action == NEUTRAL )
		{
			Actor *player = owner->player;
			if( length( player->position - position ) < 300 )
			{
				action = FADEOUT;
				frame = 0;
			}
		}
		PhysicsResponse();
	}
	return;
}

void Copycat::PhysicsResponse()
{
	if( !dead && !dying && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( COLOR_MAGENTA, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}


			//owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 6, 3, facingRight );
		//	cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
			
			
		//	cout << "Copycat received damage of: " << receivedHit->damage << endl;
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
		//	cout << "Copycat just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Copycat::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
	}

	if( deathFrame == 0 && dead )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}
	
	if( deathFrame == 60 && dying )
	{
		dying = false;
		dead = true;
	}

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

	

	UpdateSprite();
}

void Copycat::UpdateSprite()
{
	if( !dying && !dead )
	{
		sprite.setTextureRect( ts->GetSubRect( 0 ) );
		sprite.setPosition( position.x, position.y );
	}
	if( dying )
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
	}
}

void Copycat::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead && !dying )
	{
		if( hasMonitor && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 40 );

			cs.setFillColor( Color::Black );

			//cs.setFillColor( monitor-> );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		if( action != INVISIBLE )
			target->draw( sprite );
	}
	else if( !dead )
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );
	}



}

void Copycat::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying )
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
}

bool Copycat::IHitPlayer()
{
	if( action == FADEIN || action == INVISIBLE )
		return false;

	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Copycat::UpdateHitboxes()
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
pair<bool,bool> Copycat::PlayerHitMe()
{
	if( action == INVISIBLE )
		return pair<bool,bool>(false,false);

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

bool Copycat::PlayerSlowingMe()
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

void Copycat::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Copycat::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Copycat::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}