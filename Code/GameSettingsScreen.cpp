#include "GameSettingsScreen.h"
#include "Config.h"
#include "MainMenu.h"
#include "MusicPlayer.h"
#include "Session.h"
#include "NewTitleScreen.h"
#include "ItemSelector.h"

using namespace sf;
using namespace std;

SettingsModule::SettingsModule()
{
	MainMenu *mm = MainMenu::GetInstance();

	text.setFont(mm->arial);
	text.setCharacterSize(45);
	text.setFillColor(Color::White);
}

SettingsSlider::SettingsSlider(TilesetManager *tm, const std::string &name, int p_minValue, int p_maxValue, int p_defaultValue)
{
	minValue = p_minValue;
	maxValue = p_maxValue;
	defaultValue = p_defaultValue;

	int waitFrames[] = { 60, 30, 20 };
	int waitModeThresh[] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, maxValue - minValue, defaultValue - minValue, false );

	SetRectColor(underQuads, Color::Blue);
	SetRectColor(underQuads + 4, Color::Green);

	MainMenu *mm = MainMenu::GetInstance();
	valueText.setFont(mm->arial);
	valueText.setCharacterSize(45);
	valueText.setFillColor(Color::White);

	text.setString(name);

	ts_sliderBody = tm->GetSizedTileset("Menu/Options/volume_slider_1270x113.png");
	ts_marker = tm->GetSizedTileset("Menu/Options/arrow_38x50.png");
}

SettingsSlider::~SettingsSlider()
{
	delete saSelector;
}

void SettingsSlider::Reset()
{
	saSelector->currIndex = defaultValue;
	Update();
}

void SettingsSlider::SetTopLeft(sf::Vector2f p_pos)
{
	pos = p_pos;
	SetRectTopLeft(underQuads, 665, 67, pos + Vector2f( 390, 8 ));
	SetRectTopLeft(underQuads + 4, 665, 67, pos + Vector2f(390, 8));
	SetRectTopLeft(bodyQuad, ts_sliderBody->tileWidth, ts_sliderBody->tileHeight, pos);

	valueText.setPosition(pos + Vector2f(1179, 41));

	text.setPosition(pos + Vector2f( 40, 17));
}

void SettingsSlider::SetCenter(sf::Vector2f pos)
{
	
}

void SettingsSlider::CheckLeftRight()
{
	//int res = saSelector->UpdateIndex(CONTROLLERS.DirPressed_Left(), CONTROLLERS.DirPressed_Right());
	int res = saSelector->UpdateIndex(CONTROLLERS.DirPressed_Left(), CONTROLLERS.DirPressed_Right());

	if (res != 0)
	{
		if (res > 0)
		{
			saSelector->currIndex += 4;
			if (saSelector->currIndex > saSelector->totalItems)
			{
				saSelector->currIndex = saSelector->currIndex % saSelector->totalItems;
			}
		}
		else
		{
			saSelector->currIndex -= 4;
			if (saSelector->currIndex < 0)
			{
				saSelector->currIndex += saSelector->totalItems;
			}
		}
		
		Update();
	}
}

void SettingsSlider::SetValue(int val)
{
	saSelector->SetIndex(val);
}

int SettingsSlider::GetValue()
{
	return saSelector->currIndex;
}

void SettingsSlider::Update()
{
	float f = ((float)saSelector->currIndex) / maxValue;

	SetRectTopLeft(underQuads + 4, f * 665, 67, pos + Vector2f(390, 8));
	//SetRectTopLeft(underQuads + 4, f * 500, 50, pos);

	valueText.setString(to_string(saSelector->currIndex));
	//valueText.setOrigin(0, 0);
	valueText.setOrigin(valueText.getLocalBounds().left + valueText.getLocalBounds().width / 2,
		valueText.getLocalBounds().top + valueText.getLocalBounds().height / 2);
	valueText.setPosition(pos + Vector2f(1179, 41));

	if (selected)
	{
		SetRectSubRect(bodyQuad, ts_sliderBody->GetSubRect(1));
	}
	else
	{
		SetRectSubRect(bodyQuad, ts_sliderBody->GetSubRect(0));
	}
}

