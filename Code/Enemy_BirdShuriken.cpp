#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BirdShuriken.h"
#include "Actor.h"

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

BirdShurikenPool::BirdShurikenPool(Enemy *p_parentEnemy)
{
	ts = NULL;
	numShurs = 20;
	shurVec.resize(numShurs);
	verts = new Vertex[numShurs * 4];
	for (int i = 0; i < numShurs; ++i)
	{
		shurVec[i] = new BirdShuriken(verts + 4 * i);
		shurVec[i]->pool = this;
	}

	Session *sess = Session::GetSession();

	parentEnemy = p_parentEnemy;

	//ts = sess->GetSizedTileset("Bosses/Bird/shuriken01_128x128.png");
	ts = sess->GetSizedTileset("Bosses/Bird/bird_shuriken_256x256.png");
}

BirdShurikenPool::~BirdShurikenPool()
{
	for (int i = 0; i < numShurs; ++i)
	{
		delete shurVec[i];
	}

	delete[] verts;
}

void BirdShurikenPool::Reset()
{
	BirdShuriken *shur = NULL;
	for (int i = 0; i < numShurs; ++i)
	{
		shur = shurVec[i];
		if (shur->spawned)
		{
			shur->Die();
		}

		shur->Reset();
	}

	for (int i = 0; i < numShurs; ++i)
	{
		shurVec[i]->Reset();
	}
}

BirdShuriken * BirdShurikenPool::Throw(V2d &pos, V2d &dir,
	int sType )
{
	BirdShuriken *bs = NULL;
	BirdShuriken *thrownShur = NULL;
	for (int i = 0; i < numShurs; ++i)
	{
		bs = shurVec[i];
		if ( !bs->spawned )
		{
			bs->Throw(pos, dir, sType );
			thrownShur = bs;
			break;
		}
	}
	return thrownShur;
}

void BirdShurikenPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, shurVec.size() * 4, sf::Quads, ts->texture);
}

void BirdShurikenPool::RethrowAll()
{
	BirdShuriken *bs = NULL;
	for (int i = 0; i < numShurs; ++i)
	{
		bs = shurVec[i];
		if (bs->spawned && bs->action == BirdShuriken::STICK )
		{
			bs->Rethrow();
		}
	}
}

BirdShuriken::BirdShuriken( sf::Vertex *myQuad )
	:Enemy(EnemyType::EN_BIRDSHURIKEN, NULL)
{

	SetNumActions(A_Count);
	SetEditorActions(THROWN, 0, 0);

	actionLength[THROWN] = 1;
	animFactor[THROWN] = 1;

	actionLength[STICK] = 1;
	animFactor[STICK] = 1;

	actionLength[TURRET] = 1;
	animFactor[TURRET] = 1;

	pool = NULL;

	quad = myQuad;
	//SetLevel(ap->GetLevel());

	bulletSpeed = 10;
	framesBetween = 60;

	CreateSurfaceMover(startPosInfo, 12, this);

	Tileset *ts_basicBullets = sess->GetSizedTileset("Enemies/bullet_80x80.png");

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::BAT, 16, 1, GetPosition(),
		V2d(1, 0), 0, 120, false, 0, 0, ts_basicBullets );
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->damage = 60;
	frame = 0;

	ts = sess->GetSizedTileset("Bosses/Bird/bird_shuriken_256x256.png");
	//ts = GetSizedTileset("Bosses/Bird/shuriken01_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	
	//cutObject->Setup(ts, 53, 52, scale);
	rotSpeed = PI * .1;

	thrownSpeed = 15;
	accel = .15;

	startUnDodgeSpeed = 10;
	unDodgeAccel = .15;
	unDodgeMaxSpeed = 60;
	
	linearSpeed = 10;
	homingSpeed = 10;
	homingAccel = .8;

	ResetEnemy();
}

void BirdShuriken::ResetEnemy()
{
	ClearRect(quad);

	fireCounter = 0;
	dead = false;
	facingRight = true;

	currBasePos = startPosInfo.GetPosition();

	surfaceMover->SetVelocity( V2d());

	action = THROWN;
	frame = 0;

	currentRot = 0;

	//DefaultHurtboxesOn();
	DefaultHitboxesOn();

	UpdateHitboxes();

	//UpdateSprite();
}

void BirdShuriken::SetLevel(int lev)
{
	level = lev;
	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

void BirdShuriken::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		Die();

		receivedHit.SetEmpty();
	}
}

