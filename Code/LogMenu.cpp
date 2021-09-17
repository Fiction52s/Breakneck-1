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
#include "EditorTerrain.h"
#include "EditorRail.h"

using namespace sf;
using namespace std;

LogMenu::LogMenu(Session *p_sess)
	:sess(p_sess), pSkinShader("player"), pFaceSkinShader( "player")
{
	previewParams = NULL;

	waterShaderCounter = 0;
	totalFrame = 0;
	currLogMusic = NULL;

	currLogText.setCharacterSize(20);
	currLogText.setFont(sess->mainMenu->arial);
	

	worldText.setFont(sess->mainMenu->arial);
	worldText.setCharacterSize(40);
	worldText.setFillColor(Color::White);

	currLogNameText.setCharacterSize(36);
	currLogNameText.setFont(sess->mainMenu->arial);

	int edgeMargin = 65;

	Vector2f pauseTexSize(1820, 980);

	Vector2f controlCenter = Vector2f(1243 + 512 / 2, 594 + 93 / 2);

	float logBGQuadWidth = 740;

	//worldText.setPosition(edgeMargin + logBGQuadWidth / 2, 100);

	Vector2f logBGQuadTopLeft = Vector2f(edgeMargin, edgeMargin);

	SetRectTopLeft(logBGQuad, logBGQuadWidth, pauseTexSize.y - edgeMargin * 2, logBGQuadTopLeft);

	float rightOfLogBGQuad = logBGQuadTopLeft.x + logBGQuadWidth;

	float leftSideMiddle = logBGQuadTopLeft.x + logBGQuadWidth / 2;

	worldText.setPosition(leftSideMiddle, 80);

	float separationMargin = 15;
	float rightSideStartX = rightOfLogBGQuad + separationMargin;

	float rightSideWidth = (pauseTexSize.x - edgeMargin) - rightSideStartX;
	float rightSideCenterX = rightSideStartX + rightSideWidth / 2;




	float containerBGQuadHeight = 512;

	Vector2f containerBGQuadPos = Vector2f(rightSideStartX, edgeMargin);

	SetRectTopLeft(containerBGQuad, rightSideWidth, containerBGQuadHeight, containerBGQuadPos);

	previewCenter = containerBGQuadPos + Vector2f(rightSideWidth / 2, containerBGQuadHeight / 2);

	float containerQuadBottom = containerBGQuadPos.y + containerBGQuadHeight;

	float nameQuadHeight = 93;
	float descriptionQuadHeight = 215;

	Vector2f logTitleBGQuadPos = Vector2f(rightSideStartX, containerQuadBottom + separationMargin);
	Vector2f logDescriptionBGQuadPos = Vector2f(rightSideStartX, containerQuadBottom + separationMargin + nameQuadHeight + separationMargin);

	SetRectTopLeft(shardTitleBGQuad, rightSideWidth, nameQuadHeight, logTitleBGQuadPos);
	SetRectTopLeft(descriptionBGQuad, rightSideWidth, descriptionQuadHeight, logDescriptionBGQuadPos);


	currLogNameText.setPosition(Vector2f(rightSideCenterX, logTitleBGQuadPos.y + nameQuadHeight / 2));

	currLogText.setPosition(logDescriptionBGQuadPos + Vector2f(edgeMargin, edgeMargin));
	

	/*SetRectCenter(shardButtons, 128, 93, Vector2f(controlCenter.x - 128 - 128 / 2, controlCenter.y));
	SetRectCenter(shardButtons + 1 * 4, 128, 93, Vector2f(controlCenter.x - 128 / 2, controlCenter.y));
	SetRectCenter(shardButtons + 2 * 4, 128, 93, Vector2f(controlCenter.x + 128 / 2, controlCenter.y));
	SetRectCenter(shardButtons + 3 * 4, 128, 93, Vector2f(controlCenter.x + 128 + 128 / 2, controlCenter.y));*/

	SetRectColor(logBGQuad, Color(0, 0, 0, 128));
	SetRectColor(shardTitleBGQuad, Color(0, 0, 0, 128));
	SetRectColor(controlsQuadBGQuad, Color(0, 0, 0, 128));
	SetRectColor(descriptionBGQuad, Color(0, 0, 0, 128));
	SetRectColor(containerBGQuad, Color(0, 0, 0, 128));

	ts_notCapturedPreview = sess->GetSizedTileset("Menu/not_captured_512x512.png");
	ts_noPreview = sess->GetTileset("Menu/nopreview.png", 512, 512);
	ts_shardButtons = sess->GetSizedTileset("Menu/pause_shard_buttons_128x93.png");
	for (int i = 0; i < 4; ++i)
	{
		SetRectSubRect(shardButtons + i * 4, ts_shardButtons->GetSubRect(i));
	}
	previewSpr.setPosition(previewCenter);


	ts_grass = sess->GetSizedTileset("Env/grass_128x128.png");
	grassSprite.setTexture(*ts_grass->texture);
	grassSprite.setPosition(previewCenter);

	ts_kin = sess->GetSizedTileset("Menu/pause_kin_400x836.png");
	kinSprite.setTexture(*ts_kin->texture);
	kinSprite.setScale(.5, .5);
	kinSprite.setPosition(previewCenter + Vector2f( -100, 0 ));
	kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2, kinSprite.getLocalBounds().height / 2);

	pSkinShader.SetSubRect(ts_kin, ts_kin->GetSubRect(0));
	

	ts_kinFace = sess->GetSizedTileset("HUD/kin_face_320x288.png");
	kinFaceSprite.setTexture(*ts_kinFace->texture);
	kinFaceSprite.setTextureRect(ts_kinFace->GetSubRect(0));
	kinFaceSprite.setPosition(previewCenter + Vector2f(100, 0));
	kinFaceSprite.setOrigin(kinFaceSprite.getLocalBounds().width / 2, kinFaceSprite.getLocalBounds().height / 2);

	pFaceSkinShader.SetSubRect(ts_kinFace, ts_kinFace->GetSubRect(0));

	state = WAIT;

	ts_logs = sess->GetSizedTileset("Logs/logs_64x64.png");

	int waitFrames[3] = { 60, 20, 10 };
	int waitModeThresh[2] = { 2, 4 };
	int xSize = 5;
	int ySize = 7 + 1; //+1 for world selector

	numWorlds = 7;
	logInfo = new LogDetailedInfo*[numWorlds];
	for (int i = 0; i < numWorlds; ++i)
	{
		logInfo[i] = new LogDetailedInfo[LogInfo::MAX_LOGS_PER_WORLD];
	}

	worldSelector = new SingleAxisSelector( 3, waitFrames, 2, waitModeThresh, numWorlds, 0 );
	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, xSize, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, ySize, 0);


	int waterWidth = 400;
	int waterHeight = 400;
	previewPoly = new TerrainPolygon;
	previewPoly->AddPoint(Vector2i(previewCenter + Vector2f(-waterWidth / 2, -waterHeight / 2)), false);
	previewPoly->AddPoint(Vector2i(previewCenter + Vector2f(waterWidth / 2, -waterHeight / 2)), false);
	previewPoly->AddPoint(Vector2i(previewCenter + Vector2f(waterWidth/2, waterHeight/2)), false);
	previewPoly->AddPoint(Vector2i(previewCenter + Vector2f(-waterWidth/2, waterHeight/2)), false);
	previewPoly->SetAsWaterType(TerrainPolygon::WATER_NORMAL);
	previewPoly->Finalize();

	previewRail = new TerrainRail;
	previewRail->AddPoint(Vector2i(previewCenter + Vector2f( -150, -150 )), false);
	previewRail->AddPoint(Vector2i(previewCenter + Vector2f(150, 150)), false);

	previewRail->SetRailType(TerrainRail::FLOORANDCEILING);
	previewRail->Finalize();

	LoadLogInfo();

	logSelectQuads = new sf::Vertex[xSize * ySize * 4];

	int index = 0;
	int rectSize = 192/2;
	int xSpacing = 20 * 2;
	int ySpacing = 12 * 2;

	Vector2f gridStart(150, 200);
	for (int i = 0; i < ySelector->totalItems - 1; ++i)
	{
		for (int j = 0; j < xSelector->totalItems; ++j)
		{
			index = (i * xSelector->totalItems + j);

			SetRectCenter(logSelectQuads + index * 4, rectSize, rectSize, Vector2f(j * rectSize + xSpacing * j, i * rectSize + ySpacing * i) + gridStart);
			SetRectSubRect(logSelectQuads + index * 4, ts_logs->GetSubRect(0));
		}
	}

	
	SetWorldMode();
	UpdateLogsOnWorldChange();
}

