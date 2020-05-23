#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorDecorInfo.h"

using namespace std;
using namespace sf;

EnemyVariationSelector::EnemyVariationSelector()
	:PanelUpdater()
{
	SetReplaceDraw(false);

	numVariations = 0;
	Color testColor = Color::White;
	
	
	edit = EditSession::GetSession();

	int panelSize = 350;
	
	panel = new Panel("topbarpanel", panelSize, panelSize, edit, true);
	panel->SetPosition(Vector2i(0, 0));
	panel->extraUpdater = this;
	panel->SetColor(Color::Black);
	panel->ReserveEnemyRects(7);
	//centerRect = panel->AddEnemyRect(ChooseRect::ChooseRectIdentity::I_ENEMYLIBRARY,
	//	Vector2f(0, 0), NULL, 0);

	Vector2f centerPoint(panelSize / 2, panelSize / 2);
	//Vector2f enemyCenter = centerPoint - Vector2f(centerRect->boxSize / 2,
	//	centerRect->boxSize / 2);

	//centerRect->SetPosition(enemyCenter);

	Vector2f offset(0, -100);
	Transform tr;
	tr.rotate(360 / 6);
	for (int i = 0; i < 6; ++i)
	{
		varRects[i] = panel->AddEnemyRect(ChooseRect::ChooseRectIdentity::I_ENEMYLIBRARY,
			Vector2f(0, 0), NULL, 0);
		Vector2f currPos = centerPoint - Vector2f( varRects[i]->boxSize / 2,
			varRects[i]->boxSize / 2 ) + tr.transformPoint(offset);
		varRects[i]->SetPosition(currPos);
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

	for (int i = 0; i < numVariations; ++i)
	{
	}
}

void EnemyVariationSelector::Draw(RenderTarget *target)
{
	//draw orb system here
}

CreateEnemyModeUI::CreateEnemyModeUI()
{
	varSelector = new EnemyVariationSelector;
	activeHotbarSize = 0;

	hotbarEnemies.reserve(8);
	libraryEnemiesVec.resize(9);
	for (int i = 0; i < 9; ++i)
	{
		libraryEnemiesVec[i].resize(100); //too big but adjust later
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
		if ((*it).first == "player")
		{
			continue;
		}
		enemyCounter++;//= (*it).second->info.numLevels;
	}

	allEnemyRects.reserve(enemyCounter);

	int numEnemyWorlds = 9;

	int numExtraRects = 1; //search library
	numAllQuads = (enemyCounter + totalHotbarCount + numEnemyWorlds + numExtraRects);

	topbarPanel->ReserveEnemyRects(totalHotbarCount);
	topbarPanel->ReserveImageRects(numExtraRects);

	int worldSize = 100;
	int worldSpacing = 30;
	int totalWorldSize = worldSize * numEnemyWorlds + worldSpacing * (numEnemyWorlds - 1);

	int extraWorldSpacing = (1920 - totalWorldSize) / 2;

	libPanel = new Panel("libpanel", totalWorldSize + 20, 600, edit, false);
	libPanel->SetPosition(Vector2i(100, 140));
	libPanel->SetColor(panelColor);

	libPanel->ReserveEnemyRects(enemyCounter);
	libPanel->ReserveImageRects(numEnemyWorlds);

	Tileset *ts_worldChoosers = edit->GetSizedTileset("worldselector_64x64.png");

	librarySearchRect = topbarPanel->AddImageRect(ChooseRect::I_SEARCHENEMYLIBRARY, 
		Vector2f(10 + 100, 10), ts_worldChoosers, 8);

	librarySearchRect->SetShown(true);
	librarySearchRect->Init();

	{
		int i = 0;
		for (auto it = edit->types.begin(); it != edit->types.end(); ++it)
		{
			if ((*it).first == "player")
			{
				continue;
			}

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
	for (int w = 0; w < 2; ++w)
	{
		counter = 0;
		for (int i = 0; i < allEnemyRects.size(); ++i)
		{
			if (allEnemyRects[i]->actorType->info.world == w)
			{
				++counter;
			}
		}
		libraryEnemiesVec[w].reserve(counter);
	}

	int row, col;
	int maxCol = 9;
	EnemyChooseRect *ecRect;
	for (int w = 0; w < 2; ++w)
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
		edit->RemoveActivePanel(topbarPanel);
		if (showLibrary)
		{
			edit->RemoveActivePanel(libPanel);
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

void CreateEnemyModeUI::UpdateSprites(int sprUpdateFrames)
{
	for (int i = 0; i < activeHotbarSize; ++i)
	{
		if (activeLibraryWorld != hotbarEnemies[i]->actorType->info.world )
		{
			hotbarEnemies[i]->UpdateSprite(sprUpdateFrames);
		}
	}

	if (activeLibraryWorld >= 0)
	{
		for (auto it = libraryEnemiesVec[activeLibraryWorld].begin();
			it != libraryEnemiesVec[activeLibraryWorld].end(); ++it)
		{
			if ((*it) != NULL)
			{
				(*it)->UpdateSprite(sprUpdateFrames);
			}
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


ChooseRect::ChooseRect(ChooseRectIdentity ident, ChooseRectType crType, 
	Vertex *v, float size, sf::Vector2f &p_pos, Panel *p)
	:PanelMember( p ), quad(v), boxSize( size ), pos( p_pos ), chooseRectType( crType ), 
	rectIdentity( ident )
{
	idleColor = Color::Black;
	idleColor.a = 100;

	mouseOverColor = Color::Green;
	mouseOverColor.a = 100;

	SetShown(false);

	focused = false;
}

void ChooseRect::Init()
{
	SetRectColor(quad, idleColor);
	SetSize(boxSize);
}

void ChooseRect::SetPosition(sf::Vector2f &p_pos)
{
	pos = p_pos;
	UpdateRectDimensions();
}

void ChooseRect::SetSize(float s)
{
	boxSize = s;
	UpdateRectDimensions();
}

void ChooseRect::UpdateRectDimensions()
{
	//SetRectCenter(quad, boxSize, boxSize, pos);
	bounds.left = pos.x;// -boxSize / 2.f;
	bounds.top = pos.y;// -boxSize / 2.f;
	bounds.width = boxSize;
	bounds.height = boxSize;
}

void ChooseRect::SetShown(bool s)
{
	if (!s && show )
	{
		SetRectTopLeft(quad, 0, 0, Vector2f(0, 0));
	}
	else if (s && !show)
	{
		SetRectTopLeft(quad, boxSize, boxSize, pos);//GetGlobalPos());
		SetSize(boxSize);
	}
	show = s;
}

sf::Vector2f ChooseRect::GetGlobalPos()
{
	//return mouseUser->GetFloatPos() + pos;
	return Vector2f(panel->pos) + pos;
}

sf::Vector2f ChooseRect::GetGlobalCenterPos()
{
	return GetGlobalPos() + Vector2f(boxSize / 2, boxSize / 2);
}

void ChooseRect::SetActive(bool a)
{
	active = a;
}

void ChooseRect::Deactivate()
{
	focused = false;
	SetRectColor(quad, idleColor);
}

bool ChooseRect::MouseUpdate()
{
	if( !show )
	{
		return false;
	}

	Vector2i mousePos = panel->GetMousePos();//mouseUser->GetMousePos();//panel->GetMousePos();
	EditSession *edit = EditSession::GetSession();

	if (MOUSE.IsMouseLeftClicked())
	{
		if (bounds.contains(mousePos))
		{
			edit->ChooseRectEvent(this, E_CLICKED );
			focused = true;
		}
	}
	else if (!MOUSE.IsMouseDownLeft())
	{
		if (bounds.contains(mousePos))
		{
			SetRectColor(quad, mouseOverColor);
			if (!focused)
			{
				edit->ChooseRectEvent(this, E_FOCUSED);
			}
			focused = true;
		}
		else
		{
			SetRectColor(quad, idleColor);
			if (focused)
			{
				edit->ChooseRectEvent(this, E_UNFOCUSED);
			}
			focused = false;

			//Unfocus();
		}
	}

	if (MOUSE.IsMouseRightClicked())
	{
		if (bounds.contains(mousePos))
		{
			edit->ChooseRectEvent(this, E_RIGHTCLICKED);
			//focused = true;
		}
	}
	
	return true;
}

EnemyChooseRect *ChooseRect::GetAsEnemyChooseRect()
{
	if (chooseRectType == ENEMY)
	{
		return (EnemyChooseRect*)this;
	}
	else
	{
		return NULL;
	}
}

ImageChooseRect *ChooseRect::GetAsImageChooseRect()
{
	if (chooseRectType == IMAGE )
	{
		return (ImageChooseRect*)this;
	}
	else
	{
		return NULL;
	}
}

EnemyChooseRect::EnemyChooseRect(ChooseRectIdentity ident, sf::Vertex *v, Vector2f &p_pos, ActorType * p_type,int p_level,
	Panel *p )
	:ChooseRect( ident, ChooseRectType::ENEMY, v, 100, p_pos, p), level( p_level )
{
	actorType = NULL;
	SetType(p_type, level);
}

//void EnemyChooseRect::Unfocus()
//{
//	if (enemy != NULL)
//	{
//		enemy->SetActionEditLoop();
//		enemy->UpdateFromEditParams(0);
//	}
//}

void EnemyChooseRect::SetType(ActorType *type, int lev)
{
	if (type == NULL)
	{
		SetShown(false);
		actorType = type;
		level = lev;
		return;
	}

	if (!(type == actorType && lev == level))
	{
		actorType = type;
		level = lev;
		enemyParams = actorType->defaultParamsVec[level - 1];
		enemyParams->MoveTo(Vector2i(0, 0));
		enemy = enemyParams->myEnemy;
		if (enemy != NULL)
		{
			enemy->SetActionEditLoop();
			enemy->UpdateFromEditParams(0);
		}
		
		SetSize(boxSize);

		switch (level)
		{
		case 1:
			idleColor = Color::Blue;
			break;
		case 2:
			idleColor = Color::Cyan;
			break;
		case 3:
			idleColor = Color::Magenta;
			break;
		case 4:
			idleColor = Color::Red;
			break;
		
		}
		idleColor.a = 100;
		SetRectColor(quad, idleColor);
	}
}

void EnemyChooseRect::SetSize(float s)
{
	ChooseRect::SetSize(s);
	if (actorType != NULL)
	{
		Vector2f truePos = GetGlobalPos() + Vector2f( boxSize / 2.f, boxSize / 2.f );

		float test;
		FloatRect aabb = enemyParams->GetAABB();

		float max = std::max(aabb.height, aabb.width);
		//max *= 1.1f;
		test = max / boxSize;
		view.setCenter(Vector2f(960 * test - truePos.x * test, 540 * test - truePos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
		view.setSize(Vector2f(1920 * test, 1080 * test));
	}
}

void EnemyChooseRect::Draw(RenderTarget *target)
{
	if (show)
	{
		sf::View oldView = target->getView();
		target->setView(view);

		enemyParams->DrawEnemy(target);

		target->setView(oldView);
	}
}

void EnemyChooseRect::UpdateSprite(int frameUpdate)
{
	if (!show)
	{
		return;
	}

	if (actorType != NULL)
	{
		if (enemy != NULL)
		{
			if (focused)
			{
				enemy->UpdateFromEditParams(frameUpdate);
			}
			else
			{
				enemy->SetActionEditLoop();
				enemy->UpdateFromEditParams(0);	
			}
		}
	}
}

EditorDecorInfo * ImageChooseRect::CreateDecor()
{
	EditorDecorInfo *edi = new EditorDecorInfo(decorName, ts, tileIndex, 0, Vector2f( 0, 0 ),
		0, Vector2f( 1, 1 ));
	return edi;
}

void ImageChooseRect::SetImage(Tileset *p_ts, int p_index)
{
	if (p_ts == NULL)
	{
		SetShown(false);
		ts = p_ts;
		tileIndex = p_index;
		return;
	}

	if (!(p_ts == ts && p_index == tileIndex))
	{
		ts = p_ts;
		tileIndex = p_index;


		ts->SetSpriteTexture(spr);
		ts->SetSubRect(spr, tileIndex);
		spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);

		//actorType->defaultParamsVec[level - 1]->MoveTo(Vector2i(0, 0));
		//enemy = actorType->defaultParamsVec[level - 1]->myEnemy;
		//enemy->SetActionEditLoop();
		//enemy->UpdateFromEditParams(0);
		SetSize(boxSize);

		/*switch (level)
		{
		case 1:
			idleColor = Color::Blue;
			break;
		case 2:
			idleColor = Color::Cyan;
			break;
		case 3:
			idleColor = Color::Magenta;
			break;
		case 4:
			idleColor = Color::Red;
			break;

		}
		idleColor.a = 100;*/
		//SetRectColor(quad, idleColor);
	}
}

ImageChooseRect::ImageChooseRect(ChooseRectIdentity ident, sf::Vertex *v, Vector2f &p_pos, Tileset *p_ts,
	int p_tileIndex, Panel *p )
	:ChooseRect( ident, ChooseRectType::IMAGE, v, 100, p_pos, p )
{
	ts = NULL;
	SetImage(p_ts, p_tileIndex);
}

void ImageChooseRect::SetSize(float s)
{
	ChooseRect::SetSize(s);

	Vector2f truePos = GetGlobalPos() + Vector2f( boxSize/2.f, boxSize / 2.f );

	float test;
	FloatRect aabb = spr.getGlobalBounds();
	float max = std::max(aabb.height, aabb.width) * 1.2f; //.8 to give the box a little room
	test = max / boxSize;
	view.setCenter(Vector2f(960 * test - truePos.x * test, 540 * test - truePos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
	view.setSize(Vector2f(1920 * test, 1080 * test));
}

void ImageChooseRect::Draw(RenderTarget *target)
{
	if (show)
	{
		sf::View oldView = target->getView();
		target->setView(view);

		target->draw(spr);

		target->setView(oldView);
	}
}

void ImageChooseRect::UpdateSprite(int frameUpdate)
{
	//animate eventually
}


