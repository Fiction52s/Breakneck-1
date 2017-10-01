#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

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

Tree::Tree( Overgrowth *par,
	VertexArray &p_va, Tileset *p_ts, int index )
	:Enemy( par->owner, Enemy::OVERGROWTH_TREE, false, 5 ), 
	vaIndex( index ), frame( 0 ), 
	launcher( NULL ), ground( NULL ), edgeQuantity( 0 ),
	va( p_va ), ts( p_ts ), parent( par )
{
	maxFramesToLive = 60 * 4;
	framesToLive = maxFramesToLive;


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

	launcher = new Launcher( this, BasicBullet::GROWING_TREE, owner, 5, 5, V2d( 0, 0 ),
		normalize( V2d( 1, -1 ) ), 90, 300 );
	
	launcher->SetBulletSpeed( 10 );
	//launcher->SetGravity( V2d( 0, .5 ) );
	animFactor = 3;
}

void Tree::BulletHitTerrain( BasicBullet *b, 
	Edge *edge, sf::Vector2<double> &pos )
{
	b->launcher->DeactivateBullet( b );
	double q = edge->GetQuantity( pos );
	parent->ActivateTree( edge, q );
}

void Tree::BulletHitPlayer(BasicBullet *b )
{

}

void Tree::UpdatePrePhysics()
{
	launcher->UpdatePrePhysics();
	

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
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dead = true;
		}

		receivedHit = NULL;
	}
	
	
	//if( frame == 12 * animationFactor && slowCounter == 1 )
	//if( frame == 59 * parent->animationFactor - 1 && slowCounter == 1 )
	if( frame == 60 - 1 && slowCounter == 1 )
	{
		//cout << "FIRING" << endl;
		launcher->facingDir = normalize( owner->GetPlayer( 0 )->position
			- position );
		launcher->Fire();
	}
}

void Tree::UpdatePhysics()
{
	launcher->UpdatePhysics();

	PhysicsResponse();
}

void Tree::PhysicsResponse()
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
				owner->GetPlayer( 0 )->ConfirmHit( 5, 5, .8, 6 );


				if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
				{
					owner->GetPlayer( 0 )->velocity.y = 4;//.5;
				}
			}
		}

		

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		
	}
}

void Tree::ClearSprite()
{
	va[vaIndex*4+0].position = Vector2f( 0, 0 );
	va[vaIndex*4+1].position = Vector2f( 0, 0 );
	va[vaIndex*4+2].position = Vector2f( 0, 0 );
	va[vaIndex*4+3].position = Vector2f( 0, 0 );

	
}

void Tree::UpdateSprite()
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

