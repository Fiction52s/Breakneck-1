#include "Bullet.h"
#include "Tileset.h"
#include <iostream>
#include "GameSession.h"
#include "Actor.h"

using namespace std;
using namespace sf;

//Bullet * CreateBullet(BulletType::Type type, int vaIndex, Launcher *launcher)
//{
//	using namespace BulletType;
//	switch (type)
//	{
//	case NORMAL:
//		break;
//	case SIN:
//		break;
//	case BIRDBOSS:
//		break;
//	};
//
//	return NULL;
//}

void LauncherEnemy::BulletHitTerrain(BasicBullet *b,
	Edge *edge, V2d &pos)
{
	b->Kill(-edge->Normal());
}

void LauncherEnemy::BulletHitPlayer( int playerIndex, BasicBullet *b, int hitResult)
{
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		b->launcher->sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}

	b->Kill(b->velocity);
}

Launcher::Launcher(LauncherEnemy *p_handler, BasicBullet::BType p_bulletType,
	int numTotalBullets,
	int bulletsPerShot,
	sf::Vector2<double> p_position,
	sf::Vector2<double> direction,
	double p_angleSpread,
	int p_maxFramesToLive,
	bool hitTerrain,
	int p_wavelength,
	double p_amplitude,
	Tileset *ts )
	:totalBullets(numTotalBullets), perShot(bulletsPerShot),
	facingDir(direction), angleSpread(p_angleSpread),
	position(p_position), handler(p_handler),
	def_e(NULL)
{
	playerIndex = 0;
	sess = Session::GetSession();

	launcherID = sess->allLaunchersVec.size();
	sess->allLaunchersVec.push_back(this);
	

	ts_bullet = ts;
	if (ts_bullet != NULL
		|| sess->specialTempTilesetManager != NULL )
	{
		//specialtemptilesetmanager because
		//this means the enemy
		//shouldnt add its own bullets
		//to the overall bullets
		drawOwnBullets = true;
	}
	else
	{
		ts_bullet = sess->ts_basicBullets;
		drawOwnBullets = false;
	}

	bulletVA = NULL;
	
	sizeof(sf::Transform);

	interactWithPlayer = true;
	skipPlayerCollideForSubstep = false;
	bulletType = p_bulletType;
	maxBulletSpeed = 100;
	//launchType = p_launchType;
	interactWithTerrain = hitTerrain;
	maxFramesToLive = p_maxFramesToLive;
	wavelength = p_wavelength;
	amplitude = p_amplitude;
	allBullets.resize(totalBullets);
	int startIndex = 0;
	
	if (!drawOwnBullets)
	{
		startIndex = sess->totalNumberBullets;
	}
	

	activeBullets = NULL;

	switch (bulletType)
	{
	case BasicBullet::BASIC_TURRET:
		bulletTilesetIndex = 0;
		break;
	case BasicBullet::PATROLLER:
		bulletTilesetIndex = 1;
		break;
	case BasicBullet::BAT:
		bulletTilesetIndex = 6;
		break;
	case BasicBullet::CURVE_TURRET:
		bulletTilesetIndex = 2;
		break;
	case BasicBullet::LOB_TURRET:
		bulletTilesetIndex = 2;
		break;
	case BasicBullet::CACTUS:
		bulletTilesetIndex = 5;
		break;
	case BasicBullet::OWL:
		bulletTilesetIndex = 4;
		break;
	case BasicBullet::BIG_OWL:
		bulletTilesetIndex = 4;
		break;
	case BasicBullet::TURTLE:
		bulletTilesetIndex = 7;
		break;
	case BasicBullet::LIZARD:
		bulletTilesetIndex = 10;
		break;
	case BasicBullet::PARROT:
		bulletTilesetIndex = 8;
		break;
	case BasicBullet::GROWING_TREE:
		bulletTilesetIndex = 8;
		break;
	case BasicBullet::COPYCAT:
		bulletTilesetIndex = 1;
		break;
	case BasicBullet::SPECTER:
		bulletTilesetIndex = 8;
		break;
	case BasicBullet::PREDICT:
		bulletTilesetIndex = 1;
		break;
	case BasicBullet::BOSS_CRAWLER:
		bulletTilesetIndex = 2;
		break;
	case BasicBullet::BOSS_BIRD:
		bulletTilesetIndex = 1;
		break;
	}

	if (bulletType == BasicBullet::COPYCAT)
	{
		inactiveBullets = new CopycatBullet(startIndex++, this);
	}
	else if (bulletType == BasicBullet::LIZARD)
	{
		inactiveBullets = new GrindBullet(startIndex++, this);
	}
	else
	{
		if (wavelength > 0)
		{
			inactiveBullets = new SinBullet(startIndex++, this);
		}
		else
		{

			inactiveBullets = new BasicBullet(startIndex++, bulletType, this);
		}
	}

	inactiveBullets->bulletID = 0;
	allBullets[0] = inactiveBullets;


	for (int i = 1; i < numTotalBullets; ++i)
	{
		BasicBullet * temp;
		if (bulletType == BasicBullet::COPYCAT)
		{
			temp = new CopycatBullet(startIndex++, this);
		}
		else if (bulletType == BasicBullet::LIZARD)
		{
			temp = new GrindBullet(startIndex++, this);
		}
		else
		{
			if (wavelength > 0)
			{
				temp = new SinBullet(startIndex++, this);
			}
			else
			{
				temp = new BasicBullet(startIndex++, bulletType, this);
			}
		}

		temp->bulletID = i;
		allBullets[i] = temp;
		temp->next = inactiveBullets;
		inactiveBullets->prev = temp;
		inactiveBullets = temp;
	}
	
	if (sess->IsSessTypeGame() && !drawOwnBullets )
	{
		sess->totalNumberBullets = startIndex;
	}

	if (drawOwnBullets)
	{
		bulletVA = new Vertex[numTotalBullets*4];
	}

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = .3;
	hitboxInfo->drainY = .3;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 1.0;
	hitboxInfo->knockbackOnBlock = false;
	hitboxInfo->kbDir = V2d( 1, 0 );

	bytesStoredPerBullet = inactiveBullets->GetNumStoredBytes();
}

