#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Pulser.h"
#include "Eye.h"
//#include "MovingGeo.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


Pulser::Pulser(GameSession *owner, bool p_hasMonitor, Vector2i pos, list<Vector2i> &pathParam, bool loopP, int p_level)
	:Enemy(owner, EnemyType::EN_PULSER, p_hasMonitor, 1)
{
	level = p_level;

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

	checkRadius = 300;
	hitRadius = checkRadius;// -50;

	action = WAIT;
	position.x = pos.x;
	position.y = pos.y;

	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	pathLength = pathParam.size() + 1;
	//cout << "pathLength: " << pathLength << endl;
	path = new Vector2i[pathLength];
	path[0] = pos;

	int index = 1;
	for (list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it)
	{
		path[index] = (*it) + pos;
		++index;
		//path.push_back( (*it) );

	}

	loop = loopP;

	speed = 3;//pspeed;



			  //speed = 2;
	frame = 0;

	//ts = owner->GetTileset( "Pulser.png", 80, 80 );
	ts = owner->GetTileset("Enemies/pulser_64x64.png", 64, 64);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);
	
	//ts_aura = owner->GetTileset("Enemies/pulser_64x64.png", 64, 64);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 20;
	hitboxInfo->knockback = 12;

	SetupBodies(1, 2);
	AddBasicHurtCircle(16);
	AddBasicHitCircle(16);
	AddBasicHitCircle(hitRadius,1);
	hitBody->hitboxInfo = hitboxInfo;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	//hitboxInfo->kbDir;

	targetNode = 1;
	forward = true;

	dead = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(5);
	cutObject->SetSubRectBack(4);
	cutObject->SetScale(scale);

	facingRight = true;


	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	actionLength[WAIT] = 4;
	actionLength[CHARGE] = 100;
	actionLength[ELECTRIFY] = 60;
	actionLength[RECOVER] = 30;

	animFactor[WAIT] = 3;
	animFactor[CHARGE] = 1;
	animFactor[ELECTRIFY] = 1;
	animFactor[RECOVER] = 1;
	
	geoGroupWait.AddGeo(new MovingRing(32, 20, checkRadius, 10, 20, Vector2f(), Vector2f(),
		Color::Yellow, Color(0, 0, 100, 0), 60));
	geoGroupWait.Init();

	geoGroupCharge.AddGeo(new MovingRing(32, 20, hitRadius, 10, 20, Vector2f(), Vector2f(),
		Color::Red, Color(0, 0, 100, 0), 20));
	geoGroupCharge.Init();

	geoGroupExplode.AddGeo(new MovingRing(32, 0, 0, 10, hitRadius, Vector2f(), Vector2f(),
		Color::Yellow, Color::Yellow, 10));
	
	geoGroupExplode.AddGeo(new MovingRing(32, 0, 0, 10, hitRadius, Vector2f(), Vector2f(),
		Color::White, Color::White, 10), 5);

	geoGroupExplode.AddGeo(new MovingRing(32, 0, 0, 10, hitRadius, Vector2f(), Vector2f(),
		Color::Yellow, Color::White, 10), 5);
	geoGroupExplode.Init();
	

	ResetEnemy();
}

Pulser::~Pulser()
{
	delete[]path;
}

void Pulser::ResetEnemy()
{
	geoGroupWait.Reset();
	geoGroupCharge.Reset();
	geoGroupExplode.Reset();
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	targetNode = 1;
	forward = true;
	dead = false;
	action = WAIT;
	frame = 0;
	position.x = path[0].x;
	position.y = path[0].y;
	receivedHit = NULL;

	geoGroupWait.SetBase(Vector2f(position.x, position.y));
	geoGroupCharge.SetBase(Vector2f(position.x, position.y));
	geoGroupExplode.SetBase(Vector2f(position.x, position.y));


	UpdateHitboxes();

	UpdateSprite();
}

