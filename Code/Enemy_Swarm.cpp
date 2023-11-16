#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Swarm.h"
#include "AbsorbParticles.h"

using namespace std;
using namespace sf;

SwarmMember::SwarmMember(Swarm *p_parent, int index, V2d &p_targetOffset, 
		double p_maxSpeed )
		:Enemy( EnemyType::EN_SWARMMEMBER, 5 ),
		vaIndex( index ), parent( p_parent ), maxSpeed( p_maxSpeed )
{
	SetNumActions(A_Count);

	data.framesToLive = parent->liveFrames;
	targetOffset = p_targetOffset;

	data.velocity = V2d( 0, 0 );
	frame = 0;
	slowMultiple = 1;
	slowCounter = 1;
	receivedHit.SetEmpty();

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 2;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = V2d(1, 0);
	hitboxInfo->hType = HitboxInfo::RED;
	
	actionLength[FLY] = 2;
	actionLength[EXPLODE] = 8;

	animFactor[FLY] = 2;
	animFactor[EXPLODE] = 3;


	
	BasicCircleHurtBodySetup(30);
	BasicCircleHitBodySetup(20);
	hitBody.hitboxInfo = hitboxInfo;

	//cutObject->Setup(parent->ts_swarm, 0, 0, scale);

	ResetEnemy();
}

void SwarmMember::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case FLY:
		{
			frame = 0;
			break;
		}
		case EXPLODE:
		{
			active = false;
			//numHealth = 0;
			dead = true;
			ClearSprite();
			sess->RemoveEnemy(this);
			break;
		}
		}
		
	}

	if (action == FLY)
	{
		if (PlayerDir().x >= 0)
		{
			facingRight = true;
		}
		else
		{
			facingRight = false;
		}
	}
	

	if (data.framesToLive == 0 && action == FLY )
	{
		Explode();
		
		/*numHealth = 0;
		active = false;
		dead = true;
		ClearSprite();*/
	}
}

void SwarmMember::Explode()
{
	action = EXPLODE;
	frame = 0;
	HurtboxesOff();
	HitboxesOff();
	data.velocity = V2d(0, 0);
}

//void SwarmMember::HandleNoHealth()
//{
//	active = false;
//	dead = true;
//	ClearSprite();
//}

void SwarmMember::FrameIncrement()
{
	if (data.framesToLive > 0)
	{
		--data.framesToLive;
	}
}

void SwarmMember::ProcessHit()
{
	if (!dead && HasReceivedHit())// && numHealth > 0 )
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			if (hasMonitor && !suppressMonitor)
			{
				//sess->CollectKey();
			}

			Explode();
			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			ConfirmKill();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();
		}

		receivedHit.SetEmpty();
	}
}

void SwarmMember::ConfirmKill()
{
	HitboxInfo::HitboxType hType;

	if (!receivedHit.IsEmpty())
	{
		hType = receivedHit.hType;
	}
	else
	{
		hType = HitboxInfo::HitboxType::NORMAL;
	}

	if (receivedHit.comboer)
	{
		pauseFrames = 7;
		Enemy *ce = sess->GetEnemyFromID(comboHitEnemyID);
		ce->ComboKill(this);

	}

	else if (hType == HitboxInfo::WIREHITRED || hType == HitboxInfo::WIREHITBLUE)
	{
		pauseFrames = 7;
	}
	else
	{
		pauseFrames = 7;
		//sess->Pause(7);
		//pauseFrames = 0;
	}
	pauseBeganThisFrame = true;

	pauseFramesFromAttacking = false;

	sess->ActivateEffect(EffectLayer::BEHIND_ENEMIES, parent->ts_swarm, GetPosition(), true, 0, 8, 3, true, 20);

	if (!receivedHit.comboer)
	{
		sess->cam.SetRumble(1.5, 1.5, 7);
	}

	sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::ENERGY,
		sess->GetPlayer(receivedHitPlayerIndex), GetNumEnergyAbsorbParticles(), GetPosition());

	//dead = true;

	if (cutObject != NULL)
	{
		SyncCutObject();
	}

	HandleNoHealth();
	PlayDeathSound();
}

