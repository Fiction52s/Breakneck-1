#include "Enemy_BirdBoss.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

BirdBoss::BirdBoss( GameSession *owner, Vector2i &pos )
	:Enemy( owner, EnemyType::EN_BOSS_BIRD, false, 1,false )
{
	position = V2d(pos);
	origPos = position;
	numChoices = 3;
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
	actionLength[STARTPUNCH] = 0;
	actionLength[HOLDPUNCH] = 0;
	actionLength[PUNCH] = 0;
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
	animFactor[AIMSUPERKICK] = 1;
	animFactor[SUPERKICK] = 1;

	spawnRect = sf::Rect<double>(position.x - 32, position.y - 32,
		64, 64);

	ResetEnemy();
}

void BirdBoss::ResetEnemy()
{
	PlanChoices();
	choiceIndex = 0;
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
	switch (choices[choiceIndex].cType)
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
	if (choiceIndex == numChoices - 1)
	{
		PlanChoices();
		choiceIndex = 0;
		BeginChoice();
	}
	else
	{
		choiceIndex++;
		BeginChoice();
	}
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
}

void BirdBoss::BeginMove()
{
	startMovePos = position;
	endMovePos = owner->GetPlayer(0)->position;
	speed = 0;
	accel = .1;
	action = MOVE; //startglide conditionally here too
	frame = 0;
	sprite.setTexture(*tilesets[MOVE]->texture);
	sprite.setTextureRect(tilesets[MOVE]->GetSubRect(0));
}

void BirdBoss::BeginFocus()
{
	action = STARTFOCUS;
	frame = 0;
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

void BirdBoss::PlanChoices()
{
	for (int i = 0; i < numChoices; ++i)
	{
		int r = rand() % C_Count;
		ChoiceParams &cp = choices[i];
		cp.cType = (ChoiceType)r;
		cp.cType = C_FLYTOWARDS; //for testing this one thing
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
}

void BirdBoss::ProcessState()
{
	switch (action)
	{
	case WAIT:
		if (length(position - owner->GetPlayer(0)->position) < 1000.0)
		{
			BeginChoice();
		}
		break;
	case MOVE:
	{
		speed = speed + accel;
		V2d along = normalize(endMovePos - startMovePos);
		double posAlong = dot(position - startMovePos, along);
		double test = cross(position - startMovePos, along);
		if (abs(test) > .001)
		{
			int zzzz = 5;
		}
		double len = length(endMovePos - startMovePos);
		if (posAlong + speed > len)
		{
			//posAlong = len;
			position = endMovePos;
			NextChoice();
		}
		else
		{
			position += speed * along;
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