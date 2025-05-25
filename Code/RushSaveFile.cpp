#include "RushSaveFile.h"
#include <fstream>
#include <boost\filesystem.hpp>
#include "nlohmann\json.hpp"

using namespace std;
using namespace sf;
using namespace nlohmann;

RushSaveFile::RushSaveFile(int p_fileIndex)
{
	ver = 1;

	fileIndex = p_fileIndex;

	name = "file_" + to_string(fileIndex);

	string dataFolder = MainMenu::GetInstance()->appDataPath + "SaveData\\";
	myFolderName = dataFolder + name + "\\";
	fileName = myFolderName + name + SAVE_EXT;
	replayFolderName = myFolderName + name + "_bestreplay\\";

	SetDefault();
}

bool RushSaveFile::Load()
{
	ifstream is;

	is.open(fileName);

	if (!LoadInfo(is))
	{
		SetDefault();
		return false;
	}
	else
	{
		return true;
	}
}

void RushSaveFile::Save()
{
	if (!boost::filesystem::exists(myFolderName))
	{
		boost::filesystem::create_directory(myFolderName);
	}

	if (!boost::filesystem::exists(replayFolderName))
	{
		boost::filesystem::create_directory(replayFolderName);
	}

	ofstream of;

	of.open(fileName);

	cout << "saving save file" << endl;

	json j;

	if (of.is_open())
	{
		j["version"] = ver;
		j["currWorld"] = currWorld;
		j["powerMode"] = powerMode;
		j["currLevel"] = currLevel;
		j["currExp"] = currExp;
		j["skin"] = skin;

		of << j.dump(4);

		of.close();
	}
	else
	{
		cout << "error saving file: " << fileName << endl;
		assert(false);
	}
}

void RushSaveFile::Delete()
{
	boost::filesystem::remove(fileName);
	if (boost::filesystem::exists(replayFolderName))
	{
		boost::filesystem::remove_all(replayFolderName);
	}

	//boost::filesystem::directory
	//boost::filesystem::create_directory
	SetDefault();
}

bool RushSaveFile::LoadInfo(std::ifstream &is)
{
	if (is.is_open())
	{
		json j;

		is >> j;
		
		int fileVer = j["version"];
		currWorld = j["currWorld"];
		powerMode = j["powerMode"];
		currLevel = j["currLevel"];
		currExp = j["currExp"];
		skin = j["skin"];

		if (fileVer < ver)
		{
			SetDefault();

			Save();
			return Load();
		}

		is.close();
		return true;
	}
	else
	{
		return false;
	}
}

void RushSaveFile::SetDefault()
{
	currWorld = 0;
	powerMode = 0;
	currLevel = 0;
	currExp = 0;
	skin = 0;
}

void RushSaveFile::SetAndSave(RushSaveFile *rsf)
{
	//mostRecentWorldSelected = saveFile->mostRecentWorldSelected;
	//visualInfo = saveFile->visualInfo;
	//kinOptionField.Set(saveFile->kinOptionField);

	//for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	//{
	//	levelData[i] = saveFile->levelData[i];
	//}

	currWorld = rsf->currWorld;
	powerMode = rsf->powerMode;
	currLevel = rsf->currLevel;
	currExp = rsf->currExp;
	skin = rsf->skin;

	kinUpgradeLevels.Set(&(rsf->kinUpgradeLevels));

	Save();

	if (boost::filesystem::exists(replayFolderName))
	{
		boost::filesystem::remove(replayFolderName);
	}
	//boost::filesystem::create_directory(replayFolderName);

	MainMenu::copyDirectoryRecursively(rsf->replayFolderName, replayFolderName);
}