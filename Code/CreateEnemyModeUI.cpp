#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorDecorInfo.h"
#include "UIMouse.h"

using namespace std;
using namespace sf;

EnemyVariationSelector::EnemyVariationSelector( bool p_createMode )
	:PanelUpdater(), createMode( p_createMode )
{
	numVariations = 0;
	Color testColor = Color::White;
	
	
	edit = EditSession::GetSession();

	Tileset *ts_orb = edit->GetSizedTileset("Editor/orbselector_384x384.png");
	ts_orb->SetSpriteTexture(orbSpr);
	//orbSpr.setOrigin(orbSpr.getLocalBounds().width / 2, orbSpr.getLocalBounds().height / 2);

	int panelSize = 384;
	
	panel = new Panel("topbarpanel", panelSize, panelSize, edit, true);
	panel->SetPosition(Vector2i(0, 0));
	panel->extraUpdater = this;
	panel->SetColor(Color::Transparent);
	panel->ReserveEnemyRects(7);
	//centerRect = panel->AddEnemyRect(ChooseRect::ChooseRectIdentity::I_ENEMYLIBRARY,
	//	Vector2f(0, 0), NULL, 0);

	Vector2f centerPoint(panelSize / 2, panelSize / 2);
	//Vector2f enemyCenter = centerPoint - Vector2f(centerRect->boxSize / 2,
	//	centerRect->boxSize / 2);

	//centerRect->SetPosition(enemyCenter);

	int circleRadius = 64;
	int offsetRadius = circleRadius * 2;

	Vector2f offset(0, -offsetRadius);
	Transform tr;
	tr.rotate(360 / 6);

	ChooseRect::ChooseRectIdentity ident;
	if (createMode)
	{
		ident = ChooseRect::ChooseRectIdentity::I_ENEMYLIBRARY;
	}
	else
	{
		ident = ChooseRect::ChooseRectIdentity::I_CHANGEENEMYVAR;
	}

	for (int i = 0; i < 6; ++i)
	{
		varRects[i] = panel->AddEnemyRect(ident,
			Vector2f(0, 0), NULL, 0);
		Vector2f currPos = centerPoint - Vector2f( varRects[i]->boxSize.x / 2,
			varRects[i]->boxSize.y / 2 ) + tr.transformPoint(offset);
		varRects[i]->SetPosition(currPos);
		varRects[i]->SetCircleMode(circleRadius);
		tr.rotate(360 / 6);
	}
	
	//centerRect = new EnemyChooseRect(ChooseRect::ChooseRectIdentity::I_ENEMYLIBRARY,
		//enemyQuads, Vector2f(0, 0), NULL, 0, NULL);
}

EnemyVariationSelector::~EnemyVariationSelector()
{
	delete panel;
}

bool EnemyVariationSelector::MouseUpdate()
{
	/*if (MOUSE.IsMouseRightClicked())
	{
		edit->AddActivePanel(panel);
	}*/
	if (MOUSE.IsMouseRightReleased())
	{
		edit->RemoveActivePanel(panel);
	}

	return true;
}

void EnemyVariationSelector::Deactivate()
{

}

void EnemyVariationSelector::SetType(ActorType *type)
{
	for (int i = 0; i < 6; ++i)
	{
		varRects[i]->SetShown(false);
	}

	for (int i = 0; i < type->info.numLevels; ++i)
	{
		varRects[i]->SetType(type, i+1);
		varRects[i]->SetShown(true);
	}

	//centerRect->SetType(type, 1);
	//centerRect->SetShown(true);
}

void EnemyVariationSelector::SetPosition(sf::Vector2f &pos)
{
	panel->SetCenterPos(Vector2i(pos));
	//Vector2i dest = panel->pos + Vector2i(300, 0);
	//panel->Slide(dest, CubicBezier(), 120);
	//orbSpr.setPosition(pos);
	for (int i = 0; i < numVariations; ++i)
	{
	}
}

void EnemyVariationSelector::Draw(RenderTarget *target)
{
	target->draw(orbSpr);
	//draw orb system here
}

