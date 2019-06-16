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
#include "VisualEffects.h"
#include "SaveFile.h"


using namespace sf;
using namespace std;

ShardMenu::ShardMenu(MainMenu *mm)
	:mainMenu( mm )
{
	totalFrame = 0;
	currShardMusic = NULL;
	//currentMovie.scale(.5, .5);

	currShardText.setCharacterSize(20);
	currShardText.setFont(mm->arial);
	currShardText.setPosition(960, 740);

	currShardNameText.setCharacterSize(20);
	currShardNameText.setFont(mm->arial);
	currShardNameText.setPosition(Vector2f(825 + 401 / 2, 594 + 93 / 2));
	
	Vector2f controlCenter = Vector2f(1243 + 512 / 2, 594 + 93 / 2);

	SetRectCenter(shardBGQuad, 744, 848, Vector2f(65 + 744 / 2, 66 + 848 / 2));
	SetRectCenter(shardTitleBGQuad, 401, 93, Vector2f(825 + 401/ 2, 594 + 93 / 2));
	SetRectCenter(controlsQuadBGQuad, 512, 93, controlCenter );
	SetRectCenter(descriptionBGQuad, 929, 211, Vector2f(825 + 929 / 2, 703 + 211 / 2));
	
	SetRectCenter(shardButtons, 128, 93, Vector2f(controlCenter.x - 128 - 128 / 2, controlCenter.y));
	SetRectCenter(shardButtons + 1 * 4, 128, 93, Vector2f(controlCenter.x - 128 / 2, controlCenter.y));
	SetRectCenter(shardButtons + 2 * 4, 128, 93, Vector2f(controlCenter.x + 128 / 2, controlCenter.y));
	SetRectCenter(shardButtons + 3 * 4, 128, 93, Vector2f(controlCenter.x + 128 + 128 / 2, controlCenter.y));


	SetRectCenter(containerBGQuad, 401, 512, Vector2f(825 + 401 / 2, 66 + 512 / 2));

	SetRectCenter(largeShardContainer, 401, 512, Vector2f(825 + 401 / 2, 66 + 512 / 2));
	SetRectCenter(largeShard, 192, 192, Vector2f(825 + 401 / 2, 66 + 512 / 2));

	SetRectColor(shardBGQuad, Color(0, 0, 0, 128));
	SetRectColor(shardTitleBGQuad, Color(0, 0, 0, 128));
	SetRectColor(controlsQuadBGQuad, Color(0, 0, 0, 128));
	SetRectColor(descriptionBGQuad, Color(0, 0, 0, 128));
	SetRectColor(containerBGQuad, Color(0, 0, 0, 128));

	ts_shardContainer = mm->tilesetManager.GetTileset("Menu/shard_container_401x512.png", 401, 512);
	ts_sparkle = mm->tilesetManager.GetTileset("Menu/shard_sparkle_64x64.png", 64, 64);
	ts_bigShards = mm->tilesetManager.GetTileset("Shard/shards_w1_192x192.png", 192, 192);
	ts_notCapturedPreview = mm->tilesetManager.GetTileset("Menu/notcaptured.png", 512, 512);
	ts_noPreview = mm->tilesetManager.GetTileset("Menu/nopreview.png", 512, 512);
	ts_shardButtons = mm->tilesetManager.GetTileset("Menu/pause_shard_buttons_128x93.png", 128, 93);
	for (int i = 0; i < 4; ++i)
	{
		SetRectSubRect(shardButtons + i * 4, ts_shardButtons->GetSubRect(i));
	}
	

	SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(0));

	numShardsTotal = 1;
	shardQuads = new Vertex[numShardsTotal * 4];
	currButtonState = S_NEUTRAL;
	imagePos = Vector2f(1243, 66);
	previewSpr.setPosition(imagePos);

	state = PAUSED;

	ts_shards[0] = mm->tilesetManager.GetTileset("Shard/shards_w1_48x48.png", 48, 48);

	sparklePool = new EffectPool(EffectType::FX_REGULAR, 3, 1.f);
	sparklePool->ts = ts_sparkle;

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	int xSize = 11;
	int ySize = 2;//14;

	shardNames = new string*[xSize];
	shardDesc = new string*[xSize];
	shardDescriptionNames = new string*[xSize];
	seqLoadThread = new boost::thread**[xSize];
	shardSeq = new PNGSeq**[xSize];

	for (int i = 0; i < xSize; ++i)
	{
		shardNames[i] = new string[ySize];
		shardDesc[i] = new string[ySize];
		shardDescriptionNames[i] = new string[ySize];
		seqLoadThread[i] = new boost::thread*[ySize];
		shardSeq[i] = new PNGSeq*[ySize];
	}

	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, xSize, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, ySize, 0);

	ts_preview = new Tileset*[xSize * ySize];

	stringstream ss;

	shardNames[0][0] = Shard::GetShardString(ShardType::SHARD_W1_TEACH_JUMP);
	shardNames[1][0] = Shard::GetShardString(ShardType::SHARD_W1_BACKWARDS_DASH_JUMP);

	for (int x = 0; x < xSize; ++x)
	{
		for (int y = 0; y < ySize; ++y)
		{
			ss.str("");
			shardSeq[x][y] = NULL;
			seqLoadThread[x][y] = NULL;
			

			std::string &currShardName = shardNames[x][y];

			if (currShardName == "")
			{
				ts_preview[x + y * xSize] = NULL;
			}
			else
			{
				ss << "Shard/" << currShardName << "_preview.png";
				ts_preview[x + y * xSize] = mainMenu->tilesetManager.GetTileset(ss.str(), 512, 512);
			}
			
			
			//currShardName = Shard::GetShardString(ShardType::SHARD_W1_TEACH_JUMP);
			if(currShardName != "" )
				SetDescription(shardDescriptionNames[x][y], shardDesc[x][y], shardNames[x][y]);
		}
	}

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