void SettingsSlider::Draw(sf::RenderTarget *target)
{
	target->draw(underQuads, 8, sf::Quads);
	target->draw(bodyQuad, 4, sf::Quads, ts_sliderBody->texture);

	target->draw(text);
	target->draw(valueText);
}


SettingsSelector::SettingsSelector(TilesetManager *tm, std::vector<std::string> &p_options)
{
	options = p_options;

	ts_switchBody = tm->GetSizedTileset("Menu/Options/switch_body_596x113.png");
	ts_arrow = tm->GetSizedTileset("Menu/Options/arrow_38x50.png");

	ts_arrow->SetQuadSubRect(arrowQuads, 0);
	ts_arrow->SetQuadSubRect(arrowQuads + 4, 0, true);

	int waitFrames[] = { 60, 30, 20 };
	int waitModeThresh[] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, options.size(), 0);
}

SettingsSelector::~SettingsSelector()
{
	delete saSelector;
}

void SettingsSelector::Reset()
{
	saSelector->SetIndex(0);
	Update();
}

void SettingsSelector::SetTopLeft( sf::Vector2f p_pos )
{
	pos = p_pos;
	SetRectTopLeft(selectorQuad, ts_switchBody->tileWidth, ts_switchBody->tileHeight, pos);
	text.setOrigin(0, 32);
	text.setPosition(pos);

	Vector2f arrowDelta(-50, 0);
	SetRectTopLeft(arrowQuads, ts_arrow->tileWidth, ts_arrow->tileHeight, pos + arrowDelta);
	SetRectTopLeft(arrowQuads + 4, ts_arrow->tileWidth, ts_arrow->tileHeight, pos + Vector2f( ts_switchBody->tileWidth, 0 ) + Vector2f(-arrowDelta.x, arrowDelta.y));
}

void SettingsSelector::SetCenter(sf::Vector2f p_pos)
{
	pos = p_pos;
	SetRectCenter(selectorQuad, ts_switchBody->tileWidth, ts_switchBody->tileHeight, pos + Vector2f( 0, 16 ));
	text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
	text.setPosition(pos);
	
	Vector2f tLeft = pos + Vector2f(-ts_switchBody->tileWidth / 2, -ts_switchBody->tileHeight / 2);
	Vector2f arrowDelta(-50, 32);
	SetRectTopLeft(arrowQuads, ts_arrow->tileWidth, ts_arrow->tileHeight, tLeft + arrowDelta);
	SetRectTopLeft(arrowQuads + 4, ts_arrow->tileWidth, ts_arrow->tileHeight, tLeft + Vector2f(ts_switchBody->tileWidth, 0) + Vector2f(-arrowDelta.x, arrowDelta.y));
}

void SettingsSelector::CheckLeftRight()
{
	int res = saSelector->UpdateIndex(CONTROLLERS.DirPressed_Left(), CONTROLLERS.DirPressed_Right());

	if (res != 0)
	{
		Update();
	}
}

int SettingsSelector::GetValue()
{
	return saSelector->currIndex;
}

void SettingsSelector::SetValue(int val)
{
	saSelector->SetIndex(val);
}

void SettingsSelector::Update()
{
	text.setString(options[saSelector->currIndex]);
	text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2, text.getLocalBounds().top + text.getLocalBounds().height / 2);
	text.setPosition(pos);

	if( selected )
	{ 
		SetRectSubRect(selectorQuad, ts_switchBody->GetSubRect(1));
	}
	else
	{
		SetRectSubRect(selectorQuad, ts_switchBody->GetSubRect(0));
	}
	
}

