#include "Sequence.h"
#include "GameSession.h"
#include "Actor.h"
using namespace sf;
using namespace std;

MonumentSeq::MonumentSeq(GameSession *owner)
	:BasicMovieSeq( owner, "kin_monument_01b", 60, 1 )
{
	
}

void MonumentSeq::PreMovieUpdate()
{
	if (frame == 0)
	{
		owner->FadeOutCurrentMusic(60);
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
		//owner->PlayMusic();
	}
}