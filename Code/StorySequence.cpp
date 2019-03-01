#include "StorySequence.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <sstream>
#include "GameSession.h"
#include "MusicSelector.h"
#include "Config.h"

using namespace std;
using namespace sf;


EffectLayer StringToEffectLayer(const std::string &str)
{
	string testStr = str;
	std::transform(testStr.begin(), testStr.end(), testStr.begin(), ::tolower);

	if (str == "behind_terrain")
	{
		return BEHIND_TERRAIN;
	}
	else if (str == "behind_enemies")
	{
		return BEHIND_ENEMIES;
	}
	else if (str == "between_player_and_enemies")
	{
		return BETWEEN_PLAYER_AND_ENEMIES;
	}
	else if (str == "in_front_of_ui")
	{
		return IN_FRONT_OF_UI;
	}
}

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

StorySequence::StorySequence(GameSession *p_owner)
	:owner( p_owner ), tm( &p_owner->tm ), myFont( p_owner->mainMenu->arial)

{

}

void StorySequence::EndSequence()
{
	if (seqName == "queenhurt")
	{
		owner->Fade(true, 60, Color::Black);
		
		Actor *player = owner->GetPlayer(0);
		player->SeqAfterCrawlerFight();
	}
	else if (seqName == "kinhouse")
	{
		//owner->Fade(true, 60, Color::White);
	}
}

