#include "SoundManager.h"
#include <assert.h>
#include <iostream>

using namespace std;
using namespace sf;


SoundNodeList::SoundNodeList( int p_maxSounds )
{
	globalVolume = 100;
	maxSounds = p_maxSounds;
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

	//load these from a prefs file
	globalVolume = 100;
	soundVolume = 100;
	musicVolume = 100;
	enableSounds = true;
	enableMusic = true;
}

void SoundNodeList::Clear()
{
	while( activeList != NULL )
	{
		DeactivateSound( activeList );
	}
}

SoundNodeList::~SoundNodeList()
{
	SoundNode *curr = activeList;
	while( curr != NULL )
	{
		SoundNode *next = curr->next;

		delete curr;

		curr = next;
	}

	curr = inactiveList;
	while( curr != NULL )
	{
		SoundNode *next = curr->next;

		delete curr;

		curr = next;
	}
}

void SoundNodeList::ActivateSound( SoundBuffer *buffer )
{
	//return;
	//cout << "Activate sound!" << endl;
	//SoundNode *node;
	//assert( inactiveList == NULL );
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
			//inactiveList->prev = NULL;

			if( activeList != NULL )
				activeList->prev = inactiveList;
			
			activeList = inactiveList;
			inactiveList = next;
			next->prev = NULL;

			activeList->sound.setBuffer( *buffer );
			activeList->sound.play();
			
		}
	}
	//assert( activeList->prev == NULL );
	//assert( inactiveList->prev == NULL );
}

void SoundNodeList::DeactivateSound( SoundNode *sn )
{
	assert( activeList != NULL );

	if( sn->next == NULL && sn->prev == NULL )
	{
		//cout << "a" << endl;
		sn->sound.stop();
		sn->next = inactiveList;
		inactiveList->prev = sn;
		inactiveList = sn;

		activeList = NULL;
	}
	else
	{
		//inactiveList is null! bug
		if( sn->next == NULL )
		{
			assert( sn != activeList );
			//int a = GetActiveCount();
			//int n = GetInactiveCount();
			//cout << "before b count: " << a << ", inactive: " << n << endl;
		//	cout << "b" << endl;
			//assert( sn != activeList );
			sn->sound.stop();

			sn->prev->next = NULL;
			sn->prev = NULL;

			if( inactiveList != NULL )
			{
				inactiveList->prev = sn;
				sn->next = inactiveList;
				inactiveList = sn;
			}
			else
			{
				inactiveList = sn;
				inactiveList->next = NULL;
				inactiveList->prev = NULL;
			}

			//a = GetActiveCount();
			//n = GetInactiveCount();
			//cout << "b count: " << a << ", inactive: " << n << endl;
			
		}
		else if( sn->prev == NULL )
		{
			//cout << "c" << endl;
			sn->sound.stop();
			sn->next->prev = NULL;
			activeList = sn->next;

			if( inactiveList != NULL )
			{
				sn->next = inactiveList;
				inactiveList->prev = sn;
				inactiveList = sn;
			}
			else
			{
				inactiveList = sn;
				inactiveList->next = NULL;
				inactiveList->prev = NULL;
			}
			//int a = GetActiveCount();
			//int n = GetInactiveCount();
			//cout << "c count: " << a << ", inactive: " << n << endl;
		}
		else
		{
			//cout << "d" << endl;
			sn->sound.stop();
			SoundNode * next = sn->next;
			SoundNode * prev = sn->prev;
			next->prev = prev;
			prev->next = next;
			//sn->next->prev = sn->prev;
			//sn->prev->next = sn->next;

			sn->prev = NULL;

			if( inactiveList != NULL )
			{
				inactiveList->prev = sn;
				sn->next = inactiveList;
				inactiveList = sn;
			}
			else
			{
				inactiveList = sn;
				inactiveList->next = NULL;
				inactiveList->prev = NULL;
			}
			
			

			//int a = GetActiveCount();
			//int n = GetInactiveCount();
			//cout << "d count: " << a << ", inactive: " << n << endl;
		}
	}
	
	//assert( activeList->prev == NULL );
	//assert( inactiveList->prev == NULL );
}