void Launcher::SetStartIndex(int ind)
{
	int currIndex = ind;
	BasicBullet *curr = inactiveBullets;
	while (curr != NULL)
	{
		curr->SetIndex(currIndex);
		curr = curr->next;
		++currIndex;
	}
}

void Launcher::Draw(sf::RenderTarget *target)
{
	//the ts_bullet check 
	//is when you dont really want to 
	//draw bullets at all. 
	//log menu enemy special case
	if (drawOwnBullets && ts_bullet != NULL )
	{
		target->draw(bulletVA, 4 * totalBullets, sf::Quads, ts_bullet->texture);
	}
}

void Launcher::KillAllBullets()
{
	BasicBullet *b = activeBullets;
	while (b != NULL)
	{
		BasicBullet *next = b->next;

		b->Kill(b->velocity);

		b = next;
	}
}

void Launcher::DebugDraw(sf::RenderTarget *target)
{
	BasicBullet *curr = activeBullets;
	while (curr != NULL)
	{
		curr->DebugDraw(target);
		curr = curr->next;
	}
}

double Launcher::GetRadius(BasicBullet::BType bt)
{
	switch (bt)
	{
	case BasicBullet::BASIC_TURRET:
		return 28;//12;	
	case BasicBullet::PATROLLER:
		return 32;//44;
	case BasicBullet::BAT:
		return 20;
	case BasicBullet::CURVE_TURRET:
		return 20;
	case BasicBullet::LOB_TURRET:
		return 20;
	case BasicBullet::CACTUS:
		return 20;
	case BasicBullet::OWL:
		return 20;
	case BasicBullet::BIG_OWL:
		return 10;
	case BasicBullet::TURTLE:
		return 16;
	case BasicBullet::LIZARD:
		return 32;
	case BasicBullet::PARROT:
		return 20;
	case BasicBullet::GROWING_TREE:
		return 20;
	case BasicBullet::COPYCAT:
		return 20;
	case BasicBullet::SPECTER:
		return 44;
	case BasicBullet::PREDICT:
		return 44;
	}

	return 10;
}

Vector2f Launcher::GetOffset(BasicBullet::BType bt)
{
	if (bt == BasicBullet::PATROLLER)
	{
		return Vector2f(-12, 0);
	}
	else if (bt == BasicBullet::LIZARD
		|| bt == BasicBullet::BAT)
	{
		return Vector2f(0, 0);
	}
	/*else if (bt == BasicBullet::LOB_TURRET)
	{
		return Vector2f(0, 0);
	}*/
	
	return Vector2f(-6, 0);//Vector2f(-20, 0);

}

Launcher::Launcher(LauncherEnemy *handler,
	int p_maxFramesToLive)
{
	sess = Session::GetSession();

	maxBulletSpeed = 100;
	skipPlayerCollideForSubstep = false;
}

Launcher::~Launcher()
{
	DeactivateAllBullets();
	BasicBullet *curr = inactiveBullets;
	BasicBullet *cn = NULL;
	while (curr != NULL)
	{
		cn = curr->next;
		delete curr;
		curr = cn;
	}
	delete hitboxInfo;

	if (bulletVA != NULL)
		delete[] bulletVA;
}

int Launcher::GetBulletID(BasicBullet *b)
{
	if (b == NULL)
	{
		return -1;
	}	
	else
	{
		return b->bulletID;
	}
}

BasicBullet *Launcher::GetBulletFromID(int id)
{
	if (id < 0)
	{
		return NULL;
	}
	else
	{
		return allBullets[id];
	}
}

