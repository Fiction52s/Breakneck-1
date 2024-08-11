#include "GUI.h"
#include "EditSession.h"
#include "MusicSelector.h"

using namespace sf;
using namespace std;

MapOptionsUI::MapOptionsUI()
{
	edit = EditSession::GetSession();

	mapOptionsPanel = new Panel("mapoptions", 1400, 1000, this, true);

	mapOptionsPanel->ReserveImageRects(3);

	mapOptionsPanel->SetCenterPos(Vector2i(960, 540));

	mapOptionsPanel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	//mapOptionsPanel->AddLabel("mapnamelabel", Vector2i(0, 0), 28, "Map Name: " + edit->filePath.stem().string());
	mapNameBox = mapOptionsPanel->AddLabeledTextBox("mapname", Vector2i(0, 30), false, 300, 1, 20, 30, "", "Map Name:");

	descriptionBox = mapOptionsPanel->AddLabeledTextBox("description", Vector2i(0, 30), false, 600, 5, 20, 1000, "", "Description:");

	drainTextBox = mapOptionsPanel->AddLabeledTextBox("drain", Vector2i(0, 20), true, 300, 1, 20, 10, "", "Time to Drain (seconds): ");
	drainTextBox->SetNumbersOnly(true);

	bgButton = mapOptionsPanel->AddButton("bgbutton", Vector2i(0, 20), Vector2f(300, 30), "Set Environment");
	musicButton = mapOptionsPanel->AddButton("musicbutton", Vector2i(0, 20), Vector2f(300, 30), "Set Music");


	std::vector<std::string> extraSceneOptions;
	edit->LoadSpecialOptions("extrascene_options", extraSceneOptions);

	std::vector<std::string> specialMapTypeOptions;
	edit->LoadSpecialOptions("maptype_options", specialMapTypeOptions);

	mapOptionsPanel->PauseAutoSpacing();
	mapOptionsPanel->AddLabel("prelabel", Vector2i(0, 20), 28, "Pre Level scene");
	mapOptionsPanel->UnpauseAutoSpacing();
	mapOptionsPanel->AddLabel("postlabel", Vector2i(300, 20), 28, "Post Level scene");
	mapOptionsPanel->PauseAutoSpacing();

	preDropdown = mapOptionsPanel->AddDropdown("pre", Vector2i(0, 0), Vector2i(250, 28), extraSceneOptions, 0);
	mapOptionsPanel->UnpauseAutoSpacing();
	postDropdown = mapOptionsPanel->AddDropdown("post", Vector2i(300, 0), Vector2i(250, 28), extraSceneOptions, 0);

	numPlayersSlider = mapOptionsPanel->AddLabeledSlider("numplayersslider", Vector2i(0, 40), "Num player spawns: ", 100, 1, 4, 1, 24);

	specialTypeDropdown = mapOptionsPanel->AddDropdown("special", Vector2i(0, 0), Vector2i(250, 28), specialMapTypeOptions, 0);


	okButton = mapOptionsPanel->AddButton("ok", Vector2i(0, 30), Vector2f(150, 100), "OK");
	mapOptionsPanel->SetConfirmButton(okButton);

	mapOptionsPanel->SetAutoSpacing(false, true, Vector2i(40 + descriptionBox->pos.x + descriptionBox->width, descriptionBox->pos.y), Vector2i(0, 30));

	goldTextBox = mapOptionsPanel->AddLabeledTextBox("gold", Vector2i(0, 20), true, 300, 1, 20, 10, "", "Gold Medal (seconds): ");
	goldTextBox->SetNumbersOnly(true);

	silverTextBox = mapOptionsPanel->AddLabeledTextBox("silver", Vector2i(-50, 140), true, 300, 1, 20, 10, "", "Silver Medal (seconds): ");
	silverTextBox->SetNumbersOnly(true);

	bronzeTextBox = mapOptionsPanel->AddLabeledTextBox("bronze", Vector2i(-50, 140), true, 300, 1, 20, 10, "", "Bronze Medal (seconds): ");
	bronzeTextBox->SetNumbersOnly(true);

	mapOptionsPanel->StopAutoSpacing();

	float extraSpacing = 0;//300;

	bgOptionsPanel = new Panel("bgoptions", 125 * 8, 125 * 8, this, true);

	/*GridSelector *bgSel = bgPopup->AddGridSelector(
	"terraintypes", Vector2i(20, 20), 6, 7, 1920 / 8, 1080 / 8, false, true);*/

	bgNameArr = new string[8 * 8];

	bgOptionsPanel->ReserveImageRects(8 * 8);

	bgOptionsPanel->SetPosition(Vector2i(960 - bgOptionsPanel->size.x / 2,
		540 - bgOptionsPanel->size.y / 2));


	Tileset *ts_mapBGThumbnails = edit->ts_mapBGThumbnails;
	string bgName;
	string numStr;
	string fullName;
	ImageChooseRect *icr;
	int index = 0;
	for (int w = 0; w < 8; ++w)
	{
		for (int i = 0; i < 8; ++i)
		{
			index = w * 8 + i;
			numStr = to_string(i + 1);
			bgName = "w" + to_string(w + 1) + "_0" + numStr;
			fullName = "Resources/Backgrounds/BGInfo/" + bgName + ".bg";

			if (boost::filesystem::exists(fullName))
			{
				bgNameArr[index] = bgName;
				icr = bgOptionsPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_BACKGROUNDLIBRARY,
					Vector2f(i * 125, w * 125), ts_mapBGThumbnails, index, 125);
				icr->Init();
				icr->SetShown(true);
				icr->SetInfo((void*)index);
			}
		}
	}
}

