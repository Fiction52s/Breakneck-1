#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"
#include "UIMouse.h"

using namespace sf;
using namespace std;

TextBox::TextBox(const string &n, int posx, int posy, int rows, int cols, int charHeight, int lengthLimit, sf::Font &f, Panel *p, const std::string & initialText = "")
	:PanelMember(p), pos(posx, posy), maxLength(lengthLimit), clickedDown(false), name(n)
{
	numbersOnly = false;
	focused = false;
	leftBorder = 3;
	verticalBorder = 0;
	characterHeight = 20;//charHeight

	maxRows = rows;
	maxCols = cols;

	lineSpacing = f.getLineSpacing(characterHeight);

	cursor.setString("|");
	cursor.setFont(f);
	cursor.setFillColor(Color::Red);
	cursor.setCharacterSize(characterHeight);

	text.setFont(f);
	text.setFillColor(Color::Black);
	text.setCharacterSize(characterHeight);
	text.setPosition(pos.x + leftBorder, pos.y);

	testText.setFont(f);
	testText.setFillColor(Color::Black);
	testText.setCharacterSize(characterHeight);
	testText.setPosition(pos.x + leftBorder, pos.y);

	char *ws = new char[maxCols+1];
	for (int i = 0; i < maxCols; ++i)
	{
		ws[i] = 'W';
	}
	ws[maxCols] = '\0';

	testText.setString(ws);

	delete[] ws;

	width = testText.getGlobalBounds().width + leftBorder * 2;
	height = maxRows * lineSpacing;

	SetString(initialText);
	SetCursorIndex(initialText.length());

	size = Vector2i(width, height + verticalBorder);
}

TextBox::~TextBox()
{
}

void TextBox::SetString(const std::string &str)
{
	text.setString(str);

	int strLen = str.length();

	lineStartIndexes.clear();
	lineStartIndexes.push_back(0);
	for (int i = 0; i < strLen; ++i)
	{
		if (str.at(i) == '\n')
		{
			lineStartIndexes.push_back(i+1);
		}
	}

	widths.resize(strLen + 1);

	int lineIndex = 0;
	int startIndex = 0;
	for (int i = 0; i <= strLen; ++i)
	{
		startIndex = lineStartIndexes[lineIndex];
		testText.setString(text.getString().substring(startIndex, i - startIndex));
		widths[i] = testText.getGlobalBounds().width;//testText.getLocalBounds().width;

		if (i < strLen)
		{
			if (str.at(i) == '\n')
			{
				lineIndex++;

				if (lineIndex == lineStartIndexes.size())
				{
					break;
				}
			}
		}	
	}
}

int TextBox::GetIndexRow(int index)
{
	int numLines = lineStartIndexes.size();

	for (int i = 0; i < numLines - 1; ++i)
	{
		if (index < lineStartIndexes[i + 1])
		{
			return i;
		}
	}

	return numLines - 1;
}

int TextBox::GetIndexCol(int index)
{
	return 0;
}

std::string TextBox::GetString()
{
	return text.getString();
}

void TextBox::Deactivate()
{
	//focused = false; //recently turned this line back on
	clickedDown = false;
}

void TextBox::SetNumbersOnly(bool b)
{
	numbersOnly = b;
}

void TextBox::SetCursorIndex(int index)
{
	cursorIndex = index;

	if (cursorIndex < 0)
		cursorIndex = 0;

	int strLen = text.getString().getSize();
	if (cursorIndex > strLen)
		cursorIndex = strLen;

	

	int indexRow = GetIndexRow(cursorIndex);
	//int indexCol = GetIndexCol(cursorIndex);

	//the -3 is just to sliiightly make things look nicer
	cursor.setPosition(pos.x + widths[cursorIndex] + leftBorder, pos.y + indexRow * lineSpacing - 3);
	//cursor.setPosition(text.getLocalBounds().width + leftBorder, pos.y);
}

void TextBox::SetCursorIndex(Vector2i &localMousePos)
{
	int strLen = text.getString().getSize();

	int chosenIndex = -1;
	int middle;

	int currRow = 0;
	int numLines = lineStartIndexes.size();

	int row = localMousePos.y / (lineSpacing);

	if (row < 0)
		row = 0;

	if (row >= numLines)
		row = numLines - 1;

	int rowStart = lineStartIndexes[row];
	int rowEnd;
	if (row == numLines -1)
	{
		rowEnd = strLen-1;
	}
	else
	{
		rowEnd = lineStartIndexes[row + 1]-1;
	}
	//for (int i = 0; i < numLines; ++i)
	//{

	//}

	for (int i = rowStart; i <= rowEnd; ++i)
	{
		//middle = (widths[i + 1] + widths[i]) / 2;
		//if( localMousePos.x < middle )
		if (localMousePos.x >= widths[i] && localMousePos.x <= widths[i + 1] )
		{
			chosenIndex = i;
			break;
		}
	}

	if (row == numLines - 1)
	{
		if (localMousePos.x >= widths[strLen])
			//if (localMousePos.x >= middle)
		{
			//cout << "failed" << endl;
			chosenIndex = strLen + 1;
		}
	}
	else
	{
		if (localMousePos.x >= widths[rowEnd])
			//if (localMousePos.x >= middle)
		{
			//cout << "failed" << endl;
			chosenIndex = rowEnd;
		}
	}
	

	if (chosenIndex == -1)
	{
		chosenIndex = 0;
	}

	SetCursorIndex(chosenIndex);
	//cursorIndex = chosenIndex;
	//cursor.setPosition(pos.x + widths[chosenIndex] + leftBorder, pos.y);
}