int Launcher::GetNumStoredBytes()
{
	return sizeof(MyData) + bytesStoredPerBullet * totalBullets;
}

void Launcher::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));

	d.inactiveBulletsID = GetBulletID(inactiveBullets);
	d.activeBulletsID = GetBulletID(activeBullets);

	memcpy(bytes, &d, sizeof(MyData));

	/*cout << "saving launcher: " << endl;

	cout << "inactiveBullets: " << inactiveBullets << endl;
	cout << "activeBullets: " << activeBullets << endl;*/

	bytes += sizeof(MyData);
	for (int i = 0; i < totalBullets; ++i)
	{
		allBullets[i]->StoreBytes(bytes);
		bytes += bytesStoredPerBullet;
	}
}
void Launcher::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	inactiveBullets = GetBulletFromID(d.inactiveBulletsID);
	activeBullets = GetBulletFromID(d.activeBulletsID);

	bytes += sizeof(MyData);
	for (int i = 0; i < totalBullets; ++i)
	{
		allBullets[i]->SetFromBytes(bytes);
		bytes += bytesStoredPerBullet;
	}
}

void Launcher::DeactivateAllBullets()
{
	BasicBullet *curr = activeBullets;
	BasicBullet *cn = NULL;
	while (curr != NULL)
	{
		cn = curr->next;
		DeactivateBullet(curr);
		curr = cn;
	}
}

void Launcher::SetDefaultCollision(int framesToLive, Edge*e, V2d &pos)
{
	def_framesToLive = framesToLive - 1;
	if (def_framesToLive < 0)
	{
		def_framesToLive = 0;
	}
	def_e = e;
	def_pos = pos;
}

void Launcher::CapBulletVel(double speed)
{
	BasicBullet *curr = activeBullets;
	BasicBullet *temp;
	while (curr != NULL)
	{
		temp = curr->next;
		V2d norm = normalize(curr->velocity);
		if (length(curr->velocity) > speed)
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
	while (curr != NULL)
	{
		temp = curr->next;
		//	cout << "updating bullet pre" << endl;

		curr->UpdatePrePhysics();
		curr = temp;
	}

	CapBulletVel(maxBulletSpeed);
}

void Launcher::UpdatePhysics(int substep, bool lowRes)
{
	BasicBullet *curr = activeBullets;
	BasicBullet *temp;
	while (curr != NULL)
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
	if (ts_bullet == NULL)
	{
		ts_bullet = sess->ts_basicBullets;
	}
	BasicBullet *curr = activeBullets;
	while (curr != NULL)
	{
		//cout << "updating bullet sprite" << endl;
		curr->UpdateSprite();
		curr = curr->next;
	}
}

void Launcher::SetBulletSpeed(double speed)
{
	bulletSpeed = speed;
}

void Launcher::Fire()
{
	V2d dir = facingDir;
	double dirAngle = atan2(facingDir.x, -facingDir.y);
	if (dirAngle < 0)
	{
		dirAngle += PI * 2.0;
	}

	

	if (angleSpread < 2 * PI)
	{
		dirAngle -= angleSpread / 2;
	}

	for (int i = 0; i < perShot; ++i)
	{
		dir = V2d(cos(dirAngle - PI / 2.0), sin(dirAngle - PI / 2.0));
		BasicBullet * b = ActivateBullet();
		if (b != NULL)
		{
			b->Reset(position, dir * bulletSpeed);
		}

		if (handler != NULL)
		{
			handler->FireResponse(b);
		}

		if (perShot > 1)
		{
			if (angleSpread < 2 * PI)
			{
				dirAngle += angleSpread / (perShot - 1);
			}
			else
			{
				dirAngle += angleSpread / (perShot);
			}
		}
	}
}

void Launcher::Fire(double gravStrength)
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
	while (activeBullets != NULL)
	{
		++x;
		//cout << "deact" << endl;
		DeactivateBullet(activeBullets);
	}
	//cout << "reset " << x << " bullets" << endl;
}

void Launcher::UpdatePostPhysics()
{
	BasicBullet *curr = activeBullets;
	BasicBullet *temp;
	while (curr != NULL)
	{
		temp = curr->next;
		curr->UpdatePostPhysics();
		curr = temp;
	}
}

void Launcher::AddToList(BasicBullet *b, BasicBullet *&list)
{
	b->prev = NULL;
	b->next = list;
	if (list != NULL)
	{
		list->prev = b;
	}
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
	while (curr != NULL)
	{
		++count;
		curr = curr->next;
	}

	return count;
}

BasicBullet * Launcher::ActivateBullet()
{
	if (inactiveBullets == NULL)
	{
		return RanOutOfBullets();
	}
	else
	{
		BasicBullet *temp = inactiveBullets->next;
		AddToList(inactiveBullets, activeBullets);
		inactiveBullets = temp;
		if (inactiveBullets != NULL)
			inactiveBullets->prev = NULL;
		activeBullets->active = true;
		return activeBullets;
	}
}

