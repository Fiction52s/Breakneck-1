#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Crawler.h"

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

Crawler::Crawler( GameSession *owner, bool p_hasMonitor, Edge *g, double q, bool cw, int s, int p_framesUntilBurrow )
	:Enemy( owner, EnemyType::CRAWLER, p_hasMonitor, 1 ), ground( g ), edgeQuantity( q ), clockwise( cw ), groundSpeed( s )
{
	clockwise = true;
	maxFramesUntilBurrow = p_framesUntilBurrow;
	maxFramesUntilBurrow = 200;
	framesUntilBurrow = maxFramesUntilBurrow;

	dashAccel = 3;
	currDistTravelled = 0;
	//cout << "inside crawler" << endl;
	//if( hasMonitor )
	//{
	//	cout << "HAS MONMITOR NOW" << endl;
	//}
	//else
	//{
	//	cout << "no monitor" << endl;
	//}
	mover = new SurfaceMover(owner, g, q, 32 );
	mover->surfaceHandler = this;
	//testMover->gravity = V2d( 0, .5 );
	mover->SetSpeed(0);

	initHealth = 60;
	health = initHealth;
	//lastReverser = false;
	dead = false;
	deathFrame = 0;
	//ts_walk = owner->GetTileset( "crawlerwalk.png", 96, 64 );
	//ts_roll = owner->GetTileset( "crawlerroll.png", 96, 64 );
	attackFrame = -1;
	attackMult = 3;

	double height = 128;
	double width = 128;
	ts = owner->GetTileset( "crawler_128x128.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = mover->ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	V2d gNorm = mover->ground->Normal();

	double angle = atan2( gNorm.x, -gNorm.y );
	sprite.setRotation( angle / PI * 180.f );
	roll = false;
	position = gPoint + gNorm * height / 2.0;
	



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
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	crawlAnimationFactor = 2;
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
	//mover->SetSpeed(groundSpeed);
	//action = CRAWL;

	actionLength[UNBURROW] = 60;
	actionLength[CRAWL] = 35 * crawlAnimationFactor;
	actionLength[STARTROLL] = 3 * crawlAnimationFactor;
	actionLength[ROLL] = 13 * crawlAnimationFactor;
	actionLength[ENDROLL] = 4 * crawlAnimationFactor;
	actionLength[DASH] = 7 * crawlAnimationFactor;
	actionLength[BURROW] = 20;//3 * crawlAnimationFactor;
	actionLength[UNDERGROUND] = 60;//3 * crawlAnimationFactor;
	actionLength[DYING] = 1;//3 * crawlAnimationFactor;

	action = UNDERGROUND;
	frame = actionLength[UNDERGROUND];
	/*if( hasMonitor )
	{
		cout << "HAS MONMITOR NOW END" << endl;
	}
	else
	{
		cout << "no monitor END" << endl;
	}*/
	//
	//hasMonitor = false;
	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//ts_testBlood = owner->GetTileset( "fx_blood_1_256x256.png", 256, 256 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	//ts_hitSpack = owner->GetTileset( "hit_spack_1_128x128.png", 128, 128 );
}

void Crawler::ResetEnemy()
{
	framesUntilBurrow = maxFramesUntilBurrow;
	
	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);
	//mover->SetSpeed(groundSpeed);

	position = mover->physBody.globalPosition;

	currDistTravelled = 0;
	health = initHealth;
	attackFrame = -1;
	roll = false;
	ground = startGround;
	edgeQuantity = startQuant;
	V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	frame = 0;
	position = gPoint + ground->Normal() * 64.0 / 2.0;

	V2d gn = mover->ground->Normal();
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
	V2d pp = mover->ground->GetPoint(mover->edgeQuantity );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( pp.x, pp.y );
	//----

	UpdateHitboxes();

	action = UNDERGROUND;
	frame = actionLength[UNDERGROUND];
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
	hitBody.globalPosition = mover->physBody.globalPosition;
	hurtBody.globalPosition = mover->physBody.globalPosition;//position;
	physBody.globalPosition = mover->physBody.globalPosition;//position;//+ V2d( -16, 0 );// + //physBody.offset + offset;
}

