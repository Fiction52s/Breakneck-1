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

Crawler::Crawler( GameSession *owner, Edge *g, double q, bool cw, double s )
	:Enemy( owner, EnemyType::CRAWLER ), ground( g ), edgeQuantity( q ), clockwise( cw ), groundSpeed( s )
{
	initHealth = 60;
	health = initHealth;
	lastReverser = false;
	dead = false;
	deathFrame = 0;
	//ts_walk = owner->GetTileset( "crawlerwalk.png", 96, 64 );
	//ts_roll = owner->GetTileset( "crawlerroll.png", 96, 64 );
	attackFrame = -1;
	attackMult = 10;

	double height = 128;
	double width = 128;
	ts = owner->GetTileset( "crawler_128x128.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	roll = false;
	position = gPoint + ground->Normal() * height / 2.0;
	if( !clockwise )
	{
		groundSpeed = -groundSpeed;
	}

	receivedHit = NULL;

	double size = max( width, height );
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size/ 2, size, size );

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
	hitboxInfo->knockback = 0;

	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;
	physBody.isCircle = true;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 32;
	physBody.rh = 32;
	physBody.type = CollisionBox::BoxType::Physics;

	startGround = ground;
	startQuant = edgeQuantity;
	frame = 0;

	deathPartingSpeed = .4;

	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	ts_testBlood = owner->GetTileset( "fx_blood_1_256x256.png", 256, 256 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	ts_hitSpack = owner->GetTileset( "hit_spack_1_128x128.png", 128, 128 );
}

void Crawler::ResetEnemy()
{
	health = initHealth;
	attackFrame = -1;
	lastReverser = false;
	roll = false;
	ground = startGround;
	edgeQuantity = startQuant;
	V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	frame = 0;
	position = gPoint + ground->Normal() * 64.0 / 2.0;

	V2d gn = ground->Normal();
	if( gn.x > 0 )
		offset.x = physBody.rw;
	else if( gn.x < 0 )
		offset.x = -physBody.rw;
	if( gn.y > 0 )
		offset.y = physBody.rh;
	else if( gn.y < 0 )
		offset.y = -physBody.rh;

	//position = gPoint + offset;

	deathFrame = 0;
	dead = false;

	//----update the sprite
	double angle = 0;
	//position = gPoint + gn * 32.0;
	angle = atan2( gn.x, -gn.y );
		
	//sprite.setTexture( *ts_walk->texture );
	sprite.setTextureRect( ts->GetSubRect( frame / crawlAnimationFactor ) );
	V2d pp = ground->GetPoint( edgeQuantity );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( pp.x, pp.y );
	//----

	UpdateHitboxes();

}

void Crawler::HandleEntrant( QuadTreeEntrant *qte )
{
	//needs to be redone
	assert( queryMode != "" );

	if( queryMode == "resolve" )
	{
		Edge *e = (Edge*)qte;


		if( ground == e )
			return;

		if( ground != NULL && ground->edgeType == Edge::CLOSED_GATE )
		{
			Gate *g = (Gate*)ground->info;
			Edge *edgeA = g->edgeA;
			Edge *edgeB = g->edgeB;
			if( ground == g->edgeA )
			{
				if( e == edgeB->edge0 
					|| e == edgeB->edge1
					|| e == edgeB )
				{
					return;
				}

				
			}
			else if( ground == g->edgeB )
			{
				if( e == edgeA->edge0 
					|| e == edgeA->edge1
					|| e == edgeA )
				{
					return;
				}
			}
		}
		if( ground != NULL )
		{
			if( ground->edge0->edgeType == Edge::CLOSED_GATE )
			{
				Gate *g = (Gate*)ground->edge0->info;
				Edge *e0 = ground->edge0;
				if( e0 == g->edgeA )
				{
					Edge *edgeB = g->edgeB;
					if( e == edgeB->edge0 
						|| e == edgeB->edge1
						|| e == edgeB )
					{
						return;
					}
				}
				else if( e0 == g->edgeB )
				{
					Edge *edgeA = g->edgeA;
					if( e == edgeA->edge0 
						|| e == edgeA->edge1
						|| e == edgeA )
					{
						return;
					}
				}
			}
			
			
			if( ground->edge1->edgeType == Edge::CLOSED_GATE )
			{
				Gate *g = (Gate*)ground->edge1->info;
				Edge *e1 = ground->edge1;
				if( e1 == g->edgeA )
				{
					Edge *edgeB = g->edgeB;
					if( e == edgeB->edge0 
						|| e == edgeB->edge1
						|| e == edgeB )
					{
						return;
					}
				}
				else if( e1 == g->edgeB )
				{
					Edge *edgeA = g->edgeA;
					if( e == edgeA->edge0 
						|| e == edgeA->edge1
						|| e == edgeA )
					{
						return;
					}
				}
			}
		}

		Contact *c = owner->coll.collideEdge( position + physBody.offset, physBody, e, tempVel, V2d( 0, 0 ) );




		if( c != NULL )
		{

			//cout << "testing" << endl;
			double len0 = length( c->position - e->v0 );
			double len1 = length( c->position - e->v1 );

			if( e->edge0->edgeType == Edge::CLOSED_GATE && len0 < 1 )
			{
				V2d pVec = normalize( position - e->v0 );
				double pAngle = atan2( -pVec.y, pVec.x );

				if( pAngle < 0 )
				{
					pAngle += 2 * PI;
				}

				Edge *e0 = e->edge0;
				Gate *g = (Gate*)e0->info;

				V2d startVec = normalize( e0->v0 - e->v0 );
				V2d endVec = normalize( e->v1 - e->v0 );

				double startAngle = atan2( -startVec.y, startVec.x );
				if( startAngle < 0 )
				{
					startAngle += 2 * PI;
				}
				double endAngle = atan2( -endVec.y, endVec.x );
				if( endAngle < 0 )
				{
					endAngle += 2 * PI;
				}

				double temp = startAngle;
				startAngle = endAngle;
				endAngle = temp;

				if( endAngle < startAngle )
				{
					if( pAngle >= endAngle || pAngle <= startAngle )
					{
					
					}
					else
					{
						return;
					}
				}
				else
				{
					if( pAngle >= startAngle && pAngle <= endAngle )
					{
					}
					else
					{
						return;
					}
				}
			

			}
			else if( e->edge1->edgeType == Edge::CLOSED_GATE && len1 < 1 )
			{
				V2d pVec = normalize( position - e->v1 );
				double pAngle = atan2( -pVec.y, pVec.x );

				if( pAngle < 0 )
				{
					pAngle += 2 * PI;
				}

				Edge *e1 = e->edge1;
				Gate *g = (Gate*)e1->info;

				V2d startVec = normalize( e->v0 - e->v1 );
				V2d endVec = normalize( e1->v1 - e->v1 );

				double startAngle = atan2( -startVec.y, startVec.x );
				if( startAngle < 0 )
				{
					startAngle += 2 * PI;
				}
				double endAngle = atan2( -endVec.y, endVec.x );
				if( endAngle < 0 )
				{
					endAngle += 2 * PI;
				}
			
				double temp = startAngle;
				startAngle = endAngle;
				endAngle = temp;

				//double temp = startAngle;
				//startAngle = endAngle;
				//endAngle = temp;

				if( endAngle < startAngle )
				{
					/*if( pAngle > startAngle && pAngle < endAngle )
					{
						return;
					}*/


					if( pAngle >= endAngle || pAngle <= startAngle )
					{
					}
					else
					{
						return;
					}
				}
				else
				{
					/*if( pAngle < startAngle || pAngle > endAngle )
					{
						cout << "crawler edge: " << e->Normal().x << ", " << e->Normal().y << ", return b. start: " << startAngle << ", end: " << endAngle << ", p: " << pAngle << endl;
						return;
					}*/
				
					if( pAngle >= startAngle && pAngle <= endAngle )
					{
					}
					else
					{	
						return;
					}
				}
			}

			if( !col || (minContact.collisionPriority < 0 ) || (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) ) //(c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	

				if( groundSpeed > 0 && e == ground->edge1 && ( c->normal.x == 0 && c->normal.y == 0 ) )
				{

					//WHY DO I HAVE THIS WTF

					//cout << "blah" << endl;
					//return;
				}
				//else if( groundSpeed < 0 && e == ground->edge0 && ( c->normal.x == 0 && c->normal.y == 0 ) )
				//{
				//	//cout << "blah" << endl;
				//	return;
				//}

				if( c->collisionPriority == minContact.collisionPriority )
				{
					if(( c->normal.x == 0 && c->normal.y == 0 ) )
					{
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						minContact.normal = c->normal;
						minContact.movingPlat = NULL;
						col = true;
					}
				}
				else
				{
					minContact.collisionPriority = c->collisionPriority;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					minContact.normal = c->normal;
					minContact.movingPlat = NULL;
					col = true;
					
				}
			}
		}
	}
	else if( queryMode == "reverse" )
	{	
		CrawlerReverser *cr = (CrawlerReverser*)qte;
		if( cr != lastReverser )
		{
			//cout << "PLEASE" << endl;
			if( cr->hurtBody.Intersects( physBody ) )
			{
				//cout << "reversed!" << endl;
				lastReverser = cr;
				groundSpeed = -groundSpeed;
				clockwise = !clockwise;
			}
		}
	}
	++possibleEdgeCount;
}