void Launcher::DeactivateBullet(BasicBullet *b)
{
	if (!b->active)
	{
		assert(0);
		return;
	}

	assert(activeBullets != NULL);

	b->active = false;
	//size == 1
	if (activeBullets->next == NULL)
	{
		AddToList(b, inactiveBullets);
		activeBullets = NULL;
	}
	//size is greater
	else
	{
		//end
		if (b->next == NULL)
		{
			b->prev->next = NULL;
			AddToList(b, inactiveBullets);
		}
		//start
		else if (b->prev == NULL)
		{
			BasicBullet *temp = b->next;
			b->next->prev = NULL;
			AddToList(b, inactiveBullets);
			activeBullets = temp;
		}
		//middle
		else
		{
			b->next->prev = b->prev;
			b->prev->next = b->next;
			AddToList(b, inactiveBullets);
		}
	}

	b->ResetSprite();
}

void Launcher::SetGravity(sf::Vector2<double> &grav)
{
	BasicBullet *curr = activeBullets;
	while (curr != NULL)
	{
		curr->gravity = grav;
		curr = curr->next;
	}
	curr = inactiveBullets;
	while (curr != NULL)
	{
		curr->gravity = grav;
		curr = curr->next;
	}
}

void BasicBullet::Reset(V2d &pos, V2d &vel)
{
	double angle = atan2(vel.y, vel.x);
	angle = angle * 180 / PI;

	//cout << "angle: " << angle << endl;

	transform = transform.Identity;
	switch (bulletType)
	{
	case CURVE_TURRET:
	{
		double gangle = atan2(gravity.y, gravity.x);
		gangle = gangle * 180 / PI;
		transform.rotate(gangle);
		break;
	}
	default:
		transform.rotate(angle);
		break;
	}

	frame = 0;
	//gravTowardsPlayer = false;
	position = pos;
	velocity = vel;
	framesToLive = launcher->maxFramesToLive;
	slowMultiple = 1;
	slowCounter = 1;
	bounceCount = 0;

	Vertex *bva;
	if (launcher->drawOwnBullets)
	{
		bva = launcher->bulletVA;
	}
	else
	{
		bva = launcher->sess->bigBulletVA;
	}
	bva[index * 4 + 0].position = Vector2f(0, 0);
	bva[index * 4 + 1].position = Vector2f(0, 0);
	bva[index * 4 + 2].position = Vector2f(0, 0);
	bva[index * 4 + 3].position = Vector2f(0, 0);

	double len = length(pos - launcher->sess->GetPlayerPos(launcher->playerIndex));
	if (len > MAX_VELOCITY * 2)
	{
		numPhysSteps = NUM_STEPS;
	}
	else
	{
		numPhysSteps = NUM_MAX_STEPS;
		launcher->sess->GetPlayer(launcher->playerIndex)->highAccuracyHitboxes = true;
	}
}

BasicBullet::BasicBullet(int indexVA, BType bType, Launcher *launch)
	:index(indexVA), launcher(launch), next(NULL), prev(NULL),
	bulletType(bType)
{
	active = false;
	frame = 0;
	switch (bType)
	{
	case BASIC_TURRET:
		break;
	case BAT:
		break;
	case CURVE_TURRET:
		break;
	}
	double rad = Launcher::GetRadius(bType);
	/*hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = rad;
	hurtBody.rh = rad;*/

	bulletID = -1;

	prev = NULL;
	next = NULL;
	slowCounter = 1;
	active = false;
	slowMultiple = 1;
	framesToLive = 0;
	frame = 0;
	bounceCount = 0;

	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = rad;
	hitBody.rh = rad;

	physBody.isCircle = true;
	physBody.globalAngle = 0;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = rad;
	physBody.rh = rad;

	//ResetSprite();
}

void BasicBullet::SetIndex(int ind)
{
	index = ind;
}

void BasicBullet::StoreBasicBulletData(BulletData &bd)
{
	bd.prevID = launcher->GetBulletID(prev);
	bd.nextID = launcher->GetBulletID(next);
	bd.gravity = gravity;
	bd.position = position;
	bd.velocity = velocity;
	bd.slowCounter = slowCounter;
	bd.active = active;
	bd.slowMultiple = slowMultiple;
	bd.framesToLive = framesToLive;
	bd.frame = frame;
	bd.transform = transform;
	bd.bounceCount = bounceCount;
}
void BasicBullet::SetBasicBulletData(BulletData &bd)
{
	prev = launcher->GetBulletFromID(bd.prevID);
	next = launcher->GetBulletFromID(bd.nextID);
	gravity = bd.gravity;
	position = bd.position;
	velocity = bd.velocity;
	slowCounter = bd.slowCounter;
	active = bd.active;
	slowMultiple = bd.slowMultiple;
	framesToLive = bd.framesToLive;
	frame = bd.frame;
	transform = bd.transform;
	bounceCount = bd.bounceCount;
}

