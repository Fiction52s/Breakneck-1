#include "StorySequence.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <sstream>

using namespace std;
using namespace sf;

StoryText::StoryText( sf::Font &font, const std::string &p_str, Vector2f &pos )
{
	textStr = p_str;
	text.setFont(font);
	text.setFillColor(Color::White);
	text.setCharacterSize(20);
	text.setPosition(pos);
	text.setString(textStr);
}

bool StoryText::Update(ControllerState &prev, ControllerState &curr)
{
	if (curr.A && !prev.A)
	{
		done = true;
		return false;
	}

	return true;
}

void StoryText::Reset()
{
	done = false;
}

void StoryText::Draw(sf::RenderTarget *target)
{
	target->draw(text);
}

//eventually make this the font manager
StorySequence::StorySequence(sf::Font &font, TilesetManager *p_tm)
	:tm(p_tm), myFont(font)
{
	
}

bool StorySequence::Load(const std::string &sequenceName)
{
	ifstream is;
	string fName = "Resources/Scripts/";
	fName += sequenceName + string(".txt");
	is.open(fName);

	
	if (is.is_open())
	{
		string waste;
		string line;
		StoryPart *parentPart = NULL;
		while (getline(is, line))
		{
			std::replace(line.begin(), line.end(), ',', ' '); // replace all 'x' to 'y'

			stringstream ss;
			ss << line;

			string imageName;
			ss >> imageName;

			ss >> waste;
			int posx, posy;
			ss >> posx;
			ss >> posy;

			string intro;
			ss >> intro;

			bool bHasIntro;
			if (intro == "has")
			{
				bHasIntro = true;
			}
			else if (intro == "no")
			{
				bHasIntro = false;
			}
			else
			{
				bHasIntro = false;
				assert(0);
			}

			ss >> waste;

			ss >> waste;

			int layer;
			ss >> layer;

			float time;
			ss >> time;

			ss >> waste;

			string hasSubLayer;
			ss >> hasSubLayer;

			bool bhasSubLayer;
			if (hasSubLayer == "has")
			{
				bhasSubLayer = true;
			}
			else if (hasSubLayer == "no")
			{
				bhasSubLayer = false;
			}
			else
			{
				bhasSubLayer = false;
				assert(0);
			}

			ss >> waste;

			string hasText;

			ss >> hasText;

			bool bHasText;
			if (hasText == "has")
			{
				bHasText = true;
			}
			else if (hasText == "no")
			{
				bHasText = false;
			}
			else
			{
				bHasText = false;
				assert(0);
			}

			/*cout << "image: " << imageName << endl;
			cout << "pos: " << posx << ", " << posy << endl;
			cout << "hasIntro: " << intro << endl;
			cout << "layer: " << layer << endl;
			cout << "time: " << time << endl;
			cout << "hasSublayer: " << hasSubLayer << endl;
			cout << "has text: " << hasText << endl;*/

			StoryText *sText = NULL;
			if (bHasText)
			{
				if (!getline(is, line))
				{
					assert(0);
				}

				std::replace(line.begin(), line.end(), ',', ' ');

				ss << line;

				ss >> waste;

				int tposx, tposy;
				ss >> tposx;
				ss >> tposy;

				ss >> waste;

				string fontStr;
				ss >> fontStr;

				ss >> waste;

				int fontSize;
				ss >> fontSize;

				//read the actual text

				std::vector<char> readTextVec;
				readTextVec.reserve(2048);
				char c;
				while (true)
				{
					c = is.peek();
					if (c == EOF)
					{
						assert(0);
						return false;
					}
					
					if (c == '{')
					{
					}
					else if (c == '}')
					{
						getline(is, line);
						break;
					}
					else
					{
						readTextVec.push_back(c);
					}

					is.get();
				}

				std::string textStr(readTextVec.begin(), readTextVec.end());

				sText = new StoryText(myFont, textStr, Vector2f(tposx, tposy));

			}


			StoryPart *sp = new StoryPart;
			string fullImagePath = string("Story/") + imageName + string(".png");
			sp->imageName = imageName;
			sp->ts = tm->GetTileset(fullImagePath, 0, 0);
			if (sp->ts == NULL)
			{
				assert(false && "tileset not loading for story");
			}
			sp->spr.setTexture(*sp->ts->texture);
			sp->spr.setPosition(posx, posy);
			sp->hasIntro = bHasIntro;
			sp->layer = layer;
			sp->time = time;
			sp->totalFrames = time * 60.f;
			sp->text = sText;

			if (parentPart != NULL)
			{
				parentPart->sub = sp;
			}
			else
			{
				parts.push_back(sp);
			}

			if (bhasSubLayer)
			{
				parentPart = sp;
			}
			else
			{
				parentPart = NULL;
			}
		}
		
		is.close();
	}
	else
	{
		assert(0);
		return false;
	}
}

void StorySequence::Reset()
{
	for (auto it = parts.begin(); it != parts.end(); ++it)
	{
		(*it)->Reset();
	}
	currPartIt = parts.begin();
}

void StorySequence::Draw(sf::RenderTarget *target)
{
	(*currPartIt)->Draw(target);
}

bool StorySequence::Update(ControllerState &prev, ControllerState &curr)
{
	if (currPartIt == parts.end())
	{
		return false;
	}

	bool updateSuccess = (*currPartIt)->Update( prev, curr );
	if (!updateSuccess)
	{
		++currPartIt;

		if (currPartIt == parts.end())
		{
			return false;
		}
	}

	return true;
}

StoryPart::StoryPart()
{
	ts = NULL;
	layer = 0;
	time = 0;
	frame = 0;
	totalFrames = -1;
	sub = NULL;
	text = NULL;
}

void StoryPart::Reset()
{
	if (sub != NULL)
	{
		sub->Reset();
	}
	frame = 0;
	if (text != NULL)
		text->Reset();
}

void StoryPart::Draw(sf::RenderTarget *target)
{
	bool showText = false;
	if (frame == totalFrames && text != NULL && !text->done)
	{
		showText = true;
	}

	if ( !showText && frame == totalFrames && sub != NULL && sub->layer < layer)
	{
		sub->Draw(target);
	}
	target->draw(spr);
	if ( !showText && frame == totalFrames && sub != NULL && sub->layer >= layer)
	{
		sub->Draw(target);
	}

	if (showText)
	{
		text->Draw(target);
	}
}

bool StoryPart::Update(ControllerState &prev, ControllerState &curr)
{	
	if (frame == totalFrames)
	{
		if (text != NULL && !text->done)
		{
			if (text->Update(prev, curr))
			{
				return true;
			}
		}

		if ( sub != NULL && sub->Update(prev, curr))
		{
			return true;
		}
		else
		{
			return false;
		}
		
	}
	else
	{
		++frame;
		return true;
	}
}