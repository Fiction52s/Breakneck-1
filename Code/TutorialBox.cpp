#include "TutorialBox.h"
#include "Session.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

TutorialBox::TutorialBox()
{
	charHeight = 48;
	lockedSize = Vector2f();
	quadColor = Color(0, 0, 0, 100);
	rectBuffer = 30;
	textColor = Color::White;
	topLeftMode = false;
}

TutorialBox::TutorialBox( int p_charHeight, sf::Vector2f p_lockedSize, sf::Color p_quadColor, sf::Color p_textColor, float p_rectBuffer)
{
	sess = Session::GetSession();

	charHeight = p_charHeight;
	lockedSize = p_lockedSize;
	quadColor = p_quadColor;
	textColor = p_textColor;
	rectBuffer = p_rectBuffer;
	

	text.setFont(sess->mainMenu->arial);
	text.setCharacterSize(charHeight);
	text.setFillColor(textColor);

	SetRectColor(quad, quadColor);
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
	size_t newLineFound = stringBeforeFound.find_last_of('\n');

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


	
	startString.replace(found, buttonStr.length(), "    ");


	int numLines = 0;
	for (int i = 0; i < stringBeforeFound.length(); ++i)
	{
		if (stringBeforeFound.at(i) == '\n')
		{
			numLines++;
		}
	}

	int lineSpacing = text.getFont()->getLineSpacing(text.getCharacterSize());
	int textTest = text.getLineSpacing();
	int buttonY = numLines * lineSpacing;//(text.getLineSpacing() + text.getCharacterSize() );

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

	std::vector<string> inputStrings = {  "JUMP", "DASH",
	"ATTACK", "SHIELD", "LEFTWIRE", "RIGHTWIRE", "MAP", "PAUSE"};

	float buttonSize = charHeight;
	//float rectBuffer = 30;

	FloatRect globalBounds;

	vector<ButtonInfo> buttonInfos;

	for (int i = 0; i < inputStrings.size();)
	{
		Vector2f testPos;
		bool buttonTest = CalcButtonPos(s, inputStrings[i], testPos);

		text.setString(s);

		if (topLeftMode)
		{
			text.setOrigin(0, 0);	
		}
		else
		{
			text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2,
				text.getLocalBounds().top + text.getLocalBounds().height / 2);
		}
		

		if (buttonTest)
		{
			//testPos.y -= rectBuffer / 4;

			buttonInfos.push_back(ButtonInfo(i, testPos));
		}
		else
		{
			++i;
		}
	}

	/*text.setString(s);
	text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2,
		text.getLocalBounds().top + text.getLocalBounds().height / 2);*/

	globalBounds = text.getGlobalBounds();

	for (int i = 0; i < buttonInfos.size(); ++i)
	{
		SetRectCenter(buttonQuad + i * 4, buttonSize, buttonSize,
			Vector2f(globalBounds.left, globalBounds.top) + buttonInfos[i].pos + Vector2f(buttonSize / 2, buttonSize / 2));

		ControllerSettings::ButtonType bType = (ControllerSettings::ButtonType)buttonInfos[i].buttonType;
		SetRectSubRect(buttonQuad + i * 4, sess->GetButtonIconTile(0, bType));
	}
	
	Vector2f quadSize;
	bool locked = false;
	if (lockedSize.x != 0 && lockedSize.y != 0)
	{
		quadSize = lockedSize;
		locked = true;
	}
	else
	{
		quadSize = Vector2f(globalBounds.width + rectBuffer, globalBounds.height + rectBuffer);
		locked = false;
	}

	if (topLeftMode)
	{
		//SetRectCenter(quad, quadSize.x, quadSize.y);
	}
	else
	{
		if (!locked)
		{
			SetRectCenter(quad, quadSize.x, quadSize.y,
				Vector2f(globalBounds.left + globalBounds.width / 2,
					globalBounds.top + globalBounds.height / 2));
		}
	}	
}

void TutorialBox::SetCenterPos(sf::Vector2f &pos)
{
	text.setPosition(pos);
	text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2,
		text.getLocalBounds().top + text.getLocalBounds().height / 2);

	Vector2f quadSize;
	if (lockedSize.x != 0 && lockedSize.y != 0)
	{
		quadSize = lockedSize;
	}
	else
	{
		quadSize = Vector2f(text.getGlobalBounds().width + rectBuffer, text.getGlobalBounds().height + rectBuffer);
	}

	SetRectCenter(quad, quadSize.x, quadSize.y, Vector2f());
}

void TutorialBox::SetTopLeft(sf::Vector2f &pos)
{
	text.setOrigin(0, 0);
	text.setPosition(pos + Vector2f( rectBuffer/ 2, rectBuffer / 4 )); //text spacing

	Vector2f quadSize;
	if (lockedSize.x != 0 && lockedSize.y != 0)
	{
		quadSize = lockedSize;
	}
	else
	{
		quadSize = Vector2f(text.getGlobalBounds().width + rectBuffer, text.getGlobalBounds().height + rectBuffer);
	}

	SetRectTopLeft(quad, quadSize.x, quadSize.y, pos);
	/*text.setPosition(pos);

	Vector2f quadSize;
	if (lockedSize.x != 0 && lockedSize.y != 0)
	{
		quadSize = lockedSize;
	}
	else
	{
		quadSize = Vector2f(text.getGlobalBounds().width + rectBuffer, text.getGlobalBounds().height + rectBuffer);
	}

	SetRectCenter(quad, quadSize.x, quadSize.y, Vector2f());*/
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
	target->draw(buttonQuad, 4 * MAX_BUTTONS, sf::Quads, sess->GetButtonIconTileset( 0 )->texture);
}