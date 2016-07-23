#include "SoundManager.h"
#include <assert.h>

using namespace std;
using namespace sf;

sf::Music * SoundManager::GetMusic( const std::string &name )
{
	for( list<MusicInfo>::iterator it = songs.begin(); it != songs.end(); ++it )
	{
		if( (*it).name == name )
		{
			return (*it).music;
		}
	}


	//not found

	songs.push_back( MusicInfo() );
	MusicInfo &info = songs.back();
	info.name = name;
	info.music = new Music;
	bool res = info.music->openFromFile( name );
	info.music->setVolume( 0 );

	if( !res )
	{
		assert( false );
	}
	

	return info.music;
}

sf::SoundBuffer * SoundManager::GetSound( const std::string &name )
{
	for( list<SoundInfo>::iterator it = sounds.begin(); it != sounds.end(); ++it )
	{
		if( (*it).name == name )
		{
			return (*it).buffer;
		}
	}


	//not found

	sounds.push_back( SoundInfo() );
	SoundInfo &info = sounds.back();
	info.name = name;
	info.buffer = new SoundBuffer;
	bool res = info.buffer->loadFromFile( name );

	if( !res )
	{
		assert( false );
	}
	

	return info.buffer;
	//make sure to set up tileset here
}