int BasicBullet::GetNumStoredBytes()
{
	return sizeof(BulletData);
}

void BasicBullet::StoreBytes(unsigned char *bytes)
{
	BulletData d;
	memset(&d, 0, sizeof(BulletData));
	StoreBasicBulletData(d);

	memcpy(bytes, &d, sizeof(BulletData));
}

void BasicBullet::SetFromBytes(unsigned char *bytes)
{
	BulletData d;
	memcpy(&d, bytes, sizeof(BulletData));
	SetBasicBulletData(d);
}

void BasicBullet::DebugDraw(sf::RenderTarget *target)
{
	hitBody.DebugDraw( CollisionBox::Hit, target);
}

void BasicBullet::Kill( V2d facingDir )
{
	int frames = 6;

	double angle = GetVectorAngleCW(facingDir);//atan2(facingDir.y, -facingDir.x);
	launcher->sess->ActivateEffect(EffectLayer::IN_FRONT, launcher->sess->ts_basicBulletExplode, position, true, angle, frames, 2, true, launcher->bulletTilesetIndex * frames);
	launcher->DeactivateBullet(this);
}

void BasicBullet::ResetSprite()
{
	frame = 0;
	Vertex *bva;
	if (launcher->drawOwnBullets)
	{
		bva = launcher->bulletVA;
	}
	else
	{
		bva = launcher->sess->bigBulletVA;
	}

	SetRectCenter(bva + index * 4, 0, 0, Vector2f());
}

bool BasicBullet::PlayerSlowingMe()
{
	Actor *player = launcher->sess->GetPlayer(launcher->playerIndex);

	if (player->globalTimeSlowFrames > 0)
	{
		return true;
	}

	for (int i = 0; i < Actor::MAX_BUBBLES; ++i)
	{
		if (player->bubbleFramesToLive[i] > 0)
		{
			if (length(position - player->bubblePos[i]) <= player->bubbleRadius)
			{
				return true;
			}
		}
	}
	return false;
}

void BasicBullet::UpdatePrePhysics()
{
	if (PlayerSlowingMe())
	{
		Actor *player = launcher->sess->GetPlayer(launcher->playerIndex);
		int currSlowMult = player->GetBubbleTimeFactor();
		if (slowMultiple == 1)
		{
			slowCounter = 1;
			slowMultiple = currSlowMult;
		}
	}
	else
	{
		slowMultiple = 1;
		slowCounter = 1;
	}


	velocity += gravity / (double)slowMultiple;
	V2d playerPos = launcher->sess->GetPlayerPos(launcher->playerIndex);

	if (launcher->handler != NULL)
	{
		launcher->handler->UpdateBullet(this);
	}

	if (launcher->bulletType == BasicBullet::BOSS_BIRD)
	{
		int f = launcher->maxFramesToLive - framesToLive;

		//V2d endFly = launcher->owner->b_bird->endFly;
		V2d dir = normalize(velocity);
		V2d norm(dir.y, -dir.x);
		//angle = atan2( gn.x, -gn.y );
		double len = length(velocity);
		//double angle = atan2( dir.y, dir.x );//norm.x, -norm.y );
		//angle += PI / len;
		//V2d go( len, 0 );
		//velocity.x = cos( angle ) * len;
		//velocity.y = sin( angle ) * len;

		//velocity += norm * dot(normalize( playerPos - position ) * .5 / (double)slowMultiple, norm);
		if (f < 30)
			velocity += normalize(playerPos - position) * 2.0 / (double)slowMultiple;

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
		launcher->sess->GetPlayer(launcher->playerIndex)->highAccuracyHitboxes = true;
	}

	col = false;
}

void BasicBullet::UpdatePostPhysics()
{
	if (slowCounter == slowMultiple)
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


	if (framesToLive == 0 && slowCounter == 1)
	{

		//cout << "time out!" << endl;
		//explode
		if (launcher->handler != NULL)
			launcher->handler->BulletTTLDeath(this);

		launcher->DeactivateBullet(this);
		//parent->DeactivateTree( this );
		//owner->RemoveEnemy( this );
		//return;
	}
}