void Crawler::UpdatePrePhysics()
{
	if( action == DYING )
		return;

	if( !dead && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
			{
				owner->keyMarker->CollectKey();
				///
				//owner->GetPlayer( 0 )->CaptureMonitor( monitor );
			}
			//AttemptSpawnMonitor();
			dead = true;
			owner->GetPlayer( 0 )->ConfirmEnemyKill( this );
		}
		else
		{
			owner->GetPlayer( 0 )->ConfirmEnemyNoKill( this );
		}
			
		receivedHit = NULL;
	}
	V2d en = mover->ground->Normal();

	if (action != BURROW && action != UNDERGROUND )
	{
		if (framesUntilBurrow == 0)
		{
			action = BURROW;
			frame = 0;
			mover->SetSpeed(0);
		}
	}

	if (frame == actionLength[action])
	//if( false )
	{
		switch (action)
		{
		case UNBURROW:
			action = CRAWL;
			frame = 0;
			if (clockwise)
			{
				mover->SetSpeed(groundSpeed);
			}
			else
			{
				mover->SetSpeed(-groundSpeed);
			}
			break;
		case CRAWL:
			frame = 0;
			break;
		case STARTROLL:
			action = ROLL;
			frame = 0;
			break;
		case ROLL:
			frame = 0;
			break;
		case ENDROLL:
			action = CRAWL;
			frame = 0;
			break;
		case DASH:
			if (mover->groundSpeed > 0 && (en.x > 0 && en.y < 0)
				|| (mover->groundSpeed < 0 && en.x < 0 && en.y < 0))
			{
				action = STARTROLL;
				frame = 0;
			}
			else
			{
				action = CRAWL;
				frame = 0;
				//mover->SetSpeed(groundSpeed);
			}
			break;
		case BURROW:
			action = UNDERGROUND;
			frame = 0;
			mover->ground = startGround;
			mover->edgeQuantity = startQuant;
			mover->roll = false;
			mover->UpdateGroundPos();
			break;
		case UNDERGROUND:			
			action = UNBURROW;
			framesUntilBurrow = maxFramesUntilBurrow;
			frame = 0;
			if (!PlayerInFront())
			{
				clockwise = !clockwise;
			}
			break;
		case DYING:
			assert(0);
			break;
		}
	}
	

	switch (action)
	{
	case UNBURROW:
		break;
	case CRAWL:
		break;
	case DASH:
		break;
	case BURROW:
		break;
	case DYING:
		break;
	}

	
	switch (action)
	{
	case UNBURROW:
		break;
	case CRAWL:
		TryDash();
		break;
	case STARTROLL:
		TryDash();
		break;
	case ROLL:
		TryDash();
		{
			V2d gn = mover->ground->Normal();
			double acc = .08;
			
			if ((mover->groundSpeed > 0 && gn.x < 0) || (mover->groundSpeed < 0 && gn.x > 0) )
			{
				Accelerate(acc);
			}
		}
		break;
	case ENDROLL:
		break;
	case DASH:
		break;
	case BURROW:
		break;
	case DYING:
		break;
	}
}

void Crawler::UpdatePhysics()
{
	specterProtected = false;
	if( dead )
	{
		return;
	}

	mover->Move(slowMultiple);
	position = mover->physBody.globalPosition;

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
				owner->GetPlayer( 0 )->ConfirmHit( 1, 5, .8, 6 );

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

		//gotta get the correct angle upon death
		//Transform t;
		//t.rotate( angle / PI * 180 );
		//Vector2f newPoint = t.transformPoint( Vector2f( 1, -1 ) );
		deathVector = V2d(0, 0);//V2d( newPoint.x, newPoint.y );

		queryMode = "reverse";

		//physbody is a circle
		//Rect<double> r( position.x - physBody.rw, position.y - physBody.rw, physBody.rw * 2, physBody.rw * 2 );
		//owner->crawlerReverserTree->Query( this, r );
	}
}