MapOptionsUI::~MapOptionsUI()
{
	delete[] bgNameArr;
	delete mapOptionsPanel;
	delete bgOptionsPanel;
}

void MapOptionsUI::OpenMapOptionsPopup()
{
	drainTextBox->SetString(to_string(edit->mapHeader->drainSeconds));
	goldTextBox->SetString(to_string(edit->mapHeader->goldSeconds));
	silverTextBox->SetString(to_string(edit->mapHeader->silverSeconds));
	bronzeTextBox->SetString(to_string(edit->mapHeader->bronzeSeconds));

	edit->AddActivePanel(mapOptionsPanel);
	bool res = preDropdown->SetSelectedText(edit->mapHeader->preLevelSceneName);
	assert(res);
	res = postDropdown->SetSelectedText(edit->mapHeader->postLevelSceneName);
	assert(res);
	specialTypeDropdown->SetSelectedIndex(edit->mapHeader->specialMapType);

	numPlayersSlider->SetCurrValue(edit->mapHeader->numPlayerSpawns);

	mapNameBox->SetString(edit->mapHeader->fullName);
	descriptionBox->SetString(edit->mapHeader->description);
}

void MapOptionsUI::CloseMapOptionsPopup()
{
	stringstream ss;
	ss << drainTextBox->GetString();
	int dSeconds;
	ss >> dSeconds;
	if (!ss.fail())
	{
		if (dSeconds > 0)
		{
			edit->mapHeader->drainSeconds = dSeconds;
		}
	}

	int gold, silver, bronze;

	ss.clear();
	ss << goldTextBox->GetString();
	ss >> gold;
	if (!ss.fail())
	{
		if (gold > 0)
		{
			edit->mapHeader->goldSeconds = gold;
		}
	}

	ss.clear();
	ss << silverTextBox->GetString();
	ss >> silver;
	if (!ss.fail())
	{
		if (silver > 0)
		{
			edit->mapHeader->silverSeconds = silver;
		}
	}

	ss.clear();
	ss << bronzeTextBox->GetString();
	ss >> bronze;
	if (!ss.fail())
	{
		if (bronze > 0)
		{
			edit->mapHeader->bronzeSeconds = bronze;
		}
	}

	edit->mapHeader->preLevelSceneName = preDropdown->GetSelectedText();
	edit->mapHeader->postLevelSceneName = postDropdown->GetSelectedText();
	edit->mapHeader->specialMapType = specialTypeDropdown->selectedIndex;
	edit->SetNumPlayers( false, numPlayersSlider->GetCurrValue());

	edit->mapHeader->fullName = mapNameBox->GetString();
	edit->mapHeader->description = descriptionBox->GetString();
	edit->RemoveActivePanel(mapOptionsPanel);

	edit->SetMode((EditSession::Emode)oldMode);
}

void MapOptionsUI::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::E_LEFTCLICKED)// ||
		//eventType == ChooseRect::E_LEFTRELEASED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_BACKGROUNDLIBRARY)
			{
				int ind = (int)icRect->info;
				string bgName = bgNameArr[ind];

				edit->SetBackground(bgName);

				edit->RemoveActivePanel(bgOptionsPanel);
			}
		}
	}
}

void MapOptionsUI::ButtonCallback(Button *b, const std::string & e)
{
	if (b == okButton)
	{
		CloseMapOptionsPopup();
	}
	else if (b == bgButton)
	{
		edit->AddActivePanel(bgOptionsPanel);
	}
	else if (b == musicButton)
	{
		edit->musicSelectorUI->OpenPopup();
	}
}



void MapOptionsUI::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup")
	{
		/*if (p == mapOptionsPanel)
		{
			CloseMapOptionsPopup();
		}
		else
		{
			edit->RemoveActivePanel(p);
		}*/
		
		/*if (p == bgOptionsPanel)
		{
			edit->RemoveActivePanel(p);
		}*/
	}
}

void MapOptionsUI::SliderCallback(Slider *slider)
{
	edit->SetNumPlayers(false, slider->GetCurrValue());
}