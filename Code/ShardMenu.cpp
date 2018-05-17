//sfe::Movie m;
//assert(m.openFromFile("Movie/testvid.ogv"));
//m.fit(sf::FloatRect(0, 0, 1920, 1080));
//
//
//m.play();

#include "ShardMenu.h"
#include <sstream>
#include <list>
#include <assert.h>
#include <iostream>
#include "ItemSelector.h"
#include "VectorMath.h"
#include "Enemy_Shard.h"
#include "MainMenu.h"
#include "MusicSelector.h"

using namespace sf;
using namespace std;

ShardMenu::ShardMenu(MainMenu *mm)
	:mainMenu( mm )
{
	currShardMusic = NULL;
	testSeq = NULL;
	//currentMovie.scale(.5, .5);

	currShardText.setCharacterSize(20);
	currShardText.setFont(mm->arial);
	currShardText.setPosition(960, 700);

	numShardsTotal = 1;
	shardQuads = new Vertex[numShardsTotal * 4];
	currButtonState = S_NEUTRAL;
	imagePos = Vector2f(900, 100);

	ts_shards = mm->tilesetManager.GetTileset("Menu/shards_48x48.png", 48, 48);

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	int xSize = 11;
	int ySize = 14;

	shardNames = new string*[xSize];
	shardDesc = new string*[xSize];

	for (int i = 0; i < xSize; ++i)
	{
		shardNames[i] = new string[ySize];
		shardDesc[i] = new string[ySize];
	}

	for (int x = 0; x < xSize; ++x)
	{
		for (int y = 0; y < ySize; ++y)
		{
			std::string &currShardName = shardNames[x][y];
			currShardName = Shard::GetShardString(ShardType::SHARD_W1_TEACH_JUMP);
			if(currShardName != "" )
				SetDescription(shardDesc[x][y], shardNames[x][y]);
		}
	}

	shardNames[4][4] = "testanim";
	shardNames[0][4] = "Crawler_Pose";

	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, xSize, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, ySize, 0);

	shardSelectQuads = new sf::Vertex[xSize * ySize * 4];

	int index = 0;
	int rectSize = 48;
	int xSpacing = 20;
	int ySpacing = 12;
	Vector2f gridStart(100, 100);
	for (int i = 0; i < xSelector->totalItems; ++i)
	{
		for (int j = 0; j < ySelector->totalItems; ++j)
		{
			index = (i * ySelector->totalItems + j) * 4;
			SetRectCenter(shardSelectQuads + index, rectSize, rectSize, Vector2f(i * rectSize + xSpacing * i, j * rectSize + ySpacing * j) + gridStart);
			SetRectSubRect(shardSelectQuads + index, ts_shards->GetSubRect(j * xSelector->totalItems + i));
		}
	}
	
	//SetCurrSequence();
}

bool ShardMenu::SetDescription(std::string &destStr, const std::string &shardTypeStr)
{
	stringstream ss;
	ss << "Shard/" << shardTypeStr << ".sdesc";
	ifstream is;
	is.open(ss.str());

	destStr = "";
	if( is.is_open())
	{
		is.seekg(0, std::ios::end);
		destStr.reserve(is.tellg());
		is.seekg(0, std::ios::beg);

		destStr.assign((std::istreambuf_iterator<char>(is)),
			std::istreambuf_iterator<char>());

		//cout << "dest str: " << destStr << endl;
	}
	else
	{
		cout << "trying to get: " << shardTypeStr << " description" << endl;
		return false;
		assert(0);
	}
	return true;
}

void ShardMenu::SetCurrMusic()
{
	std::string &name = shardNames[xSelector->currIndex][ySelector->currIndex];
	assert(currShardMusic == NULL);
	currShardMusic = GetShardMusic(name);
	if (currShardMusic != NULL)
	{
		bool loaded = currShardMusic->music != NULL;
		if (currShardMusic->music == NULL)
			loaded = currShardMusic->Load();
		assert(loaded);
		currShardMusic->music->setVolume(100);
		currShardMusic->music->play();
	}
}

void ShardMenu::SetCurrSequence()
{
	std::string &name = shardNames[xSelector->currIndex][ySelector->currIndex];
	testSeq = GetSequence(name);//GetSequence("testanim");
	testSeq->spr.setPosition(imagePos);
}

MusicInfo *ShardMenu::GetShardMusic(const std::string &str)
{
	//stringstream ss;
	
	//ss.str("");
	//ss << "Shard/" << str << "_" << counter << ".png";
	//cout << "test: " << ss.str() << endl;
	return mainMenu->musicManager->songMap["w02_Glade"];
	//sf::Music *m = mainMenu->musicManager->//LoadSong("Audio/Music/.ogg");
}

ShardMenu::~ShardMenu()
{
	for (int i = 0; i < xSelector->totalItems; ++i)
	{
		delete[] shardNames[i];
		delete[] shardDesc[i];
	}
	delete[] shardNames;
	delete[] shardDesc;
	delete xSelector;
	delete ySelector;

	
}