void Crawler::UpdateHitboxes()
{
	if( ground != NULL )
	{
		V2d gn = ground->Normal();
		double angle = 0;
		if( !approxEquals( abs(offset.x), physBody.rw ) )
		{
			//this should never happen
		}
		else
		{
			angle = atan2( gn.x, -gn.y );
		}
		hitBody.globalAngle = angle;
		hurtBody.globalAngle = angle;

		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		if( groundSpeed > 0 )
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
			hitboxInfo->knockback = 15;
		}
	}
	else
	{
		hitBody.globalAngle = 0;
		hurtBody.globalAngle = 0;
	}

	//hitBody.globalPosition = position + V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );
	hitBody.globalPosition = position;
	hurtBody.globalPosition = position;
	physBody.globalPosition = position;//+ V2d( -16, 0 );// + //physBody.offset + offset;
}

void Crawler::UpdatePrePhysics()
{
	if( dead )
		return;

	if( !dead && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( monitor != NULL )
			{
				owner->keyMarker->CollectKey();
				///
				//owner->player->CaptureMonitor( monitor );
			}
			//AttemptSpawnMonitor();
			dead = true;
			owner->player->ConfirmEnemyKill( this );
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}
			
		

		receivedHit = NULL;
	}

	if( attackFrame == 2 * attackMult )
	{
		attackFrame = -1;
	}
	//if( attacking )
	//{
	//}
	//else
	//{

		if( !roll && frame == 16 * crawlAnimationFactor )
		{
			frame = 0;
		}

		if ( roll && frame == 10 * rollAnimationFactor )
		{
			frame = rollAnimationFactor * 2; 
		}
	//}
}

