#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;


Crawler::Crawler( GameSession *owner, Edge *g, float q, bool cw, float s )
	:Enemy( owner, EnemyType::CRAWLER ), ground( g ), edgeQuantity( q ), clockwise( cw ), groundSpeed( s )
{
	ts_walk = owner->GetTileset( "crawlerwalk.png", 96, 64 );
	ts_roll = owner->GetTileset( "crawlerroll.png", 96, 64 );
	sprite.setTexture( *ts_walk->texture );
	sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	Vector2f gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	roll = false;
	
	spawnRect = sf::Rect<float>( gPoint.x - 16, gPoint.y - 16, 16 * 2, 16 * 2 );
	crawlAnimationFactor = 2;
	rollAnimationFactor = 2;
	physBody.isCircle = true;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 16;
	physBody.rh = 16;
	physBody.type = CollisionBox::BoxType::Physics;

	startGround = ground;
	startQuant = edgeQuantity;
	frame = 0;
	position = gPoint + ground->Normal() * 16.f;
}

void Crawler::ResetEnemy()
{
	ground = startGround;
	edgeQuantity = startQuant;
	Vector2f gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );

	Vector2f gn = ground->Normal();
	if( gn.x > 0 )
		offset.x = physBody.rw;
	else if( gn.x < 0 )
		offset.x = -physBody.rw;
	if( gn.y > 0 )
		offset.y = physBody.rh;
	else if( gn.y < 0 )
		offset.y = -physBody.rh;

	position = gPoint + offset;
}

