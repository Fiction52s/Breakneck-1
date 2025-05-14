#include "NewTitleScreen.h"
#include "Tileset.h"
#include "MainMenu.h"
#include "Background.h"
#include "MusicPlayer.h"
#include "MusicSelector.h"
#include "Fader.h"
#include "VisualEffects.h"
#include "CreditsMenuScreen.h"

#include "PostPracticeMatchMenu.h"
#include "FeedbackForm.h"

using namespace std;
using namespace sf;

NewTitleScreen::NewTitleScreen(MainMenu *p_mainMenu)
	:mainMenu(p_mainMenu)
{
	frame = 0;

	int waitFrames[] = { 60, 30, 20 };
	int waitModeThresh[] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, NUM_BUTTONS, 0);

	creditsMenuScreen = new CreditsMenuScreen;

	panel = new Panel("feedbackpanel", 1920, 1080, this);
	panel->SetColor(Color::Transparent);

	panel->ReserveImageRects(1);

	ts_buttons = GetSizedTileset("Menu/Title/buttons_806x217.png");

	ts_bg = GetSizedTileset("Menu/Title/Title_Screen_BG_410x327.png");

	SetRectTopLeft(bgQuad, 1920, 1080, Vector2f(0, 0));
	
	if (!scrollShader.loadFromFile("Resources/Shader/slider.frag", sf::Shader::Fragment))
	{
		assert(0);
	}
	scrollShader.setUniform("u_texture", *ts_bg->texture);

	xRate = -.005;
	yRate = -.002;

	SetRectSubRect(bgQuad, sf::FloatRect(0, 0, 1920.f / ts_bg->tileWidth, 1080.f / ts_bg->tileHeight ));
	
	//buttonPositions[0] = Vector2f(1960, 72);
	//buttonPositions[1] = Vector2f(1777, 72);
	//buttonPositions[2] = Vector2f(1647, 189);
	//buttonPositions[3] = Vector2f(1463, 339);
	//buttonPositions[4] = Vector2f(1306, 533);
	//buttonPositions[5] = Vector2f(1463, 726);
	//buttonPositions[6] = Vector2f(1647, 876);
	//buttonPositions[7] = Vector2f(1777, 993);
	//buttonPositions[8] = Vector2f(1960, 993);

	buttonPositions[0] = Vector2f(1777, 72);
	buttonPositions[1] = Vector2f(1647, 189);
	buttonPositions[2] = Vector2f(1463, 339);
	buttonPositions[3] = Vector2f(1306, 533);
	buttonPositions[4] = Vector2f(1463, 726);
	buttonPositions[5] = Vector2f(1647, 876);
	buttonPositions[6] = Vector2f(1777, 993);


	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		SetRectSubRect(buttonQuads + 4 * i, ts_buttons->GetSubRect(i * 2));
	}

	//SetRectSubRect(buttonQuads + 4 * 0, ts_buttons->GetSubRect(0 * 2));
	//SetRectSubRect(buttonQuads + 4 * 8, ts_buttons->GetSubRect(0 * 2));
	

	/*float buttonXOrigin = 1000;
	for (int i = 0; i < 9; ++i)
	{
		buttonPositions[i] = Vector2f(buttonXOrigin, 100 + i * 100);
	}*/
	
	/*buttonScales[0] = .3;
	buttonScales[8] = .3;

	buttonScales[1] = .45;
	buttonScales[7] = .45;

	buttonScales[2] = .6;
	buttonScales[6] = .6;

	buttonScales[3] = .75;
	buttonScales[5] = .75;

	buttonScales[4] = 1.0;*/

	offScreenButtonScale = .3;
	offScreenTopButtonPos = Vector2f(1960, 72);
	offScreenBottomButtonPos = Vector2f(1960, 993);

	//buttonScales[0] = .3;
	//buttonScales[8] = .3;

	buttonScales[0] = .45;
	buttonScales[6] = .45;

	buttonScales[1] = .6;
	buttonScales[5] = .6;

	buttonScales[2] = .75;
	buttonScales[4] = .75;

	buttonScales[3] = 1.0;

	baseButtonSize = Vector2f(ts_buttons->tileWidth, ts_buttons->tileHeight);

	/*Label *lab = panel->AddLabel("ealabel", Vector2i(960, 1080 - 30), 40, "Early Access ver. 1.0");
	auto lb = lab->text.getLocalBounds();
	lab->text.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);
	lab->text.setFillColor(Color::White);*/

	titleMusic = mainMenu->musicManager->songMap["w0_Title"];
	if (titleMusic != NULL)
	{
		titleMusic->Load();
	}
	

	/*SetRectColor(buttonQuads + 0 * 4, Color::Red);
	SetRectColor(buttonQuads + 1 * 4, Color::Green);
	SetRectColor(buttonQuads + 2 * 4, Color::Yellow);
	SetRectColor(buttonQuads + 3 * 4, Color::Cyan);
	SetRectColor(buttonQuads + 4 * 4, Color::Magenta);
	SetRectColor(buttonQuads + 5 * 4, Color::White);
	SetRectColor(buttonQuads + 6 * 4, Color::Black);
	SetRectColor(buttonQuads + 7 * 4, Color::Blue);
	SetRectColor(buttonQuads + 8 * 4, Color(50, 50, 50));*/


	actionLength[A_IDLE] = 1;
	actionLength[A_CHANGE_DOWN] = 30;
	actionLength[A_CHANGE_UP] = 30;

	Reset();
}

