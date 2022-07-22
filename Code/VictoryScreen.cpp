#include "VictoryScreen.h"
#include "GameSession.h"
#include "UIWindow.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;

VictoryScreen2PlayerVS::VictoryScreen2PlayerVS( GameSession *p_owner )
	:owner( p_owner )
{
	player1Bar = new PlayerInfoBar( owner, 1920/2, 0 );
	player2Bar = new PlayerInfoBar( owner, 1920/2, 1 );
	//player2Bar->SetBottomLeftPos( Vector2f( 0, 128 ) );
}

void VictoryScreen::Reset()
{
	frame = 0;
}

void VictoryScreen2PlayerVS::Draw( sf::RenderTarget *target )
{
	player1Bar->Draw( target );
	player2Bar->Draw( target );
}

void VictoryScreen2PlayerVS::Update()
{
	player1Bar->Update( owner->GetCurrInput( 0 ).A && !owner->GetPrevInput( 0 ).A );
	player2Bar->Update( owner->GetCurrInput( 1 ).A && !owner->GetPrevInput( 1 ).A );
	++frame;
}

void VictoryScreen2PlayerVS::ResetSprites()
{
}
void VictoryScreen2PlayerVS::UpdateSprites()
{
}

void VictoryScreen2PlayerVS::Reset()
{
	VictoryScreen::Reset();
}

PlayerInfoBar::PlayerInfoBar( GameSession *p_owner, int width, int playerIndex )
	:owner( p_owner )
{
	//SetBottomLeftPos( Vector2f( 0, 0 ) );

	state = STATE_INITIAL_WAIT;

	frame = 0;

	framesBeforeShowFace = 60;
	framesExpandingShowFace = 60;
	framesExpandingFull = 60;
	framesToClose = 60;

	heightWait = 80;
	heightShowFace = 300;
	heightFull = 900;

	//windows = new UIWindow*[2];
	Tileset *ts_window = owner->GetTileset( "Menu/windows_64x24.png", 64, 24 );

	uiWindow = new UIWindow( NULL, ts_window, Vector2f( width, 1080 ) );
	uiWindow->SetTopLeft( playerIndex * width, 1080 ); //will be 1080 soon
	//windowVA[BOT_QUAD_INDEX]
}

void PlayerInfoBar::Draw( sf::RenderTarget *target )
{
	uiWindow->Draw( target );
	//target->draw( windowVA, 12, sf::Quads, ts_bar->texture );
}

void PlayerInfoBar::Update( bool pressedA )
{
	switch( state )
	{
	case STATE_INITIAL_WAIT:
		{
			if( frame == framesBeforeShowFace )
			{
				state = STATE_SHOW_FACE;
				frame = 0;
			}
			break;
		}
	case STATE_SHOW_FACE:
		{
			if( pressedA && frame > framesExpandingShowFace )
			{
				state = STATE_WAIT_EXPANDED;
				frame = 0;
			}
			break;
		}
	case STATE_WAIT_EXPANDED:
		{
			if( pressedA && frame >= framesExpandingFull )
			{
				state = STATE_CLOSED;
				frame = 0;
			}
			break;
		}
	case STATE_CLOSED:
		{
			
			break;
		}
	}

	switch( state )
	{
	case STATE_INITIAL_WAIT:
		{
			if( frame == 0 )
			{
				
				SetHeight( heightWait );
			}
			break;
		}
	case STATE_SHOW_FACE:
		{
			if( frame > framesExpandingShowFace )
			{
				frame = framesExpandingShowFace;
			}
			else
			{
				CubicBezier bez( 0, 0, 1, 1 );
				double f = bez.GetValue( (double)frame / framesExpandingShowFace );
				double h = heightWait * ( 1.0 - f ) + heightShowFace * f;
				SetHeight( floor( h + .5 ) );
			}
			break;
		}
	case STATE_WAIT_EXPANDED:
		{
			if( frame > framesExpandingFull )
			{
				frame = framesExpandingFull;
			}
			else
			{
				CubicBezier bez( 0, 0, 1, 1 );
				double f = bez.GetValue( (double)frame / framesExpandingFull );
				double h = heightShowFace * ( 1.0 - f ) + heightFull * f;
				SetHeight( floor( h + .5 ) );
			}
			break;
		}
	case STATE_CLOSED:
		{
			if( frame >= framesToClose )
			{
				frame = framesToClose;
			}
			break;
		}
	}
	++frame;
}