void Crawler::UpdatePostPhysics()
{
	if( deathFrame == 30 )
	{
		//owner->ActivateEffect( ts_testBlood, position, true, 0, 15, 2, true );
		owner->RemoveEnemy( this );
		return;
	}

	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->GetPlayer( 0 )->position + position ) / 2.0, true, 0, 10, 2, true );
	}

	

	if( deathFrame == 0 && dead )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}

	UpdateSprite();


	if( slowCounter == slowMultiple )
	{
		//++keyFrame;
		++frame;
		if( framesUntilBurrow > 0 )
			--framesUntilBurrow;
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
	Actor *player = owner->GetPlayer( 0 );
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
		if (action != UNBURROW && action != BURROW && action != UNDERGROUND)
		{
			if (hasMonitor && !suppressMonitor)
			{
				if (owner->pauseFrames < 2 || receivedHit == NULL)
				{
					target->draw(sprite, keyShader);
				}
				else
				{
					target->draw(sprite, hurtShader);
				}
				target->draw(*keySprite);
			}
			else
			{
				if (owner->pauseFrames < 2 || receivedHit == NULL)
				{
					target->draw(sprite);
				}
				else
				{
					target->draw(sprite, hurtShader);
				}

			}
		}
		else
		{
			if (action != UNDERGROUND)
			{
				sf::RectangleShape rs;
				rs.setFillColor(Color::Red);
				rs.setSize(Vector2f(64, 64));
				rs.setOrigin(rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2);
				rs.setPosition(Vector2f(mover->physBody.globalPosition));
				target->draw(rs);
			}
		}
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

	if( hasMonitor && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/

	if( !dead )
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

bool Crawler::IHitPlayer( int index )
{
	Actor *player = owner->GetPlayer( 0 );
	
	if( action != UNDERGROUND && player->invincibleFrames == 0 && hitBody.Intersects( player->hurtBody ) )
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

 pair<bool, bool> Crawler::PlayerHitMe( int index )
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
		IntRect ir = ts->GetSubRect( 62 );
		if( !clockwise )
		{
			ir = IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height );
		}

		botDeathSprite.setTextureRect( ir );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2);
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts->texture );
		ir = ts->GetSubRect( 61 );
		if( !clockwise )
		{
			ir = IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height );
		}
		topDeathSprite.setTextureRect( ir );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );

		
	}
	else
	{
		//cout << "response" << endl;
		double spaceNeeded = 0;
		V2d gn = mover->ground->Normal();
		V2d gPoint = mover->ground->GetPoint(mover->edgeQuantity);

		//return;

		double angle = 0;

		IntRect ir;
		switch (action)
		{
		case CRAWL:
			ir = ts->GetSubRect(frame / crawlAnimationFactor);
			break;
		case STARTROLL:
			ir = ts->GetSubRect(frame / crawlAnimationFactor + 35);
			break;
		case ROLL:
			ir = ts->GetSubRect(frame / crawlAnimationFactor + 38 );
			break;
		case ENDROLL:
			ir = ts->GetSubRect(frame / crawlAnimationFactor + 50);
			break;
		case DASH:
			ir = ts->GetSubRect(frame / crawlAnimationFactor + 54);
			break;
		}

		if (!clockwise)
		{
			sprite.setTextureRect(sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height));
		}
		else
		{
			sprite.setTextureRect(ir);
		}

		if (!mover->roll)
		{
			//position = gPoint + gn * 32.0;
			angle = atan2(gn.x, -gn.y);

			//			sprite.setTexture( *ts_walk->texture );
			/*IntRect r = ts->GetSubRect(frame / crawlAnimationFactor);
			if (!clockwise)
			{
				sprite.setTextureRect(sf::IntRect(r.left + r.width, r.top, -r.width, r.height));
			}
			else
			{
				sprite.setTextureRect(r);
			}*/

			V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation(angle / PI * 180);
			sprite.setPosition(pp.x, pp.y);
		}
		else
		{
			if (clockwise)
			{
				V2d vec = normalize(position - mover->ground->v1);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				/*IntRect r = ts->GetSubRect(frame / rollAnimationFactor + 35);
				if (clockwise)
				{
					sprite.setTextureRect(r);
				}
				else
				{
					sprite.setTextureRect(sf::IntRect(r.left + r.width, r.top, -r.width, r.height));
				}*/

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation(angle / PI * 180);
				V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
				sprite.setPosition(pp.x, pp.y);
			}
			else
			{
				V2d vec = normalize(position - mover->ground->v0);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				/*IntRect r = ts->GetSubRect(frame / rollAnimationFactor + 35);
				if (clockwise)
				{
					sprite.setTextureRect(r);
				}
				else
				{
					sprite.setTextureRect(sf::IntRect(r.left + r.width, r.top, -r.width, r.height));
				}*/

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation(angle / PI * 180);
				V2d pp = mover->ground->GetPoint(mover->edgeQuantity);
				sprite.setPosition(pp.x, pp.y);
			}
		}


		//if( attackFrame >= 0 )
		//{
		//	IntRect r = ts->GetSubRect( 54 + attackFrame / attackMult );
		//	if( !clockwise )
		//	{
		//		r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
		//	}
		//	sprite.setTextureRect( r );
		//}

		//if( keySprite != NULL && hasMonitor && !suppressMonitor )
		//{
		//	//cout << "frame: " << keyFrame / 2 << endl;
		//	keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
		//	keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
		//		keySprite->getLocalBounds().height / 2 );
		//	keySprite->setPosition( position.x, position.y );
		//}
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

