#include "Sequence.h"
#include "Session.h"
#include "SoundManager.h"

using namespace std;
using namespace sf;

SuperSequence::SuperSequence()
{
	SetRectColor(blackQuad, Color( 0, 0, 0, 150 ));
	SetRectTopLeft(blackQuad, 1920, 1080, Vector2f());

	ts_lit1 = sess->GetSizedTileset("FX/Multiplayer/lit_01_960x540.png");
	ts_lit2 = sess->GetSizedTileset("FX/Multiplayer/lit_02_960x540.png");
	ts_spark1 = sess->GetSizedTileset("FX/Multiplayer/spark_01_960x540.png");
	ts_spark2 = sess->GetSizedTileset("FX/Multiplayer/spark_01_960x540.png");

	ts_birdPortrait = sess->GetTileset("Enemies/Bosses/Bird/Bird_Super_01b.png", 1920, 1080);

	animFactor = 3;

	birdSound = sess->GetSound("Enemies/Bosses/bird_super_01");

	portraitLeftStart = Vector2f(-500, 0);
	portraitLeftEnd = Vector2f(0, 0);
	portraitEnterFrames = 20;
	portraitFrame = 0;

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

void SuperSequence::Reset()
{
	Sequence::Reset();
	portraitFrame = 0;
	portraitSprite.setPosition(portraitLeftStart);
}

void SuperSequence::SetSuperType(int p_sType)
{
	sType = (SuperType)p_sType;
	switch (sType)
	{

	}
	
	ts_birdPortrait->SetSpriteTexture(portraitSprite);
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
	if (seqData.frame == 0)
	{
		switch (seqData.state)
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

	superLightningSpr.setTextureRect(ts_spark1->GetSubRect(seqData.frame / animFactor));
	superLightningSpr.setOrigin(superLightningSpr.getLocalBounds().width / 2,
		superLightningSpr.getLocalBounds().height / 2);

	
	float portion = portraitFrame / (float)portraitEnterFrames;
	float fac = portraitBez.GetValue(portion);

	if (portraitFrame == 0)
	{
		sess->ActivateSound(birdSound);
	}

	if (fac <= 1.f)
	{
		Vector2f portraitPos = portraitLeftStart * (1.f - fac) + portraitLeftEnd * fac;
		portraitSprite.setPosition(portraitPos);
	}
	

	

	++portraitFrame;
	
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

		target->draw(portraitSprite);

		//V2d( 960, 540 ) + (sess->GetPlayerPos(0) - V2d(sess->cam.GetPos()));

		if (seqData.state == SPARK1 || seqData.state == SPARK2)
		{
			superLightningSpr.setPosition(playerPixel.x, playerPixel.y);//960, 540);
		}
		else if (seqData.state == LIT1)
		{
			superLightningSpr.setPosition(960, playerPixel.y);
		}
		else if (seqData.state == LIT2)
		{
			superLightningSpr.setPosition(playerPixel.x, 540);
		}

		target->draw(superLightningSpr);

		

		target->setView(oldView);
	}
}