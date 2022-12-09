#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <SFML\Graphics.hpp>

struct ConfigData
{
	ConfigData();
	static std::string GetWindowModeString( int mode );
	static std::string GetResolutionString( int x, int y );
	void SetToDefault();
	int resolutionX;
	int resolutionY;
	int windowStyle;
	int musicVolume;
	int soundVolume;
	std::string defaultProfileName;
};

struct Config
{
	Config();

	bool IsDoneLoading();
	bool IsDoneSaving();
	void Load();
	void Save();
	void Shutdown();
	bool shutDown;
	void WaitForLoad();
	void WaitForSave();
	const ConfigData &GetData();
	void SetData(ConfigData &data);
	void SetThread( boost::thread *p_t );
	static void CreateSaveThread(Config *config);
	static void CreateLoadThread(Config *config);
	void SetToDefault();

	int resolutionX;
	int resolutionY;
	

private:
	bool doneSaving;
	bool doneLoading;
	boost::thread *t;
	ConfigData data;
};

#endif