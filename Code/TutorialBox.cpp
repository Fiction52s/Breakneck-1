#include "TutorialBox.h"
#include "Session.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

TutorialBox::TutorialBox()
{
	sess = Session::GetSession();

	text.setFont(sess->mainMenu->arial);
	text.setCharacterSize(48);
	text.setFillColor(Color::White);

	SetRectColor(quad, Color(0, 0, 0, 100));
}

bool TutorialBox::CalcButtonPos( std::string &startString, 
	const std::string &buttonStr, sf::Vector2f &buttonPos )
{
	string s = startString;

	vector<size_t> buttonIndexes;

	size_t found = startString.find(buttonStr, 0);

	if (found == string::npos)
	{
		return false;
	}

	string stringBeforeFound = startString.substr(0, found);
	size_t newLineFound = stringBeforeFound.find('\n', 0);

	int buttonX = 0;
	if (newLineFound != string::npos)
	{
		string currLineStr = stringBeforeFound.substr(newLineFound);
		text.setString(currLineStr);
		buttonX = text.getGlobalBounds().width;

	}
	else
	{
		text.setString(stringBeforeFound);
		buttonX = text.getGlobalBounds().width;
	}


	
	startString.replace(found, buttonStr.length(), "     ");


	int numLines = 0;
	for (int i = 0; i < stringBeforeFound.length(); ++i)
	{
		if (stringBeforeFound.at(i) == '\n')
		{
			numLines++;
		}
	}

	int buttonY = numLines * text.getCharacterSize();

	buttonPos = Vector2f(buttonX, buttonY);

	return true;

	//text.setString(s);
}

struct ButtonInfo
{
	ButtonInfo()
	{
		buttonType = -1;
	}
	ButtonInfo(int p_buttonType, Vector2f &p_pos)
	{
		buttonType = p_buttonType;
		pos = p_pos;
	}
	int buttonType;
	Vector2f pos;
	
};

void TutorialBox::SetText(const std::string &str)
{
	ClearButtons();

	string s = str;
	std::replace(s.begin(), s.end(), '\\', '\n');

	std::vector<string> inputStrings = { "DASH", "JUMP" };

	float buttonSize = 48;
	float rectBuffer = 30;

	FloatRect globalBounds;

	vector<ButtonInfo> buttonInfos;

	for (int i = 0; i < inputStrings.size();)
	{
		Vector2f testPos;
		bool buttonTest = CalcButtonPos(s, inputStrings[i], testPos);

		text.setString(s);
		text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2,
			text.getLocalBounds().top + text.getLocalBounds().height / 2);

		if (buttonTest)
		{
			testPos.y -= rectBuffer / 4;

			buttonInfos.push_back(ButtonInfo(i, testPos));
		}
		else
		{
			++i;
		}
	}
	globalBounds = text.getGlobalBounds();

	for (int i = 0; i < buttonInfos.size(); ++i)
	{
		SetRectCenter(buttonQuad + i * 4, buttonSize, buttonSize,
			Vector2f(globalBounds.left, globalBounds.top) + buttonInfos[i].pos + Vector2f(buttonSize / 2, buttonSize / 2));

		if (buttonInfos[i].buttonType == 0)
		{
			SetRectColor(buttonQuad + i * 4, Color::Red);
		}
		else
		{
			SetRectColor(buttonQuad + i * 4, Color::Green);
		}
	}
	
	text.setString(s);
	text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2,
		text.getLocalBounds().top + text.getLocalBounds().height / 2);

	SetRectCenter(quad, globalBounds.width + rectBuffer, globalBounds.height + rectBuffer,
		Vector2f(globalBounds.left + globalBounds.width / 2,
		globalBounds.top + globalBounds.height / 2 ));
}

void TutorialBox::SetCenterPos(sf::Vector2f &pos)
{
	text.setPosition(pos);

	SetRectCenter(quad, text.getGlobalBounds().width + 20, text.getGlobalBounds().height + 20, Vector2f());
}

void TutorialBox::ClearButtons()
{
	for (int i = 0; i < MAX_BUTTONS; ++i)
	{
		ClearRect(buttonQuad + i * 4);
	}
	
}

void TutorialBox::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);
	target->draw(text);
	target->draw(buttonQuad, 4 * MAX_BUTTONS, sf::Quads);
}