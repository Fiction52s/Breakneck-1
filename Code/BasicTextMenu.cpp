#include "BasicTextMenu.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

BasicTextMenu::BasicTextMenu(std::vector<std::string> &options)
{
	mainMenu = MainMenu::GetInstance();

	SetRectColor(bgQuad, Color(50, 50, 50, 230));
	SetRectTopLeft(bgQuad, 1920, 1080, Vector2f(0, 0));

	numOptions = options.size();

	int charSize = 48;

	sf::Vector2f center(960, 540);

	int rowHeight = charSize + 10;

	int totalHeight = rowHeight * numOptions;

	Vector2f textStart(center.x, center.y - totalHeight / 2);

	textVec.resize(numOptions);
	for (int i = 0; i < numOptions; ++i)
	{
		textVec[i].setFont(mainMenu->arial);
		textVec[i].setCharacterSize(charSize);
		textVec[i].setString(options[i]);
		auto lb = textVec[i].getLocalBounds();
		textVec[i].setOrigin(lb.left + lb.width / 2, 0);
		textVec[i].setPosition(textStart.x, textStart.y + rowHeight * i);
	}

	unselectedColor = Color(150, 150, 150);
	selectedColor = Color::White;

	Reset();
}

void BasicTextMenu::Reset()
{
	SetSelected(0);
}

int BasicTextMenu::GetSelectedIndex()
{
	return selectedIndex;
}

//returns -1 when nothing is pressed, returns the index if something is pressed
int BasicTextMenu::Update()
{
	if (CONTROLLERS.DirPressed_Down() )
	{
		if (selectedIndex < numOptions - 1 )
		{
			SetSelected(selectedIndex + 1);
		}
	}
	else if (CONTROLLERS.DirPressed_Up())
	{
		if (selectedIndex > 0)
		{
			SetSelected(selectedIndex - 1);
		}
	}
	else if (CONTROLLERS.ButtonPressed_A())
	{
		return selectedIndex;
	}

	return -1;
}

void BasicTextMenu::SetSelected(int index)
{
	selectedIndex = index;
	for (int i = 0; i < numOptions; ++i)
	{
		textVec[i].setFillColor(unselectedColor);
	}

	textVec[selectedIndex].setFillColor(selectedColor);
}

void BasicTextMenu::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	for (int i = 0; i < numOptions; ++i)
	{
		target->draw(textVec[i]);
	}
}