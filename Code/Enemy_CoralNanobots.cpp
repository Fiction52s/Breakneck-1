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

CoralBlock::CoralBlock( CoralNanobots *par,
	VertexArray &p_va, Tileset *p_ts, int index )
	:Enemy( par->owner, Enemy::CORAL_BLOCK ), 
	vaIndex( index ), frame( 0 ),
	va( p_va ), ts( p_ts ), parent( par )
{
	//maxFramesToLive = 60 * 4;
	//framesToLive = maxFramesToLive;


	initHealth = 60;
	health = initHealth;

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



	frame = 0;
	deathFrame = 0;

	dead = false;

	double size = max( 64, 64 );

	deathPartingSpeed = .4;

	Transform t;
	t.rotate( angle / PI * 180 );
	Vector2f newPoint = t.transformPoint( Vector2f( -1, -1 ) );
	deathVector = V2d( newPoint.x, newPoint.y );

	ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	bloodSprite.setTexture( *ts_testBlood->texture );

	//spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );

	receivedHit = NULL;

	
	
	
	//launcher->SetGravity( V2d( 0, .5 ) );
	animFactor = 3;

	
}

void CoralBlock::UpdatePrePhysics()
{
	

	if( frame == 60 )//26 * parent->animationFactor )
	{
		frame = 0;
	}

	if( !dead && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			AttemptSpawnMonitor();
			dead = true;
		}

		receivedHit = NULL;
	}
	
	
	//if( frame == 12 * animationFactor && slowCounter == 1 )
	//if( frame == 59 * parent->animationFactor - 1 && slowCounter == 1 )
	if( frame == 60 - 1 && slowCounter == 1 )
	{
		//cout << "FIRING" << endl;
		//launcher->facingDir = normalize( owner->player.position
		//	- position );
		//launcher->Fire();
	}
}

void CoralBlock::UpdatePhysics()
{
	//launcher->UpdatePhysics();

	PhysicsResponse();
}

void CoralBlock::PhysicsResponse()
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
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		
	}
}

void CoralBlock::ClearSprite()
{
	va[vaIndex*4+0].position = Vector2f( 0, 0 );
	va[vaIndex*4+1].position = Vector2f( 0, 0 );
	va[vaIndex*4+2].position = Vector2f( 0, 0 );
	va[vaIndex*4+3].position = Vector2f( 0, 0 );

	
}

void CoralBlock::UpdateSprite()
{
	IntRect subRect = ts->GetSubRect( 0 );//frame / animFactor );
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
}

