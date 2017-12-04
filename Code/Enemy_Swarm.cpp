#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Swarm.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

SwarmMember::SwarmMember(Swarm *p_parent, 
		sf::VertexArray &p_va, int index, V2d &p_targetOffset, 
		double p_maxSpeed )
		:Enemy( p_parent->owner, Enemy::SWARM, false, 5 ), va( p_va ), 
		vaIndex( index ), parent( p_parent ), maxSpeed( p_maxSpeed )
{
	/*va[vaIndex*4+0].color = Vector2f( 0, 0 );
	va[vaIndex*4+1].color = Vector2f( 0, 0 );
	va[vaIndex*4+2].color = Vector2f( 0, 0 );
	va[vaIndex*4+3].color = Vector2f( 0, 0 );*/

	affectCameraZoom = true;
	framesToLive = parent->liveFrames;
	targetOffset = p_targetOffset;
	active = true;
	dead = false;
	deathFrame = 0;
	velocity = V2d( 0, 0 );
	frame = 0;
	slowMultiple = 1;
	slowCounter = 1;
	receivedHit = NULL;

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 32;
	hitBody.rh = 32;
	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
}

void SwarmMember::ClearSprite()
{
	va[vaIndex*4+0].position = Vector2f( 0, 0 );
	va[vaIndex*4+1].position = Vector2f( 0, 0 );
	va[vaIndex*4+2].position = Vector2f( 0, 0 );
	va[vaIndex*4+3].position = Vector2f( 0, 0 );
}

void SwarmMember::UpdatePostPhysics()
{
	if( !dead && active && receivedHit != NULL )
	{
		owner->Pause( 5 );
		dead = true;
		owner->ActivateEffect( EffectLayer::IN_FRONT, parent->ts_swarmExplode, position, true, 0, 6, 2, true );
		receivedHit = NULL;
		active = false;
		ClearSprite();
		return;
	}

	if( slowCounter == slowMultiple )
	{
		
		--framesToLive;
		++frame;	
		//--framesToLive;

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
	
	if( framesToLive == 0 )
	{
		dead = true;
		active = false;
		ClearSprite();
		return;
	}
	//if( deathFrame == 30 || framesToLive == 0 )//|| framesToLive == 0 )
	//{
	//	
	//	//launcher->Reset();//might just delete bullets
	//	//parent->DeactivateBlock( this );
	//	//owner->RemoveEnemy( this );
	//	return;
	//}




	UpdateSprite();
	//launcher->UpdateSprites();
}

void SwarmMember::UpdateSprite()
{
	if( dead )
	{
		ClearSprite();
	}
	else
	{
		IntRect subRect = parent->ts_swarm->GetSubRect( vaIndex * 3 );//frame / animFactor );
		if( owner->GetPlayer( 0 )->position.x < position.x )
		{
			subRect.left += subRect.width;
			subRect.width = -subRect.width;
		}
		va[vaIndex*4+0].texCoords = Vector2f( subRect.left, 
			subRect.top );
		va[vaIndex*4+1].texCoords = Vector2f( subRect.left 
			+ subRect.width, 
			subRect.top );
		va[vaIndex*4+2].texCoords = Vector2f( subRect.left 
			+ subRect.width, 
			subRect.top + subRect.height );
		va[vaIndex*4+3].texCoords = Vector2f( subRect.left, 
			subRect.top + subRect.height );

		Vector2f p( position.x, position.y );

		Vector2f spriteSize = parent->spriteSize;
		va[vaIndex*4+0].position = p + Vector2f( -spriteSize.x, -spriteSize.y );
		va[vaIndex*4+1].position = p + Vector2f( spriteSize.x, -spriteSize.y );
		va[vaIndex*4+2].position = p + Vector2f( spriteSize.x, spriteSize.y );
		va[vaIndex*4+3].position = p + Vector2f( -spriteSize.x, spriteSize.y );
	}
}

void SwarmMember::UpdatePrePhysics()
{
	PlayerSlowingMe();
	//cout << "index: " << vaIndex << ", position: " << position.x << ", " << position.y << endl;
}

void SwarmMember::UpdatePhysics()
{
	specterProtected = false;
	if( !dead )
	{
		
		position += velocity / NUM_STEPS / (double)slowMultiple;
		V2d pPos = owner->GetPlayer( 0 )->position + targetOffset;
		V2d dir( pPos - position );
		dir = normalize( dir );
		double gFactor = .5;
		velocity += gFactor * dir / (double)slowMultiple / NUM_STEPS;

		if( length( velocity ) > maxSpeed / (double)slowMultiple )
		{
			velocity = normalize( velocity ) * maxSpeed / (double)slowMultiple;
		}
	}

	PhysicsResponse();
}

void SwarmMember::PhysicsResponse()
{
//	PlayerSlowingMe();

	if( !dead )
	{
		UpdateHitboxes();

		
		if( receivedHit == NULL )
		{
			pair<bool, bool> result = PlayerHitMe();
			if( result.first )
			{
				owner->GetPlayer( 0 )->ConfirmHit( 5, 5, .8, 6 );


				if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
				{
					owner->GetPlayer( 0 )->velocity.y = 4;//.5;
				}
			}
		}

		

		if( IHitPlayer() )
		{
			ClearSprite();
			//maybe call something in the swarm overall to create an effect?
			active = false;
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		
	}
}

void SwarmMember::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead )
	{
		/*if( hasMonitor && !suppressMonitor )
		{
			CircleShape cs;
			cs.setRadius( 50 );
			cs.setFillColor( Color::White );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		else*/
		{
			CircleShape cs;
			cs.setRadius( 20 );
			cs.setFillColor( Color::Red );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
	}
}

void SwarmMember::Draw(sf::RenderTarget *target )
{
}

bool SwarmMember::IHitPlayer( int index )
{
	Actor *player = owner->GetPlayer( 0 );
	if( hitBody.Intersects( player->hurtBody ) )
	{
		if( player->position.x < position.x )
		{
			hitboxInfo->kbDir = V2d( -1, -1 );
			//cout << "left" << endl;
		}
		else if( player->position.x > position.x )
		{
			//cout << "right" << endl;
			hitboxInfo->kbDir = V2d( 1, -1 );
		}
		else
		{
			//dont change it
		}

		player->ApplyHit( hitboxInfo );
		return true;
	}
}

std::pair<bool,bool> SwarmMember::PlayerHitMe( int index )
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

bool SwarmMember::PlayerSlowingMe()
{
	Actor *player = owner->GetPlayer( 0 );
	bool found = false;
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
			{
				found = true;
				if( slowMultiple == 1 )
				{
					slowCounter = 1;
					slowMultiple = 5;
				}
				break;
			}
		}
	}

	if( !found )
	{
		slowCounter = 1;
		slowMultiple = 1;
	}
	return false;
}

