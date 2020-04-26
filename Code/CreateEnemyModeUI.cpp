#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"

using namespace std;
using namespace sf;


ChooseRectContainer::ChooseRectContainer(Vector2i &pos, Vector2f &p_size)
	:UIMouseUser(pos), size( p_size )
{
	SetRectTopLeft(quad, size.x, size.y, GetFloatPos());
	Color c;
	c = Color::Cyan;
	c.a = 80;
	SetRectColor(quad, c);
}

void ChooseRectContainer::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);
}


CreateEnemyModeUI::CreateEnemyModeUI()
{
	hotbarEnemies.reserve(8);
	libraryEnemiesVec.resize(9);
	for (int i = 0; i < 9; ++i)
	{
		libraryEnemiesVec[i].resize(20);
	}

	edit = EditSession::GetSession();

	int totalHotbarCount = 12;
	int hotbarRectSize = 100;
	int hotbarSpacing = 20;
	int totalHotbarSize = hotbarRectSize * totalHotbarCount + hotbarSpacing * (totalHotbarCount - 1);
	int extraHotbarSpacing = (1920 - totalHotbarSize) / 2;

	topbarCont = new ChooseRectContainer(Vector2i(extraHotbarSpacing, 20), Vector2f(totalHotbarSize, 120));

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

	int numEnemyWorlds = 9;
	int worldSize = 100;
	int worldSpacing = 30;
	int totalWorldSize = worldSize * numEnemyWorlds + worldSpacing * (numEnemyWorlds - 1);

	int extraWorldSpacing = (1920 - totalWorldSize) / 2;

	libCont = new ChooseRectContainer(Vector2i(extraWorldSpacing, 140), Vector2f(totalWorldSize, 400));

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
				allEnemyRects.push_back(EnemyChooseRect(allEnemyQuads + i * 4, libCont,
					Vector2f(0, 0), (*it).second, level));
				++i;
			}
		}
	}


	hotbarQuads = new Vertex[totalHotbarCount * 4];

	

	//activeHotbarSize = //allEnemyRects.size();
	//if (activeHotbarSize > totalHotbarCount)
	//{
	//	activeHotbarSize = totalHotbarCount;
	//}

	for (int i = 0; i < totalHotbarCount; ++i)
	{
		hotbarEnemies.push_back(EnemyChooseRect( hotbarQuads + i * 4, topbarCont,
			Vector2f( 60 + i * ( hotbarRectSize + hotbarSpacing ), 60),
			NULL, 0 ));
		hotbarEnemies[i].SetShown(false);
		hotbarEnemies[i].Init();
	}

	

	worldSelectRects.reserve(numEnemyWorlds);
	Tileset *ts = edit->GetSizedTileset("worldselector_64x64.png");
	worldSelectQuads = new Vertex[numEnemyWorlds * 4];

	

	

	for (int i = 0; i < numEnemyWorlds; ++i)
	{
		worldSelectRects.push_back(ImageChooseRect(worldSelectQuads + i * 4, libCont,
			Vector2f( i * (worldSize + worldSpacing) + 60, 60 ), ts, i));
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
				ecRect->SetPosition(Vector2f(60 + i * 120, 300));
				//ecRect->SetShown(true);
				ecRect->Init();
				++counter;
			}
		}
	}

	activeLibraryWorld = -1;
	show = true;

	UpdateHotbarTypes();
}

CreateEnemyModeUI::~CreateEnemyModeUI()
{
	delete topbarCont;
	delete libCont;
	//delete topbarPanel;
	//delete libraryPanel;

	delete [] allEnemyQuads;
	delete[] worldSelectQuads;
	delete[] hotbarQuads;
}

void CreateEnemyModeUI::UpdateHotbarTypes()
{
	int i = 0;
	for (auto it = edit->recentEnemies.begin(); it != edit->recentEnemies.end(); ++it)
	{
		hotbarEnemies[i].SetType((*it).first, (*it).second);
		hotbarEnemies[i].SetShown(true);
		++i;
	}
	activeHotbarSize = i;
	for (; i < EditSession::MAX_RECENT_ENEMIES; ++i)
	{
		hotbarEnemies[i].SetType(NULL, 0);
	}
}