ShardMenu::~ShardMenu()
{
	for (int i = 0; i < xSelector->totalItems; ++i)
	{
		delete[] shardNames[i];
		delete[] shardDesc[i];
		for (int j = 0; j < ySelector->totalItems; ++j)
		{
			if (shardSeq[i][j] != NULL)
				delete shardSeq[i][j];
		}
		delete[] shardDescriptionNames[i];
		delete[] shardSeq[i];
		delete[] seqLoadThread[i];

	}
	delete[] ts_preview;

	delete[] shardNames;
	delete[] shardDesc;
	delete[] shardSeq;
	delete[] shardDescriptionNames;
	delete[] seqLoadThread;
	delete xSelector;
	delete ySelector;

	delete[] shardQuads;
	delete[] shardSelectQuads;
	delete sparklePool;
}

bool ShardMenu::IsShardCaptured( int x, int y )
{
	SaveFile *saveFile = mainMenu->GetCurrentProgress();
	return (shardNames[x][y] != "" && saveFile->ShardIsCaptured(Shard::GetShardType(shardNames[x][y])));
}

bool ShardMenu::IsCurrShardCaptured()
{
	return IsShardCaptured(xSelector->currIndex, ySelector->currIndex);
}

void ShardMenu::UpdateUnlockedShards()
{
	int index = 0;
	SaveFile *saveFile = mainMenu->GetCurrentProgress();

	if (saveFile == NULL)
		return;

	for (int i = 0; i < ySelector->totalItems; ++i)
	{
		for (int j = 0; j < xSelector->totalItems; ++j)
		{
			index = (i * xSelector->totalItems + j) * 4;

			//SetRectCenter(shardSelectQuads + index, rectSize, rectSize, Vector2f(j * rectSize + xSpacing * j, i * rectSize + ySpacing * i) + gridStart);
			
			if (IsShardCaptured( j, i ))
			{
				SetRectColor(shardSelectQuads + index, Color(Color::White));
				//SetRectSubRect(shardSelectQuads + index, ts_shards[0]->GetSubRect(i * xSelector->totalItems + j));
			}
			else
			{
				SetRectColor(shardSelectQuads + index, Color(Color::Black));
				//SetRectSubRect(shardSelectQuads + index, FloatRect());
			}
			//ts_shards[0]->GetSubRect(i * xSelector->totalItems + j));
		}
	}
}

