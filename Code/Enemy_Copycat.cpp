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
	t = UAIR;
	angle = 0;
	//a = UAIR;
	facingRight = true;
	reversed = false;
	speedLevel = 0;
	position = Vector2f( 0, 0 );
	//delayFrames = 0;
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
	PlayerAttack *p = NULL;
	if( activeAttacksBack == NULL )
	{
		cout << "popping but im out of active attacks" << endl;
	}
	else if( activeAttacksBack->prevAttack == NULL )
	{
		p = activeAttacksBack;
		p->prevAttack = NULL;

		activeAttacksFront = NULL;
		activeAttacksBack = NULL;
	}
	else
	{
		p = activeAttacksBack;
		p->prevAttack->nextAttack = NULL;
		activeAttacksBack = p->prevAttack;
		p->prevAttack = NULL;
	}

	if( p != NULL )
	{
		if( inactiveAttacks == NULL )
		{
			inactiveAttacks = p;
			//p->nextAttack = NULL;
			//p->prevAttack = NULL;
		}
		else
		{
			p->nextAttack = inactiveAttacks;
			inactiveAttacks->prevAttack = p;
			inactiveAttacks = p;
		}
	}

	/*int num = 0;
	PlayerAttack *pa = inactiveAttacks;
	while( pa != NULL )
	{
		++num;
		pa = pa->nextAttack;
	}*/
	//cout << "num inactive: " << num << endl;

	return p;
}

