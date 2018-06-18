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
	//currentMovie.scale(.5, .5);

	currShardText.setCharacterSize(20);
	currShardText.setFont(mm->arial);
	currShardText.setPosition(960, 700);

	numShardsTotal = 1;
	shardQuads = new Vertex[numShardsTotal * 4];
	currButtonState = S_NEUTRAL;
	imagePos = Vector2f(900, 100);

	ts_shards[0] = mm->tilesetManager.GetTileset("Menu/shards_w1_48x48.png", 48, 48);

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	int xSize = 11;
	int ySize = 2;//14;

	shardNames = new string*[xSize];
	shardDesc = new string*[xSize];
	seqLoadThread = new boost::thread**[xSize];
	shardSeq = new PNGSeq**[xSize];

	for (int i = 0; i < xSize; ++i)
	{
		shardNames[i] = new string[ySize];
		shardDesc[i] = new string[ySize];
		seqLoadThread[i] = new boost::thread*[ySize];
		shardSeq[i] = new PNGSeq*[ySize];
	}

	

	for (int x = 0; x < xSize; ++x)
	{
		for (int y = 0; y < ySize; ++y)
		{
			shardSeq[x][y] = NULL;
			seqLoadThread[x][y] = NULL;

			std::string &currShardName = shardNames[x][y];
			currShardName = Shard::GetShardString(ShardType::SHARD_W1_TEACH_JUMP);
			if(currShardName != "" )
				SetDescription(shardDesc[x][y], shardNames[x][y]);
		}
	}

	//shardNames[4][4] = "testanim";
	//shardNames[0][4] = "Crawler_Pose";
	//sf::Clock testC;
	//GetSequence(shardNames[0][0]);
	/*for (int x = 0; x < xSize; ++x)
	{
		for (int y = 0; y < ySize; ++y)
		{
			
		}
	}*/
	//int f = testC.getElapsedTime().asMilliseconds();
	//cout << f << endl;
	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, xSize, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, ySize, 0);

	shardSelectQuads = new sf::Vertex[xSize * ySize * 4];

	selectedShardHighlight.setTexture(*ts_shards[0]->texture);

	int index = 0;
	int rectSize = 48;
	int xSpacing = 20;
	int ySpacing = 12;
	Vector2f gridStart(100, 100);
	for (int i = 0; i < ySelector->totalItems; ++i)
	{
		for (int j = 0; j < xSelector->totalItems; ++j)
		{
			index = (i * xSelector->totalItems + j) * 4;
			
			SetRectCenter(shardSelectQuads + index, rectSize, rectSize, Vector2f(j * rectSize + xSpacing * j, i * rectSize + ySpacing * i) + gridStart);
			SetRectSubRect(shardSelectQuads + index, ts_shards[0]->GetSubRect(i * xSelector->totalItems + j));
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

PNGSeq *&ShardMenu::GetCurrSeq()
{
	return shardSeq[xSelector->currIndex][ySelector->currIndex];
}

void ShardMenu::AsyncGetSequence(const std::string &str)
{
	PNGSeq *& ps = GetCurrSeq();
	ps = GetSequence(str);
	ps->spr.setPosition(imagePos);
}

void ShardMenu::sGetSequence( const std::string &str, ShardMenu *sMenu)
{
	sMenu->AsyncGetSequence(str);
}

boost::thread *&ShardMenu::GetCurrLoadThread()
{
	return seqLoadThread[xSelector->currIndex][ySelector->currIndex];
}

void ShardMenu::SetCurrSequence()
{
	std::string &name = shardNames[xSelector->currIndex][ySelector->currIndex];
	//sGetSequence(name, this);
	if (GetCurrSeq() == NULL && GetCurrLoadThread() == NULL )
	{
		GetCurrLoadThread() = new boost::thread(&(ShardMenu::sGetSequence), name, this);
	}
	
	//testSeq = GetSequence(name);//GetSequence("testanim");
	
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
		for (int j = 0; j < ySelector->totalItems; ++j)
		{
			if( shardSeq[i][j] != NULL )
				delete shardSeq[i][j];
		}
		delete[] shardSeq[i];
		delete[] seqLoadThread[i];
			
	}
	delete[] shardNames;
	delete[] shardDesc;
	delete[] shardSeq;
	delete[] seqLoadThread;
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
	//for (int i = 0; i < xSelector->totalItems; ++i)
	//{
	//	for (int j = 0; j < ySelector->totalItems; ++j)
	//	{
	//		index = (i * ySelector->totalItems + j) * 4;
	//		
	//		//SetRectColor(shardSelectQuads + index, c);
	//	}
	//}

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

	index = (xSelector->currIndex + xSelector->totalItems * ySelector->currIndex);

	selectedShardHighlight.setTextureRect(ts_shards[0]->GetSubRect(index + 22));
	selectedShardHighlight.setOrigin(selectedShardHighlight.getLocalBounds().width / 2,
		selectedShardHighlight.getLocalBounds().height / 2);


	//SetRectCenter(shardSelectQuads + index, rectSize, rectSize, Vector2f(i * rectSize + xSpacing * i, j * rectSize + ySpacing * j) + gridStart);

	selectedShardHighlight.setPosition((shardSelectQuads + index * 4)->position + Vector2f(24, 24));
	//SetRectColor(shardSelectQuads + index, currColor);

	
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
	boost::thread *& loadThread = GetCurrLoadThread();

	if (loadThread == NULL && GetCurrSeq() != NULL && currButtonState == S_SELECTED)
	{
		GetCurrSeq()->Update();
	}

	if (loadThread  != NULL && loadThread->try_join_for(boost::chrono::milliseconds(0)))
	{
		delete loadThread;
		loadThread = NULL;
	}

	UpdateShardSelectQuads();
}

void ShardMenu::Draw(sf::RenderTarget *target)
{
	if(GetCurrSeq() != NULL )
		GetCurrSeq()->Draw(target);
	target->draw(shardSelectQuads, 22 * 4/*xSelector->totalItems * ySelector->totalItems * 4*/,
		sf::Quads, ts_shards[0]->texture);
	target->draw(selectedShardHighlight);
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