void BirdShuriken::Throw( V2d &pos, V2d &dir, int p_shurType)
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);
	hitboxInfo->canBeBlocked = true;

	shurType = p_shurType;
	surfaceMover->SetCollisionOn( true );

	surfaceMover->SetVelocity(dir * thrownSpeed );
	framesToLive = 120;

	SetHurtboxes(NULL);

	switch (shurType)
	{
	case UNDODGEABLE:
	{
		unDodgeSpeed = startUnDodgeSpeed;
		surfaceMover->SetCollisionOn(false ); 
		break;
	}
	case UNBLOCKABLE:
	{
		surfaceMover->SetVelocity( dir * homingSpeed );
		surfaceMover->SetCollisionOn(false );
		hitboxInfo->canBeBlocked = false;
		DefaultHurtboxesOn();
		break;
	}
	case SLIGHTHOMING:
	{
		surfaceMover->SetCollisionOn(false );
		break;
	}
		
	}
	
	action = THROWN;
	frame = 0;
	
}

void BirdShuriken::Rethrow()
{
	V2d pDir = PlayerDir();

	framesToLive = 120;

	SetHurtboxes(NULL);

	switch (shurType)
	{
	case SLIGHTHOMING_STICK:
	{
		surfaceMover->SetVelocity(pDir * thrownSpeed );
		break;
	}
	case SURFACENORMAL_STICK:
	{
		surfaceMover->SetVelocity(surfaceMover->ground->Normal() * linearSpeed );
		break;
	}
	case UNDODGEABLE_STICK:
	{
		surfaceMover->SetVelocity(pDir * startUnDodgeSpeed );
		unDodgeSpeed = startUnDodgeSpeed;
		break;
	}
	case UNBLOCKABLE_STICK:
	{
		surfaceMover->SetVelocity(surfaceMover->ground->Normal() * homingSpeed );
		unDodgeSpeed = startUnDodgeSpeed;
		DefaultHurtboxesOn();
		break;
	}
	case RETURN_STICK:
	{
		V2d parentDir = normalize(pool->parentEnemy->GetPosition() - GetPosition());
		surfaceMover->SetVelocity(parentDir * startUnDodgeSpeed);
		unDodgeSpeed = startUnDodgeSpeed;
		break;
	}	
	case MACHINEGUNTURRET_STICK:
	{
		action = TURRET;
		frame = 0;
		break;
	}
	}

	if (shurType != MACHINEGUNTURRET_STICK)
	{
		action = RETHROW;
		frame = 0;
		surfaceMover->ground = NULL;
		surfaceMover->SetCollisionOn(false);
	}
}

void BirdShuriken::Die()
{
	ClearRect(quad);
	sess->RemoveEnemy(this);
	spawned = false;
	dead = true;
}

void BirdShuriken::FrameIncrement()
{
	++fireCounter;

	if (action == THROWN)
	{
		if (shurType == SLIGHTHOMING || shurType == UNBLOCKABLE)
		{
			--framesToLive;
			if (framesToLive == 0)
			{
				Die();
			}
		}
	}
	else if (action == RETHROW)
	{
		if (shurType == SLIGHTHOMING_STICK
			|| shurType == SURFACENORMAL_STICK
			|| shurType == UNBLOCKABLE_STICK
			|| shurType == MACHINEGUNTURRET_STICK)
		{
			--framesToLive;
			if (framesToLive == 0)
			{
				Die();
			}
		}
	}
}

