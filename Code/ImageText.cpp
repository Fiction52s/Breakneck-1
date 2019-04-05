#include "ImageText.h"
#include <assert.h>
#include "GameSession.h"

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

TextDisp::TextDisp( GameSession *owner)
{
	//message = "hello this is a test";
	bgRect.setFillColor(Color( 0, 0, 0, 100 ));
	rectSize = Vector2f(500, 200);
	bgRect.setSize(rectSize);
	nextLetterWait = 3;

	text.setCharacterSize(40);
	text.setFont(owner->mainMenu->arial);
	text.setFillColor(Color::White);
	//bgRect.setOrigin(bgRect.getLocalBounds().width / 2, bgRect.getLocalBounds().height / 2);
	//Reset()
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
	message = str;
	Reset();
}

void TextDisp::Reset()
{
	text.setString("");
	frame = 0;
}

bool TextDisp::Update()
{
	int len = message.length();
	int textLen = text.getString().getSize();

	//text.setString(message);//.substr(0, textLen + 1));
	if (textLen == len)
	{
		return false;
	}
	else
	{
		if (frame >= nextLetterWait)
		{
			frame = 0;
			text.setString(message.substr(0, textLen + 1));
		}
		++frame;
	}
}

void TextDisp::Draw(sf::RenderTarget *target)
{
	target->draw(bgRect);
	target->draw(text);
}

void Script::Load(const std::string &name)
{
	numSections = 5;
	sections = new string[5];

	string path = "Resources/Text/";
	string suffix = ".script";

	//ifstream is;
	//is.open(path + name + suffix);

	//assert(is.is_open());


	for (int i = 0; i < 5; ++i)
	{
		sections[i] = "hello this is a test hello this\n"
			"hello this is a test hello this\n"
			"hello this is a test hello this\n"
			"hello this is a test hello this";
	}

	sections[1] = "blah blah";
}

const std::string & Script::GetSection(int index)
{
	return sections[index];
}

Script::~Script()
{
	delete[] sections;
}