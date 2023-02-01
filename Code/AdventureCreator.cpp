#include "AdventureCreator.h"
#include "VectorMath.h"
#include <iostream>
#include "EditSession.h"
#include "UIMouse.h"
#include "globals.h"

using namespace sf;
using namespace std;
using namespace boost::filesystem;

AdventureCreator::AdventureCreator()
	:MapBrowserHandler(3, 4, true, EXTRA_RECTS)
{
	adventure = new AdventureFile;

	musicListHandler = new MusicChooserHandler(10);

	SetRectSubRect(largePreview, FloatRect(0, 0, 912, 492));

	Panel *panel = chooser->panel;

	int rightSideStart = 610;

	SetRectTopLeft(largePreview, 912, 492, Vector2f(rightSideStart + 150, 550 ));

	Vector2f startWorldPos(rightSideStart, 90 - 20);
	Vector2f startSectorPos(rightSideStart, 210 - 20);
	Vector2f startMapPos(rightSideStart, 385 - 20);
	Vector2f startMusicPos(startMapPos.x, startMapPos.y + 130);

	//EditSession *edit = EditSession::GetSession();
	//assert(edit != NULL);

	Tileset *ts_worldChoosers = chooser->GetSizedTileset("Editor/worldselector_64x64.png");

	ts_sectorIcons = chooser->GetSizedTileset("Editor/sectoricons_64x64.png");

	currWorldRect = panel->AddImageRect(ChooseRect::I_ADVENTURECREATOR_WORLD_SEARCH,
		startWorldPos, ts_worldChoosers, 1, 100);
	currWorldRect->SetShown(true);
	currWorldRect->Init();

	for (int i = 0; i < 8; ++i)
	{
		worldRects[i] = panel->AddImageRect(ChooseRect::I_ADVENTURECREATOR_WORLD,
			startWorldPos + Vector2f((i+1) * 120, 0), ts_worldChoosers, i+1, 100);
		worldRects[i]->SetShown(false);
		worldRects[i]->Init();
		worldRects[i]->SetInfo((void*)(i+1));
	}

	for (int i = 0; i < 8; ++i)
	{
		sectorRects[i] = panel->AddImageRect(ChooseRect::I_ADVENTURECREATOR_SECTOR,
			startSectorPos + Vector2f((i + 1) * 120, 0), ts_sectorIcons, 0, 100);
		sectorRects[i]->SetShown(true);
		sectorRects[i]->Init();
		sectorRects[i]->SetInfo((void*)(i+1));
	}

	
	int mapSpacing = 160;//140;

	for (int i = 0; i < 8; ++i)
	{
		mapRects[i] = panel->AddImageRect(ChooseRect::I_ADVENTURECREATOR_MAP,
			startMapPos + Vector2f(i * mapSpacing, 0), NULL, 0, 120);
		mapRects[i]->SetShown(true);
		mapRects[i]->Init();
		//mapRects[i]->SetInfo((void*)(i+1));
	}

	panel->ReserveTextRects(8);
	for (int i = 0; i < 8; ++i)
	{
		musicRects[i] = panel->AddTextRect(ChooseRect::I_MUSICLEVEL,
			startMusicPos + Vector2f(i * mapSpacing, 0), Vector2f(120, 40), "w0_awhfoewahfehf");
		musicRects[i]->SetShown(true);
		musicRects[i]->Init();
		musicRects[i]->SetTextHeight(14);
	}

	Vector2i sectorLabelPos(rightSideStart, 340 - 20);

	sectorLabel = panel->AddLabel("sectorlabel", sectorLabelPos, 40, "Sector 1");

	//Vector2i sliderPos(rightSideStart + 300, 350 - 20);
	//sectorRequirementsSlider = panel->AddSlider("requirements", sliderPos, 200, 0, 7, 0);

	
}

AdventureCreator::~AdventureCreator()
{
	delete adventure;
	delete musicListHandler;
}

void AdventureCreator::Open()
{
	action = BROWSE;
	chooser->StartRelative(MAP_EXT, MapBrowser::Mode::EDITOR_OPEN, "Resources\\Maps");

	CollapseWorlds();
	currWorld = 1;
	ChooseSector(1);
}