void SettingsSelector::Draw(sf::RenderTarget *target)
{
	target->draw(selectorQuad, 4, sf::Quads, ts_switchBody->texture);
	target->draw(text);
	if (selected)
	{
		target->draw(arrowQuads, 8, sf::Quads, ts_arrow->texture);
	}
}


SettingsSwitch::SettingsSwitch( TilesetManager *tm, const std::string &name, bool p_hasBody )
{
	ts_switchBody = tm->GetSizedTileset("Menu/Options/switch_body_596x113.png");
	ts_switchOnOff = tm->GetSizedTileset("Menu/Options/onoff_298x113.png");

	text.setString(name);

	hasBody = p_hasBody;

	Reset();
}

void SettingsSwitch::Reset()
{
	on = false;
	selected = false;
	Update();
}

void SettingsSwitch::SetTopLeft(sf::Vector2f p_pos)
{
	if (hasBody)
	{
		pos = p_pos;
		SetRectTopLeft(bodyQuad, ts_switchBody->tileWidth, ts_switchBody->tileHeight, pos);
		SetRectTopLeft(onOffQuad, ts_switchOnOff->tileWidth, ts_switchOnOff->tileHeight, pos + Vector2f(609, 0));
		text.setPosition(pos);
	}
	else
	{
		//TODO
	}
}

void SettingsSwitch::SetCenter(sf::Vector2f p_pos)
{
	if (hasBody)
	{
		//TODO
	}
	else
	{
		pos = p_pos;
		SetRectCenter(onOffQuad, ts_switchOnOff->tileWidth, ts_switchOnOff->tileHeight, pos);
		text.setPosition(pos);
	}
}

void SettingsSwitch::Update()
{
	int tileIndex = 0;
	//switch bodies
	int switchStartIndex = 2;

	if (selected)
	{
		tileIndex = 1;
	}
	else
	{
		tileIndex = 0;
	}
	SetRectSubRect(bodyQuad, ts_switchBody->GetSubRect(tileIndex));

	if (on)
	{
		tileIndex = 1;
	}
	else
	{
		tileIndex = 0;
	}
	if (selected)
	{
		tileIndex += 2;
	}
	SetRectSubRect(onOffQuad, ts_switchOnOff->GetSubRect(tileIndex));
}

void SettingsSwitch::Press()
{
	if (CONTROLLERS.ButtonPressed_A())
	{
		Swap();
	}
}

void SettingsSwitch::Swap()
{
	on = !on;
	Update();
}

void SettingsSwitch::Draw(sf::RenderTarget *target)
{
	if (hasBody)
	{
		target->draw(bodyQuad, 4, sf::Quads, ts_switchBody->texture);
		target->draw(text);
	}
	
	target->draw(onOffQuad, 4, sf::Quads, ts_switchOnOff->texture);
}

