#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorDecorInfo.h"

using namespace std;
using namespace sf;

PanelSlider::PanelSlider(Panel *p, sf::Vector2i &p_origPos, sf::Vector2i &p_destPos)
	:panel (p), slid(false)
{
	origPos = p_origPos;//Vector2i(-270, 0);
	destPos = p_destPos;
	panel->SetPosition(origPos);
	normalDuration = 10;
	bez = CubicBezier(0, 0, 1, 1);
}


bool PanelSlider::MouseUpdate()
{
	if (!slid)
	{
		slid = true;
		int duration = normalDuration;// - outFrame;
		int skip = 0;
		if (panel->IsSliding())
		{
			skip = normalDuration - panel->slideFrame;
		}
		panel->SetPosition(origPos);
		panel->Slide(destPos, bez, duration);
		panel->slideFrame = skip;
	}

	return true;
}

void PanelSlider::Deactivate()
{
	int duration = normalDuration;
	int skip = 0;
	if (panel->IsSliding())
	{
		skip = normalDuration - panel->slideFrame;
	}
	panel->SetPosition(destPos);
	panel->Slide(origPos, bez, duration);
	panel->slideFrame = skip;
	slid = false;
}

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
		Vector2f currPos = centerPoint - Vector2f( varRects[i]->boxSize / 2,
			varRects[i]->boxSize / 2 ) + tr.transformPoint(offset);
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
		if ((*it).first == "player")
		{
			continue;
		}
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
		libraryEnemiesVec[w].resize(counter);
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
	:PanelMember(p), quad(v), boxSize(size), pos(p_pos), chooseRectType(crType),
		rectIdentity(ident), circleMode(false)
{
	idleColor = Color::Black;
	idleColor.a = 100;

	mouseOverColor = Color::Green;
	mouseOverColor.a = 100;

	SetShown(false);

	focused = false;

	EditSession *edit = EditSession::GetSession();
	nameText.setFont(edit->mainMenu->arial);
	nameText.setCharacterSize(18);
	nameText.setFillColor(Color::White);
	nameText.setOutlineColor(Color::Black);
	nameText.setOutlineThickness(3);
	nameText.setPosition(Vector2f( pos.x + boxSize / 2, pos.y ));
}

void ChooseRect::SetCircleMode(int p_radius)
{
	circleMode = true;
	circleRadius = p_radius;
}

void ChooseRect::SetRectMode()
{
	circleMode = false;
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
	nameText.setPosition(Vector2f(pos.x + boxSize / 2, pos.y));
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
		if (circleMode)
		{
			SetRectTopLeft(quad, 0, 0, Vector2f(0, 0));
		}
		else
		{
			SetRectTopLeft(quad, boxSize, boxSize, pos);
		}
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

bool ChooseRect::ContainsPoint(sf::Vector2i &mousePos)
{
	if (circleMode)
	{
		Vector2i center(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
		Vector2f diff(mousePos - center);
		return (length(diff) <= circleRadius);
	}
	else
	{
		return bounds.contains(mousePos);
	}
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
		if (ContainsPoint( mousePos ) )
		{
			edit->ChooseRectEvent(this, E_LEFTCLICKED );
			focused = true;
		}
	}
	else if (!MOUSE.IsMouseDownLeft())
	{
		if (ContainsPoint(mousePos))
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
		if (ContainsPoint(mousePos))
		{
			edit->ChooseRectEvent(this, E_RIGHTCLICKED);
			//focused = true;
		}
	}
	else if (MOUSE.IsMouseRightReleased())
	{
		if (ContainsPoint(mousePos))
		{
			edit->ChooseRectEvent(this, E_RIGHTRELEASED);
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
			nameText.setString(enemy->name);
			nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
				0);
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

		target->draw(nameText);
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

void ImageChooseRect::SetImage(Tileset *p_ts, const sf::IntRect &subRect)
{
	if (p_ts == NULL)
	{
		SetShown(false);
		ts = p_ts;
		tileIndex = -1;
		return;
	}

	ts = p_ts;
	tileIndex = -1;

	ts->SetSpriteTexture(spr);
	spr.setTextureRect(subRect);
	spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);

	SetSize(boxSize);
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

		SetSize(boxSize);
	}
}

ImageChooseRect::ImageChooseRect(ChooseRectIdentity ident, sf::Vertex *v, Vector2f &p_pos, Tileset *p_ts,
	int p_tileIndex, Panel *p )
	:ChooseRect( ident, ChooseRectType::IMAGE, v, 100, p_pos, p )
{
	ts = NULL;
	SetImage(p_ts, p_tileIndex);
}

ImageChooseRect::ImageChooseRect(ChooseRectIdentity ident, sf::Vertex *v,
	sf::Vector2f &p_pos, Tileset *p_ts, const sf::IntRect &subRect, Panel *p)
	:ChooseRect(ident, ChooseRectType::IMAGE, v, 100, p_pos, p)
{
	ts = NULL;
	SetImage(p_ts, subRect);
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


