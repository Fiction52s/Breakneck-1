#include "KeyMarker.h"
#include "Session.h"
#include "ImageText.h"
#include "Actor.h"
#include "MainMenu.h"
#include "Zone.h"

using namespace std;
using namespace sf;

KeyMarker::KeyMarker()
{
	sess = Session::GetSession();
	//backSprite.setTexture( *ts_keys->texture );
	//energySprite.setTexture( *ts_keyEnergy->texture );

	//backSprite.setPosition( 1920 - 256 - 40, 1080 - 256 - 40 );
	//energySprite.setPosition( backSprite.getPosition() );

	frame = 0;

	scale = .5;

	xKeyText.setFont(sess->mainMenu->arial);
	xKeyText.setCharacterSize(40);
	xKeyText.setFillColor(Color::White);
	xKeyText.setString("X");
	xKeyText.setOrigin(xKeyText.getLocalBounds().left + xKeyText.getLocalBounds().width / 2,
		xKeyText.getLocalBounds().top + xKeyText.getLocalBounds().height / 2);
	xKeyText.setScale(scale,scale);

	Tileset *scoreTS = sess->GetSizedTileset("Menu/keynum_small_32x32.png");
	ts_keyNumLight = sess->GetSizedTileset("Menu/keynum_light_80x80.png");
	ts_keyNumDark = sess->GetSizedTileset("Menu/keynum_dark_80x80.png");

	ts_enemyNumLight = sess->GetSizedTileset("Menu/keynum_red_light_80x80.png");
	ts_enemyNumDark = sess->GetSizedTileset("Menu/keynum_red_dark_80x80.png");

	ts_keyIcon = sess->GetSizedTileset("FX/key_128x128.png");
	ts_enemyIcon = sess->GetSizedTileset("HUD/enemy_hud_icon_64x64.png");

	keyNumberNeededHUD = new ImageText(2, ts_keyNumDark);
	keyNumberNeededHUDBack = new ImageText(2, ts_keyNumLight);

	keyNumberNeededHUD->SetScale(scale);
	keyNumberNeededHUDBack->SetScale(scale);

	SetMarkerType(KEY);
	//keyNumberTotalHUD = new ImageText(2, scoreTS);

	SetPosition(Vector2f(1920 - 70, 50));
	//SetPosition(Vector2f(226 + 10, 141 + 10));
}

KeyMarker::~KeyMarker()
{
	delete keyNumberNeededHUD;
	delete keyNumberNeededHUDBack;
}

void KeyMarker::SetPosition(Vector2f &pos)
{
	neededCenter = pos;//Vector2f(1920- 100, 100 );//Vector2f(62, 33) + pos;

	keyNumberNeededHUD->SetCenter(neededCenter );
	keyNumberNeededHUDBack->SetCenter(neededCenter);

	keyIconSpr.setPosition(neededCenter + Vector2f( -150 * scale, 0 ));
	xKeyText.setPosition(neededCenter + Vector2f(-75 * scale, 0));
	//keyNumberTotalHUD->SetCenter(neededCenter + Vector2f(-60, -40));
	
}

Vector2f KeyMarker::GetPosition()
{
	return neededCenter;
}

void KeyMarker::SetMarkerType(int k)
{
	markerType = (MarkerType)k;

	Tileset *ts_icon = NULL;
	if (markerType == KEY)
	{
		ts_icon = ts_keyIcon;

		keyNumberNeededHUD->ts = ts_keyNumDark;
		keyNumberNeededHUDBack->ts = ts_keyNumLight;
	}
	else if (markerType == ENEMY)
	{
		ts_icon = ts_enemyIcon;

		keyNumberNeededHUD->ts = ts_enemyNumDark;
		keyNumberNeededHUDBack->ts = ts_enemyNumLight;
	}

	keyIconSpr.setTexture(*ts_icon->texture);
	keyIconSpr.setTextureRect(ts_icon->GetSubRect(0));
	keyIconSpr.setOrigin(keyIconSpr.getLocalBounds().width / 2,
		keyIconSpr.getLocalBounds().height / 2);

	if (markerType == ENEMY)
	{
		keyIconSpr.setScale(scale * 2, scale * 2);
	}
	else
	{
		keyIconSpr.setScale(scale, scale);
	}
}