LogMenu::~LogMenu()
{
	for (int i = 0; i < numWorlds; ++i)
	{
		delete[] logInfo[i];
	}

	delete[] logInfo;
	delete xSelector;
	delete ySelector;
	delete worldSelector;

	delete[] logSelectQuads;

	delete previewPoly;
	delete previewRail;
}

void LogMenu::LoadLogInfo()
{
	for (int i = 0; i < numWorlds; ++i)
	{
		ifstream is;
		is.open("Resources/Logs/loginfo" + to_string(i + 1) + ".txt");

		if (is.is_open())
		{
			string lineString;
			string typeString;
			string detailString;
			string skinString;
			string enemyString;
			string waterString;
			string railString;
			string descriptionString;
			string specialTerrainString;
			int index = 0;//LogInfo::MAX_LOGS_PER_WORLD * i;

			int currWorld = i;
			int x;
			int y;

			int tile;


			while (getline(is, lineString))
			{
				if (lineString == "")
				{
					continue;
				}

				LogDetailedInfo &currLog = logInfo[currWorld][index];

				currLog.name = lineString;

				descriptionString = "";

				bool first = true;

				getline(is, typeString);

				if (typeString == "Enemy")
				{
					getline(is, detailString);

					currLog.logType = LogDetailedInfo::LT_ENEMY;
					currLog.enemyTypeName = detailString;
				}
				else if (typeString == "Water")
				{
					getline(is, detailString);

					currLog.logType = LogDetailedInfo::LT_WATER;
					currLog.waterIndex = TerrainPolygon::GetWaterIndexFromString(detailString);
				}
				else if (typeString == "Rail")
				{
					getline(is, detailString);
					currLog.logType = LogDetailedInfo::LT_RAIL;
					currLog.railIndex = TerrainRail::GetRailIndexFromString(detailString);
				}
				else if (typeString == "Grass")
				{
					getline(is, detailString);
					currLog.logType = LogDetailedInfo::LT_GRASS;
					currLog.grassTypeIndex = Grass::GetGrassTypeFromString(detailString);
				}
				else if (typeString == "SpecialTerrain")
				{
					getline(is, specialTerrainString);
					currLog.logType = LogDetailedInfo::LT_SPECIALTERRAIN;

					if (specialTerrainString == "Phase")
					{

					}
					else if (specialTerrainString == "InversePhase")
					{
						
					}
					else if (specialTerrainString == "Fade")
					{

					}
				}
				else if (typeString == "Skin")
				{
					getline(is, skinString);
					currLog.logType = LogDetailedInfo::LT_SKIN;
					currLog.skinIndex = Actor::GetSkinIndexFromString(skinString);
				}
				
				if (currLog.logType == LogDetailedInfo::LT_SKIN)
				{
					descriptionString = "Special costume for Kin";
				}
				else
				{
					while (getline(is, lineString))
					{
						if (lineString == "")
						{
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
				}
				index++;
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

bool LogMenu::IsLogFound(int w, int li)
{
	return true; //testing

	SaveFile *saveFile = sess->mainMenu->GetCurrentProgress();
	if (saveFile == NULL)
	{
		return false;
	}

	LogDetailedInfo &currLog = logInfo[w][li];

	return (currLog.name != "" 
		&& saveFile->HasLog(LogItem::GetLogTypeFromWorldAndIndex(w, li)));
}

bool LogMenu::IsCurrLogFound()
{
	return IsLogFound(worldSelector->currIndex, selectedIndex);
}

void LogMenu::UpdateUnlockedLogs()
{
	int index = 0;
	SaveFile *saveFile = sess->mainMenu->GetCurrentProgress();

	if (saveFile == NULL)
		return;

	for (int i = 0; i < ySelector->totalItems-1; ++i)
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

int LogMenu::GetLogTile(int w, int li)
{
	LogDetailedInfo &currInfo = logInfo[w][li];

	if (currInfo.logType == -1)
	{
		return -1;
		//SetRectSubRect(logSelectQuads + index * 4, FloatRect());
	}
	else
	{
		int tileIndex = 0;
		switch (currInfo.logType)
		{
		case LogDetailedInfo::LT_MOMENTA:
			tileIndex = 8;
			break;
		case LogDetailedInfo::LT_SKIN:
			tileIndex = 9;
			break;
		case LogDetailedInfo::LT_ENEMY:
			tileIndex = w;
			break;
		case LogDetailedInfo::LT_WATER:
			tileIndex = 13;
			break;
		case LogDetailedInfo::LT_RAIL:
			tileIndex = 14;
			break;
		case LogDetailedInfo::LT_GRASS:
			tileIndex = 12;
			break;
		case LogDetailedInfo::LT_SPECIALTERRAIN:
			tileIndex = 11;
			break;
		case LogDetailedInfo::LT_MUSIC:
			tileIndex = 10;
			break;
		case LogDetailedInfo::LT_LORE:
			break;
		}

		return tileIndex;
	}
}

void LogMenu::UpdateLogsOnWorldChange()
{
	int index = 0;
	for (int i = 1; i < ySelector->totalItems; ++i)
	{
		for (int j = 0; j < xSelector->totalItems; ++j)
		{
			index = ((i - 1) * xSelector->totalItems + j);

			int tile = GetLogTile(worldSelector->currIndex, index);

			if (tile == -1)
			{
				SetRectSubRect(logSelectQuads + index * 4, FloatRect());
			}
			else
			{
				SetRectSubRect(logSelectQuads + index * 4, ts_logs->GetSubRect(tile));
			}
		}
	}

	worldText.setString("World " + to_string(worldSelector->currIndex + 1));
	sf::FloatRect localBounds = worldText.getLocalBounds();
	worldText.setOrigin(localBounds.left + localBounds.width / 2, 0);
}

void LogMenu::UpdateLogSelectQuads()
{
	int index = (ySelector->currIndex - 1) * xSelector->totalItems + xSelector->currIndex;

	float quadSize = 192/2;
	SetRectCenter(selectedBGQuad, quadSize, quadSize,
		Vector2f((logSelectQuads + index * 4)->position + Vector2f(quadSize/2, quadSize/2)));
	SetRectColor(selectedBGQuad, Color::White);
}

void LogMenu::SetCurrentDescription(bool captured)
{
	LogDetailedInfo &currLog = logInfo[worldSelector->currIndex][selectedIndex];
	currLogNameText.setString(currLog.name);
	FloatRect lBounds = currLogNameText.getLocalBounds();
	currLogNameText.setOrigin(lBounds.left + lBounds.width / 2, lBounds.top + lBounds.height / 2);
	if (captured)
		currLogText.setString(currLog.desc);
}

std::string LogMenu::GetLogDesc(int w, int li)
{
	//int x = li % xSelector->totalItems;
	//int y = w * 2 + li / xSelector->totalItems;

	LogDetailedInfo &currLog = logInfo[w][li];

	return currLog.desc;
}

std::string LogMenu::GetLogName(int w, int li)
{
	//int x = li % 11;//li + (w % 2) * 11;
	//int y = w * 2 + li / 11;

	LogDetailedInfo &currLog = logInfo[w][li];

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
	if (currSelectMode == SM_WORLD)
		return;

	bool captured = IsCurrLogFound();
	if (captured)
	{
		int index = xSelector->currIndex + (ySelector->currIndex-1) * xSelector->totalItems;
		selectedIndex = index;
		Tileset *tp = NULL;//ts_preview[index];
		if (tp != NULL)
		{
			previewSpr.setTexture(*tp->texture);
			previewSpr.setTextureRect(tp->GetSubRect(0));
		}
		else
		{
			LogDetailedInfo &currInfo = logInfo[worldSelector->currIndex][selectedIndex];

			if (previewParams != NULL)
			{
				delete previewParams;
				previewParams = NULL;
				tMan.ClearTilesets();
				sMan.ClearAll();
			}

			currLogType = currInfo.logType;

			if (currLogType == LogDetailedInfo::LT_ENEMY )
			{
				ActorType *at;
				at = sess->types[currInfo.enemyTypeName];

				

				sess->specialTempTilesetManager = &tMan;
				sess->specialTempSoundManager = &sMan;
				previewParams = at->info.pMaker(at, 1);

				previewParams->SetPosition(previewCenter);
				previewParams->CreateMyEnemy();

				sess->specialTempTilesetManager = NULL;
				sess->specialTempSoundManager = NULL;

				previewParams->myEnemy->UpdateFromEditParams(0);

				//currInfo.ts_preview = tMan.GetSizedTileset(currInfo.imageStr + ".png");
				//previewSpr.setTexture(*currInfo.ts_preview->texture);
				//previewSpr.setTextureRect(currInfo.ts_preview->GetSubRect(currInfo.tile));
			}
			else if (currLogType == LogDetailedInfo::LT_WATER)
			{
				previewPoly->SetAsWaterType(currInfo.waterIndex);
			}
			else if (currLogType == LogDetailedInfo::LT_RAIL)
			{
				previewRail->SetRailType(currInfo.railIndex);
				previewRail->UpdateTexturedQuads();
			}
			else if (currLogType == LogDetailedInfo::LT_SKIN)
			{
				pSkinShader.SetSkin(currInfo.skinIndex);
				pFaceSkinShader.SetSkin(currInfo.skinIndex);
			}
			else if (currLogType == LogDetailedInfo::LT_GRASS)
			{
				grassSprite.setTextureRect(ts_grass->GetSubRect(currInfo.grassTypeIndex));
				grassSprite.setOrigin(grassSprite.getLocalBounds().width / 2, grassSprite.getLocalBounds().height / 2);
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

void LogMenu::Update(ControllerState &currInput, ControllerState &prevInput)
{
	int oldX = xSelector->currIndex;
	int oldY = ySelector->currIndex;

	int testIndex;

	if (currSelectMode == SM_WORLD)
	{
		int worldChanged = worldSelector->UpdateIndex(currInput.LLeft(), currInput.LRight());
		int ychanged = ySelector->UpdateIndex(currInput.LUp(), currInput.LDown());

		if (worldChanged != 0)
		{
			xSelector->currIndex = 0;
			UpdateLogsOnWorldChange();
		}

		if (ychanged != 0 )
		{
			currSelectMode = SM_LOG;
			SetCurrLog();

			//worldText.setOutlineColor(Color::Red);
			worldText.setOutlineThickness(0);
		
			int index = (ySelector->currIndex - 1) * xSelector->totalItems + xSelector->currIndex;
			LogDetailedInfo &currLog = logInfo[worldSelector->currIndex][index];

			if (currLog.logType == -1)
			{
				for (int i = ySelector->totalItems - 1; i >= 1; --i)
				{
					ySelector->currIndex--;
					testIndex = (ySelector->currIndex - 1) * xSelector->totalItems + xSelector->currIndex;
					LogDetailedInfo &testLog = logInfo[worldSelector->currIndex][testIndex];
					if (testLog.logType != -1)
					{
						break;
					}
				}
			}

			UpdateLogSelectQuads();
		}
	}
	else if (currSelectMode == SM_LOG)
	{
		int xchanged = xSelector->UpdateIndex(currInput.LLeft(), currInput.LRight());
		int ychanged = ySelector->UpdateIndex(currInput.LUp(), currInput.LDown());

		bool currLogFound = IsCurrLogFound();

		int index = (ySelector->currIndex - 1) * xSelector->totalItems + xSelector->currIndex;
		LogDetailedInfo &currLog = logInfo[worldSelector->currIndex][index];

		if (ychanged > 0)
		{
			if (currLog.logType == -1)
			{
				ySelector->currIndex = 0;
			}
		}

		if (ySelector->currIndex == 0)
		{
			SetWorldMode();
			StopMusic();
			state = WAIT;
		}
		else
		{
			if (xchanged != 0 || ychanged != 0)
			{
				if (currLog.logType == -1)
				{
					if (xchanged > 0)
					{
						xSelector->currIndex = 0;
					}
					else if (xchanged < 0)
					{
						for (int i = xSelector->totalItems - 1; i >= 0; --i)
						{
							xSelector->currIndex--;
							testIndex = (ySelector->currIndex - 1) * xSelector->totalItems + xSelector->currIndex;
							LogDetailedInfo &testLog = logInfo[worldSelector->currIndex][testIndex];
							if (testLog.logType != -1)
							{
								break;
							}
						}
					}
				}

				StopMusic();
				state = WAIT;

				/*LogDetailedInfo &li = logInfo[oldY][oldX];
				if (li.ts_preview != NULL)
				{
					tMan.DestroyTileset(li.ts_preview);
					li.ts_preview = NULL;
				}*/
				//SetCurrSequence();
				SetCurrLog();
			}

			if (currInput.A && !prevInput.A)
			{
				if (currLogFound)
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


			if (previewParams != NULL)
			{
				previewParams->myEnemy->UpdateFromEditParams(1);
			}

			if (currLogType == LogDetailedInfo::LT_WATER)
			{
				//set oldshaderzoom when you pause the game, so that if it gets changed
				//it updates when you return to the game
				//oldShaderZoom = zoom;

				for (int i = 0; i < TerrainPolygon::WATER_Count; ++i)
				{
					sess->waterShaders[i].setUniform("u_slide", waterShaderCounter);
					sess->waterShaders[i].setUniform("zoom", 1.f);
				}
				waterShaderCounter += .01f;
			}
		}
	}

	++totalFrame;
}

void LogMenu::SetLogTab()
{
	currSelectMode = SM_WORLD;
	ySelector->currIndex = 0;
	xSelector->ResetCounters();
	ySelector->ResetCounters();
	UpdateUnlockedLogs();
	//UpdateLogSelectQuads();

	state = LogMenu::WAIT;
	//logMenu->SetCurrLog();
}

void LogMenu::SetWorldMode()
{
	currSelectMode = SM_WORLD;
	worldText.setOutlineColor(Color::Red);
	worldText.setOutlineThickness(2);
}

void LogMenu::Draw(sf::RenderTarget *target)
{
	target->draw(logBGQuad, 4, sf::Quads);
	target->draw(containerBGQuad, 4, sf::Quads);
	target->draw(descriptionBGQuad, 4, sf::Quads);
	target->draw(controlsQuadBGQuad, 4, sf::Quads);
	target->draw(shardTitleBGQuad, 4, sf::Quads);
	
	target->draw(shardButtons, 4 * 4, sf::Quads, ts_shardButtons->texture);

	if (currSelectMode == SM_LOG)
	{
		switch (currLogType)
		{
		case LogDetailedInfo::LT_ENEMY:
		{
			previewParams->DrawEnemy(target);
			break;
		}
		case LogDetailedInfo::LT_WATER:
		{
			previewPoly->Draw(target);
			break;
		}
		case LogDetailedInfo::LT_RAIL:
		{
			previewRail->Draw(target);
			break;
		}
		case LogDetailedInfo::LT_SKIN:
		{
			target->draw(kinSprite, &pSkinShader.pShader);
			target->draw(kinFaceSprite, &pFaceSkinShader.pShader);
			break;
		}
		case LogDetailedInfo::LT_GRASS:
		{
			target->draw(grassSprite);
			break;
		}
		}

		target->draw(selectedBGQuad, 4, sf::Quads);
	}
	

	target->draw(logSelectQuads, xSelector->totalItems * (ySelector->totalItems - 1) * 4,
		sf::Quads, ts_logs->texture );

	
	target->draw(worldText);

	if (currLogText.getString() != "")
	{
		target->draw(currLogText);
	}
	if (currLogNameText.getString() != "")
	{
		target->draw(currLogNameText);
	}

}