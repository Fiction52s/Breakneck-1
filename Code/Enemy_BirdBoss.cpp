#include "Enemy_BirdBoss.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

BirdBoss::BirdBoss( GameSession *owner, Vector2i &pos )
	:Enemy( owner, EnemyType::EN_BOSS_BIRD, false, 1,false )
{
	ringPool = new ObjectPool;
	for (int i = 0; i < 100; ++i)
	{
		GravRing *r = new GravRing(owner, this, ringPool, i);
		ringPool->AddToInactiveList(r);
	}

	position = V2d(pos);
	origPos = position;
	numChoices = 4;//hold the next choice to be slid in
	choices = new ChoiceParams[numChoices];
	tilesets[MOVE] = owner->GetTileset("Bosses/Bird/glide_256x256.png", 256, 256);
	
	actionLength[WAIT] = 1;
	actionLength[MOVE] = 1;
	actionLength[STARTGLIDE] = 0;
	actionLength[GLIDE] = 0;
	actionLength[ENDGLIDE] = 0;
	actionLength[STARTFOCUS] = 30;
	actionLength[FOCUSLOOP] = 1;
	actionLength[ENDFOCUS] = 30;
	actionLength[FOCUSATTACK] = 20;
	actionLength[STARTPUNCH] = 20;
	actionLength[HOLDPUNCH] = 1;
	actionLength[PUNCH] = 50;
	actionLength[RINGTHROW] = 30;
	actionLength[GRAVITYCHOOSE] = 30;
	actionLength[AIMSUPERKICK] = 30;
	actionLength[SUPERKICK] = 50;
	actionLength[SUPERKICKRECOVER] = 50;
	actionLength[SUPERKICKIMPACT] = 30;

	animFactor[WAIT] = 1;
	animFactor[MOVE] = 1;
	animFactor[STARTGLIDE] = 1;
	animFactor[GLIDE] = 1;
	animFactor[ENDGLIDE] = 1;
	animFactor[STARTFOCUS] = 1;
	animFactor[FOCUSLOOP] = 1;
	animFactor[ENDFOCUS] = 1;
	animFactor[FOCUSATTACK] = 1;
	animFactor[STARTPUNCH] = 1;
	animFactor[HOLDPUNCH] = 1;
	animFactor[PUNCH] = 1;
	animFactor[RINGTHROW] = 1;
	animFactor[GRAVITYCHOOSE] = 1;
	animFactor[AIMSUPERKICK] = 1;
	animFactor[SUPERKICK] = 1;
	animFactor[SUPERKICKRECOVER] = 1;
	animFactor[SUPERKICKIMPACT] = 1;

	spawnRect = sf::Rect<double>(position.x - 32, position.y - 32,
		64, 64);

	ResetEnemy();
}

void BirdBoss::ResetEnemy()
{
	for (int i = 0; i < numChoices; ++i)
	{
		PlanChoice(i);
	}
	position = origPos;
	//BeginChoice();
	
	action = WAIT;
	frame = 0;

	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(0));

	GravRing *r = (GravRing*)ringPool->activeListStart;
	while (r != NULL)
	{
		GravRing *grNext = (GravRing*)r->pmnext;
		r->Reset();
		r = grNext;
	}

	ringPool->DeactivateAll();
}

BirdBoss::~BirdBoss()
{
	delete[]choices;
}

void BirdBoss::BeginChoice()
{
	switch (choices[0].cType)
	{
	case C_FLYTOWARDS:
		BeginMove();
		break;
	case C_FOCUS:
		BeginFocus();
		break;
	case C_PUNCH:
		BeginPunch();
		break;
	case C_RINGTHROW:
		BeginThrow();
		break;
	case C_GRAVITYCHOOSE:
		BeginGravityChoose();
		break;
	case C_SUPERKICK:
		BeginSuperKick();
		break;
	}
}