void SwarmMember::DebugDraw(sf::RenderTarget *target)
{
	hitBody.DebugDraw( target );
	hurtBody.DebugDraw( target );
}

void SwarmMember::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hitBody.globalPosition = position;
}

void SwarmMember::SaveEnemyState()
{
}

void SwarmMember::LoadEnemyState()
{
}
	
void SwarmMember::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void SwarmMember::ResetEnemy()
{
	framesToLive = parent->liveFrames;
	frame = 0;
	dead = false;
	//active = true;
	deathFrame = 0;
	velocity = V2d( 0, 0 );
	slowMultiple = 1;
	slowCounter = 1;
	receivedHit = NULL;
	ClearSprite();

	
}

//spriteSize = Vector2f( 16, 16 );

Swarm::Swarm( GameSession *owner,
	sf::Vector2i &pos, int p_liveFrames )
	:Enemy( owner, Enemy::SWARM, false, 5 ), swarmVA( sf::Quads, 5 * 4 )
{
	
	ts_swarmExplode = owner->GetTileset( "bullet_explode2_64x64.png", 64, 64 );

	actionLength[NEUTRAL] = 10;
	actionLength[FIRE] = 6;
	actionLength[USED] = 10;
	actionLength[REFILL] = 30;

	animFactor[NEUTRAL] = 2;
	animFactor[FIRE] = 10;
	animFactor[USED] = 1;
	animFactor[REFILL] = 1;

	liveFrames = p_liveFrames;
	ts = owner->GetTileset( "swarm_pod_128x128.png", 128, 128 );
	ts_swarm = owner->GetTileset( "swarm_64x64.png", 64, 64 );
	nestSprite.setTexture( *ts->texture );
	position = V2d( pos.x, pos.y );
	origPosition = position;
	//SwarmMember *mem = new SwarmMember()
	int blah = 200;
	double angle = PI;
	V2d offset; 
	double radius = 90;
	double speed = 12;
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		offset = V2d( cos( angle - PI / 2 ), sin( angle - PI / 2 ) ) * radius;
		//V2d offset( ( rand() % blah ) - blah, (rand() %blah) - blah );
		members[i] = new SwarmMember( this, swarmVA, i, offset, speed );
		members[i]->position = position + offset;


		angle += 2 * PI / NUM_SWARM;
		speed++;
	}

	spawnRect = Rect<double>( pos.x - 50, pos.y - 50, 100, 100 );
	//members[0] = new SwarmMember( this, swarmVA, 0 );
	initHealth = 40;
	health = initHealth;
	deathFrame = 0;

	spriteSize = Vector2f( 24, 24 );

	dead = false;
	dying = false;

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	//facingRight = true;
	//maxSpeed = 12; 

	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;
	hurtBody.globalPosition = position;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 32;
	hitBody.rh = 32;
	hitBody.globalPosition = position;
	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	ResetEnemy();
}

