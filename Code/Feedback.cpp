#include "Feedback.h"
#include "Tileset.h"
#include "GameSession.h"

using namespace sf;
using namespace std;

Feedback::Feedback( GameSession *p_owner )
	:owner(p_owner), t( NULL )
{
	t = owner->GetTileset( "jump_64x64.png", 64, 64 );
	animFactor = 1;
	totalFrames = t->GetNumTiles();
	rt = new RenderTexture;
	rt->create( 300, 300 );
	rt->clear();
}

void Feedback::Draw( sf::RenderTarget *target )
{
	//target->
}