void Crawler::TransferEdge(Edge *e)
{
	V2d en = e->Normal();
	if ( mover->groundSpeed > 0 && (en.x > 0 && en.y < 0) 
		|| ( mover->groundSpeed < 0 && en.x < 0 && en.y < 0))
	{
		if (action != ROLL && action != DASH )
		{
			action = ROLL;
			frame = 0;
		}
	}
	else if (action == ROLL)
	{
		action = ENDROLL;
		frame = 0;

		//mover->SetSpeed(groundSpeed);
		/*if (mover->groundSpeed > 0)
		{
			mover->SetSpeed(groundSpeed);
		}
		else if (mover->groundSpeed < 0)
		{
			mover->SetSpeed(-groundSpeed);
		}*/
	}
}

bool Crawler::ShouldDash()
{
	if (length(owner->GetPlayer(0)->position - position) < 200)
	{
		
		
		if (PlayerInFront())
			return true;
	}
	return false;
}

bool Crawler::PlayerInFront()
{
	V2d dir;
	if (clockwise )
	{
		dir = normalize(mover->ground->v1 - mover->ground->v0);
	}
	else if (!clockwise )
	{
		dir = normalize(mover->ground->v0 - mover->ground->v1);
	}
	double alongDist = dot(owner->GetPlayer(0)->position - mover->physBody.globalPosition, dir);
	if (alongDist > -60)
		return true;
	else
		return false;
}

void Crawler::Accelerate(double amount)
{
	if ( clockwise )
	{
		amount = abs(amount);
	}
	else
	{
		amount = -abs(amount);
	}
	mover->SetSpeed( mover->groundSpeed + amount );
}

void Crawler::SetForwardSpeed( double speed )
{
	double aSpeed = abs(speed);
	if (clockwise)
	{
		mover->SetSpeed(aSpeed);
	}
	else
	{
		mover->SetSpeed(-aSpeed);
	}
}

void Crawler::SetActionDash()
{
	action = DASH;
	frame = 0;

	Accelerate(dashAccel);
}

bool Crawler::TryDash()
{
	if (ShouldDash())
	{
		SetActionDash();
		return true;
	}

	return false;
}