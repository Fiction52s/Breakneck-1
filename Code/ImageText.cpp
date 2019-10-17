#include "ImageText.h"
#include <assert.h>
#include "GameSession.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

ImageText::ImageText( int p_maxDigits, Tileset *ts_tex )
{
	maxDigits = p_maxDigits;
	ts = ts_tex;
	int numVertices = maxDigits * 4;
	vert = new Vertex[numVertices];
	value = 0;
	
	positionCenter = false;
	numShowZeroes = 0;
	activeDigits = 1;
}

ImageText::~ImageText()
{
	delete[]vert;
}

void ImageText::SetCenter(sf::Vector2f &p_center)
{
	center = p_center;
	positionCenter = true;
	UpdateSprite();
}

void ImageText::SetTopRight(sf::Vector2f &p_topRight)
{
	topRight = p_topRight;
	positionCenter = false;
}

void ImageText::UpdateSprite()
{
	int div = 10;
	int val = value;
	int ind = 0;
	
	while( true )
	{
		int res = val % div;

		IntRect subRect = ts->GetSubRect( res );
		vert[ind*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		vert[ind*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		vert[ind*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		vert[ind*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );

		++ind;

		val /= div;

		if( val == 0 )
		{
			break;
		}
	}

	for(; ind < numShowZeroes; ++ind )
	{
		IntRect subRect = ts->GetSubRect( 0 );
		vert[ind*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		vert[ind*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		vert[ind*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		vert[ind*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
	}

	activeDigits = ind;

	Vector2f currTopRight;
	if (positionCenter)
	{
		currTopRight = Vector2f(center.x + (ts->tileWidth * activeDigits) / 2, center.y - ts->tileHeight / 2);
	}
	else
	{
		currTopRight = topRight;
	}
	

	for (int i = 0; i < activeDigits; ++i)
	{
		vert[i * 4 + 0].position = Vector2f(-i * ts->tileWidth, 0) + currTopRight + Vector2f(-ts->tileWidth, 0);
		vert[i * 4 + 1].position = Vector2f(-i * ts->tileWidth, 0) + currTopRight;
		vert[i * 4 + 2].position = Vector2f(-i * ts->tileWidth, 0) + currTopRight + Vector2f(0, ts->tileHeight);
		vert[i * 4 + 3].position = Vector2f(-i * ts->tileWidth, 0) + currTopRight + Vector2f(-ts->tileWidth, ts->tileHeight);
	}
}

void ImageText::Draw( sf::RenderTarget *target )
{
	target->draw( vert, activeDigits * 4, sf::Quads, ts->texture );
}

void ImageText::ShowZeroes( int numZ )
{
	assert( numZ <= maxDigits && numZ >= 0 );
	numShowZeroes = numZ;
}

void ImageText::SetNumber( int num )
{
	assert( num >= 0 );
	value = num;
}

TimerText::TimerText( Tileset *ts_tex )
	:ImageText(5, ts_tex )
{
	int colonIndex = 5/2;
	IntRect subRect = ts->GetSubRect( 10 );
	vert[colonIndex*4+0].texCoords = Vector2f( subRect.left, subRect.top );
	vert[colonIndex*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
	vert[colonIndex*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
	vert[colonIndex*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
}

void TimerText::UpdateSprite()
{
	int numMinutes = value / 60;
	int numSeconds = value % 60;

	for( int i = 0; i < maxDigits; ++i )
	{
		vert[i*4 + 0].position = Vector2f( -i * ts->tileWidth, 0 ) + topRight + Vector2f( -ts->tileWidth, 0 );
		vert[i*4 + 1].position = Vector2f( -i * ts->tileWidth, 0 ) + topRight;
		vert[i*4 + 2].position = Vector2f( -i * ts->tileWidth, 0 ) + topRight + Vector2f( 0, ts->tileHeight );
		vert[i*4 + 3].position = Vector2f( -i * ts->tileWidth, 0 ) + topRight + Vector2f( -ts->tileWidth, ts->tileHeight );
	}

	int div = 10;
	int val = value;
	int ind = 0;
	
	int numDigit1 = numSeconds / 10;
	int numDigit2 = numSeconds % 10;

	IntRect subRect = ts->GetSubRect( numDigit2 );
	vert[0*4+0].texCoords = Vector2f( subRect.left, subRect.top );
	vert[0*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
	vert[0*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
	vert[0*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );

	subRect = ts->GetSubRect( numDigit1 );
	vert[1*4+0].texCoords = Vector2f( subRect.left, subRect.top );
	vert[1*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
	vert[1*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
	vert[1*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );

	
	numDigit1 = numMinutes / 10;
	numDigit2 = numMinutes % 10;

	subRect = ts->GetSubRect( numDigit2 );
	vert[3*4+0].texCoords = Vector2f( subRect.left, subRect.top );
	vert[3*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
	vert[3*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
	vert[3*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );

	subRect = ts->GetSubRect( numDigit1 );
	vert[4*4+0].texCoords = Vector2f( subRect.left, subRect.top );
	vert[4*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
	vert[4*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
	vert[4*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );

	activeDigits = maxDigits;
}

TextDisp::TextDisp( GameSession *owner, int width, int height, int charSize, int frameLetterWait,
	int p_letterPerShow )
{
	show = false;
	//message = "hello this is a test";
	bgRect.setFillColor(Color( 0, 0, 0, 100 ));
	rectSize = Vector2f(width, height);//Vector2f(670, 220);
	bgRect.setSize(rectSize);
	nextLetterWait = frameLetterWait;

	text.setCharacterSize(charSize);
	text.setFont(owner->mainMenu->consolas);
	text.setFillColor(Color::White);
	
	letterPerShow = p_letterPerShow;

	sectionWait = false;
	//bgRect.setOrigin(bgRect.getLocalBounds().width / 2, bgRect.getLocalBounds().height / 2);
	//Reset()
}

TextDisp::~TextDisp()
{
	delete[]sections;
}

void TextDisp::Load(const std::string &name)
{
	string path = "Resources/Text/";
	string suffix = ".txt";

	ifstream is;
	is.open(path + name + suffix);

	//assert(is.is_open());
	string fullMessage = "";
	if (is.is_open())
	{
		list<string> lineList;
		string line;
		while (std::getline(is, line))
		{
			lineList.push_back(line);
		}

		for (auto it = lineList.begin(); it != lineList.end(); ++it)
		{
			if (!fullMessage.empty() && fullMessage.back() != ' ')
			{
				fullMessage += " ";
			}

			fullMessage += (*it);
		}

		is.close();
	}
	else
	{
		assert(0);
	}

	SetString(fullMessage);
}

bool TextDisp::NextSection()
{
	if (sectionWait )
	{
		if (currSection < numSections - 1)
		{
			text.setString("");
		}

		++currSection;
		sectionWait = false;
		frame = 0;
		
		return true;
	}
	else
	{
		return false;
	}
}

//have a character limit for each line. you place text on each line as the writer.

void TextDisp::SetTopLeft(sf::Vector2f &pos)
{
	bgRect.setPosition(pos);
	Vector2f spacing(10, 5);
	text.setPosition(bgRect.getPosition().x + spacing.x, bgRect.getPosition().y + spacing.y);
}

void TextDisp::SetString(const std::string &str)
{
	AddLineBreaks(str);
	bool s = show;
	Reset();
	show = s;
}

void TextDisp::Show()
{
	show = true;
}

void TextDisp::Hide()
{
	show = false;
}

void TextDisp::Reset()
{
	text.setString("");
	frame = 0;
	show = false;
	currSection = 0;
	sectionWait = false;
}

void TextDisp::AddLineBreaks( const std::string &m )
{
	string msg = m;
	int len = msg.length();

	string newStr;
	list<string> lineList;

	newStr == "";
	int lastInd;
	for (int i = 0; i < len; ++i)
	{
		newStr += msg.at(i);
		if (newStr.back() == ' ')
		{
			text.setString(newStr);
			if( text.getGlobalBounds().width >= rectSize.x )
			{ 
				lastInd = newStr.find_last_of(' ');
				newStr = newStr.substr(0, lastInd);
				lastInd = newStr.find_last_of(' ');
				i -= newStr.size() - lastInd;
				newStr = newStr.substr(0, lastInd);
				lineList.push_back(newStr);
				newStr = "";
			}
		}
		else if (i == len - 1)
		{
			text.setString(newStr);
			if (text.getGlobalBounds().width >= rectSize.x)
			{
				lastInd = newStr.find_last_of(' ');
				i -= newStr.size() - lastInd;
				++i;
				newStr = newStr.substr(0, lastInd);
				lineList.push_back(newStr);
				newStr = "";
			}
		}
	}

	if (newStr.size() > 0)
	{
		lineList.push_back(newStr);
	}

	msg = "";

	int lineCount = lineList.size();


	unsigned int charSize = text.getCharacterSize();
	float spacing = text.getFont()->getLineSpacing(charSize);
	float vert = spacing;//((float)charSize) + spacing;
	linesShownAtOnce = rectSize.y / vert;

	float fLineCount = lineCount;
	numSections = ceil(fLineCount / linesShownAtOnce);
	sections = new string[numSections];

	int secIndex = 0;
	int i = 0;
	for (auto it = lineList.begin(); it != lineList.end(); ++it)
	{
		msg += (*it);
		if (i < lineCount - 1)
		{
			msg += "\n";
		}
		
		++i;

		if ( (i % linesShownAtOnce == 0) || i == lineCount)
		{
			sections[secIndex] = msg;
			++secIndex;
			msg = "";
		}
	}
}

bool TextDisp::Update()
{
	if (sectionWait)
	{
		return true;
	}

	if (currSection == numSections)
	{
		return false;
	}

	string &message = sections[currSection];
	int len = message.length();
	int textLen = text.getString().getSize();

	//text.setString(message);//.substr(0, textLen + 1));
	if (textLen == len || !show )
	{
		sectionWait = true;
		return true;
		//if (currSection < numSections - 1)
		//{
		//	sectionWait = true;
		//	//++currSection;
		//	//text.setString("");
		//	return true;
		//}
		//else
		//{
		//	return false;
		//}
	}
	else
	{
		if (frame >= nextLetterWait)
		{
			frame = 0;

			string sub = message.substr(0, textLen + letterPerShow);
			if (sub.back() == ' ')
			{
			}
			else
			{

			}
			text.setString(sub);


		}
		++frame;
	}
	return true;
}

void TextDisp::Draw(sf::RenderTarget *target)
{
	if (show)
	{
		target->draw(bgRect);
		target->draw(text);
	}
}

Script::Script()
{
	sections = NULL;
	numSections = 0;
}

void Script::Load(const std::string &name)
{
	string path = "Resources/Text/";
	string suffix = ".txt";

	ifstream is;
	is.open(path + name + suffix);

	//assert(is.is_open());
	fullMessage = "";
	if (is.is_open())
	{
		list<string> lineList;
		string line;
		while (std::getline(is, line))
		{
			lineList.push_back(line);
		}
		
		for (auto it = lineList.begin(); it != lineList.end(); ++it)
		{
			if (!fullMessage.empty() && fullMessage.back() != ' ')
			{
				fullMessage += " ";	
			}

			fullMessage += (*it);
		}
		
		is.close();
	}
	else
	{
		assert(0);
	}



	/*if (is.is_open())
	{
		list<string> lineList;

		string fullText;
		string line;

		while (std::getline(is, line))
		{
			if (line == "")
			{
				lineList.push_back(fullText);
				fullText = "";
			}
			else
			{
				fullText += line + "\n";
			}
		}

		lineList.push_back(fullText);

		numSections = lineList.size();
		sections = new string[numSections];
		for (int i = 0; i < numSections; ++i)
		{
			sections[i] = "";
		}
		int sectionIndex = 0;
		for (auto it = lineList.begin(); it != lineList.end(); ++it)
		{
			sections[sectionIndex] = (*it);
			++sectionIndex;
		}

		is.close();
	}
	else
	{
		assert(0);
	}*/


	//int x = 5;
	//sections[sectionIndex] = fullText;
	//fullText = "";
	//sectionIndex++;

	/*for (int i = 0; i < 5; ++i)
	{
		sections[i] = "hello this is a test hello this abcdefghijklmno\n"
			"hello this is a test hello this\n"
			"hello this is a test hello this\n"
			"hello this is a test hello this";
	}

	sections[1] = "blah blah";*/
}

const std::string & Script::GetSection(int index)
{
	return sections[index];
}

Script::~Script()
{
	if( sections != NULL)
		delete[] sections;
}