void BirdBoss::NextChoice()
{
	for (int i = 0; i < numChoices - 1; ++i)
	{
		choices[i] = choices[i + 1];
	}
	PlanChoice(numChoices - 1);

	BeginChoice();
}

void BirdBoss::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{		
		switch (action)
		{
		case WAIT:
			frame = 0;
			break;
		case MOVE:
			frame = 0;
			break;
		case STARTGLIDE:
			action = GLIDE;
			frame = 0;
			break;
		case GLIDE:
			break;
		case ENDGLIDE:
			break;
		case STARTFOCUS:
			action = FOCUSLOOP;
			frame = 0;
			break;
		case FOCUSLOOP:
			break;
		case ENDFOCUS:
			NextChoice();
			break;
		case FOCUSATTACK:
			NextChoice();
			break;
		case STARTPUNCH:
			action = HOLDPUNCH;
			frame = 0;
			break;
		case HOLDPUNCH:
			//keep going
			break;
		case PUNCH:
			NextChoice();
			break;
		case RINGTHROW:
			NextChoice();
			break;
		case GRAVITYCHOOSE:
			NextChoice();
			break;
		case AIMSUPERKICK:
			NextChoice();
			break;
		case SUPERKICK:
			//BeginChoice();
			break;
		case SUPERKICKRECOVER:
		{
			break;
		}
		case SUPERKICKIMPACT:
		{
			action = SUPERKICKRECOVER;
			frame = 0;
			break;
		}
		}
	}
}

void BirdBoss::BeginMove()
{
	startMovePos = position;
	endMovePos = owner->GetPlayer(0)->position;
	moveSpeed = 0;
	moveAccel = .1;
	action = MOVE; //startglide conditionally here too
	frame = 0;
	velocity = V2d(0, 0);
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(0));
}

void BirdBoss::BeginFocus()
{
	action = STARTFOCUS;
	frame = 0;
	focusRadius = 0;
	velocity = V2d(0, 0);
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(0));
}

void BirdBoss::BeginPunch()
{
	action = STARTPUNCH;
	frame = 0;
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(0));
	velocity = V2d(0, 0);
	if (owner->GetPlayer(0)->position.x >= position.x)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}
}

void BirdBoss::BeginThrow()
{
	action = RINGTHROW;
	frame = 0;
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(1));

	GravRing *r = (GravRing*)ringPool->ActivatePoolMember();
	assert(r != NULL);
	r->Init( position, V2d( 0, 0 ));
	owner->AddEnemy(r);
	velocity = V2d(0, 0);
	//cout << "spawning bomb: " << fb << ": " << fb->position.x << ", " << fb->position.y << endl;
}

void BirdBoss::BeginSuperKick()
{
	action = SUPERKICK;//AIMSUPERKICK;
	frame = 0;
	superKickWaitChoices = 2;
	currTrackingFrame = 0;
	maxTrackingFrames = 60;
	//velocity = V2d(0, 0);
	rcEdge = NULL;
	V2d playerPos = owner->GetPlayer(0)->position;
	V2d dir = normalize(playerPos - position);
	velocity = dir * 25.0;
	rayStart = playerPos;
	rayEnd = playerPos + dir * 3000.0;
	RayCast(this, owner->terrainTree->startNode, rayStart, rayEnd);

	if (rcEdge != NULL)
	{
		superKickPoint = rcEdge->GetPoint(rcQuantity);
	}
	else
	{
		superKickPoint = playerPos;
		//assert(0);
	}

	//superKickPoint = owner->GetPlayer(0)->position;//rcEdge->GetPoint(rcQuantity);

	superKickStart = position;//later move this to when you activate super kick
	

	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(3));
}

void BirdBoss::HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion)
{
	V2d dir = normalize(rayEnd - rayStart);
	V2d pos = edge->GetPoint(edgeQuantity);
	double along = dot(dir, edge->Normal());
	if( along < 0 && ( rcEdge == NULL || length(edge->GetPoint(edgeQuantity) - rayStart) <
		length(rcEdge->GetPoint(rcQuantity) - rayStart)))
	{
		rcEdge = edge;
		rcQuantity = edgeQuantity;
	}
}

