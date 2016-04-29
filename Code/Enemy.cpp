#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Zone.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Launcher::Launcher( LauncherEnemy *p_handler,
	GameSession *p_owner,
		int numTotalBullets,
		int bulletsPerShot,
		sf::Vector2<double> p_position,
		sf::Vector2<double> direction,
		double p_angleSpread )
		:totalBullets( numTotalBullets ), perShot( bulletsPerShot ),
		facingDir( direction ), angleSpread( p_angleSpread ), 
		position( p_position ), owner( p_owner ),handler(p_handler)

{

	//increment the global counter
	 //+= numTotalBullets;
	int startIndex = owner->totalNumberBullets;

	activeBullets = NULL;
	inactiveBullets = new BasicBullet( startIndex++, this );

	for( int i = 1; i < numTotalBullets; ++i )
	{
		BasicBullet * temp = new BasicBullet( startIndex++, this );
		temp->next = inactiveBullets;
		inactiveBullets->prev = temp;
		inactiveBullets = temp;
	}

	owner->totalNumberBullets = startIndex;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = .3;
	hitboxInfo->drainY = .3;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;
}

void Launcher::UpdatePrePhysics()
{
	BasicBullet *curr = activeBullets;
	BasicBullet *temp;
	while( curr != NULL )
	{
		temp = curr->next;
	//	cout << "updating bullet pre" << endl;
		curr->UpdatePrePhysics();
		curr = temp;
	}
}

void Launcher::UpdatePhysics()
{
	BasicBullet *curr = activeBullets;
	BasicBullet *temp;
	while( curr != NULL )
	{
		temp = curr->next;
	//	cout << "updating bullet physics" << endl;
		curr->UpdatePhysics();
		curr = temp;
	}
}



void Launcher::UpdateSprites()
{
	BasicBullet *curr = activeBullets;
	while( curr != NULL )
	{
		//cout << "updating bullet sprite" << endl;
		curr->UpdateSprite();
		curr = curr->next;
	}
}

void Launcher::SetBulletSpeed( double speed )
{
	bulletSpeed = speed;
}

void Launcher::Fire()
{
	
	for( int i = 0; i < perShot; ++i )
	{
		//cout << "trying to activate bullet" << endl;
		BasicBullet * b = ActivateBullet();
		//cout << "bullet done activating" << endl;
		if( b != NULL )
		{
			//cout << "FIRE" << endl;
			b->Reset( position, facingDir * bulletSpeed );
		}
	}
}

void Launcher::Reset()
{
	int x = 0;
	//cout << "resetting" << endl;
	while( activeBullets != NULL )
	{
		++x;
		//cout << "deact" << endl;
		DeactivateBullet( activeBullets );
	}
	//cout << "reset " << x << " bullets" << endl;
}

void Launcher::UpdatePostPhysics()
{
	BasicBullet *curr = activeBullets;
	BasicBullet *temp;
	while( curr != NULL )
	{
		temp = curr->next;
		curr->UpdatePostPhysics();
		curr = temp;
	}
}

void Launcher::AddToList( BasicBullet *b, BasicBullet *&list )
{
	b->prev = NULL;
	b->next = list;
	if( list != NULL )
	{
		list->prev = b;
	}
	//cout << "list is: " << b << endl;
	list = b;
}

//might deactivate the oldest one or something. for now just return null
BasicBullet * Launcher::RanOutOfBullets()
{
	cout << "ran out of bullets!" << endl;
	return NULL;
}

int Launcher::GetActiveCount()
{
	int count = 0;
	BasicBullet *curr = activeBullets;
	while( curr != NULL )
	{
		++count;
		curr = curr->next;
	}

	return count;
}

void BasicBullet::Reset( V2d &pos, V2d &vel )
{
	position = pos;
	velocity = vel;
	framesToLive = maxFramesToLive;
	slowMultiple = 1;
	slowCounter = 1;

	VertexArray &bva = *(launcher->owner->bigBulletVA);
	bva[index*4+0].position = Vector2f( 0, 0 );
	bva[index*4+1].position = Vector2f( 0, 0 );
	bva[index*4+2].position = Vector2f( 0, 0 );
	bva[index*4+3].position = Vector2f( 0, 0 );
	//transform.
}

