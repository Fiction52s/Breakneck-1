//sfe::Movie m;
//assert(m.openFromFile("Movie/testvid.ogv"));
//m.fit(sf::FloatRect(0, 0, 1920, 1080));
//
//
//m.play();

#include "LogMenu.h"
#include <sstream>
#include <list>
#include <assert.h>
#include <iostream>
#include "ItemSelector.h"
#include "VectorMath.h"
#include "Enemy_LogItem.h"
#include "MainMenu.h"
#include "MusicSelector.h"
#include "VisualEffects.h"
#include "SaveFile.h"
#include "Session.h"
#include "Actor.h"


using namespace sf;
using namespace std;

LogMenu::LogMenu(Session *p_sess)
	:sess(p_sess)
{
	testParams = NULL;

	ActorType *at = NULL;
	//at = sess->types["crawler"];

	//testParams = at->info.pMaker(at, 1);

	//testParams->SetPosition(imagePos);
	//testParams->CreateMyEnemy();
	//testParams->myEnemy->UpdateFromEditParams(0);

	totalFrame = 0;
	currLogMusic = NULL;
	//currentMovie.scale(.5, .5);

	currLogText.setCharacterSize(20);
	currLogText.setFont(sess->mainMenu->arial);
	currLogText.setPosition(960, 740);

	currLogNameText.setCharacterSize(20);
	currLogNameText.setFont(sess->mainMenu->arial);
	currLogNameText.setPosition(Vector2f(825 + 401 / 2, 594 + 93 / 2));

	Vector2f controlCenter = Vector2f(1243 + 512 / 2, 594 + 93 / 2);

	SetRectCenter(logBGQuad, 744, 848, Vector2f(65 + 744 / 2, 66 + 848 / 2));
	SetRectCenter(shardTitleBGQuad, 401, 93, Vector2f(825 + 401 / 2, 594 + 93 / 2));
	SetRectCenter(controlsQuadBGQuad, 512, 93, controlCenter);
	SetRectCenter(descriptionBGQuad, 929, 211, Vector2f(825 + 929 / 2, 703 + 211 / 2));

	SetRectCenter(shardButtons, 128, 93, Vector2f(controlCenter.x - 128 - 128 / 2, controlCenter.y));
	SetRectCenter(shardButtons + 1 * 4, 128, 93, Vector2f(controlCenter.x - 128 / 2, controlCenter.y));
	SetRectCenter(shardButtons + 2 * 4, 128, 93, Vector2f(controlCenter.x + 128 / 2, controlCenter.y));
	SetRectCenter(shardButtons + 3 * 4, 128, 93, Vector2f(controlCenter.x + 128 + 128 / 2, controlCenter.y));


	SetRectCenter(containerBGQuad, 401, 512, Vector2f(825 + 401 / 2, 66 + 512 / 2));

	SetRectCenter(largeShardContainer, 401, 512, Vector2f(825 + 401 / 2, 66 + 512 / 2));
	SetRectCenter(largeShard, 192, 192, Vector2f(825 + 401 / 2, 66 + 512 / 2));

	SetRectColor(logBGQuad, Color(0, 0, 0, 128));
	SetRectColor(shardTitleBGQuad, Color(0, 0, 0, 128));
	SetRectColor(controlsQuadBGQuad, Color(0, 0, 0, 128));
	SetRectColor(descriptionBGQuad, Color(0, 0, 0, 128));
	SetRectColor(containerBGQuad, Color(0, 0, 0, 128));

	ts_shardContainer = sess->GetSizedTileset("Menu/shard_container_401x512.png");
	ts_sparkle = sess->GetSizedTileset("Menu/shard_sparkle_64x64.png");
	ts_notCapturedPreview = sess->GetSizedTileset("Menu/not_captured_512x512.png");
	ts_noPreview = sess->GetTileset("Menu/nopreview.png", 512, 512);
	ts_shardButtons = sess->GetSizedTileset("Menu/pause_shard_buttons_128x93.png");
	for (int i = 0; i < 4; ++i)
	{
		SetRectSubRect(shardButtons + i * 4, ts_shardButtons->GetSubRect(i));
	}


	SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(0));

	imagePos = Vector2f(1243, 66);
	previewSpr.setPosition(imagePos);

	

	state = WAIT;

	ts_logs = sess->GetSizedTileset("Enemies/poweritem_128x128.png");

	sparklePool = new EffectPool(EffectType::FX_REGULAR, 3, 1.f);
	sparklePool->ts = ts_sparkle;

	int waitFrames[3] = { 60, 20, 10 };
	int waitModeThresh[2] = { 2, 4 };
	int xSize = 11;
	int ySize = 14;

	logInfo = new LogDetailedInfo*[ySize];
	for (int i = 0; i < ySize; ++i)
	{
		logInfo[i] = new LogDetailedInfo[xSize];
	}

	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, xSize, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, ySize, 0);

	ts_preview = new Tileset*[xSize * ySize];
	for (int i = 0; i < xSize * ySize; ++i)
	{
		ts_preview[i] = NULL;
	}


	stringstream ss;

	LoadLogInfo();

	logSelectQuads = new sf::Vertex[xSize * ySize * 4];

	selectedLogHighlight.setTexture(*ts_logs->texture);

	int index = 0;
	int rectSize = 48;
	int xSpacing = 20;
	int ySpacing = 12;
	Vector2f gridStart(100, 100);
	for (int i = 0; i < ySelector->totalItems; ++i)
	{
		for (int j = 0; j < xSelector->totalItems; ++j)
		{
			index = (i * xSelector->totalItems + j);

			SetRectCenter(logSelectQuads + index * 4, rectSize, rectSize, Vector2f(j * rectSize + xSpacing * j, i * rectSize + ySpacing * i) + gridStart);
			SetRectSubRect(logSelectQuads + index * 4, ts_logs->GetSubRect(0));
		}
	}
}