void BasicBullet::UpdatePhysics()
{
	V2d movement = velocity / numPhysSteps / (double)slowMultiple;

	double movementLen = length(movement);

	//for debugging, numphyssteps wasnt getting initialized correctly when launching the bullet
	if (movementLen == INFINITY)
	{
		cout << "vel is:" << velocity.x << ", " << velocity.y << endl;
		cout << "num: " << numPhysSteps << endl;
		cout << "slowmultiple: " << slowMultiple << endl;
		cout << "movement: " << movement.x << ", " << movement.y << endl;
	}
	assert(movementLen != INFINITY);
	V2d moveDir = normalize(movement);
	double move = 0;

	do
	{
		//cout << "loop: " << movementLen << endl;
		if (movementLen > physBody.rw)
		{
			movementLen -= physBody.rw;
			move = physBody.rw;
		}
		else
		{
			move = movementLen;
			movementLen = 0;
		}

		if (move != 0)
		{
			bool hit = ResolvePhysics(moveDir * move);
			if (hit)
			{
				HitTerrain();
				break;
			}
		}

		hitBody.globalPosition = position;

		if (!launcher->skipPlayerCollideForSubstep && launcher->interactWithPlayer)
		{
			Actor *player = launcher->sess->GetPlayer(launcher->playerIndex);

			Actor::HitResult res = player->CheckIfImHitByBullet( this, hitBody,
				HitboxInfo::GetAirType( velocity ),
				position, velocity.x >= 0,
				launcher->hitboxInfo->canBeParried,
				launcher->hitboxInfo->canBeBlocked);

			if (res != Actor::HitResult::MISS )
			{
				//cout << "hit??" << endl;
				HitPlayer( player->actorIndex, res );
				break;
			}
		}
	} while (movementLen > 0);

	if (!col && launcher->def_e != NULL && framesToLive == launcher->def_framesToLive && active)
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
	//cout << "hit terrain" << endl;
	launcher->handler->BulletHitTerrain(this,
		minContact.edge, minContact.position);
	return true;
}

void BasicBullet::HitPlayer( int pIndex, int hitResult )
{
	launcher->handler->BulletHitPlayer( pIndex, this, hitResult );
	//launcher->DeactivateBullet( this );
}

bool BasicBullet::CanInteractWithTerrain()
{
	return launcher->interactWithTerrain;
}

bool BasicBullet::ResolvePhysics(V2d vel)
{
	if ( CanInteractWithTerrain() && !col)
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



		launcher->sess->terrainTree->Query(this, r);
	}
	else
	{
		col = false;
		position += vel;
	}

	return col;
}

void BasicBullet::HandleEntrant(QuadTreeEntrant *qte)
{
	Edge *e = (Edge*)qte;

	Contact *c = launcher->sess->collider.collideEdge(position + tempVel, physBody, e, tempVel, V2d(0, 0));

	if (c != NULL)
	{
		//cout << "attempted hit!" << endl;
		if (!col)
		{
			minContact = *c;
			col = true;
		}
		else if (c->collisionPriority < minContact.collisionPriority)
		{
			minContact = *c;
		}
	}
}

void BasicBullet::UpdateSprite()
{
	sf::Vertex *VA;
	if (launcher->drawOwnBullets)
	{
		VA = launcher->bulletVA;
	}
	else
	{
		VA = launcher->sess->bigBulletVA;
	}
	//IntRect ir = ts->GetSubRect( (maxFramesToLive - framesToLive) % 5 );
	Vector2f dims(48, 48);

	switch (bulletType)
	{
	case PATROLLER:
		dims = Vector2f(64, 64);
		break;
	case BASIC_TURRET:
		dims = Vector2f(64, 64);
		break;
	case SPECTER:
		dims = Vector2f(64, 64);
		break;
	}
	//Vector2f dims = Vector2f( ir.width / 2, ir.height / 2 );
	Vector2f offset = Launcher::GetOffset(bulletType);
	Vector2f center(position.x, position.y);

	Vector2f topLeft = -dims + offset;
	Vector2f topRight = Vector2f(dims.x, -dims.y) + offset;
	Vector2f botRight = dims + offset;
	Vector2f botLeft = Vector2f(-dims.x, dims.y) + offset;

	int animFactor = 2;

	switch (bulletType)
	{
	case BAT:
	case PATROLLER:
	case OWL:
	case GROWING_TREE:
	case BIG_OWL:
	case SPECTER:
		animFactor = 4;
		break;
	}

	switch (bulletType)
	{
	default:
		double angle = atan2(velocity.y, velocity.x);
		angle = angle * 180 / PI;
		transform = transform.Identity;
		transform.rotate(angle);
		break;
	}


	

	/*switch (bulletType)
	{
	case BAT:
	case PATROLLER:
	case OWL:
	case GROWING_TREE:
	case BIG_OWL:
	case SPECTER:
	{
		double angle = atan2(velocity.y, velocity.x);
		angle = angle * 180 / PI;
		transform = transform.Identity;
		transform.rotate(angle);
		animFactor = 4;
		break;
	}
	case LIZARD:
	case LOB_TURRET:
	{
		double angle = atan2(velocity.y, velocity.x);
		angle = angle * 180 / PI;
		transform = transform.Identity;
		transform.rotate(angle);
		break;
	}*/
	/*case CURVE_TURRET:
	{
	double gangle = atan2(gravity.y, gravity.x);
	gangle = gangle * 180 / PI;
	transform.rotate(gangle);
	break;
	}
	case BASIC_TURRET:
	transform.rotate(angle);
	break;
	case BOSS_BIRD:
	{
	transform.rotate(angle);
	break;
	}*/

	//}

	VA[index * 4 + 0].position = center + transform.transformPoint(topLeft);
	VA[index * 4 + 1].position = center + transform.transformPoint(topRight);
	VA[index * 4 + 2].position = center + transform.transformPoint(botRight);
	VA[index * 4 + 3].position = center + transform.transformPoint(botLeft);

	int ind = 6 * launcher->bulletTilesetIndex + ((frame / animFactor) % 6);
	IntRect sub = launcher->ts_bullet->GetSubRect(ind);
	//SetRectColor(VA + index * 4, Color::White);
	SetRectSubRect(VA + index * 4, sub);
}

