#include "MedalTimeForm.h"
#include "MainMenu.h"
#include "AdventureManager.h"
#include "GameSession.h"
#include "FeedbackManager.h"
#include <sstream>
#include "steam\steam_api.h"

using namespace std;
using namespace sf;

MedalTimeForm::MedalTimeForm()
{
	Vector2i size(700, 420);//300 + 20);

	panel = new Panel("medaltimepanel", size.x, size.y, this, true);
	panel->SetCenterPos(Vector2i(960, 540));
	//panel->SetPosition(Vector2i(0, 0));//Vector2i(960 - panel->size.x / 2, 540 - panel->size.y / 2));

	int textBoxXBorder = 25;

	game = NULL;

	levelNameLabel = panel->AddLabel("levelNamelabel", Vector2i(textBoxXBorder, 10), 30, "");

	//Vector2i textBoxSize(size.x - textBoxXBorder * 2, size.y - 100);
	Vector2i textBoxPos(textBoxXBorder, 140);

	goldTimeTextBox = panel->AddLabeledTextBox("gold", textBoxPos, true, 300, 1, 20, 10, "", "Gold Medal (seconds): ");
	goldTimeTextBox->SetNumbersOnly(true);

	silverTimeTextBox = panel->AddLabeledTextBox("silver", textBoxPos + Vector2i(0, 70), true, 300, 1, 20, 10, "", "Silver Medal (seconds): ");
	silverTimeTextBox->SetNumbersOnly(true);

	Vector2i textBoxBottomLeft(textBoxPos.x, silverTimeTextBox->pos.y + silverTimeTextBox->size.y);//textBoxPos.y + silverTimeTextBox->pos.y + silverTimeTextBox->size.y);

	levelNameLabel->text.setString("Level:");

	okButton = panel->AddButton("okbutton", textBoxBottomLeft + Vector2i(0, 20), Vector2f(150, 30), "Submit");
	cancelButton = panel->AddButton("cancelbutton", textBoxBottomLeft + Vector2i(300, 20), Vector2f(200, 30), "Cancel");

	panel->SetConfirmButton(okButton);
	panel->SetCancelButton(cancelButton);

	action = A_SHOW;
}

MedalTimeForm::~MedalTimeForm()
{
	delete panel;
}

void MedalTimeForm::Activate(GameSession *p_game)
{
	game = p_game;

	MainMenu *mainMenu = MainMenu::GetInstance();

	if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
	{
		levelName = mainMenu->adventureManager->GetLeaderboardDisplayName(game);
		levelNameLabel->text.setString("Level:" + levelName);
	}
	else if (game->IsRushSession())
	{
		levelName = game->mapHeader->fullName;
		levelNameLabel->text.setString("Level:" + levelName);
	}
	else
	{
		//not used yet
		levelName = game->filePath.stem().string();
	}

	previewPath = game->GetMapPreviewPath();

	//MainMenu::GetInstance()->window->setKeyRepeatEnabled(true);
	action = A_SHOW;

	goldTimeTextBox->SetString(to_string(game->mapHeader->goldSeconds));
	silverTimeTextBox->SetString(to_string(game->mapHeader->silverSeconds));

	panel->SetFocusedMember(goldTimeTextBox);
}

void MedalTimeForm::ButtonCallback(Button *b,
	const std::string &e)
{
	if (b == okButton)
	{
		ConfirmCallback(panel);
	}
	else if (b == cancelButton)
	{
		CancelCallback(panel);
	}
}

void MedalTimeForm::ConfirmCallback(Panel *p)
{
	if (goldTimeTextBox->GetString() == "" || silverTimeTextBox->GetString() == "")
	{
		return;
	}

	action = A_CONFIRM;
	bool res = Submit();

	if (game != NULL)
	{
		if (res)
		{
			game->StartAlertBox("Medal times successfully edited!");
		}
		else
		{
			game->StartAlertBox("Medal times failed to edit.");
		}

	}
	//MainMenu::GetInstance()->window->setKeyRepeatEnabled(false);
}

void MedalTimeForm::CancelCallback(Panel *p)
{
	if (goldTimeTextBox->GetString() == "" || silverTimeTextBox->GetString() == "")
	{
		return;
	}

	action = A_CANCEL;
	panel->Deactivate();
	//MainMenu::GetInstance()->window->setKeyRepeatEnabled(false);
}

void MedalTimeForm::Update()
{
	panel->MouseUpdate();
}

bool MedalTimeForm::Submit()
{
	stringstream ss;
	string goldTimeStr = goldTimeTextBox->GetString();
	string silverTimeStr = silverTimeTextBox->GetString();

	ss << goldTimeStr;
	int gt;
	ss >> gt;

	ss.clear();
	ss.str("");

		
	ss << silverTimeStr;
	int st;
	ss >> st;

	game->mapHeader->goldSeconds = gt;
	game->mapHeader->silverSeconds = st;

	return game->mapHeader->Replace(game->filePath);
}

bool MedalTimeForm::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void MedalTimeForm::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}