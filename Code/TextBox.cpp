#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

TextBox::TextBox(const string &n, int posx, int posy, int width_p, int lengthLimit, sf::Font &f, Panel *p, const std::string & initialText = "")
	:PanelMember(p), pos(posx, posy), width(width_p), maxLength(lengthLimit), clickedDown(false), name(n)
{
	numbersOnly = false;
	focused = false;
	leftBorder = 3;
	verticalBorder = 10;
	characterHeight = 20;

	cursor.setString("|");
	cursor.setFont(f);
	cursor.setFillColor(Color::Red);
	cursor.setCharacterSize(characterHeight);

	text.setFont(f);
	text.setFillColor(Color::Black);
	text.setCharacterSize(characterHeight);
	text.setPosition(pos.x + leftBorder, pos.y);

	SetString(initialText);
	SetCursorIndex(initialText.length());

	size = Vector2i(width, characterHeight + verticalBorder);
}

void TextBox::SetString(const std::string &str)
{
	text.setString(str);
}

std::string TextBox::GetString()
{
	return text.getString();
}

void TextBox::Deactivate()
{
	//focused = false;
	clickedDown = false;
}

void TextBox::SetNumbersOnly(bool b)
{
	numbersOnly = b;
}

void TextBox::SetCursorIndex(int index)
{
	cursorIndex = index;
	cursor.setPosition(pos.x + text.getLocalBounds().width + leftBorder, pos.y);
	//cursor.setPosition(text.getLocalBounds().width + leftBorder, pos.y);
}

void TextBox::SetCursorIndex(Vector2i &mousePos)
{
	//do this later

	/*int stringLength = text.getString().getSize();
	Text textCopy( text );
	Text temp( text );
	for( int i = 0; i < stringLength; ++i )
	{
	temp.setString( text.getString().substring(

	}*/
}

void TextBox::SendKey(Keyboard::Key k, bool shift)
{
	char c = 0;
	switch (k)
	{
	case Keyboard::A:
		c = 'a';
		break;
	case Keyboard::B:
		c = 'b';
		break;
	case Keyboard::C:
		c = 'c';
		break;
	case Keyboard::D:
		c = 'd';
		break;
	case Keyboard::E:
		c = 'e';
		break;
	case Keyboard::F:
		c = 'f';
		break;
	case Keyboard::G:
		c = 'g';
		break;
	case Keyboard::H:
		c = 'h';
		break;
	case Keyboard::I:
		c = 'i';
		break;
	case Keyboard::J:
		c = 'j';
		break;
	case Keyboard::K:
		c = 'k';
		break;
	case Keyboard::L:
		c = 'l';
		break;
	case Keyboard::M:
		c = 'm';
		break;
	case Keyboard::N:
		c = 'n';
		break;
	case Keyboard::O:
		c = 'o';
		break;
	case Keyboard::P:
		c = 'p';
		break;
	case Keyboard::Q:
		c = 'q';
		break;
	case Keyboard::R:
		c = 'r';
		break;
	case Keyboard::S:
		c = 's';
		break;
	case Keyboard::T:
		c = 't';
		break;
	case Keyboard::U:
		c = 'u';
		break;
	case Keyboard::V:
		c = 'v';
		break;
	case Keyboard::W:
		c = 'w';
		break;
	case Keyboard::X:
		c = 'x';
		break;
	case Keyboard::Y:
		c = 'y';
		break;
	case Keyboard::Z:
		c = 'z';
		break;
	case Keyboard::Space:
		c = ' ';
		break;
	case Keyboard::Num0:
	case Keyboard::Numpad0:
		c = '0';
		break;
	case Keyboard::Num1:
	case Keyboard::Numpad1:
		c = '1';
		break;
	case Keyboard::Num2:
	case Keyboard::Numpad2:
		c = '2';
		break;
	case Keyboard::Num3:
	case Keyboard::Numpad3:
		c = '3';
		break;
	case Keyboard::Num4:
	case Keyboard::Numpad4:
		c = '4';
		break;
	case Keyboard::Num5:
	case Keyboard::Numpad5:
		c = '5';
		break;
	case Keyboard::Num6:
	case Keyboard::Numpad6:
		c = '6';
		break;
	case Keyboard::Num7:
	case Keyboard::Numpad7:
		c = '7';
		break;
	case Keyboard::Num8:
	case Keyboard::Numpad8:
		c = '8';
		break;
	case Keyboard::Num9:
	case Keyboard::Numpad9:
		c = '9';
		break;
	case Keyboard::Dash:
		c = '-';
		break;
	case Keyboard::Period:
		c = '.';
		break;
	case Keyboard::Comma:
		c = ',';
		break;
	case Keyboard::BackSpace:
	{
		//text.setString( text.getString().substring( 0, cursorIndex ) + text.getString().substring( cursorIndex + 1 ) );
		cursorIndex -= 1;

		if (cursorIndex < 0)
			cursorIndex = 0;
		else
		{

			sf::String s = text.getString();
			if (s.getSize() > 0)
			{
				s.erase(cursorIndex);
				text.setString(s);
			}
		}

		break;
	}
	case Keyboard::Left:
		cursorIndex -= 1;
		if (cursorIndex < 0)
			cursorIndex = 0;
		break;
	case Keyboard::Right:
		if (cursorIndex < text.getString().getSize())
			cursorIndex += 1;
		break;
	case Keyboard::BackSlash:
		c = '\\';
		break;

	}

	if (c != 0 && text.getString().getSize() < maxLength)
	{
		if (numbersOnly && !(c >= '0' && c <= '9'))
		{
			return;
		}

		if (shift && c >= 'a' && c <= 'z')
		{
			c -= 32;
		}
		else if (shift && c == '-')
		{
			c = '_';
		}
		sf::String s = text.getString();

		s.insert(cursorIndex, sf::String(c));
		text.setString(s);
		cursorIndex++;
	}

	sf::Text test;
	test = text;
	test.setString(test.getString().substring(0, cursorIndex));
	cursor.setPosition(pos.x + test.getLocalBounds().width, pos.y);
}