void Crawler::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	Edge *e = (Edge*)qte;


	if( ground == e )
			return;

	if( queryMode == "resolve" )
	{
		Contact *c = owner->coll.collideEdge( position + physBody.offset, physBody, e, tempVel, Vector2f( 0, 0 ) );

		if( c != NULL )
		{
			if( !col || (minContact.collisionPriority < 0 ) || (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) ) //(c->collisionPriority >= -.f0001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.f0001 ) ) )
			{	

				if( e == ground->edge1 && ( c->normal.x == 0 && c->normal.y == 0 ) )
				{
					return;
				}

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
	++possibleEdgeCount;
}

void Crawler::UpdateHitboxes()
{
	if( ground != NULL )
	{
		Vector2f gn = ground->Normal();
		float angle = 0;
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
	}
	else
	{
		hitBody.globalAngle = 0;
		hurtBody.globalAngle = 0;
	}

	hitBody.globalPosition = position + Vector2f( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	physBody.globalPosition = position;//+ Vector2f( -16, 0 );// + //physBody.offset + offset;
}

void Crawler::UpdatePrePhysics()
{
	if( ( !roll && frame == 17 * crawlAnimationFactor )
		|| ( roll && frame == 7 * rollAnimationFactor ) )
	{
		frame = 0;
	}
	groundSpeed = 1.5;
}

void Crawler::UpdatePhysics()
{
	float movement = 0;
	float maxMovement = min( physBody.rw, physBody.rh );
	movement = groundSpeed;

	while( movement != 0 )
	{
		//ground is always some value

		float steal = 0;
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

		float extra = 0;
		bool leaveGround = false;
		float q = edgeQuantity;

		Vector2f gNormal = ground->Normal();


		float m = movement;
		float groundLength = length( ground->v1 - ground->v0 ); 

		if( approxEquals( q, 0 ) )
			q = 0;
		else if( approxEquals( q, groundLength ) )
			q = groundLength;

		Edge *e0 = ground->edge0;
		Edge *e1 = ground->edge1;
		Vector2f e0n = e0->Normal();
		Vector2f e1n = e1->Normal();

		bool transferLeft = false;
		bool transferRight = false;

		if( q == groundLength )
		{
			if( !roll )
			{
				roll = true;
				rollFactor = 0;
				frame = 0;
			}
			else
			{
				if( rollFactor < 1.f )
				{ 
					float oldRollFactor = rollFactor;
					float rollStart = atan2( gNormal.y, gNormal.x );
					Vector2f startVec = Vector2f( cos( rollStart ), sin( rollStart ) );
					float rollEnd = atan2( e1n.y, e1n.x );

					if( rollStart < 0 )
						rollStart += 2 * PI;
					if( rollEnd < 0 )
						rollEnd += 2 * PI;

					Vector2f currentVec = position - ground->v1;
					currentVec = normalize( currentVec );
					float rollCurrent = atan2( currentVec.y, currentVec.x );
					if( rollCurrent < 0 )
						rollCurrent += 2 * PI;


					float totalAngleDist = rollEnd - rollStart;
					if( rollEnd < rollStart )
					{
						totalAngleDist = ( 2 * PI - rollStart ) + rollEnd;
					}


					float angleDist = rollEnd - rollCurrent;

					if( rollEnd < rollCurrent )
					{
						angleDist = ( 2 * PI - rollCurrent ) + rollEnd;
					}

					

					float arcDist = angleDist * physBody.rw;
					//arcDist *= 100;
					float oldArcDist = arcDist;
					//m /= 10;
					movement -= m;
					if( movement < 0 )
					{
						assert( false );
						movement = 0;
					}
					if( m > arcDist )
					{
						//cout << "m: " << m << ", arcDist: " << arcDist << endl;
						//float realMove = ;
						m -= arcDist;
						if( approxEquals( m, 0 ) )
						{
							m = 0;
						}
						rollFactor = 1;
						movement += m;

						Vector2f oldPos = position;
						Vector2f rollEndVec = Vector2f( cos( rollEnd ), sin ( rollEnd ) );
						Vector2f newPos = ground->v1 + rollEndVec * physBody.rw;

						bool hit = ResolvePhysics( newPos - oldPos );
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							Vector2f eNorm = minContact.edge->Normal();
							ground = minContact.edge;
							q = ground->GetQuantity( minContact.position + minContact.resolution );
							edgeQuantity = q;
							Vector2f gn = ground->Normal();
							roll = false;
							break;
						}			
					}
					else
					{
						//m = 0;
						arcDist -= m;
						rollFactor = ( totalAngleDist - arcDist / physBody.rw ) / totalAngleDist;

						Vector2f oldPos = position;
						float trueAngle = rollStart + angleDist * rollFactor;
						if( trueAngle > PI * 2 )
						{
							trueAngle -= PI * 2;
						}

						Vector2f trueVec = Vector2f( cos( trueAngle ), sin( trueAngle ) );
						
						Vector2f newPos = ground->v1 + trueVec * physBody.rw;

						//cout << "current: " << rollCurrent << ", new: " << rollFactor << "total: " << totalAngleDist << ", arcdist: " << arcDist << endl;
						//cout << "other vel: " << (newPos-oldPos).x << ", " << (newPos-oldPos).y << endl;
						bool hit = ResolvePhysics( newPos - oldPos );
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							Vector2f eNorm = minContact.edge->Normal();
							ground = minContact.edge;
							q = ground->GetQuantity( minContact.position + minContact.resolution );
							edgeQuantity = q;
							Vector2f gn = ground->Normal();
							roll = false;
							break;
						}			
						//rollFactor = 
					}




					
			

					//movement += m;
					//rollFactor += .01f;

					//float diff = abs( rollStart - rollEnd );

					//if( rollFactor > 1.f )
					//	rollFactor = 1.f;
				}
				else
				{
					ground = e1;
					q = 0;
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
					Vector2f eNorm = minContact.edge->Normal();
					ground = minContact.edge;
					q = ground->GetQuantity( minContact.position + minContact.resolution );
					edgeQuantity = q;
					Vector2f gn = ground->Normal();
					break;
				}			
			}
			else
			{
				ground = e1;
				q = 0;
			}
		}


		if( movement == extra )
			movement += steal;
		else
			movement = steal;

		edgeQuantity = q;
	}
}

void Crawler::UpdatePhysics3()
{
	float movement = 0;
	float maxMovement = min( physBody.rw, physBody.rh );
	movement = groundSpeed;

	while( movement != 0 )
	{
		//ground is always some value
		float steal = 0;
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

		float extra = 0;
		bool leaveGround = false;
		float q = edgeQuantity;

		Vector2f gNormal = ground->Normal();


		float m = movement;
		float groundLength = length( ground->v1 - ground->v0 ); 

		if( approxEquals( q, 0 ) )
			q = 0;
		else if( approxEquals( q, groundLength ) )
			q = groundLength;

		if( approxEquals( offset.x, physBody.rw ) )
			offset.x = physBody.rw;
		else if( approxEquals( offset.x, -physBody.rw ) )
			offset.x = -physBody.rw;

		if( approxEquals( offset.y, physBody.rh ) )
			offset.y = physBody.rh;
		else if( approxEquals( offset.y, -physBody.rh ) )
			offset.y = -physBody.rh;

		Edge *e0 = ground->edge0;
		Edge *e1 = ground->edge1;
		Vector2f e0n = e0->Normal();
		Vector2f e1n = e1->Normal();

		bool transferLeft = false;
		bool transferRight = false;
		bool changeOffsetX = false;
		bool changeOffsetY = false;
		
		cout << "offset: " << offset.x << ", " << offset.y << endl;
		cout << "q: " << q << ", " << groundLength << endl;
		if( movement < 0 )
		{
			if( e1n.x > 0 )
			{

			}
			else if( e1n.x < 0 )
			{
			
			}
			else //e1n.x == 0
			{
			}
		}
		else if( movement > 0 )
		{
			if( q == groundLength )
			{
				if( e1n.x < 0 )
				{
					if( e1n.y < 0 )
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
					else if( e1n.y > 0 )
					{
						if( offset.x == -physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( gNormal.y <= 0 )
							{
								if( offset.y < physBody.rh )
									changeOffsetY = true;
								else if( offset.x > -physBody.rw )
									changeOffsetX = true;
								else
									assert( false && "what 1" );
							}
							else
							{
								if( offset.x > -physBody.rw )
									changeOffsetX = true;
								else if( offset.y < physBody.rh )
									changeOffsetY = true;
								else
									assert( false && "what 1" );
							}
						}
					}
					else
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
				}
				else if( e1n.x > 0 )
				{
					if( e1n.y < 0 )
					{
						if( offset.x == physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( gNormal.x < 0 )
							{
								if( offset.y > -physBody.rh )
									changeOffsetY = true;
								else if( offset.x < physBody.rw )
									changeOffsetX = true;
								else
									assert( false && "what 1" );
							}
							else
							{
								if( offset.x < physBody.rw )
									changeOffsetX = true;
								else if( offset.y > -physBody.rh )
									changeOffsetY = true;
								else
									assert( false && "what 1" );
							}
							
						}
					}
					else if( e1n.y > 0 )
					{
						if( offset.x == physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( gNormal.x <= 0 )
							{
								if( offset.y < physBody.rh )
									changeOffsetY = true;
								else if( offset.x < physBody.rw )
									changeOffsetX = true;
								else
									assert( false && "what 1" );
							}
							else
							{
								if( offset.x < physBody.rw )
									changeOffsetX = true;
								else if( offset.y < physBody.rh )
									changeOffsetY = true;
								else
									assert( false && "what 1" );
							}
						}
					}
					else
					{
						if( offset.x == physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
				}
				else
				{
					if( e1n.y < 0 )
					{
						if( offset.x == physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
							{
								changeOffsetX = true;
								cout << "blahhhh" << endl;
							}
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "blah1" );
						}
					}
					else if( e1n.y > 0 )
					{
						
						if( offset.x == -physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.y < physBody.rh )
							{
								changeOffsetY = true;
							}
							else if( offset.x > -physBody.rw )
							{
								changeOffsetX = true;
								cout << "ADfsdfsdfsdf" << endl;
							}
							else
								assert( false && "blah2" );
						}
					}
					else
						assert( false && "cant happen" );
				}
			}		
		}


		if( transferLeft )
			{
				cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				
				ground = next;
				q = length( ground->v1 - ground->v0 );					
			}
		else if( transferRight )
			{
				cout << "transfer right" << endl;
				Edge *next = ground->edge1;
				
				ground = next;
				q = 0;
			}
		else if( changeOffsetX )
			{
				cout << "change offset x" << endl;
				if( gNormal.y < 0 && e1n.y <= 0 ) // || (gNormal.y == 0 && gNormal.x < 0 ) )
				{
				if( movement > 0 )
				{				
					extra = (offset.x + movement) - physBody.rw;				
				}
				else 
				{
					extra = (offset.x + movement) + physBody.rw;
				}
				float m = movement;
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					m -= extra;
					movement = extra;

					if( movement > 0 )
					{
						offset.x = physBody.rw;
					}
					else
					{
						offset.x = -physBody.rw;
					}
				}
				else
				{
					movement = 0;
					offset.x += m;
				}

				if(!approxEquals( m, 0 ) )
				{
					bool hit = ResolvePhysics( Vector2f( m, 0 ));
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						Vector2f eNorm = minContact.edge->Normal();
						if( eNorm.y < 0 )
						{
							//if( minContact.position.y >= position.y + physBody.rh - 5 )
							{
								if( m > 0 && eNorm.x < 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offset.x = -physBody.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offset.x = physBody.rw;
									continue;
								}
								

							}
						}
						else
						{
								offset.x += minContact.resolution.x;
								groundSpeed = 0;
								break;
						}
					}
				}

				}
				else
				{
					if( movement > 0 )
					{				
						extra = (-offset.x + movement) - physBody.rw;				
					}
					else 
					{
						extra = (-offset.x + movement) + physBody.rw;
					}
					float m = movement;
					if( (m > 0 && extra > 0) || (m < 0 && extra < 0) )
					{
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offset.x = -physBody.rw;
						}
						else
						{
							offset.x = physBody.rw;
						}
					}
					else
					{
						movement = 0;
						offset.x -= m;
					}

					if(!approxEquals( m, 0 ) )
					{
						bool hit = ResolvePhysics( Vector2f( -m, 0 ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							Vector2f eNorm = minContact.edge->Normal();
							if( eNorm.y < 0 )
							{
								//if( minContact.position.y >= position.y + physBody.rh - 5 )
								{
									if( m > 0 && eNorm.x < 0 )
									{
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										edgeQuantity = q;
										offset.x = -physBody.rw;
										continue;
									}
									else if( m < 0 && eNorm.x > 0 )
									{
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										edgeQuantity = q;
										offset.x = physBody.rw;
										continue;
									}
								

								}
							}
							else
							{
									offset.x += minContact.resolution.x;
									groundSpeed = 0;
									break;
							}
						}
					}
				}

			}
		else if( changeOffsetY )
			{
				cout << "change offset y" << endl;
				//if( ( gNormal.x < 0 && e1n.y <= 0 || gNormal.x >= 0 && e1n.y >= 0) )// || (gNormal.x == 0 && gNormal.y > 0 ) )
				if( gNormal.x >= 0 && gNormal.y > 0 && e1n.x <= 0 && e1n.y <= 0  )
				{
					cout << "changing offsety: " << offset.x << ", " << offset.y << endl;
					if( movement > 0 )
					{				
						extra = (-offset.y + movement) - physBody.rh;				
					}
					else 
					{
						extra = (-offset.y + movement) + physBody.rh;
					}
					float m = movement;

					if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
					{
						
						//m -= extra;
					
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offset.y = -physBody.rh;
						}
						else
						{
							offset.y = physBody.rh;
						}
					}
					else
					{
						
						//m = -m;
						movement = 0;
						offset.y -= m;
					}

					if(!approxEquals( m, 0 ) )
					{
						bool hit = ResolvePhysics( Vector2f( 0, -m ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							Vector2f eNorm = minContact.edge->Normal();

							ground = minContact.edge;

							q = ground->GetQuantity( minContact.position + minContact.resolution );
							edgeQuantity = q;

							Vector2f gn = ground->Normal();
							if( gn.x > 0 )
								offset.x = physBody.rw;
							else if( gn.x < 0 )
								offset.x = -physBody.rw;

							if( gn.y > 0 )
								offset.y = physBody.rh;
							else if( gn.y < 0 )
								offset.y = -physBody.rh;

							position = ground->GetPoint( edgeQuantity ) + offset;

							break;
						}
					}
				}
				else
				{
					
					if( movement > 0 )
					{				
						extra = (offset.y + movement) - physBody.rh;				
					}
					else 
					{
						extra = (offset.y + movement) + physBody.rh;
					}
					float m = movement;

					if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
					{
						//m -= extra;
					
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offset.y = physBody.rh;
						}
						else
						{
							offset.y = -physBody.rh;
						}
					}
					else
					{
						
						//m = -m;
						movement = 0;
						offset.y += m;
					}

					if(!approxEquals( m, 0 ) )
					{
						bool hit = ResolvePhysics( Vector2f( 0, m ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							Vector2f eNorm = minContact.edge->Normal();

							ground = minContact.edge;

							q = ground->GetQuantity( minContact.position + minContact.resolution );
							edgeQuantity = q;

							Vector2f gn = ground->Normal();
							if( gn.x > 0 )
								offset.x = physBody.rw;
							else if( gn.x < 0 )
								offset.x = -physBody.rw;

							if( gn.y > 0 )
								offset.y = physBody.rh;
							else if( gn.y < 0 )
								offset.y = -physBody.rh;

							position = ground->GetPoint( edgeQuantity ) + offset;

							break;
						}
					}
					else
					{
						cout << "problelm" << endl;
					}
				}
				
			}
		else
			{
				cout << "else" << endl;
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
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				

				if( m == 0 )
				{
					cout << "secret: " << endl;//<< gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					groundSpeed = 0;
					break;
				}

				if( !approxEquals( m, 0 ) )//	if(m != 0 )
				{	
					bool down = true;
					bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						Vector2f eNorm = minContact.edge->Normal();

						ground = minContact.edge;

						//q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
						q = ground->GetQuantity( minContact.position + minContact.resolution );
						//groundSpeed = 0;
						edgeQuantity = q;

						Vector2f gn = ground->Normal();
						if( gn.x > 0 )
							offset.x = physBody.rw;
						else if( gn.x < 0 )
							offset.x = -physBody.rw;
						else
						{
							offset.x = position.x + minContact.resolution.x - minContact.position.x;
							//offsetX = position.x + minContact.resolution.x - minContact.position.x;
						}

						if( gn.y > 0 )
							offset.y = physBody.rh;
						else if( gn.y < 0 )
							offset.y = -physBody.rh;
						else
						{
							offset.y = position.y + minContact.resolution.y - minContact.position.y;
						}

						position = ground->GetPoint( edgeQuantity ) + offset;

						break;
					}
						
				}	
			}

		if( movement == extra )
			movement += steal;
		else
			movement = steal;

		edgeQuantity = q;
		//cout << "offset: " << offset.x << ", " << offset.y << endl;
		//cout << "q:" << q << ", " << groundLength << endl;
	}

}