void Copycat::QueueAttack( PlayerAttack::Type t,
		bool facingRight,
		bool reversed, int speedLevel,
		const sf::Vector2<float> &pos,
		const sf::Vector2<float> &swordPos,
		float angle )
{
	fire = true;
	destPos = pos;
	PlayerAttack *p = GetAttack();
	if( p == NULL )
	{
		//assert( 0 );
	}
	else
	{
		p->t = t;
		p->facingRight = facingRight;
		p->reversed = reversed;
		p->speedLevel = speedLevel;
		p->position = pos;
		p->swordPosition = swordPos;
		p->angle = angle;
		SetTarget( p->index, pos );

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

void Copycat::ClearTargets()
{
	VertexArray &va = *targetVA;
	for( int i = 0; i < attackBufferSize; ++i )
	{
		va[i*4+0].position = Vector2f( 0, 0 );
		va[i*4+1].position = Vector2f( 0, 0 );
		va[i*4+2].position = Vector2f( 0, 0 );
		va[i*4+3].position = Vector2f( 0, 0 );
	}
}

void Copycat::ClearTarget( int index )
{
	VertexArray &va = *targetVA;
	va[index*4+0].position = Vector2f( 0, 0 );
	va[index*4+1].position = Vector2f( 0, 0 );
	va[index*4+2].position = Vector2f( 0, 0 );
	va[index*4+3].position = Vector2f( 0, 0 );
}

void Copycat::SetTarget( int index, const Vector2f &pos )
{
	V2d p( pos.x, pos.y );
	V2d dir = normalize( position - p );

	double a = atan2( dir.x, -dir.y );
	float ra = a / PI * 180.f;
	Transform t;
	t.rotate( ra );

	IntRect sub = ts_target->GetSubRect( 0 );
	float hwidth = sub.width / 2;
	float hheight = sub.height / 2;

	VertexArray &va = *targetVA;
	va[index*4+0].position = pos + t.transformPoint( Vector2f( -hwidth, -hheight ) );
	va[index*4+1].position = pos +  t.transformPoint( Vector2f( hwidth, -hheight ) );
	va[index*4+2].position = pos +  t.transformPoint( Vector2f( hwidth, hheight ) );
	va[index*4+3].position = pos +  t.transformPoint( Vector2f( -hwidth, hheight ) );
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
		p->index = i;
		allAttacks[i] = p;
	}

	ts_bulletExplode = owner->GetTileset( "bullet_explode2_64x64.png", 64, 64 );

	fire = false;
	action = NEUTRAL;

	animFactor[NEUTRAL] = 1;
	animFactor[THROW] = 1;
	/*animFactor[RETURN] = 1;
	animFactor[FAIR] = 1;
	animFactor[DAIR] = 1;
	animFactor[UAIR] = 1;
	animFactor[STANDN] = 1;
	animFactor[WALLATTACK] = 1;
	animFactor[CLIMBATTACK] = 1;
	animFactor[SLIDEATTACK] = 1;*/


	actionLength[NEUTRAL] = 10;
	actionLength[THROW] = 60;
	/*actionLength[FAIR] = 40;
	actionLength[DAIR] = 40;
	actionLength[UAIR] = 40;
	actionLength[STANDN] = 40;
	actionLength[WALLATTACK] = 40;
	actionLength[CLIMBATTACK] = 40;
	actionLength[SLIDEATTACK] = 40;*/

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
	ts_target = owner->GetTileset( "copycattarget_64x64.png", 64, 64 );

	targetVA = new VertexArray( sf::Quads, attackBufferSize * 4 );
	VertexArray &tva = *targetVA;
	IntRect sub = ts_target->GetSubRect( 0 );
	for( int i = 0; i < attackBufferSize; ++i )
	{
		tva[i*4+0].texCoords = Vector2f( sub.left, sub.top);
		tva[i*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top);
		tva[i*4+2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height);
		tva[i*4+3].texCoords = Vector2f( sub.left, sub.top + sub.height );				
	}

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

	launcher = new Launcher( this, BasicBullet::BType::COPYCAT, owner,
		20, 1, position, V2d( 0, -1 ), 0, 800, false );
	

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
	Actor *player = owner->player;
	//for( map<int, list<CollisionBox*>>::iterator it = owner->player->fairHitboxes.begin();
	CopyHitboxes( fairHitboxes, player->fairHitboxes );
	CopyHitboxes( dairHitboxes, player->dairHitboxes );
	CopyHitboxes( uairHitboxes, player->uairHitboxes );
	CopyHitboxes( standHitboxes, player->standHitboxes );
	CopyHitboxes( steepClimbHitboxes, player->steepClimbHitboxes );
	CopyHitboxes( steepSlideHitboxes, player->steepSlideHitboxes );
	CopyHitboxes( wallHitboxes, player->wallHitboxes );

	attackLength[FAIR] = player->actionLength[Actor::FAIR];
	actionLength[DAIR] = player->actionLength[Actor::DAIR];
	actionLength[UAIR] = player->actionLength[Actor::UAIR];
	actionLength[STANDN] = player->actionLength[Actor::STANDN];
	actionLength[CLIMBATTACK] = player->actionLength[Actor::STEEPCLIMBATTACK];
	actionLength[SLIDEATTACK] = player->actionLength[Actor::STEEPSLIDEATTACK];
	actionLength[WALLATTACK] = player->actionLength[Actor::WALLATTACK];

	UpdateHitboxes();

	ResetEnemy();
	//cout << "finish init" << endl;
}

void Copycat::CopyHitboxes( std::map<int, std::list<CollisionBox>*> &boxes,
		std::map<int, std::list<CollisionBox>*> &playerBoxes )
{
	for( map<int, list<CollisionBox>*>::iterator it = playerBoxes.begin();
		it != playerBoxes.end(); ++it )
	{
		list<CollisionBox>*& currList = boxes[(*it).first];
		currList = new list<CollisionBox>( (*it).second );
	}
}

void Copycat::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Copycat::BulletHitTarget( BasicBullet *b )
{
	PlayerAttack *p = PopAttack();
	if( p != NULL )
	{
		ClearTarget( p->index );
		currAttack = *p;
		currAttackFrame = 0;
	}
	else
	{
		assert( 0 );
	}

	//V2d vel = b->velocity;
	//double angle = atan2( vel.y, vel.x );
	//owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	//owner->player->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
	//assert( p != NULL );

}

void Copycat::ResetEnemy()
{
	launcher->Reset();
	ClearTargets();
	ResetAttacks();
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
		case THROW:
			frame = 0;
			action = NEUTRAL;
			//frame = 0;
			break;
		}
	}
}

