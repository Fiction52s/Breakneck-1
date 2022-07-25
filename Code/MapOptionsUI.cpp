#include "GUI.h"
#include "EditSession.h"
#include "MusicSelector.h"

using namespace sf;
using namespace std;

MapOptionsUI::MapOptionsUI()
{
	edit = EditSession::GetSession();
	mapOptionsPanel = new Panel("mapoptions", 600, 600, this, true);

	mapOptionsPanel->SetPosition(Vector2i(960 - mapOptionsPanel->size.x / 2,
		540 - mapOptionsPanel->size.y / 2 - 300));

	mapOptionsPanel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	mapOptionsPanel->AddLabel("mapnamelabel", Vector2i(0, 0), 28, "Map Name: " + edit->filePath.stem().string());
	drainTextbox = mapOptionsPanel->AddLabeledTextBox("drain", Vector2i(0, 50), 200, 10, "", "Time to Drain (seconds): ");
	drainTextbox->SetNumbersOnly(true);
	bgButton = mapOptionsPanel->AddButton("bgbutton", Vector2i(0, 20), Vector2f(300, 30), "Set Environment");
	musicButton = mapOptionsPanel->AddButton("musicbutton", Vector2i(0, 20), Vector2f(300, 30), "Set Music");
	

	string fileName = "Resources/Editor/SpecialOptions/extrascene_options.txt";

	ifstream is;
	is.open(fileName);
	string s;

	std::vector<std::string> specialTypeOptions;

	if (is.is_open())
	{
		specialTypeOptions.reserve(10); //can bump this up later

		while (true)
		{
			is >> s;
			specialTypeOptions.push_back(s);
			if (is.eof())
			{
				break;
			}
		}
	}
	else
	{
		cout << "failed to open options file: " << fileName << endl;
		assert(0);
	}

	mapOptionsPanel->PauseAutoSpacing();
	mapOptionsPanel->AddLabel("prelabel", Vector2i(0, 20), 28, "Pre Level scene");
	mapOptionsPanel->UnpauseAutoSpacing();
	mapOptionsPanel->AddLabel("postlabel", Vector2i(300, 20), 28, "Post Level scene");
	mapOptionsPanel->PauseAutoSpacing();

	preDropdown = mapOptionsPanel->AddDropdown("pre", Vector2i(0, 0), Vector2i(250, 28), specialTypeOptions, 0);
	mapOptionsPanel->UnpauseAutoSpacing();
	postDropdown = mapOptionsPanel->AddDropdown("post", Vector2i(300, 0), Vector2i(250, 28), specialTypeOptions, 0);

	numPlayersSlider = mapOptionsPanel->AddLabeledSlider("numplayersslider", Vector2i(0, 40), "Num player spawns: ", 100, 1, 4, 1);

	okButton = mapOptionsPanel->AddButton("ok", Vector2i(0, 70), Vector2f(60, 30), "OK");
	mapOptionsPanel->SetConfirmButton(okButton);
	

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
	drainTextbox->SetString(to_string(edit->mapHeader->drainSeconds));
	edit->AddActivePanel(mapOptionsPanel);
	bool res = preDropdown->SetSelectedText(edit->mapHeader->preLevelSceneName);
	assert(res);
	res = postDropdown->SetSelectedText(edit->mapHeader->postLevelSceneName);
	assert(res);

	numPlayersSlider->SetCurrValue(edit->mapHeader->numPlayerSpawns);
}

void MapOptionsUI::CloseMapOptionsPopup()
{
	stringstream ss;
	ss << drainTextbox->GetString();
	int dSeconds;
	ss >> dSeconds;
	if (!ss.fail())
	{
		if (dSeconds > 0)
		{
			edit->mapHeader->drainSeconds = dSeconds;
		}
	}

	edit->mapHeader->preLevelSceneName = preDropdown->GetSelectedText();
	edit->mapHeader->postLevelSceneName = postDropdown->GetSelectedText();
	edit->SetNumPlayers(numPlayersSlider->GetCurrValue());
	//edit->SetGameMode(gameModeDropdown->selectedIndex);
	edit->RemoveActivePanel(mapOptionsPanel);
}

void MapOptionsUI::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::E_LEFTCLICKED ||
		eventType == ChooseRect::E_LEFTRELEASED)
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
		if (p == mapOptionsPanel)
		{
			CloseMapOptionsPopup();
		}
		else
		{
			edit->RemoveActivePanel(p);
		}
		
		/*if (p == bgOptionsPanel)
		{
			edit->RemoveActivePanel(p);
		}*/
	}
}

void MapOptionsUI::SliderCallback(Slider *slider)
{
	edit->SetNumPlayers(slider->GetCurrValue());
}