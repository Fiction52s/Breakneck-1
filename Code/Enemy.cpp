#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Zone.h"
#include <sstream>
#include "Boss.h"

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

EnemyParams * Enemy::enemyTypeHitParams[] = { 0 };

Bullet * CreateBullet( BulletType::Type type, int vaIndex, Launcher *launcher )
{
	using namespace BulletType;
	switch( type )
	{
	case NORMAL:
		break;
	case SIN:
		break;
	case BIRDBOSS:
		break;
	};

	return NULL;
}

Launcher::Launcher( LauncherEnemy *p_handler, BasicBullet::BType p_bulletType,
	GameSession *p_owner,
		int numTotalBullets,
		int bulletsPerShot,
		sf::Vector2<double> p_position,
		sf::Vector2<double> direction,
		double p_angleSpread,
		int p_maxFramesToLive,
		bool hitTerrain,
		int p_wavelength,
		double p_amplitude )
		:totalBullets( numTotalBullets ), perShot( bulletsPerShot ),
		facingDir( direction ), angleSpread( p_angleSpread ), 
		position( p_position ), owner( p_owner ),handler(p_handler),
		def_e ( NULL )
{
	skipPlayerCollideForSubstep = false;
	bulletType = p_bulletType;
	maxBulletSpeed = 100;
	//launchType = p_launchType;
	interactWithTerrain = hitTerrain;
	maxFramesToLive = p_maxFramesToLive;
	wavelength = p_wavelength;
	amplitude = p_amplitude;
	//increment the global counter
	 //+= numTotalBullets;
	int startIndex = owner->totalNumberBullets;

	activeBullets = NULL;

	switch( bulletType )
	{
	case BasicBullet::BASIC_TURRET:
		bulletTilesetIndex = 0;
		break;
	case BasicBullet::BAT:
		bulletTilesetIndex = 2;
		break;
	case BasicBullet::CURVE_TURRET:
		bulletTilesetIndex = 1;
		break;
	case BasicBullet::CACTUS_TURRET:
		bulletTilesetIndex = 1;
		break;
	case BasicBullet::OWL:
		bulletTilesetIndex = 1;
		break;
	case BasicBullet::TURTLE:
		bulletTilesetIndex = 1;
		break;
	case BasicBullet::GROWING_TREE:
		bulletTilesetIndex = 1;
		break;
	case BasicBullet::COPYCAT:
		bulletTilesetIndex = 1;
	case BasicBullet::BOSS_CRAWLER:
		bulletTilesetIndex = 2;
		break;
	case BasicBullet::BOSS_BIRD:
		bulletTilesetIndex = 1;
		break;

	}
	
	if( bulletType == BasicBullet::COPYCAT )
	{
		inactiveBullets = new CopycatBullet( startIndex++, this );
	}
	else
	{
		if( wavelength > 0 )
		{
			inactiveBullets = new SinBullet( startIndex++, this );
		}
		else
		{

			inactiveBullets = new BasicBullet( startIndex++, bulletType, this );
		}
	}
		

	for( int i = 1; i < numTotalBullets; ++i )
	{
		BasicBullet * temp;
		if( bulletType == BasicBullet::COPYCAT )
		{
			temp = new CopycatBullet( startIndex++, this );
		}
		else
		{
			if( wavelength > 0 )
			{
				temp = new SinBullet( startIndex++, this );
			}
			else
			{
				temp = new BasicBullet( startIndex++, bulletType, this );
			}
		}

		
		temp->next = inactiveBullets;
		inactiveBullets->prev = temp;
		inactiveBullets = temp;
	}

	owner->totalNumberBullets = startIndex;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = .3;
	hitboxInfo->drainY = .3;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;
}

double Launcher::GetRadius(BasicBullet::BType bt)
{
	switch (bt)
	{
	case BasicBullet::BASIC_TURRET:
		return 12;
	}

	return 10;
}

Launcher::Launcher( LauncherEnemy *handler, GameSession *owner,
	int p_maxFramesToLive )
{
	maxBulletSpeed = 100;
	skipPlayerCollideForSubstep = false;
}

void Launcher::SetDefaultCollision(int framesToLive, Edge*e, V2d &pos)
{
	def_framesToLive = framesToLive;
	def_e = e;
	def_pos = pos;
}

void Launcher::CapBulletVel( double speed )
{
	BasicBullet *curr = activeBullets;
	BasicBullet *temp;
	while( curr != NULL )
	{
		temp = curr->next;
		V2d norm = normalize( curr->velocity );
		if( length( curr->velocity ) > speed )
		{
			curr->velocity = norm * speed;
		}
		
		//curr->UpdatePrePhysics();
		curr = temp;
	}
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

	CapBulletVel( maxBulletSpeed );
}

