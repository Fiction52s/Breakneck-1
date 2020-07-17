#include "Sequence.h"
#include "Session.h"

using namespace std;
using namespace sf;

SuperSequence::SuperSequence()
{
	SetRectColor(blackQuad, Color( 0, 0, 0, 100 ));
	SetRectTopLeft(blackQuad, 1920, 1080, Vector2f());
}

void SuperSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[LIGHTNING] = 30;
}

void SuperSequence::ReturnToGame()
{
}

void SuperSequence::AddShots()
{

}

void SuperSequence::AddFlashes()
{

}

void SuperSequence::AddEnemies()
{

}

void SuperSequence::UpdateState()
{

}

void SuperSequence::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer == EffectLayer::BEHIND_ENEMIES)
	{
		sf::View oldView = target->getView();
		target->setView(sess->uiView);
		target->draw(blackQuad, 4, sf::Quads);
		target->setView(oldView);
	}
}