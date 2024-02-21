#include "TitleScreen.h"
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

TitleScreen::TitleScreen(MainMenu *p_mainMenu)
	:mainMenu( p_mainMenu )
{

	//testMenu = new PostPracticeMatchMenu;

	//testForm = new FeedbackForm;

	panel = new Panel("feedbackpanel", 1920, 1080, this);
	panel->SetColor(Color::Transparent);

	panel->ReserveImageRects(1);
	
	//feedbackButton = panel->AddButton("feedbackbutton", Vector2i(1500, 700), Vector2f(500, 50), "Submit Feedback");

	feedbackURL = "https://docs.google.com/forms/d/e/1FAIpQLSewgTgR6kh-0vX3_un4T_WqQkyGZLkH1jGaz1ysI8rPcZSBBQ/viewform?usp=sf_link";

	ts_kineticTitle = GetSizedTileset("Menu/Title/Kinetic_Title_1024x440.png");
	ts_titleBG = GetTileset("Menu/Title/title_base_1920x1080.png", 1920, 1080);

	titleBGSprite.setTexture(*ts_titleBG->texture);
	kineticTitleSprite.setTexture(*ts_kineticTitle->texture);

	kineticTitleSprite.setOrigin(kineticTitleSprite.getLocalBounds().width / 2, 0);
	kineticTitleSprite.setPosition(960, 0);

	background = new Background;

	ts_energy = GetTileset("Menu/Title/energy_1920x1080.png", 1920, 1080);
	energySpr.setTexture(*ts_energy->texture);

	ts_discord = GetSizedTileset("Menu/Title/kinetic_discord_256x256.png");//GetSizedTileset("Menu/Title/discord_256x256.png");

	ImageChooseRect *icr = panel->AddImageRect(ChooseRect::I_DISCORD_LINK, Vector2f(1920 - 300, 1080 - 300), ts_discord, 0, 256);
	icr->Init();
	icr->SetShown(true);

	energySpr.setColor(Color::Blue);

	ts_kin = GetTileset("Menu/Title/kintitle_128x256.png", 128, 256);
	kinSpr.setTexture(*ts_kin->texture);
	kinSpr.setTextureRect(ts_kin->GetSubRect(0));
	kinSpr.setPosition(913, 710);

	ts_kinHandEnergy = GetTileset("Menu/Title/kintitle_energy_128x256.png", 128, 256);
	kinHandEnergySpr.setTexture(*ts_kinHandEnergy->texture);
	kinHandEnergySpr.setTextureRect(ts_kinHandEnergy->GetSubRect(0));
	kinHandEnergySpr.setPosition(913, 710);

	scrollingBackgrounds.push_back(
		new ScrollingBackground(
			GetTileset("Menu/Title/titlecloud_1_1920x1080.png", 1920, 1080), 0, 1, 5 * 5));
	scrollingBackgrounds.push_back(
		new ScrollingBackground(
			GetTileset("Menu/Title/titlecloud_2_1920x1080.png", 1920, 1080), 0, 1, 10 * 5));

	frame = 0;

	creditsMenuScreen = new CreditsMenuScreen;

	Label *lab = panel->AddLabel("ealabel", Vector2i(960, 1080 - 30), 40, "Early Access ver. 1.0");
	auto lb = lab->text.getLocalBounds();
	lab->text.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);
	lab->text.setFillColor(Color::White);

	titleMusic = mainMenu->musicManager->songMap["w0_Title"];
	if (titleMusic != NULL)
	{
		titleMusic->Load();
	}
}

TitleScreen::~TitleScreen()
{
	delete panel;
	delete background;

	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		delete (*it);
	}

	delete creditsMenuScreen;
}

void TitleScreen::Reset()
{
	//testForm->Activate();
	//return;
	//testMenu->Reset();
	//return;

	background->Reset();
	frame = 0;
}

void TitleScreen::Update()
{
	//testForm->Update();
	//return;
	//testMenu->Update();
	//return;


	panel->MouseUpdate();

	if (frame == 0)
	{
		mainMenu->musicPlayer->PlayMusic(titleMusic);
	}
	

	if (frame == 60)
	{
		//mainMenu->fader->CrossFade(60, 60, Color::Red);
		
	}

	Vector2f empty(0, 0);
	background->Update(empty);

	for ( auto it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->Update(Vector2f( 960, 540 ));
	}

	Color c = energySpr.getColor();
	int energyBreathe = 360;
	int ff = frame % (energyBreathe);
	if (ff < energyBreathe / 2)
	{
		float factor = (float)ff / (energyBreathe / 2);
		c.a = 150 * factor;
		//c.a = std::max(20.f, (float)c.a);
	}
	else
	{
		float factor = 1.f - (float)(ff - energyBreathe / 2) / (energyBreathe / 2);
		c.a = 150 * factor;
		//c.a = std::max(20.f, (float)c.a);
	}

	if (ff % (energyBreathe * 2) == 0)
	{
		c = Color(rand() % 255, rand() % 255, rand() % 255, c.a );
	}

	energySpr.setColor(c);

	int kinMult = 5;
	int hairAnimFrames = 12;
	int waitFrames = 120;
	int totalFrames = hairAnimFrames * 6 * kinMult + waitFrames * kinMult;
	int kinFrame = (frame % totalFrames) / kinMult;
	if (kinFrame < hairAnimFrames * 6)
	{
		int wind = 30;
		if (kinFrame <= hairAnimFrames * 2)
		{
			wind = ((float)kinFrame / ( hairAnimFrames*2)) * wind;
		}
		else if (kinFrame < hairAnimFrames * 4)
		{
			
		}
		else
		{
			wind = wind - ((float)(kinFrame % ( hairAnimFrames*2))/ (hairAnimFrames*2)) * wind;
		}
		//cout << "wind: " << wind << endl;
		auto it = scrollingBackgrounds.begin();
		(*it)->scrollSpeedX = 5 * 5 + wind;
		++it;
		(*it)->scrollSpeedX = 10 * 5 + wind / 2;
		int realKinFrame = kinFrame % hairAnimFrames;
		kinSpr.setTextureRect(ts_kin->GetSubRect(realKinFrame));
	}
	else 
	{
		auto it = scrollingBackgrounds.begin();
		(*it)->scrollSpeedX = 5 * 5;
		++it;
		(*it)->scrollSpeedX = 10 * 5;
		kinSpr.setTextureRect(ts_kin->GetSubRect(0));
	}
	
	
	
	kinHandEnergySpr.setTextureRect(ts_kinHandEnergy->GetSubRect(kinFrame % 12));

	frame++;
}

void TitleScreen::ButtonCallback(Button *b, const std::string & e)
{
	if (b == feedbackButton )
	{
		//SteamFriends()->ActivateGameOverlayToWebPage(linkURL.c_str());
	}
}

void TitleScreen::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
	{
		SteamFriends()->ActivateGameOverlayToWebPage("https://discord.gg/S7ePrzA");
		//cout << "clicked on discord" << endl;
	}
}

void TitleScreen::Draw(sf::RenderTarget *target)
{
	//testForm->Draw(target);
	//return;
	//testMenu->Draw(target);
	//return;

	background->Draw(target);
	target->draw(titleBGSprite);

	target->draw(energySpr);

	for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->Draw(target);
	}

	target->draw(kinHandEnergySpr);
	target->draw(kinSpr);

	mainMenu->DrawMenuOptionText(target);

	target->draw(kineticTitleSprite);

	panel->Draw(target);
	
}