void Launcher::UpdatePhysics( int substep, bool lowRes )
{
	BasicBullet *curr = activeBullets;
	BasicBullet *temp;
	while( curr != NULL )
	{
		if (lowRes)
		{
			curr->numPhysSteps = NUM_STEPS;
		}
		temp = curr->next;
	//	cout << "updating bullet physics" << endl;
		if (substep < curr->numPhysSteps)
		{
			curr->UpdatePhysics();
		}
		curr = temp;
	}

	skipPlayerCollideForSubstep = false;
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
	
	V2d dir = facingDir;
	//cout << "shooting dir: " << dir.x << ", " << dir.y << endl;
	double dirAngle = atan2( facingDir.x, -facingDir.y );
	if( dirAngle < 0 )
	{
		dirAngle += PI * 2.0;
	}
	dirAngle -= angleSpread / 2;
	for( int i = 0; i < perShot; ++i )
	{
		dir = V2d(cos(dirAngle - PI / 2.0), sin(dirAngle - PI / 2.0));
		//cout << "trying to activate bullet" << endl;
		BasicBullet * b = ActivateBullet();
		//cout << "bullet done activating" << endl;
		if( b != NULL )
		{
			//cout << "FIRE" << endl;
			b->Reset( position, dir * bulletSpeed );
		}

		dirAngle += angleSpread / (perShot-1);
		//dir = V2d( cos( dirAngle - PI / 2.0 ), sin( dirAngle - PI / 2.0 ) );
	}
}