void Pulser::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case WAIT:
			break;
		case CHARGE:
		{
			action = ELECTRIFY;
			SetHitboxes(hitBody, 1);
			break;
		}
		case ELECTRIFY:
			//action = WAIT;
			break;
		}
	}

	switch (action)
	{
	case WAIT:
		
		break;
	case CHARGE:
		break;
	case ELECTRIFY:
		break;
	}

	if (action != WAIT)
	{
		if (geoGroupWait.running)
		{
			if (!geoGroupWait.Update())
			{
				geoGroupWait.Reset();
			}
		}
	}
	else
	{
		if (!geoGroupWait.Update())
		{
			geoGroupWait.Reset();
		}
	}

	if (action != CHARGE)
	{
		if (geoGroupCharge.running)
		{
			if (!geoGroupCharge.Update())
			{
				geoGroupCharge.Reset();
			}
		}
	}
	else
	{
		if (!geoGroupCharge.Update())
		{
			geoGroupCharge.Reset();
		}
	}

	if (action == ELECTRIFY)
	{
		if (!geoGroupExplode.Update())
		{
			geoGroupExplode.Reset();
			action = WAIT;
			frame = 0;
			SetHitboxes(hitBody, 0);
		}
	}
	

	V2d playerPos = owner->GetPlayer(0)->position;
	if (action == WAIT && length(playerPos - position) < checkRadius)
	{
		action = CHARGE;
		frame = 0;
	}
}

void Pulser::UpdateEnemyPhysics()
{
	double movement = speed / numPhysSteps;

	if (pathLength > 1)
	{
		movement /= (double)slowMultiple;

		while (movement != 0)
		{
			//cout << "movement loop? "<< endl;
			V2d targetPoint = V2d(path[targetNode].x, path[targetNode].y);
			V2d diff = targetPoint - position;
			double len = length(diff);
			if (len >= abs(movement))
			{
				position += normalize(diff) * movement;
				movement = 0;
			}
			else
			{
				position += diff;
				movement -= length(diff);
				AdvanceTargetNode();
			}
		}
	}
}

void Pulser::AdvanceTargetNode()
{
	if (loop)
	{
		++targetNode;
		if (targetNode == pathLength)
			targetNode = 0;
	}
	else
	{
		if (forward)
		{
			++targetNode;
			if (targetNode == pathLength)
			{
				targetNode -= 2;
				forward = false;
			}
		}
		else
		{
			--targetNode;
			if (targetNode < 0)
			{
				targetNode = 1;
				forward = true;
			}
		}
	}
}

void Pulser::UpdateSprite()
{

	geoGroupWait.SetBase(Vector2f(position.x, position.y));
	geoGroupCharge.SetBase(Vector2f(position.x, position.y));
	geoGroupExplode.SetBase(Vector2f(position.x, position.y));

	int tile = 0;
	switch (action)
	{
	case WAIT:
		tile = frame / animFactor[WAIT];
		break;
	case CHARGE:
		tile = 3;
		break;
	case ELECTRIFY:
		tile = 2;
	case RECOVER:
		tile = 0;
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(tile));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(Vector2f(position));
}

void Pulser::EnemyDraw(sf::RenderTarget *target)
{
	geoGroupWait.Draw( target );

	geoGroupCharge.Draw(target);

	geoGroupExplode.Draw(target);

	DrawSpriteIfExists(target, sprite);
}

void Pulser::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(currHitboxFrame)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	if (owner->GetPlayer(0)->ground != NULL)
	{
		hitboxInfo->kbDir = normalize(-owner->GetPlayer(0)->groundSpeed * (owner->GetPlayer(0)->ground->v1 - owner->GetPlayer(0)->ground->v0));
	}
	else
	{
		hitboxInfo->kbDir = normalize(owner->GetPlayer(0)->position - position);//normalize(-owner->GetPlayer(0)->velocity);
	}
}