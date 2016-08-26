#include "SoundManager.h"
#include <assert.h>

using namespace std;
using namespace sf;


SoundNodeList::SoundNodeList( int maxSounds )
{
	activeList = NULL;
	inactiveList = NULL;

	inactiveList = new SoundNode;
	SoundNode *prev = inactiveList,*curr = NULL;
	for( int i = 1; i < maxSounds; ++i )
	{
		curr = new SoundNode;
		curr->prev = prev;
		prev->next = curr;

		prev = curr;
	}
}

void SoundNodeList::ActivateSound( SoundBuffer *buffer )
{
	//SoundNode *node;
	if( inactiveList == NULL )
	{
		SoundNode *curr = activeList;
		while( curr->next != NULL )
		{
			curr = curr->next;
		}

		curr->sound.stop();
		curr->sound.setBuffer( *buffer );
		curr->sound.play();
	}
	else
	{
		if( inactiveList->next == NULL )
		{
			activeList->prev = inactiveList;
			inactiveList->next = activeList;
			activeList = inactiveList;
			inactiveList = NULL;

			activeList->sound.setBuffer( *buffer );
			activeList->sound.play();
			//activeList->sound.get
		}
		else
		{
			SoundNode *next = inactiveList->next;
			inactiveList->next = activeList;

			if( activeList != NULL )
				activeList->prev = inactiveList;
			activeList = inactiveList;
			inactiveList = next;
			next->prev = NULL;

			activeList->sound.setBuffer( *buffer );
			activeList->sound.play();
			
		}
	}
}

void SoundNodeList::DeactivateSound( SoundNode *sn )
{
	assert( activeList != NULL );

	if( sn->next == NULL && sn->prev == NULL )
	{
		sn->sound.stop();
		sn->next = inactiveList;
		inactiveList->prev = sn;
		inactiveList = sn;

		activeList = NULL;
	}
	else
	{
		if( sn->next == NULL )
		{
			sn->sound.stop();

			sn->prev->next = NULL;
			sn->prev = NULL;
			inactiveList->prev = sn;
			sn->next = inactiveList;
			inactiveList = sn;
			
		}
		else if( sn->prev == NULL )
		{
			sn->sound.stop();
			sn->next->prev = NULL;
			activeList = sn->next;
		}
		else
		{
			sn->sound.stop();
			sn->next->prev = sn->prev;
			sn->prev->next = sn->next;

			inactiveList->prev = sn;
			sn->next = inactiveList;
			inactiveList = sn;
		}
	}
}

void SoundNodeList::Update()
{
	SoundNode *curr = activeList;
	while( curr != NULL )
	{
		SoundNode *next = curr->next;

		if( curr->sound.getStatus() == Sound::Status::Stopped )
		{
			DeactivateSound( curr );
		}

		curr = next;
	}
}

void SoundNodeList::Reset()
{
	SoundNode *curr = activeList;
	while( curr != NULL )
	{
		DeactivateSound( curr );
		curr = activeList;
	}
}

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