void Launcher::Fire( double gravStrength )
{
	//for( int i = 0; i < perShot; ++i )
	//{
	//	//cout << "trying to activate bullet" << endl;
	//	BasicBullet * b = ActivateBullet();
	//	//cout << "bullet done activating" << endl;
	//	if( b != NULL )
	//	{
	//		//cout << "FIRE" << endl;
	//		b->Reset( position, facingDir * bulletSpeed );
	//		b->gravTowardsPlayer = true;
	//		b->gravity = V2d( 0, 1 );
	//	}
	//}
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

void BasicBullet::Reset( V2d &pos, V2d &vel )
{
	double angle = atan2( vel.y, vel.x );
	angle = angle * 180 / PI;

	//cout << "angle: " << angle << endl;
	
	transform = transform.Identity;
	switch( bulletType )
	{
	case BAT:
		transform.rotate( angle );
		break;
	case CURVE_TURRET:
		{
			double gangle = atan2( gravity.y, gravity.x );
			gangle = gangle * 180 / PI;
			transform.rotate( gangle );
			break;
		}
	case BASIC_TURRET:
		transform.rotate( angle );
		break;
	case BOSS_BIRD:
		{
			transform.rotate( angle );
			break;
		}
		
	}

	frame = 0;
	//gravTowardsPlayer = false;
	position = pos;
	velocity = vel;
	framesToLive = launcher->maxFramesToLive;
	slowMultiple = 1;
	slowCounter = 1;
	bounceCount = 0;

	VertexArray &bva = *(launcher->owner->bigBulletVA);
	bva[index*4+0].position = Vector2f( 0, 0 );
	bva[index*4+1].position = Vector2f( 0, 0 );
	bva[index*4+2].position = Vector2f( 0, 0 );
	bva[index*4+3].position = Vector2f( 0, 0 );
	//transform.
}

BasicBullet::BasicBullet( int indexVA, BType bType, Launcher *launch )
	:index( indexVA ), launcher( launch ), next( NULL ), prev( NULL ),
	bulletType( bType )
{
	frame = 0;
	switch( bType )
	{
	case BASIC_TURRET:
		break;
	case BAT:
		break;
	case CURVE_TURRET:
		break;
	}
	//framesToLive = maxFram
	double rad = Launcher::GetRadius( bType );
	bounceCount = 0;
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
	frame = 0;
	VertexArray &bva = *(launcher->owner->bigBulletVA);
	bva[index*4+0].position = Vector2f( 0, 0 );
	bva[index*4+1].position = Vector2f( 0, 0 );
	bva[index*4+2].position = Vector2f( 0, 0 );
	bva[index*4+3].position = Vector2f( 0, 0 );
}

//bool BasicBullet::PlayerSlowingMe()
//{
//	Actor *player = launcher->owner->GetPlayer( 0 );
//	for( int i = 0; i < player->maxBubbles; ++i )
//	{
//		if( player->bubbleFramesToLive[i] > 0 )
//		{
//			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
//			{
//				return true;
//			}
//		}
//	}
//	return false;
//}

void BasicBullet::UpdatePrePhysics()
{
	
	/*if( PlayerSlowingMe() )
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
	}*/

	velocity += gravity / (double)slowMultiple;
	V2d playerPos = launcher->owner->GetPlayer(0)->position;

	if( launcher->bulletType == BasicBullet::BOSS_BIRD )
	{
		int f = launcher->maxFramesToLive - framesToLive;
		
		//V2d endFly = launcher->owner->b_bird->endFly;
		V2d dir = normalize( velocity );
		V2d norm( dir.y, -dir.x );
		//angle = atan2( gn.x, -gn.y );
		double len = length( velocity );
		//double angle = atan2( dir.y, dir.x );//norm.x, -norm.y );
		//angle += PI / len;
		//V2d go( len, 0 );
		//velocity.x = cos( angle ) * len;
		//velocity.y = sin( angle ) * len;
		
		//velocity += norm * dot(normalize( playerPos - position ) * .5 / (double)slowMultiple, norm);
		if( f < 30 )
			velocity += normalize( playerPos - position ) * 2.0 / (double)slowMultiple;

		//cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
		//velocity += 
	}

	double len = length(position - playerPos);
	if (len > MAX_VELOCITY * 2)
	{
		numPhysSteps = NUM_STEPS;
	}
	else
	{
		numPhysSteps = NUM_MAX_STEPS;
		launcher->owner->GetPlayer(0)->highAccuracyHitboxes = true;
	}
}

void BasicBullet::UpdatePostPhysics()
{
	if( slowCounter == slowMultiple )
	{
		frame++;
		framesToLive--;
		//cout << "frames to live: " << framesToLive << endl;
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
	

	if( framesToLive == 0 && slowCounter == 1 )
	{
		
		//cout << "time out!" << endl;
		//explode
		if( launcher->handler != NULL )
			launcher->handler->BulletTTLDeath(this);

		launcher->DeactivateBullet( this );
		//parent->DeactivateTree( this );
		//owner->RemoveEnemy( this );
		//return;
	}
}

void BasicBullet::UpdatePhysics()
{

	V2d movement = velocity / numPhysSteps / (double)slowMultiple;

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

		if (!launcher->skipPlayerCollideForSubstep)
		{
			Actor *player = launcher->owner->GetPlayer(0);
			if (player->hurtBody.Intersects(hitBody) && player->invincibleFrames == 0)
			{
				//cout << "hit??" << endl;
				HitPlayer();
				break;
			}
		}
	}
	while( movementLen > 0 );

	if (!col && launcher->def_e != NULL && framesToLive == launcher->def_framesToLive )
	{
		launcher->handler->BulletHitTerrain(this,
			launcher->def_e, launcher->def_pos);
		col = true;
		//int x = 5;
	}
	//bool slowed = PlayerSlowingMe();
}

bool BasicBullet::HitTerrain()
{
	launcher->handler->BulletHitTerrain( this,
		minContact.edge, minContact.position );
	return true;
}

void BasicBullet::HitPlayer()
{
	launcher->handler->BulletHitPlayer( this );
	//launcher->DeactivateBullet( this );
}

bool BasicBullet::ResolvePhysics( V2d vel )
{
	if (launcher->interactWithTerrain && !col )
	{
		Rect<double> oldR(position.x - physBody.rw, position.y - physBody.rw,
			2 * physBody.rw, 2 * physBody.rw);

		position += vel;

		Rect<double> newR(position.x - physBody.rw, position.y - physBody.rw,
			2 * physBody.rw, 2 * physBody.rw);

		double oldRight = oldR.left + oldR.width;
		double right = newR.left + newR.width;

		double oldBottom = oldR.top + oldR.height;
		double bottom = newR.top + newR.height;

		double maxRight = max(right, oldRight);
		double maxBottom = max(oldBottom, bottom);
		double minLeft = min(oldR.left, newR.left);
		double minTop = min(oldR.top, newR.top);


		double ex = 1;
		Rect<double> r(minLeft - ex, minTop - ex,
			(maxRight - minLeft) + ex * 2, (maxBottom - minTop) + ex * 2);
		//Rect<double> realRect( min( oldR.left, r.left ),
		//	min( oldR.top, r.top ));

		minContact.collisionPriority = 1000000;

		col = false;

		tempVel = vel;
		minContact.edge = NULL;

		//queryMode = "resolve";
	//	Query( this, owner->testTree, r );
		//queryBullet = bullet;



		launcher->owner->terrainTree->Query(this, r);
	}
	else
	{
		col = false;
		position += vel;
	}

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
	Vector2f dims( 32, 32 );
	//Vector2f dims = Vector2f( ir.width / 2, ir.height / 2 );

	Vector2f center( position.x, position.y );

	Vector2f topLeft = -dims;
	Vector2f topRight = Vector2f( dims.x, -dims.y );
	Vector2f botRight = dims;
	Vector2f botLeft = Vector2f( -dims.x, dims.y );


	VA[index*4+0].position = center + transform.transformPoint( topLeft );
	VA[index*4+1].position = center + transform.transformPoint( topRight );
	VA[index*4+2].position = center + transform.transformPoint( botRight );
	VA[index*4+3].position = center + transform.transformPoint( botLeft );

	/*VA[index*4+0].texCoords = Vector2f( ir.left, ir.top );
	VA[index*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
	VA[index*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
	VA[index*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );*/

	int ind = 6 * launcher->bulletTilesetIndex + ((frame/2) % 6);
	//cout << "index: " << ind << ", frame: " << frame << endl;
	IntRect sub = launcher->owner->ts_basicBullets->GetSubRect( ind );
	/*VA[index*4+0].color = Color::Red;
	VA[index*4+1].color = Color::Red;
	VA[index*4+2].color = Color::Red;
	VA[index*4+3].color = Color::Red;*/

	VA[index*4+0].texCoords = Vector2f( sub.left, sub.top );
	VA[index*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top );
	VA[index*4+2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
	VA[index*4+3].texCoords = Vector2f( sub.left, sub.top + sub.height );
}

SinBullet::SinBullet( int indexVA, Launcher *launcher )
	:BasicBullet( indexVA, BasicBullet::CACTUS_TURRET, launcher )
{
}

void SinBullet::UpdatePrePhysics()
{
	/*if( PlayerSlowingMe() )
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
	}*/

	//cout << "framestolive: " << framesToLive << endl;
	//cout << "position: " << position.x << ", " << position.y << endl;
	position -= tempadd;

	int waveLength = launcher->wavelength * 5;
	int ftl = framesToLive * 5;
	int maxFrames = launcher->maxFramesToLive * 5;
	//launcher->wavelength * slowMultiple;
	int tempFrame = (maxFrames - ftl) % waveLength;
	tempFrame += ( slowCounter - 1 );
	double test = tempFrame / (double)( waveLength );
	
	//cout << "temp: " << tempFrame << ", wl: " << waveLength << ", test: " << test << endl;
	
	double t = test * 2 * PI;//2.0 * PI;
	//double c = cos( t );
	//V2d dir( cos( t ) - sin( t ), sin( t ) + cos( t ) );
	V2d dir( 0, sin( t ) );
	//cout << "test: " << test <<  ", t: " << t << ", temp: " << tempFrame << ", wl: " << waveLength << ", diry: " << dir.y << endl;

	V2d other = normalize( velocity );
	other = V2d( other.y, -other.x );
	//dir = normalize( velocity );

	double d = dot( dir, other );
	tempadd = d * launcher->amplitude * other;//d * other * launcher->amplitude;

	position += tempadd;

	//tempadd = dir * 100.0;
}

void SinBullet::UpdatePhysics()
{
	V2d movement = velocity / numPhysSteps / (double)slowMultiple;

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

		position += move * moveDir;
		/*bool hit = ResolvePhysics( moveDir * move );
		if( hit )
		{
			HitTerrain();
			break;
		}*/

		hitBody.globalPosition = position;
		hurtBody.globalPosition = position;

		Actor *player = launcher->owner->GetPlayer( 0 );
		if( player->hurtBody.Intersects( hitBody ) )
		{
			HitPlayer();
			break;
		}
	}
}

void SinBullet::Reset( sf::Vector2<double> &pos,
	sf::Vector2<double> &vel )
{
	BasicBullet::Reset( pos, vel );
	tempadd = V2d( 0, 0 );
}


CopycatBullet::CopycatBullet( int indexVA, Launcher *launcher )
	:BasicBullet( indexVA, BasicBullet::COPYCAT, launcher )
{
	//bulletSpeed =
	speed = 10;
	//launcher->bulletSpeed = speed;
}

void CopycatBullet::UpdatePrePhysics()
{
	/*if( PlayerSlowingMe() )
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
	}*/
}

void CopycatBullet::UpdatePhysics()
{
	V2d movement = velocity / numPhysSteps / (double)slowMultiple;

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

		if( length( destination - position ) <= move )
		{
			position = destination;
			velocity = V2d( 0, 0 );
			launcher->handler->BulletHitTarget( this );
			return;
		}

		position += move * moveDir;
		/*bool hit = ResolvePhysics( moveDir * move );
		if( hit )
		{
			HitTerrain();
			break;
		}*/

		hitBody.globalPosition = position;
		//hurtBody.globalPosition = position;

		/*Actor *player = launcher->owner->GetPlayer( 0 );
		if( player->hurtBody.Intersects( hitBody ) )
		{
			HitPlayer();
			break;
		}*/
	}
}

