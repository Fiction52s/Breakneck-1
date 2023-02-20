#include "ImageText.h"
#include <assert.h>
#include "GameSession.h"
#include "MainMenu.h"
#include "PaletteShader.h"

using namespace std;
using namespace sf;

ImageText::ImageText( int p_maxDigits, Tileset *ts_tex )
{
	maxDigits = p_maxDigits;
	ts = ts_tex;
	int numVertices = maxDigits * 4;
	vert = new Vertex[numVertices + 4];
	value = 0;

	symbolType = SymbolType::SYMBOL_NONE;

	ClearRect(vert + (maxDigits * 4));
	
	posType = TOP_LEFT;
	numShowZeroes = 0;
	activeDigits = 1;
	scale = 1.f;

	sh = NULL;
}

ImageText::~ImageText()
{
	delete[]vert;
}

void ImageText::SetSymbol(SymbolType sType)
{
	symbolType = sType;

	if (symbolType == SYMBOL_NONE)
	{
		ClearRect(vert + maxDigits * 4);
	}
}

void ImageText::SetShader(sf::Shader *p_sh)
{
	sh = p_sh;
}

void ImageText::SetCenter(sf::Vector2f &p_center)
{
	anchor = p_center;
	posType = CENTER;
	UpdateSprite();
}

void ImageText::SetTopRight(sf::Vector2f &p_topRight)
{
	anchor = p_topRight;
	posType = TOP_RIGHT;
}

void ImageText::SetTopLeft(sf::Vector2f &p_topLeft)
{
	anchor = p_topLeft;
	posType = TOP_LEFT;
}

void ImageText::UpdateSprite()
{
	int div = 10;
	int val = value;
	int ind = 0;
	
	for (int i = 0; i < maxDigits + 1; ++i)
	{
		ClearRect(vert + i * 4);
	}

	while( true )
	{
		int res = val % div;

		IntRect subRect = ts->GetSubRect( res );
		if (sh != NULL)
		{
			SetRectSubRectGL(vert + ind * 4, subRect, Vector2f(ts->texture->getSize()));
		}
		else
		{
			SetRectSubRect(vert + ind * 4, subRect);
		}

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

		if (sh != NULL)
		{
			SetRectSubRectGL(vert + ind * 4, subRect, Vector2f(ts->texture->getSize()));
		}
		else
		{
			SetRectSubRect(vert + ind * 4, subRect);
		}
	}

	activeDigits = ind;

	float tw = ts->tileWidth * scale;
	float th = ts->tileHeight * scale;

	Vector2f anchorTopLeft;

	float halfOfDigits = maxDigits / 2.f;

	//its possible there is an issue with this on alignments that aren't 
	if (posType == CENTER)
	{
		if (activeDigits % 2 == 0 )
		{
			anchorTopLeft = anchor + Vector2f(tw * halfOfDigits, th * -.5); //-2.5 on x before
		}
	}
	else if (posType == TOP_RIGHT)
	{
		anchorTopLeft = anchor + Vector2f(tw * -5, 0);
	}
	else if (posType == TOP_LEFT)
	{
		anchorTopLeft = anchor;
	}

	if (symbolType != SYMBOL_NONE)
	{
		SetRectTopLeft(vert + maxDigits * 4, tw, th, anchorTopLeft);
		anchorTopLeft.x += tw;

		int tile = 0;

		switch (symbolType)
		{
		case SYMBOL_PLUS:
			tile = 11;
			break;
		case SYMBOL_MINUS:
			tile = 12;
			break;
		}

		IntRect subRect = ts->GetSubRect(tile);
		if (sh != NULL)
		{
			SetRectSubRectGL(vert + maxDigits * 4, subRect, Vector2f(ts->texture->getSize()));
		}
		else
		{
			SetRectSubRect(vert + maxDigits * 4, subRect);
		}
	}

	/*for (int i = 0; i < maxDigits; ++i)
	{
		SetRectTopLeft(vert + i * 4, tw, th, anchorTopLeft + Vector2f(tw * ((maxDigits-1) - i), 0));
	}*/

	for (int i = 0; i < maxDigits; ++i)
	{
		SetRectTopLeft(vert + i * 4, tw, th, anchorTopLeft + Vector2f(tw/2 * i, 0));//tw * i, 0));
	}


}

