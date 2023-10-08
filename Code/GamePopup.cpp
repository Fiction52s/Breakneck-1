#include "GamePopup.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

GamePopup::GamePopup()
{
	MainMenu *mainMenu = MainMenu::GetInstance();

	//size is set in setText
	size = Vector2f(500, 300);

	SetRectColor(popupBGQuad, Color::Black);

	optionText.setFont(mainMenu->arial);
	optionText.setCharacterSize(40);
	optionText.setFillColor(Color::White);
	optionText.setString("HELLO");

	yesText.setFont(mainMenu->arial);
	yesText.setCharacterSize(40);
	yesText.setFillColor(Color::White);
	yesText.setString("YES");
	yesText.setOrigin(yesText.getLocalBounds().left + yesText.getLocalBounds().width / 2,
		yesText.getLocalBounds().top + yesText.getLocalBounds().height / 2);

	okText.setFont(mainMenu->arial);
	okText.setCharacterSize(40);
	okText.setFillColor(Color::White);
	okText.setString("OK");
	okText.setOrigin(yesText.getLocalBounds().left + okText.getLocalBounds().width / 2,
		okText.getLocalBounds().top + okText.getLocalBounds().height / 2);

	noText.setFont(mainMenu->arial);
	noText.setCharacterSize(40);
	noText.setFillColor(Color::White);
	noText.setString("NO");
	noText.setOrigin(noText.getLocalBounds().left + noText.getLocalBounds().width / 2,
		noText.getLocalBounds().top + noText.getLocalBounds().height / 2);

	backText.setFont(mainMenu->arial);
	backText.setCharacterSize(40);
	backText.setFillColor(Color::White);
	backText.setString("BACK");
	backText.setOrigin(backText.getLocalBounds().left + backText.getLocalBounds().width / 2,
		backText.getLocalBounds().top + backText.getLocalBounds().height / 2);


	SetPos(Vector2f(960, 540));

	SetNumOptions(3);
	SetOption(OPTION_YES);
}

void GamePopup::SetPos(sf::Vector2f &pos)
{
	position = pos;
	SetRectCenter(popupBGQuad, size.x, size.y, pos);

	float yOffset = 50;

	Vector2f yesOffset = Vector2f(-200, yOffset);
	Vector2f noOffset = Vector2f(0, yOffset);
	Vector2f backOffset = Vector2f(200, yOffset);

	if (numOptions == 1)
	{
		yesOffset = Vector2f(0, yOffset);
		okText.setPosition(pos + yesOffset);
	}
	else if (numOptions == 2)
	{
		yesOffset = Vector2f(-100, yOffset);
		noOffset = Vector2f(100, yOffset);
	}

	SetRectCenter(yesSelectedQuad, 128, 128, pos + yesOffset);
	yesText.setPosition(pos + yesOffset);

	SetRectCenter(noSelectedQuad, 128, 128, pos + noOffset);
	noText.setPosition(pos + noOffset);

	SetRectCenter(backSelectedQuad, 128, 128, pos + backOffset);
	backText.setPosition(pos + backOffset);


	optionText.setPosition(Vector2f(pos.x, (pos.y - size.y / 2) + 10));
}

int GamePopup::Update(ControllerDualStateQueue *controllerInput)
{
	int oldOption = currentlySelectedOption;

	if (numOptions > 1)
	{
		if (controllerInput->MenuDirPressed_Left())
		{
			currentlySelectedOption--;
			if (currentlySelectedOption < 0)
			{
				currentlySelectedOption = numOptions - 1;//OPTION_BACK;
			}
		}
		if (controllerInput->MenuDirPressed_Right())
		{
			currentlySelectedOption++;
			if (currentlySelectedOption == numOptions)
			{
				currentlySelectedOption = OPTION_YES;
			}
		}
	}


	if (oldOption != currentlySelectedOption)
	{
		SetOption(currentlySelectedOption);
	}

	if (controllerInput->ButtonPressed_A())
	{
		return currentlySelectedOption;
	}
	else if (controllerInput->ButtonPressed_B())
	{
		if (numOptions > 1)
		{
			return OPTION_BACK;
		}
	}


	return OPTION_NOTHING;
}

void GamePopup::SetOption(int op)
{
	Color yesSelectedColor = Color(29, 208, 9);
	Color noSelectedColor = Color::Red;
	Color backSelectorColor = Color::Cyan;
	Color unselectedColor = Color(93, 93, 93);//Color::Green;
	currentlySelectedOption = op;
	if (op == OPTION_YES)
	{
		SetRectColor(yesSelectedQuad, yesSelectedColor);
		SetRectColor(noSelectedQuad, unselectedColor);
		SetRectColor(backSelectedQuad, unselectedColor);
	}
	else if (op == OPTION_NO)
	{
		SetRectColor(yesSelectedQuad, unselectedColor);
		SetRectColor(noSelectedQuad, noSelectedColor);
		SetRectColor(backSelectedQuad, unselectedColor);
	}
	else if (op == OPTION_BACK)
	{
		SetRectColor(yesSelectedQuad, unselectedColor);
		SetRectColor(noSelectedQuad, unselectedColor);
		SetRectColor(backSelectedQuad, backSelectorColor);
	}
}

void GamePopup::SetNumOptions(int n)
{
	SetOption(OPTION_YES);
	numOptions = n;
	SetPos(position);
}

void GamePopup::SetInfo(const std::string &str, int numOptions, int initialOption)
{
	SetText(str);
	SetNumOptions(numOptions);
	SetOption(initialOption);
}

void GamePopup::Draw(sf::RenderTarget *target)
{
	target->draw(popupBGQuad, 4, sf::Quads);
	target->draw(optionText);

	target->draw(yesSelectedQuad, 4, sf::Quads);

	if (numOptions == 1)
	{
		target->draw(okText);
	}
	else
	{
		target->draw(yesText);

		target->draw(noSelectedQuad, 4, sf::Quads);
		target->draw(noText);

		if (numOptions == 3)
		{
			target->draw(backSelectedQuad, 4, sf::Quads);
			target->draw(backText);
		}
	}
}

void GamePopup::SetText(const std::string &str)
{
	optionText.setString(str);
	optionText.setOrigin(optionText.getLocalBounds().left
		+ optionText.getLocalBounds().width / 2, 0);
	size = Vector2f(max(optionText.getGlobalBounds().width + 40.f, 600.f), 300);
	SetPos(position);
}