void CopycatBullet::Reset( sf::Vector2<double> &pos0,
	sf::Vector2<double> &pos1 )
{
	if( bulletType == BasicBullet::COPYCAT )
	{
		attackIndex = launcher->handler->GetAttackIndex();
	}
	destination = pos1;
	trueVel = normalize( pos1 - pos0 ) * speed;
	BasicBullet::Reset( pos0, trueVel );
	//tempadd = V2d( 0, 0 );
}

Enemy::Enemy( GameSession *own, EnemyType t, bool p_hasMonitor,
	int world, bool cuttable )
	:owner( own ), prev( NULL ), next( NULL ), spawned( false ),
	type( t ),zone( NULL ), dead( false ),
	suppressMonitor( false ), ts_hitSpack( NULL ), keyShader( NULL ),
	affectCameraZoom( true )
{
	ts_zoned = owner->GetTileset("Enemies/zonedenemy_64x64.png", 64, 64);
	zonedSprite.setTexture(*ts_zoned->texture);
	

	highResPhysics = false;
	numLaunchers = 0;
	launchers = NULL;
	currHitboxes = NULL;
	currHurtboxes = NULL;
	EnemyParams *ep = own->eHitParamsMan->GetHitParams(t);
	ResetSlow();
	if (ep == NULL)
	{
		numHealth = 1;
	}
	else
	{
		numHealth = ep->maxHealth;
	}
	
	if (cuttable)
	{
		cutObject = new CuttableObject;
	}
	else
	{
		cutObject = NULL;
	}
	hasMonitor = p_hasMonitor;
	if( world == 0 )
	{
		ts_hitSpack = NULL;
		ts_blood = NULL;
		keySprite = NULL;
		return;
	}


	auraColor = Color( 0, 0, 0, 0 );
	switch( t )
	{
	case EnemyType::EN_PATROLLER:
			auraColor = Color( 0x55, 0xbb, 0xff );
			break;
		case EnemyType::EN_CRAWLER:
			auraColor = Color( 0x77, 0xcc, 0xff );
			break;
		case EnemyType::EN_BASICTURRET:
			auraColor = Color( 0x66, 0x99, 0xff );
			break;
		case EnemyType::EN_FOOTTRAP:
			auraColor = Color( 0xaa, 0xcc, 0xff );
			break;
		case EnemyType::EN_BAT:
			auraColor = Color( 0x99, 0xff, 0xcc );
			break;
		case EnemyType::EN_STAGBEETLE:
			auraColor = Color( 0xaa, 0xff, 0x99 );
			break;
		case EnemyType::EN_POISONFROG:
			auraColor = Color( 0x66, 0xff, 0xee );
			break;
		case EnemyType::EN_CURVETURRET:
			auraColor = Color( 0x99, 0xff, 0x99 );
			break;
		default:
			break;
	}


	if( hasMonitor )
	{
		switch( world )
		{
		case 1:
			keyColor = COLOR_BLUE;
			break;
		case 2:
			keyColor = COLOR_GREEN;
			break;
		case 3:
			keyColor = COLOR_YELLOW;
			break;
		case 4:
			keyColor = COLOR_ORANGE;
			break;
		case 5:
			keyColor = COLOR_RED;
			break;
		case 6:
			keyColor = COLOR_MAGENTA;
			break;
		}
		keyColor.r = (sf::Uint8)(floor( (float)(keyColor.r) * .1f + .5f ));
		keyColor.g = (sf::Uint8)(floor( (float)(keyColor.g) * .1f + .5f ));
		keyColor.b = (sf::Uint8)(floor( (float)(keyColor.b) * .1f + .5f ));
		//keyColor = Color::Black;
		
		keyColor = Color::Black;

		//cout << "doing the add monitor thing" << endl;
		keyShader = new Shader();
		if( !keyShader->loadFromFile( "Shader/key_shader.frag", sf::Shader::Fragment ) )
		{
			cout << "couldnt load enemy key shader" << endl;
			assert( false );
		}

		keyShader->setUniform( "toColor", Glsl::Vec4( keyColor.r, keyColor.g, keyColor.b, keyColor.a ) );
		keyShader->setUniform( "auraColor", Glsl::Vec4(auraColor.r, auraColor.g, auraColor.b, auraColor.a) );

	//	keyFrame = 0;
	//ts_key = owner->GetTileset( "key_w02_1_128x128.png", 128, 128 );
		stringstream ss;
		ss << "key_w0" << world << "_1_128x128.png";
		ts_key = owner->GetTileset( ss.str(), 128, 128 );

		keySprite = new Sprite;
		keySprite->setTexture( *ts_key->texture );
	}
	else
	{
		cout << "doing the no monitor thing" << endl;
		ts_key = NULL;
		keyShader = NULL;
		keySprite = NULL;
	}


	stringstream ss;
	ss << "hit_spack_" << world << "_128x128.png";

	ts_hitSpack = owner->GetTileset( ss.str(), 128, 128 );

	ss.clear();
	ss.str("");

	ss << "w" << world << "_kill_fx_512x512.png";

	ts_killSpack = owner->GetTileset(ss.str(), 512, 512);

	ss.clear();
	ss.str("");

	ss << "fx_blood_" << world << "_256x256.png";

	ts_blood = owner->GetTileset( ss.str(), 256, 256 );


	hurtShader = new Shader();
	if( !hurtShader->loadFromFile( "Shader/enemyhurt_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "couldnt load enemy enemyhurt shader" << endl;
		assert( false );
	}
	hurtShader->setUniform( "toColor", Glsl::Vec4( Color::White.r, Color::White.g, Color::White.b, Color::White.a ));
	hurtShader->setUniform( "auraColor", Glsl::Vec4(auraColor.r, auraColor.g, auraColor.b, auraColor.a ) );

		//hurtShader->setUniform( "toColor", keyColor );
	

	
	

}

void Enemy::SetZoneSpritePosition()
{
	zonedSprite.setPosition(position.x, position.y);
}

int Enemy::NumTotalBullets()
{
	return 0;
}

void Enemy::Reset()
{
	EnemyParams *eInfo = owner->eHitParamsMan->GetHitParams(type);
	if (eInfo != NULL)
	{
		numHealth = eInfo->maxHealth;
	}
	else
	{
		numHealth = 1;
	}
	
	if (cutObject != NULL)
		cutObject->Reset();
	ResetSlow();
	suppressMonitor = false;
	spawned = false;
	prev = NULL;
	next = NULL;
	spawnedByClone = false;
	currHitboxes = NULL;
	currHurtboxes = NULL;
	dead = false;

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->Reset();
	}

	ResetEnemy();

	UpdateHitboxes();
}