SinBullet::SinBullet(int indexVA, Launcher *launcher)
	:BasicBullet(indexVA, BasicBullet::BASIC_TURRET, launcher)
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
	position -= data.tempadd;

	int waveLength = launcher->wavelength * 5;
	int ftl = framesToLive * 5;
	int maxFrames = launcher->maxFramesToLive * 5;
	//launcher->wavelength * slowMultiple;
	int tempFrame = (maxFrames - ftl) % waveLength;
	tempFrame += (slowCounter - 1);
	double test = tempFrame / (double)(waveLength);

	//cout << "temp: " << tempFrame << ", wl: " << waveLength << ", test: " << test << endl;

	double t = test * 2 * PI;//2.0 * PI;
							 //double c = cos( t );
							 //V2d dir( cos( t ) - sin( t ), sin( t ) + cos( t ) );
	V2d dir(0, sin(t));
	//cout << "test: " << test <<  ", t: " << t << ", temp: " << tempFrame << ", wl: " << waveLength << ", diry: " << dir.y << endl;

	V2d other = normalize(velocity);
	other = V2d(other.y, -other.x);
	//dir = normalize( velocity );

	double d = dot(dir, other);
	data.tempadd = sin(t) * launcher->amplitude * other;//d * other * launcher->amplitude;

	position += data.tempadd;//other * sin(t) * launcher->amplitude;//tempadd;
						//cout << "tempadd: " << tempadd.x << ", " << tempadd.y << endl;

						//tempadd = dir * 100.0;
}

void SinBullet::UpdatePhysics()
{
	V2d movement = velocity / numPhysSteps / (double)slowMultiple;

	double movementLen = length(movement);
	V2d moveDir = normalize(movement);
	double move = 0;
	while (movementLen > 0)
	{
		//cout << "loop: " << movementLen << endl;
		if (movementLen > physBody.rw)
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
		//hurtBody.globalPosition = position;

		Actor *player = launcher->sess->GetPlayer(launcher->playerIndex);
		//player->CheckIfImHit( )

		Actor::HitResult res = player->CheckIfImHitByBullet( this, hitBody,
			HitboxInfo::GetAirType( velocity ),
			position, velocity.x >= 0,
			launcher->hitboxInfo->canBeParried,
			launcher->hitboxInfo->canBeBlocked);

		if (res != Actor::HitResult::MISS)
		{
			HitPlayer( player->actorIndex, res );
		}
		
	}
}

void SinBullet::Reset(V2d &pos,
	V2d &vel)
{
	BasicBullet::Reset(pos, vel);
	data.tempadd = V2d(0, 0);
}

int SinBullet::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void SinBullet::StoreBytes(unsigned char *bytes)
{
	StoreBasicBulletData(data);
	memcpy(bytes, &data, sizeof(MyData));
}

void SinBullet::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicBulletData(data);
}

GrindBullet::GrindBullet(int indexVA, Launcher *launcher)
	:BasicBullet(indexVA, BasicBullet::LIZARD, launcher)
{

}

bool GrindBullet::CanInteractWithTerrain()
{
	return launcher->interactWithTerrain && grindEdge == NULL;
}

void GrindBullet::UpdatePrePhysics()
{
	if (PlayerSlowingMe())
	{
		Actor *player = launcher->sess->GetPlayer(launcher->playerIndex);
		int currSlowMult = player->GetBubbleTimeFactor();
		if (slowMultiple == 1)
		{
			slowCounter = 1;
			slowMultiple = currSlowMult;
		}
	}
	else
	{
		slowMultiple = 1;
		slowCounter = 1;
	}

	V2d playerPos = launcher->sess->GetPlayerPos(launcher->playerIndex);

	if (launcher->handler != NULL)
	{
		launcher->handler->UpdateBullet(this);
	}

	double len = length(position - playerPos);
	if (len > MAX_VELOCITY * 2)
	{
		numPhysSteps = NUM_STEPS;
	}
	else
	{
		numPhysSteps = NUM_MAX_STEPS;
		launcher->sess->GetPlayer(launcher->playerIndex)->highAccuracyHitboxes = true;
	}

	col = false;
}

