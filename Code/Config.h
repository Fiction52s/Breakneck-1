#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <SFML\Graphics.hpp>

struct ConfigData
{
	ConfigData();
	std::string GetWindowModeString();
	void SetToDefault();
	int resolutionX;
	int resolutionY;
	int windowStyle;
	int volume;
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