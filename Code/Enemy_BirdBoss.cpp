#include "Enemy_BirdBoss.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

BirdBoss::BirdBoss( GameSession *owner, Vector2i &pos )
	:Enemy( owner, EnemyType::EN_BOSS_BIRD, false, 1,false )
{
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
	actionLength[STARTFOCUS] = 0;
	actionLength[FOCUSLOOP] = 0;
	actionLength[ENDFOCUS] = 0;
	actionLength[FOCUSATTACK] = 0;
	actionLength[STARTPUNCH] = 20;
	actionLength[HOLDPUNCH] = 1;
	actionLength[PUNCH] = 30;
	actionLength[RINGTHROW] = 0;
	actionLength[GRAVITYCHOOSE] = 0;
	actionLength[AIMSUPERKICK] = 0;
	actionLength[SUPERKICK] = 0;

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
	animFactor[AIMSUPERKICK] = 30;
	animFactor[SUPERKICK] = 1;

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
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(0));
}

void BirdBoss::BeginFocus()
{
	action = STARTFOCUS;
	frame = 0;
	focusRadius = 0;
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(0));
}

void BirdBoss::BeginPunch()
{
	action = STARTPUNCH;
	frame = 0;
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(0));
}

void BirdBoss::BeginThrow()
{
	action = RINGTHROW;
	frame = 0;
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(1));
}

void BirdBoss::BeginSuperKick()
{
	action = AIMSUPERKICK;
	frame = 0;
	superKickWaitChoices = 2;
	currTrackingFrame = 0;
	maxTrackingFrames = 60;
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(2));
}

void BirdBoss::BeginGravityChoose()
{
	action = GRAVITYCHOOSE;
	frame = 0;
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(3));
}

void BirdBoss::PlanChoice( int ind )
{
	int r = rand() % C_Count;
	ChoiceParams &cp = choices[ind];
	cp.cType = (ChoiceType)r;
	cp.cType = C_PUNCH; //for testing this one thing
	switch (cp.cType)
	{
	case C_FLYTOWARDS:
		cp.moveSpeed = 10.f;
		break;
	case C_FOCUS:
		cp.focusRadius = 100.f;
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
			NextChoice();
		}
		else
		{
			position += moveSpeed * along;
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
	{
		if (length(position - playerPos) < 20 || frame > 60)
		{
			action = PUNCH;
			frame = 0;
		}
		else
		{
			if (playerPos.y < position.y - 20)
			{
				punchVel.y = -10;
			}
			else if (playerPos.y > position.y + 20)
			{
				punchVel.y = 10;
			}
			else
			{
				punchVel.y = 0;
			}

			if (playerPos.x > position.x + 20)
			{
				punchVel.x = 10;
			}
			else if (playerPos.x < position.x - 20)
			{
				punchVel.x = -10;
			}
			else
			{
				punchVel.x = 0;
			}


			position += punchVel;
		}
		break;
	}
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

GravRing::GravRing(GameSession *owner, ObjectPool *p_myPool, int index)
	:Enemy(owner, EnemyType::EN_GRAVRING, false, 1, false),
	PoolMember(index), myPool(p_myPool)
{
	//preload
	owner->GetTileset("Enemies/bombexplode_512x512.png", 512, 512);

	mover = new SurfaceMover(owner, NULL, 0, 32);
	mover->surfaceHandler = this;
	mover->SetSpeed(0);

	ts = owner->GetTileset("Enemies/bomb_128x160.png", 128, 160);
	sprite.setTexture(*ts->texture);

	action = FLOATING;

	actionLength[FLOATING] = 9;
	actionLength[EXPLODING] = 4;

	animFactor[FLOATING] = 3;
	animFactor[EXPLODING] = 3;

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
	position = pos;
	mover->velocity = vel;
	action = FLOATING;
	frame = 0;
	mover->physBody.globalPosition = position;
}

void GravRing::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case FLOATING:
			frame = 0;
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
		}
	}

	switch (action)
	{
	case FLOATING:
		break;
	case EXPLODING:
		break;
	}

	switch (action)
	{
	case FLOATING:
		break;
	case EXPLODING:
		break;
	}
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
	if (action == FLOATING)
	{
		action = EXPLODING;
		frame = 0;
	}
}

void GravRing::UpdateSprite()
{
	switch (action)
	{
	case FLOATING:
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[FLOATING]));
		break;
	case EXPLODING:
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[EXPLODING]));
		break;
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(Vector2f(position));
}

void GravRing::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	if (!dead)
		mover->physBody.DebugDraw(target);
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

void GravRing::HitTerrainAerial(Edge * e, double q)
{
	if (action == FLOATING)
	{
		mover->velocity = V2d(0, 0);
		action = EXPLODING;
		frame = 0;
	}
}

void GravRing::ResetEnemy()
{
	mover->ground = NULL;
	mover->edgeQuantity = 0;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);
	action = FLOATING;
	frame = 0;
	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);
}

void GravRing::UpdateEnemyPhysics()
{
	if (!dead)
	{
		mover->Move(slowMultiple, numPhysSteps);
		position = mover->physBody.globalPosition;
	}
}

void GravRing::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOATING)
	{
		action = EXPLODING;
		frame = 0;
	}

}