bool TextBox::MouseUpdate()
{
	if (hidden)
		return false;

	sf::Vector2i mousePos = panel->GetMousePos();
	sf::Rect<int> r(pos.x, pos.y, width, characterHeight + verticalBorder);

	bool containsMouse = r.contains(mousePos);
	UpdateToolTip(containsMouse);

	if (MOUSE.IsMouseDownLeft())
	{
		if (containsMouse)
		{
			clickedDown = true;
		}
		else
		{
			clickedDown = false;
		}
	}
	else
	{
		if (containsMouse && clickedDown)
		{
			clickedDown = false;

			//need to make it so that if you click a letter the cursor goes to the left of it. too lazy for now.

			/*int textLength = text.getString().getSize();

			sf::Text tempText;
			tempText = text;
			tempText.setString( text.getString().substring( 0, 1 ) );

			sf::Rect<int> first( pos.x, pos.y, tempText.getLocalBounds().width / 2 , characterHeight + verticalBorder );
			if( first.contains( sf::Vector2i( posx, posy ) ) )
			{
			cursorIndex = 0;
			cursor.setPosition( pos.x, pos.y);
			}*/

			//if( textLength > 1 )
			//{
			//	int startX = 0;
			//	for( int i = 1; i <= textLength; ++i )
			//	{
			//		tempText.setString( text.getString().substring( 0, i );
			//		 //= tempText.getLocalBounds().left + tempText.getLocalBounds().width;

			//		//tempText.setString( text.getString().substring( i-1, 2 ) );
			//		sf::Rect<int> temp( pos.x + startX, pos.y, tempText.getLocalBounds().width / 2 , characterHeight + verticalBorder );
			//		if( temp.contains( sf::Vector2i( posx, posy ) ) )
			//		{

			//		}
			//	}
			//}

			//SetCursorIndex( pos );
			cursor.setPosition(pos.x + text.getLocalBounds().width + leftBorder, pos.y);
			cursorIndex = text.getString().getSize();
			return true;
		}
		else
		{
			clickedDown = false;
		}
	}

	return false;
}

void TextBox::Draw(sf::RenderTarget *target)
{
	if (hidden)
		return;
	sf::RectangleShape rs;
	//rs.setSize( Vector2f( 300, characterHeight + verticalBorder) );


	rs.setSize(Vector2f(size));

	rs.setFillColor(Color::White);
	rs.setPosition(pos.x, pos.y);

	target->draw(rs);

	if (focused)
	{
		target->draw(cursor);
	}
	target->draw(text);
}