void Crawler::UpdatePhysics()
{
	specterProtected = false;
	if( dead )
	{
		return;
	}

	double movement = 0;
	double maxMovement = min( physBody.rw, physBody.rh );
	movement = groundSpeed;

	movement /= slowMultiple * NUM_STEPS;
	while( movement != 0 )
	{
		//ground is always some value

		double steal = 0;
		if( movement > 0 )
		{
			if( movement > maxMovement )
			{
				steal = movement - maxMovement;
				movement = maxMovement;
			}
		}
		else 
		{
			if( movement < -maxMovement )
			{
				steal = movement + maxMovement;
				movement = -maxMovement;
			}
		}

		double extra = 0;
		bool leaveGround = false;
		double q = edgeQuantity;

		V2d gNormal = ground->Normal();


		double m = movement;
		double groundLength = length( ground->v1 - ground->v0 ); 

		if( approxEquals( q, 0 ) )
			q = 0;
		else if( approxEquals( q, groundLength ) )
			q = groundLength;

		Edge *e0 = ground->edge0;
		Edge *e1 = ground->edge1;
		V2d e0n = e0->Normal();
		V2d e1n = e1->Normal();

		bool transferLeft = false;
		bool transferRight = false;

		if( movement > 0 && q == groundLength )
		{
			double c = cross( e1n, gNormal );
			double d = dot( e1n, gNormal );
			//cout << "c: " << c << ", d: " << d << endl;
			//if( c >= -.5 && d > 0 )
			//if( d > . )
			if( gNormal == e1n )
			{
				//cout << "t1" << endl;
				q = 0;
				ground = e1;
			}
			else if( !roll )
			{
				//cout << "roll is true" << endl;
				roll = true;
				rollFactor = 0;
				frame = 0;
			}
			else
			{
				//cout << "rollin" << endl;
				double angle = m / 3.0 /  physBody.rw;
				V2d currVec = position - ground->v1;
				V2d newPos;
				newPos.x = currVec.x * cos( angle ) - 
					currVec.y * sin( angle ) + ground->v1.x;
				newPos.y = currVec.x * sin( angle ) + 
					currVec.y * cos( angle ) + ground->v1.y;
				V2d newVec = newPos - ground->v1;
				double rollNew = atan2( newVec.y, newVec.x );
				if( rollNew < 0 )
				{
					rollNew += 2 * PI;
				}

				double oldRollFactor = rollFactor;
				double rollStart = atan2( gNormal.y, gNormal.x );
				V2d startVec = V2d( cos( rollStart ), sin( rollStart ) );
				double rollEnd = atan2( e1n.y, e1n.x );

				if( rollStart < 0 )
					rollStart += 2 * PI;
				if( rollEnd < 0 )
					rollEnd += 2 * PI;

				//cout << "totalAngleDist: " << totalAngleDist << endl;
				//cout << "angleDist: " << angleDist << endl;
				//cout << "rollEnd: " << rollEnd << endl;
				//cout << "rollNew: " << rollNew << endl;

				bool changed = false;
				if( rollEnd > rollStart && ( rollNew > rollEnd || rollNew < rollStart ) )
				{
					changed = true;
					newPos = ground->v1 + e1n * physBody.rw;
				}
				//else if( rollEnd < rollStart && rollNew < rollEnd )
				else if( rollEnd < rollStart && ( rollNew > rollEnd && rollNew < rollStart ) )
				{
					changed = true;
					newPos = ground->v1 + e1n * physBody.rw;
				}

				//V2d newVec = newPos - ground->v1;


				bool hit = ResolvePhysics( newPos - position );
				if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
				{
					//hitting while in a roll already. do the normal one first
					V2d eNorm = minContact.normal;//
					Edge *e = minContact.edge;
					if( eNorm.x == 0 && eNorm.y == 0 )
					{
						if( minContact.position == e->v0 )
						{

						}
						else if( minContact.position == e->v1 )
						{

						}

					}
					ground = minContact.edge;
					q = ground->GetQuantity( minContact.position + minContact.resolution );
					edgeQuantity = q;
					V2d gn = ground->Normal();
					roll = false;
					//cout << "hitting" << endl;
					break;
				}

				if( changed )
				{
				//	cout << "t2" << endl;
					ground = e1;
					q = 0;
					roll = false;
				}
			}
		}
		else if( movement < 0 && q == 0 )
		{
			double d = dot( e1n, gNormal );

			if( gNormal == e0n )
			{
				q = length( e0->v1 - e0->v0 );
				ground = e0;
			}
			else if( !roll )
			{
				roll = true;
				rollFactor = 0;
				frame = 0;
			}
			else
			{
				//cout << "roll left" << endl;
				double angle = m / 3.0 / physBody.rw;
				V2d currVec = position - ground->v0;
				V2d newPos;
				newPos.x = currVec.x * cos( angle ) - 
					currVec.y * sin( angle ) + ground->v0.x;
				newPos.y = currVec.x * sin( angle ) + 
					currVec.y * cos( angle ) + ground->v0.y;
				V2d newVec = newPos - ground->v0;
				double rollNew = atan2( newVec.y, newVec.x );
				if( rollNew < 0 )
				{
					rollNew += 2 * PI;
				}

				double oldRollFactor = rollFactor;
				double rollStart = atan2( gNormal.y, gNormal.x );
				V2d startVec = V2d( cos( rollStart ), sin( rollStart ) );
				double rollEnd = atan2( e0n.y, e0n.x );

				if( rollStart < 0 )
					rollStart += 2 * PI;
				if( rollEnd < 0 )
					rollEnd += 2 * PI;

				bool changed = false;
				if( rollEnd < rollStart && ( rollNew < rollEnd || rollNew > rollStart ) )
				{
					//cout << "first" << endl;
					changed = true;
					newPos = ground->v0 + e0n * physBody.rw;
				}
				else if( rollEnd > rollStart && ( rollNew < rollEnd && rollNew > rollStart ) )
				{
					//cout << "second: " << rollStart << ", end: " << rollEnd << ", new: " << rollNew << endl;
					changed = true;
					newPos = ground->v0 + e0n * physBody.rw;
				}

				bool hit = ResolvePhysics( newPos - position );
				if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
				{
					V2d eNorm = minContact.normal;//edge->Normal();
					Edge *e = minContact.edge;

					if( eNorm.x == 0 && eNorm.y == 0 )
					{
						if( minContact.position == e->v0 )
						{
							//q = ;

						}
						else //v1
						{
						}
					}
					ground = minContact.edge;
					q = ground->GetQuantity( minContact.position + minContact.resolution );
					edgeQuantity = q;
					V2d gn = ground->Normal();
					roll = false;
					//cout << "hitting" << endl;
					break;
				}	

				if( changed )
				{
					//cout << "cmon" << endl;
					ground = e0;
					q = length( e0->v1 - e0->v0 );
					roll = false;
				}
			}
		}
		else
		{
			if( movement > 0 )
			{	
				extra = (q + movement) - groundLength;
			}
			else 
			{
				extra = (q + movement);
			}
					
			if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
			{
				if( movement > 0 )
				{
					q = groundLength;
				}
				else
				{
					q = 0;
				}
				movement = extra;
				m -= extra;
				//cout << "adjusting m to: " << m << endl;
						
			}
			else
			{
				movement = 0;
				q += m;
			}
				
			if( !approxEquals( m, 0 ) )//	if(m != 0 )
			{	
				bool down = true;
				bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
				if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
				{
					V2d eNorm = minContact.edge->Normal();
					V2d movetest =  (normalize( ground->v1 - ground->v0 ) * m);
					ground = minContact.edge;
					if( minContact.normal.x == 0 && minContact.normal.y == 0 )
					{
						roll = true;
						edgeQuantity = ground->GetQuantity( minContact.position );
						position += minContact.resolution;
					}
					else
					{
						//cout << "point!" << endl;
						edgeQuantity = ground->GetQuantity( minContact.position + minContact.resolution );
					}
					break;
				}			
			}
			else
			{
				if( clockwise )
				{
					//cout << "t33" << endl;
					ground = e1;
					q = 0;
				}
				else
				{
					//cout << "here>? " << endl;
					ground = e0;
					q = length( e0->v1 - e0->v0 );
				}
				
			}
		}

		if( movement == extra )
			movement += steal;
		else
			movement = steal;

		edgeQuantity = q;
	}

	PhysicsResponse();
}

