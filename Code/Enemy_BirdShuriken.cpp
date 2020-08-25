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
	numShurs = 10;
	shurVec.resize(numShurs);
	verts = new Vertex[numShurs * 4];
	for (int i = 0; i < numShurs; ++i)
	{
		shurVec[i] = new BirdShuriken(verts + 4 * i);
		shurVec[i]->pool = this;
	}

	Session *sess = Session::GetSession();

	parentEnemy = p_parentEnemy;

	ts = sess->GetSizedTileset("Bosses/Bird/shuriken01_128x128.png");
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

	Tileset *ts_basicBullets = sess->GetTileset("Enemies/bullet_64x64.png", 64, 64);

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::BAT, 16, 1, GetPosition(),
		V2d(1, 0), 0, 120, false, 0, 0, ts_basicBullets );
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->damage = 18;
	frame = 0;

	ts = sess->GetSizedTileset("Bosses/Bird/shuriken01_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	//BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	//cutObject->Setup(ts, 53, 52, scale);

	thrownSpeed = 10;
	accel = .1;

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

	surfaceMover->velocity = V2d();

	action = THROWN;
	frame = 0;

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

void BirdShuriken::Throw( V2d &pos, V2d &dir, int p_shurType)
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);

	shurType = p_shurType;
	surfaceMover->collisionOn = true;

	surfaceMover->velocity = dir * thrownSpeed;
	framesToLive = 120;

	switch (shurType)
	{
	case UNDODGEABLE:
	{
		unDodgeSpeed = startUnDodgeSpeed;
		surfaceMover->collisionOn = false;
		break;
	}
	case UNBLOCKABLE:
	{
		surfaceMover->velocity = dir * homingSpeed;
		surfaceMover->collisionOn = false;
		break;
	}
	case SLIGHTHOMING:
	{
		surfaceMover->collisionOn = false;
		break;
	}
		
	}
	
	action = THROWN;
	frame = 0;
	
}

void BirdShuriken::Rethrow()
{
	
	
	V2d pDir = normalize(sess->GetPlayerPos(0) - GetPosition());

	if( shurType != MACHINEGUNTURRET_STICK)
	{
		action = RETHROW;
		frame = 0;
		surfaceMover->ground = NULL;
		surfaceMover->collisionOn = false;
	}
	
	framesToLive = 120;

	switch (shurType)
	{
	case SLIGHTHOMING_STICK:
	{
		surfaceMover->velocity = pDir * thrownSpeed;
		break;
	}
	case SURFACENORMAL_STICK:
	{
		surfaceMover->velocity = surfaceMover->ground->Normal() * linearSpeed;
		break;
	}
	case UNDODGEABLE_STICK:
	{
		surfaceMover->velocity = pDir * startUnDodgeSpeed;
		unDodgeSpeed = startUnDodgeSpeed;
		break;
	}
	case UNBLOCKABLE_STICK:
	{
		surfaceMover->velocity = surfaceMover->ground->Normal() * homingSpeed;
		unDodgeSpeed = startUnDodgeSpeed;
		break;
	}
	case RETURN_STICK:
	{
		V2d parentDir = normalize(pool->parentEnemy->GetPosition() - GetPosition());
		surfaceMover->velocity = parentDir * startUnDodgeSpeed;
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
}

void BirdShuriken::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	//V2d vel = b->velocity;
	//double angle = atan2( vel.y, vel.x );
	V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);

	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);
}

void BirdShuriken::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
	
	b->launcher->DeactivateBullet(b);
}

void BirdShuriken::DirectKill()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[0]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}
	receivedHit = NULL;
}

void BirdShuriken::Die()
{
	ClearRect(quad);
	sess->RemoveEnemy(this);
	spawned = false;
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
			surfaceMover->velocity += pDir * accel;
			surfaceMover->velocity = normalize(surfaceMover->velocity) * thrownSpeed;
			break;
		}
		case UNBLOCKABLE:
		{
			surfaceMover->velocity += pDir * homingAccel;
			if (length(surfaceMover->velocity) > homingSpeed)
			{
				surfaceMover->velocity = normalize(surfaceMover->velocity) * homingSpeed;
			}
			break;
		}
		case UNDODGEABLE:
		{
			surfaceMover->velocity = pDir * unDodgeSpeed;
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
			surfaceMover->velocity += pDir * accel;
			surfaceMover->velocity = normalize(surfaceMover->velocity) * thrownSpeed;
			break;
		}
		case SURFACENORMAL_STICK:
		{
			break;
		}
		case UNBLOCKABLE_STICK:
		{
			surfaceMover->velocity += pDir * homingAccel;
			if (length(surfaceMover->velocity) > homingSpeed)
			{
				surfaceMover->velocity = normalize(surfaceMover->velocity) * homingSpeed;
			}
			break;
		}
		case UNDODGEABLE_STICK:
		{
			surfaceMover->velocity = pDir * unDodgeSpeed;
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
			surfaceMover->velocity = parentDir * unDodgeSpeed;
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

//void BirdShuriken::UpdateEnemyPhysics()
//{
//	V2d movementVec = velocity;
//	movementVec /= slowMultiple * (double)numPhysSteps;
//
//	currPosInfo.position += movementVec;
//}

void BirdShuriken::UpdateSprite()
{
	ts->SetQuadSubRect(quad, 0);
	SetRectCenter(quad, 128, 128, GetPositionF());
}

void BirdShuriken::EnemyDraw(sf::RenderTarget *target)
{
	//DrawSprite(target, sprite);
}

void BirdShuriken::HandleHitAndSurvive()
{
	fireCounter = 0;
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