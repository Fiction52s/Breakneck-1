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
#include "RecordedAnimation.h"
#include "UIMouse.h"
#include "Physics.h"
#include "Movie.h"
#include "MovingGeo.h"

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
	
	currMovie = NULL;
	//string path = "Resources/Movie/";
	//string fileType = ".ogv";
	//string fullName = path + movieName + fileType;

	//assert(movies.count(movieName) == 0);

	movies.resize(NUM_BUTTONS);

	colorFadeTotalFrames = 30;

	movies[0] = new Movie("Stats/combat_tutorial_small", true);
	movies[1] = new Movie("Stats/speed_tutorial_small", true);
	movies[2] = new Movie("Stats/combat_tutorial_small", true);
	movies[3] = new Movie("Stats/speed_tutorial_small", true);
	movies[4] = new Movie("Stats/combat_tutorial_small", true);
	movies[5] = new Movie("Stats/speed_tutorial_small", true);
	movies[6] = new Movie("Stats/energy_tutorial_small", true);

	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		movies[i]->SetSize(Vector2f(750, 505));
		movies[i]->SetPosition(Vector2f(75, 46));
	}

	panel = new Panel("socials", 1920, 1080, this);
	panel->SetColor(Color::Transparent);

	panel->ReserveImageRects(4);

	int socialsSize = 128;

	ts_socials = GetSizedTileset("Menu/Title/socials_128x128.png");

	Vector2f discordPos = Vector2f(100, 100);
	ImageChooseRect *icrDiscord = panel->AddImageRect(ChooseRect::I_DISCORD_LINK, discordPos, ts_socials, 0, socialsSize);
	icrDiscord->Init();
	icrDiscord->SetShown(true);

	Vector2f instagramPos = Vector2f(200, 100);
	ImageChooseRect *icrInstagram = panel->AddImageRect(ChooseRect::I_INSTAGRAM_LINK, instagramPos, ts_socials, 1, socialsSize);
	icrInstagram->Init();
	icrInstagram->SetShown(true);

	Vector2f twitterPos = Vector2f(300, 100);
	ImageChooseRect *icrTwitter = panel->AddImageRect(ChooseRect::I_TWITTER_LINK, twitterPos, ts_socials, 2, socialsSize);
	icrTwitter->Init();
	icrTwitter->SetShown(true);

	Vector2f tiktokPos = Vector2f(400, 100);
	ImageChooseRect *icrTikTok = panel->AddImageRect(ChooseRect::I_TIKTOK_LINK, tiktokPos, ts_socials, 3, socialsSize);
	icrTikTok->Init();
	icrTikTok->SetShown(true);

	ts_backpack = GetSizedTileset("Menu/Title/back_371x774.png");//GetSizedTileset("Menu/Title/backpack_816x775.png");
		//backpack_816x775.png");//GetSizedTileset("Menu/Title/backpack_816x775.png");

	SetRectSubRect(backpackQuad, ts_backpack->GetSubRect(0));
	SetRectTopLeft( backpackQuad, ts_backpack->tileWidth, ts_backpack->tileHeight, Vector2f(1549, 191));

	ts_buttons = GetSizedTileset("Menu/Title/buttons_806x217.png");

	ts_bg = GetSizedTileset("Menu/Title/title_screen_bg_410x327.png");

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
	actionLength[A_CHANGE_DOWN] = 15;
	actionLength[A_CHANGE_UP] = 15;


	
	/*Blue: 0x4d89c2
	Green : 0x4ba233
	Yellow : 0xb4a53e
	Orange : 0xc67d3c
	Red : 0xa01515
	Pink : 0xaf6db0
	Purple : 0x8344c4*/


	
	
	


	tintColors[0] = Color(0x4d, 0x89, 0xc2);
	tintColors[1] = Color(0x4b, 0xa2, 0x33);
	tintColors[2] = Color(0xb4, 0xa5, 0x3e);
	tintColors[3] = Color(0xc5, 0x99, 0x5b);
	tintColors[4] = Color(0xc6, 0x6f, 0x6f);
	tintColors[5] = Color(0xaf, 0x6d, 0xb0);
	tintColors[6] = Color(0x83, 0x44, 0xc4);

	Reset();
}