bool Crawler::ResolvePhysics( V2d vel )
{
	possibleEdgeCount = 0;

	Rect<double> oldR( position.x + physBody.offset.x - physBody.rw, 
		position.y + physBody.offset.y - physBody.rh, 2 * physBody.rw, 2 * physBody.rh );
	position += vel;
	
	Rect<double> newR( position.x + physBody.offset.x - physBody.rw, 
		position.y + physBody.offset.y - physBody.rh, 2 * physBody.rw, 2 * physBody.rh );
	//minContact.collisionPriority = 1000000;
	
	double oldRight = oldR.left + oldR.width;
	double right = newR.left + newR.width;

	double oldBottom = oldR.top + oldR.height;
	double bottom = newR.top + newR.height;

	double maxRight = max( right, oldRight );
	double maxBottom = max( oldBottom, bottom );
	double minLeft = min( oldR.left, newR.left );
	double minTop = min( oldR.top, newR.top );
	//Rect<double> r( minLeft - 5 , minTop - 5, maxRight - minLeft + 5, maxBottom - minTop + 5 );
	Rect<double> r( minLeft , minTop, maxRight - minLeft, maxBottom - minTop );

	
	minContact.collisionPriority = 1000000;

	

	tempVel = vel;

	col = false;
	minContact.edge = NULL;

	queryMode = "resolve";
	owner->terrainTree->Query( this, r );
	//Query( this, owner->testTree, r );

	return col;
}

