#include "ParallelPracticeSettingsMenu.h"
#include "Config.h"
#include "MainMenu.h"
#include "MusicPlayer.h"
#include "Session.h"

using namespace sf;
using namespace std;

ParallelPracticeSettingsMenu::ParallelPracticeSettingsMenu(MainMenu *mm)
{
	mainMenu = mm;

	panel = new Panel("ParallelPracticeSettingsMenu", 700, 700, this, true);
	//panel->SetColor(Color::Transparent);
	//panel->SetTop
	panel->SetCenterPos(Vector2i(960, 540));

	SetRectColor(bgQuad, Color(100, 100, 100));
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	Label *lab = panel->AddLabel("parallelplaylabel", Vector2i(10, 10), 50, "Parallel Play Settings");
	lab->text.setStyle(sf::Text::Underlined);


	panel->SetAutoSpacing(false, true, Vector2i(10, 90), Vector2i(0, 60));

	showKinsOnMinimapCheckBox = panel->AddLabeledCheckBox("showKinsOnMinimapCheckBox", Vector2i(0, 0), "Show Parallel Players on Minimap:");
	showKinsOnPauseMapCheckBox = panel->AddLabeledCheckBox("showKinsOnPauseMapCheckBox", Vector2i(0, 0), "Show Parallel Players on Pause Map:");
	showLobbyInCornerCheckBox = panel->AddLabeledCheckBox("showLobbyInCornerCheckBox", Vector2i(0, 0), "Show Parallel Player Lobby:");

	panel->SetAutoSpacing(false, true, Vector2i(30, 350), Vector2i(0, 60));

	defaultButton = panel->AddButton("defaultbutton", Vector2i(0, 0), Vector2f(400, 40), "Restore Defaults");
	applyButton = panel->AddButton("applybutton", Vector2i(0, 0), Vector2f(400, 40), "APPLY");
	backButton = panel->AddButton("backbutton", Vector2i(0, 0), Vector2f(400, 40), "BACK");
	
	panel->StopAutoSpacing();

	Start();
}

ParallelPracticeSettingsMenu::~ParallelPracticeSettingsMenu()
{
	delete panel;
}

void ParallelPracticeSettingsMenu::UpdateFromConfig()
{
	const ConfigData &cd = mainMenu->config->GetData();

	showKinsOnMinimapCheckBox->checked = cd.parallelPracticeShowKinsOnMinimap;
	showKinsOnPauseMapCheckBox->checked = cd.parallelPracticeShowKinsOnPauseMap;
	showLobbyInCornerCheckBox->checked = cd.parallelPracticeShowLobby;
}

void ParallelPracticeSettingsMenu::Start()
{
	SetAction(A_ACTIVE);
	UpdateFromConfig();
}

void ParallelPracticeSettingsMenu::Quit()
{
	SetAction(A_CANCEL);
}

bool ParallelPracticeSettingsMenu::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void ParallelPracticeSettingsMenu::DrawPopupBG(sf::RenderTarget *target)
{
	sf::RectangleShape rect;
	rect.setFillColor(Color(0, 0, 0, 100));
	rect.setSize(Vector2f(1920, 1080));
	rect.setPosition(0, 0);
	target->draw(rect);
}

void ParallelPracticeSettingsMenu::Update()
{
	panel->MouseUpdate();
}

void ParallelPracticeSettingsMenu::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}


void ParallelPracticeSettingsMenu::ConfirmCallback(Panel *p)
{
	SetAction(A_CONFIRM);

	const ConfigData &cd = mainMenu->config->GetData();

	ConfigData d = cd;
	d.parallelPracticeShowKinsOnMinimap = showKinsOnMinimapCheckBox->checked;
	d.parallelPracticeShowKinsOnPauseMap = showKinsOnPauseMapCheckBox->checked;
	d.parallelPracticeShowLobby = showLobbyInCornerCheckBox->checked;

	mainMenu->config->SetData(d);
	mainMenu->config->Save();
}

void ParallelPracticeSettingsMenu::CancelCallback(Panel *p)
{
	SetAction(A_CANCEL);
}

void ParallelPracticeSettingsMenu::SetAction(int a)
{
	action = a;
	frame = 0;
}

void ParallelPracticeSettingsMenu::ButtonCallback(Button *b,
	const std::string &e)
{
	if (b == defaultButton)
	{
		mainMenu->config->SetToDefault();
		UpdateFromConfig();
	}
	else if (b == applyButton)
	{
		ConfirmCallback(panel);
	}
	else if (b == backButton)
	{
		CancelCallback(panel);
	}
}