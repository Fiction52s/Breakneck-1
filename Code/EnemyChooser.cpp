#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"

using namespace std;
using namespace sf;

ChooseEnemyRect::ChooseEnemyRect(EnemyChooser *eChooser, int p_quadIndex, ActorType *aType)
	:chooser( eChooser ), quadIndex( p_quadIndex ), actorType( aType )
{
	boxSize = 100;
	pos = Vector2f(400 + quadIndex * boxSize * 1.5, boxSize / 2 + 40);
	
	idleColor = Color::Blue;
	idleColor.a = 100;

	mouseOverColor = Color::Green;
	mouseOverColor.a = 100;
	
	sf::Vertex *quad = GetQuad();

	SetRectColor(quad, idleColor );
	SetRectCenter(quad, boxSize, boxSize, pos);

	bounds.left = pos.x - boxSize / 2.f;
	bounds.top = pos.y - boxSize / 2.f;
	bounds.width = boxSize;
	bounds.height = boxSize;
	
	actorType->defaultParams->MoveTo(Vector2i(0, 0));

	enemy = actorType->defaultParams->myEnemy;
	enemy->SetActionEditLoop();
	enemy->UpdateFromEditParams(0);

	float test = 2.f;

	focused = false;

	FloatRect aabb = enemy->GetAABB();
	float max = std::max(aabb.height, aabb.width);
	test = max / boxSize;
	view.setCenter(Vector2f(960 * test - pos.x * test, 540 * test - pos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
	view.setSize(Vector2f(1920 * test, 1080 * test));
}

bool ChooseEnemyRect::Update(bool mouseDown, int posx, int posy)
{
	if (bounds.contains(Vector2i(posx, posy)))
	{
		focused = true;
		SetRectColor(GetQuad(), mouseOverColor);
	}
	else
	{
		focused = false;
		SetRectColor(GetQuad(), idleColor);
	}
	return true;
}

sf::Vertex * ChooseEnemyRect::GetQuad()
{
	return chooser->allQuads + quadIndex * 4;
}

void ChooseEnemyRect::Draw(RenderTarget *target)
{
	sf::View oldView = target->getView();
	target->setView(view);

	enemy->Draw(target);

	target->setView(oldView);
}

void ChooseEnemyRect::UpdateSprite(int frameUpdate)
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

EnemyChooser::EnemyChooser( std::map<std::string, ActorType*> &types, Panel *p)
	:active(true), owner(p)
{
	numEnemies = types.size() - 1;

	chooseRects.reserve(numEnemies);
	allQuads = new Vertex[numEnemies * 4];

	int i = 0;
	for (auto it = types.begin(); it != types.end(); ++it, ++i)
	{
		if ((*it).first == "player")
		{
			continue;
		}
		chooseRects.push_back(ChooseEnemyRect( this, i, (*it).second));
	}
}

void EnemyChooser::UpdateSprites(int frameUpdate)
{
	for (int i = 0; i < numEnemies; ++i)
	{
		chooseRects[i].UpdateSprite(frameUpdate);
	}
}

EnemyChooser::~EnemyChooser()
{
	delete[] allQuads;
}

//void EnemyChooser::Set(int xi, int yi, Sprite s, const std::string &name)
//{
//	icons[xi][yi] = s;
//	icons[xi][yi].setPosition(xi * tileSizeX, yi * tileSizeY);
//	names[xi][yi] = name;
//}

void EnemyChooser::Draw(sf::RenderTarget *target)
{
	target->draw(allQuads, numEnemies * 4, sf::Quads);

	for (int i = 0; i < numEnemies; ++i)
	{
		chooseRects[i].Draw(target);
	}
}

//returns true if a selection has been made
bool EnemyChooser::Update(bool mouseDown, int posx, int posy)
{
	for (int i = 0; i < numEnemies; ++i)
	{
		chooseRects[i].Update(mouseDown, posx, posy);
	}
	//cout << "update: " << posx << ", " << posy << endl;
	//if (!active)
	//{
	//	return false;
	//	//assert( false && "trying to update inactive grid selector" );
	//}
	//if (mouseDown)
	//{
	//	sf::Rect<int> r(pos.x, pos.y, xSize * tileSizeX, ySize * tileSizeY);
	//	if (r.contains(sf::Vector2i(posx, posy)))
	//	{
	//		focusX = (posx - pos.x) / tileSizeX;
	//		focusY = (posy - pos.y) / tileSizeY;
	//		cout << "contains index: " << focusX << ", " << focusY << endl;
	//	}
	//	else
	//	{
	//		focusX = -1;
	//		focusY = -1;
	//	}
	//}
	//else
	//{
	//	sf::Rect<int> r(pos.x, pos.y, xSize * tileSizeX, ySize * tileSizeY);
	//	if (r.contains(sf::Vector2i(posx, posy)))
	//	{
	//		int tempX = (posx - pos.x) / tileSizeX;
	//		int tempY = (posy - pos.y) / tileSizeY;
	//		if (tempX == focusX && tempY == focusY)
	//		{
	//			selectedX = tempX;
	//			selectedY = tempY;
	//			cout << "tempX: " << tempX << ", tempY: " << tempY << endl;
	//			owner->SendEvent(this, names[tempX][tempY]);//->EnemyChooserCallback( this, names[tempX][tempY] );
	//			return true;
	//			//		cout << "success!" << endl;
	//		}
	//		else
	//		{
	//			focusX = -1;
	//			focusY = -1;
	//		}
	//		//cout << "contains index: " << posx / tileSizeX << ", " << posy / tileSizeY << endl;		

	//	}
	//	else
	//	{
	//		//	cout << "doesn't contain!" << endl;
	//		//	cout << "pos: " << posx << ", " << posy << endl;
	//		focusX = -1;
	//		focusY = -1;
	//	}
	//}

	return false;
}