void ImageText::Draw( sf::RenderTarget *target )
{
	if (sh != NULL)
	{
		target->draw(vert, (maxDigits +1 )* 4, sf::Quads, sh);
	}
	else
	{
		target->draw(vert, (maxDigits + 1) * 4, sf::Quads, ts->texture);
	}
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

void ImageText::SetScale(float s)
{
	scale = s;
}

TimerText::TimerText( Tileset *ts_tex )
	:ImageText(5, ts_tex )
{
	

	//vert[colonIndex*4+0].texCoords = Vector2f( subRect.left, subRect.top );
	//vert[colonIndex*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
	//vert[colonIndex*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
	//vert[colonIndex*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
}

void TimerText::UpdateSprite()
{
	int numMinutes = value / 60;
	int numSeconds = value % 60;

	int tw = ts->tileWidth * scale;
	int th = ts->tileHeight * scale;

	Vector2f anchorTopLeft;

	if (posType == CENTER)
	{
		anchorTopLeft = anchor + Vector2f( tw * -2.5, th * -.5 );
	}
	else if( posType == TOP_RIGHT )
	{
		anchorTopLeft = anchor + Vector2f(tw * -5, 0);
	}
	else if (posType == TOP_LEFT)
	{
		anchorTopLeft = anchor;
	}

	if (symbolType != SYMBOL_NONE)
	{
		SetRectTopLeft(vert + maxDigits * 4, tw, th, anchorTopLeft);
		anchorTopLeft.x += tw;

		int tile = 0;

		switch (symbolType)
		{
		case SYMBOL_PLUS:
			tile = 11;
			break;
		case SYMBOL_MINUS:
			tile = 12;
			break;
		}

		IntRect subRect = ts->GetSubRect(tile);
		if (sh != NULL)
		{
			SetRectSubRectGL(vert + maxDigits * 4, subRect, Vector2f(ts->texture->getSize()));
		}
		else
		{
			SetRectSubRect(vert + maxDigits * 4, subRect);
		}
	}

	for (int i = 0; i < maxDigits; ++i)
	{
		SetRectTopLeft(vert + i * 4, tw, th, anchorTopLeft + Vector2f(tw * i, 0));
	}

	int div = 10;
	int val = value;
	int ind = 0;
	
	int second1s = numSeconds % 10;
	int second10s = numSeconds / 10;

	int minute1s = numMinutes % 10;
	int minute10s = numMinutes / 10;

	

	int colonIndex = 2;//5/2;

	if (sh != NULL)
	{
		SetRectSubRectGL(vert, ts->GetSubRect(minute10s), Vector2f(ts->texture->getSize()));
		SetRectSubRectGL(vert + 4, ts->GetSubRect(minute1s), Vector2f(ts->texture->getSize()));

		SetRectSubRectGL(vert + 3 * 4, ts->GetSubRect(second10s), Vector2f(ts->texture->getSize()));
		SetRectSubRectGL(vert + 4 * 4, ts->GetSubRect(second1s), Vector2f(ts->texture->getSize()));

		//colon
		SetRectSubRectGL(vert + colonIndex * 4, ts->GetSubRect(10), Vector2f(ts->texture->getSize()));
	}
	else
	{
		SetRectSubRect(vert, ts->GetSubRect(minute10s));
		SetRectSubRect(vert + 4, ts->GetSubRect(minute1s));

		SetRectSubRect(vert + 3 * 4, ts->GetSubRect(second10s));
		SetRectSubRect(vert + 4 * 4, ts->GetSubRect(second1s));

		//colon
		SetRectSubRect(vert + colonIndex * 4, ts->GetSubRect(10));
	}

	activeDigits = maxDigits;
}

TextDisp::TextDisp( int width, int height, int charSize, int frameLetterWait,
	int p_letterPerShow )
{
	show = false;
	//message = "hello this is a test";
	bgRect.setFillColor(Color( 0, 0, 0, 100 ));
	rectSize = Vector2f(width, height);//Vector2f(670, 220);
	bgRect.setSize(rectSize);
	nextLetterWait = frameLetterWait;

	text.setCharacterSize(charSize);
	text.setFont(Session::GetSession()->mainMenu->consolas);
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

sf::Vector2f TextDisp::GetTopLeft()
{
	return bgRect.getPosition();
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

void TextDisp::SetRate(int wait, int letterPer)
{
	nextLetterWait = wait;
	letterPerShow = letterPer;
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

Speech::Speech()
{
	sess = Session::GetSession();
}


Speech::~Speech()
{
	delete disp;
}

void Speech::SetTextMode(TextMode tMode)
{
	int left = 256;

	if (speaker != "None")
	{
		left += 128;
	}


	switch (tMode)
	{
	case TOPTEXT:
		disp->SetTopLeft(Vector2f(left, 80));
		break;
	case BOTTOMTEXT:
		disp->SetTopLeft(Vector2f(left, 1080 - (220 + 80) ));
		break;
	}
}

void Speech::SetupSprite()
{
	if (speaker == "None")
	{
		ts = NULL;
	}
	else
	{
		ts = GetTileset();
		speakerSpr.setTexture(*ts->texture);
		speakerSpr.setTextureRect(ts->GetSubRect(speakerTile));

		if (ts->tileWidth == 384 && ts->tileHeight == 384)
		{
			speakerSpr.setScale(2.0 / 3.0, 2.0 / 3.0);
		}
		Vector2f dispPos = disp->GetTopLeft();
		speakerSpr.setPosition(dispPos.x - 256, dispPos.y);
	}
}

void Speech::Draw(sf::RenderTarget *target)
{
	disp->Draw(target);
	if (ts != NULL && disp->show )
	{
		target->draw(speakerSpr);
	}
}

Tileset *Speech::GetTileset()
{
	if (speaker == "Momenta")
	{
		return sess->GetTileset("Momenta/momentaportrait_320x288.png", 320, 288);
	}
	else if (speaker == "Kin")
	{
		return sess->GetTileset("Story/Portraits/w0_kin_256x256.png", 256, 256);
	}
	else if (speaker == "Crawler")
	{
		return sess->GetTileset("Story/Portraits/w1_crawler_face_256x256.png", 256, 256);
	}
	else if (speaker == "Bird")
	{
		return sess->GetTileset("Story/Portraits/w2_bird_face_256x256.png", 256, 256);
	}
	else if (speaker == "Coyote")
	{
		return sess->GetTileset("Story/Portraits/w3_coyote_face_256x256.png", 256, 256);
	}
	else if (speaker == "Tiger")
	{
		return sess->GetTileset("Story/Portraits/w4_tiger_face_256x256.png", 256, 256);
	}
	else if (speaker == "Gator")
	{
		return sess->GetTileset("Story/Portraits/w5_gator_face_256x256.png", 256, 256);
	}
	else if (speaker == "Skeleton")
	{
		return sess->GetTileset("Story/Portraits/w6_skeleton_face_256x256.png", 256, 256);
	}
	else if (speaker == "Bear")
	{
		return sess->GetTileset("Story/Portraits/w8_bear_face_256x256.png", 256, 256);
	}
	else
	{
		//assert(0);
		return sess->GetTileset("Bosses/Bird/bird_face_384x384.png", 384, 384);
	}
}

ConversationGroup::ConversationGroup()
{
	convs = NULL;
	numConvs = NULL;
}

ConversationGroup::~ConversationGroup()
{
	if (convs != NULL)
	{
		for (int i = 0; i < numConvs; ++i)
		{
			delete convs[i];
		}
		delete[] convs;
	}
}

Conversation* ConversationGroup::GetConv(int index)
{
	if (index < 0 || index >= numConvs)
	{
		return NULL;
	}
	else
	{
		return convs[index];
	}
}

void ConversationGroup::Load(const std::string &name)
{
	sceneName = name;

	string path = "Resources/Text/";
	string suffix = ".txt";

	string fullFileName = path + name + suffix;

	ifstream is;
	is.open(fullFileName);

	if (!is.is_open())
	{
		assert(0);
	}



	Conversation *c;

	std::list<Conversation*> convList;

	while (true)
	{
		c = new Conversation;
		convList.push_back(c);
		if (!c->Load(is))
		{
			break;
		}
	}
	

	numConvs = convList.size();
	convs = new Conversation*[numConvs];

	int i = 0;
	for (auto it = convList.begin(); it != convList.end(); ++it)
	{
		convs[i] = (*it);
		++i;
	}

	is.close();
}

void ConversationGroup::Reset()
{
	for (int i = 0; i < numConvs; ++i)
	{
		convs[i]->Reset();
	}
}

Conversation::Conversation()
{
	speeches = NULL;
	numSpeeches = 0;
	show = false;
}

Conversation::~Conversation()
{
	if (speeches != NULL)
	{
		for (int i = 0; i < numSpeeches; ++i)
		{
			delete speeches[i];
		}
		delete[] speeches;
	}
	
}

std::string Conversation::GetSpeakerName( const std::string &line, int &tileIndex )
{
	if (line.front() == '$')
	{
		size_t pos = line.find(' ');
		string name;
		
		if (pos != std::string::npos)
		{
			name = line.substr(1, pos - 1);
			
			if (name != "None")
			{
				string number = line.substr(pos);

				stringstream ss;
				ss << number;
				ss >> tileIndex;
			}	
		}
		else
		{
			name = line.substr(1);
			tileIndex = 0;
		}

		return name;
	}
	else
	{
		assert(0);
	}
}

void Conversation::Show()
{
	if (!show)
	{
		show = true;
		speeches[currSpeech]->disp->Show();
	}
}

void Conversation::Hide()
{
	if (show)
	{
		show = false;
		speeches[currSpeech]->disp->Hide();
	}
}

void Conversation::NextSection()
{
	//if (currSpeech < numSpeeches)
	//{
	speeches[currSpeech]->disp->NextSection();
	//}
}

void Conversation::SetRate(int wait, int letterPer)
{
	speeches[currSpeech]->disp->SetRate(wait, letterPer);
}

bool Conversation::Load(ifstream &is)
{
	list<pair<string, list<string>>> parts;
	list<int> partTile;

	string fullMessage = "";
	bool getSpeaker = true;
	list<string> *lineList = NULL;
	int tileIndex = -1;
	bool keepLoading = false;
	if (is.is_open())
	{
		if (parts.size() > 0)
		{
			lineList = &parts.back().second;
		}

		string line;
		while (std::getline(is, line))
		{
			if (line == "")
			{
				continue;
			}
			if (line.at(0) == '$')
			{
				parts.push_back(pair<string, list<string>>(GetSpeakerName(line, tileIndex), list<string>()));
				partTile.push_back(tileIndex);
				lineList = &parts.back().second;
			}
			else if (line.at(0) == '&' )
			{
				keepLoading = true;
				break;
			}
			else
			{
				lineList->push_back(line);
			}
		}
	}
	else
	{
		assert(0);
	}

	numSpeeches = parts.size();
	speeches = new Speech*[numSpeeches];

	int i = 0;
	if (parts.size() > 0)
	{

		auto tileIt = partTile.begin();
		for (auto it = parts.begin(); it != parts.end(); ++it, ++tileIt)
		{
			Speech *sp = new Speech;
			sp->speaker = (*it).first;
			sp->speakerTile = (*tileIt);
			lineList = &(*it).second;

			fullMessage = "";
			for (auto it2 = lineList->begin(); it2 != lineList->end(); ++it2)
			{
				if (!fullMessage.empty() && fullMessage.back() != ' ')
				{
					fullMessage += " ";
				}

				fullMessage += (*it2);
			}

			//TextDisp *textDisp = new TextDisp((1920 - 512), 220, 30, 1);
			//textDisp->SetTopLeft(Vector2f(512, 1080 - 220));
			TextDisp *textDisp = new TextDisp((1920 - 512), 256, 30, 1);
			textDisp->SetTopLeft(Vector2f(256 + 128, 80));
			textDisp->SetString(fullMessage);
			sp->disp = textDisp;

			sp->SetupSprite();

			speeches[i] = sp;

			++i;
		}

	}

	return keepLoading;
}

void Conversation::SetTextMode(Speech::TextMode tMode)
{
	for (int i = 0; i < numSpeeches; ++i)
	{
		speeches[i]->SetTextMode(tMode);
	}
}

void Conversation::Load( const std::string &name )
{
	string path = "Resources/Text/";
	string suffix = ".txt";

	ifstream is;
	is.open(path + name + suffix);
	bool res = Load(is);
	is.close();
}

bool Conversation::Update()
{
	Speech *sp = speeches[currSpeech];

	if (!sp->disp->Update())
	{
		sp->disp->Hide();
		

		if (currSpeech + 1 == numSpeeches)
		{
			return false;
		}
		else
		{
			++currSpeech;
		}

		sp = speeches[currSpeech];
		sp->disp->Show();
	}

	return true;
}

void Conversation::Reset()
{
	currSpeech = 0;
	for (int i = 0; i < numSpeeches; ++i)
	{
		speeches[i]->disp->Reset();
	}
	Hide();
}

void Conversation::Draw(sf::RenderTarget *target)
{
	if (show)
	{
		if (currSpeech < numSpeeches)
		{
			speeches[currSpeech]->Draw(target);
		}
	}
}