bool ShardMenu::SetDescription( std::string &nameStr, std::string &destStr, const std::string &shardTypeStr)
{
	stringstream ss;
	ss << "Resources/Shard/" << shardTypeStr << ".sdesc";
	ifstream is;
	is.open(ss.str());

	nameStr = "";
	destStr = "";
	if( is.is_open())
	{
		getline(is, nameStr);
		getline(is, destStr);
		/*is.seekg(0, std::ios::end);
		destStr.reserve(is.tellg());
		is.seekg(0, std::ios::beg);

		destStr.assign((std::istreambuf_iterator<char>(is)),
			std::istreambuf_iterator<char>());*/
		/*if (!is.good())
		{
			assert(is.good());
		}*/
		
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

	if( ps == NULL )
		ps = GetSequence(str);

	if (ps != NULL)
	{
		ps->spr.setPosition(imagePos);
	}
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



PNGSeq * ShardMenu::GetSequence(const std::string &str)
{
	if (str == "")
	{
		return NULL;
	}
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

	//assert(counter > 0);
	
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

	if (IsCurrShardCaptured())
	{
		SetRectSubRect(largeShard, ts_bigShards->GetSubRect(index));
	}
	else
	{
		SetRectSubRect(largeShard, FloatRect());
	}
	
	
}

void ShardMenu::SetCurrentDescription( bool captured)
{
	currShardNameText.setString(shardDescriptionNames[xSelector->currIndex][ySelector->currIndex]);
	FloatRect lBounds = currShardNameText.getLocalBounds();
	currShardNameText.setOrigin(lBounds.left + lBounds.width / 2, lBounds.top + lBounds.height / 2);
	if( captured )
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

void ShardMenu::SetCurrShard()
{
	bool captured = IsCurrShardCaptured();
	if (captured)
	{
		
		PNGSeq *seq = GetCurrSeq();
		if (seq != NULL)
		{
			seq->Reset();
		}
		else
		{
			int index = xSelector->currIndex + ySelector->currIndex * xSelector->totalItems;
			Tileset *tp = ts_preview[index];
			if (tp != NULL)
			{
				previewSpr.setTexture( *tp->texture);
				previewSpr.setTextureRect(tp->GetSubRect(0));
			}
			else
			{
				previewSpr.setTexture(*ts_noPreview->texture);
				previewSpr.setTextureRect(ts_noPreview->GetSubRect(0));
			}
		}
	}
	else
	{
		previewSpr.setTexture(*ts_notCapturedPreview->texture);
		previewSpr.setTextureRect(ts_notCapturedPreview->GetSubRect(0));
	}
	SetCurrentDescription(captured);
}

void ShardMenu::Update( ControllerState &currInput, ControllerState &prevInput )
{
	int xchanged = xSelector->UpdateIndex(currInput.LLeft(), currInput.LRight());
	int ychanged = ySelector->UpdateIndex(currInput.LUp(), currInput.LDown());

	bool currShardCap = IsCurrShardCaptured();

	if (xchanged != 0 || ychanged != 0 )
	{
		//state = PAUSED;
		if (!currShardCap)
		{
			sparklePool->Reset();
		}
		currButtonState = S_NEUTRAL;
		StopMusic();
		state = WAIT;

		
		//SetCurrSequence();
		SetCurrShard();
	}

	if (currInput.A && !prevInput.A)
	{
		if (currShardCap)
		{
			if (state == WAIT)
			{
				state = LOADTOPLAY;
				SetCurrSequence();
			}
			else if (state == PAUSED)
			{
				state = PLAYING;
				SetCurrMusic();
			}
			else if (state == PLAYING)
			{
				state = PAUSED;
				StopMusic();
			}

		}
	}

	PNGSeq* seq = GetCurrSeq();
	if (seq != NULL)
	{
		if (currInput.X && !prevInput.X)
		{
			seq->Reset();
		}
		else if ( state == PAUSED && currInput.PLeft() && !prevInput.PLeft())
		{
			seq->DecrementFrame();
		}
		else if (state == PAUSED && currInput.PRight() && !prevInput.PRight())
		{
			seq->IncrementFrame();
		}
	}
	boost::thread *& loadThread = GetCurrLoadThread();

	if (loadThread == NULL && seq != NULL && state == LOADTOPLAY )//currButtonState == S_SELECTED)
	{
		state = PLAYING;
		SetCurrMusic();
	}

	if (state == PLAYING && loadThread == NULL && seq != NULL)
	{
		GetCurrSeq()->Update();
	}

	if (loadThread  != NULL && loadThread->try_join_for(boost::chrono::milliseconds(0)))
	{
		delete loadThread;
		loadThread = NULL;
	}

	UpdateShardSelectQuads();

	
	if (currShardCap)
	{
		int lightningFactor = 8;
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect((totalFrame / lightningFactor) % 12));
	}
	else
	{
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(12));
	}
	

	sparklePool->Update( );


	Vector2f sparkleCenter(825 + 401 / 2, 66 + 512 / 2);
	
	if (totalFrame % 120 == 0 && currShardCap)
	{
		Vector2f off(rand() % 101 - 50, rand() % 101 - 50);
		EffectInstance ei;

		int r = rand() % 3;
		if (r == 0)
		{
			ei.SetParams(sparkleCenter + off,
				Transform(Transform::Identity), 11 , 5, 0);
		}
		else if (r == 1)
		{
			ei.SetParams(sparkleCenter + off,
				Transform(Transform::Identity), 10, 5, 11);
		}
		else if (r == 2)
		{
			ei.SetParams(sparkleCenter+ off,
				Transform(Transform::Identity), 10, 5, 11);
		}
		
		//ei.pos = sparkleCenter;
		//ei.animFactor = 8;
		
		sparklePool->ActivateEffect(&ei);
	}

	++totalFrame;
}

void ShardMenu::SetShardTab()
{
	bool currShardCap = IsCurrShardCaptured();
	if (currShardCap)
	{
		int lightningFactor = 8;
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect((totalFrame / lightningFactor) % 12));
	}
	else
	{
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(12));
	}
	UpdateUnlockedShards();
	UpdateShardSelectQuads();
}

