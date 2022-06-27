#include "MainMenu.h"
#include "GameSession.h"
#include "ControlProfile.h"

using namespace sf;
using namespace std;

MultiLoadingScreen::MultiLoadingScreen(MainMenu *p_mainMenu)
	:mainMenu(p_mainMenu), loadThread(NULL)
{
	int quarter = 1920 / 4;
	menuOffset = Vector2f(0, 0);
	for (int i = 0; i < 4; ++i)
	{
		Vector2f topMid = Vector2f(quarter * i + quarter / 2, 1080 - 400) + menuOffset;
		//playerSection[i] = new MultiSelectionSection( mainMenu, this, i, topMid);
	}

	progressDisplay = new LoadingMapProgressDisplay(mainMenu, menuOffset + Vector2f(480, 540));
	//progressDisplay->SetProgressString("hello here i am");
	//progressDisplay->SetProgressString("hello here i am", 2);

	gs = NULL;
	loadThread = NULL;
}

MultiLoadingScreen::~MultiLoadingScreen()
{
	delete progressDisplay;
}

void MultiLoadingScreen::Reset(boost::filesystem::path p_path)
{
	filePath = p_path;

	previewSprite.setTexture(*mainMenu->mapSelectionMenu->previewSprite.getTexture());
	previewSprite.setPosition(Vector2f(400, 0) + menuOffset);
	//SetPreview();

	if (gs != NULL)
	{
		delete gs;
	}
	else if (loadThread != NULL)
	{
		delete loadThread;
	}

	MatchParams mp;
	mp.mapPath = p_path.string();
	gs = new GameSession(&mp);

	gs->progressDisplay = progressDisplay;

	loadThread = new boost::thread(GameSession::sLoad, gs);

	for (int i = 0; i < 4; ++i)
	{
		playerSection[i]->active = false;
	}
}

//void MultiLoadingScreen::SetPreview()
//{
//	string previewFile = filePath.parent_path().relative_path().string() + string( "/" ) + filePath.stem().string() + string( "_preview_960x540.png" );
//	previewTex.loadFromFile( previewFile );
//	previewSprite.setTexture( previewTex );
//	previewSprite.setPosition( Vector2f( 480, 0 ) + menuOffset );
//}

void MultiLoadingScreen::Draw(sf::RenderTarget *target)
{
	target->draw(previewSprite);
	for (int i = 0; i < 4; ++i)
	{
		playerSection[i]->Draw(target);
	}
	progressDisplay->Draw(target);
}

int MultiLoadingScreen::GetNumActivePlayers()
{
	int activeCount = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (playerSection[i]->active)
		{
			activeCount++;
		}
	}

	return activeCount;
}

bool MultiLoadingScreen::AllPlayersReady()
{
	int readyCount = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (playerSection[i]->active)
		{
			if (playerSection[i]->IsReady())
			{
				readyCount++;
			}
			else
			{
				return false;
			}
		}
	}

	return readyCount > 0;
}

void MultiLoadingScreen::Update()
{
	progressDisplay->UpdateText();

	for (int i = 0; i < 4; ++i)
	{
		if (playerSection[i]->ShouldGoBack())
		{
			mainMenu->SetMode(MainMenu::Mode::TRANS_MULTIPREVIEW_TO_MAPSELECT);

			gs->SetContinueLoading(false);
			loadThread->join();
			delete loadThread;
			loadThread = NULL;
			delete gs;
			gs = NULL;
			return;
		}
		if (!playerSection[i]->active)
		{
			if (mainMenu->GetCurrInput(i).B && !mainMenu->GetPrevInput(i).B)
			{

			}
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		playerSection[i]->Update();
	}

	if (AllPlayersReady())
	{
		//loadThread->join();
		//boost::chrono::steady_clock::now()
		if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
		{
			for (int i = 0; i < 4; ++i)
			{
				mainMenu->GetController(i).SetFilter(playerSection[i]->profileSelect->currProfile->filter);
			}

			mainMenu->gameRunType = MainMenu::GRT_FREEPLAY;
			int res = gs->Run();


			XBoxButton filter[ControllerSettings::Count];
			SetFilterDefault(filter);

			for (int i = 0; i < 4; ++i)
			{
				mainMenu->GetController(i).SetFilter(filter);
			}

			delete loadThread;
			loadThread = NULL;
			delete gs;
			gs = NULL;

			View vv;
			vv.setCenter(960, 540);
			vv.setSize(1920, 1080);
			mainMenu->window->setView(vv);

			mainMenu->v.setCenter(mainMenu->leftCenter);
			mainMenu->v.setSize(Vector2f(1920, 1080));
			mainMenu->preScreenTexture->setView(mainMenu->v);

			mainMenu->SetMode(MainMenu::Mode::MAPSELECT);
			mainMenu->mapSelectionMenu->state = MapSelectionMenu::State::S_MAP_SELECTOR;
			mainMenu->v.setCenter(mainMenu->leftCenter);
			mainMenu->preScreenTexture->setView(mainMenu->v);
		}
	}
}