NewTitleScreen::~NewTitleScreen()
{
	delete creditsMenuScreen;
	delete panel;
	delete saSelector;
}

void NewTitleScreen::Reset()
{
	action = A_IDLE;
	frame = 0;

	quantX = 0;
	quantY = 0;

	saSelector->currIndex = 0;
	
	isMusicStarted = false;
}

void NewTitleScreen::Update()
{
	if (frame == actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case A_IDLE:
			break;
		case A_CHANGE_DOWN:
			action = A_IDLE;
			break;
		case A_CHANGE_UP:
			action = A_IDLE;
			break;
		}
	}


	if (!isMusicStarted)
	{
		mainMenu->musicPlayer->PlayMusic(titleMusic);
		isMusicStarted = true;
	}

	panel->MouseUpdate();

	quantX += xRate;
	quantY += yRate;

	scrollShader.setUniform("quantX", quantX);
	scrollShader.setUniform("quantY", quantY);

	

	if (action == A_IDLE)
	{
		int res = saSelector->UpdateIndex(CONTROLLERS.DirPressed_Up(), CONTROLLERS.DirPressed_Down());

		if (res > 0)
		{
			action = A_CHANGE_UP;
			frame = 0;
		}
		else if (res < 0)
		{
			action = A_CHANGE_DOWN;
			frame = 0;
		}
	}

	if (action == A_IDLE)
	{
		int realIndex = 0;
		for (int i = 0; i < NUM_BUTTONS; ++i)
		{
			realIndex = i + saSelector->currIndex;
			if (realIndex >= NUM_BUTTONS)
			{
				realIndex = realIndex - NUM_BUTTONS;
			}

			SetRectCenter(buttonQuads + realIndex * 4, baseButtonSize.x * buttonScales[i], baseButtonSize.y * buttonScales[i], buttonPositions[i]);
		}
	}
	else if (action == A_CHANGE_UP || action == A_CHANGE_DOWN)
	{
		float factor = frame / (float)actionLength[action];

		int realIndex = 0;
		Vector2f pos;
		float scale;

		Vector2f extraPos;
		float extraScale;

		int prevIndex = 0;

		for (int i = 0; i < NUM_BUTTONS; ++i)
		{
			realIndex = i + saSelector->currIndex;
			if (realIndex >= NUM_BUTTONS)
			{
				realIndex = realIndex - NUM_BUTTONS;
			}

			if (action == A_CHANGE_UP)
			{
				prevIndex = i + 1;
				if (prevIndex > NUM_BUTTONS - 1)
				{
					prevIndex = 0;
				}
			}
			else if (action == A_CHANGE_DOWN)
			{
				prevIndex = i - 1;
				if (prevIndex < 0)
				{
					prevIndex = NUM_BUTTONS - 1;
				}
				
			}

			if ((action == A_CHANGE_UP && prevIndex == 0) || ( action == A_CHANGE_DOWN && prevIndex == NUM_BUTTONS - 1 ))
			{
				//ClearRect(buttonQuads + realIndex * 4);
				if (action == A_CHANGE_UP)
				{
					pos = offScreenTopButtonPos * factor + buttonPositions[0] * (1.f - factor);
					scale = offScreenButtonScale * factor + buttonScales[0] * (1.f - factor);

					SetRectCenter(buttonQuads + realIndex * 4, baseButtonSize.x * scale, baseButtonSize.y * scale, pos);

					extraPos = buttonPositions[NUM_BUTTONS-1] * factor + offScreenBottomButtonPos * (1.f - factor);
					extraScale = buttonScales[NUM_BUTTONS - 1] * factor + offScreenButtonScale * (1.f - factor);

					SetRectCenter(extraButtonQuad, baseButtonSize.x * extraScale, baseButtonSize.y * extraScale, extraPos);
					SetRectSubRect(extraButtonQuad, ts_buttons->GetSubRect(realIndex * 2));
				}
				else if (action == A_CHANGE_DOWN)
				{
					pos = offScreenBottomButtonPos * factor + buttonPositions[NUM_BUTTONS - 1] * (1.f - factor);
					scale = offScreenButtonScale * factor + buttonScales[NUM_BUTTONS - 1] * (1.f - factor);

					SetRectCenter(buttonQuads + realIndex * 4, baseButtonSize.x * scale, baseButtonSize.y * scale, pos);

					extraPos = buttonPositions[0] * factor + offScreenTopButtonPos * (1.f - factor);
					extraScale = buttonScales[0] * factor + offScreenButtonScale * (1.f - factor);

					SetRectCenter(extraButtonQuad, baseButtonSize.x * extraScale, baseButtonSize.y * extraScale, extraPos);
					SetRectSubRect(extraButtonQuad, ts_buttons->GetSubRect(realIndex * 2));
				}
			}
			else
			{
				pos = buttonPositions[i] * factor + buttonPositions[prevIndex] * (1.f - factor);
				scale = buttonScales[i] * factor + buttonScales[prevIndex] * (1.f - factor);

				SetRectCenter(buttonQuads + realIndex * 4, baseButtonSize.x * scale, baseButtonSize.y * scale, pos);
			}
		}
	}

	if (action == A_CHANGE_UP || action == A_CHANGE_DOWN )
	{
	}


	/*if (res != 0)
	{
		soundNodeList->ActivateSound(soundManager.GetSound("main_menu_change"));
	}*/

	if (CONTROLLERS.ButtonPressed_A())
	{
		//currOptionPressed = true;
	}


	frame++;
}