BasicBullet * Launcher::ActivateBullet( )
{
	if( inactiveBullets == NULL )
	{

		return RanOutOfBullets();
	}
	else
	{
		//cout << "b" << endl;
		BasicBullet *temp = inactiveBullets->next;
		AddToList( inactiveBullets, activeBullets );
		//cout << "activeBullets: " << activeBullets << endl;
		inactiveBullets = temp;
		if( inactiveBullets != NULL )
			inactiveBullets->prev = NULL;
		//cout << "c" << endl;
		return activeBullets;
	}
}

void Launcher::DeactivateBullet( BasicBullet *b )
{
	assert( activeBullets != NULL );
	
	//size == 1
	if( activeBullets->next == NULL )
	{
		AddToList( b, inactiveBullets );
		activeBullets = NULL;
	}
	//size is greater
	else
	{
		//end
		if( b->next == NULL )
		{
			b->prev->next = NULL;
			AddToList( b, inactiveBullets );
		}
		//start
		else if( b->prev == NULL )
		{
			BasicBullet *temp = b->next;
			b->next->prev = NULL;
			AddToList( b, inactiveBullets );
			activeBullets = temp;
		}
		//middle
		else
		{
			b->next->prev = b->prev;
			b->prev->next = b->next;
			AddToList( b, inactiveBullets );
		}
	}

	b->ResetSprite();
}

void Launcher::SetGravity( sf::Vector2<double> &grav )
{
	BasicBullet *curr = activeBullets;
	while( curr != NULL )
	{
		curr->gravity = grav;
		curr = curr->next;
	}
	curr = inactiveBullets;
	while( curr != NULL )
	{
		curr->gravity = grav;
		curr = curr->next;
	}
}

BasicBullet::BasicBullet( int indexVA, Launcher *launch )
	:index( indexVA ), launcher( launch ), next( NULL ), prev( NULL )
{
	double rad = 12;
	/*hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = rad;
	hurtBody.rh = rad;*/

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = rad;
	hitBody.rh = rad;

	physBody.type = CollisionBox::Physics;
	physBody.isCircle = true;
	physBody.globalAngle = 0;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = rad;
	physBody.rh = rad;

	//ResetSprite();
}

void BasicBullet::ResetSprite()
{
	VertexArray &bva = *(launcher->owner->bigBulletVA);
	bva[index*4+0].position = Vector2f( 0, 0 );
	bva[index*4+1].position = Vector2f( 0, 0 );
	bva[index*4+2].position = Vector2f( 0, 0 );
	bva[index*4+3].position = Vector2f( 0, 0 );
}

void BasicBullet::UpdatePrePhysics()
{
	velocity += gravity / (double)slowMultiple;
}

void BasicBullet::UpdatePostPhysics()
{
	if( slowCounter == slowMultiple )
	{
		
		framesToLive--;
		//++frame;		
	
		slowCounter = 1;
	
		//if( dead )
		//{
		//	//cout << "DEAD" << endl;
		//	deathFrame++;
		//}

	}
	else
	{
		slowCounter++;
	}
	

	if( framesToLive == 0 )
	{
		//explode
		//launcher->DeactivateBullet( this );
		//parent->DeactivateTree( this );
		//owner->RemoveEnemy( this );
		//return;
	}
}

void BasicBullet::UpdatePhysics()
{
	V2d movement = velocity / NUM_STEPS / (double)slowMultiple;

	double movementLen = length( movement );
	V2d moveDir = normalize( movement );
	double move = 0;
	while( movementLen > 0 )
	{
		//cout << "loop: " << movementLen << endl;
		if( movementLen > physBody.rw )
		{
			movementLen -= physBody.rw;
			move = physBody.rw;
		}
		else
		{
			move = movementLen;
			movementLen = 0;
		}

		bool hit = ResolvePhysics( moveDir * move );
		if( hit )
		{
			HitTerrain();
			break;
		}

		hitBody.globalPosition = position;

		Actor &player = launcher->owner->player;
		if( player.hurtBody.Intersects( hitBody ) )
		{
			HitPlayer();
			break;
		}
	}
}

bool BasicBullet::HitTerrain()
{
	launcher->handler->BulletHitTerrain( this,
		minContact.edge, minContact.position );
	launcher->DeactivateBullet( this );
	return true;
}

void BasicBullet::HitPlayer()
{
	launcher->handler->BulletHitPlayer( this );
	launcher->DeactivateBullet( this );
}

