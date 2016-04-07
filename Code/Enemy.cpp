#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Zone.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Launcher::Launcher( GameSession *owner, int numBullets )
{
	inactiveBullets = new BasicBullet;
	for( int i = 1; i < numBullets; ++i )
	{
		BasicBullet * temp = new BasicBullet;
		temp->next = inactiveBullets;
		inactiveBullets->prev = temp;
		inactiveBullets = temp;
	}

}

void Launcher::Reset()
{
	while( activeBullets != NULL )
	{
		DeactivateBullet( activeBullets );
	}
}

void Launcher::AddToList( BasicBullet *b, BasicBullet *list )
{
	b->prev = NULL;
	b->next = list;
	if( list != NULL )
	{
		list->prev = b;
	}
	list = b;
}

void Launcher::RanOutOfBullets()
{
	cout << "ran out of bullets!" << endl;
}

void Launcher::ActivateBullet( )
{
	if( inactiveBullets == NULL )
	{
		RanOutOfBullets();
		return;
	}
	else
	{
		BasicBullet *temp = inactiveBullets->next;
		AddToList( inactiveBullets, activeBullets );
		inactiveBullets = temp;
		inactiveBullets->prev = NULL;
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
			b->next->prev = NULL;
			AddToList( b, inactiveBullets );
		}
		//middle
		else
		{
			b->next->prev = b->prev;
			b->prev->next = b->next;
			AddToList( b, inactiveBullets );
		}
	}
}

BasicBullet::BasicBullet()
{

}

void BasicBullet::UpdatePrePhysics()
{

}

void BasicBullet::UpdatePhysics()
{
	V2d movement = velocity / NUM_STEPS / (double)slowMultiple;

	double movementLen = length( movement );
	V2d moveDir = normalize( movement );
	double move = 0;
	while( movementLen > 0 )
	{
		if( movementLen > hitBody.rw )
		{
			movementLen -= hitBody.rw;
			move = hitBody.rw;
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
	}
}

bool BasicBullet::HitTerrain()
{

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

	Contact *c = owner->coll.collideEdge( position + tempVel, physBody, e, tempVel, V2d( 0, 0 ) );

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
	VertexArray &VA = *va;
	IntRect ir = ts->GetSubRect( (maxFramesToLive - framesToLive) % 5 );
	Vector2f dims = Vector2f( ir.width / 2, ir.height / 2 );

	Vector2f center( position.x, position.y );
	Vector2f topLeft = center - dims;
	Vector2f topRight = center + Vector2f( dims.x, -dims.y );
	Vector2f botRight = center + dims;
	Vector2f botLeft = center + Vector2f( -dims.x, dims.y );


	VA[index*4+0].position = transform.transformPoint( topLeft );
	VA[index*4+1].position = transform.transformPoint( topRight );
	VA[index*4+2].position = transform.transformPoint( botRight );
	VA[index*4+3].position = transform.transformPoint( botLeft );

	VA[index*4+0].texCoords = Vector2f( ir.left, ir.top );
	VA[index*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
	VA[index*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
	VA[index*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );
}

Enemy::Enemy( GameSession *own, EnemyType t )
	:owner( own ), prev( NULL ), next( NULL ), spawned( false ), slowMultiple( 1 ), slowCounter( 1 ),
	spawnedByClone( false ), type( t ),zone( NULL ), monitor( NULL ), dead( false )
{

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




