#include "GameSettingsScreen.h"
#include "Config.h"
#include "MainMenu.h"
#include "MusicPlayer.h"
#include "Session.h"

using namespace sf;
using namespace std;

GameSettingsScreen::GameSettingsScreen(MainMenu *mm)
{
	mainMenu = mm;

	panel = new Panel("gamesettingsscreen", 1000, 700, this, true);
	//panel->SetColor(Color::Transparent);
	//panel->SetTop
	panel->SetCenterPos(Vector2i(960, 540));

	SetRectColor(bgQuad, Color(100, 100, 100));
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 60));

	resolutionLabel = panel->AddLabel("resolutionlabel", Vector2i(0, 0), 30, "Resolution:");
	windowModeLabel = panel->AddLabel("windowmodelabel", Vector2i(0, 0), 30, "Window mode:");

	musicVolumeSlider = panel->AddLabeledSlider( "musicslider", Vector2i( 0, 0 ), "Music Volume:", 400, 0, 100, 100, 30 );
	soundVolumeSlider = panel->AddLabeledSlider("soundslider", Vector2i(0, 0), "Sound Volume:", 400, 0, 100, 100, 30);
	defaultButton = panel->AddButton("defaultbutton", Vector2i(0, 0), Vector2f(400, 40), "Restore Defaults");
	applyButton = panel->AddButton("applybutton", Vector2i(0, 0), Vector2f(400, 40), "APPLY");
	backButton = panel->AddButton("backbutton", Vector2i(0, 0), Vector2f(400, 40), "BACK");
	checkForControllerButton = NULL;
	//checkForControllerButton = panel->AddButton("checkforcontrollersbutton", Vector2i(0, 0), Vector2f(400, 40), "Check for controllers");

	panel->StopAutoSpacing();

	CreateResolutionDropdown();

	CreateWindowModeDropdown();


	Start();
}

GameSettingsScreen::~GameSettingsScreen()
{
	delete panel;
}

void GameSettingsScreen::CreateResolutionDropdown()
{
	assert(resolutions.empty());

	auto &modes = sf::VideoMode::getFullscreenModes();
	resolutions.reserve(modes.size());
	for (auto it = modes.begin(); it != modes.end(); ++it)
	{
		resolutions.push_back( Vector2i( (*it).width, (*it).height ) );
	}

	//resolutions.push_back(Vector2i(1920, 1080));
	//resolutions.push_back(Vector2i(1600, 900));
	//resolutions.push_back(Vector2i(1366, 768));
	//resolutions.push_back(Vector2i(1280, 800));
	//resolutions.push_back(Vector2i(1280, 720));

	std::vector<string> resolutionOptions;
	resolutionOptions.reserve(resolutions.size());
	for (auto it = resolutions.begin(); it != resolutions.end(); ++it)
	{
		resolutionOptions.push_back(ConfigData::GetResolutionString((*it).x, (*it).y));
	}

	resolutionDropdown = panel->AddDropdown("resolutiondropdown", resolutionLabel->GetTopRight() + Vector2i( 30, 0 ), Vector2i(400, 28), resolutionOptions, 0);
}

void GameSettingsScreen::CreateWindowModeDropdown()
{
	assert(windowModes.empty());

	windowModes.push_back(sf::Style::Fullscreen);
	windowModes.push_back(sf::Style::None);
	windowModes.push_back(sf::Style::Default);

	std::vector<string> windowModeOptions;
	windowModeOptions.reserve(windowModes.size());

	for (auto it = windowModes.begin(); it != windowModes.end(); ++it)
	{
		windowModeOptions.push_back(ConfigData::GetWindowModeString((*it)));
	}
	
	windowModeDropdown = panel->AddDropdown("windowmodedropdown", windowModeLabel->GetTopRight() + Vector2i(30, 0), Vector2i(400, 28), windowModeOptions, 0);
}

void GameSettingsScreen::UpdateFromConfig()
{
	const ConfigData &cd = mainMenu->config->GetData();

	resolutionDropdown->SetSelectedText(ConfigData::GetResolutionString(cd.resolutionX, cd.resolutionY));
	windowModeDropdown->SetSelectedText(ConfigData::GetWindowModeString(cd.windowStyle));

	musicVolumeSlider->SetCurrValue(cd.musicVolume);
	soundVolumeSlider->SetCurrValue(cd.soundVolume);
}

void GameSettingsScreen::ApplyConfig()
{

}

void GameSettingsScreen::Start()
{
	SetAction(A_ACTIVE);
	UpdateFromConfig();
}

void GameSettingsScreen::Quit()
{
	SetAction(A_CANCEL);
}

bool GameSettingsScreen::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void GameSettingsScreen::DrawPopupBG(sf::RenderTarget *target)
{
	sf::RectangleShape rect;
	rect.setFillColor(Color(0, 0, 0, 100));
	rect.setSize(Vector2f(1920, 1080));
	rect.setPosition(0, 0);
	target->draw(rect);
}

void GameSettingsScreen::Update()
{
	panel->MouseUpdate();
}

void GameSettingsScreen::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}


void GameSettingsScreen::ConfirmCallback(Panel *p)
{
	SetAction(A_CONFIRM);

	Vector2i res(resolutions[resolutionDropdown->selectedIndex]);

	int winMode = windowModes[windowModeDropdown->selectedIndex];

	int mVol = musicVolumeSlider->GetCurrValue();
	int sVol = soundVolumeSlider->GetCurrValue();

	ConfigData d;
	d.resolutionX = res.x;
	d.resolutionY = res.y;
	d.windowStyle = winMode;
	d.musicVolume = mVol;
	d.soundVolume = sVol;

	bool windowNeedsReset = false;

	const ConfigData &currData = mainMenu->config->GetData();

	if (d.resolutionX != currData.resolutionX
		|| d.resolutionY != currData.resolutionY
		|| d.windowStyle != currData.windowStyle)
	{
		windowNeedsReset = true;
	}

	mainMenu->config->SetData(d);
	mainMenu->config->Save();
	//Config::CreateSaveThread(mainMenu->config);
	//mainMenu->config->WaitForSave();

	mainMenu->musicPlayer->Update();
	mainMenu->musicPlayer->UpdateVolume();
	mainMenu->soundNodeList->SetSoundVolume(sVol);

	Session *sess = Session::GetSession();
	if (sess != NULL)
	{
		if (sess->soundNodeList != NULL)
		{
			sess->soundNodeList->SetSoundVolume(sVol);
		}
		if (sess->pauseSoundNodeList != NULL)
		{
			sess->pauseSoundNodeList->SetSoundVolume(sVol);
		}
	}

	if (windowNeedsReset)
	{
		mainMenu->SetupWindow();
	}
	//mainMenu->ResizeWindow(res.x, res.y, winMode);
}

void GameSettingsScreen::CancelCallback(Panel *p)
{
	SetAction(A_CANCEL);
}

void GameSettingsScreen::SetAction(int a)
{
	action = a;
	frame = 0;
}

void GameSettingsScreen::ButtonCallback(Button *b,
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