LogMenu::~LogMenu()
{
	for (int i = 0; i < ySelector->totalItems; ++i)
	{
		delete[] logInfo[i];
	}
	delete[] ts_preview;

	delete[] logInfo;
	delete xSelector;
	delete ySelector;

	delete[] logSelectQuads;
	delete sparklePool;
}

void LogMenu::LoadLogInfo()
{
	for (int i = 0; i < 7; ++i)
	{
		ifstream is;
		is.open("Resources/Logs/loginfo" + to_string(i + 1) + ".txt");

		if (is.is_open())
		{
			string lineString;
			string typeString;
			string enemyString;
			string descriptionString;
			int index = 22 * i;
			int x;
			int y;

			int tile;


			while (getline(is, lineString))
			{
				x = index % 11;
				y = index / 11;

				LogDetailedInfo &currLog = logInfo[y][x];

				currLog.name = lineString;

				descriptionString = "";

				bool first = true;

				getline(is, typeString);

				if (typeString == "Enemy")
				{
					getline(is, enemyString);

					currLog.enemyTypeName = enemyString;
				}

				while (getline(is, lineString))
				{
					if (lineString == "")
					{
						index++;
						break;
					}
					else
					{
						if (first)
						{
							first = false;
						}
						else
						{
							descriptionString += "\n";
						}

						descriptionString += lineString;

					}
				}
				currLog.desc = descriptionString;
			}

			is.close();
		}
		else
		{
			cout << "couldnt open log info file" << endl;
			assert(0);
		}
	}

}

bool LogMenu::IsLogFound(int x, int y)
{
	return true; //testing

	SaveFile *saveFile = sess->mainMenu->GetCurrentProgress();
	if (saveFile == NULL)
	{
		return false;
	}

	LogDetailedInfo &currLog = logInfo[y][x];

	return (currLog.name != "" 
		&& saveFile->ShardIsCaptured(LogItem::GetLogTypeFromGrid(y, x)));
}

