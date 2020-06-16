#include "Enemy.h"
#include <iostream>
#include "Session.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Movable::Movable()
{
	slowMultiple = 1;
	slowCounter = 1;
	collideWithTerrain = false;
	collideWithPlayer = false;
	sess = Session::GetSession();

	//12
	//bounceCount = 0;
	/*hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = rad;
	hurtBody.rh = rad;

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
	physBody.rh = rad;*/

	//ResetSprite();
}
	
void Movable::HandleEntrant( QuadTreeEntrant *qte )
{
	Edge *e = (Edge*)qte;

	Contact *c = sess->collider.collideEdge( position + tempVel, physBody, e, tempVel, V2d( 0, 0 ) );

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

void Movable::UpdatePrePhysics()
{
	//cout << "vel: " << velocity.x << ", " << velocity.y << endl;
	//cout << "pos: " << position.x << ", " << position.y << endl;
}

void Movable::Reset( sf::Vector2<double> &pos )
{
	position = pos;
	velocity = V2d( 0, 0 );
	slowMultiple = 1;
	slowCounter = 1;
}
void Movable::UpdatePhysics()
{
	V2d movement = velocity / NUM_STEPS / (double)slowMultiple;
	//cout << "movement: " << movement.x << ", " << movement.y << endl;
	double movementLen = length( movement );
	V2d moveDir = normalize( movement );
	double move = 0;

	do
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

		if( move != 0 )
		{
			bool hit = ResolvePhysics( moveDir * move );
			if( hit )
			{
				HitTerrain();
				break;
			}
		}

		hitBody.globalPosition = position;

		Actor *player = sess->GetPlayer( 0 );
		if( player->hurtBody.Intersects( hitBody ) )
		{	
			HitPlayer();
			break;
		}
	}
	while( movementLen > .00001 );
}

void Movable::IncrementFrame()
{

}

void Movable::UpdatePostPhysics()
{
	if( slowCounter == slowMultiple )
	{	
		IncrementFrame();
		slowCounter = 1;
	}
	else
	{
		slowCounter++;
	}
}

bool Movable::ResolvePhysics( sf::Vector2<double> vel )
{
	if( collideWithTerrain )
	{
		Rect<double> oldR( position.x - physBody.rw, position.y - physBody.rw, 
			2 * physBody.rw, 2 * physBody.rw );

		position += vel;
		//cout << "new pos: " << position.x << ", " << position.y << endl;
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


		sess->terrainTree->Query( this, r );

		return col;
	}
	else
	{
		position += vel;
		return false;
	}
}

void Movable::HitTerrain()
{
}

void Movable::HitPlayer()
{
}