VideoSettingsTab::VideoSettingsTab()
{
	//panel = new Panel("screen", 1920, 1080, this, true);
	//panel->SetCenterPos(Vector2i(960, 540));

	mainMenu = MainMenu::GetInstance();

	displayText.setFont( mainMenu->arial );
	displayText.setCharacterSize(45);
	displayText.setFillColor(Color::White);
	displayText.setString("DISPLAY");
	displayText.setOrigin(displayText.getLocalBounds().left + displayText.getLocalBounds().width / 2,
		displayText.getLocalBounds().top + displayText.getLocalBounds().height / 2);
	displayText.setPosition(960, 323);

	resolutionText.setFont(mainMenu->arial);
	resolutionText.setCharacterSize(45);
	resolutionText.setFillColor(Color::White);
	resolutionText.setString("RESOLUTION");
	resolutionText.setOrigin(resolutionText.getLocalBounds().left + resolutionText.getLocalBounds().width / 2,
		resolutionText.getLocalBounds().top + resolutionText.getLocalBounds().height / 2);
	resolutionText.setPosition(960, 520);

	vSyncText.setFont(mainMenu->arial);
	vSyncText.setCharacterSize(45);
	vSyncText.setFillColor(Color::White);
	vSyncText.setString("V-SYNC");
	vSyncText.setOrigin(vSyncText.getLocalBounds().left + vSyncText.getLocalBounds().width / 2,
		vSyncText.getLocalBounds().top + vSyncText.getLocalBounds().height / 2);
	vSyncText.setPosition(960, 715);


	std::vector<string> resolutionOptions;
	
	auto &modes = sf::VideoMode::getFullscreenModes();
	resolutions.reserve(modes.size());
	for (auto it = modes.begin(); it != modes.end(); ++it)
	{
		resolutions.push_back(Vector2i((*it).width, (*it).height));
	}
	
	resolutionOptions.reserve(resolutions.size());
	for (auto it = resolutions.begin(); it != resolutions.end(); ++it)
	{
		resolutionOptions.push_back(ConfigData::GetResolutionString((*it).x, (*it).y));
	}

	windowModes.push_back(sf::Style::Fullscreen);
	windowModes.push_back(sf::Style::None);
	windowModes.push_back(sf::Style::Default);

	std::vector<string> windowModeOptions;
	windowModeOptions.reserve(windowModes.size());

	for (auto it = windowModes.begin(); it != windowModes.end(); ++it)
	{
		windowModeOptions.push_back(ConfigData::GetWindowModeString((*it)));
	}
	//kinda like the space in borderless window. easy to change later

	std::vector<std::string> windowStrs = { "Fullscreen", "Borderless Window", "Window" };
	//std::vector<std::string> resolutions = { "1920 x 1080", "1600 x 900" };
	

	/*switchPos[0] = Vector2f(504, 380);
	switchPos[1] = Vector2f(504, 503);
	switchPos[2] = Vector2f(504, 626);
	switchPos[3] = Vector2f(504, 749);*/


	modules.push_back(new SettingsSelector(mainMenu, windowStrs));
	modules[0]->SetCenter(Vector2f(960, 420));
	modules.push_back(new SettingsSelector(mainMenu, resolutionOptions));
	modules[1]->SetCenter(Vector2f(960, 617));
	modules.push_back(new SettingsSwitch(mainMenu, "V-Sync", false));
	modules[2]->SetCenter(Vector2f(960, 820));

	int waitFrames[] = { 60, 30, 20 };
	int waitModeThresh[] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 3, 0);
}

VideoSettingsTab::~VideoSettingsTab()
{
	delete saSelector;

	for (int i = 0; i < modules.size(); ++i)
	{
		delete modules[i];
	}
}

void VideoSettingsTab::Start()
{
	//saSelector->currIndex = 0;
	saSelector->Reset();

	for (int i = 0; i < modules.size(); ++i)
	{
		modules[i]->Reset();
		if (i == saSelector->currIndex)
		{
			modules[i]->selected = true;
		}
		else
		{
			modules[i]->selected = false;
		}
	}
}

void VideoSettingsTab::LoadFromConfig(const ConfigData &cd)
{
	int foundWindowStyleIndex = -1;
	for (int i = 0; i < windowModes.size(); ++i)
	{
		if (cd.windowStyle == windowModes[i])
		{
			foundWindowStyleIndex = i;
			break;
		}
	}

	if (foundWindowStyleIndex >= 0)
	{
		modules[0]->SetValue(foundWindowStyleIndex);
	}
	else
	{
		modules[0]->SetValue(0);
	}
	

	int foundResolutionIndex = -1;
	for (int i = 0; i < resolutions.size(); ++i)
	{
		if (cd.resolutionX == resolutions[i].x && cd.resolutionY == resolutions[i].y)
		{
			foundResolutionIndex = i;
			break;
		}
	}

	if (foundResolutionIndex >= 0)
	{
		modules[1]->SetValue(foundResolutionIndex);
	}
	else
	{
		modules[1]->SetValue(0);
	}

	for (int i = 0; i < modules.size(); ++i)
	{
		if (i == saSelector->currIndex)
		{
			modules[i]->selected = true;
		}
		else
		{
			modules[i]->selected = false;
		}

		modules[i]->Update();
	}
}