bool StorySequence::Load(const std::string &sequenceName)
{
	ifstream is;
	string fName = "Resources/Scripts/";
	fName += sequenceName + string(".txt");
	is.open(fName);
	seqName = sequenceName;

	
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

			if (imageName == "")
			{
				continue;
			}
			bool blankImage = false;
			if (imageName == "wait")
			{
				blankImage = true;
				//empty wait on a certain layer
			}

			int posx, posy;
			if (!blankImage)
			{


				ss >> waste;
				ss >> posx;
				ss >> posy;

			}

			

			//ss >> waste;

			ss >> waste;

			int layer;
			ss >> layer;

			float time;
			ss >> time;

			ss >> waste;

			
			StoryPart *sp = new StoryPart( this );
			sp->time = time;
			sp->totalFrames = time * 60.f;

			bool bHasIntro = false;
			bool bHasOutro = false;
			bool bhasSubLayer = false;
			bool bHasMusic = false;
			bool bHasText = false;
			EffectLayer efL = IN_FRONT;

			while (true)
			{
				string typeStr;
				ss >> typeStr;
				if (ss.fail())
				{
					break;
				}
				if (typeStr == "intro")
				{
					bHasIntro = true;
				}
				else if (typeStr == "outro")
				{
					bHasOutro = true;
				}
				else if (typeStr == "music")
				{
					bHasMusic = true;
				}
				else if (typeStr == "text")
				{
					bHasText = true;
				}
				else if (typeStr == "sublayer")
				{
					bhasSubLayer = true;
				}
				else if (typeStr == "drawlayer")
				{
					string layerStr;
					ss >> layerStr;
					efL = StringToEffectLayer(layerStr);
				}
				//continue;
			}

			ss.clear();
			ss.str("");

			if (bHasOutro)
			{
				if (!getline(is, line))
				{
					assert(0);
				}
				std::replace(line.begin(), line.end(), ',', ' ');

				ss << line;

				string outroType;
				ss >> outroType;

				ss >> waste;

				int colorR;
				int colorG;
				int colorB;
				
				ss >> colorR;
				ss >> colorG;
				ss >> colorB;

				float otime;
				ss >> otime;

				if (outroType == "fade")
				{
					sp->outType = StoryPart::OutroType::O_FADE;
				}

				sp->fadeOutColor = Color(colorR, colorG, colorB);
				sp->fadeOutFrames = otime * 60.f;
				sp->startOutroFadeFrame = sp->totalFrames - sp->fadeOutFrames;
			}


			StoryMusic *sm = NULL;
			if (bHasMusic)
			{
				if (!getline(is, line))
				{
					assert(0);
				}

				std::replace(line.begin(), line.end(), ',', ' ');

				ss << line;

				ss >> waste;

				string musicName;
				ss >> musicName;

				ss >> waste;

				float startTime;
				ss >> startTime;

				ss >> waste;

				float fadeTime = 0;
				if (!ss.fail())
				{
					ss >> fadeTime;
				}
				/*ss >> waste;

				string transitionType;
				ss >> transitionType;

				if (transitionType == "fade")
				{

				}
				else
				{

				}*/

				sm = new StoryMusic;
				sm->musicName = musicName;
				sm->startTime = sf::seconds(startTime);
				sm->transitionSeconds = fadeTime;
			}

			ss.clear();
			ss.str("");

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


			

			
			string fullImagePath = string("Story/") + imageName + string(".png");
			sp->imageName = imageName;

			if (!blankImage)
			{
				sp->ts = tm->GetTileset(fullImagePath, 0, 0);
				if (sp->ts == NULL)
				{
					assert(false && "tileset not loading for story");
				}
				sp->spr.setTexture(*sp->ts->texture);
				sp->spr.setPosition(posx, posy);
				sp->blank = false;
			}
			else
			{
				sp->blank = true;
			}
			
			sp->hasIntro = bHasIntro;
			sp->layer = layer;
			
			sp->text = sText;
			sp->music = sm;
			sp->effectLayer = efL;

			/*if (owner->mainMenu->musicManager->songMap.count() == 0)
			{
				assert(0);
			}*/

			if (owner != NULL)
			{
				if (sm != NULL)
				{
					sm->musicInfo = NULL;
					sm->musicInfo = owner->mainMenu->musicManager->songMap[sm->musicName];
					if (sm->musicInfo == NULL)
					{
						assert(0);
					}
					sm->musicInfo->Load();
					owner->musicMap[sm->musicName] = sm->musicInfo;
				}
			}

			if (parentPart != NULL)
			{
				parentPart->sub = sp;
			}
			else
			{
				parts[layer].push_back(sp);
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
	for (int i = 0; i < NUM_LAYERS; ++i)
	{
		auto &layerParts = parts[i];

		for (auto it = layerParts.begin(); it != layerParts.end(); ++it)
		{
			(*it)->Reset();
		}

		if (!layerParts.empty())
		{
			currPartIt[i] = layerParts.begin();
		}

		pUpdate[i] = false;
	}
}

void StorySequence::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < NUM_LAYERS; ++i)
	{
		if( pUpdate[i] )
		{
			(*currPartIt[i])->Draw(target);
		}
	}
}

void StorySequence::DrawLayer(sf::RenderTarget *target, EffectLayer eLayer )
{
	int startIndex = 0;
	int endIndex = NUM_LAYERS;

	for (int i = startIndex; i < endIndex; ++i)
	{
		if (pUpdate[i])
		{
			auto &cpi = (*currPartIt[i]);
			if (cpi->effectLayer == eLayer)
			{
				cpi->Draw(target);
			}
		}
	}
}

bool StorySequence::UpdateLayer(int layer, ControllerState &prev, ControllerState &curr)
{
	list<StoryPart*> &layerParts = parts[layer];
	list<StoryPart*>::iterator &layerCurrPartIt = currPartIt[layer];

	if ( layerParts.empty() || layerCurrPartIt == layerParts.end())
	{
		return false;
	}

	if ((*layerCurrPartIt)->frame == 0)
	{
		if ((*layerCurrPartIt)->music != NULL)
		{
			float transTime = (*layerCurrPartIt)->music->transitionSeconds;
			if (transTime > 0)
			{
				int transFrames = transTime * 60.f;
				owner->TransitionMusic((*layerCurrPartIt)->music->musicName,
					(*layerCurrPartIt)->music->startTime, transFrames);
			}
			else
			{
				owner->PlayMusic((*layerCurrPartIt)->music->musicName, (*layerCurrPartIt)->music->startTime);
			}

		}
	}

	bool updateSuccess = (*layerCurrPartIt)->Update(prev, curr);
	if (!updateSuccess)
	{
		++layerCurrPartIt;

		if (layerCurrPartIt == layerParts.end())
		{
			return false;
		}
	}

	return true;
}

