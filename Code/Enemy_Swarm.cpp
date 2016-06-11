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

SwarmMember::SwarmMember(Swarm *p_parent, 
		sf::VertexArray &p_va, int index, V2d &p_targetOffset  )
		:Enemy( p_parent->owner, Enemy::SWARM ), va( p_va ), 
		vaIndex( index ), parent( p_parent )
{
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
	hitboxInfo->damage = 100;
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
		receivedHit = NULL;
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
	

	if( deathFrame == 30 || framesToLive == 0 )//|| framesToLive == 0 )
	{
		active = false;
		ClearSprite();
		//launcher->Reset();//might just delete bullets
		//parent->DeactivateBlock( this );
		//owner->RemoveEnemy( this );
		return;
	}




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
		IntRect subRect = parent->ts->GetSubRect( 0 );//frame / animFactor );
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

		Color c = Color::Red;
		va[vaIndex*4+0].color = c;
		va[vaIndex*4+1].color = c;
		va[vaIndex*4+2].color = c;
		va[vaIndex*4+3].color = c;


		Vector2f spriteSize = parent->spriteSize;
		va[vaIndex*4+0].position = p + Vector2f( -spriteSize.x, -spriteSize.y );
		va[vaIndex*4+1].position = p + Vector2f( spriteSize.x, -spriteSize.y );
		va[vaIndex*4+2].position = p + Vector2f( spriteSize.x, spriteSize.y );
		va[vaIndex*4+3].position = p + Vector2f( -spriteSize.x, spriteSize.y );
	
	}
}

void SwarmMember::UpdatePrePhysics()
{
	//cout << "index: " << vaIndex << ", position: " << position.x << ", " << position.y << endl;
}

void SwarmMember::UpdatePhysics()
{
	specterProtected = false;
	if( !dead )
	{
		
		position += velocity / NUM_STEPS;
		V2d pPos = owner->player.position + targetOffset;
		V2d dir( pPos - position );
		dir = normalize( dir );
		double gFactor = .5;
		velocity += gFactor * dir / (double)slowMultiple / NUM_STEPS;

		if( length( velocity ) > parent->maxSpeed)
		{
			velocity = normalize( velocity ) * parent->maxSpeed;
		}
	}

	PhysicsResponse();
}

void SwarmMember::PhysicsResponse()
{
	PlayerSlowingMe();

	if( !dead )
	{
		UpdateHitboxes();

		
		if( receivedHit == NULL )
		{
			pair<bool, bool> result = PlayerHitMe();
			if( result.first )
			{
				owner->player.test = true;
				owner->player.currAttackHit = true;
				owner->player.flashColor = COLOR_BLUE;
				owner->player.flashFrames = 5;
				owner->player.currentSpeedBar += .8;
				owner->player.swordShader.setParameter( "energyColor", COLOR_BLUE );
				owner->player.desperationMode = false;
				owner->powerBar.Charge( 2 * 6 * 3 );


				if( owner->player.ground == NULL && owner->player.velocity.y > 0 )
				{
					owner->player.velocity.y = 4;//.5;
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
}

void SwarmMember::Draw(sf::RenderTarget *target )
{
}

bool SwarmMember::IHitPlayer()
{
	Actor &player = owner->player;
	if( hitBody.Intersects( player.hurtBody ) )
	{
		if( player.position.x < position.x )
		{
			hitboxInfo->kbDir = V2d( -1, -1 );
			//cout << "left" << endl;
		}
		else if( player.position.x > position.x )
		{
			//cout << "right" << endl;
			hitboxInfo->kbDir = V2d( 1, -1 );
		}
		else
		{
			//dont change it
		}

		player.ApplyHit( hitboxInfo );
		return true;
	}
}

std::pair<bool,bool> SwarmMember::PlayerHitMe()
{
	Actor &player = owner->player;

	if( player.currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player.currHitboxes->begin(); it != player.currHitboxes->end(); ++it )
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
				receivedHit = player.currHitboxInfo;
				return pair<bool, bool>(true,false);
			}
			else
			{
				return pair<bool, bool>(false,false);
			}
			
		}
		
	}

	for( int i = 0; i < player.recordedGhosts; ++i )
	{
		if( player.ghostFrame < player.ghosts[i]->totalRecorded )
		{
			if( player.ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player.ghosts[i]->currHitboxes->begin(); it != player.ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player.currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player.ghosts[i]->curhi
		}
	}
	return pair<bool, bool>(false,false);
}

bool SwarmMember::PlayerSlowingMe()
{
	Actor &player = owner->player;
	bool found = false;
	for( int i = 0; i < player.maxBubbles; ++i )
	{
		if( player.bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player.bubblePos[i] ) <= player.bubbleRadius )
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
	active = true;
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
	:Enemy( owner, Enemy::SWARM ), swarmVA( sf::Quads, 5 * 4 )
{
	liveFrames = p_liveFrames;
	ts = owner->GetTileset( "bat_48x48.png", 48, 48 );
	position = V2d( pos.x, pos.y );
	origPosition = position;
	//SwarmMember *mem = new SwarmMember()
	int blah = 200;
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		V2d offset( ( rand() % blah ) - blah, (rand() %blah) - blah );
		members[i] = new SwarmMember( this, swarmVA, i, 
			 offset );
		members[i]->position = position + offset;
	}

	spawnRect = Rect<double>( pos.x - 50, pos.y - 50, 100, 100 );
	//members[0] = new SwarmMember( this, swarmVA, 0 );
	initHealth = 40;
	health = initHealth;
	deathFrame = 0;

	spriteSize = Vector2f( 24, 24 );

	dead = false;

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	//facingRight = true;
	maxSpeed = 12; 

	ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	bloodSprite.setTexture( *ts_testBlood->texture );

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
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
}

void Swarm::Launch()
{
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		members[i]->Reset();
		members[i]->position = position + members[i]->targetOffset;
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

void Swarm::UpdatePrePhysics()
{
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
		double dist = length( owner->player.position - position );
		if( dist < 900 )
		{
			Launch();
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
}

void Swarm::PhysicsResponse()
{
}

void Swarm::UpdatePostPhysics()
{
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		if( members[i]->active )
			members[i]->UpdatePostPhysics();
	}
}

void Swarm::DrawMinimap( sf::RenderTarget *target )
{
}

void Swarm::Draw(sf::RenderTarget *target )
{
	sf::RectangleShape rs;
	rs.setFillColor( Color::Red );
	rs.setSize( Vector2f( 100, 100 ) );
	rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
	rs.setPosition( position.x, position.y );
	target->draw( rs );

	
	target->draw( swarmVA, ts->texture );
}

void Swarm::UpdateSprite()
{
}

void Swarm::DebugDraw(sf::RenderTarget *target)
{
}

void Swarm::UpdateHitboxes()
{
}

bool Swarm::IHitPlayer()
{
	return false;
}

std::pair<bool,bool> Swarm::PlayerHitMe()
{
	return pair<bool,bool>(false,false);
}

bool Swarm::PlayerSlowingMe()
{
	return false;
}

void Swarm::SaveEnemyState()
{
}

void Swarm::LoadEnemyState()
{
}

void Swarm::ResetEnemy()
{
	dead = false;

	for( int i = 0; i < NUM_SWARM; ++i )
	{
		members[i]->Reset();
		members[i]->position = position + members[i]->targetOffset;
	}
}