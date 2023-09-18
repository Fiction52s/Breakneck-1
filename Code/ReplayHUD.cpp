#include "ReplayHUD.h"
#include "GameSession.h"
#include "Tileset.h"
#include "MainMenu.h"
#include "Actor.h"

using namespace std;
using namespace sf;

ReplayHUD::ReplayHUD()
{
	
	//game = GameSession::GetSession();
	//SetRectTopLeft( bgQuad, )
	goingToNextFrame = false;

	sess = NULL;
	//Session *sess = Session::GetSession();

	MainMenu *mainMenu = MainMenu::GetInstance();

	SetSession();

	//sf::Text playText;
	//sf::Text advanceFrameText;

	playText.setFont(mainMenu->arial);
	playText.setCharacterSize(30);
	playText.setFillColor(Color::White);
	playText.setString("Play/Pause =");

	advanceFrameText.setFont(mainMenu->arial);
	advanceFrameText.setCharacterSize(30);
	advanceFrameText.setFillColor(Color::White);
	advanceFrameText.setString("Advance frame =");

	SetRectColor(bgQuad, Color(0, 0, 0, 100));//Color::Black);

	SetTopLeft(Vector2f(960 - 300, 970));
	//ts_buttons->SetQuadSubRect( buttonIconQuads, )
	//sf::Vertex buttonIconQuads[4 * 2];
	//sf::Text playText;
	//sf::Text advanceFrameText;
}

ReplayHUD::~ReplayHUD()
{

}
void ReplayHUD::Reset()
{
	goingToNextFrame = false;
}

void ReplayHUD::SetSession()
{
	if (sess != NULL)
		return;

	sess = Session::GetSession();
	if (sess != NULL)
	{
		MainMenu *mainMenu = MainMenu::GetInstance();
		ts_buttons = sess->GetButtonIconTileset(0);
		int cType = sess->controllerStates[0]->GetControllerType();
		SetRectSubRect(buttonIconQuads, mainMenu->GetButtonIconTileForMenu(cType, XBOX_A));
		SetRectSubRect(buttonIconQuads + 4, mainMenu->GetButtonIconTileForMenu(cType, XBOX_X));
	}
}

void ReplayHUD::SetTopLeft(sf::Vector2f pos)
{
	Vector2f topLeft = pos;

	Vector2f bgSize(600, 100);

	float buttonSize = 64;
	SetRectTopLeft(bgQuad, bgSize.x, bgSize.y, topLeft);

	playText.setPosition(topLeft + Vector2f(10, 30));

	Vector2f buttonOffset(10, 0);

	auto &playGB = playText.getGlobalBounds();
	advanceFrameText.setPosition(playGB.left + playGB.width + buttonOffset.x + buttonSize + 30, playText.getPosition().y );

	auto &advanceGB = advanceFrameText.getGlobalBounds();

	SetRectTopLeft(buttonIconQuads, buttonSize, buttonSize, Vector2f(playGB.left + playGB.width + buttonOffset.x, (playGB.top + playGB.height / 2) - buttonSize / 2));
	SetRectTopLeft(buttonIconQuads+4, buttonSize, buttonSize, Vector2f(advanceGB.left + advanceGB.width + 10, (advanceGB.top + advanceGB.height / 2) - buttonSize / 2));
}

void ReplayHUD::Update()
{
	goingToNextFrame = false;

	ControllerState currInput;

	ControllerState prevInput;


	currInput = sess->GetCurrInput(0);//GetCurrInputFiltered(0);
	prevInput = sess->GetPrevInput(0);// Filtered(0);

	bool controlsActive = sess->IsReplayHUDOn();

	if (!controlsActive)
	{
		if (sess->oneFrameMode)
		{
			sess->oneFrameMode = false;
			sess->currentTime = sess->gameClock.getElapsedTime().asSeconds();
			sess->accumulator = 0;
		}
	}
	else
	{
		if (currInput.A && !prevInput.A)
		{
			if (sess->oneFrameMode)
			{
				sess->oneFrameMode = false;
				sess->currentTime = sess->gameClock.getElapsedTime().asSeconds();
				sess->accumulator = 0;
			}
			else
			{
				sess->oneFrameMode = true;
			}
		}

		if (currInput.X && !prevInput.X)
		{
			if (sess->oneFrameMode)
			{
				goingToNextFrame = true;
			}
			else
			{
				sess->oneFrameMode = true;
				goingToNextFrame = true;
			}
		}
	}

	//if (currInput.B)
	//{
	//	if (sess->oneFrameMode)
	//	{
	//		goingToNextFrame = true;
	//	}
	//}
	//else
	
}

bool ReplayHUD::IsGoingToNextFrame()
{
	return goingToNextFrame;
}

void ReplayHUD::Draw(sf::RenderTarget *target)
{
	if (sess->IsReplayHUDOn())
	{
		target->draw(bgQuad, 4, sf::Quads);
		target->draw(playText);
		target->draw(advanceFrameText);
		target->draw(buttonIconQuads, 2 * 4, sf::Quads, ts_buttons->texture);
	}
	
}