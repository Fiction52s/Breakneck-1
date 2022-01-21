#include "SoundManager.h"
#include <assert.h>
#include <iostream>

using namespace std;
using namespace sf;


void SoundNode::Play(SoundInfo *p_info, int p_volume, bool p_loop)
{
	info = p_info;
	loop = p_loop;
	volume = p_volume;

	sound.stop();
	sound.setBuffer(*(info->buffer));
	sound.setLoop(loop);
	sound.setVolume(volume);
	sound.play();
}

void SoundNode::Stop()
{
	sound.stop();
	info = NULL;
	volume = -1;
	loop = false;
}

SoundNodeList::SoundNodeList( int p_maxSounds )
{
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
	soundVolume = 100;
	//enableSounds = true;
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

SoundNode * SoundNodeList::ActivateSound(SoundInfo *info, bool loop)
{
	if (info == NULL || info->buffer == NULL )
		return NULL;

	if (soundVolume == 0)
		return NULL;
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

		//curr->sound.stop();

		curr->Play(info, soundVolume, loop);

		return curr;
		
	}
	else
	{
		if( inactiveList->next == NULL )
		{
			activeList->prev = inactiveList;
			inactiveList->next = activeList;
			activeList = inactiveList;
			inactiveList = NULL;

			activeList->Play(info, soundVolume, loop);

			return activeList;
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

			activeList->Play(info, soundVolume, loop);

			return activeList;
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
		sn->Stop();
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
			sn->Stop();

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
			sn->Stop();
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
			sn->Stop();
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

void SoundNodeList::SetSoundVolume(int vol)
{
	soundVolume = vol;
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

SoundInfo * SoundManager::GetSound( const std::string &name )
{
	for( list<SoundInfo*>::iterator it = sounds.begin(); it != sounds.end(); ++it )
	{
		if( (*it)->name == name )
		{
			return (*it);
		}
	}


	//not found

	SoundInfo *si = new SoundInfo;
	sounds.push_back( si );
	//SoundInfo &info = sounds.back();
	

	string fullName = "Resources/Audio/Sounds/";
	fullName += name + ".ogg";

	si->name = name;
	si->buffer = new SoundBuffer;
	bool res = si->buffer->loadFromFile(fullName);

	if( !res )
	{
		cout << "failed to load sound: " << fullName << endl;
		//delete si->buffer;
		delete si;
		sounds.pop_back();

		return NULL;
		//assert( false );
	}
	

	return si;
	//make sure to set up tileset here
}

void SoundManager::ClearAll()
{
	for( list<SoundInfo*>::iterator it = sounds.begin(); it != sounds.end(); ++it )
	{
		delete (*it);
	}
	sounds.clear();

	/*for( list<MusicInfo*>::iterator it = songs.begin(); it != songs.end(); ++it )
	{
		delete (*it);
	}*/
}

SoundInfo::~SoundInfo()
{
	delete buffer;
}

SoundInfo::SoundInfo()
{
	buffer = NULL;
}