PNGSeq * ShardMenu::GetSequence(const std::string &str)
{
	
	if ( seqMap.count( str ) > 0 )
	{
		PNGSeq *cSeq = seqMap[str];
		cSeq->Reset();
		return cSeq;
	}


	stringstream ss;
	int counter = 0;
	list<Tileset*> tList;
	while (true)
	{
		ss.str("");
		ss << "Shard/" << str << "_" << counter << ".png";
		cout << "test: " << ss.str() << endl;
		Tileset *t = tMan.GetTileset(ss.str(), 512, 512);
		if (t == NULL)
		{
			break;
		}
		else
		{
			tList.push_back(t);
		}
		++counter;
	}

	assert(counter > 0);
	
	if (counter == 0)
	{
		return NULL;
	}
	else
	{

		bool single = tList.size() == 1 && tList.front()->texture->getSize().x == 512;
		
		PNGSeq *newSeq = new PNGSeq(str, tList, single);

		seqMap[str] = newSeq;

		return newSeq;
	}
}

void ShardMenu::SetupShardImages()
{
	
}

void ShardMenu::UpdateShardSelectQuads()
{
	int index = 0;
	Color c = Color::Red;
	for (int i = 0; i < xSelector->totalItems; ++i)
	{
		for (int j = 0; j < ySelector->totalItems; ++j)
		{
			index = (i * ySelector->totalItems + j) * 4;
			
			SetRectColor(shardSelectQuads + index, c);
		}
	}

	Color currColor;
	switch (currButtonState)
	{
	case S_NEUTRAL:
		currColor = Color::White;
		break;
	case S_PRESSED:
		currColor = Color::Blue;
		break;
	case S_UNPRESSED:
		currColor = Color::Green;
		break;
	case S_SELECTED:
		currColor = Color::Magenta;
		break;
	}

	index = (xSelector->currIndex * ySelector->totalItems + ySelector->currIndex) * 4;
	SetRectColor(shardSelectQuads + index, currColor);

	
}

void ShardMenu::SetCurrentDescription()
{
	currShardText.setString(shardDesc[xSelector->currIndex][ySelector->currIndex]);
}

void ShardMenu::StopMusic()
{
	if (currShardMusic != NULL)
	{
		currShardMusic->music->stop();
		currShardMusic = NULL;
	}
}

void ShardMenu::Update( ControllerState &currInput )
{
	int xchanged = xSelector->UpdateIndex(currInput.LLeft(), currInput.LRight());
	int ychanged = ySelector->UpdateIndex(currInput.LUp(), currInput.LDown());

	if (xchanged != 0 || ychanged != 0 )
	{
		currButtonState = S_NEUTRAL;
		StopMusic();
		SetCurrSequence();
		SetCurrentDescription();
	}
	bool A = currInput.A;
	switch (currButtonState)
	{
	case S_NEUTRAL:
		if (A)
		{
			currButtonState = S_PRESSED;
		}
		break;
	case S_PRESSED:
		if (!A)
		{
			currButtonState = S_SELECTED;
			SetCurrSequence();
			SetCurrMusic();
		}
		break;
	case S_SELECTED:
		if (A)
		{
			currButtonState = S_UNPRESSED;
		}
		break;
	case S_UNPRESSED:
		if (!A)
		{
			currButtonState = S_NEUTRAL;
			StopMusic();
		}
		break;
	}
	if( testSeq != NULL && currButtonState == S_SELECTED )
		testSeq->Update();
	UpdateShardSelectQuads();
}

void ShardMenu::Draw(sf::RenderTarget *target)
{
	if( testSeq != NULL )
		testSeq->Draw(target);
	target->draw(shardSelectQuads, xSelector->totalItems * ySelector->totalItems * 4,
		sf::Quads, ts_shards->texture);
	if (currShardText.getString() != "")
	{
		target->draw(currShardText);
	}
	//target->draw(currentMovie);
}

bool ShardMenu::LoadPNGSequences()	
{
	return false;
}

PNGSeq::PNGSeq(const
	std::string &p_seqName, list<Tileset*> &tList, bool p_singleImage )
	:seqName(p_seqName), singleImage( p_singleImage )
{
	if (singleImage)
	{
		numFrames = tList.size();
	}
	else
	{
		numFrames = tList.size() * 16;
	}
	
	numSets = tList.size();
	tSets = new Tileset*[numSets];
	int index = 0;
	for (auto it = tList.begin(); it != tList.end(); ++it)
	{
		tSets[index] = (*it);
		++index;
	}
	Reset();
}

PNGSeq::~PNGSeq()
{
	delete[] tSets;
}

void PNGSeq::Load()
{

}

void PNGSeq::Update()
{
	if (tileIndex == 16)
	{
		++setIndex;
		if (setIndex == numSets)
		{
			setIndex = 0;
		}
		tileIndex = 0;

		spr.setTexture(*tSets[setIndex]->texture);
	}

	spr.setTextureRect(tSets[setIndex]->GetSubRect(tileIndex));

	if (!singleImage)
	{
		tileIndex++;
	}
}

void PNGSeq::Reset()
{
	tileIndex = 0;
	setIndex = 0;
	spr.setTexture( *tSets[0]->texture );
	spr.setTextureRect(tSets[0]->GetSubRect(0));
}

void PNGSeq::Draw(sf::RenderTarget *target)
{
	target->draw(spr);
}