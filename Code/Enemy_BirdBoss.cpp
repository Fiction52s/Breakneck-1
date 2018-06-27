#include "Enemy_BirdBoss.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

BirdBoss::BirdBoss( GameSession *owner, Vector2i &pos )
	:Enemy( owner, EnemyType::EN_BOSS_BIRD, false, 1,false )
{
	position = V2d(pos);
	numChoices = 3;
	choices = new ChoiceParams[numChoices];
	tilesets[MOVE] = owner->GetTileset("Bosses/Bird/glide_256x256.png", 256, 256);

	actionLength[WAIT] = 0;
	actionLength[MOVE] = 0;
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

	spawnRect = sf::Rect<double>(position.x - 32, position.y - 32,
		64, 64);

	ResetEnemy();
}

void BirdBoss::ResetEnemy()
{
	PlanChoices();
	choiceIndex = 0;
	BeginChoice();
}

BirdBoss::~BirdBoss()
{
	delete[]choices;
}

void BirdBoss::BeginChoice()
{
	switch (choices[choiceIndex].cType)
	{
	C_FLYTOWARDS:
		BeginMove();
		break;
	C_FOCUS:
		BeginFocus();
		break;
	C_PUNCH:
		BeginPunch();
		break;
	C_RINGTHROW:
		BeginThrow();
		break;
	C_GRAVITYCHOOSE:
		BeginGravityChoose();
		break;
	C_SUPERKICK:
		BeginSuperKick();
		break;
	}
}

void BirdBoss::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		if (choiceIndex == numChoices - 1)
		{
			PlanChoices();
			choiceIndex = 0;
		}
		else
		{
			choiceIndex++;
			BeginChoice();
		}
		/*switch (action)
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
		}*/
	}
	
	
}

void BirdBoss::BeginMove()
{
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
		switch (r)
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