NewTitleScreen::~NewTitleScreen()
{
	delete creditsMenuScreen;
	delete panel;
	delete saSelector;
	
	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		delete movies[i];
	}
}

void NewTitleScreen::Reset()
{
	action = A_IDLE;
	frame = 0;

	quantX = 0;
	quantY = 0;

	colorFadeFrame = -1;

	pressedIndex = -1;

	isCursorModeOn = false;

	if( currMovie != NULL )
		currMovie->Stop();

	saSelector->currIndex = 4;

	

	currMovie = movies[GetSelectedIndex()];
	currMovie->Play();
	
	isMusicStarted = false;
}

void NewTitleScreen::Update()
{
	pressedIndex = -1;

	if (frame == actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case A_IDLE:
			break;
		case A_CHANGE_DOWN:
			ClearRect(highlightQuads);
			ClearRect(highlightQuads + 4);
			action = A_IDLE;
			break;
		case A_CHANGE_UP:
			float factor = frame / (float)actionLength[action];
			ClearRect(highlightQuads);
			ClearRect(highlightQuads + 4);
			action = A_IDLE;
			break;
		}
	}

	currMovie->Update();

	if (!isMusicStarted)
	{
		mainMenu->musicPlayer->PlayMusic(titleMusic);
		isMusicStarted = true;
	}

	//panel->MouseUpdate();

	quantX += xRate;
	quantY += yRate;

	scrollShader.setUniform("quantX", quantX);
	scrollShader.setUniform("quantY", quantY);

	if (isCursorModeOn)
	{
		if (CONTROLLERS.DirPressed_Up() || CONTROLLERS.DirPressed_Down())
		{
			isCursorModeOn = false;
			MOUSE.Hide();
		}
	}
	else
	{
		if (mainMenu->oldMousePixelPos.x >= 0 && mainMenu->oldMousePixelPos.y >= 0 && mainMenu->mousePixelPos != mainMenu->oldMousePixelPos)
		{
			isCursorModeOn = true;
			MOUSE.Show();
		}
	}

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

		if (res != 0)
		{
			colorFadeFrame = 0;
			oldColor = currTint;
			currMovie->Stop();
			currMovie = movies[GetSelectedIndex()];
			currMovie->Play();
			mainMenu->ActivateSound("main_menu_change");
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

			if (i == NUM_BUTTONS / 2)
			{
				SetRectCenter(highlightQuads, baseButtonSize.x * buttonScales[i], baseButtonSize.y * buttonScales[i], buttonPositions[i]);
				SetRectSubRect(highlightQuads, ts_buttons->GetSubRect(realIndex * 2 + 1));
				SetRectColor(highlightQuads, Color(255, 255, 255, 255));
			}
			
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

				if (prevIndex == NUM_BUTTONS / 2)
				{
					SetRectCenter(highlightQuads, baseButtonSize.x * scale, baseButtonSize.y * scale, pos);
					SetRectSubRect(highlightQuads, ts_buttons->GetSubRect(realIndex * 2 + 1));
					SetRectColor(highlightQuads, Color(255, 255, 255, 255 * (1.f - factor)));
				}
				else if (i == NUM_BUTTONS / 2)
				{
					SetRectCenter(highlightQuads + 4, baseButtonSize.x * scale, baseButtonSize.y * scale, pos);
					SetRectSubRect(highlightQuads + 4, ts_buttons->GetSubRect(realIndex * 2 + 1));
					SetRectColor(highlightQuads + 4, Color(255, 255, 255, 255 * factor));
				}
			}
		}
	}

	if (action == A_CHANGE_UP || action == A_CHANGE_DOWN )
	{
	}


	//probably refine this later to capture the mouse etc zzz (old comment not sure of meaning)
	if (isCursorModeOn)
	{
		Vector2f mfPos = MOUSE.GetFloatPos();
		for (int i = 0; i < saSelector->totalItems; ++i)
		{
			if (QuadContainsPoint(buttonQuads + i * 4, mfPos))
			{
				//mainMenu->ActivateSound("main_menu_change");
				break;
			}
		}
	}

	if (isCursorModeOn)
	{
		int tempIndex = 0;
		Vector2f mfPos = MOUSE.GetFloatPos();

		for (int i = 0; i < saSelector->totalItems; ++i)
		{
			if (QuadContainsPoint(buttonQuads + i * 4, mfPos) && MOUSE.IsMouseLeftClicked() )
			{
				tempIndex = i;
				tempIndex += 4;//saSelector->currIndex; //arcade mode starts in the middle of the screen (currIndex starts the game at 4)
				if (tempIndex >= saSelector->totalItems)
				{
					tempIndex -= saSelector->totalItems;
				}
				pressedIndex = tempIndex;
				break;
			}
		}
	}
	else
	{
		if (CONTROLLERS.ButtonPressed_A())
		{
			pressedIndex = saSelector->currIndex;
			//currOptionPressed = true;
		}
	}

	UpdateBGColor();

	if (colorFadeFrame >= 0)
	{
		++colorFadeFrame;
		if (colorFadeFrame == colorFadeTotalFrames)
		{
			colorFadeFrame = -1;
		}
	}

	frame++;
}