void BirdBoss::BeginGravityChoose()
{
	action = GRAVITYCHOOSE;
	frame = 0;
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(3));
	PoolMember *pm = ringPool->activeListStart;
	V2d fallDir(0, -1);
	double fallGrav = .1;
	while (pm != NULL)
	{
		GravRing *gr = (GravRing*)pm;
		gr->SetFall( fallDir, fallGrav );
		pm = pm->pmnext;
	}
}

void BirdBoss::PlanChoice( int ind )
{
	int r = rand() % C_Count;
	ChoiceParams &cp = choices[ind];
	cp.cType = (ChoiceType)r;

	//if (cp.cType == C_FOCUS || cp.cType == C_GRAVITYCHOOSE || cp.cType == C_SUPERKICK)
	//{
	//	cp.cType = C_FLYTOWARDS;
	//}
	//else
	//{
	//	cp.cType = C_RINGTHROW; //for testing this one thing
	//}
	//cp.cType = C_FOCUS;
		
	switch (cp.cType)
	{
	case C_FLYTOWARDS:
		cp.moveSpeed = 10.f;
		break;
	case C_FOCUS:
		cp.focusRadius = 400.f;
		break;
	case C_PUNCH:
		break;
	case C_RINGTHROW:
		cp.rType = NORMAL_LARGE;
		break;
	case C_GRAVITYCHOOSE:
		break;
	case C_SUPERKICK:
		break;
	}
}