void KeyMarker::UpdateKeyNumbers()
{
	//owner->numKeysCollected++;
	if (sess->currentZone == NULL)
	{
		return;
	}

	//--keysRequired;

	//if( keysRequired == 0 )
	//{
		//int soundIndex = GameSession::SoundType::S_KEY_COMPLETE_W1 + ( startKeys - 1 );
		//owner->soundNodeList->ActivateSound( owner->gameSoundBuffers[soundIndex] );
		//state = TOZERO;
		//frame = 0;
	//}
	//else
	//{
		//SetEnergySprite();
	//}

	VibrateNumbers();

	if (markerType == KEY)
	{
		int numKeys = sess->GetPlayer(0)->numKeysHeld;
		keyNumberNeededHUD->SetNumber(numKeys);
		keyNumberNeededHUDBack->SetNumber(numKeys);

		bool makeRing = false;
		for (auto it = sess->currentZone->gates.begin(); it != sess->currentZone->gates.end(); ++it)
		{
			Gate *g = (Gate*)(*it)->info;
			if (g->category == Gate::ALLKEY || g->NUMBER_KEY)
			{
				if (g->numToOpen == numKeys)
				{
					makeRing = true;
					break;
				}
			}
		}

		if (makeRing)
		{
			sess->GetPlayer(0)->CreateEnoughKeysRing();
		}
	}
	else if (markerType == ENEMY)
	{
		int numEnemiesRemaining = sess->currentZone->GetNumRemainingKillableEnemies();
		keyNumberNeededHUD->SetNumber(numEnemiesRemaining);
		keyNumberNeededHUDBack->SetNumber(numEnemiesRemaining);

		if (numEnemiesRemaining == 0)
		{
			sess->GetPlayer(0)->CreateEnemiesClearedRing();
		}
	}
	
	//keyNumberTotalHUD->SetNumber(numKeys);

	keyNumberNeededHUD->UpdateSprite();
	keyNumberNeededHUDBack->UpdateSprite();


	//keyNumberTotalHUD->UpdateSprite();
}

void KeyMarker::SetStartKeysZone(Zone *z)
{
	//keyNumberNeededHUD->SetNumber(0);
	//keyNumberNeededHUDBack->SetNumber(0);
	//keyNumberTotalHUD->SetNumber(z->;
	Reset();
	//SetStartKeys(z->requiredKeys, z->totalStartingKeys);
}

void KeyMarker::SetStartKeys( int neededKeys, int totalKeys )
{
	if (neededKeys > totalKeys)
		neededKeys = totalKeys;

	
	keyNumberNeededHUD->SetNumber(neededKeys);
	keyNumberNeededHUDBack->SetNumber(neededKeys);
	//keyNumberTotalHUD->SetNumber(totalKeys);


	keyNumberNeededHUD->UpdateSprite();
	keyNumberNeededHUDBack->UpdateSprite();
//	keyNumberTotalHUD->UpdateSprite();
	//set bg sprite

	Reset();
	//SetEnergySprite();
}

void KeyMarker::Reset()
{
	action = IDLE;
	frame = 0;
	

	if (markerType == KEY)
	{
		keyNumberNeededHUD->SetNumber(0);
		keyNumberNeededHUDBack->SetNumber(0);

		keyNumberNeededHUD->ts = ts_keyNumDark;
	}
	else if (markerType == ENEMY)
	{
		int numEnemiesRemaining = sess->currentZone->GetNumRemainingKillableEnemies();
		keyNumberNeededHUD->SetNumber(numEnemiesRemaining);
		keyNumberNeededHUDBack->SetNumber(numEnemiesRemaining);

		keyNumberNeededHUD->ts = ts_enemyNumDark;
	}
	

	keyNumberNeededHUDBack->SetCenter(neededCenter);
	keyNumberNeededHUDBack->UpdateSprite();
	keyNumberNeededHUD->SetCenter(neededCenter);
	keyNumberNeededHUD->UpdateSprite();
}