void VideoSettingsTab::UpdateConfig(ConfigData &cd)
{
	cd.windowStyle = windowModes[modules[0]->GetValue()];
	cd.resolutionX = resolutions[modules[1]->GetValue()].x;
	cd.resolutionY = resolutions[modules[1]->GetValue()].y;
}

void VideoSettingsTab::Update()
{
	int res = saSelector->UpdateIndex(CONTROLLERS.DirPressed_Up(), CONTROLLERS.DirPressed_Down());

	if (res != 0)
	{
		for (int i = 0; i < modules.size(); ++i)
		{
			if (i == saSelector->currIndex)
			{
				modules[i]->selected = true;
			}
			else
			{
				modules[i]->selected = false;
			}
			
			modules[i]->Update();
		}
		//mainMenu->ActivateSound("main_menu_change");
	}
	else
	{
		modules[saSelector->currIndex]->CheckLeftRight();
	}

	modules[saSelector->currIndex]->Press();
}

void VideoSettingsTab::Draw(sf::RenderTarget *target)
{
	target->draw(displayText);
	target->draw(resolutionText);
	target->draw(vSyncText);

	for (int i = 0; i < modules.size(); ++i)
	{
		modules[i]->Draw(target);
	}
}

AudioSettingsTab::AudioSettingsTab()
{
	//panel = new Panel("screen", 1920, 1080, this, true);
	//panel->SetCenterPos(Vector2i(960, 540));

	mainMenu = MainMenu::GetInstance();

	volumeText.setFont(mainMenu->arial);
	volumeText.setCharacterSize(45);
	volumeText.setFillColor(Color::White);
	volumeText.setString("VOLUME");
	volumeText.setOrigin(volumeText.getLocalBounds().left + volumeText.getLocalBounds().width / 2,
		volumeText.getLocalBounds().top + volumeText.getLocalBounds().height / 2);
	volumeText.setPosition(960, 323);

	modules.push_back(new SettingsSlider(mainMenu, "Master", 0, 100, 50 ));
	modules[0]->SetTopLeft(Vector2f(325, 380));
	modules.push_back(new SettingsSlider(mainMenu, "Music", 0, 100, 50));
	modules[1]->SetTopLeft(Vector2f(325, 503));
	modules.push_back(new SettingsSlider(mainMenu, "Sounds", 0, 100, 50));
	modules[2]->SetTopLeft(Vector2f(325, 625));
	modules.push_back(new SettingsSlider(mainMenu, "Rumble", 0, 100, 50));
	modules[3]->SetTopLeft(Vector2f(325, 800));

	int waitFrames[] = { 60, 30, 20 };
	int waitModeThresh[] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, modules.size(), 0);
}

AudioSettingsTab::~AudioSettingsTab()
{
	delete saSelector;

	for (int i = 0; i < modules.size(); ++i)
	{
		delete modules[i];
	}
}

void AudioSettingsTab::Start()
{
	//saSelector->currIndex = 0;
	saSelector->Reset();

	for (int i = 0; i < modules.size(); ++i)
	{
		modules[i]->Reset();
		if (i == saSelector->currIndex)
		{
			modules[i]->selected = true;
		}
		else
		{
			modules[i]->selected = false;
		}
	}
}

void AudioSettingsTab::LoadFromConfig(const ConfigData &cd)
{
	modules[1]->SetValue(cd.musicVolume);
	modules[2]->SetValue(cd.soundVolume);
	modules[3]->SetValue(cd.rumbleFactor);

	for (int i = 0; i < modules.size(); ++i)
	{
		if (i == saSelector->currIndex)
		{
			modules[i]->selected = true;
		}
		else
		{
			modules[i]->selected = false;
		}

		modules[i]->Update();
	}
}

