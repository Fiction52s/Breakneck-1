#include "Config.h"
#include <iostream>
#include <fstream>
#include "Input.h"


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
	SetToDefault();
}

void ConfigData::SetToDefault()
{
	resolutionX = 1920;
	resolutionY = 1080;
	windowStyle = sf::Style::Fullscreen;
	musicVolume = 100;
	soundVolume = 100;
	defaultProfileName = "KIN_Default";
}

Config::Config()
	:doneLoading( false ), doneSaving( false )
{
	t = NULL;
	SetToDefault();
	CreateLoadThread( this );
}

void Config::SetToDefault()
{
	data.SetToDefault();
}

void Config::SetThread( boost::thread *p_t )
{
	assert(t == NULL);
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
	is.open( "Resources/config" );
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
			else if (settingName == "defaultprofile")
			{
				is >> data.defaultProfileName;
			}
			else if (settingName == "defaultinputformat")
			{
				is >> data.defaultInputFormat;
				/*string defaultInputName;
				is >> defaultInputName;

				if (defaultInputName == "XBOX")
				{
					data.defaultInputFormat = ControllerType::CTYPE_XBOX;
				}
				else if (defaultInputName == "Gamecube")
				{
					data.defaultInputFormat = ControllerType::CTYPE_GAMECUBE;
				}
				else if (defaultInputName == "Keyboard")
				{
					data.defaultInputFormat = ControllerType::CTYPE_KEYBOARD;
				}
				else
				{
					assert(0);
				}*/
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

void Config::Save()
{
	cout << "config::save\n";
	ofstream of;
	of.open("Resources/config");
	if (of.is_open())
	{
		of << "ResolutionX " << data.resolutionX << "\n";
		of << "ResolutionY " << data.resolutionY << "\n";
		of << "WindowMode " << data.GetWindowModeString() << "\n";
		of << "MusicVolume " << data.musicVolume << "\n";
		of << "SoundVolume " << data.soundVolume << "\n";
		of << "DefaultProfile " << data.defaultProfileName << "\n";
		of << "DefaultInputFormat " << data.defaultInputFormat;
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