void NewTitleScreen::UpdateBGColor()
{
	Color col;
	if (colorFadeFrame == -1)
	{
		col = tintColors[GetSelectedIndex()];
	}
	else
	{
		float factor = colorFadeFrame / (float)colorFadeTotalFrames;
		col = GetBlendColor(oldColor, tintColors[GetSelectedIndex()], factor);
	}

	currTint = col;

	scrollShader.setUniform("tintColor", ColorGL( col ) );
}

int NewTitleScreen::GetSelectedIndex()
{
	int ind = saSelector->currIndex;
	ind -= 4; //arcade mode starts in the middle of the screen (currIndex starts the game at 4)
	if (ind < 0)
	{
		ind += saSelector->totalItems;
	}

	return ind;
}

int NewTitleScreen::GetPressedIndex()
{
	if (pressedIndex < 0)
	{
		return -1;
	}

	int ind = pressedIndex;
	ind -= 4; //arcade mode starts in the middle of the screen (currIndex starts the game at 4)
	if (ind < 0)
	{
		ind += saSelector->totalItems;
	}

	return ind;
}

void NewTitleScreen::ButtonCallback(Button *b, const std::string & e)
{
	//if (b == feedbackButton)
	//{
	//	//SteamFriends()->ActivateGameOverlayToWebPage(linkURL.c_str());
	//}
}

void NewTitleScreen::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
	{
		if (mainMenu->steamOn)
		{
			if (cr->rectIdentity == ChooseRect::I_DISCORD_LINK)
			{
				SteamFriends()->ActivateGameOverlayToWebPage("https://discord.gg/S7ePrzA");
			}
			else if (cr->rectIdentity == ChooseRect::I_INSTAGRAM_LINK)
			{
				SteamFriends()->ActivateGameOverlayToWebPage("https://discord.gg/S7ePrzA");
			}
			else if (cr->rectIdentity == ChooseRect::I_TWITTER_LINK)
			{
				SteamFriends()->ActivateGameOverlayToWebPage("https://discord.gg/S7ePrzA");
			}
			else if (cr->rectIdentity == ChooseRect::I_TIKTOK_LINK)
			{
				SteamFriends()->ActivateGameOverlayToWebPage("https://discord.gg/S7ePrzA");
			}
		}
	}
}

void NewTitleScreen::Draw(sf::RenderTarget *target)
{
	
	target->draw(bgQuad, 4, sf::Quads, &scrollShader);


	if (action == A_CHANGE_UP || action == A_CHANGE_DOWN)
	{
		target->draw(extraButtonQuad, 4, sf::Quads, ts_buttons->texture);
	}

	target->draw(backpackQuad, 4, sf::Quads, ts_backpack->texture);

	target->draw(buttonQuads, 4 * NUM_BUTTONS, sf::Quads, ts_buttons->texture);

	target->draw(highlightQuads, 2 * 4, sf::Quads, ts_buttons->texture);

	currMovie->Draw(target);

	//panel->Draw(target);
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
