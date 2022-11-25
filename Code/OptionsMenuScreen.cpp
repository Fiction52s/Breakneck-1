#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"

using namespace sf;
using namespace std;

void OptionsMenuScreen::Center(Vector2f &windowSize)
{
	optionsWindow->SetTopLeftVec(Vector2f(windowSize.x / 2 - optionsWindow->dimensions.x / 2, windowSize.y / 2 - optionsWindow->dimensions.y / 2));
}

OptionsMenuScreen::OptionsMenuScreen(MainMenu *p_mainMenu)
	:mainMenu(p_mainMenu)
{
	int width = 1000;
	int height = 900;
	Vector2f menuOffset(0, 0);

	optionsWindow = new UIWindow(NULL, mainMenu->tilesetManager.GetTileset("Menu/windows_64x24.png", 64, 24),
		Vector2f(width, height));
	Center(Vector2f(1920, 1080));
	//optionsWindow->SetTopLeftVec(Vector2f(1920/2 - width / 2, 1080/2 - height / 2) + menuOffset);

	string options[] = { "1920 x 1080", "1600 x 900" , "1280 x 720" };
	string results[] = { "blah", "blah2" , "blah3" };
	string resolutionOptions[] = { "1920 x 1080", "1600 x 900", "1366 x 768", "1280 x 800", "1280 x 720" };
	string windowModes[] = { "Borderless Windowed", "Windowed", "Fullscreen" };

	int windowModeInts[] = { sf::Style::None, sf::Style::Default, sf::Style::Fullscreen };

	int controllerOptionInts[] = { ControllerType::CTYPE_XBOX, ControllerType::CTYPE_GAMECUBE,
		ControllerType::CTYPE_PS5,
	ControllerType::CTYPE_KEYBOARD };

	Vector2i resolutions[] = { Vector2i(1920, 1080), Vector2i(1600, 900), Vector2i(1366, 768),
		Vector2i(1280, 800), Vector2i(1280, 720) };

	horizResolution = new UIHorizSelector<Vector2i>(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, 4,
		resolutionOptions, "Resolution", 300, resolutions, true, 0, 400);

	horizWindowModes = new UIHorizSelector<int>(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, 3,
		windowModes, "Window Mode", 300, windowModeInts, true, 0, 500);

	string controllerOptions[] = { "XBOX", "Gamecube", "PS5", "Keyboard" };
	horizDefaultController = new UIHorizSelector<int>(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, 3,
		controllerOptions, "Default control type:", 600, controllerOptionInts, true, 0, 400);

	int vol[101];
	for (int i = 0; i < 101; ++i)
		vol[i] = i;

	string volStrings[101];
	for (int i = 0; i < 101; ++i)
		volStrings[i] = to_string(i);

	musicVolume = new UIHorizSelector<int>(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, 101,
		volStrings, "Music Volume", 400, vol, true, 0, 200);
	soundVolume = new UIHorizSelector<int>(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, 101,
		volStrings, "Sound Volume", 400, vol, true, 0, 200);

	defaultButton = new UIButton(NULL, this, &mainMenu->tilesetManager, &mainMenu->arial, "set to defaults", 300);
	applyButton = new UIButton(NULL, this, &mainMenu->tilesetManager, &mainMenu->arial, "apply settings", 300);

	checkForControllerButton = new UIButton(NULL, this, &mainMenu->tilesetManager, &mainMenu->arial, "Check for controllers", 300);

	//UICheckbox *check = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300);
	//test->SetTopLeft( Vector2f( 50, 0 ) );

	UIControl *testBlah[] = { horizResolution, horizWindowModes,
		musicVolume, soundVolume, horizDefaultController, defaultButton, applyButton, checkForControllerButton };

	//check->SetTopLeft(100, 50);
	UIVerticalControlList *cList = new UIVerticalControlList(optionsWindow, sizeof(testBlah) / sizeof(UIControl*), testBlah, 20);
	cList->SetTopLeft(50, 50);
	optionsWindow->controls.push_back(cList);
	//optionsWindow->controls.push_back(check);
}

OptionsMenuScreen::~OptionsMenuScreen()
{
	delete optionsWindow;
}

bool OptionsMenuScreen::ButtonEvent(UIEvent eType,
	ButtonEventParams *param)
{
	UIButton *pButton = param->button;
	if (eType == UIEvent::E_BUTTON_PRESSED)
	{
		if (pButton == defaultButton)
		{
			mainMenu->config->SetToDefault();
			Load();
		}
		else if (pButton == applyButton)
		{
			Vector2i res = horizResolution->GetResult(horizResolution->currIndex);
			int winMode = horizWindowModes->GetResult(horizWindowModes->currIndex);
			int mVol = musicVolume->GetResult(musicVolume->currIndex);
			int sVol = soundVolume->GetResult(soundVolume->currIndex);
			ConfigData d;
			d.resolutionX = res.x;
			d.resolutionY = res.y;
			d.windowStyle = winMode;
			d.musicVolume = mVol;
			d.soundVolume = sVol;
			d.defaultInputFormat = horizDefaultController->GetResult(horizDefaultController->currIndex);

			mainMenu->config->SetData(d);
			Config::CreateSaveThread(mainMenu->config);
			mainMenu->config->WaitForSave();


			mainMenu->musicPlayer->Update();
			mainMenu->musicPlayer->UpdateVolume();
			mainMenu->soundNodeList->SetSoundVolume(sVol);

			mainMenu->ResizeWindow(res.x, res.y, winMode);
			//mainMenu->config->
		}
		else if (pButton == checkForControllerButton)
		{
			CONTROLLERS.CheckForControllers();
		}
	
		
	}

	return true;
}

bool OptionsMenuScreen::CheckboxEvent(UIEvent eType,
	CheckboxEventParams *param)
{
	return false;
}


bool OptionsMenuScreen::SelectorEvent(UIEvent eType,
	SelectorEventParams *param)
{
	return false;
}

void OptionsMenuScreen::Load()
{
	const ConfigData &cd = mainMenu->config->GetData();

	bool hRes = horizResolution->SetCurrAsResult(Vector2i(cd.resolutionX, cd.resolutionY));
	bool hWinMode = horizWindowModes->SetCurrAsResult(cd.windowStyle);
	bool mvRes = musicVolume->SetCurrAsResult(cd.musicVolume);
	bool svRes = soundVolume->SetCurrAsResult(cd.soundVolume);
	bool cRes = horizDefaultController->SetCurrAsResult(cd.defaultInputFormat);
	assert(hRes);
	assert(hWinMode);
	assert(mvRes);
	assert(svRes);
	assert(cRes);
}

void OptionsMenuScreen::Update(ControllerState &currInput, ControllerState &prevInput)
{
	if (mainMenu->menuCurrInput.B && !mainMenu->menuPrevInput.B)
	{
		mainMenu->SetMode(MainMenu::Mode::TRANS_OPTIONS_TO_MAIN);
	}

	optionsWindow->Update(currInput, prevInput);
}

void OptionsMenuScreen::Draw(RenderTarget *target)
{
	optionsWindow->Draw(target);
}