void Crawler::UpdatePhysics2()
{
	float movement = 0;
	float maxMovement = min( physBody.rw, physBody.rh );
	Vector2f movementVec;
	Vector2f lastExtra( 100000, 100000 );

	//make sure ground != NULL
	movement = groundSpeed;


	while( (movement != 0) )
	{
		if( ground != NULL )
		{
			float steal = 0;
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

			float extra = 0;
			bool leaveGround = false;
			float q = edgeQuantity;

			Vector2f gNormal = ground->Normal();


			float m = movement;
			float groundLength = length( ground->v1 - ground->v0 ); 

			if( approxEquals( q, 0 ) )
				q = 0;
			else if( approxEquals( q, groundLength ) )
				q = groundLength;

			if( approxEquals( offset.x, physBody.rw ) )
				offset.x = physBody.rw;
			else if( approxEquals( offset.x, -physBody.rw ) )
				offset.x = -physBody.rw;

			if( approxEquals( offset.y, physBody.rh ) )
				offset.y = physBody.rh;
			else if( approxEquals( offset.y, -physBody.rh ) )
				offset.y = -physBody.rh;

			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;
			Vector2f e0n = e0->Normal();
			Vector2f e1n = e1->Normal();

			bool transferLeft = false;
			bool transferRight = false;
			bool changeOffsetX = false;
			bool changeOffsetY = false;
			
			cout << "offset: " << offset.x << ", " << offset.y << endl;
			cout << "q:" << q << ", " << groundLength << endl;

			if( q == 0 && movement < 0 )
			{
				if( e0n.x < 0 )
				{
					if( e0n.y < 0 )
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
					else if( e0n.y > 0 )
					{
						if( offset.x == -physBody.rw && offset.y == physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
				}
				else if( e0n.x > 0 )
				{
					if( e0n.y < 0 )
					{
						if( offset.x == physBody.rw && offset.y == -physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else if( e0n.y > 0 )
					{
						if( offset.x == physBody.rw && offset.y == physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else
					{
						if( offset.x == physBody.rw && offset.y == physBody.rh )
						{
							transferLeft = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
				}
				else
				{
					if( e0n.y < 0 )
					{
						if( offset.x == -physBody.rw )
						{
							transferLeft = true;
						}
						else
						{
							changeOffsetX = true;
						}
					}
					else if( e0n.y > 0 )
					{
						if( offset.x == physBody.rw )
						{
							transferLeft = true;
						}
						else
						{
							changeOffsetX = true;
						}
					}
					else
						assert( false && "cant happen" );
				}
			}
			else if( q == groundLength && movement > 0 )
			{
				if( e1n.x < 0 )
				{
					if( e1n.y < 0 )
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
					else if( e1n.y > 0 )
					{
						if( offset.x == -physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else
					{
						if( offset.x == -physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x > -physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
				}
				else if( e1n.x > 0 )
				{
					if( e1n.y < 0 )
					{
						if( offset.x == physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else if( e1n.y > 0 )
					{
						if( offset.x == physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 1" );
						}
					}
					else
					{
						if( offset.x == physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
								changeOffsetX = true;
							else if( offset.y < physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "what 0" );
						}
					}
				}
				else
				{
					if( e1n.y < 0 )
					{
						if( offset.x == physBody.rw && offset.y == -physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.x < physBody.rw )
							{
								changeOffsetX = true;
								cout << "blahhhh" << endl;
							}
							else if( offset.y > -physBody.rh )
								changeOffsetY = true;
							else
								assert( false && "blah1" );
						}
					}
					else if( e1n.y > 0 )
					{
						
						if( offset.x == -physBody.rw && offset.y == physBody.rh )
						{
							transferRight = true;
						}
						else
						{
							if( offset.y < physBody.rh )
							{
								changeOffsetY = true;
							}
							else if( offset.x > -physBody.rw )
							{
								changeOffsetX = true;
								cout << "ADfsdfsdfsdf" << endl;
							}
							else
								assert( false && "blah2" );
						}
					}
					else
						assert( false && "cant happen" );
				}
			}
				
			if( transferLeft )
			{
				cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				
				ground = next;
				q = length( ground->v1 - ground->v0 );					
			}
			else if( transferRight )
			{
				cout << "transfer right" << endl;
				Edge *next = ground->edge1;
				
				ground = next;
				q = 0;
			}
			else if( changeOffsetX )
			{
				cout << "change offset x" << endl;
				if( gNormal.y < 0 && e1n.y < 0 ) // || (gNormal.y == 0 && gNormal.x < 0 ) )
				{
				if( movement > 0 )
				{				
					extra = (offset.x + movement) - physBody.rw;				
				}
				else 
				{
					extra = (offset.x + movement) + physBody.rw;
				}
				float m = movement;
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					m -= extra;
					movement = extra;

					if( movement > 0 )
					{
						offset.x = physBody.rw;
					}
					else
					{
						offset.x = -physBody.rw;
					}
				}
				else
				{
					movement = 0;
					offset.x += m;
				}

				if(!approxEquals( m, 0 ) )
				{
					bool hit = ResolvePhysics( Vector2f( m, 0 ));
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						Vector2f eNorm = minContact.edge->Normal();
						if( eNorm.y < 0 )
						{
							//if( minContact.position.y >= position.y + physBody.rh - 5 )
							{
								if( m > 0 && eNorm.x < 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offset.x = -physBody.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offset.x = physBody.rw;
									continue;
								}
								

							}
						}
						else
						{
								offset.x += minContact.resolution.x;
								groundSpeed = 0;
								break;
						}
					}
				}

				}
				else
				{
					if( movement > 0 )
					{				
						extra = (-offset.x + movement) - physBody.rw;				
					}
					else 
					{
						extra = (-offset.x + movement) + physBody.rw;
					}
					float m = movement;
					if( (m > 0 && extra > 0) || (m < 0 && extra < 0) )
					{
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offset.x = -physBody.rw;
						}
						else
						{
							offset.x = physBody.rw;
						}
					}
					else
					{
						movement = 0;
						offset.x -= m;
					}

					if(!approxEquals( m, 0 ) )
					{
						bool hit = ResolvePhysics( Vector2f( -m, 0 ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							Vector2f eNorm = minContact.edge->Normal();
							if( eNorm.y < 0 )
							{
								//if( minContact.position.y >= position.y + physBody.rh - 5 )
								{
									if( m > 0 && eNorm.x < 0 )
									{
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										edgeQuantity = q;
										offset.x = -physBody.rw;
										continue;
									}
									else if( m < 0 && eNorm.x > 0 )
									{
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										edgeQuantity = q;
										offset.x = physBody.rw;
										continue;
									}
								

								}
							}
							else
							{
									offset.x += minContact.resolution.x;
									groundSpeed = 0;
									break;
							}
						}
					}
				}

			}
			else if( changeOffsetY )
			{
				cout << "change offset y" << endl;
				if( ( gNormal.x < 0 && e1n.y < 0 ) )// || (gNormal.x == 0 && gNormal.y > 0 ) )
				{
					cout << "changing offsety: " << offset.x << ", " << offset.y << endl;
					if( movement > 0 )
					{				
						extra = (-offset.y + movement) - physBody.rh;				
					}
					else 
					{
						extra = (-offset.y + movement) + physBody.rh;
					}
					float m = movement;

					if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
					{
						
						//m -= extra;
					
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offset.y = -physBody.rh;
						}
						else
						{
							offset.y = physBody.rh;
						}
					}
					else
					{
						
						//m = -m;
						movement = 0;
						offset.y -= m;
					}

					if(!approxEquals( m, 0 ) )
					{
						bool hit = ResolvePhysics( Vector2f( 0, -m ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							Vector2f eNorm = minContact.edge->Normal();

							ground = minContact.edge;

							q = ground->GetQuantity( minContact.position + minContact.resolution );
							edgeQuantity = q;

							Vector2f gn = ground->Normal();
							if( gn.x > 0 )
								offset.x = physBody.rw;
							else if( gn.x < 0 )
								offset.x = -physBody.rw;

							if( gn.y > 0 )
								offset.y = physBody.rh;
							else if( gn.y < 0 )
								offset.y = -physBody.rh;

							position = ground->GetPoint( edgeQuantity ) + offset;

							break;
						}
					}
				}
				else
				{
					
					if( movement > 0 )
					{				
						extra = (offset.y + movement) - physBody.rh;				
					}
					else 
					{
						extra = (offset.y + movement) + physBody.rh;
					}
					float m = movement;

					if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
					{
						//m -= extra;
					
						m -= extra;
						movement = extra;

						if( movement > 0 )
						{
							offset.y = physBody.rh;
						}
						else
						{
							offset.y = -physBody.rh;
						}
					}
					else
					{
						
						//m = -m;
						movement = 0;
						offset.y += m;
					}

					if(!approxEquals( m, 0 ) )
					{
						bool hit = ResolvePhysics( Vector2f( 0, m ));
						if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
						{
							Vector2f eNorm = minContact.edge->Normal();

							ground = minContact.edge;

							q = ground->GetQuantity( minContact.position + minContact.resolution );
							edgeQuantity = q;

							Vector2f gn = ground->Normal();
							if( gn.x > 0 )
								offset.x = physBody.rw;
							else if( gn.x < 0 )
								offset.x = -physBody.rw;

							if( gn.y > 0 )
								offset.y = physBody.rh;
							else if( gn.y < 0 )
								offset.y = -physBody.rh;

							position = ground->GetPoint( edgeQuantity ) + offset;

							break;
						}
					}
					else
					{
						cout << "problelm" << endl;
					}
				}
				
			}
			else
			{
				cout << "else" << endl;
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
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				

				if( m == 0 )
				{
					cout << "secret: " << endl;//<< gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					groundSpeed = 0;
					break;
				}

				if( !approxEquals( m, 0 ) )//	if(m != 0 )
				{	
					bool down = true;
					bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						Vector2f eNorm = minContact.edge->Normal();

						ground = minContact.edge;

						//q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
						q = ground->GetQuantity( minContact.position + minContact.resolution );
						//groundSpeed = 0;
						edgeQuantity = q;

						Vector2f gn = ground->Normal();
						if( gn.x > 0 )
							offset.x = physBody.rw;
						else if( gn.x < 0 )
							offset.x = -physBody.rw;
						else
						{
							offset.x = position.x + minContact.resolution.x - minContact.position.x;
							//offsetX = position.x + minContact.resolution.x - minContact.position.x;
						}

						if( gn.y > 0 )
							offset.y = physBody.rh;
						else if( gn.y < 0 )
							offset.y = -physBody.rh;
						else
						{
							offset.y = position.y + minContact.resolution.y - minContact.position.y;
						}

						position = ground->GetPoint( edgeQuantity ) + offset;

						break;
					}
						
				}	
			}

			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;
		}
	}
}

bool Crawler::ResolvePhysics( Vector2f vel )
{
	possibleEdgeCount = 0;

	Rect<float> oldR( position.x + physBody.offset.x - physBody.rw, 
		position.y + physBody.offset.y - physBody.rh, 2 * physBody.rw, 2 * physBody.rh );
	position += vel;
	
	Rect<float> newR( position.x + physBody.offset.x - physBody.rw, 
		position.y + physBody.offset.y - physBody.rh, 2 * physBody.rw, 2 * physBody.rh );
	//minContact.collisionPriority = 1000000;
	
	float oldRight = oldR.left + oldR.width;
	float right = newR.left + newR.width;

	float oldBottom = oldR.top + oldR.height;
	float bottom = newR.top + newR.height;

	float maxRight = max( right, oldRight );
	float maxBottom = max( oldBottom, bottom );
	float minLeft = min( oldR.left, newR.left );
	float minTop = min( oldR.top, newR.top );
	//Rect<float> r( minLeft - 5 , minTop - 5, maxRight - minLeft + 5, maxBottom - minTop + 5 );
	Rect<float> r( minLeft , minTop, maxRight - minLeft, maxBottom - minTop );

	
	minContact.collisionPriority = 1000000;

	

	tempVel = vel;

	col = false;
	minContact.edge = NULL;

	queryMode = "resolve";
	owner->terrainTree->Query( this, r );
	//Query( this, owner->testTree, r );

	return col;
}

void Crawler::UpdatePostPhysics()
{
	float spaceNeeded = 0;
	Vector2f gn = ground->Normal();
	Vector2f gPoint = ground->GetPoint( edgeQuantity );
	

	float angle = 0;
	
	if( !roll )
	{
		position = gPoint + gn * 16.f;
		angle = atan2( gn.x, -gn.y );
		
		sprite.setTexture( *ts_walk->texture );
		sprite.setTextureRect( ts_walk->GetSubRect( frame / crawlAnimationFactor ) );
		Vector2f pp = ground->GetPoint( edgeQuantity );
		sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
		sprite.setRotation( angle / PI * 180 );
		sprite.setPosition( pp.x, pp.y );

	}
	else
	{
		Vector2f e1n = ground->edge1->Normal();
		float rollStart = atan2( gn.y, gn.x );
		float rollEnd = atan2( e1n.y, e1n.x );
		float adjRollStart = rollStart;
		float adjRollEnd = rollEnd;

		if( rollStart < 0 )
			adjRollStart += 2 * PI;
		if( rollEnd < 0 )
			adjRollEnd += 2 * PI;


	/*	float angleDist = rollEnd - rollStart;

		if( rollEnd < rollStart )
		{
			angleDist = ( 2 * PI - rollStart ) + rollEnd;
		}
*/
		
		if( adjRollEnd > adjRollStart )
		{
			angle  = adjRollStart * ( 1.f - rollFactor ) + adjRollEnd  * rollFactor ;
			
			//angle = -angle;
		}
		else
		{
			
			angle = rollStart * ( 1.f - rollFactor ) + rollEnd  * rollFactor;

			if( rollStart < 0 )
				rollStart += 2 * PI;
			if( rollEnd < 0 )
				rollEnd += 2 * PI;
			//angle = -angle;
		}
		//angle = rollStart * ( 1.f - rollFactor ) + rollEnd  * rollFactor ;
		if( angle < 0 )
			angle += PI * 2;
		//angle = -angle;
		//angle -= PI / 2;
		

		
		

		

		Vector2f angleVec = Vector2f( cos( angle ), sin( angle ) );
		angleVec = normalize( angleVec );

		position = gPoint + angleVec * 16.f;

		angle += PI / 2.f;
	
			
		
		//cout << "rollStart: " << adjRollStart << ", rollEnd: " << adjRollEnd << ", factor: " << rollFactor << ", angle: " << angle << endl;

		sprite.setTexture( *ts_roll->texture );
		sprite.setTextureRect( ts_roll->GetSubRect( frame / rollAnimationFactor ) );

		

		sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
		sprite.setRotation( angle / PI * 180 );
		Vector2f pp = ground->GetPoint( edgeQuantity );
		//pp += angleVec * 16.f;
		sprite.setPosition( pp.x, pp.y );
		//sprite.setPosition( position.x, position.y );
		
	}
	if( slowCounter == slowMultiple )
	{
		++frame;
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

	//sprite.setPosition( position );
	UpdateHitboxes();
}

bool Crawler::PlayerSlowingMe()
{
	return false;
}

void Crawler::Draw(sf::RenderTarget *target )
{
	target->draw( sprite );
}

bool Crawler::IHitPlayer()
{
	return false;
}

bool Crawler::PlayerHitMe()
{
	return false;
}

void Crawler::UpdateSprite()
{
}

void Crawler::DebugDraw( RenderTarget *target )
{
	CircleShape cs;
	cs.setFillColor( Color::Cyan );
	cs.setRadius( 10 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	Vector2f g = ground->GetPoint( edgeQuantity );
	cs.setPosition( g.x, g.y );

	owner->window->draw( cs );

	physBody.DebugDraw( target );
//	hurtBody.DebugDraw( target );
//	hitBody.DebugDraw( target );
}

void Crawler::SaveEnemyState()
{
}

void Crawler::LoadEnemyState()
{
}
