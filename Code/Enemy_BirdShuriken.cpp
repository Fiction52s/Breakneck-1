#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BirdShuriken.h"

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

BirdShurikenPool::BirdShurikenPool()
{
	ts = NULL;
	numShurs = 10;
	shurVec.resize(numShurs);
	verts = new Vertex[numShurs * 4];
	for (int i = 0; i < numShurs; ++i)
	{
		shurVec[i] = new BirdShuriken(verts + 4 * i);
	}

	Session *sess = Session::GetSession();

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

BirdShuriken * BirdShurikenPool::Throw(V2d &pos, V2d &dir)
{
	BirdShuriken *bs = NULL;
	for (int i = 0; i < numShurs; ++i)
	{
		bs = shurVec[i];
		if ( !bs->spawned )
		{
			bs->Throw(pos, dir);
			break;
		}
	}
	return bs;
}

void BirdShurikenPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, shurVec.size() * 4, sf::Quads, ts->texture);
}

BirdShuriken::BirdShuriken(sf::Vertex *myQuad )
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

void BirdShuriken::Throw( V2d &pos, V2d &dir)
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);
	surfaceMover->velocity = dir * thrownSpeed;
	
	action = THROWN;
	frame = 0;
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
	sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
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

void BirdShuriken::FrameIncrement()
{
	++fireCounter;
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

	/*V2d extra(0, 0);
	V2d velDir = normalize(surfaceMover->velocity);
	double res = cross(pDir, velDir);
	if ( res > 0)
	{
		extra = V2d(velDir.y, -velDir.x) * accel;
	}
	else if (res < 0)
	{
		extra = V2d(velDir.y, -velDir.x) * -accel;
	}

	surfaceMover->velocity += extra;*/
	surfaceMover->velocity += pDir * accel;
	surfaceMover->velocity = normalize(surfaceMover->velocity) * thrownSpeed;

	//if( (fireCounter == 0 || fireCounter == 10 || fireCounter == 20/*framesBetween - 1*/) && slowCounter == 1 )// frame == 0 && slowCounter == 1 )
	if (slowCounter == 1 && action == TURRET )//&& action == FLY )
	{
		int f = fireCounter % 60;

		if (f % 5 == 0 && f >= 25 && f < 50)
		{
			launchers[0]->position = position;
			launchers[0]->facingDir = pDir;
			launchers[0]->Fire();
		}
	}
}

void BirdShuriken::IHitPlayer(int index)
{
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
	action = TURRET;
	frame = 0;
	fireCounter = 0;
	//ClearRect(quad);


	//spawned = false;
	//sess->RemoveEnemy(this);


}