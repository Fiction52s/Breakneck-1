#include "Sequence.h"
#include "GameSession.h"
#include "Actor.h"
#include "MusicPlayer.h"
#include "MainMenu.h"
using namespace sf;
using namespace std;

MonumentSeq::MonumentSeq(GameSession *owner)
	:BasicMovieSeq( owner, "kin_monument_01", 60, 1 )
{
	
}

void MonumentSeq::PreMovieUpdate()
{
	if (frame == 0)
	{
		MainMenu *mm = owner->mainMenu;
		mm->musicPlayer->FadeOutCurrentMusic(60);
		owner->Fade(false, 60, Color::Black);
	}
	//if (frame == stateLength[PREMOVIE] - 1)
	//{
	//	
	//}
}

void MonumentSeq::PostMovieUpdate()
{
	if (frame == stateLength[POSTMOVIE] - 1)
	{
		owner->Fade(true, 60, Color::Black);
		owner->GetPlayer(0)->StandInPlace();
		if (owner->originalMusic != NULL)
		{
			MainMenu *mm = owner->mainMenu;
			mm->musicPlayer->TransitionMusic(owner->originalMusic, 60);
		}
		//owner->PlayMusic();
	}
}