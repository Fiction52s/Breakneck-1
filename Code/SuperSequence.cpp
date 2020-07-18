#include "Sequence.h"
#include "Session.h"

using namespace std;
using namespace sf;

SuperSequence::SuperSequence()
{
	SetRectColor(blackQuad, Color( 0, 0, 0, 100 ));
	SetRectTopLeft(blackQuad, 1920, 1080, Vector2f());

	ts_superLightning = sess->GetTileset("Bosses/Gator/super_lightning_960x540.png", 940, 700);
	superLightningSpr.setTexture(*ts_superLightning->texture);
	superLightningSpr.setScale(2, 2);
}

void SuperSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[LIGHTNING] = 16 * 4;
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
	superLightningSpr.setTextureRect(ts_superLightning->GetSubRect(frame / 4));
}

void SuperSequence::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer == EffectLayer::BEHIND_ENEMIES)
	{
		sf::View oldView = target->getView();
		target->setView(sess->uiView);
		target->draw(blackQuad, 4, sf::Quads);

		target->draw(superLightningSpr);

		target->setView(oldView);
	}
}