#include "AdventureCreator.h"
#include "VectorMath.h"
#include <iostream>
#include "EditSession.h"
#include "SaveFile.h"

using namespace sf;
using namespace std;
using namespace boost::filesystem;

AdventureCreator::AdventureCreator()
	:FileChooserHandler( 3, 4, EXTRA_RECTS )
{
	adventure = new AdventureFile;

	ts_largePreview = NULL;
	SetRectSubRect(largePreview, FloatRect(0, 0, 912, 492));
	

	Panel *panel = chooser->panel;

	int rightSideStart = 650;

	SetRectTopLeft(largePreview, 912, 492, Vector2f(rightSideStart, 540 ));

	Vector2f startWorldPos(rightSideStart, 100);
	Vector2f startSectorPos(rightSideStart, 220);

	Tileset *ts_worldChoosers = chooser->edit->GetSizedTileset("Editor/worldselector_64x64.png");

	ts_sectorIcons = chooser->edit->GetSizedTileset("Editor/sectoricons_64x64.png");

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

	Vector2f startMapPos(rightSideStart, 360);
	for (int i = 0; i < 8; ++i)
	{
		mapRects[i] = panel->AddImageRect(ChooseRect::I_ADVENTURECREATOR_MAP,
			startMapPos + Vector2f(i * 140, 0), NULL, 0, 120);
		mapRects[i]->SetShown(true);
		mapRects[i]->Init();
		//mapRects[i]->SetInfo((void*)(i+1));
	}
}

AdventureCreator::~AdventureCreator()
{
	delete adventure;
}


void AdventureCreator::Cancel()
{
	chooser->TurnOff();
}

void AdventureCreator::Open()
{
	state = BROWSE;
	chooser->StartRelative( ".brknk", FileChooser::Mode::OPEN, "Resources/Maps");

	CollapseWorlds();
	currWorld = 1;
	ChooseSector(1);
}

void AdventureCreator::LoadAdventure(const std::string &path, 
	const std::string &adventureName)
{
	adventure->Load(path, adventureName);

	string worldStr;
	string sectorStr;
	string file;
	int ind;
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
				name = adventure->worlds[w].sectors[s].maps[m].name;
				if (name != "----");
				{
					file = sectorStr + name;
					adventureNodes[ind].filePath = file + ".brknk";
					adventureNodes[ind].ts_preview = chooser->GetTileset(file + ".png");
				}
				
			}
		}
	}
}

void AdventureCreator::SaveAdventure(const std::string &p_path, 
	const std::string &adventureName)
{
	string pathStr = p_path + "/" + adventureName;
	path p(pathStr);

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
	FileNode *currentNode;
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
					boost::filesystem::copy_file(currentNode->filePath, sectorStr + "/" + fileName + ".brknk");

					imagePath = currentNode->filePath.parent_path().string() + "/" + fileName + ".png";
					boost::filesystem::copy_file(imagePath, sectorStr + "/" + fileName + ".png");
				}
			}
		}
	}

	adventure->Save(p.string(), adventureName);
}

void AdventureCreator::Confirm()
{
	SaveAdventure(chooser->currPath.string(), "tadventure");
}

bool AdventureCreator::MouseUpdate()
{
	if (MOUSE.IsMouseLeftReleased())
	{
		state = BROWSE;
		grabbedFile = NULL;
	}

	if (state == DRAG)
	{
		SetRectCenter(grabbedFileQuad, grabbedFile->ts_preview->tileWidth / 8,
			grabbedFile->ts_preview->tileHeight / 8, Vector2f(chooser->panel->GetMousePos()));
	}
	return true;
}

void AdventureCreator::ClickFile(ChooseRect *cr)
{
	FileNode *fn = (FileNode*)cr->info;
	
	if (fn->ts_preview != NULL)
	{
		grabbedFile = fn;
		fn->ts_preview->SetQuadSubRect(grabbedFileQuad, 0);
		state = DRAG;
	}
	//grabbedFileSpr.setTexture( )
}

void AdventureCreator::FocusFile(ChooseRect *cr)
{
	ts_largePreview = cr->GetAsImageChooseRect()->ts;
}

void AdventureCreator::UnfocusFile(ChooseRect *cr)
{
	ts_largePreview = NULL;
}

void AdventureCreator::Draw(sf::RenderTarget *target)
{
	if (ts_largePreview)
	{
		target->draw(largePreview, 4, sf::Quads, ts_largePreview->texture);
	}
}

void AdventureCreator::LateDraw(sf::RenderTarget *target)
{
	if (state == DRAG)
	{
		target->draw(grabbedFileQuad, 4, sf::Quads, grabbedFile->ts_preview->texture);
	}
}

void AdventureCreator::ChangePath()
{
	ts_largePreview = NULL;
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

void AdventureCreator::SetRectNode(ChooseRect *cr, FileNode *fn)
{
	ImageChooseRect *icRect = cr->GetAsImageChooseRect();
	icRect->SetInfo(fn);
	icRect->SetImage(fn->ts_preview, 0);
	if (fn->ts_preview == NULL)
		icRect->SetShown(true);
	icRect->SetName(fn->filePath.stem().string());
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
		SetRectNode(mapRects[i], &(adventureNodes[nodeStart + i]));
	}
	//update maps
}

FileNode * AdventureCreator::GetCurrNode(int m)
{
	return &(adventureNodes[GetNodeStart() + m]);
}

void AdventureCreator::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (cr->rectIdentity == ChooseRect::I_FILESELECT)
	{
		FileChooserHandler::ChooseRectEvent(cr, eventType);
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
	}
	else if( cr->rectIdentity == ChooseRect::I_ADVENTURECREATOR_MAP )//map
	{
		if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED)
		{
			ts_largePreview = cr->GetAsImageChooseRect()->ts;
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_UNFOCUSED)
		{
			ts_largePreview = NULL;
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
		{
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTRELEASED)
		{
			if (state == DRAG)
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

				FileNode *currNode = GetCurrNode(mapIndex);

				currNode->ts_preview = grabbedFile->ts_preview;
				currNode->filePath = grabbedFile->filePath;

				SetRectNode(cr, currNode);
			}
		}
	}
	else
	{
		assert(0);
	}
}