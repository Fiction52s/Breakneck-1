#include "MainMenu.h"

using namespace sf;
using namespace std;

LoadingMapProgressDisplay::LoadingMapProgressDisplay(MainMenu *p_mainMenu,
	Vector2f &topLeft)
	:mainMenu(p_mainMenu)
{
	text = new Text[NUM_LOAD_THREADS];
	currString = new string[NUM_LOAD_THREADS];
	for (int i = 0; i < NUM_LOAD_THREADS; ++i)
	{
		sf::Text &t = text[i];
		t.setCharacterSize(20);
		t.setFont(mainMenu->arial);
		t.setPosition(topLeft + Vector2f(0, 30 * i + 20 * i));
		t.setFillColor(Color::Red);
	}
}

LoadingMapProgressDisplay::~LoadingMapProgressDisplay()
{
	delete[] text;
	delete[] currString;
}

void LoadingMapProgressDisplay::Reset()
{
	for (int i = 0; i < NUM_LOAD_THREADS; ++i)
	{
		text[i].setString("");
		currString[i] = "";
	}
}



void LoadingMapProgressDisplay::SetProgressString(const std::string & str,
	int threadIndex)
{
	stringLock.lock();

	currString[threadIndex] = str;

	stringLock.unlock();
}

void LoadingMapProgressDisplay::UpdateText()
{
	string temp;
	for (int i = 0; i < NUM_LOAD_THREADS; ++i)
	{
		stringLock.lock();
		temp.assign(currString[i]);
		stringLock.unlock();

		sf::Text &t = text[i];
		if (temp != t.getString().toAnsiString())
		{
			t.setString(temp);
		}
	}
}

void LoadingMapProgressDisplay::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < NUM_LOAD_THREADS; ++i)
	{
		target->draw(text[i]);
	}
}