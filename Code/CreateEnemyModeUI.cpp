#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"

using namespace std;
using namespace sf;


CreateEnemyModeUI::CreateEnemyModeUI()
{
	hotbarEnemies.reserve(8);
	libraryEnemiesVec.resize(9);
	for (int i = 0; i < 9; ++i)
	{
		libraryEnemiesVec[i].resize(20);
	}

	edit = EditSession::GetSession();

	topbarPanel = new Panel("topbar", 1920, 150, edit);
	libraryPanel = new Panel("library", 900, 500, edit);
	libraryPanel->pos = Vector2i(300, 300);

	int enemyCounter = 0;
	for (auto it = edit->types.begin(); it != edit->types.end(); ++it)
	{
		if ((*it).first == "player")
		{
			continue;
		}
		enemyCounter += (*it).second->info.numLevels;
	}

	allEnemyRects.reserve(enemyCounter);
	allEnemyQuads = new Vertex[enemyCounter * 4];

	{
		int i = 0;
		for (auto it = edit->types.begin(); it != edit->types.end(); ++it)
		{
			if ((*it).first == "player")
			{
				continue;
			}
			for (int level = 1; level <= (*it).second->info.numLevels; ++level)
			{
				allEnemyRects.push_back(EnemyChooseRect(allEnemyQuads + i * 4, topbarPanel,
					Vector2f(0, 0), (*it).second, level));
				//		allEnemyRects.back().Init(); //need to set the position first
				//allEnemyRects.back().SetShown(false);
				++i;
			}
		}
	}

	activeHotbarSize = allEnemyRects.size();
	if (activeHotbarSize > 9)
	{
		activeHotbarSize = 9;
	}

	hotbarQuads = new Vertex[9 * 4];
	for (int i = 0; i < 9 && i < activeHotbarSize; ++i)
	{
		hotbarEnemies.push_back(EnemyChooseRect( hotbarQuads + i * 4, topbarPanel, Vector2f(400 + i * 120, 90),
			allEnemyRects[i].actorType, allEnemyRects[i].level ));
		hotbarEnemies[i].SetShown(true);
		hotbarEnemies[i].Init();
	}

	worldSelectRects.reserve(9);
	Tileset *ts = edit->GetSizedTileset("worldselector_64x64.png");
	worldSelectQuads = new Vertex[9 * 4];
	for (int i = 0; i < 9; ++i)
	{
		worldSelectRects.push_back(ImageChooseRect(worldSelectQuads + i * 4, topbarPanel,
			Vector2f( i * 130 + 300, 400 ), ts, i));
		worldSelectRects[i].SetShown(true);
		worldSelectRects[i].Init();
	}

	int counter;
	for (int w = 0; w < 2; ++w)
	{
		counter = 0;
		for (int i = 0; i < allEnemyRects.size(); ++i)
		{
			if (allEnemyRects[i].actorType->info.world == w)
			{
				++counter;
			}
		}
		libraryEnemiesVec[w].reserve(counter);
		//libraryEnemiesVec[0].push_back( )
	}

	EnemyChooseRect *ecRect;
	for (int w = 0; w < 2; ++w)
	{
		counter = 0;
		for (int i = 0; i < allEnemyRects.size(); ++i)
		{
			if (allEnemyRects[i].actorType->info.world == w)
			{
				libraryEnemiesVec[w][counter] = &allEnemyRects[i];
				ecRect = libraryEnemiesVec[w][counter];
				ecRect->SetPosition(Vector2f(400 + i * 120, 600));
				//ecRect->SetShown(true);
				ecRect->Init();
				++counter;
			}
		}
	}

	activeLibraryWorld = -1;
}

CreateEnemyModeUI::~CreateEnemyModeUI()
{
	delete topbarPanel;
	delete libraryPanel;

	delete [] allEnemyQuads;
	delete[] worldSelectQuads;
	delete[] hotbarQuads;
}

