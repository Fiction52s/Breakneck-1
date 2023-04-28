#include "FeedbackForm.h"
#include "MainMenu.h"
#include "AdventureManager.h"
#include "GameSession.h"
#include "FeedbackManager.h"
#include <sstream>
#include "steam\steam_api.h"

using namespace std;
using namespace sf;

FeedbackForm::FeedbackForm()
{
	Vector2i size(700, 420);//300 + 20);

	ts_star = MainMenu::GetInstance()->tilesetManager.GetSizedTileset("Menu/ratingstar_64x64.png");

	panel = new Panel("feedbackpanel", size.x, size.y, this, true);
	panel->SetCenterPos(Vector2i(960, 540));
	//panel->SetPosition(Vector2i(0, 0));//Vector2i(960 - panel->size.x / 2, 540 - panel->size.y / 2));

	int textBoxXBorder = 25;

	game = NULL;

	feedbackName = panel->AddLabel("feedbacktitle", Vector2i(textBoxXBorder, 10), 30, "");

	panel->ReserveImageRects(5);

	starRects.resize(5);

	for (int i = 0; i < 5; ++i)
	{
		starRects[i] = panel->AddImageRect(ChooseRect::I_FEEDBACK_RATING, Vector2f(textBoxXBorder + 64 * i, 60 ), ts_star, 0, 64);
		starRects[i]->Init();
		starRects[i]->SetShown(true);
	}
	
	//Vector2i textBoxSize(size.x - textBoxXBorder * 2, size.y - 100);
	Vector2i textBoxPos(textBoxXBorder, 140);
	//feedbackTextBox = panel->AddTextBox("feedbacktextbox", Vector2i(textBoxXBorder, 50), size.x - 2 * textBoxXBorder, 1000, "");
	feedbackTextBox = panel->AddTextBox("feedbacktextbox", textBoxPos, size.x - 2 * textBoxXBorder, 8, 20, 1000, "");

	Vector2i textBoxBottomLeft(textBoxPos.x, textBoxPos.y + feedbackTextBox->size.y);

	

	feedbackName->text.setString("Feedback");

	okButton = panel->AddButton("okbutton", textBoxBottomLeft + Vector2i(0, 20), Vector2f(150, 30), "Submit");
	cancelButton = panel->AddButton("cancelbutton", textBoxBottomLeft + Vector2i( 300, 20 ), Vector2f(200, 30), "Cancel");

	panel->SetConfirmButton(okButton);
	panel->SetCancelButton(cancelButton);

	SetRating(0);

	action = A_SHOW;
}

FeedbackForm::~FeedbackForm()
{
	delete panel;
}

void FeedbackForm::Activate( GameSession *p_game )
{
	game = p_game;

	MainMenu *mainMenu = MainMenu::GetInstance();

	if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
	{
		feedbackName->text.setString("Feedback for Level " + mainMenu->adventureManager->GetLeaderboardDisplayName(game));
	}

	//"level " + mainMenu->adventureManager->GetLeaderboardDisplayName(this));

	
	//MainMenu::GetInstance()->window->setKeyRepeatEnabled(true);
	action = A_SHOW;
	SetRating(0);
	feedbackTextBox->Clear();
	panel->SetFocusedMember(feedbackTextBox);
}

void FeedbackForm::ButtonCallback(Button *b,
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

void FeedbackForm::SetRating(int r)
{
	rating = r;
	for (int i = 0; i < 5; ++i)
	{
		if (i < r)
		{
			starRects[i]->SetImage(ts_star, 1);
		}
		else
		{
			starRects[i]->SetImage(ts_star, 0);
		}
	}
}

void FeedbackForm::ConfirmCallback(Panel *p)
{
	action = A_CONFIRM;
	bool res = Submit();

	if (game != NULL)
	{
		if (res)
		{
			game->StartAlertBox("Feedback successfully sent!");
		}
		else
		{
			game->StartAlertBox("Feedback failed to send.");
		}
		
	}
	//MainMenu::GetInstance()->window->setKeyRepeatEnabled(false);
}

void FeedbackForm::CancelCallback(Panel *p)
{
	action = A_CANCEL;
	panel->Deactivate();
	//MainMenu::GetInstance()->window->setKeyRepeatEnabled(false);
}

void FeedbackForm::Update()
{
	panel->MouseUpdate();
}

void FeedbackForm::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
	{
		for (int i = 0; i < 5; ++i)
		{
			if (starRects[i] == cr)
			{
				SetRating(i + 1);
				break;
			}
		}
	}
}

bool FeedbackForm::Submit()
{
	cout << "submit feedback form" << "\n";// (not actually submitting yet" << "\n";

	stringstream ss;

	ss << "Username: " << SteamFriends()->GetPersonaName() << "\n";
	ss << "SteamID: " << SteamUser()->GetSteamID().ConvertToUint64() << "\n";
	ss << "Rating: " << rating << "/5" << "\n";
	ss << "Feedback: \n\n" << feedbackTextBox->GetString();

	FeedbackManager::SetPayload("1-1-1", ss.str());
	return FeedbackManager::SubmitFeedback();
}

bool FeedbackForm::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void FeedbackForm::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}