void KeyMarker::Draw( sf::RenderTarget *target )
{

	if (action == VIBRATING && frame < 20 )
	{
		//cout << "blah: " << keyNumberNeededHUDBack->center.x << ", " << keyNumberNeededHUDBack->center.y << endl;
		keyNumberNeededHUDBack->Draw(target);
	}
	

	//keyNumberTotalHUD->Draw(target);
	
	keyNumberNeededHUD->Draw(target);
	
	int val = keyNumberNeededHUD->value;

	target->draw(xKeyText);
	target->draw(keyIconSpr);

	//target->draw( backSprite );
	//if( state == NONZERO )
	//{
	//	target->draw( energySprite );
	//}
}

void KeyMarker::Update()
{
	switch (action)
	{
	case IDLE:
		break;
	case VIBRATING:
	{
		if (frame == 30)
		{
			action = IDLE;
			frame = 0;
			if (markerType == KEY)
			{
				keyNumberNeededHUD->ts = ts_keyNumDark;
			}
			else if (markerType == ENEMY)
			{
				keyNumberNeededHUD->ts = ts_enemyNumDark;
			}
			
		}
		else if (frame < 20)
		{
			int rx = (rand() % 3) - 1;
			int ry = (rand() % 3) - 1;
			rx *= 10;
			ry *= 10;
			keyNumberNeededHUDBack->SetCenter(keyNumberNeededHUD->center + Vector2f(rx, ry));
			keyNumberNeededHUDBack->UpdateSprite();

			int rx1 = (rand() % 3) - 1;
			int ry1 = (rand() % 3) - 1;
			rx1 *= 2;
			ry1 *= 2;

			keyNumberNeededHUD->SetCenter(keyNumberNeededHUD->center + Vector2f(rx1, ry1));
			keyNumberNeededHUD->UpdateSprite();
		}
		else if (frame == 20)
		{
			keyNumberNeededHUDBack->SetCenter(neededCenter);
			keyNumberNeededHUDBack->UpdateSprite();
			keyNumberNeededHUD->SetCenter(neededCenter);
			keyNumberNeededHUD->UpdateSprite();
		}

		if (frame % 2 == 0)
		{
			if (markerType == KEY)
			{
				keyNumberNeededHUD->ts = ts_keyNumDark;
			}
			else if (markerType == ENEMY)
			{
				keyNumberNeededHUD->ts = ts_enemyNumDark;
			}
		}
		else
		{
			if (markerType == KEY)
			{
				keyNumberNeededHUD->ts = ts_keyNumLight;
			}
			else if (markerType == ENEMY)
			{
				keyNumberNeededHUD->ts = ts_enemyNumLight;
			}
		}
		//8 frames vibrate, 
		//blink every other frame
		break;
	}
		
	}
	++frame;
	/*int growMult = 8;
	int shrinkMult = 8;
	switch( state )
	{
		case TOZERO:
		{
			if( frame == 4 * shrinkMult )
			{
				state = ZERO;
				frame = 0;
				backSprite.setTextureRect( ts_keys->GetSubRect( 24 ) );
				break;
			}

			int f = ((startKeys-1) * 4) + (3 - (frame / shrinkMult));
			backSprite.setTextureRect( ts_keys->GetSubRect( f ) );
			++frame;

			break;
		}
		case FROMZERO:
		{
			if( frame == 4 * growMult )
			{
				state = NONZERO;
				frame = 0;
				break;
			}
			int f = ((startKeys-1) * 4) + (frame / growMult);
			backSprite.setTextureRect( ts_keys->GetSubRect( f ) );
			++frame;
			break;
		}
	}*/
}

void KeyMarker::VibrateNumbers()
{
	action = VIBRATING;
	frame = 0;
}