void BirdShuriken::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case THROWN:
			break;
		case STICK:
			break;
		case TURRET:
			break;
		}
		frame = 0;
	}

	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	V2d diff = playerPos - position;
	V2d pDir = normalize(diff);

	if (action == THROWN)
	{
		switch (shurType)
		{
		case SLIGHTHOMING:
		case SLIGHTHOMING_STICK:
		case SURFACENORMAL_STICK:
		case UNBLOCKABLE_STICK:
		case UNDODGEABLE_STICK:
		case MACHINEGUNTURRET_STICK:
		case RETURN_STICK:
		{
			V2d vel = surfaceMover->GetVel();
			vel += pDir * accel;
			surfaceMover->SetVelocity(normalize(vel) * thrownSpeed);
			break;
		}
		case UNBLOCKABLE:
		{
			V2d vel = surfaceMover->GetVel();

			vel += pDir * homingAccel;
			if (length(vel) > homingSpeed)
			{
				surfaceMover->SetVelocity(normalize(vel) * homingSpeed);
			}
			else
			{
				surfaceMover->SetVelocity(vel);
			}
			break;
		}
		case UNDODGEABLE:
		{
			surfaceMover->SetVelocity(pDir * unDodgeSpeed);
			unDodgeSpeed += unDodgeAccel;
			if (unDodgeSpeed > unDodgeMaxSpeed)
			{
				unDodgeSpeed = unDodgeMaxSpeed;
			}
			break;
		}
		}
	}
	else if (action == RETHROW)
	{
		switch (shurType)
		{
		case SLIGHTHOMING_STICK:
		{
			V2d vel = surfaceMover->GetVel();
			vel += pDir * accel;
			surfaceMover->SetVelocity(normalize(vel) * thrownSpeed);
			break;
		}
		case SURFACENORMAL_STICK:
		{
			break;
		}
		case UNBLOCKABLE_STICK:
		{
			V2d vel = surfaceMover->GetVel();

			vel += pDir * homingAccel;
			if (length(vel) > homingSpeed)
			{
				surfaceMover->SetVelocity(normalize(vel) * homingSpeed);
			}
			else
			{
				surfaceMover->SetVelocity(vel);
			}
			break;
		}
		case UNDODGEABLE_STICK:
		{
			surfaceMover->SetVelocity(pDir * unDodgeSpeed);
			unDodgeSpeed += unDodgeAccel;
			if (unDodgeSpeed > unDodgeMaxSpeed)
			{
				unDodgeSpeed = unDodgeMaxSpeed;
			}
			break;
		}
		case MACHINEGUNTURRET_STICK:
		{
			//if( (fireCounter == 0 || fireCounter == 10 || fireCounter == 20/*framesBetween - 1*/) && slowCounter == 1 )// frame == 0 && slowCounter == 1 )
			if (slowCounter == 1 && action == TURRET)//&& action == FLY )
			{
				int f = fireCounter % 60;

				if (f % 5 == 0 && f >= 25 && f < 50)
				{
					launchers[0]->position = position;
					launchers[0]->facingDir = pDir;
					launchers[0]->Fire();
				}
			}
			break;
		}
		case RETURN_STICK:
		{
			V2d parentDir = normalize(pool->parentEnemy->GetPosition() - GetPosition());
			surfaceMover->SetVelocity(parentDir * unDodgeSpeed );
			unDodgeSpeed += unDodgeAccel;
			if (unDodgeSpeed > unDodgeMaxSpeed)
			{
				unDodgeSpeed = unDodgeMaxSpeed;
			}
			break;
		}
		}
	}

	
}

void BirdShuriken::IHitPlayer(int index)
{
	if (action == THROWN)
	{
		if (shurType == SLIGHTHOMING || shurType == UNBLOCKABLE || shurType == UNDODGEABLE)
		{
			Die();
		}
	}
	else if (action == RETHROW)
	{
		if (shurType == SLIGHTHOMING_STICK
			|| shurType == UNBLOCKABLE_STICK
			|| shurType == UNDODGEABLE_STICK)
		{
			Die();
		}
	}
}

void BirdShuriken::UpdateEnemyPhysics()
{
	Enemy::UpdateEnemyPhysics();

	hitboxInfo->hitPosType = HitboxInfo::GetAirType(surfaceMover->GetVel());
}

void BirdShuriken::UpdateSprite()
{
	ts->SetQuadSubRect(quad, 0);

	SetRectRotation(quad, currentRot, 128, 128, GetPositionF());
	//SetRectCenter(quad, 128, 128, GetPositionF());

	//SetRectCenter(quad, 128, 128, GetPositionF());

	if (shurType == UNDODGEABLE)
	{
		SetRectColor(quad, Color::Red);
	}
	else
	{
		SetRectColor(quad, Color::White);
	}

	currentRot += rotSpeed;
	/*if (currentRot > PI * 2)
	{
		currentRot -= PI * 2;
	}
	else if (currentRot < -PI * 2)
	{
		currentRot += PI * 2;
	}*/
}

void BirdShuriken::EnemyDraw(sf::RenderTarget *target)
{
	//DrawSprite(target, sprite);
}

void BirdShuriken::HandleHitAndSurvive()
{
	fireCounter = 0;
}

bool BirdShuriken::IsHitFacingRight()
{
	return surfaceMover->GetVel().x > 0;
}

void BirdShuriken::HitTerrainAerial(Edge *e, double q)
{
	if (shurType == MACHINEGUNTURRET_STICK)
	{
		action = TURRET;
		frame = 0;
		fireCounter = 0;
	}
	else
	{
		action = STICK;
		frame = 0;
	}
	
	//ClearRect(quad);


	//spawned = false;
	//sess->RemoveEnemy(this);


}