bool LogMenu::IsCurrLogFound()
{
	return IsLogFound(xSelector->currIndex, ySelector->currIndex);
}

void LogMenu::UpdateUnlockedLogs()
{
	int index = 0;
	SaveFile *saveFile = sess->mainMenu->GetCurrentProgress();

	if (saveFile == NULL)
		return;

	for (int i = 0; i < ySelector->totalItems; ++i)
	{
		for (int j = 0; j < xSelector->totalItems; ++j)
		{
			index = (i * xSelector->totalItems + j) * 4;

			//SetRectCenter(logSelectQuads + index, rectSize, rectSize, Vector2f(j * rectSize + xSpacing * j, i * rectSize + ySpacing * i) + gridStart);

			if (IsLogFound(j, i))
			{
				SetRectColor(logSelectQuads + index, Color(Color::White));
				//SetRectSubRect(logSelectQuads + index, ts_shards[0]->GetSubRect(i * xSelector->totalItems + j));
			}
			else
			{
				SetRectColor(logSelectQuads + index, Color(Color::Black));
				//SetRectSubRect(logSelectQuads + index, FloatRect());
			}
			//ts_shards[0]->GetSubRect(i * xSelector->totalItems + j));
		}
	}
}

bool LogMenu::SetDescription(std::string &nameStr, std::string &destStr, const std::string &shardTypeStr)
{
	stringstream ss;
	ss << "Resources/Shard/Descriptions/" << shardTypeStr << ".sdesc";
	ifstream is;
	is.open(ss.str());

	nameStr = "";
	destStr = "";
	if (is.is_open())
	{
		getline(is, nameStr);
		destStr.assign((std::istreambuf_iterator<char>(is)),
			(std::istreambuf_iterator<char>()));
		/*char c;
		while (is.get(c))
		{

		}*/

		//getline(is, destStr);
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

void LogMenu::SetCurrMusic()
{
	LogDetailedInfo &currLog = logInfo[ySelector->currIndex][xSelector->currIndex];

	assert(currLogMusic == NULL);
	currLogMusic = GetLogMusic(currLog.name);
	if (currLogMusic != NULL)
	{
		bool loaded = currLogMusic->music != NULL;
		if (currLogMusic->music == NULL)
			loaded = currLogMusic->Load();
		assert(loaded);
		currLogMusic->music->setVolume(100);
		currLogMusic->music->play();
	}
}

MusicInfo *LogMenu::GetLogMusic(const std::string &str)
{
	//stringstream ss;

	//ss.str("");
	//ss << "Shard/" << str << "_" << counter << ".png";
	//cout << "test: " << ss.str() << endl;
	return sess->mainMenu->musicManager->songMap["w02_Glade"];
	//sf::Music *m = mainMenu->musicManager->//LoadSong("Audio/Music/.ogg");
}

void LogMenu::SetupLogImages()
{

}

void LogMenu::UpdateLogSelectQuads()
{
	int index = 0;
	Color c = Color::Red;
	
	index = (xSelector->currIndex + xSelector->totalItems * ySelector->currIndex);

	//SetRectColor(logSelectQuads + index, currColor);
	int selectedShardTileIndex = -1;
	if (IsCurrLogFound())
	{
		SetRectSubRect(largeShard, ts_logs->GetSubRect(0));
		//selectedShardTileIndex = index + 22;
	}
	else
	{
		SetRectSubRect(largeShard, FloatRect());
		//selectedShardTileIndex = index + 44;
	}

	selectedLogHighlight.setScale(48 / 192.0, 48 / 192.0);
	selectedLogHighlight.setTexture(*ts_logs->texture);
	selectedLogHighlight.setTextureRect(ts_logs->GetSubRect(0));
	selectedLogHighlight.setOrigin(selectedLogHighlight.getLocalBounds().width / 2,
		selectedLogHighlight.getLocalBounds().height / 2);

	selectedLogHighlight.setPosition((logSelectQuads + index * 4)->position + Vector2f(24, 24));

	SetRectCenter(selectedBGQuad, 48, 48, Vector2f(selectedLogHighlight.getPosition()));
	SetRectColor(selectedBGQuad, Color::White);
}

void LogMenu::SetCurrentDescription(bool captured)
{
	LogDetailedInfo &currLog = logInfo[ySelector->currIndex][xSelector->currIndex];
	currLogNameText.setString(currLog.name);
	FloatRect lBounds = currLogNameText.getLocalBounds();
	currLogNameText.setOrigin(lBounds.left + lBounds.width / 2, lBounds.top + lBounds.height / 2);
	if (captured)
		currLogText.setString(currLog.desc);
}

std::string LogMenu::GetLogDesc(int w, int li)
{
	int x = li % 11;//li + (w % 2) * 11;
	int y = w * 2 + li / 11;

	LogDetailedInfo &currLog = logInfo[y][x];

	return currLog.desc;
}

std::string LogMenu::GetLogName(int w, int li)
{
	int x = li % 11;//li + (w % 2) * 11;
	int y = w * 2 + li / 11;

	LogDetailedInfo &currLog = logInfo[y][x];

	return currLog.name;
}

void LogMenu::StopMusic()
{
	if (currLogMusic != NULL)
	{
		currLogMusic->music->stop();
		currLogMusic = NULL;
	}
}

void LogMenu::SetCurrLog()
{
	bool captured = IsCurrLogFound();
	if (captured)
	{
		int index = xSelector->currIndex + ySelector->currIndex * xSelector->totalItems;
		selectedIndex = index;
		Tileset *tp = ts_preview[index];
		if (tp != NULL)
		{
			previewSpr.setTexture(*tp->texture);
			previewSpr.setTextureRect(tp->GetSubRect(0));
		}
		else
		{
			LogDetailedInfo &currInfo = logInfo[ySelector->currIndex][xSelector->currIndex];
			if (currInfo.enemyTypeName != "" )
			{
				ActorType *at;
				at = sess->types[currInfo.enemyTypeName];

				if (testParams != NULL)
				{
					delete testParams;
					tMan.ClearTilesets();
					sMan.ClearAll();
				}

				sess->specialTempTilesetManager = &tMan;
				sess->specialTempSoundManager = &sMan;
				testParams = at->info.pMaker(at, 1);
				

				testParams->SetPosition(imagePos + Vector2f( 256, 256 ));
				testParams->CreateMyEnemy();

				sess->specialTempTilesetManager = NULL;
				sess->specialTempSoundManager = NULL;

				testParams->myEnemy->UpdateFromEditParams(0);

				//currInfo.ts_preview = tMan.GetSizedTileset(currInfo.imageStr + ".png");
				//previewSpr.setTexture(*currInfo.ts_preview->texture);
				//previewSpr.setTextureRect(currInfo.ts_preview->GetSubRect(currInfo.tile));
			}
			else
			{
				if (testParams != NULL)
				{
					delete testParams;
					testParams = NULL;
				}
				 
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

void LogMenu::Update(ControllerState &currInput, ControllerState &prevInput)
{
	int oldX = xSelector->currIndex;
	int oldY = ySelector->currIndex;

	int xchanged = xSelector->UpdateIndex(currInput.LLeft(), currInput.LRight());
	int ychanged = ySelector->UpdateIndex(currInput.LUp(), currInput.LDown());

	bool currShardCap = IsCurrLogFound();

	if (xchanged != 0 || ychanged != 0)
	{
		//state = PAUSED;
		if (!currShardCap)
		{
			sparklePool->Reset();
		}
		StopMusic();
		state = WAIT;

		LogDetailedInfo &li = logInfo[oldY][oldX];
		if (li.ts_preview != NULL)
		{
			tMan.DestroyTileset(li.ts_preview);
			li.ts_preview = NULL;
		}
		//SetCurrSequence();
		SetCurrLog();
	}

	if (currInput.A && !prevInput.A)
	{
		if (currShardCap)
		{
			//if (state == WAIT)
			//{
			//	//state = LOADTOPLAY;
			//	//SetCurrSequence();
			//}
			//else if (state == PAUSED)
			//{
			//	//state = PLAYING;
			//	//SetCurrMusic();
			//}
			//else if (state == PLAYING)
			//{
			//	state = PAUSED;
			//	StopMusic();
			//}

		}
	}

	UpdateLogSelectQuads();


	if (currShardCap)
	{
		int lightningFactor = 7;//14;//8;
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect((totalFrame / lightningFactor) % 12));
	}
	else
	{
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(12));
	}


	sparklePool->Update();


	Vector2f sparkleCenter(825 + 401 / 2, 66 + 512 / 2);

	if (totalFrame % 120 == 0 && currShardCap)
	{
		Vector2f off(rand() % 101 - 50, rand() % 101 - 50);
		EffectInstance ei;

		int r = rand() % 3;
		if (r == 0)
		{
			ei.SetParams(sparkleCenter + off,
				Transform(Transform::Identity), 11, 5, 0);
		}
		else if (r == 1)
		{
			ei.SetParams(sparkleCenter + off,
				Transform(Transform::Identity), 10, 5, 11);
		}
		else if (r == 2)
		{
			ei.SetParams(sparkleCenter + off,
				Transform(Transform::Identity), 10, 5, 11);
		}

		//ei.pos = sparkleCenter;
		//ei.animFactor = 8;

		sparklePool->ActivateEffect(&ei);
	}

	if (testParams != NULL)
	{
		testParams->myEnemy->UpdateFromEditParams(1);
	}
	
	++totalFrame;
}

void LogMenu::SetLogTab()
{
	bool currShardCap = IsCurrLogFound();
	if (currShardCap)
	{
		int lightningFactor = 7;//8;
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect((totalFrame / lightningFactor) % 12));
	}
	else
	{
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(12));
	}
	UpdateUnlockedLogs();
	UpdateLogSelectQuads();
}

void LogMenu::Draw(sf::RenderTarget *target)
{
	target->draw(logBGQuad, 4, sf::Quads);
	target->draw(containerBGQuad, 4, sf::Quads);
	target->draw(descriptionBGQuad, 4, sf::Quads);
	target->draw(controlsQuadBGQuad, 4, sf::Quads);
	target->draw(shardTitleBGQuad, 4, sf::Quads);
	target->draw(largeShardContainer, 4, sf::Quads, ts_shardContainer->texture);
	target->draw(shardButtons, 4 * 4, sf::Quads, ts_shardButtons->texture);


	target->draw(largeShard, 4, sf::Quads, ts_logs->texture);
	sparklePool->Draw(target);
	//if (GetCurrSeq() != NULL)
	//	GetCurrSeq()->Draw(target);
	//else
	{
		//target->draw(previewSpr);

		if (testParams != NULL)
		{
			testParams->DrawEnemy(target);
		}
		
	}


	target->draw(selectedBGQuad, 4, sf::Quads);

	for (int i = 0; i < 7; ++i)
	{
		target->draw(logSelectQuads + 22 * 4 * i, 22 * 4,
			sf::Quads, ts_logs->texture);
	}



	/*target->draw(logSelectQuads, 22 * 4,
	sf::Quads, ts_shards[0]->texture);*/
	//target->draw(selectedShardHighlight);
	if (currLogText.getString() != "")
	{
		target->draw(currLogText);
	}
	if (currLogNameText.getString() != "")
	{
		target->draw(currLogNameText);
	}


	//target->draw(currentMovie);
}