bool BasicBullet::ResolvePhysics( V2d vel )
{
	Rect<double> oldR( position.x - physBody.rw, position.y - physBody.rw, 
		2 * physBody.rw, 2 * physBody.rw );

	position += vel;

	Rect<double> newR( position.x - physBody.rw, position.y - physBody.rw, 
		2 * physBody.rw, 2 * physBody.rw );

	double oldRight = oldR.left + oldR.width;
	double right = newR.left + newR.width;

	double oldBottom = oldR.top + oldR.height;
	double bottom = newR.top + newR.height;

	double maxRight = max( right, oldRight );
	double maxBottom = max( oldBottom, bottom );
	double minLeft = min( oldR.left, newR.left );
	double minTop = min( oldR.top, newR.top );


	double ex = 1;
	Rect<double> r( minLeft - ex, minTop - ex, 
		(maxRight - minLeft) + ex * 2, (maxBottom - minTop) + ex * 2 );
	//Rect<double> realRect( min( oldR.left, r.left ),
	//	min( oldR.top, r.top ));

	minContact.collisionPriority = 1000000;

	col = false;

	tempVel = vel;
	minContact.edge = NULL;

	//queryMode = "resolve";
//	Query( this, owner->testTree, r );
	//queryBullet = bullet;
	launcher->owner->terrainTree->Query( this, r );

	return col;
}

void BasicBullet::HandleEntrant( QuadTreeEntrant *qte )
{
	Edge *e = (Edge*)qte;

	Contact *c = launcher->owner->coll.collideEdge( position + tempVel, physBody, e, tempVel, V2d( 0, 0 ) );

	if( c != NULL )
	{
		if( !col )
		{
			minContact = *c;
			col = true;
		}
		else if( c->collisionPriority < minContact.collisionPriority )
		{
			minContact = *c;
		}
	}
}

void BasicBullet::UpdateSprite()
{
	
	VertexArray &VA = *(launcher->owner->bigBulletVA);
	//IntRect ir = ts->GetSubRect( (maxFramesToLive - framesToLive) % 5 );
	Vector2f dims( 12, 12 );
	//Vector2f dims = Vector2f( ir.width / 2, ir.height / 2 );

	Vector2f center( position.x, position.y );
	Vector2f topLeft = center - dims;
	Vector2f topRight = center + Vector2f( dims.x, -dims.y );
	Vector2f botRight = center + dims;
	Vector2f botLeft = center + Vector2f( -dims.x, dims.y );


	VA[index*4+0].position = transform.transformPoint( topLeft );
	VA[index*4+1].position = transform.transformPoint( topRight );
	VA[index*4+2].position = transform.transformPoint( botRight );
	VA[index*4+3].position = transform.transformPoint( botLeft );

	/*VA[index*4+0].texCoords = Vector2f( ir.left, ir.top );
	VA[index*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
	VA[index*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
	VA[index*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );*/
	VA[index*4+0].color = Color::Red;
	VA[index*4+1].color = Color::Red;
	VA[index*4+2].color = Color::Red;
	VA[index*4+3].color = Color::Red;
}

Enemy::Enemy( GameSession *own, EnemyType t )
	:owner( own ), prev( NULL ), next( NULL ), spawned( false ), slowMultiple( 1 ), slowCounter( 1 ),
	spawnedByClone( false ), type( t ),zone( NULL ), monitor( NULL ), dead( false )
{

}

int Enemy::NumTotalBullets()
{
	return 0;
}

void Enemy::AttemptSpawnMonitor()
{
	if( monitor != NULL )
	{
		if( !owner->player.CaptureMonitor( monitor ) )
		{
			cout << "adding monitor!" << endl;
			monitor->position = position;
			owner->AddEnemy( monitor );
		}
	}
}

void Enemy::Reset()
{
	slowMultiple = 1;
	slowCounter = 1;
	spawned = false;
	prev = NULL;
	next = NULL;
	spawnedByClone = false;

	ResetEnemy();

	//cout << "resetting enemy!" << endl;
}

void Enemy::HandleQuery( QuadTreeCollider * qtc )
{
	if( !spawned )
		qtc->HandleEntrant( this );
}

bool Enemy::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsBoxTouchingBox( spawnRect, r );//r.intersects( spawnRect );// 
}

void Enemy::SaveState()
{
	stored.next = next;
	stored.prev = prev;
	stored.receivedHit = receivedHit;
	stored.slowCounter = slowCounter;
	stored.slowMultiple = slowMultiple;
	stored.spawned = spawned;

	SaveEnemyState();
}

void Enemy::LoadState()
{
	next = stored.next;
	prev = stored.prev;
	receivedHit = stored.receivedHit;
	slowCounter = stored.slowCounter;
	slowMultiple = stored.slowMultiple;
	spawned = stored.spawned;

	LoadEnemyState();
}




