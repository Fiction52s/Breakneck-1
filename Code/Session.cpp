#include "Session.h"
#include "Actor.h"
#include "MainMenu.h"
#include "MapHeader.h"

using namespace sf;
using namespace std;

Session::Session(const boost::filesystem::path &p_filePath)
{
	mainMenu = MainMenu::GetInstance();
	assert(mainMenu != NULL);

	filePath = p_filePath;
	filePathStr = filePath.string();

	players.resize(MAX_PLAYERS);
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		players[i] = nullptr;
	}
}

bool Session::ReadDecorImagesFile()
{
	ifstream is;
	is.open("Resources/decor.txt");
	if (is.is_open())
	{
		string name;
		int width;
		int height;
		int tile;
		while (!is.eof())
		{
			is >> name;
			is >> width;
			is >> height;

			is >> tile;

			string fullName = name + string(".png");

			Tileset *ts = GetTileset(fullName, width, height);
			assert(ts != NULL);
			decorTSMap[name] = ts;

			//this is only in the editor version atm
			//decorTileIndexMap[name].push_back(tile);
		}

		is.close();
	}
	else
	{
		return false;
	}

	return true;
}

bool Session::ReadHeader(std::ifstream &is)
{
	mapHeader = mainMenu->ReadMapHeader(is);
	if (mapHeader == NULL)
		return false;

	//ProcessHeader

	return true;
}

bool Session::ReadDecor(std::ifstream &is)
{
	int numDecorImages;
	is >> numDecorImages;

	for (int i = 0; i < numDecorImages; ++i)
	{
		string dName;
		is >> dName;
		int dLayer;
		is >> dLayer;

		Vector2f dPos;
		is >> dPos.x;
		is >> dPos.y;

		float dRot;
		is >> dRot;

		Vector2f dScale;
		is >> dScale.x;
		is >> dScale.y;

		int dTile;
		is >> dTile;

		Sprite dSpr;
		dSpr.setScale(dScale);
		dSpr.setRotation(dRot);
		dSpr.setPosition(dPos);

		Tileset *ts = decorTSMap[dName];
		dSpr.setTexture(*ts->texture);
		dSpr.setTextureRect(ts->GetSubRect(dTile));
		dSpr.setOrigin(dSpr.getLocalBounds().width / 2, dSpr.getLocalBounds().height / 2);		

		DecorPtr dec = new EditorDecorInfo(dSpr, dLayer, dName, dTile);
		if (dLayer > 0)
		{
			dec->myList = &decorImagesBehindTerrain;
			//decorImagesBehindTerrain.sort(CompareDecorInfo);
			//decorImagesBehindTerrain.push_back(dec);
		}
		else if (dLayer < 0)
		{
			dec->myList = &decorImagesFrontTerrain;
			//decorImagesFrontTerrain.push_back(dec);
		}
		else if (dLayer == 0)
		{
			dec->myList = &decorImagesBetween;
			//decorImagesBetween.push_back(dec);
		}

		CreateDecorImage(dec);

		mapStartBrush->AddObject(dec);
	}

	return true;
}

bool Session::ReadPlayer(std::ifstream &is)
{

}

bool Session::ReadTerrain(std::ifstream &is)
{

}

bool Session::ReadSpecialTerrain(std::ifstream &is)
{

}

bool Session::ReadBGTerrain(std::ifstream &is)
{

}

bool Session::ReadRails(std::ifstream &is)
{

}

bool Session::ReadActors(std::ifstream &is)
{

}

bool Session::ReadGates(std::ifstream &is)
{

}


bool Session::ReadFile()
{
	ifstream is;
	is.open(filePathStr);

	if (is.is_open())
	{
		ReadHeader(is);
		ReadDecor(is);
		ReadPlayer(is);
		ReadTerrain(is);
		ReadSpecialTerrain(is);
		ReadBGTerrain(is);
		ReadRails(is);
		ReadActors(is);
		ReadGates(is);

		is.close();
	}
	else
	{
		cout << "file read failed: " << filePathStr << endl;
		assert(false && "error getting file to edit ");

		return false;
	}


	grassTex.loadFromFile("Resources/Env/grass_128x128.png");

	return true;
}

ControllerState &Session::GetPrevInput(int index)
{
	return mainMenu->GetPrevInput(index);
}

ControllerState &Session::GetCurrInput(int index)
{
	return mainMenu->GetCurrInput(index);
}

ControllerState &Session::GetPrevInputUnfiltered(int index)
{
	return mainMenu->GetPrevInputUnfiltered(index);
}

ControllerState &Session::GetCurrInputUnfiltered(int index)
{
	return mainMenu->GetCurrInputUnfiltered(index);
}

GameController &Session::GetController(int index)
{
	return mainMenu->GetController(index);
}

Actor *Session::GetPlayer(int index)
{
	assert(index >= 0 && index < MAX_PLAYERS);
	return players[index];
}

void Session::UpdateControllers()
{
	bool gccEnabled = mainMenu->gccDriverEnabled;

	if (gccEnabled)
		gcControllers = mainMenu->gccDriver->getState();

	for (int i = 0; i < 4; ++i)
	{
		GetPrevInput(i) = GetCurrInput(i);
		GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);

		GameController &con = GetController(i);
		if (gccEnabled)
			con.gcController = gcControllers[i];

		con.UpdateState();

		GetCurrInput(i) = con.GetState();
		GetCurrInputUnfiltered(i) = con.GetUnfilteredState();
	}
}

void Session::UpdatePlayerInput(int index)
{
	Actor *player = GetPlayer(index);
	if (player == NULL)
		return;

	ControllerState &pCurr = player->currInput;
	GameController &controller = GetController(index);
	ControllerState &currInput = GetCurrInput(index);
	ControllerState &prevInput = GetPrevInput(index);

	bool alreadyBounce = pCurr.X;
	bool alreadyGrind = pCurr.Y;
	bool alreadyTimeSlow = pCurr.leftShoulder;

	if (cutPlayerInput)
	{
		player->currInput = ControllerState();
	}
	else
	{
		player->currInput = currInput;

		if (controller.keySettings.toggleBounce)
		{
			if (currInput.X && !prevInput.X)
			{
				pCurr.X = !alreadyBounce;
			}
			else
			{
				pCurr.X = alreadyBounce;
			}
		}
		if (controller.keySettings.toggleGrind)
		{
			if (currInput.Y && !prevInput.Y)
			{
				pCurr.Y = !alreadyGrind;
			}
			else
			{
				pCurr.Y = alreadyGrind;
			}
		}
		if (controller.keySettings.toggleTimeSlow)
		{
			if (currInput.leftShoulder && !prevInput.leftShoulder)
			{
				pCurr.leftShoulder = !alreadyTimeSlow;

			}
			else
			{
				pCurr.leftShoulder = alreadyTimeSlow;
			}
		}
	}
}

bool Session::IsKeyPressed(int k)
{
	return mainMenu->IsKeyPressed(k);
}

bool Session::IsMousePressed(int m)
{
	return mainMenu->IsMousePressed(m);
}