CreateEnemyModeUI::CreateEnemyModeUI()
{
	varSelector = new EnemyVariationSelector( true );
	activeHotbarSize = 0;

	hotbarEnemies.reserve(8);
	libraryEnemiesVec.resize(9);
	for (int i = 0; i < 9; ++i)
	{
		//libraryEnemiesVec[i].resize(100); //too big but adjust later
	}

	edit = EditSession::GetSession();

	int totalHotbarCount = 12;
	int hotbarRectSize = 100;
	int hotbarSpacing = 20;
	int totalHotbarSize = hotbarRectSize * totalHotbarCount + hotbarSpacing * (totalHotbarCount - 1);
	int extraHotbarSpacing = (1920 - totalHotbarSize) / 2;

	topbarPanel = new Panel("topbarpanel", 1920, 120, edit, false);
	topbarPanel->SetPosition(Vector2i(0, 20));

	Color panelColor;
	panelColor = Color::Cyan;
	panelColor.a = 80;
	topbarPanel->SetColor(panelColor);

	int enemyCounter = 0;
	for (auto it = edit->types.begin(); it != edit->types.end(); ++it)
	{
		if ((*it).second->unlisted)
			continue;
		/*if ((*it).first == "player")
		{
			continue;
		}*/
		enemyCounter++;//= (*it).second->info.numLevels;
	}

	allEnemyRects.reserve(enemyCounter);

	int numEnemyWorlds = 9;

	int numExtraRects = 1; //search library
	//numAllQuads = (enemyCounter + totalHotbarCount + numEnemyWorlds + numExtraRects);

	topbarPanel->ReserveEnemyRects(totalHotbarCount);
	topbarPanel->ReserveImageRects(numExtraRects);

	int worldSize = 100;
	int worldSpacing = 30;
	int totalWorldSize = worldSize * numEnemyWorlds + worldSpacing * (numEnemyWorlds - 1);

	int extraWorldSpacing = (1920 - totalWorldSize) / 2;

	libPanel = new Panel("libpanel", totalWorldSize + 20, 700, edit, false);
	libPanel->SetPosition(Vector2i(100, 140));
	libPanel->SetColor(panelColor);

	libPanel->ReserveEnemyRects(enemyCounter);
	libPanel->ReserveImageRects(numEnemyWorlds);

	Tileset *ts_worldChoosers = edit->GetSizedTileset("Editor/worldselector_64x64.png");

	librarySearchRect = topbarPanel->AddImageRect(ChooseRect::I_SEARCHENEMYLIBRARY, 
		Vector2f(10 + 100, 10), ts_worldChoosers, 8);

	librarySearchRect->SetShown(true);
	librarySearchRect->Init();

	{
		int i = 0;
		for (auto it = edit->types.begin(); it != edit->types.end(); ++it)
		{
			if ((*it).second->unlisted)
				continue;
			/*if ((*it).first == "player" || (*it).first == "multiplayerbase")
			{
				continue;
			}*/

			allEnemyRects.push_back(libPanel->AddEnemyRect(
				ChooseRect::I_ENEMYLIBRARY, Vector2f(0, 0), (*it).second, 1));
			/*for (int level = 1; level <= (*it).second->info.numLevels; ++level)
			{
				allEnemyRects.push_back(libPanel->AddEnemyRect(
					ChooseRect::I_ENEMYLIBRARY, Vector2f(0, 0), (*it).second, level));
				++i;
			}*/
		}
	}

	for (int i = 0; i < totalHotbarCount; ++i)
	{
		hotbarEnemies.push_back(topbarPanel->AddEnemyRect(
			ChooseRect::I_ENEMYHOTBAR,
			Vector2f(extraHotbarSpacing + 10 + i * (hotbarRectSize + hotbarSpacing), 10),
			NULL, 0));
		hotbarEnemies[i]->SetShown(false);
		hotbarEnemies[i]->Init();
	}

	worldSelectRects.reserve(numEnemyWorlds);

	for (int i = 0; i < numEnemyWorlds; ++i)
	{
		worldSelectRects.push_back(
			libPanel->AddImageRect(
				ChooseRect::I_WORLDCHOOSER,
				Vector2f(i * (worldSize + worldSpacing) + 10, 10),
				ts_worldChoosers,
				i));
		worldSelectRects[i]->SetShown(false);
		worldSelectRects[i]->Init();
	}

	int counter;
	for (int w = 0; w < 8; ++w)
	{
		counter = 0;
		for (int i = 0; i < allEnemyRects.size(); ++i)
		{
			if (allEnemyRects[i]->actorType->info.world == w)
			{
				++counter;
			}
		}
		libraryEnemiesVec[w].resize(counter);
	}

	int row, col;
	int maxCol = 9;
	EnemyChooseRect *ecRect;
	for (int w = 0; w < 8; ++w)
	{
		counter = 0;
		for (int i = 0; i < allEnemyRects.size(); ++i)
		{
			if (allEnemyRects[i]->actorType->info.world == w)
			{
				libraryEnemiesVec[w][counter] = allEnemyRects[i];
				ecRect = libraryEnemiesVec[w][counter];

				col = counter % maxCol;
				row = counter / maxCol;

				ecRect->SetPosition(Vector2f(10 + col * 120, 240 + row * 120));
				//ecRect->SetShown(true);
				ecRect->Init();
				++counter;
			}
		}
	}

	activeLibraryWorld = -1;
	showLibrary = false;
	show = true;

	UpdateHotbarTypes();
}