bool GrindBullet::HitTerrain()
{
	launcher->handler->BulletHitTerrain(this,
		minContact.edge, minContact.position);
	grindEdge = minContact.edge;
	data.edgeQuantity = grindEdge->GetQuantity(minContact.position);
	data.grindSpeed = launcher->bulletSpeed;
	if (!data.clockwise)
	{
		data.grindSpeed = -data.grindSpeed;
	}

	

	return true;
}

void GrindBullet::UpdatePhysics()
{
	if (grindEdge != NULL)
	{
		double factor = slowMultiple * (double)numPhysSteps;
		double movement = data.grindSpeed / factor;

		//int grindEdgeIndex = edge->//currPosInfo.GetEdgeIndex();
		//PolyPtr groundPoly = currPosInfo.ground;
		//int numPoints = groundPoly->GetNumPoints();

		while (!approxEquals(movement, 0))
		{
			double gLen = grindEdge->GetLength();

			if (movement > 0)
			{
				double extra = data.edgeQuantity + movement - gLen;

				if (extra > 0)
				{
					movement -= gLen - data.edgeQuantity;
					grindEdge = grindEdge->GetNextEdge();
					//++grindEdgeIndex;
					//if (grindEdgeIndex == numPoints)
					//{
					//	grindEdgeIndex = 0;
				//	}

					data.edgeQuantity = 0;
				}
				else
				{
					data.edgeQuantity += movement;
					movement = 0;
				}
			}
			else
			{
				double extra = data.edgeQuantity + movement;

				if (extra < 0)
				{
					movement -= movement - extra;
					grindEdge = grindEdge->GetPrevEdge();
					//--grindEdgeIndex;
					//if (grindEdgeIndex < 0)
				//	{
				//		grindEdgeIndex = numPoints - 1;
				//	}
					data.edgeQuantity = grindEdge->GetLength();
				}
				else
				{
					data.edgeQuantity += movement;
					movement = 0;
				}
			}
		}

		velocity = grindEdge->Along() * data.grindSpeed;
		position = grindEdge->GetPosition(data.edgeQuantity);
		hitBody.globalPosition = position;

		if (!launcher->skipPlayerCollideForSubstep)
		{
			Actor *player = launcher->sess->GetPlayer(launcher->playerIndex);

			Actor::HitResult res = player->CheckIfImHitByBullet( this, hitBody,
				HitboxInfo::GetAirType(velocity),
				position, velocity.x >= 0,
				launcher->hitboxInfo->canBeParried,
				launcher->hitboxInfo->canBeBlocked);

			if (res != Actor::HitResult::MISS)
			{
				HitPlayer(player->actorIndex, res);
			}
		}
	}
	else
	{
		BasicBullet::UpdatePhysics();
	}
}

void GrindBullet::Reset(V2d &pos,
	V2d &vel)
{
	BasicBullet::Reset(pos, vel);
	grindEdge = NULL;
	data.edgeQuantity = -1;
	data.clockwise = true;
}



int GrindBullet::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void GrindBullet::StoreBytes(unsigned char *bytes)
{
	StoreBasicBulletData(data);

	data.grindEdgeInfo.SetFromEdge(grindEdge);

	memcpy(bytes, &data, sizeof(MyData));
}

void GrindBullet::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicBulletData(data);

	grindEdge = launcher->sess->GetEdge(&data.grindEdgeInfo);
}


CopycatBullet::CopycatBullet(int indexVA, Launcher *launcher)
	:BasicBullet(indexVA, BasicBullet::COPYCAT, launcher)
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

	double movementLen = length(movement);
	V2d moveDir = normalize(movement);
	double move = 0;
	while (movementLen > 0)
	{
		//cout << "loop: " << movementLen << endl;
		if (movementLen > physBody.rw)
		{
			movementLen -= physBody.rw;
			move = physBody.rw;
		}
		else
		{
			move = movementLen;
			movementLen = 0;
		}

		if (length(data.destination - position) <= move)
		{
			position = data.destination;
			velocity = V2d(0, 0);
			launcher->handler->BulletHitTarget(this);
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

void CopycatBullet::Reset(V2d &pos0,
	V2d &pos1)
{
	if (bulletType == BasicBullet::COPYCAT)
	{
		data.attackIndex = launcher->handler->GetAttackIndex();
	}
	data.destination = pos1;
	data.trueVel = normalize(pos1 - pos0) * speed;
	BasicBullet::Reset(pos0, data.trueVel);
	//tempadd = V2d( 0, 0 );
}

int CopycatBullet::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void CopycatBullet::StoreBytes(unsigned char *bytes)
{
	StoreBasicBulletData(data);
	memcpy(bytes, &data, sizeof(MyData));
}

void CopycatBullet::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicBulletData(data);
}