void AudioSettingsTab::UpdateConfig(ConfigData &cd)
{
	cd.musicVolume = modules[1]->GetValue();
	cd.soundVolume = modules[2]->GetValue();
	cd.rumbleFactor = modules[3]->GetValue();
}

void AudioSettingsTab::Update()
{
	int res = saSelector->UpdateIndex(CONTROLLERS.DirPressed_Up(), CONTROLLERS.DirPressed_Down());

	if (res != 0)
	{
		for (int i = 0; i < modules.size(); ++i)
		{
			if (i == saSelector->currIndex)
			{
				modules[i]->selected = true;
			}
			else
			{
				modules[i]->selected = false;
			}

			modules[i]->Update();
		}
		//mainMenu->ActivateSound("main_menu_change");
	}
	else
	{
		modules[saSelector->currIndex]->CheckLeftRight();
	}


	modules[saSelector->currIndex]->Press();
}

void AudioSettingsTab::Draw(sf::RenderTarget *target)
{
	target->draw(volumeText);

	for (int i = 0; i < modules.size(); ++i)
	{
		modules[i]->Draw(target);
	}
}



GameSettingsTab::GameSettingsTab()
{
	//panel = new Panel("screen", 1920, 1080, this, true);
	//panel->SetCenterPos(Vector2i(960, 540));

	mainMenu = MainMenu::GetInstance();

	Vector2f switchPos[NUM_SWITCHES];
	switchPos[0] = Vector2f(504, 380);
	switchPos[1] = Vector2f(504, 503);
	switchPos[2] = Vector2f(504, 626);
	switchPos[3] = Vector2f(504, 749);

	std::string switchNames[NUM_SWITCHES];
	switchNames[0] = "";
	switchNames[1] = "";
	switchNames[2] = "";
	switchNames[3] = "";

	for (int i = 0; i < NUM_SWITCHES; ++i)
	{
		switches[i] = new SettingsSwitch(mainMenu, switchNames[i]);
		switches[i]->SetTopLeft(switchPos[i]);
	}

	int waitFrames[] = { 60, 30, 20 };
	int waitModeThresh[] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, NUM_SWITCHES, 0);
}

GameSettingsTab::~GameSettingsTab()
{
	delete saSelector;

	for (int i = 0; i < NUM_SWITCHES; ++i)
	{
		delete switches[i];
	}
}

void GameSettingsTab::Start()
{
	//saSelector->currIndex = 0;
	saSelector->Reset();

	for (int i = 0; i < NUM_SWITCHES; ++i)
	{
		switches[i]->Reset();
	}

	UpdateSwitches();
}

void GameSettingsTab::LoadFromConfig(const ConfigData &cd)
{

}

void GameSettingsTab::UpdateConfig(ConfigData &cd)
{

}

void GameSettingsTab::Update()
{
	int res = saSelector->UpdateIndex(CONTROLLERS.DirPressed_Up(), CONTROLLERS.DirPressed_Down());

	if (res != 0)
	{
		UpdateSwitches();
		//mainMenu->ActivateSound("main_menu_change");
	}

	if (CONTROLLERS.ButtonPressed_A())
	{
		switches[saSelector->currIndex]->Swap();
		UpdateSwitches();
		//currOptionPressed = true;
	}
	
}

void GameSettingsTab::UpdateSwitches()
{
	int tileIndex = 0;
	//switch bodies
	for (int i = 0; i < NUM_SWITCHES; ++i)
	{
		if (saSelector->currIndex == i)
		{
			switches[i]->selected = true;
		}
		else
		{
			switches[i]->selected = false;
		}

		switches[i]->Update();
	}
}

void GameSettingsTab::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < NUM_SWITCHES; ++i)
	{
		switches[i]->Draw(target);
	}
}