void BirdBoss::ProcessState()
{
	ActionEnded();
	V2d playerPos = owner->GetPlayer(0)->position;
	V2d punchTarget;
	if (facingRight)
	{
		punchTarget = playerPos + V2d(-300, 0 );
	}
	else
	{
		punchTarget = playerPos + V2d(300, 0);
	}
	switch (action)
	{
	case WAIT:
		if (length(position - playerPos) < 1000.0)
		{
			BeginChoice();
		}
		break;
	case MOVE:
	{
		moveSpeed = moveSpeed + moveAccel;
		V2d along = normalize(endMovePos - startMovePos);
		double posAlong = dot(position - startMovePos, along);
		/*double test = cross(position - startMovePos, along);
		if (abs(test) > .001)
		{
			int zzzz = 5;
		}*/
		double len = length(endMovePos - startMovePos);
		if (posAlong + moveSpeed > len)
		{
			//posAlong = len;
			position = endMovePos;
			velocity = V2d(0, 0);
			NextChoice();
		}
		else
		{
			velocity = moveSpeed * along;
			//position += moveSpeed * along;
		}
		break;
	}
	case STARTGLIDE:
		break;
	case GLIDE:
		break;
	case ENDGLIDE:
		break;
	case STARTFOCUS:
		if (length(playerPos - position) < focusRadius)
		{
			action = FOCUSATTACK;
			frame = 0;
			position = playerPos;
			break;
		}
		if (frame == 60)
		{
			action = ENDFOCUS;
			frame = 0;
		}
		break;
	case FOCUSLOOP:
		if (length(playerPos - position) < focusRadius)
		{
			action = FOCUSATTACK;
			frame = 0;
			position = playerPos;
			break;
		}
		if (frame == 60)
		{
			action = ENDFOCUS;
			frame = 0;
		}
		break;
	case ENDFOCUS:
		break;
	case FOCUSATTACK:
		break;
	case STARTPUNCH:
		break;
	case HOLDPUNCH:
	{
		if ( (((velocity.y >= 0 && position.y > punchTarget.y)|| (velocity.y < 0 && position.y < punchTarget.y) ) && abs( position.x - punchTarget.x ) < 200 ) || frame > 180)
		{
			action = PUNCH;
			frame = 0;
			if (facingRight)
			{
				velocity.x = 20;
			}
			else
			{
				velocity.x = -20;
			}
			velocity.y = 0;
		}
		else
		{
			double seekAccel = .1;
			if (punchTarget.y < position.y)
			{
				velocity.y -= seekAccel;
			}
			else if (punchTarget.y > position.y)
			{
				velocity.y += seekAccel;
			}
			else
			{
				//velocity.y = 0;
			}

			if (punchTarget.x > position.x + 20)
			{
				velocity.x = 10;
			}
			else if (punchTarget.x < position.x - 20)
			{
				velocity.x = -10;
			}
			else
			{
				velocity.x = 0;
			}
			//cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
			//velocity = punchVel;
			//position += punchVel;
		}
		break;
	}
	case PUNCH:
		velocity = V2d(velocity.x, velocity.y);
		break;
	case RINGTHROW:
		break;
	case GRAVITYCHOOSE:
		break;
	case AIMSUPERKICK:

		break;
	case SUPERKICK:
		break;
	case SUPERKICKRECOVER:
	{
		break;
	}
	case SUPERKICKIMPACT:
	{
		break;
	}
	}

	switch (action)
	{
	case WAIT:
		break;
	case MOVE:
		break;
	case STARTGLIDE:
		break;
	case GLIDE:
		break;
	case ENDGLIDE:
		break;
	case STARTFOCUS:
		focusRadius += choices[0].focusRadius / actionLength[STARTFOCUS];
		break;
	case FOCUSLOOP:
		break;
	case ENDFOCUS:
		break;
	case FOCUSATTACK:
		break;
	case STARTPUNCH:
		break;
	case HOLDPUNCH:
		break;
	case PUNCH:
		break;
	case RINGTHROW:
		break;
	case GRAVITYCHOOSE:
		break;
	case AIMSUPERKICK:
		break;
	case SUPERKICK:
	{
		velocity += normalize(superKickPoint - superKickStart) * .1;
		break;
	}
	case SUPERKICKRECOVER:
	{
		velocity += normalize(superKickPoint - superKickStart) * .1;
		break;
	}
	case SUPERKICKIMPACT:
	{
		break;
	}
		
	}

	if (length(velocity) > 60)
	{
		velocity = normalize(velocity) * 60.0;
	}

	position += velocity;

	if (action == SUPERKICK)
	{
		double len = length(position - superKickStart);
		
		if ( len >= length(superKickPoint - superKickStart))
		{
			position = superKickPoint;
			if (rcEdge == NULL)
			{
				velocity = V2d(0, 0);
				NextChoice();
			}
			else
			{
				superKickPoint = superKickPoint + rcEdge->Normal() * 300.0;
				superKickStart = position;
				velocity = V2d(0, 0);
				action = SUPERKICKIMPACT;
				frame = 0;
			}
			//NextChoice(); //not great
		}
	}
	else if (action == SUPERKICKRECOVER)
	{
		double len = length(position - superKickStart);
		if (len >= length(superKickPoint - superKickStart))
		{
			position = superKickPoint;
			superKickPoint = superKickStart + normalize(superKickPoint - superKickStart) * 300.0;
			superKickStart = position;
			velocity = V2d(0, 0);
			//action = SUPERKICKRECOVER;
			NextChoice(); //not great
		}
	}
}

void BirdBoss::HandleHitAndSurvive()
{

}

void BirdBoss::UpdateEnemyPhysics()
{

}

void BirdBoss::FrameIncrement()
{
	if (action == AIMSUPERKICK)
	{
		++currTrackingFrame;
	}
}

void BirdBoss::EnemyDraw(sf::RenderTarget *target)
{
	if (action == STARTFOCUS || action == FOCUSLOOP)
	{
		sf::CircleShape cs;
		cs.setFillColor(Color(0, 0, 255, 60));
		cs.setRadius(focusRadius);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setPosition(position.x, position.y);
		target->draw(cs);
	}
	target->draw(sprite);
	
}

