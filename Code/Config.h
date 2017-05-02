#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <SFML\Graphics.hpp>

struct ConfigData
{
	ConfigData();
	int resolutionX;
	int resolutionY;
	int windowStyle;
	int volume;
};

struct Config
{
	Config();

	bool IsDoneLoading();
	void Load();
	void Shutdown();
	bool shutDown;
	void WaitForLoad();
	const ConfigData &GetData();
	void SetThread( boost::thread *p_t );

	int resolutionX;
	int resolutionY;

private:
	bool doneLoading;
	boost::thread *t;
	ConfigData data;
};

#endif