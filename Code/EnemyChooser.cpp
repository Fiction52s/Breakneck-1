#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"

using namespace std;
using namespace sf;

ChooseEnemyRect::ChooseEnemyRect(sf::Vertex *p_quad)
{
	boxSize = Vector2i(64, 64);
	pos = Vector2f(400, 600);
	float test = 2.f;
	view.setCenter(Vector2f(960 * test - pos.x * test, 540 * test - pos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
	view.setSize(Vector2f(1920 * test, 1080 * test));
	quad = p_quad;

	SetRectColor(quad, Color(Color::Blue));
	SetRectCenter(quad, boxSize.x, boxSize.y, pos);
}

void ChooseEnemyRect::Draw(RenderTarget *target)
{
	sf::CircleShape cs;
	cs.setFillColor(Color::Red);
	cs.setRadius(boxSize.x / 2);
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
	cs.setPosition(Vector2f(0,0));

	sf::View oldView = target->getView();
	target->setView(view);

	target->draw(cs);

	target->setView(oldView);
}

EnemyChooser::EnemyChooser(int p_numEnemies, Panel *p)
	:numEnemies(p_numEnemies), active(true), owner(p)
{
	chooseRects.reserve(numEnemies);
	allQuads = new Vertex[numEnemies * 4];

	for (int i = 0; i < numEnemies; ++i)
	{
		chooseRects.push_back(ChooseEnemyRect(allQuads + i * 4));
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