#include "MusicPlayer.h"
#include "MusicSelector.h"
#include "MainMenu.h"
#include "Config.h"

MusicPlayer::MusicPlayer(MainMenu *p_mm)
{
	mm = p_mm;
	currMusic = NULL;
	Reset();
}

void MusicPlayer::Reset()
{
	musicFadeOutMax = -1;
	musicFadeInMax = -1;
	fadingOutMusic = NULL;
	fadingInMusic = NULL;
	//paused = false;
}

void MusicPlayer::StopCurrentMusic()
{
	if (currMusic != NULL)
	{
		StopMusic(currMusic);
		currMusic = NULL;

		StopMusic(fadingInMusic);
		fadingInMusic = NULL;

		StopMusic(fadingOutMusic);
		fadingOutMusic = NULL;

		musicFadeOutMax = -1;
		musicFadeInMax = -1;
		
	}
	//paused = false;
}

void MusicPlayer::UpdateVolume()
{
	if (currMusic != NULL)
	{
		currMusic->music->setVolume(mm->config->GetData().musicVolume);
	}
}

void MusicPlayer::PlayMusic(MusicInfo *newMusic, sf::Time startTime)
{
	//MusicInfo *newMusic = musicMap[name];
	
	StopFades();

	if (currMusic != NULL)
	{
		StopMusic(currMusic);
	}
		

	newMusic->music->setVolume(mm->config->GetData().musicVolume);
	newMusic->music->setLoop(true);
	newMusic->music->play();
	newMusic->music->setPlayingOffset(startTime);

	currMusic = newMusic;
}

void MusicPlayer::Update()
{
	int maxVol = mm->config->GetData().musicVolume;
	if (musicFadeOutMax > 0)
	{
		assert(fadingOutMusic != NULL);
		if (musicFadeOutCurr == musicFadeOutMax)
		{
			StopMusic(fadingOutMusic);
			musicFadeOutMax = -1;
			fadingOutMusic = NULL;
		}
		else
		{
			musicFadeOutCurr++;
			float fadeOutPart = ((float)musicFadeOutCurr) / musicFadeOutMax;

			int vol = maxVol - (fadeOutPart * maxVol);
			fadingOutMusic->music->setVolume(vol);
		}
	}
	if (musicFadeInMax > 0)
	{
		assert(fadingInMusic != NULL);
		if (musicFadeInCurr == musicFadeInMax)
		{
			currMusic = fadingInMusic;
			currMusic->music->setVolume(maxVol);
			fadingInMusic = NULL;
			musicFadeInMax = -1;
		}
		else
		{
			musicFadeInCurr++;
			float fadeInPart = ((float)musicFadeInCurr) / musicFadeInMax;
			int vol = fadeInPart *  maxVol;
			fadingInMusic->music->setVolume(vol);
		}
	}
}

//pause and unpause aren't really
//done. need to account for fades.
void MusicPlayer::PauseCurrentMusic()
{
	if (currMusic != NULL)
	{
		currMusic->music->pause();
	}
}

void MusicPlayer::UnpauseCurrentMusic()
{
	if (currMusic != NULL)
	{
		currMusic->music->play();
	}
}

void MusicPlayer::StopFades()
{
	if (fadingInMusic != NULL)
	{
		StopMusic(fadingInMusic);
		fadingInMusic = NULL;
	}

	if (fadingOutMusic != NULL)
	{
		StopMusic(fadingOutMusic);
		fadingOutMusic = NULL;
	}

	musicFadeOutMax = -1;
	musicFadeInMax = -1;
}

void MusicPlayer::TransitionMusic(MusicInfo *newMusic,
	int crossFadeFrames, sf::Time startTime)
{
	StopFades();


	if (newMusic == NULL)
	{
		FadeOutCurrentMusic(crossFadeFrames);
		return;
		//assert(0);
	}
	if (currMusic == NULL)
	{
		musicFadeInMax = crossFadeFrames;
		musicFadeInCurr = 0;

		musicFadeOutMax = -1;
	}
	else
	{
		musicFadeInMax = crossFadeFrames;
		musicFadeOutMax = crossFadeFrames;
		musicFadeInCurr = 0;
		musicFadeOutCurr = 0;

		fadingOutMusic = currMusic;
	}

	

	fadingInMusic = newMusic;


	//MusicInfo *newMusic = musicMap[name];
	newMusic->music->setVolume(0);
	newMusic->music->setLoop(true);
	newMusic->music->play();
	newMusic->music->setPlayingOffset(startTime);


	
}

void MusicPlayer::StopMusic(MusicInfo *m)
{
	if (m != NULL)
	{
		m->music->setVolume(0);
		m->music->stop();
		//m = NULL;
	}
}

void MusicPlayer::FadeOutCurrentMusic(int numFrames)
{
	StopFades();

	if (currMusic != NULL)
	{
		fadingOutMusic = currMusic;
		musicFadeOutMax = numFrames;
		musicFadeOutCurr = 0;
		currMusic = NULL;
	}
}