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
	bool parallelPlayOn;
	bool showRunningTimer;
	bool showFPS;
};

struct Config
{
	Config();
	bool Load();
	void Save();
	const ConfigData &GetData();
	void SetData(ConfigData &data);
	void SetToDefault();
private:
	ConfigData data;
	std::string path;
};

#endif