void Tree::UpdatePostPhysics()
{
	launcher->UpdatePostPhysics();
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	if( slowCounter == slowMultiple )
	{
		
		++frame;	
		--framesToLive;

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
	

	if( deathFrame == 30 || framesToLive == 0 )
	{
		launcher->Reset();//might just delete bullets
		parent->DeactivateTree( this );
		//owner->RemoveEnemy( this );
		return;
	}

	UpdateSprite();
	launcher->UpdateSprites();
}

void Tree::Draw(sf::RenderTarget *target )
{
	//if( !dead )
	//{
	//	if( hasMonitor )
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

void Tree::DrawMinimap( sf::RenderTarget *target )
{
	/*CircleShape cs;
	cs.setRadius( 50 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );

	if( hasMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/
}

bool Tree::IHitPlayer( int index )
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

 pair<bool, bool> Tree::PlayerHitMe( int index )
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

bool Tree::PlayerSlowingMe()
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

void Tree::DebugDraw(sf::RenderTarget *target)
{
	hitBody.DebugDraw( target );
	hurtBody.DebugDraw( target );
}

void Tree::UpdateHitboxes()
{
	hurtBody.globalPosition = position;// + gn * 8.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;// + gn * 8.0;
	hitBody.globalAngle = 0;
}


void Tree::SaveEnemyState()
{
}

void Tree::LoadEnemyState()
{
}

void Tree::ResetEnemy()
{

	/*dead = false;
	frame = 0;
	deathFrame = 0;*/
}

void Tree::HandleEntrant( QuadTreeEntrant *qte )
{
}

void Tree::SetParams( Edge *p_ground, 
			double p_edgeQuantity )
{
	receivedHit = NULL;
	ground = p_ground;
	launcher->facingDir = ground->Normal();
	edgeQuantity = p_edgeQuantity;
	frame = 0;

	framesToLive = maxFramesToLive;

	V2d gn = ground->Normal();

	V2d gPoint = ground->GetPoint( edgeQuantity );

	double hh = 64.0 / 2.0;
	double hw = 64.0 / 2.0;
	V2d centerPos = gPoint + gn * hh;
	V2d along = normalize( ground->v1 - ground->v0 );

	
	double ang = atan2( along.y, along.x );
			
	double cs = cos( ang );
	double sn = sin( ang );

	Vector2f centerP( centerPos.x, centerPos.y );

	position = centerPos;
	//position = gPoint + gn * height / 2.0;
	//position = gPoint + gn * hh;

	angle = atan2( gn.x, -gn.y );


	Vector2f topLeft( (-hw) * cs - (-hh) * sn, (-hw) * sn + (-hh) * cs );
	Vector2f topRight( (hw) * cs - (-hh) * sn, (hw) * sn + (-hh) * cs );
	Vector2f botRight( (hw) * cs - (hh) * sn, (hw) * sn + (hh) * cs );
	Vector2f botLeft( (-hw) * cs - (hh) * sn, (-hw) * sn + (hh) * cs );

	va[vaIndex*4+0].position = centerP + topLeft;
	va[vaIndex*4+1].position = centerP + topRight;
	va[vaIndex*4+2].position = centerP + botRight;
	va[vaIndex*4+3].position = centerP + botLeft;

	UpdateSprite();

	launcher->position = position;
	//launcher->Reset();
}

void Overgrowth::InitTrees()
{
	for( int i = 0; i < MAX_TREES; ++i )
	{
		Tree *tree = new Tree( this, treeVA, ts, i );
		AddToList( tree, inactiveTrees );
	}
}



void Overgrowth::AddToList( Tree *tree, Tree *&list )
{
	tree->prev = NULL;
	tree->next = list;
	if( list != NULL )
	{
		list->prev = tree;
	}
	list = tree;
}

Overgrowth::Overgrowth( GameSession *owner, bool p_hasMonitor, Edge *g, double q, double speed,int wait )
		:Enemy( owner, EnemyType::OVERGROWTH, p_hasMonitor, 5 ), treeVA( sf::Quads, MAX_TREES * 4 )
{

	origGround = g;
	origQuantity = q;

	animationFactor = 3;
	activeTrees = NULL;
	inactiveTrees = NULL;
	

	initHealth = 60;
	health = initHealth;

	double width = 64; //112;
	double height = 64;

	//ts = owner->GetTileset( "basicturret_112x64.png", width, height );
	ts = owner->GetTileset( "curveturret_64x64.png", width, height );
	InitTrees();
	V2d gPoint = g->GetPoint( q );

	

	dead = false;

	double size = max( width, height );

//	deathPartingSpeed = .4;

	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );

	Tree *t = ActivateTree( g, q );
	t->launcher->facingDir = g->Normal();
}

void Overgrowth::HandleEntrant( QuadTreeEntrant *qte )
{
}

void Overgrowth::ResetEnemy()
{
	dead = false;

	
	//frame = 0;
//	deathFrame = 0;

	while( activeTrees != NULL )
	{
		DeactivateTree( activeTrees );
	}

	Tree *curr = inactiveTrees;
	while( curr != NULL )
	{
		curr->launcher->Reset();
		curr = (Tree*)curr->next;
	}
	ActivateTree( origGround, origQuantity );
}

bool Overgrowth::IHitPlayer( int index )
{
	return false;
}

std::pair<bool,bool> Overgrowth::PlayerHitMe( int index )
{
	return pair<bool,bool>(false,false);
}

bool Overgrowth::PlayerSlowingMe()
{
	return false;
}

void Overgrowth::UpdatePrePhysics()
{
	Tree *curr = activeTrees;
	Tree *temp;
	//int count = 0;
	while( curr != NULL )
	{
		temp = (Tree*)curr->next;
		//++count;
		curr->UpdatePrePhysics();
		curr = temp;
	}
	//cout << "count: " << count << endl;
}

void Overgrowth::UpdatePhysics()
{
	Tree *curr = activeTrees;
	Tree *temp;
	while( curr != NULL )
	{
		temp = (Tree*)curr->next;
		curr->UpdatePhysics();
		curr = temp;
	}
}

void Overgrowth::DeactivateTree( Tree *tree )
{
	//cout << "deactivating tree" << endl;
	assert( activeTrees != NULL );
	
	//size == 1
	if( activeTrees->next == NULL )
	{
		AddToList( tree, inactiveTrees  );
		activeTrees = NULL;
	}
	//size is greater
	else
	{
		//end
		if( tree->next == NULL )
		{
			tree->prev->next = NULL;
			AddToList( tree, inactiveTrees  );
		}
		//start
		else if( tree->prev == NULL )
		{
			Tree *temp = (Tree*)tree->next;
			tree->next->prev = NULL;
			AddToList( tree, inactiveTrees );
			activeTrees = temp;
		}
		//middle
		else
		{
			tree->next->prev = tree->prev;
			tree->prev->next = tree->next;
			AddToList( tree, inactiveTrees  );
		}
	}

	tree->ClearSprite();
	//tree->launcher->

	tree->active = false;
}

Tree * Overgrowth::ActivateTree( Edge *g, double q )
{
	if( inactiveTrees == NULL )
	{
		//assert( false );
		return NULL;//RanOutOfBullets();
	}
	else
	{
		Tree *temp = (Tree*)inactiveTrees->next;
		AddToList( inactiveTrees, activeTrees );
		//cout << "activeBullets: " << activeBullets << endl;
		inactiveTrees = temp;
		if( inactiveTrees != NULL )
			inactiveTrees->prev = NULL;

		activeTrees->SetParams( g, q );

		return activeTrees;
	}
}

void Overgrowth::PhysicsResponse()
{
	
}

void Overgrowth::UpdatePostPhysics()
{
	Tree *curr = activeTrees;
	Tree *temp;
	while( curr != NULL )
	{
		temp = (Tree*)curr->next;
		curr->UpdatePostPhysics();
		curr = temp;
	}
}

//incorrect currently
int Overgrowth::NumTotalBullets()
{
	
	return MAX_TREES * 5;
}

void Overgrowth::Draw(sf::RenderTarget *target )
{
	target->draw( treeVA, ts->texture );
}

void Overgrowth::DrawMinimap( sf::RenderTarget *target )
{
}

void Overgrowth::UpdateSprite()
{
}

void Overgrowth::DebugDraw(sf::RenderTarget *target)
{
}

void Overgrowth::UpdateHitboxes()
{
}


void Overgrowth::SaveEnemyState()
{
}

void Overgrowth::LoadEnemyState()
{
}
