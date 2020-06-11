#include "AdventureCreator.h"
#include "VectorMath.h"
#include <iostream>
#include "EditSession.h"

using namespace sf;
using namespace std;

AdventureCreator::AdventureCreator()
	:FileChooserHandler( 3, 4, EXTRA_RECTS )
{
	ts_largePreview = NULL;
	SetRectSubRect(largePreview, FloatRect(0, 0, 912, 492));
	SetRectTopLeft(largePreview, 912, 492, Vector2f(1000, 540 - 492 / 2));

	Panel *panel = chooser->panel;

	int rightSideStart = 650;

	Vector2f startWorldPos(rightSideStart, 100);

	Tileset *ts_worldChoosers = chooser->edit->GetSizedTileset("Editor/worldselector_64x64.png");

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
	}

	Vector2f startMapPos(rightSideStart, 360);
	for (int i = 0; i < 8; ++i)
	{
		mapRects[i] = panel->AddImageRect(ChooseRect::I_ADVENTURECREATOR_MAP,
			startMapPos + Vector2f(i * 140, 0), NULL, 0, 120);
		mapRects[i]->SetShown(true);
		mapRects[i]->Init();
	}
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
}

void AdventureCreator::Confirm()
{

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
		//target->draw(largePreview, 4, sf::Quads, ts_largePreview->texture);
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


void AdventureCreator::ChooseWorld(int w)
{
	currWorld = w;

	//update sector and map info
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
			ChooseWorld(icRect->tileIndex);
			CollapseWorlds();
		}
	}
	else
	{
		if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED)
		{
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_UNFOCUSED)
		{
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
		{
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTRELEASED)
		{
			if (state == DRAG)
			{
				FileNode *newNode = new FileNode(*grabbedFile);
				if (cr->info != NULL)
				{
					FileNode *oldNode = (FileNode*)cr->info;
					delete oldNode;
				}
				cr->SetInfo(newNode);
				ImageChooseRect *icRect = cr->GetAsImageChooseRect();
				icRect->SetImage(newNode->ts_preview, 0);
				icRect->SetName(newNode->filePath.stem().string());
			}
		}
	}
}