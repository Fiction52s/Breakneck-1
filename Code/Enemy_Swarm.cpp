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
		:Enemy( p_parent->owner, EnemyType::EN_SWARMMEMBER, false, 1, false ), va( p_va ), 
		vaIndex( index ), parent( p_parent ), maxSpeed( p_maxSpeed )
{
	framesToLive = parent->liveFrames;
	targetOffset = p_targetOffset;

	active = true;
	dead = false;


	velocity = V2d( 0, 0 );
	frame = 0;
	slowMultiple = 1;
	slowCounter = 1;
	receivedHit = NULL;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	SetupBodies(1, 1);
	AddBasicHurtCircle(32);
	AddBasicHitCircle(32);
	hitBody->hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void SwarmMember::ProcessState()
{
	if (framesToLive == 0)
	{
		numHealth = 0;
		active = false;
		dead = true;
		ClearSprite();
	}
}

void SwarmMember::HandleNoHealth()
{
	active = false;
	dead = true;
	ClearSprite();
}

void SwarmMember::FrameIncrement()
{
	if (framesToLive > 0)
	{
		--framesToLive;
	}
}

void SwarmMember::ClearSprite()
{
	va[vaIndex*4+0].position = Vector2f( 0, 0 );
	va[vaIndex*4+1].position = Vector2f( 0, 0 );
	va[vaIndex*4+2].position = Vector2f( 0, 0 );
	va[vaIndex*4+3].position = Vector2f( 0, 0 );
}

void SwarmMember::UpdateSprite()
{
	IntRect subRect = parent->ts_swarm->GetSubRect( vaIndex * 3 );//frame / animFactor );
	if( owner->GetPlayerPos( 0 ).x < position.x )
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

void SwarmMember::UpdateEnemyPhysics()
{
	double steps = numPhysSteps;
	V2d movementVec = velocity;
	movementVec /= slowMultiple * steps;

	position += movementVec;

	V2d pPos = owner->GetPlayerPos(0) + targetOffset;
	V2d dir(pPos - position);
	dir = normalize(dir);
	double gFactor = .5;
	velocity += gFactor * dir / (double)slowMultiple / steps;

	if (length(velocity) > maxSpeed)
	{
		velocity = normalize(velocity) * maxSpeed;
	}
}

void SwarmMember::ResetEnemy()
{
	action = FLY;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	framesToLive = parent->liveFrames;
	frame = 0;
	dead = false;
	velocity = V2d( 0, 0 );
	slowMultiple = 1;
	slowCounter = 1;
	receivedHit = NULL;
	ClearSprite();
}


Swarm::Swarm( GameSession *owner,
	sf::Vector2i &pos, bool p_hasMonitor, int p_level )
	:Enemy( owner, EnemyType::EN_SWARM, p_hasMonitor, 1 ), swarmVA( sf::Quads, 5 * 4 )
{
	level = p_level;

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
	
	liveFrames = 300;;



	//ts_swarmExplode = owner->GetTileset( "bullet_explode2_64x64.png", 64, 64 );

	actionLength[NEUTRAL] = 10;
	actionLength[FIRE] = 6;
	actionLength[USED] = 10;
	actionLength[REFILL] = 30;

	animFactor[NEUTRAL] = 2;
	animFactor[FIRE] = 10;
	animFactor[USED] = 1;
	animFactor[REFILL] = 1;

	
	ts = owner->GetTileset( "Enemies/swarm_pod_128x128.png", 128, 128 );
	ts_swarm = owner->GetTileset( "Enemies/swarm_64x64.png", 64, 64 );

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

	spriteSize = Vector2f( 24, 24 );

	//dead = false;
	//dying = false;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	SetupBodies(1, 1);
	AddBasicHurtCircle(48);
	AddBasicHitCircle(48);

	hitBody->hitboxInfo = hitboxInfo;

	ResetEnemy();
}

Swarm::~Swarm()
{
	for (int i = 0; i < NUM_SWARM; ++i)
	{
		delete members[i];
	}
}

void Swarm::Launch()
{
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		members[i]->active = true;
		members[i]->Reset();
		members[i]->position = position + members[i]->targetOffset;
		members[i]->velocity = normalize( members[i]->targetOffset ) * 6.0;
		owner->AddEnemy(members[i]);
	}
}

void Swarm::ResetEnemy()
{
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	dead = false;
	dying = false;

	for( int i = 0; i < NUM_SWARM; ++i )
	{
		members[i]->Reset();
		members[i]->position = position + members[i]->targetOffset;
		members[i]->active = false;
	}

	action = NEUTRAL;
	frame = 0;
	receivedHit = NULL;

	UpdateHitboxes();
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

void Swarm::ProcessState()
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

	int activeMembers = 0;

	for( int i = 0; i < NUM_SWARM; ++i )
	{
		if( members[i]->active )
		{
			activeMembers++;
		}
	}

	if( activeMembers == 0 )
	{
		if (!dying && !dead)
		{
			if (action == USED)
			{
				action = REFILL;
				frame = 0;
			}
			else if (action == NEUTRAL)
			{
				double dist = length(owner->GetPlayerPos(0) - position);
				if (dist < 900)
				{
					action = FIRE;
					frame = 0;
					//Launch();
				}
			}
		}
		else if (dying)
		{
			dead = true;
			numHealth = 0;
		}
	}
}

void Swarm::HandleNoHealth()
{
	dead = false;
	numHealth = 1;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	dying = true;
}

void Swarm::EnemyDraw(sf::RenderTarget *target )
{
	if (!dying)
	{
		DrawSpriteIfExists(target, nestSprite);
	}
	target->draw( swarmVA, ts_swarm->texture );
}

void Swarm::UpdateSprite()
{
	if( !dying )
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
}