void CoralBlock::UpdatePostPhysics()
{
	//launcher->UpdatePostPhysics();
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	if( slowCounter == slowMultiple )
	{
		
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
	

	if( deathFrame == 30 )//|| framesToLive == 0 )
	{
		//launcher->Reset();//might just delete bullets
		parent->DeactivateBlock( this );
		//owner->RemoveEnemy( this );
		return;
	}

	UpdateSprite();
	//launcher->UpdateSprites();
}

void CoralBlock::Draw(sf::RenderTarget *target )
{
	//if( !dead )
	//{
	//	if( monitor != NULL )
	//	{
	//		//owner->AddEnemy( monitor );
	//		CircleShape cs;
	//		cs.setRadius( 40 );
	//		cs.setFillColor( COLOR_BLUE );
	//		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	//		cs.setPosition( position.x, position.y );
	//		target->draw( cs );
	//	}
	//	//target->draw( sprite );
	//}
	//else
	//{
	//	target->draw( botDeathSprite );

	//	if( deathFrame / 3 < 6 )
	//	{
	//		bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
	//		bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
	//		bloodSprite.setPosition( position.x, position.y );
	//		bloodSprite.setScale( 2, 2 );
	//		target->draw( bloodSprite );
	//	}
	//	
	//	target->draw( topDeathSprite );
	//}
	

	/*if( activeBullets != NULL )
	{
		target->draw( bulletVA, ts_bullet->texture );
	}*/
	
}

void CoralBlock::DrawMinimap( sf::RenderTarget *target )
{
	/*CircleShape cs;
	cs.setRadius( 50 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );

	if( monitor != NULL )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/
}

bool CoralBlock::IHitPlayer()
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

 pair<bool, bool> CoralBlock::PlayerHitMe()
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
			receivedHit = player.currHitboxInfo;
			return pair<bool, bool>(true,false);
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

bool CoralBlock::PlayerSlowingMe()
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

void CoralBlock::DebugDraw(sf::RenderTarget *target)
{
	hitBody.DebugDraw( target );
	hurtBody.DebugDraw( target );
}

void CoralBlock::UpdateHitboxes()
{
	hurtBody.globalPosition = position;// + gn * 8.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;// + gn * 8.0;
	hitBody.globalAngle = 0;
}


void CoralBlock::SaveEnemyState()
{
}

void CoralBlock::LoadEnemyState()
{
}

void CoralBlock::ResetEnemy()
{

	/*dead = false;
	frame = 0;
	deathFrame = 0;*/
}

void CoralBlock::HandleEntrant( QuadTreeEntrant *qte )
{
	//actually use this for telling when to stop 
	//expanding in a certain direction
}

void CoralBlock::SetParams( sf::Vector2<double> &pos,
		sf::Vector2<double> &dir )
{
	position = pos;


	receivedHit = NULL;

	
	
	frame = 0;

	//framesToLive = maxFramesToLive;

	Vector2f p( position.x, position.y );

	va[vaIndex*4+0].position = p + Vector2f( -parent->blockSizeX/2, -parent->blockSizeY/2 );
	va[vaIndex*4+1].position = p + Vector2f( parent->blockSizeX/2, -parent->blockSizeY/2 );
	va[vaIndex*4+2].position = p + Vector2f( parent->blockSizeX/2, parent->blockSizeY/2 );
	va[vaIndex*4+3].position = p + Vector2f( -parent->blockSizeX/2, parent->blockSizeY/2 );

	UpdateSprite();

	//launcher->position = position;
	//launcher->Reset();
}

void CoralNanobots::InitBlocks()
{
	for( int i = 0; i < MAX_BLOCKS; ++i )
	{
		CoralBlock *block = new CoralBlock( this, blockVA, ts, i );
		AddToList( block, inactiveBlocks );
	}
}

void CoralNanobots::AddToList( CoralBlock *block, 
	CoralBlock *&list )
{
	block->prev = NULL;
	block->next = list;
	if( list != NULL )
	{
		list->prev = block;
	}
	list = block;
}

CoralNanobots::CoralNanobots( GameSession *owner, sf::Vector2i &pos, double speed )
		:Enemy( owner, EnemyType::OVERGROWTH ), blockVA( sf::Quads, MAX_BLOCKS * 4 )
{
	origPosition = V2d( pos.x, pos.y );

	animationFactor = 3;
	activeBlocks = NULL;
	inactiveBlocks = NULL;
	

	initHealth = 60;
	health = initHealth;

	double width = 64; //112;
	double height = 64;
	blockSizeX = width;
	blockSizeY = height;
	//ts = owner->GetTileset( "basicturret_112x64.png", width, height );
	ts = owner->GetTileset( "curveturret_64x64.png", width, height );
	InitBlocks();
	

	

	dead = false;

	double size = max( width, height );

//	deathPartingSpeed = .4;

	//spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );
	spawnRect = sf::Rect<double>( pos.x - size / 2, pos.y - size / 2, size, size );

	CoralBlock *c = ActivateBlock( origPosition, V2d( 0, -1 ) );
	//t->launcher->facingDir = g->Normal();
}

void CoralNanobots::HandleEntrant( QuadTreeEntrant *qte )
{
}

void CoralNanobots::ResetEnemy()
{
	dead = false;

	
	//frame = 0;
//	deathFrame = 0;

	while( activeBlocks != NULL )
	{
		DeactivateBlock( activeBlocks );
	}

	/*Tree *curr = inactiveBlocks;
	while( curr != NULL )
	{
		curr->launcher->Reset();
		curr = (Tree*)curr->next;
	}*/
	ActivateBlock( origPosition, V2d( 0, -1 ) );
}

bool CoralNanobots::IHitPlayer()
{
	return false;
}

std::pair<bool,bool> CoralNanobots::PlayerHitMe()
{
	return pair<bool,bool>(false,false);
}

bool CoralNanobots::PlayerSlowingMe()
{
	return false;
}

void CoralNanobots::UpdatePrePhysics()
{
	CoralBlock *curr = activeBlocks;
	CoralBlock *temp;
	//int count = 0;
	while( curr != NULL )
	{
		temp = (CoralBlock*)curr->next;
		//++count;
		curr->UpdatePrePhysics();
		curr = temp;
	}
	//cout << "count: " << count << endl;
}

void CoralNanobots::UpdatePhysics()
{
	CoralBlock *curr = activeBlocks;
	CoralBlock *temp;
	while( curr != NULL )
	{
		temp = (CoralBlock*)curr->next;
		curr->UpdatePhysics();
		curr = temp;
	}
}

void CoralNanobots::DeactivateBlock( CoralBlock *block )
{
	//cout << "deactivating tree" << endl;
	assert( activeBlocks != NULL );
	
	//size == 1
	if( activeBlocks->next == NULL )
	{
		AddToList( block, inactiveBlocks );
		activeBlocks = NULL;
	}
	//size is greater
	else
	{
		//end
		if( block->next == NULL )
		{
			block->prev->next = NULL;
			AddToList( block, inactiveBlocks );
		}
		//start
		else if( block->prev == NULL )
		{
			CoralBlock *temp = (CoralBlock*)block->next;
			block->next->prev = NULL;
			AddToList( block, inactiveBlocks );
			activeBlocks = temp;
		}
		//middle
		else
		{
			block->next->prev = block->prev;
			block->prev->next = block->next;
			AddToList( block, inactiveBlocks  );
		}
	}

	block->ClearSprite();
	//tree->launcher->

	block->active = false;
}

CoralBlock * CoralNanobots:: ActivateBlock( 
		sf::Vector2<double> &pos,
		sf::Vector2<double> &dir )
{
	if( inactiveBlocks == NULL )
	{
		//assert( false );
		return NULL;//RanOutOfBullets();
	}
	else
	{
		CoralBlock *temp = (CoralBlock*)inactiveBlocks->next;
		AddToList( inactiveBlocks, activeBlocks );
		//cout << "activeBullets: " << activeBullets << endl;
		inactiveBlocks = temp;
		if( inactiveBlocks != NULL )
			inactiveBlocks->prev = NULL;

		activeBlocks->SetParams( pos, dir );

		return activeBlocks;
	}
}

void CoralNanobots::PhysicsResponse()
{
	
}

void CoralNanobots::UpdatePostPhysics()
{
	CoralBlock *curr = activeBlocks;
	CoralBlock *temp;
	while( curr != NULL )
	{
		temp = (CoralBlock*)curr->next;
		curr->UpdatePostPhysics();
		curr = temp;
	}
}

void CoralNanobots::Draw(sf::RenderTarget *target )
{
	target->draw( blockVA, ts->texture );
}

void CoralNanobots::DrawMinimap( sf::RenderTarget *target )
{
}

void CoralNanobots::UpdateSprite()
{
}

void CoralNanobots::DebugDraw(sf::RenderTarget *target)
{
}

void CoralNanobots::UpdateHitboxes()
{
}


void CoralNanobots::SaveEnemyState()
{
}

void CoralNanobots::LoadEnemyState()
{
}