void BirdBoss::UpdateSprite()
{
	switch (action)
	{
	case WAIT:
		break;
	case MOVE:
		break;
	case STARTGLIDE:
		break;
	case GLIDE:
		break;
	case ENDGLIDE:
		break;
	case STARTFOCUS:
		break;
	case FOCUSLOOP:
		break;
	case ENDFOCUS:
		break;
	case FOCUSATTACK:
		break;
	case STARTPUNCH:
		break;
	case HOLDPUNCH:
		break;
	case PUNCH:
		break;
	case RINGTHROW:
		break;
	case GRAVITYCHOOSE:
		break;
	case AIMSUPERKICK:
		break;
	case SUPERKICK:
		break;
	}

	sprite.setPosition(position.x, position.y);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void BirdBoss::UpdateHitboxes()
{

}



void BirdBoss::HandleNoHealth()
{

}

GravRing::GravRing(GameSession *owner, BirdBoss *p_parent, ObjectPool *p_myPool, int index)
	:Enemy(owner, EnemyType::EN_GRAVRING, false, 1, false),
	PoolMember(index), myPool(p_myPool)
{
	parent = p_parent;
	//preload
	owner->GetTileset("Enemies/bombexplode_512x512.png", 512, 512);

	ts = owner->GetTileset("Enemies/bomb_128x160.png", 128, 160);
	sprite.setTexture(*ts->texture);

	action = HOMING;

	actionLength[HOMING] = 1;
	actionLength[ORBITING] = 1;
	actionLength[EXPLODING] = 60;
	actionLength[FALLING] = 1;

	animFactor[HOMING] = 1;
	animFactor[ORBITING] = 1;
	animFactor[EXPLODING] = 1;
	animFactor[FALLING] = 1;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody->AddCollisionBox(0, hurtBox);


	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody->AddCollisionBox(0, hitBox);
	hitBody->hitboxInfo = hitboxInfo;


	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);
}

void GravRing::Init(V2d pos, V2d vel)
{
	double len = length(position - parent->position);
	orbitRadius = len;
	maxOrbitSpeed = 10.0;
	position = pos;
	velocity = vel;
	orbitAxis = normalize(position - parent->position);
	//mover->velocity = vel;
	action = HOMING;
	frame = 0;
}