void Swarm::Launch()
{
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		members[i]->active = true;
		members[i]->Reset();
		members[i]->position = position + members[i]->targetOffset;
		members[i]->velocity = normalize( members[i]->targetOffset ) * 6.0;
	}
}


void Swarm::HandleEntrant( QuadTreeEntrant *qte )
{
	//fix when correcting
	/*SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}*/
}

void Swarm::ResetEnemy()
{
	dead = false;
	dying = false;
	health = initHealth;
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		members[i]->Reset();
		members[i]->position = position + members[i]->targetOffset;
		members[i]->active = false;
	}

	slowCounter = 1;
	slowMultiple = 1;

	action = NEUTRAL;
	frame = 0;
	receivedHit = NULL;
	deathFrame = 0;

	UpdateSprite();
}

void Swarm::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case NEUTRAL:
			frame = 0;
			break;
		case FIRE:
			action = USED;
			frame = 0;
			break;
		case USED:
			frame = 0;
			break;
		case REFILL:
			action = NEUTRAL;
			frame = 0;
			break;
		}
	}
}

void Swarm::UpdatePrePhysics()
{
	ActionEnded();
	switch( action )
	{
	case NEUTRAL:
		cout << "neutral: " << frame << endl;
		break;
	case FIRE:
		cout << "fire: " << frame << endl;
		if( frame == ( actionLength[FIRE] - 2 ) * animFactor[FIRE] )
			Launch();
		break;
	case USED:
		cout << "used: " << frame << endl;
		break;
	case REFILL:
		cout << "refill: " << frame << endl;
		break;
	}

	if( !dead && !dying && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dying = true;
			owner->GetPlayer( 0 )->ConfirmEnemyKill( this );
		}
		else
		{
			owner->GetPlayer( 0 )->ConfirmEnemyNoKill( this );
		}

		receivedHit = NULL;
	}


	int activeMembers = 0;

	/*if( framesSinceLaunch == liveFrames - 1 )
	{
		for( int i = 0; i < NUM_SWARM; ++i )
		{
			if( members[i]->active )
				members[i]->active = false;
		}
	}*/
	

	for( int i = 0; i < NUM_SWARM; ++i )
	{
		if( members[i]->active )
		{
			activeMembers++;
		}
	}

	if( activeMembers > 0 )
	{
		for( int i = 0; i < NUM_SWARM; ++i )
		{
			if( members[i]->active )
				members[i]->UpdatePrePhysics();
		}
	}
	else
	{
		if( !dying && !dead )
		{
			if( action == USED )
			{
				action = REFILL;
				frame = 0;
			}
			else if( action == NEUTRAL )
			{
				double dist = length( owner->GetPlayer( 0 )->position - position );
				if( dist < 900 )
				{
					action = FIRE;
					frame = 0;
					//Launch();
				}
			}
		}
	}

	
	//for( int i = 0; i < NUM_
}

void Swarm::UpdatePhysics()
{
	specterProtected = false;
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		if( members[i]->active )
			members[i]->UpdatePhysics();
	}

	PhysicsResponse();
}