int TextBox::GetLineWidth(int lineNumber)
{
	string str = text.getString();

	int len = str.length();

	int currLine = 0;

	int startChar = 0;
	int endChar = -1;

	for (int i = 0; i <= len; ++i)
	{
		if (i == len)
		{
			endChar = i;
			break;
		}
		else if (str.at(i) == '\n')
		{
			if (lineNumber == currLine)
			{
				endChar = i;
				break;
			}
			else
			{
				startChar = i + 1;
				currLine++;
			}
		}
	}

	string sub = str.substr(startChar, endChar - startChar);
	testText.setString(sub);

	return testText.getGlobalBounds().width;
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

		if (cursorIndex > 0)
		{
			sf::String s = text.getString();
			if (s.getSize() > 0)
			{
				s.erase(cursorIndex-1);
				SetString(s);
				SetCursorIndex(cursorIndex - 1);
			}
		}

		break;
	}
	case Keyboard::Enter:
	{
		int numLines = lineStartIndexes.size();
		if (numLines < maxRows)
		{
			c = '\n';
		}
		break;
	}
	case Keyboard::Left:
		SetCursorIndex(cursorIndex - 1);
		break;
	case Keyboard::Right:
		SetCursorIndex(cursorIndex + 1);
		break;
	case Keyboard::Up:
	{
		int row = GetIndexRow(cursorIndex);

		if (row == 0)
		{
			break;
		}
		else
		{
			int startIndex = lineStartIndexes[row];
			int colIndex = cursorIndex - startIndex;
			int prevRowStart = lineStartIndexes[row - 1];

			int prevRowLength = startIndex - prevRowStart - 1;

			if (colIndex > prevRowLength)
			{
				colIndex = prevRowLength;
			}

			SetCursorIndex(prevRowStart + colIndex);
			break;
		}
	}
	case Keyboard::Down:
	{
		int row = GetIndexRow(cursorIndex);
		int numLines = lineStartIndexes.size();

		if (row == numLines -1)
		{
			break;
		}
		else
		{
			int strLen = text.getString().getSize();

			int startIndex = lineStartIndexes[row];
			int colIndex = cursorIndex - startIndex;

			int nextRowStart = lineStartIndexes[row + 1];

			int nextnextRowStart;
			if (row < numLines - 2)
			{
				nextnextRowStart = lineStartIndexes[row + 2];
			}
			else
			{
				nextnextRowStart = strLen;
			}

			int newIndex = nextRowStart + colIndex;

			int nextRowLength = nextnextRowStart - nextRowStart; //-1

			if (colIndex > nextRowLength)
			{
				colIndex = nextRowLength;
			}

			SetCursorIndex(nextRowStart + colIndex);
			break;
		}
		break;
	}
	case Keyboard::Home:
	{
		int row = GetIndexRow(cursorIndex);

		int startIndex = lineStartIndexes[row];

		SetCursorIndex(startIndex);
			
		break;
	}
	case Keyboard::End:
	{
		int row = GetIndexRow(cursorIndex);
		int numLines = lineStartIndexes.size();
		

		if (row == numLines - 1)
		{
			int strLen = text.getString().getSize();
			SetCursorIndex(strLen);
		}
		else
		{
			int nextStartIndex = lineStartIndexes[row + 1];
			SetCursorIndex(nextStartIndex - 1);
		}
		break;
	}
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

		int row = GetIndexRow(cursorIndex);
		int rowStart = lineStartIndexes[row];

		int rowEnd;
		if (row == lineStartIndexes.size() - 1)
		{
			rowEnd = text.getString().getSize();
		}
		else
		{
			int nextStart = lineStartIndexes[row + 1];
			rowEnd = nextStart - 1;
		}

		int rowWidth = rowEnd - rowStart;

		sf::String s = text.getString();

		string charStr(1, c);
		if (rowWidth >= maxCols)
		{
			charStr = "\n" + charStr;
		}

		s.insert(cursorIndex, sf::String(charStr));
		SetString(s);
		SetCursorIndex(cursorIndex + charStr.size());
		//cursorIndex++;
	}

	//testText.setString(text.getString().substring(0, cursorIndex));
	//cursor.setPosition(pos.x + testText.getLocalBounds().width, pos.y);
}

bool TextBox::MouseUpdate()
{
	if (hidden)
		return false;

	sf::Vector2i mousePos = panel->GetMousePos();
	sf::Vector2i localMousePos = panel->GetMousePos() - pos;
	sf::Rect<int> r(pos.x, pos.y, size.x, size.y);

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
			focused = false;
			clickedDown = false;
		}
	}
	else
	{
		if (containsMouse && clickedDown)
		{
			clickedDown = false;

			SetCursorIndex(localMousePos);

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