void AdventureCreator::LoadAdventure(const std::string &path, 
	const std::string &adventureName )
{
	adventure->Load(path, adventureName);
	adventure->LoadMapHeaders();

	int ind;
	if (adventure->copyMode == AdventureFile::COPY)
	{
		string worldStr;
		string sectorStr;
		string file;
		
		string name;
		for (int w = 0; w < 8; ++w)
		{
			worldStr = path + "/W" + to_string(w + 1);
			for (int s = 0; s < 8; ++s)
			{
				sectorStr = worldStr + "/Sector" + to_string(s + 1) + "/";
				for (int m = 0; m < 8; ++m)
				{
					ind = w * 64 + s * 8 + m;
					if (adventure->worlds[w].sectors[s].maps[m].Exists())
					{
						name = adventure->worlds[w].sectors[s].maps[m].name;
						file = sectorStr + name;
						adventureNodes[ind].filePath = file + MAP_EXT;
						adventureNodes[ind].ts_preview = chooser->GetTileset(file + ".png");
						adventureNodes[ind].index = ind;
					}
				}
			}
		}
	}
	else if (adventure->copyMode == AdventureFile::PATH)
	{
		bool oldResourceMode = chooser->IsResourceMode();
		chooser->SetGameResourcesMode(true);
		AdventureMap *am;
		string filePath;
		string localDir = "BreakneckEmergence\\Resources\\";
		for (int w = 0; w < 8; ++w)
		{
			for (int s = 0; s < 8; ++s)
			{
				for (int m = 0; m < 8; ++m)
				{
					ind = w * 64 + s * 8 + m;
					am = &(adventure->worlds[w].sectors[s].maps[m]);
					if (am->Exists() )
					{
						filePath = am->path + "\\" + am->name;
						adventureNodes[ind].filePath = 
							localDir + filePath + MAP_EXT;
						adventureNodes[ind].ts_preview = 
							chooser->GetTileset(filePath + ".png");
						adventureNodes[ind].index = ind;
					}
				}
			}
		}
		chooser->SetGameResourcesMode(oldResourceMode);
	}
}

void AdventureCreator::SaveAdventure(const std::string &p_path, 
	const std::string &adventureName, AdventureFile::CopyMode copyMode )
{
	MapNode *currentNode;	
	
	if (copyMode == AdventureFile::COPY)
	{
		string pathStr = p_path + "/" + adventureName;
		path p(pathStr);

		adventure->Save(p.string(), adventureName, copyMode);

		if (boost::filesystem::exists(p))
		{
			if (boost::filesystem::is_directory(p))
			{
				//boost::filesystem::remove(p);
				cout << "directory already exists. cannot create adventure. overwrite?" << endl;
				return;
			}
		}

		boost::filesystem::create_directory(p);

		//populate adventure from filenodes
		string worldDirStr;
		string sectorStr;
		string fileName;
		
		string imagePath;
		for (int w = 0; w < 8; ++w)
		{
			worldDirStr = pathStr + "/W" + to_string(w + 1);
			boost::filesystem::create_directory(worldDirStr);

			for (int s = 0; s < 8; ++s)
			{
				sectorStr = worldDirStr + "/Sector" + to_string(s + 1);
				boost::filesystem::create_directory(sectorStr);

				for (int m = 0; m < 8; ++m)
				{
					currentNode = &(adventureNodes[w * 64 + s * 8 + m]);
					fileName = currentNode->filePath.stem().string();

					adventure->worlds[w].sectors[s].maps[m].name
						= currentNode->filePath.stem().string();
					if (currentNode->ts_preview != NULL)
					{
						boost::filesystem::copy_file(currentNode->filePath, sectorStr + "/" + fileName + MAP_EXT);

						imagePath = currentNode->filePath.parent_path().string() + "/" + fileName + ".png";
						boost::filesystem::copy_file(imagePath, sectorStr + "/" + fileName + ".png");
					}
				}
			}
		}

		adventure->Save(p.string(), adventureName, AdventureFile::COPY);
	}
	else if (copyMode == AdventureFile::PATH)
	{
		string findPath = "BreakneckEmergence\\Resources\\";
		string nodePath;
		string nodeName;

		for (int w = 0; w < 8; ++w)
		{
			for (int s = 0; s < 8; ++s)
			{
				for (int m = 0; m < 8; ++m)
				{
					currentNode = &(adventureNodes[w * 64 + s * 8 + m]);

					if (currentNode->ts_preview != NULL)
					{
						nodePath = currentNode->filePath.parent_path().string();
						nodeName = currentNode->filePath.stem().string();
						auto pathClip = nodePath.find(findPath);
						if (pathClip == string::npos)
						{
							cout << "cannot save adventure. bad path: " << nodePath << endl;
							assert(0);
						}
						else
						{
							adventure->worlds[w].sectors[s].maps[m].name = nodeName;
							adventure->worlds[w].sectors[s].maps[m].path =
								nodePath.substr(pathClip + findPath.length());
						}
					}
					else
					{
						adventure->worlds[w].sectors[s].maps[m].name = "";
						adventure->worlds[w].sectors[s].maps[m].path = "";
					}
				}
			}
		}

		adventure->Save(p_path, adventureName, AdventureFile::PATH);
	}
	
}