void Crawler::PhysicsResponse()
{
	if( !dead  )
	{
		//cout << "response" << endl;
		double spaceNeeded = 0;
		V2d gn = ground->Normal();
		V2d gPoint = ground->GetPoint( edgeQuantity );
	

		double angle = 0;
	
		if( !roll )
		{
			position = gPoint + gn * 32.0;
			angle = atan2( gn.x, -gn.y );
		
//			sprite.setTexture( *ts_walk->texture );
			IntRect r = ts->GetSubRect( frame / crawlAnimationFactor );
			if( !clockwise )
			{
				sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
			}
			else
			{
				sprite.setTextureRect( r );
			}
			
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		else
		{
			
			if( clockwise )
			{
				/*V2d e1n = ground->edge1->Normal();
				double rollStart = atan2( gn.y, gn.x );
				double rollEnd = atan2( e1n.y, e1n.x );
				double adjRollStart = rollStart;
				double adjRollEnd = rollEnd;

				if( rollStart < 0 )
					adjRollStart += 2 * PI;
				if( rollEnd < 0 )
					adjRollEnd += 2 * PI;
		
				if( adjRollEnd > adjRollStart )
				{
					angle  = adjRollStart * ( 1.0 - rollFactor ) + adjRollEnd  * rollFactor ;
				}
				else
				{
			
					angle = rollStart * ( 1.0 - rollFactor ) + rollEnd  * rollFactor;

					if( rollStart < 0 )
						rollStart += 2 * PI;
					if( rollEnd < 0 )
						rollEnd += 2 * PI;
				}

				if( angle < 0 )
					angle += PI * 2;

		

				V2d angleVec = V2d( cos( angle ), sin( angle ) );
				angleVec = normalize( angleVec );

				position = gPoint + angleVec * 16.0;

				angle += PI / 2.0;*/

				V2d vec = normalize( position - ground->v1 );
				angle = atan2( vec.y, vec.x );
				angle += PI / 2.0;
	

				//sprite.setTexture( *ts->texture );
				IntRect r = ts->GetSubRect( frame / rollAnimationFactor + 17 );
				if( clockwise )
				{
					sprite.setTextureRect( r );
				}
				else
				{
					sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
				}
			
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				V2d pp = ground->GetPoint( edgeQuantity );
				sprite.setPosition( pp.x, pp.y );
			}
			else
			{
				//angle = 
				/*V2d e0n = ground->edge0->Normal();
				double rollStart = atan2( gn.y, gn.x );
				double rollEnd = atan2( e0n.y, e0n.x );
				double adjRollStart = rollStart;
				double adjRollEnd = rollEnd;

				if( rollStart < 0 )
					adjRollStart += 2 * PI;
				if( rollEnd < 0 )
					adjRollEnd += 2 * PI;
		
				if( adjRollEnd > adjRollStart )
				{
					angle  = adjRollStart * ( 1.0 - rollFactor ) + adjRollEnd  * rollFactor ;
				}
				else
				{
			
					angle = rollStart * ( 1.0 - rollFactor ) + rollEnd  * rollFactor;

					if( rollStart < 0 )
						rollStart += 2 * PI;
					if( rollEnd < 0 )
						rollEnd += 2 * PI;
				}

				if( angle < 0 )
					angle += PI * 2;*/

			

			//	V2d angleVec = V2d( cos( angle ), sin( angle ) );
			//	angleVec = normalize( angleVec );

			//	position = gPoint + angleVec * 16.0;
				V2d vec = normalize( position - ground->v0 );
				angle = atan2( vec.y, vec.x );
				angle += PI / 2.0;
	

				//sprite.setTexture( *ts->texture );
				IntRect r = ts->GetSubRect( frame / rollAnimationFactor + 17 );
				if( clockwise )
				{
					sprite.setTextureRect( r );
				}
				else
				{
					sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
				}
			
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				V2d pp = ground->GetPoint( edgeQuantity );
				sprite.setPosition( pp.x, pp.y );
			}

			
		}
	

		UpdateHitboxes();

		if( PlayerSlowingMe() )
		{
			if( slowMultiple == 1 )
			{
				slowCounter = 1;
				slowMultiple = 5;
			//	cout << "yes slow" << endl;
			}
		}
		else
		{
			slowCounter = 1;
			slowMultiple = 1;
		//	cout << "no slow" << endl;
		}

		if( receivedHit == NULL )
		{
			pair<bool, bool> result = PlayerHitMe();
			if( result.first )
			{
				//cout << "hit here!" << endl;
				//triggers multiple times per frame? bad?
				owner->player->ConfirmHit( COLOR_BLUE, 5, .8, 2 * 6 * 2 );

				if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
				{
					owner->player->velocity.y = 4;//.5;
				}

															//cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
			//owner->ActivateEffect( ts_testBlood, position, true, 0, 6, 3, facingRight );
		//	cout << "patroller received damage of: " << receivedHit->damage << endl;
			
			/*if( !result.second )
			{
				owner->Pause( 6 );
			}*/
			
			//dead = true;
			//receivedHit = NULL;
			}
		}

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		//gotta get the correct angle upon death
		Transform t;
		t.rotate( angle / PI * 180 );
		Vector2f newPoint = t.transformPoint( Vector2f( 1, -1 ) );
		deathVector = V2d( newPoint.x, newPoint.y );

		queryMode = "reverse";

		//physbody is a circle
		Rect<double> r( position.x - physBody.rw, position.y - physBody.rw, physBody.rw * 2, physBody.rw * 2 );
		owner->crawlerReverserTree->Query( this, r );
	}
}

void Crawler::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
		owner->ActivateEffect( ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
	}

	if( deathFrame == 30 )
	{
		//owner->ActivateEffect( ts_testBlood, position, true, 0, 15, 2, true );
		owner->RemoveEnemy( this );
		return;
	}

	if( deathFrame == 0 && dead )
	{
		owner->ActivateEffect( ts_testBlood, position, true, 0, 15, 2, true );
	}

	UpdateSprite();


	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
		
		if( dead )
		{
			deathFrame++;
		}
		else
		{
			if( attackFrame >= 0 )
				++attackFrame;
		}
	}
	else
	{
		slowCounter++;
	}
	//need to calculate frames in here!!!!

	//sprite.setPosition( position );
	//UpdateHitboxes();
}

