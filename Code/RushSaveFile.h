#ifndef __RUSHSAVEFILE_H__
#define __RUSHSAVEFILE_H__

#include "RushManager.h"
#include "MainMenu.h"
#include "md5.h"
#include "globals.h"
#include "KinUpgrades.h"
#include "RushFile.h"
#include "KinStore.h"



struct RushSaveFile
{
	//info

	int ver;

	std::string name;
	std::string fileName;
	std::string replayFolderName;
	std::string myFolderName;

	int medalRanks[8 * 8];


	UpgradeLevels kinUpgradeLevels;
	int currWorld;
	int powerMode;
	int currLevel;
	int currExp;
	int skin;
	int fileIndex;

	RushSaveFile( int p_fileIndex );
	void SetDefault();
	void SetAndSave(RushSaveFile *rsf);
	void Delete();
	bool LoadInfo(std::ifstream &is);
	bool Load();
	void Save();
};

#endif