#include "Enemy.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

Bullet::Bullet( int indexVA, Launcher *launcher, double rad )
{
	//12
	//bounceCount = 0;
	hurtBody.isCircle = true;
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
	physBody.rh = rad;

	//ResetSprite();
}
	
void Bullet::HandleEntrant( QuadTreeEntrant *qte )
{
}
void Bullet::UpdatePrePhysics()
{
}
void Bullet::Reset( sf::Vector2<double> &pos, sf::Vector2<double> &vel )
{
	position = pos;
	velocity = vel;
	framesToLive = launcher->maxFramesToLive;
	slowMultiple = 1;
	slowCounter = 1;

	
	VertexArray &bva = *(launcher->owner->bigBulletVA);
	bva[index*4+0].position = Vector2f( 0, 0 );
	bva[index*4+1].position = Vector2f( 0, 0 );
	bva[index*4+2].position = Vector2f( 0, 0 );
	bva[index*4+3].position = Vector2f( 0, 0 );
	//transform.
}
void Bullet::UpdatePhysics()
{
}
void Bullet::UpdateSprite()
{
}
void Bullet::UpdatePostPhysics()
{
}
void Bullet::ResetSprite()
{
}

bool Bullet::ResolvePhysics( sf::Vector2<double> vel )
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


	if( launcher->interactWithTerrain )
		launcher->owner->terrainTree->Query( this, r );

	return col;
}

bool Bullet::HitTerrain()
{
	return false;
}
void Bullet::HitPlayer()
{
}






