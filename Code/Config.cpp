#include "Config.h"
#include <iostream>
#include <fstream>


using namespace std;


void Config::CreateLoadThread( Config *config )
{
	config->SetThread( new boost::thread( boost::bind( &Config::Load, config ) ) );
}

void Config::CreateSaveThread(Config *config)
{
	config->SetThread(new boost::thread(boost::bind(&Config::Save, config)));
}

//config data struct. honestly this should create a warning when your config file is messed up

ConfigData::ConfigData()
	:resolutionX( 1920 ), resolutionY( 1080 )
{
}

void ConfigData::SetToDefault()
{
	resolutionX = 1920;
	resolutionY = 1080;
	windowStyle = sf::Style::Fullscreen;
	volume = 100;
}

Config::Config()
	:doneLoading( false )
{
	CreateLoadThread( this );
}

void Config::SetToDefault()
{
	data.SetToDefault();
}

void Config::SetThread( boost::thread *p_t )
{
	t = p_t;
}

bool Config::IsDoneLoading()
{
	return doneLoading;
}

bool Config::IsDoneSaving()
{
	return doneSaving;
}

void Config::WaitForLoad()
{
	t->join();
	delete t;
	t = NULL;
}

void Config::WaitForSave()
{
	t->join();
	delete t;
	t = NULL;
}

void Config::Load()
{
	ifstream is;
	is.open( "config" );
	if( is.is_open() )
	{
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
				else if( mode == "window" )
				{
					data.windowStyle = sf::Style::Default;
				}
				else if( mode == "borderlesswindow" )
				{
					data.windowStyle = sf::Style::None;
				}
			}
			else if( settingName == "volume" )
			{
				int vol;
				is >> vol;
				data.volume = vol;
			}

			int c = is.peek();
			if( c == EOF )
			{
				break;
			}
		}
	}
	is.close();
}

std::string ConfigData::GetWindowModeString()
{
	string s;
	switch (windowStyle)
	{
	case sf::Style::Fullscreen:
		s = "Fullscreen";
		break;
	case sf::Style::None:
		s = "BorderlessWindow";
		break;
	case sf::Style::Default:
		s = "Window";
		break;
	default:
		assert(0);
	}
	return s;
}

void Config::Save()
{
	ofstream of;
	of.open("config");
	if (of.is_open())
	{
		of << "ResolutionX " << data.resolutionX << "\n";
		of << "ResolutionY " << data.resolutionY << "\n";


		of << "WindowMode " << data.GetWindowModeString() << "\n";
		of << "ResolutionY " << data.volume;// << "\n";

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