void PlayerInfoBar::SetHeight( int height )
{
	uiWindow->SetTopLeft( uiWindow->GetTopLeftRel().x, 1080 - height );
	//uiWindow->Resize( uiWindow->GetWidth(), height );
}

ResultsScreen::ResultsScreen(GameSession *p_owner)
	:owner( p_owner )
{
	frame = 0;
	state = FADEIN;

	memset(columnReady, 0, sizeof(columnReady));

	for (int i = 0; i < 4; ++i)
	{
		SetBoxPos(i, 1080);
	}

	memset(ts_column, 0, sizeof(ts_column));

	slideInStartFrame[0] = 0;
	slideInStartFrame[1] = 60;
	slideInStartFrame[2] = 120;
	slideInStartFrame[3] = 180;

	slideInFrames[0] = 60;
	slideInFrames[1] = 60;
	slideInFrames[2] = 60;
	slideInFrames[3] = 60;

	slideOutFrames = 60;
}

void ResultsScreen::SetBoxPos(int boxIndex, float yHeight)
{
	columnSprites[boxIndex].setPosition(boxIndex * 1920 / 4, yHeight);
	/*boxes[boxIndex * 4].position = Vector2f(boxIndex * 1920 / 4, yHeight);
	boxes[boxIndex * 4+1].position = Vector2f((boxIndex+1)* 1920 / 4, yHeight);
	boxes[boxIndex * 4+2].position = Vector2f((boxIndex + 1) * 1920 / 4, yHeight + 1080);
	boxes[boxIndex * 4+3].position = Vector2f(boxIndex * 1920 / 4, yHeight + 1080);*/
}

void ResultsScreen::SetTile(int boxIndex, int tile)
{

	if (ts_column[boxIndex] != NULL )
	{
		sf::Texture *tex = ts_column[boxIndex]->texture;
		sf::IntRect rect = ts_column[boxIndex]->GetSubRect(tile);
		columnSprites[boxIndex].setTexture(*tex);
		columnSprites[boxIndex].setTextureRect(rect);
	}
	/*boxes[boxIndex * 4].texCoords = Vector2f(rect.left, rect.top);
	boxes[boxIndex * 4 + 1].texCoords = Vector2f(rect.left + rect.width, rect.top);
	boxes[boxIndex * 4 + 2].texCoords = Vector2f(rect.left + rect.width, rect.top + rect.height);
	boxes[boxIndex * 4 + 3].texCoords = Vector2f(rect.left, rect.top + rect.height);*/
}

Tileset * ResultsScreen::GetTeamTileset(int teamIndex, bool win )
{
	switch (teamIndex)
	{
	case 0:
		if (win)
		{
			return owner->GetTileset("Menu/Results/blue_win_480x1080.png", 480, 1080);
		}
		else
		{
			return owner->GetTileset("Menu/Results/blue_lose_480x1080.png", 480, 1080);
		}
		
		break;
	case 1:
		if (win)
		{
			return owner->GetTileset("Menu/Results/red_win_480x1080.png", 480, 1080);
		}
		else
		{
			return owner->GetTileset("Menu/Results/red_lose_480x1080.png", 480, 1080);
		}
	case 2:
		if (win)
		{
			return owner->GetTileset("Menu/Results/green_win_480x1080.png", 480, 1080);
		}
		else
		{
			return owner->GetTileset("Menu/Results/green_lose_480x1080.png", 480, 1080);
		}
	case 3:
		if (win)
		{
			return owner->GetTileset("Menu/Results/purp_win_480x1080.png", 480, 1080);
		}
		else
		{
			return owner->GetTileset("Menu/Results/purp_lose_480x1080.png", 480, 1080);
		}
	
		break;
	}
}

Tileset * ResultsScreen::GetSoloTilset(int soloIndex, bool win)
{
	return NULL;
}

void ResultsScreen::SetupColumns()
{
	maxPlacing = 0;
	for (int i = 0; i < 4; ++i)
	{
		SetBoxPos(i, 1080);
	}

	int currPlace;
	switch (owner->gameModeType)
	{
	case MatchParams::GAME_MODE_REACHENEMYBASE:
	{
		for (int i = 0; i < 4; ++i)
		{
			currPlace = owner->raceFight->place[i];
			if (currPlace == 1)
			{
				ts_column[i] = GetTeamTileset(owner->GetPlayerTeamIndex(i), true);
				maxPlacing = 1;
			}
			else if (currPlace > 1)
			{
				ts_column[i] = GetTeamTileset(owner->GetPlayerTeamIndex(i), false);
				if (currPlace > maxPlacing)
					maxPlacing = currPlace;
			}
			else
			{
				ts_column[i] = NULL;
			}
		}
		break;
	}

	}
}