void ShardMenu::Draw(sf::RenderTarget *target)
{
	target->draw(shardBGQuad, 4, sf::Quads);
	target->draw(containerBGQuad, 4, sf::Quads);
	target->draw(descriptionBGQuad, 4, sf::Quads);
	target->draw(controlsQuadBGQuad, 4, sf::Quads);
	target->draw(shardTitleBGQuad, 4, sf::Quads);
	target->draw(largeShardContainer, 4, sf::Quads, ts_shardContainer->texture);
	target->draw(shardButtons, 4 * 4, sf::Quads, ts_shardButtons->texture);
	

	target->draw(largeShard, 4, sf::Quads, ts_bigShards->texture);
	sparklePool->Draw(target);
	if(GetCurrSeq() != NULL )
		GetCurrSeq()->Draw(target);
	else
	{
		target->draw(previewSpr);
	}
	target->draw(shardSelectQuads, 22 * 4/*xSelector->totalItems * ySelector->totalItems * 4*/,
		sf::Quads, ts_shards[0]->texture);
	target->draw(selectedShardHighlight);
	if (currShardText.getString() != "")
	{
		target->draw(currShardText);
	}
	if (currShardNameText.getString() != "")
	{
		target->draw(currShardNameText);
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
	//load preview image

	Reset();
}

void PNGSeq::IncrementFrame()
{
	if (singleImage)
		return;

	tileIndex++;

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

	
}

void PNGSeq::DecrementFrame()
{
	if (singleImage)
		return;

	tileIndex--;

	if (tileIndex < 0)
	{
		--setIndex;
		if (setIndex < 0)
		{
			setIndex = numSets - 1;
		}

		int rem = (numFrames-1) % 16;
		tileIndex = rem;

		spr.setTexture(*tSets[setIndex]->texture);
	}

	spr.setTextureRect(tSets[setIndex]->GetSubRect(tileIndex));
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
	if (singleImage)
		return;

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

	tileIndex++;
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

//Sparkle::Sparkle(Tileset *p_ts, int type, Vector2f &p_pos)
//	:ts( p_ts ), pos( p_pos )
//{
//
//}