void SwarmMember::Throw( V2d &pos )
{
	if (active)
	{
		sess->RemoveEnemy(this);
	}

	swarmTypeIndex = vaIndex;

	if (swarmTypeIndex == 0 || swarmTypeIndex == 1)
	{
		animFactor[FLY] = 4;
	}
	else
	{
		animFactor[FLY] = 4;
	}

	action = FLY;
	frame = 0;

	HurtboxesOff();
	HitboxesOff();

	spawned = false;
	sess->AddEnemy(this);
	currPosInfo.position = pos + targetOffset;
	data.velocity = normalize(targetOffset) * 6.0;
}

void SwarmMember::ClearSprite()
{
	ClearRect(parent->swarmVA + vaIndex * 4);
}

void SwarmMember::UpdateSprite()
{
	SetRectCenter(parent->swarmVA + vaIndex * 4, parent->spriteSize.x, parent->spriteSize.y, GetPositionF());

	switch (action)
	{
	case FLY:
	{
		parent->ts_swarm->SetQuadSubRect(parent->swarmVA + vaIndex * 4, (frame / animFactor[FLY]) + swarmTypeIndex * 2, !facingRight);
		break;
	}
	case EXPLODE:
	{
		parent->ts_swarm->SetQuadSubRect(parent->swarmVA + vaIndex * 4, 20 + frame / animFactor[EXPLODE], !facingRight);
		break;
	}
	}
	

	/*if( sess->GetPlayerPos( 0 ).x < GetPosition().x )
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

	Vector2f p( GetPositionF() );

	Vector2f spriteSize = parent->spriteSize;
	va[vaIndex*4+0].position = p + Vector2f( -spriteSize.x, -spriteSize.y );
	va[vaIndex*4+1].position = p + Vector2f( spriteSize.x, -spriteSize.y );
	va[vaIndex*4+2].position = p + Vector2f( spriteSize.x, spriteSize.y );
	va[vaIndex*4+3].position = p + Vector2f( -spriteSize.x, spriteSize.y );*/
}

void SwarmMember::UpdateEnemyPhysics()
{
	double steps = numPhysSteps;
	V2d movementVec = data.velocity;
	movementVec /= slowMultiple * steps;

	currPosInfo.position += movementVec;

	if (action == FLY)
	{
		V2d pPos = sess->GetPlayerPos(0) + targetOffset;
		V2d dir(pPos - GetPosition());
		dir = normalize(dir);
		double gFactor = .5;
		data.velocity += gFactor * dir / (double)slowMultiple / steps;

		if (length(data.velocity) > maxSpeed)
		{
			data.velocity = normalize(data.velocity) * maxSpeed;
		}
	}
}

void SwarmMember::ResetEnemy()
{
	action = FLY;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	data.framesToLive = parent->liveFrames;
	
	
	active = false;

	data.velocity = V2d( 0, 0 );
	slowMultiple = 1;
	slowCounter = 1;
	receivedHit.SetEmpty();

	UpdateHitboxes();
	ClearSprite();
}

int SwarmMember::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void SwarmMember::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void SwarmMember::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}

int SwarmMember::GetNumEnergyAbsorbParticles()
{
	return 1;
}


Swarm::Swarm( ActorParams *ap )
	:Enemy( EnemyType::EN_SWARM, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());
	


	liveFrames = 420;//600;//300;

	actionLength[NEUTRAL] = 10;
	actionLength[FIRE] = 9;
	actionLength[USED] = 10;
	actionLength[REFILL] = 30;

	animFactor[NEUTRAL] = 2;
	animFactor[FIRE] = 7;
	animFactor[USED] = 1;
	animFactor[REFILL] = 1;

	
	ts = GetSizedTileset("Enemies/W5/swarm_pod_160x160.png");
	ts_swarm = GetSizedTileset( "Enemies/W5/swarm_80x80.png");

	sprite.setTexture( *ts->texture );
	
	int blah = 200;
	double angle = PI;
	V2d offset; 
	double radius = 90;
	double speed = 12;

	for( int i = 0; i < NUM_SWARM; ++i )
	{
		offset = V2d( cos( angle - PI / 2 ), sin( angle - PI / 2 ) ) * radius;
		//V2d offset( ( rand() % blah ) - blah, (rand() %blah) - blah );
		members[i] = new SwarmMember( this, i, offset, speed );
		//members[i]->position = position + offset;


		angle += 2 * PI / NUM_SWARM;
		speed++;
	}

	spriteSize = Vector2f(80, 80);//24, 24);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 20;
	hitboxInfo->knockback = 10;
	hitboxInfo->hType = HitboxInfo::RED;

	BasicCircleHitBodySetup(60);
	BasicCircleHurtBodySetup(70);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(10);
	cutObject->SetSubRectBack(11);
	cutObject->SetScale(scale);

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