CreateEnemyModeUI::~CreateEnemyModeUI()
{
	delete varSelector;
	delete topbarPanel;
	delete libPanel;
}

void CreateEnemyModeUI::UpdateHotbarTypes()
{
	int i = 0;
	for (auto it = edit->recentEnemies.begin(); it != edit->recentEnemies.end(); ++it)
	{
		hotbarEnemies[i]->SetType((*it).first, (*it).second);
		hotbarEnemies[i]->SetShown(true);
		++i;
	}
	activeHotbarSize = i;
	for (; i < EditSession::MAX_RECENT_ENEMIES; ++i)
	{
		hotbarEnemies[i]->SetType(NULL, 0);
	}
}

void CreateEnemyModeUI::SetShown(bool s)
{
	show = s;
	if (show)
	{
		edit->AddActivePanel(topbarPanel);
		if (showLibrary)
		{
			edit->AddActivePanel(libPanel);
		}
	}
	else
	{
		edit->RemoveActivePanel(varSelector->panel);
		//varSelector->Deactivate();
		edit->RemoveActivePanel(topbarPanel);
		//topbarPanel->Deactivate();
		if (showLibrary)
		{
			edit->RemoveActivePanel(libPanel);
			//libPanel->Deactivate();
		}
	}
}

void CreateEnemyModeUI::SetActiveLibraryWorld(int w)
{
	if (w == activeLibraryWorld)
		return;

	if (activeLibraryWorld >= 0)
	{
		for (auto it = libraryEnemiesVec[activeLibraryWorld].begin();
			it != libraryEnemiesVec[activeLibraryWorld].end(); ++it)
		{
			if( (*it) != NULL )
				(*it)->SetShown(false);
		}
	}

	activeLibraryWorld = w;

	if (activeLibraryWorld >= 0)
	{
		for (auto it = libraryEnemiesVec[activeLibraryWorld].begin();
			it != libraryEnemiesVec[activeLibraryWorld].end(); ++it)
		{
			if( (*it) != NULL )
				(*it)->SetShown(true);
		}
	}
}

void CreateEnemyModeUI::SetLibraryShown(bool s)
{
	if (showLibrary != s)
	{
		showLibrary = s;
		SetActiveLibraryWorld(-1);

		if (showLibrary)
		{
			edit->AddActivePanel(libPanel);
			for (int i = 0; i < 9; ++i)
			{
				worldSelectRects[i]->SetShown(true);
			}
		}
		else
		{
			edit->RemoveActivePanel(libPanel);
			for (int i = 0; i < 9; ++i)
			{
				worldSelectRects[i]->SetShown(false);
			}
		}

	}
}

void CreateEnemyModeUI::FlipLibraryShown()
{
	SetLibraryShown(!showLibrary);
}

void CreateEnemyModeUI::ExpandVariation(EnemyChooseRect *ceRect)
{
	edit->AddActivePanel(varSelector->panel);
	varSelector->SetPosition(ceRect->GetGlobalCenterPos());
	varSelector->SetType(ceRect->enemyParams->type);
}