bool StorySequence::Update(ControllerState &prev, ControllerState &curr)
{
	bool keepUpdating = false;
	bool updateSuccess;
	for (int i = 0; i < NUM_LAYERS; ++i)
	{
		updateSuccess = UpdateLayer(i, prev, curr);
		pUpdate[i] = updateSuccess;
		if (updateSuccess)
		{
			keepUpdating = true;
		}
	}
	return keepUpdating;
}

StoryPart::StoryPart( StorySequence *p_seq)
{
	seq = p_seq;
	ts = NULL;
	layer = 0;
	time = 0;
	frame = 0;
	totalFrames = -1;
	sub = NULL;
	text = NULL;
	effectLayer = EffectLayer::IN_FRONT;
	blank = true;
	outType = OutroType::O_NONE;
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

	if (!blank)
	{
		target->draw(spr);
	}

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
		if ( outType == O_FADE && frame == startOutroFadeFrame)
		{
			seq->owner->Fade(false, fadeOutFrames, fadeOutColor);
		}
		++frame;
		return true;
	}
}

StoryImage::StoryImage()
{
	isConfirmNeeded = false;
	introLength = 0;
	intro = I_NONE;

	flipx = false;
	flipy = false;

	ts = NULL;
}

void StoryImage::SetIntro(ImageIntro iType, int frameLength)
{
	intro = iType;
	introLength = frameLength;
}

void StoryImage::SetOutro(ImageOutro oType, int frameLength)
{
	outro = oType;
	outroLength = frameLength;
}

void StoryImage::Reset()
{
	frame = 0;
}

void StoryImage::SetConfirmNeeded(bool cn)
{
	isConfirmNeeded = cn;
}

//to do multi image sequences in a row or animations, just make more of these objects
bool StoryImage::Update( bool confirm )
{
	if (frame < introLength )
	{
		UpdateIntro();
	}
	else
	{
		
		int aFrame = frame - introLength;
		if (isConfirmNeeded && aFrame >= activeLength)
		{
			frame = introLength + activeLength - 1;
		}

		if (aFrame < activeLength)
		{
			UpdateActive();
		}
		else
		{
			int oFrame = aFrame - activeLength;
			if (oFrame < outroLength)
			{
				UpdateOutro();
			}
			else
			{
				return false;
			}
		}
	}

	++frame;

	return true;
}

void StoryImage::SetFlip(bool x, bool y)
{
	if (flipx != x || flipy != y)
	{
		flipx = x;
		flipy = y;

		UpdateImage();
	}
}

void StoryImage::SetImage(Tileset *t, int tileIndex)
{
	ts = t;
	tile = tileIndex;

	UpdateImage();
}

void StoryImage::UpdateImage()
{
	IntRect ir = ts->GetSubRect(tile);
	if (flipx)
	{
		ir.left += ir.width;
		ir.width = -ir.width;
	}
	if (flipy)
	{
		ir.top += ir.height;
		ir.height = -ir.height;
	}

	spr.setTextureRect(ir);
}

void StoryImage::UpdateIntro()
{
	int tFrame = frame;
}

void StoryImage::UpdateActive()
{
	int tFrame = frame - introLength;
}

void StoryImage::UpdateOutro()
{
	int tFrame = frame - introLength - activeLength;
}

void StoryImage::Draw(sf::RenderTarget *target)
{
	target->draw(spr);
}

void StoryImage::SetPositionTopLeft(sf::Vector2f &pos)
{
	spr.setPosition(pos);
	spr.setOrigin(0, 0);
}

void StoryImage::SetPositionCenter(sf::Vector2f &pos)
{
	spr.setPosition(pos);
	spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2 );
}