//void BasicBullet::Reset( V2d &pos, V2d &vel )
//{
//	//gravTowardsPlayer = false;
//	position = pos;
//	velocity = vel;
//	framesToLive = launcher->maxFramesToLive;
//	slowMultiple = 1;
//	slowCounter = 1;
//	bounceCount = 0;
//
//	VertexArray &bva = *(launcher->owner->bigBulletVA);
//	bva[index*4+0].position = Vector2f( 0, 0 );
//	bva[index*4+1].position = Vector2f( 0, 0 );
//	bva[index*4+2].position = Vector2f( 0, 0 );
//	bva[index*4+3].position = Vector2f( 0, 0 );
//	//transform.
//}
//
//BasicBullet::BasicBullet( int indexVA, Launcher *launch )
//	:index( indexVA ), launcher( launch ), next( NULL ), prev( NULL )
//{
//	//framesToLive = maxFram
//	double rad = 12;
//	bounceCount = 0;
//	/*hurtBody.isCircle = true;
//	hurtBody.globalAngle = 0;
//	hurtBody.offset.x = 0;
//	hurtBody.offset.y = 0;
//	hurtBody.rw = rad;
//	hurtBody.rh = rad;*/
//
//	hitBody.type = CollisionBox::Hit;
//	hitBody.isCircle = true;
//	hitBody.globalAngle = 0;
//	hitBody.offset.x = 0;
//	hitBody.offset.y = 0;
//	hitBody.rw = rad;
//	hitBody.rh = rad;
//
//	physBody.type = CollisionBox::Physics;
//	physBody.isCircle = true;
//	physBody.globalAngle = 0;
//	physBody.offset.x = 0;
//	physBody.offset.y = 0;
//	physBody.rw = rad;
//	physBody.rh = rad;
//
//	//ResetSprite();
//}
//
//void BasicBullet::ResetSprite()
//{
//	VertexArray &bva = *(launcher->owner->bigBulletVA);
//	bva[index*4+0].position = Vector2f( 0, 0 );
//	bva[index*4+1].position = Vector2f( 0, 0 );
//	bva[index*4+2].position = Vector2f( 0, 0 );
//	bva[index*4+3].position = Vector2f( 0, 0 );
//}
//
//void BasicBullet::UpdatePrePhysics()
//{
//	velocity += gravity / (double)slowMultiple;
//	//if( gravTowardsPlayer )
//	//{
//	//	double len = gravity.y;//length( gravity );
//	//	V2d diff = launcher->owner->player.position - position;
//	//	int t = 100;
//	//	diff += V2d( (rand() % t) - t / 2, (rand() % t) - t / 2);
//	//	V2d towards = normalize( diff );
//	//	V2d other( towards.y, -towards.x );
//
//	//	double off = dot( velocity, other );
//	//	double on = dot( velocity, towards );
//	//	//off *= .99;//1.0/2.0;//7.0 / 8.0;
//
//	//	velocity = on * towards + off * other;
//	//	
//	//	//double to = 
//
//	//	velocity += (towards * len ) / (double) slowMultiple;
//	//}
//	//else
//	//{
//	//	
//	//}
//}
//
//void BasicBullet::UpdatePostPhysics()
//{
//	if( slowCounter == slowMultiple )
//	{
//		
//		framesToLive--;
//		//++frame;		
//	
//		slowCounter = 1;
//	
//		//if( dead )
//		//{
//		//	//cout << "DEAD" << endl;
//		//	deathFrame++;
//		//}
//
//	}
//	else
//	{
//		slowCounter++;
//	}
//	
//
//	if( framesToLive == 0 )
//	{
//		//cout << "time out!" << endl;
//		//explode
//		launcher->DeactivateBullet( this );
//		//parent->DeactivateTree( this );
//		//owner->RemoveEnemy( this );
//		//return;
//	}
//}
//
//void BasicBullet::UpdatePhysics()
//{
//	V2d movement = velocity / NUM_STEPS / (double)slowMultiple;
//
//	double movementLen = length( movement );
//	V2d moveDir = normalize( movement );
//	double move = 0;
//
//	do
//	{
//		//cout << "loop: " << movementLen << endl;
//		if( movementLen > physBody.rw )
//		{
//			movementLen -= physBody.rw;
//			move = physBody.rw;
//		}
//		else
//		{
//			move = movementLen;
//			movementLen = 0;
//		}
//
//		if( move != 0 )
//		{
//			bool hit = ResolvePhysics( moveDir * move );
//			if( hit )
//			{
//				HitTerrain();
//				break;
//			}
//		}
//
//		hitBody.globalPosition = position;
//
//		Actor &player = launcher->owner->player;
//		if( player.hurtBody.Intersects( hitBody ) )
//		{	
//			//cout << "hit??" << endl;
//			HitPlayer();
//			break;
//		}
//	}
//	while( movementLen > 0 );
//}
//
//bool BasicBullet::HitTerrain()
//{
//	launcher->handler->BulletHitTerrain( this,
//		minContact.edge, minContact.position );
//	return true;
//}
//
//void BasicBullet::HitPlayer()
//{
//	launcher->handler->BulletHitPlayer( this );
//	//launcher->DeactivateBullet( this );
//}
//
//bool BasicBullet::ResolvePhysics( V2d vel )
//{
//	Rect<double> oldR( position.x - physBody.rw, position.y - physBody.rw, 
//		2 * physBody.rw, 2 * physBody.rw );
//
//	position += vel;
//
//	Rect<double> newR( position.x - physBody.rw, position.y - physBody.rw, 
//		2 * physBody.rw, 2 * physBody.rw );
//
//	double oldRight = oldR.left + oldR.width;
//	double right = newR.left + newR.width;
//
//	double oldBottom = oldR.top + oldR.height;
//	double bottom = newR.top + newR.height;
//
//	double maxRight = max( right, oldRight );
//	double maxBottom = max( oldBottom, bottom );
//	double minLeft = min( oldR.left, newR.left );
//	double minTop = min( oldR.top, newR.top );
//
//
//	double ex = 1;
//	Rect<double> r( minLeft - ex, minTop - ex, 
//		(maxRight - minLeft) + ex * 2, (maxBottom - minTop) + ex * 2 );
//	//Rect<double> realRect( min( oldR.left, r.left ),
//	//	min( oldR.top, r.top ));
//
//	minContact.collisionPriority = 1000000;
//
//	col = false;
//
//	tempVel = vel;
//	minContact.edge = NULL;
//
//	//queryMode = "resolve";
////	Query( this, owner->testTree, r );
//	//queryBullet = bullet;
//
//
//	if( launcher->interactWithTerrain )
//		launcher->owner->terrainTree->Query( this, r );
//
//	return col;
//}
//
//void BasicBullet::HandleEntrant( QuadTreeEntrant *qte )
//{
//	Edge *e = (Edge*)qte;
//
//	Contact *c = launcher->owner->coll.collideEdge( position + tempVel, physBody, e, tempVel, V2d( 0, 0 ) );
//
//	if( c != NULL )
//	{
//		if( !col )
//		{
//			minContact = *c;
//			col = true;
//		}
//		else if( c->collisionPriority < minContact.collisionPriority )
//		{
//			minContact = *c;
//		}
//	}
//}
//
//void BasicBullet::UpdateSprite()
//{
//	
//	VertexArray &VA = *(launcher->owner->bigBulletVA);
//	//IntRect ir = ts->GetSubRect( (maxFramesToLive - framesToLive) % 5 );
//	Vector2f dims( 12, 12 );
//	//Vector2f dims = Vector2f( ir.width / 2, ir.height / 2 );
//
//	Vector2f center( position.x, position.y );
//	Vector2f topLeft = center - dims;
//	Vector2f topRight = center + Vector2f( dims.x, -dims.y );
//	Vector2f botRight = center + dims;
//	Vector2f botLeft = center + Vector2f( -dims.x, dims.y );
//
//
//	VA[index*4+0].position = transform.transformPoint( topLeft );
//	VA[index*4+1].position = transform.transformPoint( topRight );
//	VA[index*4+2].position = transform.transformPoint( botRight );
//	VA[index*4+3].position = transform.transformPoint( botLeft );
//
//	/*VA[index*4+0].texCoords = Vector2f( ir.left, ir.top );
//	VA[index*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
//	VA[index*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
//	VA[index*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );*/
//	VA[index*4+0].color = Color::Red;
//	VA[index*4+1].color = Color::Red;
//	VA[index*4+2].color = Color::Red;
//	VA[index*4+3].color = Color::Red;
//}