void ResultsScreen::Update()
{
	switch (state)
	{
	case FADEIN:
		state = SLIDEIN;
		frame = 0;
		break;
	case SLIDEIN:
		if (frame <= slideInStartFrame[maxPlacing-1] + slideInFrames[maxPlacing-1])
		{
			for (int i = 0; i < 4; ++i)
			{
				if (owner->gameModeType == MatchParams::GAME_MODE_REACHENEMYBASE)
				{
					if (owner->raceFight->place[i] == 1 && frame <= slideInFrames[0])
					{
						double f = (double)frame / slideInFrames[0];
						double a = slideInBez[1].GetValue(f);

						SetBoxPos(i, 1080.0 * (1.0 - a));
					}
					else if (owner->raceFight->place[i] == 2 && frame <= slideInFrames[1] + slideInStartFrame[1])
					{
						double f = (double)(frame - slideInFrames[1]) / slideInStartFrame[1];
						double a = slideInBez[1].GetValue(f);

						SetBoxPos(i, 1080.0 * (1.0 - a));
					}
					/*else if (owner->raceFight->place[i] == 3 && frame <= secondPlaceStartSlideFrame + secondPlaceSlideInFrames)
					{
						double f = (double)(frame - secondPlaceStartSlideFrame) / secondPlaceSlideInFrames;
						double a = secondPlaceBez.GetValue(f);

						SetBoxPos(i, 1080.0 * (1.0 - a));
					}*/
				}

				
			}
		}
		else
		{
			state = WAIT;
			frame = 0;
		}
		break;
	case WAIT:
	{
		for (int i = 0; i < 4; ++i)
		{
			if ((owner->GetCurrInput(i).A && !owner->GetPrevInput(i).A)
				|| (owner->GetCurrInput(i).start && !owner->GetPrevInput(i).start))
			{
				columnReady[i] = !columnReady[i];
			}
		}

		bool allReady = true;
		for (int i = 0; i < 4; ++i)
		{
			if (ts_column[i] != NULL && !columnReady[i] )
			{
				allReady = false;
				break;
			}
		}

		if (allReady)
		{
			state = SLIDEOUT;
			frame = 0;
		}

		break;
	}
	case SLIDEOUT:
		/*if (frame == 60)
		{
			state = FADEOUT;
			frame = 0;
		}*/

		if (frame <= slideOutFrames )
		{
			for (int i = 0; i < 4; ++i)
			{
				if (owner->gameModeType == MatchParams::GAME_MODE_REACHENEMYBASE)
				{
					double f = (double)frame / slideOutFrames;
					double a = slideOutBez.GetValue(f);
					SetBoxPos(i, 1080.0 * a);
				}
			}
		}
		else
		{
			state = DONE;
			frame = 0;
		}
		break;
	case FADEOUT:
		if (frame == 0)
		{
			state = DONE;
			frame = 0;
		}
		break;
	}

	switch (state)
	{
	case FADEIN:
		break;
	case SLIDEIN:
		break;
	case WAIT:
		break;
	case SLIDEOUT:
		break;
	case FADEOUT:
		break;
	}

	for (int i = 0; i < 4; ++i)
	{
		SetTile(i, (int)( !columnReady[i] ));
	}

	++frame;
}

void ResultsScreen::ResetSprites()
{

}

bool ResultsScreen::IsDone()
{
	return (state == DONE);
}

void ResultsScreen::UpdateSprites()
{

}

void ResultsScreen::Reset()
{
	for (int i = 0; i < 4; ++i)
	{
		columnReady[i] = false;
		columnSprites[i] = sf::Sprite();
		ts_column[i] = NULL;
	}
	frame = 0;
	state = SLIDEIN;
}

void ResultsScreen::Draw(RenderTarget *target)
{
	switch (owner->gameModeType)
	{
	case MatchParams::GAME_MODE_REACHENEMYBASE:
	{
		for (int i = 0; i < 4; ++i)
		{
			if( owner->raceFight->place[i] > 0 )
				target->draw(columnSprites[i]);
		}
		break;
	}
	}
	
}