void GravRing::ProcessState()
{
	V2d playerPos = owner->GetPlayer(0)->position;
	V2d playerDir = normalize(playerPos - position);
	V2d parentDir = normalize(parent->position - position);
	V2d parentOther = V2d(parentDir.y, -parentDir.x);
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case HOMING:
			//frame = 0;
			break;
		case ORBITING:
			//frame = 0;
			break;
		case EXPLODING:
			numHealth = 0;
			dead = true;
			owner->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
				owner->GetTileset("Enemies/bombexplode_512x512.png", 512, 512),
				position, false, 0, 10, 3, true);
			//cout << "deactivating " << this << " . currently : " << myPool->numActiveMembers << endl;
			//myPool->DeactivatePoolMember(this);
			break;
		case FALLING:
			//frame = 0;
			break;
		}
	}

	switch (action)
	{
	case HOMING:
		if (frame >= 60 && length( parent->position - position ) < 700)
		{
			action = ORBITING;
			frame = 0;
			velocity = V2d(0, 0);
			
			orbitRadius = length(position - parent->position);
			orbitAxis = normalize(position - parent->position);
			double test = dot(velocity, parentOther);
			orbitSpeed = test;
			if (orbitSpeed > maxOrbitSpeed)
			{
				orbitSpeed = maxOrbitSpeed;
			}
			else if (orbitSpeed < -maxOrbitSpeed)
			{
				orbitSpeed = -maxOrbitSpeed;
			}
			orbitSpeed = 10;
			//velocity = parentOther * orbitSpeed;
		}
		break;
	case ORBITING:
		break;
	case EXPLODING:
		break;
	case FALLING:
		if (frame == 180)
		{
			action = EXPLODING;
			frame = 0;
		}
		break;
	}

	
	double homingAccel = .1;
	double orbitAccel = .1;
	

	float orbitFrames = 180;
	double a = frame / orbitFrames * 2.0 * PI;
	switch (action)
	{
	case HOMING:
		velocity += homingAccel * playerDir;
		break;
	case ORBITING:
	{
		//parentDir * dot(velocity, parentDir);
		
		//velocity += parentDir * 1.0;
		V2d p(orbitRadius * cos(a), orbitRadius / 2 * sin(a));

		V2d orbitOther(orbitAxis.y, -orbitAxis.x);
		position = parent->position + p.x * orbitAxis + p.y * orbitOther;//V2d(p.x * orbitAxis.x, p.x * orbitAxis.y) + V2d(p.y * orbitOther.x, p.y * orbitOther.y);
		//if (orbitSpeed >= 0)
		//{
		//	orbitSpeed += orbitAccel;
		//}
		//else
		//{
		//	orbitSpeed -= orbitAccel;
		//}
		//if (orbitSpeed > maxOrbitSpeed)
		//{
		//	orbitSpeed = maxOrbitSpeed;
		//}
		//else if (orbitSpeed < -maxOrbitSpeed)
		//{
		//	orbitSpeed = -maxOrbitSpeed;
		//}

		//velocity = orbitSpeed * parentOther;//+= //orbitAccel * 3.0 * parentDir;
		
		
		break;
	}
	case EXPLODING:
		velocity = V2d(0, 0);
		break;
	case FALLING:
		velocity += fallDir * grav;
		break;
	}

	
	if (length(velocity) > 17)
	{
		velocity = normalize(velocity) * 17.0;
	}

	position += velocity;

	if (action == ORBITING)
	{
		//velocity += parentDir * orbitSpeed * 10.0;

		//V2d relative = position - parent->position;
		//double x = dot(relative, orbitAxis);
		//double y = cross(relative, orbitAxis);

		//double w = orbitRadius;
		//double h = orbitRadius / 2;
		//double res = x * x / w + y * y / h;
		////if (length(position - parent->position) > orbitRadius)
		//if( res > 1.0 )
		//{
		//	position = parent->position - parentDir * orbitRadius;
		//}
	}
}

void GravRing::SetFall( V2d &dir, double p_grav)
{
	action = FALLING;
	frame = 0;
	grav = p_grav;
	fallDir = dir;
}

void GravRing::HandleNoHealth()
{

}

void GravRing::FrameIncrement()
{

}

void GravRing::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void GravRing::IHitPlayer(int index)
{
	if (action != EXPLODING)
	{
		action = EXPLODING;
		frame = 0;
	}
}

void GravRing::UpdateSprite()
{
	switch (action)
	{
	case HOMING:
		sprite.setTextureRect(ts->GetSubRect(0));
		break;
	case ORBITING:
		sprite.setTextureRect(ts->GetSubRect(1));
		break;
	case EXPLODING:
		sprite.setTextureRect(ts->GetSubRect(5));
		break;
	case FALLING:
		sprite.setTextureRect(ts->GetSubRect(2));
		break;
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(Vector2f(position));
}

void GravRing::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;
}

void GravRing::ResetEnemy()
{
	//mover->ground = NULL;
	//mover->edgeQuantity = 0;
	//mover->roll = false;
	//mover->UpdateGroundPos();
	//mover->SetSpeed(0);
	action = HOMING;
	frame = 0;
	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);
}

void GravRing::UpdateEnemyPhysics()
{
	if (!dead )
	{
		V2d v = velocity / (slowMultiple * numPhysSteps);
		position += v;
	}	
}

void GravRing::ProcessHit()
{
	if (!dead && ReceivedHit() && action != EXPLODING)
	{
		action = EXPLODING;
		frame = 0;
	}
}