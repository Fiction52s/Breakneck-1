#include "SoundManager.h"
#include <assert.h>

using namespace std;
using namespace sf;


SoundManager::~SoundManager()
{
	for( list<SoundInfo*>::iterator it = sounds.begin(); it != sounds.end(); ++it )
	{
		delete (*it);
	}

	for( list<MusicInfo*>::iterator it = songs.begin(); it != songs.end(); ++it )
	{
		delete (*it);
	}
}
sf::Music * SoundManager::GetMusic( const std::string &name )
{
	for( list<MusicInfo*>::iterator it = songs.begin(); it != songs.end(); ++it )
	{
		if( (*it)->name == name )
		{
			return (*it)->music;
		}
	}


	//not found
	MusicInfo *ms = new MusicInfo;
	songs.push_back( ms );
	//MusicInfo &info = songs.back();
	ms->name = name;
	ms->music = new Music;
	bool res = ms->music->openFromFile( name );
	ms->music->setVolume( 0 );

	if( !res )
	{
		assert( false );
	}
	

	return ms->music;
}

sf::SoundBuffer * SoundManager::GetSound( const std::string &name )
{
	for( list<SoundInfo*>::iterator it = sounds.begin(); it != sounds.end(); ++it )
	{
		if( (*it)->name == name )
		{
			return (*it)->buffer;
		}
	}


	//not found

	SoundInfo *si = new SoundInfo;
	sounds.push_back( si );
	//SoundInfo &info = sounds.back();
	
	si->name = name;
	si->buffer = new SoundBuffer;
	bool res = si->buffer->loadFromFile( name );

	if( !res )
	{
		assert( false );
	}
	

	return si->buffer;
	//make sure to set up tileset here
}
