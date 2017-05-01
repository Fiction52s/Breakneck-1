#include "Config.h"
#include <iostream>
#include <fstream>


using namespace std;


void CreateLoadThread( Config *config )
{
	config->SetThread( new boost::thread( boost::bind( &Config::Load, config ) ) );
}

//config data struct. honestly this should create a warning when your config file is messed up

ConfigData::ConfigData()
	:resolutionX( 1920 ), resolutionY( 1080 )
{
}

Config::Config()
	:doneLoading( false )
{
	CreateLoadThread( this );
}

void Config::SetThread( boost::thread *p_t )
{
	t = p_t;
}

bool Config::IsDoneLoading()
{
	return doneLoading;
}

void Config::WaitForLoad()
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
				else if( mode == "windownoborder" )
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

const ConfigData &Config::GetData()
{
	return data;
}

