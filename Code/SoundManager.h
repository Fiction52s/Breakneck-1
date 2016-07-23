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
	sf::Music *GetMusic( const std::string &name );
	sf::SoundBuffer *GetSound( const std::string &name );
	std::list<SoundInfo> sounds;
	std::list<MusicInfo> songs;
};

#endif

//sf::Music music;
//if (!music.openFromFile("music.ogg"))
//    return -1; // error
//music.play();