void SoundNodeList::Update()
{
	//int a = GetActiveCount();
	//int n = GetInactiveCount();
	//cout << "count: " << a << ", inactive: " << n << endl;
	//assert( a + n == maxSounds );
	
	SoundNode *curr = activeList;
	while( curr != NULL )
	{
		SoundNode *next = curr->next;
		//assert( next != curr );
		//cout << curr->sound.getStatus() << endl;
		if( curr->sound.getStatus() == Sound::Status::Stopped )
		{
			DeactivateSound( curr );
		}

		curr = next;
	}
	//cout << "end update" << endl;
}

void SoundNodeList::SetSoundsEnable( bool e )
{
	//if( enableSounds != e )
	//{
	enableSounds = e;
	if( e )
	{
		SetRelativeSoundVolume( soundVolume );
		//SetSoundVolume( 
	}
	else
	{
		SetSoundVolume( 0 );
	}
	//}
}

void SoundNodeList::SetRelativeMusicVolume( int vol )
{
	//no complicated music system is in yet. be able to control several simultaneous tracks
}

void SoundNodeList::SetSoundVolume( int v )
{
	assert(v >= 0);
	if( !enableSounds )
	{
		v = 0;
	}

	cout << "set final volume: " << v << endl;

	SoundNode *curr = activeList;
	while( curr != NULL )
	{
		SoundNode *next = curr->next;
		
		curr->sound.setVolume( v );

		curr = next;
	}

	curr = inactiveList;

	while( curr != NULL )
	{
		SoundNode *next = curr->next;
		
		curr->sound.setVolume( v );

		curr = next;
	}
}

void SoundNodeList::SetRelativeSoundVolume( int vol )
{
	soundVolume = vol;
	float percent = vol / 100.f;
	float final = globalVolume * percent;

	SetSoundVolume( final );
	
}

void SoundNodeList::SetGlobalVolume( float vol )
{
	assert(vol >= 0);
	globalVolume = vol;

	SetRelativeSoundVolume( soundVolume );
	//set relative music volume
}

int SoundNodeList::GetActiveCount()
{
	int count = 0;
	SoundNode *curr = activeList;
	while( curr != NULL )
	{
		++count;
		curr = curr->next;
	}

	return count;
}

int SoundNodeList::GetInactiveCount()
{
	int count = 0;
	SoundNode *curr = inactiveList;
	while( curr != NULL )
	{
		++count;
		curr = curr->next;
	}
	return count;
}

void SoundNodeList::Reset()
{
	SoundNode *curr = activeList;
	while( curr != NULL )
	{
		DeactivateSound( curr );
		curr = activeList;
	}
	//activeList = NULL;
}

void SoundNodeList::Pause( bool p )
{
	 if( paused != p )
	 {
		 if( p )
		 {
			 SoundNode *curr = activeList;
			 while( curr != NULL )
			 {
				 curr->sound.pause();
				 curr = curr->next;
			 }
		 }
		 else
		 {
			 SoundNode *curr = activeList;
			 while( curr != NULL )
			 {
				 curr->sound.play();
				 curr = curr->next;
			 }
		 }
	 }
}

SoundManager::~SoundManager()
{
	ClearAll();
}

//sf::Music * SoundManager::GetMusic( const std::string &name )
//{
//	for( list<MusicInfo*>::iterator it = songs.begin(); it != songs.end(); ++it )
//	{
//		if( (*it)->name == name )
//		{
//			return (*it)->music;
//		}
//	}
//
//
//	//not found
//	MusicInfo *ms = new MusicInfo;
//	songs.push_back( ms );
//	//MusicInfo &info = songs.back();
//	ms->name = name;
//	ms->music = new Music;
//	bool res = ms->music->openFromFile( name );
//	ms->music->setVolume( 0 );
//
//	if( !res )
//	{
//		assert( false );
//	}
//	
//
//	return ms->music;
//}

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

void SoundManager::ClearAll()
{
	for( list<SoundInfo*>::iterator it = sounds.begin(); it != sounds.end(); ++it )
	{
		delete (*it);
	}

	/*for( list<MusicInfo*>::iterator it = songs.begin(); it != songs.end(); ++it )
	{
		delete (*it);
	}*/
}