void Enemy::SetHitboxes(CollisionBody *cb, int frame)
{
	currHitboxes = cb;
	currHitboxFrame = frame;
}

void Enemy::SetHurtboxes(CollisionBody *cb, int frame)
{
	currHurtboxes = cb;
	currHurtboxFrame = frame;
}

void Enemy::HandleQuery( QuadTreeCollider * qtc )
{
	//if( !spawned )
	qtc->HandleEntrant( this );
}

bool Enemy::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsBoxTouchingBox( spawnRect, r );//r.intersects( spawnRect );// 
}
	
void Enemy::DirectKill()
{
	owner->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_killSpack, position, true, 0, 10, 4, true);

	dead = true;

	health = 0;
	HandleNoHealth();
	receivedHit = NULL;

	if (cutObject != NULL)
	{
		cutObject->SetCutRootPos(Vector2f(position.x, position.y));
	}
}

bool Enemy::RightWireHitMe( CollisionBox p_hurtBox )
{
	if( owner->GetPlayer( 0 )->hasPowerRightWire )
	{
		Wire::WireCharge *charge = owner->GetPlayer( 0 )->rightWire->activeChargeList;
		while( charge != NULL )
		{
			if( charge->hitbox.Intersects( p_hurtBox ) )
			{
				receivedHit = owner->GetPlayer( 0 )->wireChargeInfo;
				charge->HitEnemy();
				return true;
			}
			charge = charge->next;
		}
	}

	return false;
}

