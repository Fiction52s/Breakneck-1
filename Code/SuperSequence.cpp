#include "Sequence.h"
#include "Session.h"

using namespace std;
using namespace sf;

SuperSequence::SuperSequence()
{
	SetRectColor(blackQuad, Color( 0, 0, 0, 150 ));
	SetRectTopLeft(blackQuad, 1920, 1080, Vector2f());

	ts_lit1 = sess->GetSizedTileset("FX/lit_01_960x540.png");
	ts_lit2 = sess->GetSizedTileset("FX/lit_02_960x540.png");
	ts_spark1 = sess->GetSizedTileset("FX/spark_01_960x540.png");
	ts_spark2 = sess->GetSizedTileset("FX/spark_01_960x540.png");

	animFactor = 3;

	//superLightningSpr.setTexture(*ts_superLightning->texture);
	superLightningSpr.setScale(2, 2);
}

void SuperSequence::SetupStates()
{
	SetNumStates(Count);

	
	stateLength[SPARK1] = 4 * animFactor;
	stateLength[SPARK2] = 4 * animFactor;
	stateLength[LIT1] = 4 * animFactor;
	stateLength[LIT2] = 4 * animFactor;
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
	if (frame == 0)
	{
		switch (state)
		{
		case SPARK1:
			superLightningSpr.setTexture(*ts_spark1->texture);
			break;
		case SPARK2:
			superLightningSpr.setTexture(*ts_spark2->texture);
			break;
		case LIT1:
			superLightningSpr.setTexture(*ts_lit1->texture);
			break;
		case LIT2:
			superLightningSpr.setTexture(*ts_lit2->texture);
			break;
		}
	}

	superLightningSpr.setTextureRect(ts_spark1->GetSubRect(frame / animFactor));
	superLightningSpr.setOrigin(superLightningSpr.getLocalBounds().width / 2,
		superLightningSpr.getLocalBounds().height / 2);

	

	
}

void SuperSequence::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer == EffectLayer::BEHIND_ENEMIES)
	{
		Vector2i playerPixel = target->mapCoordsToPixel(Vector2f(sess->GetPlayerPos(0)));
		//V2d playerUIPos = 

		sf::View oldView = target->getView();
		target->setView(sess->uiView);
		target->draw(blackQuad, 4, sf::Quads);

		//V2d( 960, 540 ) + (sess->GetPlayerPos(0) - V2d(sess->cam.GetPos()));

		if (state == SPARK1 || state == SPARK2)
		{
			superLightningSpr.setPosition(playerPixel.x, playerPixel.y);//960, 540);
		}
		else if (state == LIT1)
		{
			superLightningSpr.setPosition(960, playerPixel.y);
		}
		else if (state == LIT2)
		{
			superLightningSpr.setPosition(playerPixel.x, 540);
		}

		target->draw(superLightningSpr);

		target->setView(oldView);
	}
}