GameSettingsScreen::GameSettingsScreen(MainMenu *mm)
{
	mainMenu = mm;

	tabs[0] = new GameSettingsTab;
	tabs[1] = new VideoSettingsTab;
	tabs[2] = new AudioSettingsTab;


	ts_frame = GetSizedTileset("Menu/Options/options_frame_1920x1080.png");
	SetRectSubRect(frameQuad, ts_frame->GetSubRect(0));
	SetRectTopLeft(frameQuad, 1920, 1080, Vector2f(0, 0));

	Start();
}

GameSettingsScreen::~GameSettingsScreen()
{
	for (int i = 0; i < NUM_TABS; ++i)
	{
		delete tabs[i];
	}
}

void GameSettingsScreen::UpdateFromConfig()
{
	const ConfigData &cd = mainMenu->config->GetData();

	tabs[0]->LoadFromConfig(cd);
	tabs[1]->LoadFromConfig(cd);
	tabs[2]->LoadFromConfig(cd);
}

void GameSettingsScreen::Start()
{
	currTab = 0;
	for (int i = 0; i < NUM_TABS; ++i)
	{
		tabs[i]->Start();
	}
	SetAction(A_ACTIVE);
	UpdateFromConfig();
	nts = mainMenu->newTitleScreen;
}

void GameSettingsScreen::Quit()
{
	SetAction(A_CANCEL);
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
	nts->quantX += nts->xRate;
	nts->quantY += nts->yRate;

	nts->scrollShader.setUniform("quantX", nts->quantX);
	nts->scrollShader.setUniform("quantY", nts->quantY);

	if (CONTROLLERS.ButtonPressed_RightShoulder())
	{
		currTab++;
		if (currTab == NUM_TABS)
		{
			currTab = 0;
		}
	}
	else if (CONTROLLERS.ButtonPressed_LeftShoulder())
	{
		currTab--;
		if (currTab == -1)
		{
			currTab = NUM_TABS - 1;
		}
	}

	tabs[currTab]->Update();

	if (CONTROLLERS.ButtonPressed_X())
	{
		SaveSettingsAndApply();
	}
	else if (CONTROLLERS.ButtonPressed_B())
	{
		Quit();
	}
}

void GameSettingsScreen::Draw(sf::RenderTarget *target)
{
	target->draw(mainMenu->newTitleScreen->bgQuad, 4, sf::Quads, &nts->scrollShader);

	target->draw(frameQuad, 4, sf::Quads, ts_frame->texture);

	tabs[currTab]->Draw(target);
}


void GameSettingsScreen::SaveSettingsAndApply()
{
	ConfigData d;
	d.SetToDefault();

	for (int i = 0; i < 3; ++i)
	{
		tabs[i]->UpdateConfig(d);
	}

	//tabs[2]->UpdateConfig(d);
	

	mainMenu->musicPlayer->Update();
	mainMenu->musicPlayer->UpdateVolume();
	mainMenu->soundNodeList->SetSoundVolume(d.soundVolume);

	Session *sess = Session::GetSession();
	if (sess != NULL)
	{
		if (sess->soundNodeList != NULL)
		{
			sess->soundNodeList->SetSoundVolume(d.soundVolume);
		}
		if (sess->pauseSoundNodeList != NULL)
		{
			sess->pauseSoundNodeList->SetSoundVolume(d.soundVolume);
		}

		sess->runningTimerDisplay.showRunningTimer = d.showRunningTimer;
		sess->frameRateDisplay.showFrameRate = d.showFPS;
	}

	bool windowNeedsReset = false;

	const ConfigData &currData = mainMenu->config->GetData();

	if (d.resolutionX != currData.resolutionX
		|| d.resolutionY != currData.resolutionY
		|| d.windowStyle != currData.windowStyle)
	{
		windowNeedsReset = true;
	}

	if (windowNeedsReset)
	{
		mainMenu->SetupWindow();
	}

	mainMenu->config->SetData(d);
	mainMenu->config->Save();
}

void GameSettingsScreen::SetAction(int a)
{
	action = a;
	frame = 0;
}