void AdventureCreator::Confirm()
{
	SaveAdventure("Adventure"/*chooser->currPath.string()*/, "tadventure", AdventureFile::PATH );
}

bool AdventureCreator::MouseUpdate()
{
	if (MOUSE.IsMouseLeftReleased())
	{
		action = BROWSE;
		grabbedFile = NULL;
	}

	if (action == DRAG)
	{
		SetRectCenter(grabbedFileQuad, grabbedFile->ts_preview->tileWidth / 8,
			grabbedFile->ts_preview->tileHeight / 8, Vector2f(chooser->panel->GetMousePos()));
	}
	return true;
}

void AdventureCreator::ClickFile(ChooseRect *cr)
{
	MapNode *mn = (MapNode*)cr->info;
	
	if (mn->ts_preview != NULL)
	{
		grabbedFile = mn;
		mn->ts_preview->SetQuadSubRect(grabbedFileQuad, 0);
		action = DRAG;
	}
}

//void AdventureCreator::FocusFile(ChooseRect *cr)
//{
//	ts_largePreview = cr->GetAsImageChooseRect()->ts;
//}


void AdventureCreator::LateDraw(sf::RenderTarget *target)
{
	if (action == DRAG)
	{
		target->draw(grabbedFileQuad, 4, sf::Quads, grabbedFile->ts_preview->texture);
	}
}

void AdventureCreator::ExpandWorlds()
{
	for (int i = 0; i < 8; ++i)
	{
		worldRects[i]->SetShown(true);
	}
	worldsExpanded = true;
}
void AdventureCreator::CollapseWorlds()
{
	for (int i = 0; i < 8; ++i)
	{
		worldRects[i]->SetShown(false);
	}
	worldsExpanded = false;
}

void AdventureCreator::SetRectNode(ChooseRect *cr, MapNode *fn)
{
	ImageChooseRect *icRect = cr->GetAsImageChooseRect();
	icRect->SetInfo(fn);

	icRect->SetImage(fn->ts_preview, 0);
	if (fn->ts_preview == NULL)
		icRect->SetShown(true);
	icRect->SetName(fn->filePath.stem().string());
	
	if (fn->index >= 0)
	{
		auto &mhi = adventure->GetMapHeaderInfo(fn->index);
		//if (mhi.mainSongName != "")
		//{
		int levelIndex = fn->index % 8;
		TextChooseRect *tcRect = musicRects[levelIndex];
		tcRect->SetText(mhi.mainSongName);
		//}
		//TextChooseRect *tcRect = fn-
	}
	else
	{
		
	}
	
}

void AdventureCreator::ChooseWorld(int w)
{
	currWorld = w;

	ChooseSector(1);

	//update sector and map info
}

int AdventureCreator::GetNodeStart()
{
	return (currWorld - 1) * 64 + (currSector - 1) * 8;
}

void AdventureCreator::ChooseSector(int s)
{
	currSector = s;

	int nodeStart = GetNodeStart();


	for (int i = 0; i < 8; ++i)
	{
		musicRects[i]->SetText("");

		SetRectNode(mapRects[i], &(adventureNodes[nodeStart + i]));
	}

	//sectorRequirementsSlider->SetCurrValue(
	//	adventure->worlds[currWorld - 1].sectors[currSector - 1].requiredRunes);

	sectorLabel->text.setString("Sector " + to_string(currSector));
	//update maps
}