bool Crawler::PlayerSlowingMe()
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

void Crawler::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		if( monitor != NULL && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 55 );
			cs.setFillColor( Color::Black );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		target->draw( sprite );
	}
	else
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			//bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			//bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			//bloodSprite.setPosition( position.x, position.y );
			//bloodSprite.setScale( 2, 2 );
			//target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );
	}
}

void Crawler::DrawMinimap( sf::RenderTarget *target )
{
	/*CircleShape cs;
	cs.setRadius( 50 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );

	if( monitor != NULL && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/

	if( !dead && monitor != NULL && !suppressMonitor )
	{
		CircleShape cs;
		cs.setRadius( 50 );
		cs.setFillColor( Color::White );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( position.x, position.y );
		target->draw( cs );
	}
}

bool Crawler::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( player->invincibleFrames == 0 && hitBody.Intersects( player->hurtBody ) )
	{
		if( player->position.x < position.x )
		{
			hitboxInfo->kbDir.x = -abs( hitboxInfo->kbDir.x );
			//cout << "left" << endl;
		}
		else if( player->position.x > position.x )
		{
			//cout << "right" << endl;
			hitboxInfo->kbDir.x = abs( hitboxInfo->kbDir.x );
		}
		else
		{
			//dont change it
		}
		attackFrame = 0;
		player->ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

 pair<bool, bool> Crawler::PlayerHitMe()
{
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
			receivedHit = player->currHitboxInfo;
			return pair<bool, bool>(true,false);
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

void Crawler::UpdateSprite()
{
	if( dead )
	{
		//cout << "deathVector: " << deathVector.x << ", " << deathVector.y << endl;
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 31 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2);
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 30 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );
	}
	else
	{
		if( attackFrame >= 0 )
		{
			IntRect r = ts->GetSubRect( 28 + attackFrame / attackMult );
			if( !clockwise )
			{
				r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
			}
			sprite.setTextureRect( r );
		}
	}
}

void Crawler::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		V2d g = ground->GetPoint( edgeQuantity );
		cs.setPosition( g.x, g.y );

		//owner->window->draw( cs );
		//UpdateHitboxes();
		physBody.DebugDraw( target );
	}
//	hurtBody.DebugDraw( target );
//	hitBody.DebugDraw( target );
}

void Crawler::SaveEnemyState()
{
}

void Crawler::LoadEnemyState()
{
}

CrawlerReverser::CrawlerReverser( GameSession *owner, Edge *edge, double q )
	:ground( edge ), quantity( q )
{

	V2d gNorm = edge->Normal();
	ts = owner->GetTileset( "crawlerreverser.png", 32, 32 );
	position = edge->GetPoint( quantity ) + 16.0 * gNorm;
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );

	drawNext = NULL;
	double angle = atan2( gNorm.y, gNorm.x );
	sprite.setRotation( angle );
	sprite.setPosition( position.x, position.y );

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;

	hurtBody.globalAngle = 0;
	hurtBody.globalPosition = position;
}

void CrawlerReverser::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool CrawlerReverser::IsTouchingBox( const sf::Rect<double> &r )
{
	V2d rA( r.left, r.top );
	V2d rB( r.left + r.width, r.top );
	V2d rC( r.left + r.width, r.top + r.height );
	V2d rD( r.left, r.top + r.height );

	V2d gPos = ground->GetPoint( quantity );
	V2d along = normalize( ground->v1 - ground->v0 );
	V2d other( along.y, -along.x );

	double width = 16;
	double height = 16;
	V2d A = gPos - along * width;
	V2d B = A + other * height;
	V2d C = B + along * width;
	V2d D = C - other * height;

	if( isQuadTouchingQuad( rA, rB, rC, rD, A, B, C, D ) )
	{
		return true;
	}

	return false;
}

void CrawlerReverser::Draw( sf::RenderTarget *target )
{
	target->draw( sprite );
}