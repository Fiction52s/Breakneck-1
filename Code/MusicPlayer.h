#ifndef __MUSIC_PLAYER_H__
#define __MUSIC_PLAYER_H__

#include <SFML/Graphics.hpp>

struct MusicInfo;
struct MainMenu;
struct MusicPlayer
{
	MusicPlayer(MainMenu *mm);
	void Reset();
	MainMenu *mm;
	void PlayMusic(MusicInfo *music, sf::Time startTime = sf::Time::Zero); //add transitions later
	void TransitionMusic(MusicInfo *music,
		int crossFadeFrames, sf::Time startTime = sf::Time::Zero );

	void StopMusic(MusicInfo *m);
	void StopCurrentMusic();
	void FadeOutCurrentMusic(int numFrames);
	void Update();
	void PauseCurrentMusic();
	void UnpauseCurrentMusic();
	int musicFadeOutMax;
	int musicFadeOutCurr;
	MusicInfo *fadingOutMusic;
	MusicInfo *fadingInMusic;
	int musicFadeInMax;
	int musicFadeInCurr;

	MusicInfo *currMusic;



	//void AddMusic(MusicInfo *music);
	//std::map<std::string, MusicInfo*> musicMap;
};

#endif