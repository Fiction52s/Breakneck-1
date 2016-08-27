#ifndef __SOUNDMANAGER_H__
#define __SOUNDMANAGER_H__
#include <SFML/Audio.hpp>
#include <list>

struct SoundInfo
{
	sf::SoundBuffer *buffer;
	std::string name;
};

struct MusicInfo
{
	sf::Music *music;
	std::string name;
};

struct SoundManager
{
	~SoundManager();
	sf::Music *GetMusic( const std::string &name );
	sf::SoundBuffer *GetSound( const std::string &name );
	std::list<SoundInfo*> sounds;
	std::list<MusicInfo*> songs;
};


struct SoundNode
{
	SoundNode()
		:next( NULL ),
		prev( NULL )
	{
	}
	SoundNode *next;
	SoundNode *prev;
	sf::Sound sound;
};

struct SoundNodeList
{
	SoundNodeList( int maxSounds );
	//void AddActive( SoundNode *sn );
	//void AddInactive( SoundNode *sn );
	void DeactivateSound( SoundNode *sn );
	void Update();
	void Reset();
	int GetActiveCount();
	int GetInactiveCount();
	SoundNode *activeList;
	SoundNode *inactiveList;
	void ActivateSound( sf::SoundBuffer *buffer );

};

#endif

//sf::Music music;
//if (!music.openFromFile("music.ogg"))
//    return -1; // error
//music.play();