bool Enemy::LeftWireHitMe( CollisionBox p_hurtBox )
{
	if( owner->GetPlayer( 0 )->hasPowerLeftWire )
	{
		Wire::WireCharge *charge = owner->GetPlayer( 0 )->leftWire->activeChargeList;
		while( charge != NULL )
		{
			if( charge->hitbox.Intersects( p_hurtBox ) )
			{
				receivedHit = owner->GetPlayer( 0 )->wireChargeInfo;
				charge->HitEnemy();
				return true;
			}
			charge = charge->next;
		}
	}

	return false;
}

void Enemy::Record( int enemyIndex )
{
	Buf & b = owner->testBuf;

	b.Send( enemyIndex );

	//note: in order for this to work I can't send any pointers to the buf
	//and expect it to work on the next run. just doesnt work

	//need to index the enemies in the list somehow. list changes all the time

	//b.Send( prev );
	//b.Send( next );
	//b.Send( spawned );
	//b.Send( receivedHit );
	//b.Send( 
}

void Enemy::RecordEnemy()
{
	//stub
}

void Enemy::UpdatePrePhysics()
{
	if ( dead )
		return;

	receivedHit = NULL;

	ProcessState();

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdatePrePhysics();
	}


	double len = length(position - owner->GetPlayer( 0 )->position );
	if (len > MAX_VELOCITY * 2)
	{
		numPhysSteps = NUM_STEPS;
	}
	else
	{
		numPhysSteps = NUM_MAX_STEPS;
		owner->GetPlayer(0)->highAccuracyHitboxes = true;
	}

	if (highResPhysics)
	{
		numPhysSteps = NUM_MAX_STEPS;
	}
	
}

bool Enemy::LaunchersAreDone()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		if (launchers[i]->activeBullets != NULL)
			return false;
	}

	return true;
}

void Enemy::UpdatePostPhysics()
{


	//cout << "suppress: " << (int)suppressMonitor << endl;
	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdatePostPhysics();
	}
	
	ProcessHit();

	if (numHealth == 0 && LaunchersAreDone()
		&& ( ( cutObject != NULL && !cutObject->active ) 
			|| cutObject == NULL && dead ) )
	{
		dead = true;
		owner->RemoveEnemy(this);
		return;
	}

	if( !dead )
		UpdateSprite();
	else if( cutObject != NULL )
		cutObject->UpdateCutObject( slowCounter );

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdateSprites();
	}

	if (UpdateAccountingForSlow())
	{
		++frame;
		if (cutObject != NULL)
			cutObject->IncrementFrame();
		
		if( !dead )
			FrameIncrement();
	}
}

void Enemy::CheckedMiniDraw(sf::RenderTarget *target, sf::FloatRect &rect)
{
	/*if (rect.intersects(zonedSprite.getGlobalBounds()))
	{
		DrawMinimap(target);
	}*/
	if ( zone == NULL || ( zone != NULL && zone->active ) )
	{
		if (rect.intersects(zonedSprite.getGlobalBounds()))
		{
			DrawMinimap(target);
		}
	}
}

void Enemy::CheckedZoneDraw(sf::RenderTarget *target, sf::FloatRect &rect)
{
	if (zone != NULL && zone->action == Zone::UNEXPLORED || (zone->action == Zone::OPENING && zone->frame <= 20 ) )// && !zone->active )
	{
		if (rect.intersects(zonedSprite.getGlobalBounds()))
		{
			UpdateZoneSprite();
			ZoneDraw(target);
		}
	}
}

void Enemy::CheckedZoneUpdate(sf::FloatRect &rect)
{
	if (zone != NULL && !zone->active)
	{
		if (rect.intersects(zonedSprite.getGlobalBounds()))
		{
			UpdateZoneSprite();
		}
	}
}

void Enemy::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0 )
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			if (hasMonitor && !suppressMonitor)
			{
				owner->keyMarker->CollectKey();
			}

			owner->GetPlayer(0)->ConfirmEnemyKill(this);
			ConfirmKill();
		}
		else
		{
			owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}

		//receivedHit = NULL;
	}
}
void Enemy::MovePos(V2d &vel,
	int slowMult,
	int numPhysSteps)
{
	V2d movementVec = vel;
	movementVec /= slowMultiple * (double)numPhysSteps;
	position += movementVec;
}

void Enemy::ConfirmHitNoKill()
{
	owner->Pause(5);
	HandleHitAndSurvive();
	owner->cam.SetRumble(.5, .5, 5);
	//owner->cam.SetRumble(3, 3, 5);
}

void Enemy::ConfirmKill()
{
	owner->ActivateEffect(EffectLayer::BEHIND_ENEMIES, ts_killSpack, position, true, 0, 10, 5, true);
	owner->Pause(7);
	owner->cam.SetRumble(1, 1, 7 );
	
	if (hasMonitor)
	{
		owner->absorbDarkParticles->Activate(owner->GetPlayer(0), 1, position, AbsorbParticles::DARK);
	}
	else
	{
		owner->absorbParticles->Activate(owner->GetPlayer(0), 6, position, AbsorbParticles::ENERGY);
	}
	

	dead = true;

	HandleNoHealth();

	if (cutObject != NULL)
	{
		cutObject->SetCutRootPos(Vector2f(position.x, position.y));
	}

	
}

void Enemy::Draw(sf::RenderTarget *target)
{
	if (cutObject != NULL)
	{
		if (dead && cutObject->active )
		{
			cutObject->Draw(target);
		}
		else if( !dead )
		{
			EnemyDraw( target );
		}
	}
	else
	{
		EnemyDraw( target );
	}
}