Swarm::~Swarm()
{
	for (int i = 0; i < NUM_SWARM; ++i)
	{
		delete members[i];
	}
}

void Swarm::SetLevel(int lev)
{
	level = lev;

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
}

void Swarm::Launch()
{
	for( int i = 0; i < NUM_SWARM; ++i )
	{
		members[i]->Throw( GetPosition() );
	}
}

void Swarm::ResetEnemy()
{
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	dead = false;
	data.dying = false;

	for( int i = 0; i < NUM_SWARM; ++i )
	{
		members[i]->Reset();
		//members[i]->position = position + members[i]->targetOffset;
		//members[i]->active = false;
	}

	action = NEUTRAL;
	frame = 0;
	receivedHit.SetEmpty();

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
		//cout << "neutral: " << frame << endl;
		break;
	case FIRE:
		//cout << "fire: " << frame << endl;
		if( frame == 3 * animFactor[FIRE] && slowCounter == 1 )
			Launch();
		break;
	case USED:
		//cout << "used: " << frame << endl;
		break;
	case REFILL:
		//cout << "refill: " << frame << endl;
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
		if (!data.dying && !dead)
		{
			if (action == USED)
			{
				action = REFILL;
				frame = 0;
			}
			else if (action == NEUTRAL)
			{
				double dist = PlayerDist(0);
				if (dist < DEFAULT_DETECT_RADIUS )//1200)
				{
					action = FIRE;
					frame = 0;
					//Launch();
				}
			}
		}
		else if (data.dying)
		{
			dead = true;
			numHealth = 0;
		}
	}
}

void Swarm::AddToGame()
{
	Enemy::AddToGame();

	for (int i = 0; i < NUM_SWARM; ++i)
	{
		members[i]->SetEnemyIDAndAddToAllEnemiesVec();
	}
}

void Swarm::HandleNoHealth()
{
	dead = false;
	numHealth = 1;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	data.dying = true;
}

void Swarm::EnemyDraw(sf::RenderTarget *target )
{
	if (!data.dying)
	{
		DrawSprite(target, sprite);
	}
	target->draw( swarmVA, NUM_SWARM * 4, sf::Quads, ts_swarm->texture );
}

void Swarm::UpdateSprite()
{
	if( !data.dying )
	{
		switch( action )
		{
		case NEUTRAL:
			sprite.setTextureRect( ts->GetSubRect( 0 ) );
			break;
		case FIRE:
			sprite.setTextureRect( ts->GetSubRect( frame / animFactor[FIRE] + 1) );
			break;
		case USED:
			sprite.setTextureRect( ts->GetSubRect( 6 ) );
			break;
		case REFILL:
			sprite.setTextureRect( ts->GetSubRect( 6 ) );
			break;
		}
		sprite.setOrigin(sprite.getLocalBounds().width / 2,
			sprite.getLocalBounds().height / 2 );
		sprite.setPosition( GetPositionF() );
	}
}

int Swarm::GetNumEnergyAbsorbParticles()
{
	return Enemy::GetNumEnergyAbsorbParticles();// +NUM_SWARM * members[0]->GetNumEnergyAbsorbParticles();
}

int Swarm::GetTotalEnergyAbsorbParticles()
{
	return Enemy::GetNumEnergyAbsorbParticles() + NUM_SWARM * members[0]->GetNumEnergyAbsorbParticles();
}

int Swarm::GetNumStoredBytes()
{
	return sizeof(MyData) + members[0]->GetNumStoredBytes() * NUM_SWARM;
}

void Swarm::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	for (int i = 0; i < NUM_SWARM; ++i)
	{
		members[i]->StoreBytes(bytes);
		bytes += members[i]->GetNumStoredBytes();
	}
}

void Swarm::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	for (int i = 0; i < NUM_SWARM; ++i)
	{
		members[i]->SetFromBytes(bytes);
		bytes += members[i]->GetNumStoredBytes();
	}
}
