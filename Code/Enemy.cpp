#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Zone.h"
#include <sstream>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

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
}

Launcher::Launcher( LauncherEnemy *p_handler,
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
		position( p_position ), owner( p_owner ),handler(p_handler)

{
	interactWithTerrain = hitTerrain;
	maxFramesToLive = p_maxFramesToLive;
	wavelength = p_wavelength;
	amplitude = p_amplitude;
	//increment the global counter
	 //+= numTotalBullets;
	int startIndex = owner->totalNumberBullets;

	activeBullets = NULL;

	
	if( wavelength > 0 )
	{
		inactiveBullets = new SinBullet( startIndex++, this );
	}
	else
	{
		inactiveBullets = new BasicBullet( startIndex++, this );
	}
		

	for( int i = 1; i < numTotalBullets; ++i )
	{
		BasicBullet * temp;
		if( wavelength > 0 )
		{
			temp = new SinBullet( startIndex++, this );
		}
		else
		{
			temp = new BasicBullet( startIndex++, this );
		}
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

Launcher::Launcher( LauncherEnemy *handler, GameSession *owner,
	int p_maxFramesToLive )
{

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
	
	V2d dir = facingDir;
	double dirAngle = atan2( facingDir.x, -facingDir.y );
	if( dirAngle < 0 )
	{
		dirAngle += PI * 2.0;
	}
	dirAngle -= angleSpread / 2;
	for( int i = 0; i < perShot; ++i )
	{

		//cout << "trying to activate bullet" << endl;
		BasicBullet * b = ActivateBullet();
		//cout << "bullet done activating" << endl;
		if( b != NULL )
		{
			//cout << "FIRE" << endl;
			b->Reset( position, dir * bulletSpeed );
		}

		dirAngle += angleSpread / perShot;
		dir = V2d( cos( dirAngle - PI / 2.0 ), sin( dirAngle - PI / 2.0 ) );
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

BasicBullet::BasicBullet( int indexVA, Launcher *launch )
	:index( indexVA ), launcher( launch ), next( NULL ), prev( NULL )
{
	//framesToLive = maxFram
	double rad = 12;
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
	VertexArray &bva = *(launcher->owner->bigBulletVA);
	bva[index*4+0].position = Vector2f( 0, 0 );
	bva[index*4+1].position = Vector2f( 0, 0 );
	bva[index*4+2].position = Vector2f( 0, 0 );
	bva[index*4+3].position = Vector2f( 0, 0 );
}

void BasicBullet::UpdatePrePhysics()
{
	velocity += gravity / (double)slowMultiple;
	//if( gravTowardsPlayer )
	//{
	//	double len = gravity.y;//length( gravity );
	//	V2d diff = launcher->owner->player->position - position;
	//	int t = 100;
	//	diff += V2d( (rand() % t) - t / 2, (rand() % t) - t / 2);
	//	V2d towards = normalize( diff );
	//	V2d other( towards.y, -towards.x );

	//	double off = dot( velocity, other );
	//	double on = dot( velocity, towards );
	//	//off *= .99;//1.0/2.0;//7.0 / 8.0;

	//	velocity = on * towards + off * other;
	//	
	//	//double to = 

	//	velocity += (towards * len ) / (double) slowMultiple;
	//}
	//else
	//{
	//	
	//}
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
		//cout << "time out!" << endl;
		//explode
		launcher->DeactivateBullet( this );
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

		Actor *player = launcher->owner->player;
		if( player->hurtBody.Intersects( hitBody ) )
		{	
			//cout << "hit??" << endl;
			HitPlayer();
			break;
		}
	}
	while( movementLen > 0 );
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

SinBullet::SinBullet( int indexVA, Launcher *launcher )
	:BasicBullet( indexVA, launcher )
{
}

void SinBullet::UpdatePrePhysics()
{
	position -= tempadd;
	int tempFrame = (launcher->maxFramesToLive - framesToLive) % launcher->wavelength;
	double test = tempFrame / (double)launcher->wavelength;
	//cout << "test: " << test << endl;
	//cout << "tempframe: " << tempFrame << ", framestolive: "
	//	<< framesToLive <<", wv: " << launcher->wavelength << endl;
	double t = test * 2.0 * PI;
	double c = cos( t );
	V2d dir( cos( t ) - sin( t ), sin( t ) + cos( t ) );

	V2d other = normalize( velocity );
	other = V2d( other.y, -other.x );

	double d = dot( dir, other );
	tempadd = d * other * launcher->amplitude;
	//cout << "tempadd: " << tempadd.x << ", " << tempadd.y << endl;
	position += tempadd;

	//tempadd = dir * 100.0;
}

void SinBullet::UpdatePhysics()
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

		position += move * moveDir;
		/*bool hit = ResolvePhysics( moveDir * move );
		if( hit )
		{
			HitTerrain();
			break;
		}*/

		hitBody.globalPosition = position;
		hurtBody.globalPosition = position;

		Actor *player = launcher->owner->player;
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

Enemy::Enemy( GameSession *own, EnemyType t, bool p_hasMonitor,
	int world )
	:owner( own ), prev( NULL ), next( NULL ), spawned( false ), slowMultiple( 1 ), slowCounter( 1 ),
	spawnedByClone( false ), type( t ),zone( NULL ), dead( false ),
	suppressMonitor( false ), ts_hitSpack( NULL ), keyShader( NULL )
{
	hasMonitor = p_hasMonitor;
	if( world == 0 )
	{
		keyFrame = 0;
		ts_hitSpack = NULL;
		ts_blood = NULL;
		keySprite = NULL;
		return;
	}

	
	if( hasMonitor )
	{

		cout << "doing the add monitor thing" << endl;
		keyShader = new Shader();
		if( !keyShader->loadFromFile( "key_shader.frag", sf::Shader::Fragment ) )
		{
			cout << "couldnt load enemy key shader" << endl;
			assert( false );
		}

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


	keyFrame = 0;

	stringstream ss;
	ss << "hit_spack_" << world << "_128x128.png";

	ts_hitSpack = owner->GetTileset( ss.str(), 128, 128 );

	ss.clear();
	ss.str("");

	ss << "fx_blood_" << world << "_256x256.png";

	ts_blood = owner->GetTileset( ss.str(), 256, 256 );

	
}

int Enemy::NumTotalBullets()
{
	return 0;
}

void Enemy::Reset()
{
	keyFrame = 0;
	suppressMonitor = false;
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