void Enemy::UpdateZoneSprite()
{
	if (zone != NULL && zone->action == Zone::OPENING && zone->frame <= 20 )
	{
		float f = 1.f - zone->frame / 20.f;
		zonedSprite.setColor(Color(255, 255, 255, f * 255));
	}
	else
	{
		zonedSprite.setColor(Color::White);
	}
	zonedSprite.setTextureRect(ts_zoned->GetSubRect(0));
	zonedSprite.setOrigin(zonedSprite.getLocalBounds().width / 2,
		zonedSprite.getLocalBounds().height / 2);
	
}

void Enemy::ZoneDraw(sf::RenderTarget *target)
{
	target->draw(zonedSprite);
}

void Enemy::DrawMinimap(sf::RenderTarget *target)
{
	if (!dead)
	{
		if (hasMonitor && !suppressMonitor)
		{
			CircleShape cs;
			cs.setRadius(50);
			cs.setFillColor(Color::White);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(position.x, position.y);
			target->draw(cs);
		}
		else
		{
			CircleShape cs;
			cs.setRadius(40);
			cs.setFillColor(Color::Red);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(position.x, position.y);
			target->draw(cs);
		}
	}
}

void Enemy::DebugDraw(sf::RenderTarget *target)
{
	if (!dead)
	{
		if( currHitboxes != NULL )
			currHitboxes->DebugDraw( currHitboxFrame, target);
		if( currHurtboxes != NULL )
			currHurtboxes->DebugDraw( currHurtboxFrame, target);
	}
}

void Enemy::UpdatePhysics( int substep )
{
	specterProtected = false;
	bool validSubstep = (substep < numPhysSteps);

	if (validSubstep)
	{
		UpdatePreLauncherPhysics();
	}
	
	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdatePhysics( substep );
	}

	if (dead || !validSubstep )
	{
		return;
	}

	UpdateEnemyPhysics();

	UpdateHitboxes();

	SlowCheck(0);

	CheckHit(owner->GetPlayer(0), type);

	if (CheckHitPlayer(0))
	{
	}
}

bool Enemy::IsSlowed( int index )
{
	Actor *player = owner->GetPlayer(index);
	return (player->IntersectMySlowboxes(currHurtboxes, currHurtboxFrame));
}

HitboxInfo * Enemy::IsHit(Actor *player)
{
	if (player->IntersectMyHitboxes(currHurtboxes, currHurtboxFrame))
	{
		return player->currHitboxes->hitboxInfo;
	}
	
	return NULL;
}

bool Enemy::CheckHitPlayer(int index)
{
	Actor *player = owner->GetPlayer(index);

	if (currHitboxes != NULL)
	{
		if (player->IntersectMyHurtboxes(currHitboxes, currHitboxFrame))
		{
			IHitPlayer(index);
			player->ApplyHit(currHitboxes->hitboxInfo);
			return true;
		}
	}

	return false;

	//if (action != UNDERGROUND && player->invincibleFrames == 0 && )
	//{
	//	if (player->position.x < position.x)
	//	{
	//		hitboxInfo->kbDir.x = -abs(hitboxInfo->kbDir.x);
	//		//cout << "left" << endl;
	//	}
	//	else if (player->position.x > position.x)
	//	{
	//		//cout << "right" << endl;
	//		hitboxInfo->kbDir.x = abs(hitboxInfo->kbDir.x);
	//	}
	//	else
	//	{
	//		//dont change it
	//	}
	//	attackFrame = 0;
	//	
	//	return true;
	//}

	//return false;
}

EnemyParamsManager::EnemyParamsManager()
{
	memset(params, 0, sizeof(params));
}

EnemyParams *EnemyParamsManager::GetHitParams(EnemyType et)
{
	EnemyParams *ep = params[et];
	if ( ep == NULL)
	{
		switch (et)
		{
		case EnemyType::EN_CRAWLER:
			ep = new EnemyParams(1, 5, .8, 6, 4);
			break;
		case EnemyType::EN_GOAL:
			ep = new EnemyParams(1, 5, .8, 6, 3);
			break;
		case EnemyType::EN_PATROLLER:
			ep = new EnemyParams(1, 5, .8, 6, 4);
			break;
		case EnemyType::EN_FOOTTRAP:
			ep = new EnemyParams(1, 5, .8, 6, 3);
			break;
		case EnemyType::EN_BASICTURRET:
			ep = new EnemyParams(1, 5, .8, 6, 3);
			break;
		case EnemyType::EN_SHROOM:
			ep = new EnemyParams(1, 5, .8, 6, 3);
			break;
		case EnemyType::EN_SHROOMJELLY:
			ep = new EnemyParams(1, 5, .8, 6, 3);
			break;
		case EnemyType::EN_CRAWLERQUEEN:
			ep = new EnemyParams(1, 5, .8, 6, 3);
			break;
		case EnemyType::EN_FLOATINGBOMB:
			ep = new EnemyParams(1, 5, .8, 6, 3);
			break;
		case EnemyType::EN_BLOCKER:
			ep = new EnemyParams(1, 5, .8, 6, 3);
			break;
		default:
			return NULL;
		}
	}

	return ep;
}

EnemyParamsManager::~EnemyParamsManager()
{
	for (int i = 0; i < E_Count; ++i)
	{
		if (params[i] != NULL)
		{
			delete params[i];
			params[i] = NULL;
		}
	}
}