void CreateEnemyModeUI::SetShown(bool s)
{
	show = s;
	if (!show)
	{
		topbarCont->ResetMouse();
		libCont->ResetMouse();
	}
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
	for (int i = 0; i < activeHotbarSize; ++i)
	{
		if (activeLibraryWorld != hotbarEnemies[i].actorType->info.world )
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

void CreateEnemyModeUI::Update(bool mouseDownL, bool mouseDownR, sf::Vector2i &mPos)
{
	if (!show)
	{
		return;
	}
	topbarCont->Update(mouseDownL, mouseDownR, mPos);
	libCont->Update(mouseDownL, mouseDownR, mPos);

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
	if (!show)
	{
		return;
	}
	sf::View oldView = target->getView();
	target->setView(edit->uiView);


	//topbarPanel->Draw(target);
	//libraryPanel->Draw(target);

	topbarCont->Draw(target);
	libCont->Draw(target);
	//topbarCont->Draw(target);
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




ChooseRect::ChooseRect( ChooseRectType crType, Vertex *v, UIMouseUser *mUser, float size, sf::Vector2f &p_pos)
	:mouseUser( mUser ), quad(v), boxSize( size ), pos( p_pos ), chooseRectType( crType )
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
		SetRectCenter(quad, boxSize, boxSize, GetGlobalPos());
		SetSize(boxSize);
	}
	show = s;
}

sf::Vector2f ChooseRect::GetGlobalPos()
{
	return mouseUser->GetFloatPos() + pos;
}

void ChooseRect::SetActive(bool a)
{
	active = a;
}

bool ChooseRect::Update()
{
	Vector2i mousePos = mouseUser->GetMousePos();//panel->GetMousePos();
	EditSession *edit = EditSession::GetSession();
	
	if (mouseUser->IsMouseLeftClicked())
	{
		if (bounds.contains(mousePos))
		{
			edit->ChooseRectEvent(this, E_CLICKED );
			focused = true;
		}
	}
	else if (!mouseUser->IsMouseDownLeft())
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

EnemyChooseRect::EnemyChooseRect(sf::Vertex *v, UIMouseUser *mUser, Vector2f &p_pos, ActorType * p_type,int p_level)
	:ChooseRect( ChooseRectType::ENEMY, v, mUser, 100, p_pos), level( p_level )
{
	actorType = NULL;
	SetType(p_type, level);
}

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
		actorType->defaultParamsVec[level - 1]->MoveTo(Vector2i(0, 0));
		enemy = actorType->defaultParamsVec[level - 1]->myEnemy;
		enemy->SetActionEditLoop();
		enemy->UpdateFromEditParams(0);
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
		Vector2f truePos = GetGlobalPos();

		float test;
		FloatRect aabb = enemy->GetAABB();
		float max = std::max(aabb.height, aabb.width);
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

		enemy->Draw(target);

		target->setView(oldView);
	}
}

void EnemyChooseRect::UpdateSprite(int frameUpdate)
{
	if (actorType != NULL)
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




ImageChooseRect::ImageChooseRect(sf::Vertex *v, UIMouseUser *mUser, Vector2f &p_pos, Tileset *p_ts,
	int p_tileIndex )
	:ChooseRect(ChooseRectType::IMAGE, v, mUser, 100, p_pos), ts( p_ts ), tileIndex( p_tileIndex )
{
	ts->SetSpriteTexture(spr);
	ts->SetSubRect(spr, tileIndex);
	spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);
}

void ImageChooseRect::SetSize(float s)
{
	ChooseRect::SetSize(s);

	Vector2f truePos = GetGlobalPos();

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


void UIMouseUser::Update(bool mouseDownL,bool mouseDownR,sf::Vector2i &mPos)
{
	lastMouseDownLeft = isMouseDownLeft;
	isMouseDownLeft = mouseDownL;

	lastMouseDownRight = isMouseDownRight;
	isMouseDownRight = mouseDownR;

	mousePos = mPos - position;
}

bool UIMouseUser::IsMouseDownLeft()
{
	return isMouseDownLeft;
}

bool UIMouseUser::IsMouseDownRight()
{
	return isMouseDownRight;
}

bool UIMouseUser::IsMouseLeftClicked()
{
	return isMouseDownLeft && !lastMouseDownLeft;
}

bool UIMouseUser::IsMouseLeftReleased()
{
	return !isMouseDownLeft && lastMouseDownLeft;
}

bool UIMouseUser::IsMouseRightClicked()
{
	return isMouseDownRight && !lastMouseDownRight;
}

bool UIMouseUser::IsMouseRightReleased()
{
	return !isMouseDownRight && lastMouseDownRight;
}

const sf::Vector2i & UIMouseUser::GetMousePos()
{
	return mousePos;
}

void UIMouseUser::ResetMouse()
{
	isMouseDownLeft = false;
	lastMouseDownLeft = false;
	isMouseDownRight = false;
	lastMouseDownRight = false;
}