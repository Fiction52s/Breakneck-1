#include "PauseMenu.h"
#include "PlayerBox.h"
#include "GameSettingsScreen.h"
#include "MainMenu.h"
#include "GameSession.h"
#include "AdventureManager.h"
#include "UIMouse.h"
#include "UIController.h"

using namespace std;
using namespace sf;

OptionsMenu::OptionsMenu(TilesetManager *p_tm)
{
	tm = p_tm;
	state = CHOOSESTATE;
	game = NULL;
	
	MainMenu *mm = MainMenu::GetInstance();

	gameSettingsMenu = mm->gameSettingsScreen;

	//temporary init here
	basePos = Vector2f(100, 100);

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	optionModeSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 4, 0);

	ts_optionMode = tm->GetSizedTileset("Menu/Pause/optionsptions_768x128.png");

	//Vector2f startOffset(1820 / 2, 100);
	Vector2f startOffset(1820 / 2, 128 / 2 + 150);
	int spacing = 30;
	for (int i = 0; i < 4; ++i)
	{

		SetRectCenter(optionModeQuads + i * 4, 768, 128, startOffset + Vector2f(0, (128 + spacing) * i));
	}

	playerBoxGroup = new PlayerBoxGroup(tm, 1, 450, 450, 100);
	playerBoxGroup->SetMode(PlayerBox::MODE_CONTROLLER_ONLY);
	playerBoxGroup->SetBoxCenter(0, startOffset + Vector2f(0, 128 + 150));
}

OptionsMenu::~OptionsMenu()
{
	delete optionModeSelector;

	delete playerBoxGroup;
}

void OptionsMenu::SetGame(GameSession *p_game)
{
	game = p_game;
	playerBoxGroup->SetControllerStates(0, game->controllerStates[0], game->GetPlayerNormalSkin(0));
	playerBoxGroup->SetControlProfile(0, game->controlProfiles[0]);
}

void OptionsMenu::Start()
{
	state = OptionsMenu::CHOOSESTATE;
}

void OptionsMenu::HandleEvent(sf::Event ev)
{
	if (state == GAMEPLAY)
	{
		gameSettingsMenu->HandleEvent(ev);
	}
}

void OptionsMenu::UpdateOptionModeQuads()
{
	for (int i = 0; i < 4; ++i)
	{
		if (optionModeSelector->currIndex == i)
		{
			SetRectSubRect(optionModeQuads + i * 4, ts_optionMode->GetSubRect(i + 4));
		}
		else
		{
			SetRectSubRect(optionModeQuads + i * 4, ts_optionMode->GetSubRect(i));
		}
	}
}

void OptionsMenu::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	

	switch (state)
	{
	case CHOOSESTATE:
	{
		int res = optionModeSelector->UpdateIndex(currInput.LUp() || currInput.PUp(), currInput.LDown() || currInput.PDown());
		if (res != 0)
		{
			game->pauseSoundNodeList->ActivateSound(game->soundManager->GetSound("pause_change"));
			//owner->pauseSoundNodeList->ActivateSound(owner->soundManager->GetSound("pause_change"));
		}
		UpdateOptionModeQuads();

		if (currInput.A && !prevInput.A)
		{
			state = (State)(optionModeSelector->currIndex + 1);


			switch (state)
			{
			case CONTROL:
				//playerBoxGroup->Update();
				//csm->UpdateXboxButtonIcons();
				break;
			case SOUND:
				//mainMenu->gameSettingsScreen->UpdateFromConfig();
				//break;
			case VISUAL:
				//state = SOUND;
				//mainMenu->gameSettingsScreen->UpdateFromConfig();
				//break;
			case GAMEPLAY:
		
				gameSettingsMenu->UpdateFromConfig();
				MOUSE.Show();
				MOUSE.SetControllersOn(true);
				MOUSE.Update(MOUSE.GetRealPixelPos());
				UICONTROLLER.Update();

				gameSettingsMenu->Start();
				break;
			}
		}
		break;
	}
	case CONTROL:
	{
		//playerBoxGroup->Update();
		//bool preUpdateChangingControls = //playerBoxGroup->IsChangingControls();

		if (playerBoxGroup->BackButtonPressed())
		{
			state = CHOOSESTATE;
			break;
		}

		playerBoxGroup->Update();

		

		if (playerBoxGroup->IsReady())
		{
			ControlProfile *cp = playerBoxGroup->GetControlProfile(0);
			if (cp != game->controlProfiles[0])
			{
				MainMenu *mm = MainMenu::GetInstance();
				if (mm->adventureManager != NULL)
				{
					mm->adventureManager->currProfile = cp;
				}
				game->controlProfiles[0] = cp;
			}
		}
		break;
	}
	case SOUND:
	{
		MOUSE.Update(MOUSE.GetRealPixelPos());// -Vector2i(50, 50)); //just testing
		UICONTROLLER.Update();

		gameSettingsMenu->Update();

		if (gameSettingsMenu->action == GameSettingsScreen::A_CANCEL)
		{
			MOUSE.Hide();
			state = CHOOSESTATE;
			break;
		}
		break;
	}
	case VISUAL:
	case GAMEPLAY:
	{
		MOUSE.Update(MOUSE.GetRealPixelPos());// -Vector2i(50, 50)); //just testing
		UICONTROLLER.Update();

		gameSettingsMenu->Update();

		if (gameSettingsMenu->action == GameSettingsScreen::A_CANCEL)
		{
			MOUSE.Hide();
			state = CHOOSESTATE;
			break;
		}

		break;
	}
	}
}

void OptionsMenu::Draw(sf::RenderTarget *target)
{
	switch (state)
	{
	case CHOOSESTATE:
		target->draw(optionModeQuads, 4 * 4, sf::Quads, ts_optionMode->texture);
		break;
	case CONTROL:
		playerBoxGroup->Draw(target);
		//csm->Draw(target);
		break;
	case SOUND:
	{
		gameSettingsMenu->Draw(target);
		break;
	}
		
	case VISUAL:
	case GAMEPLAY:
	{
		gameSettingsMenu->Draw(target);
		break;
	}
	}
}

bool OptionsMenu::CanChangeTab()
{
	if (state == CONTROL)
	{
		/*if (csm->IsEditingButtons())
		{
		return false;
		}*/
	}

	return true;
}