void Swarm::PhysicsResponse()
{

	PlayerSlowingMe();

	if( !( dead || dying ) )
	{
		UpdateHitboxes();
		
		if( receivedHit == NULL )
		{
			pair<bool, bool> result = PlayerHitMe();
			if( result.first && !specterProtected )
			{
				owner->GetPlayer( 0 )->ConfirmHit( 5, 5, .8, 6 );

				if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
				{
					owner->GetPlayer( 0 )->velocity.y = 4;//.5;
				}
			}
		}

		if( IHitPlayer() )
		{
		}
	}
}

void Swarm::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->GetPlayer( 0 )->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	if( deathFrame == 0 && dying )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}


	int activeNum = 0;
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		if( members[i]->active )
		{
			activeNum++;
			members[i]->UpdatePostPhysics();
		}
	}

	if( deathFrame == 30 && dying )
	{
		dying = false;
		dead = true;
	}

	UpdateSprite();

	if( slowCounter == slowMultiple )
	{	
		++frame;		

		slowCounter = 1;
	
		if( dying )
		{
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	if( activeNum == 0 && dead )
	{
		owner->RemoveEnemy( this );
	}
}

void Swarm::DrawMinimap( sf::RenderTarget *target )
{
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

void Swarm::Draw(sf::RenderTarget *target )
{
	if( !(dead || dying ) )
	{
		if( hasMonitor && !suppressMonitor )
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( nestSprite, keyShader );
			}
			else
			{
				target->draw( nestSprite, hurtShader );
			}
			target->draw( *keySprite );
		}
		else
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( nestSprite );
			}
			else
			{
				target->draw( nestSprite, hurtShader );
			}
			
		}
	}
	else if( dying )
	{
		target->draw( botDeathSprite );
		target->draw( topDeathSprite );
	}

	/*sf::RectangleShape rs;
	rs.setFillColor( Color::Red );
	rs.setSize( Vector2f( 100, 100 ) );
	rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
	rs.setPosition( position.x, position.y );
	target->draw( rs );*/
	//target->draw( nestSprite );
	target->draw( swarmVA, ts_swarm->texture );
	
	
}

void Swarm::UpdateSprite()
{
	if( !dead && !dying )
	{
		switch( action )
		{
		case NEUTRAL:
			nestSprite.setTextureRect( ts->GetSubRect( 0 ) );
			break;
		case FIRE:
			nestSprite.setTextureRect( ts->GetSubRect( frame / animFactor[FIRE] + 1) );
			break;
		case USED:
			nestSprite.setTextureRect( ts->GetSubRect( 6 ) );
			break;
		case REFILL:
			nestSprite.setTextureRect( ts->GetSubRect( 6 ) );
			break;
		}
		nestSprite.setOrigin( nestSprite.getLocalBounds().width / 2, 
			nestSprite.getLocalBounds().height / 2 );
		nestSprite.setPosition( position.x, position.y );
	}
	else if( dying )
	{
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 8 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, 
			botDeathSprite.getLocalBounds().height / 2  );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( 0 );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 7 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, 
			topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( 0 );
	}
}

void Swarm::DebugDraw(sf::RenderTarget *target)
{
	hurtBody.DebugDraw( target );
	hitBody.DebugDraw( target );
}

void Swarm::UpdateHitboxes()
{
}

bool Swarm::IHitPlayer( int index )
{
	Actor *player = owner->GetPlayer( 0 );
	if( hitBody.Intersects( player->hurtBody ) )
	{
		if( player->position.x < position.x )
		{
			hitboxInfo->kbDir = V2d( -1, -1 );
			//cout << "left" << endl;
		}
		else if( player->position.x > position.x )
		{
			//cout << "right" << endl;
			hitboxInfo->kbDir = V2d( 1, -1 );
		}
		else
		{
			//dont change it
		}

		player->ApplyHit( hitboxInfo );
		return true;
	}
}

std::pair<bool,bool> Swarm::PlayerHitMe( int index )
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

bool Swarm::PlayerSlowingMe()
{
	Actor *player = owner->GetPlayer( 0 );

	bool found = false;
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
			{
				found = true;
				if( slowMultiple == 1 )
				{
					slowCounter = 1;
					slowMultiple = 5;
				}
				break;
			}
		}
	}

	if( !found )
	{
		slowCounter = 1;
		slowMultiple = 1;
	}
	
	return false;
}

void Swarm::SaveEnemyState()
{
}

void Swarm::LoadEnemyState()
{
}