MapNode * AdventureCreator::GetCurrNode(int m)
{
	return &(adventureNodes[GetNodeStart() + m]);
}

void AdventureCreator::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (cr->rectIdentity == ChooseRect::I_FILESELECT)
	{
		MapBrowserHandler::ChooseRectEvent(cr, eventType);
	}
	else if (cr->rectIdentity == ChooseRect::I_ADVENTURECREATOR_WORLD_SEARCH)
	{
		if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
		{
			if (worldsExpanded)
			{
				CollapseWorlds();
			}
			else
			{
				ExpandWorlds();
			}
		}
	}
	else if (cr->rectIdentity == ChooseRect::I_ADVENTURECREATOR_WORLD)
	{
		if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED
			|| eventType == ChooseRect::ChooseRectEventType::E_LEFTRELEASED)
		{
			ImageChooseRect *icRect = cr->GetAsImageChooseRect();
			currWorldRect->SetImage(icRect->ts, icRect->tileIndex);
			ChooseWorld((int)cr->info);
			CollapseWorlds();
		}
	}
	else if (cr->rectIdentity == ChooseRect::I_ADVENTURECREATOR_SECTOR)
	{
		if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED
			|| eventType == ChooseRect::ChooseRectEventType::E_LEFTRELEASED)
		{
			ChooseSector((int)cr->info);
		}
		else if (action == DRAG && eventType == ChooseRect::ChooseRectEventType::E_FOCUSED )
		{
			ChooseSector((int)cr->info);
		}
		
	}
	else if( cr->rectIdentity == ChooseRect::I_ADVENTURECREATOR_MAP )//map
	{
		if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED)
		{
			FocusFile( cr );
			//ts_largePreview = cr->GetAsImageChooseRect()->ts;
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_UNFOCUSED)
		{
			ClearFocus();
			//ts_largePreview = NULL;
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
		{
			MapNode *mn = (MapNode*)cr->info;

			if (mn->ts_preview != NULL)
			{
				tempGrabbedFile.Copy(mn);

				grabbedFile = &tempGrabbedFile;
				grabbedFile->ts_preview->SetQuadSubRect(grabbedFileQuad, 0);
				action = DRAG;

				mn->ts_preview = NULL;
				mn->filePath = "";

				SetRectNode(cr, mn);
			}
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTRELEASED)
		{
			if (action == DRAG )
			{
				int mapIndex = -1;
				for (int i = 0; i < 8; ++i)
				{
					if (mapRects[i] == cr)
					{
						mapIndex = i;
						break;
					}
				}

				assert(mapIndex >= 0);

				MapNode *currNode = GetCurrNode(mapIndex);

				currNode->ts_preview = grabbedFile->ts_preview;
				currNode->filePath = grabbedFile->filePath;


				SetRectNode(cr, currNode);
			}
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_RIGHTCLICKED)
		{
			MapNode *fn = (MapNode*)cr->info;

			EditSession *edit = EditSession::GetSession();
			assert(edit != NULL);

			chooser->TurnOff();
			edit->Reload(fn->filePath);
		}
	}
	else if (cr->rectIdentity == ChooseRect::I_MUSICLEVEL)
	{
		if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
		{
			//ClickText(cr);
			//cr->SetName("");
			//chooser->SetStoppedColorMyRects();

		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTRELEASED)
		{
			if (action == DRAG)
			{
				//cr->SetName(grabbedString);
				//chooser->ResetSlider(grabbedString);
				//if (chooser->playingSongName != "")
				//{
				//	chooser->SetPlayingColorMyRects(chooser->playingSongName);
				//}
			}
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_RIGHTCLICKED)
		{
			musicListHandler->ChooseRectEvent(cr, eventType);
			/*string crName = cr->nameText.getString();

			if (cr == chooser->currPlayingMyRect)
			{
				chooser->edit->StopMusic(chooser->edit->previewMusic);
			}
			else
			{
				chooser->edit->SetPreviewMusic(crName);
				chooser->edit->PlayMusic(chooser->edit->previewMusic);
			}*/
		}
	}
	else
	{
		assert(0);
	}
}

//void AdventureCreator::SliderCallback(Slider *slider)
//{
//	adventure->worlds[currWorld - 1].sectors[currSector - 1].requiredRunes = slider->currValue;
//}