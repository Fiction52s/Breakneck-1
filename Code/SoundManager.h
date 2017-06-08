#ifndef __SOUNDMANAGER_H__
#define __SOUNDMANAGER_H__
#include <SFML/Audio.hpp>
#include <list>

struct SoundInfo
{
	sf::SoundBuffer *buffer;
	std::string name;
};

//struct MusicInfo
//{
//	sf::Music *music;
//	boost::filesystem::path songPath;
//	//std::string name;
//};

struct SoundManager
{
	~SoundManager();
	//sf::Music *GetMusic( const std::string &name );
	sf::SoundBuffer *GetSound( const std::string &name );
	void ClearAll();
	std::list<SoundInfo*> sounds;
	//std::list<MusicInfo*> songs;
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
	~SoundNodeList();
	//void AddActive( SoundNode *sn );
	//void AddInactive( SoundNode *sn );
	void DeactivateSound( SoundNode *sn );
	void Update();
	void SetGlobalVolume( float vol );
	void SetRelativeMusicVolume( int vol );
	void SetRelativeSoundVolume( int vol );
	
	void SetSoundsEnable( bool e );
	void Reset();
	void Clear();
	int GetActiveCount();
	int maxSounds;
	int GetInactiveCount();
	SoundNode *activeList;
	SoundNode *inactiveList;
	void ActivateSound( sf::SoundBuffer *buffer );
	void Pause( bool p );
	bool paused;
	int globalVolume;
	int soundVolume;
	int musicVolume;
	bool enableSounds;
	bool enableMusic;
private:
	void SetSoundVolume( int v );
};

#endif

//sf::Music music;
//if (!music.openFromFile("music.ogg"))
//    return -1; // error
//music.play();