void CreateEnemyModeUI::SetActiveLibraryWorld(int w)
{
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
	for (int i = 0; i < hotbarEnemies.size(); ++i)
	{
		if (hotbarEnemies[i].actorType->info.world != activeLibraryWorld)
		{
			hotbarEnemies[i].UpdateSprite(sprUpdateFrames);
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

void CreateEnemyModeUI::Update(bool mouseDownL, bool mouseDownR, sf::Vector2i &mousePos)
{
	topbarPanel->Update(mouseDownL, mouseDownR, mousePos.x, mousePos.y);
	libraryPanel->Update(mouseDownL, mouseDownR, mousePos.x, mousePos.y);

	for (int i = 0; i < hotbarEnemies.size(); ++i)
	{
		hotbarEnemies[i].Update();
	}

	for (int i = 0; i < 9; ++i)
	{
		worldSelectRects[i].Update();
	}

	if (activeLibraryWorld >= 0)
	{
		for (auto it = libraryEnemiesVec[activeLibraryWorld].begin();
			it != libraryEnemiesVec[activeLibraryWorld].end(); ++it)
		{
			if ((*it) != NULL)
			{
				(*it)->Update();
			}
		}
	}
}

void CreateEnemyModeUI::Draw(sf::RenderTarget *target)
{
	sf::View oldView = target->getView();
	target->setView(edit->uiView);


	topbarPanel->Draw(target);
	libraryPanel->Draw(target);

	target->draw(hotbarQuads, activeHotbarSize * 4, sf::Quads);
	for (int i = 0; i < activeHotbarSize; ++i)
	{
		hotbarEnemies[i].Draw(target);
	}

	target->draw(allEnemyQuads, allEnemyRects.size() * 4, sf::Quads);
	target->draw(worldSelectQuads, 9 * 4, sf::Quads);
	for (int i = 0; i < 9; ++i)
	{
		worldSelectRects[i].Draw(target);
	}

	if (activeLibraryWorld >= 0)
	{
		for (auto it = libraryEnemiesVec[activeLibraryWorld].begin();
			it != libraryEnemiesVec[activeLibraryWorld].end(); ++it)
		{
			if ((*it) != NULL)
			{
				(*it)->Draw(target);
			}
		}
	}

	target->setView(oldView);
}

ChooseRect::ChooseRect( ChooseRectType crType, Vertex *v, Panel *p_panel, float size, sf::Vector2f &p_pos)
	:panel( p_panel), quad(v), boxSize( size ), pos( p_pos ), chooseRectType( crType )
{
	idleColor = Color::Black;
	idleColor.a = 100;

	mouseOverColor = Color::Green;
	mouseOverColor.a = 100;

	show = false;
	//UpdateRectDimensions();

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
	bounds.left = pos.x - boxSize / 2.f;
	bounds.top = pos.y - boxSize / 2.f;
	bounds.width = boxSize;
	bounds.height = boxSize;
}

void ChooseRect::SetShown(bool s)
{
	if (!s && show )
	{
		SetRectCenter(quad, 0, 0, Vector2f(0, 0));
	}
	else if (s && !show)
	{
		SetRectCenter(quad, boxSize, boxSize, pos);
		SetSize(boxSize);
	}
	show = s;
}

void ChooseRect::SetActive(bool a)
{
	active = a;
}

bool ChooseRect::Update()
{
	Vector2i mousePos = panel->GetMousePos();
	EditSession *edit = EditSession::GetSession();
	
	if (panel->IsMouseLeftClicked())
	{
		if (bounds.contains(mousePos))
		{
			edit->ChooseRectEvent(this, E_CLICKED );
			focused = true;
		}
	}
	else if (!panel->IsMouseDownLeft())
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

EnemyChooseRect::EnemyChooseRect(sf::Vertex *v,Panel *p, Vector2f &p_pos, ActorType * p_type,int p_level)
	:ChooseRect( ChooseRectType::ENEMY, v, p, 100, p_pos), actorType( p_type ), level( p_level )
{
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

	actorType->defaultParamsVec[level - 1]->MoveTo(Vector2i(0, 0));
	enemy = actorType->defaultParamsVec[level - 1]->myEnemy;

	enemy->SetActionEditLoop();
	enemy->UpdateFromEditParams(0);	
}

void EnemyChooseRect::SetSize(float s)
{
	ChooseRect::SetSize(s);

	float test;
	FloatRect aabb = enemy->GetAABB();
	float max = std::max(aabb.height, aabb.width);
	test = max / boxSize;
	view.setCenter(Vector2f(960 * test - pos.x * test, 540 * test - pos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
	view.setSize(Vector2f(1920 * test, 1080 * test));
}

void EnemyChooseRect::Draw(RenderTarget *target)
{
	if (show)
	{
		sf::View oldView = target->getView();
		target->setView(view);

		enemy->Draw(target);

		target->setView(oldView);
	}
}

void EnemyChooseRect::UpdateSprite(int frameUpdate)
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




ImageChooseRect::ImageChooseRect(sf::Vertex *v, Panel *p, Vector2f &p_pos, Tileset *p_ts,
	int p_tileIndex )
	:ChooseRect(ChooseRectType::IMAGE, v, p, 100, p_pos), ts( p_ts ), tileIndex( p_tileIndex )
{
	ts->SetSpriteTexture(spr);
	ts->SetSubRect(spr, tileIndex);
	spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);
}

void ImageChooseRect::SetSize(float s)
{
	ChooseRect::SetSize(s);

	float test;
	FloatRect aabb = spr.getGlobalBounds();
	float max = std::max(aabb.height, aabb.width) * 1.2f; //.8 to give the box a little room
	test = max / boxSize;
	view.setCenter(Vector2f(960 * test - pos.x * test, 540 * test - pos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
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