void NewTitleScreen::ButtonCallback(Button *b, const std::string & e)
{
	if (b == feedbackButton)
	{
		//SteamFriends()->ActivateGameOverlayToWebPage(linkURL.c_str());
	}
}

void NewTitleScreen::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
	{
		//SteamFriends()->ActivateGameOverlayToWebPage("https://discord.gg/S7ePrzA");
		//cout << "clicked on discord" << endl;
	}
}

void NewTitleScreen::Draw(sf::RenderTarget *target)
{
	
	target->draw(bgQuad, 4, sf::Quads, &scrollShader);


	target->draw(buttonQuads, 4 * NUM_BUTTONS, sf::Quads, ts_buttons->texture);

	if (action == A_CHANGE_UP || action == A_CHANGE_DOWN)
	{
		target->draw(extraButtonQuad, 4, sf::Quads, ts_buttons->texture);
	}
	/*if (action == A_CHANGE_UP)
	{
		target->draw(buttonQuads + 4, 4 * 8, sf::Quads);
	}
	else if (action == A_CHANGE_DOWN)
	{
		target->draw(buttonQuads, 4 * 8, sf::Quads);
	}
	else
	{
		target->draw(buttonQuads, 4 + 4 * 7, sf::Quads);
	}*/


	//mainMenu->DrawMenuOptionText(target);

	//panel->Draw(target);

}