void Copycat::UpdatePrePhysics()
{
	ActionEnded();

	launcher->UpdatePrePhysics();

	if( fire )
	{
		//V2d launchDir = V2d( destPos.x, destPos.y ) - position;
		launcher->bulletSpeed = 1;		
		launcher->facingDir = V2d( destPos.x, destPos.y );
		
		//cout << "destpos: " << destPos.x << ", " << destPos.y << endl;
		//cout << "dir: " << launchDir.x << ", " << launchDir.y << endl;
		//launcher->bulletSpeed = 10;
		launcher->Fire();
		action = THROW;
		frame = 0;
	}

	switch( action )
	{
	case NEUTRAL:
		break;
	case THROW:
		break;
	}


	Actor *player = owner->player;

	


	switch( action )
	{
	case NEUTRAL:
		{
			
		}
		break;
	case THROW:
		{
			
		}
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
	launcher->UpdatePhysics();
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
		/*if( action == NEUTRAL )
		{
			Actor *player = owner->player;
			if( length( player->position - position ) < 300 )
			{
				action = FADEOUT;
				frame = 0;
			}
		}*/
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

void Copycat::DirectKill()
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

void Copycat::UpdatePostPhysics()
{
	launcher->UpdatePostPhysics();

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
	Actor *player = owner->player;
	//the player already did a ++frame for their action

	fire = false;
	if( player->frame == 1 )
	{
		switch( player->action )
		{
		case Actor::Action::FAIR:
			QueueAttack( PlayerAttack::FAIR, player->facingRight, 
				false, player->speedLevel, player->sprite->getPosition(),
				player->fairSword.getPosition(), 0 );
			break;
		case Actor::Action::DAIR:
			QueueAttack( PlayerAttack::FAIR, player->facingRight, 
				false, player->speedLevel, player->sprite->getPosition(),
				player->dairSword.getPosition(), 0 );
			break;
		case Actor::Action::UAIR:
			QueueAttack( PlayerAttack::FAIR, player->facingRight, 
				false, player->speedLevel, player->sprite->getPosition(),
				player->uairSword.getPosition(), 0 );
			break;
		case Actor::Action::STANDN:
			QueueAttack( PlayerAttack::FAIR, player->facingRight, 
				false, player->speedLevel, player->sprite->getPosition(),
				player->standingNSword.getPosition(), player->sprite->getRotation() );
			break;
		case Actor::Action::STEEPCLIMBATTACK:
			QueueAttack( PlayerAttack::FAIR, player->facingRight, 
				false, player->speedLevel, player->sprite->getPosition(),
				player->steepClimbAttackSword.getPosition(), player->sprite->getRotation() );
			break;
		case Actor::Action::STEEPSLIDEATTACK:
			QueueAttack( PlayerAttack::FAIR, player->facingRight, 
				false, player->speedLevel, player->sprite->getPosition(),
				player->steepSlideAttackSword.getPosition(), 
				player->sprite->getRotation() );
			break;
		case Actor::Action::WALLATTACK:
			QueueAttack( PlayerAttack::FAIR, player->facingRight, 
				false, player->speedLevel, player->sprite->getPosition(),
				player->wallAttackSword.getPosition(), 0 );
			break;
		}
	}

	UpdateSprite();

	if( dead && launcher->GetActiveCount() == 0 )
	{
		owner->RemoveEnemy( this );
	}
}

void Copycat::UpdateSprite()
{	
	if( !dying && !dead )
	{
		switch( action )
		{
		case NEUTRAL:
			//frame = 0;
			break;
		case THROW:
			//frame = 0;
			break;
		}

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

	launcher->UpdateSprites();
}

void Copycat::Draw( sf::RenderTarget *target )
{
	if( !(dead || dying ) )
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

		target->draw( *targetVA, ts_target->texture );
	}
	else if( dying )
	{
		target->draw( botDeathSprite );
		target->draw( topDeathSprite );
	}



}

void Copycat::DrawMinimap( sf::RenderTarget *target )
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

bool Copycat::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}

	//add in hitboxes here

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

	Actor *player = owner->player;
	int frameLimit = 0;
	list<CollisionBox*> *hitboxes = NULL;
	if( attackActive )
	{
		switch( currAttack.t )
		{
		case FAIR:
			hitboxes = player->fairHitboxes[currAttackFrame];
			break;
		case DAIR:
			hitboxes = player->dairHitboxes[currAttackFrame];
			break;
		case UAIR:
			hitboxes = player->uairHitboxes[currAttackFrame];
			break;
		case STANDN:
			hitboxes = player->standHitboxes[currAttackFrame];
			break;
		case CLIMBATTACK:
			hitboxes = player->steepClimbHitboxes[currAttackFrame];
			break;
		case SLIDEATTACK:
			hitboxes = player->steepSlideHitboxes[currAttackFrame];
			break;
		case WALLATTACK:
			hitboxes = player->wallHitboxes[currAttackFrame];
			break;
		}
	}

}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Copycat::PlayerHitMe()
{
	//if( action == INVISIBLE )
	//	return pair<bool,bool>(false,false);

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