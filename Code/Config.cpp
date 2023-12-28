#include "Config.h"
#include <iostream>
#include <fstream>
#include "Input.h"
#include "MainMenu.h"


using namespace std;


//config data struct. honestly this should create a warning when your config file is messed up

ConfigData::ConfigData()
	:resolutionX( 1920 ), resolutionY( 1080 )
{
	SetToDefault();
}

void ConfigData::SetToDefault()
{
	resolutionX = 1920;
	resolutionY = 1080;
	windowStyle = sf::Style::Fullscreen;
	musicVolume = 40;
	soundVolume = 50;
	parallelPlayOn = true;
	showRunningTimer = false;
	showFPS = false;
}

Config::Config()
{
	ver = 1;
	path = MainMenu::GetInstance()->appDataPath + "config.txt";
	SetToDefault();
}

void Config::SetToDefault()
{
	data.SetToDefault();
}

bool Config::Load()
{
	ifstream is;

	is.open( path );
	if( is.is_open() )
	{
		int v;
		is >> v;

		if (v < ver)
		{
			cout << "config file is out of date. updating" << endl;
			is.close();
			SetToDefault();
			Save();
			return Load();
		}

		std::string settingName;
		while( true )
		{
			is >> settingName;
			boost::algorithm::to_lower( settingName );
		
			if( settingName == "resolutionx" )
			{
				is >> data.resolutionX;
			}
			else if( settingName == "resolutiony" )
			{
				is >> data.resolutionY;
			}
			else if( settingName == "windowmode" )
			{
				string mode;
				is >> mode;
				boost::algorithm::to_lower( mode );
				if( mode == "fullscreen" )
				{
					data.windowStyle = sf::Style::Fullscreen;
				}
				else if( mode == "window" || mode == "windowed" )
				{
					data.windowStyle = sf::Style::Default;
				}
				else if( mode == "borderlesswindow" || mode == "borderlesswindowed" )
				{
					data.windowStyle = sf::Style::None;
				}
			}
			else if( settingName == "soundvolume" )
			{
				int vol;
				is >> vol;
				data.soundVolume = vol;
			}
			else if (settingName == "musicvolume")
			{
				int vol;
				is >> vol;
				data.musicVolume = vol;
			}
			else if (settingName == "parallelplayon")
			{
				int par;
				is >> par;
				data.parallelPlayOn = par;
			}
			else if (settingName == "showrunningtimer")
			{
				int rtd;
				is >> rtd;
				data.showRunningTimer = rtd;
			}
			else if (settingName == "showfps")
			{
				int fps;
				is >> fps;
				data.showFPS = fps;
			}

			int c = is.peek();
			if( c == EOF )
			{
				break;
			}
		}

		is.close();

		return true;
	}
	else
	{
		return false;
	}
}

std::string ConfigData::GetWindowModeString( int mode )
{
	string s;
	switch (mode)
	{
	case sf::Style::Fullscreen:
		s = "Fullscreen";
		break;
	case sf::Style::None:
		s = "BorderlessWindowed";
		break;
	case sf::Style::Default:
		s = "Windowed";
		break;
	default:
		assert(0);
	}
	return s;
}

std::string ConfigData::GetResolutionString( int x, int y )
{
	string s = to_string(x) + " x " + to_string(y);
	return s;
}

void Config::Save()
{
	cout << "config::save\n";
	ofstream of;
	//of.open("Resources/config.txt");
	of.open(path);
	if (of.is_open())
	{
		of << ver << "\n";

		of << "ResolutionX " << data.resolutionX << "\n";
		of << "ResolutionY " << data.resolutionY << "\n";
		of << "WindowMode " << data.GetWindowModeString(data.windowStyle) << "\n";
		of << "MusicVolume " << data.musicVolume << "\n";
		of << "SoundVolume " << data.soundVolume << "\n";
		of << "ParalelPlayOn " << (int)data.parallelPlayOn << "\n";
		of << "ShowRunningTimer " << (int)data.showRunningTimer << "\n";
		of << "ShowFPS " << (int)data.showFPS << "\n";
		of.close();
	}
	else
	{
		assert(0 && "failed to find config file" );
	}
}

void Config::SetData(ConfigData &p_data)
{
	data = p_data;
}

const ConfigData &Config::GetData()
{
	return data;
}