bool HittableObject::CheckHit( Actor *player, EnemyType et )
{
	if (receivedHit == NULL)
	{
		receivedHit = IsHit(player);
		if (receivedHit == NULL)
			return false;

		if (receivedHit->hType < HitboxInfo::HitboxType::WIREHITRED)
		{
			player->ConfirmHit(player->owner->eHitParamsMan->GetHitParams(et));
		}
		else
		{
			assert(0);
		}
	}
	return false;
}

CuttableObject::CuttableObject()
{
	Reset();
	separateSpeed = .4;
	ts = NULL;
	rectWidth = 32;
	rectHeight = 32;

	for (int i = 0; i < 2; ++i)
	{
		quads[i * 4 + 0].color = Color::Blue;
		quads[i * 4 + 1].color = Color::Blue;
		quads[i * 4 + 2].color = Color::Blue;
		quads[i * 4 + 3].color = Color::Blue;
	}
}

void CuttableObject::SetSubRectFront( int fIndex )
{
	if (ts != NULL)
	{
		tIndexFront = fIndex;
		IntRect fr = ts->GetSubRect(tIndexFront);
		quads[4].texCoords = Vector2f(fr.left, fr.top);
		quads[5].texCoords = Vector2f(fr.left + fr.width, fr.top);
		quads[6].texCoords = Vector2f(fr.left + fr.width, fr.top + fr.height);
		quads[7].texCoords = Vector2f(fr.left, fr.top + fr.height);

		quads[4].color = Color::White;
		quads[5].color = Color::White;
		quads[6].color = Color::White;
		quads[7].color = Color::White;
	}
}

void CuttableObject::SetTileset(Tileset *p_ts)
{
	ts = p_ts;
	rectWidth = ts->tileWidth;
	rectHeight = ts->tileHeight;
}

void CuttableObject::SetSubRectBack( int bIndex )
{
	if (ts != NULL)
	{
		tIndexBack = bIndex;

		IntRect fr = ts->GetSubRect(tIndexBack);
		quads[0].texCoords = Vector2f(fr.left, fr.top);
		quads[1].texCoords = Vector2f(fr.left + fr.width, fr.top);
		quads[2].texCoords = Vector2f(fr.left + fr.width, fr.top + fr.height);
		quads[3].texCoords = Vector2f(fr.left, fr.top + fr.height);

		quads[0].color = Color::White;
		quads[1].color = Color::White;
		quads[2].color = Color::White;
		quads[3].color = Color::White;
	}
}

void CuttableObject::SetCutRootPos(sf::Vector2f &p_rPos )
{
	separateFrame = 0;
	rootPos = p_rPos;
	active = true;
}
bool CuttableObject::DoneSeparatingCut()
{
	return active
		&& separateFrame == totalSeparateFrames;
}

void CuttableObject::Reset()
{
	splitDir = Vector2f(-1, 0);
	separateFrame = 0;
	totalSeparateFrames = 60;
	active = false;
	rotateAngle = 0;
	flipHoriz = false;
	flipVert = false;
}

void CuttableObject::UpdateCutObject( int slowCounter )
{
	Vector2f currSplitDir = splitDir;

	if (active)
	{
		Transform sprT;
		sprT.rotate(rotateAngle);
		
		if (flipHoriz)
		{
			sprT.scale(-1, 1);
		}
		if (flipVert)
		{
			sprT.scale(1, -1);
		}

		currSplitDir = sprT.transformPoint(currSplitDir);

		int halfWidth = rectWidth / 2;
		int halfHeight = rectHeight / 2;

		Vector2f root[2];
		root[0] = rootPos + currSplitDir
			* (separateSpeed * (float)separateFrame + (separateSpeed / slowCounter));
		root[1] = rootPos - currSplitDir
			* (separateSpeed * (float)separateFrame + (separateSpeed / slowCounter));

		Vector2f temp;

		float f;
		if (separateFrame <= 30)
		{
			f = 1.f;
		}
		else
		{
			f = 1.f - ( separateFrame - (totalSeparateFrames / 2) ) / (float)(totalSeparateFrames / 2);
		}
		for (int i = 0; i < 2; ++i)
		{
			quads[i * 4 + 0].position = root[i] + sprT.transformPoint(Vector2f(-halfWidth, -halfHeight));
			quads[i * 4 + 1].position = root[i] + sprT.transformPoint(Vector2f(halfWidth, -halfHeight));
			quads[i * 4 + 2].position = root[i] + sprT.transformPoint(Vector2f(halfWidth, halfHeight));
			quads[i * 4 + 3].position = root[i] + sprT.transformPoint( Vector2f(-halfWidth, halfHeight) );

			SetRectColor(quads + i * 4, Color(255, 255, 255, 255 * f));
			
			/*if (flipHoriz)
			{
				temp = quads[i * 4 + 0].position;
				quads[i * 4 + 0].position = quads[i * 4 + 1].position;
				quads[i * 4 + 1].position = temp;

				temp = quads[i * 4 + 3].position;
				quads[i * 4 + 3].position = quads[i * 4 + 2].position;
				quads[i * 4 + 2].position = temp;
			}*/
			

			
		}
	}
}

void CuttableObject::Draw(sf::RenderTarget *target)
{
	if( ts != NULL )
		target->draw(quads, 8, sf::Quads, ts->texture);
	else
		target->draw(quads, 8, sf::Quads);
}

void CuttableObject::IncrementFrame()
{
	if (active)
	{
		if (separateFrame < totalSeparateFrames)
		{
			++separateFrame;
		}
		else
		{
			active = false;
		}
	}
}