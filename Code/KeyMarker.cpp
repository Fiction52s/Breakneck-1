#include "KeyMarker.h"
#include "Session.h"
#include "ImageText.h"
#include "Actor.h"

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

	Tileset *scoreTS = sess->GetTileset("Menu/keynum_small_32x32.png", 32, 32);
	ts_keyNumLight = sess->GetTileset("Menu/keynum_light_80x80.png", 80, 80);
	ts_keyNumDark = sess->GetTileset("Menu/keynum_dark_80x80.png", 80, 80);

	keyNumberNeededHUD = new ImageText(2, ts_keyNumDark);
	keyNumberNeededHUDBack = new ImageText(2, ts_keyNumLight);
	//keyNumberTotalHUD = new ImageText(2, scoreTS);

	SetPosition(Vector2f(1920 - 100, 100));
	//SetPosition(Vector2f(226 + 10, 141 + 10));
}

KeyMarker::~KeyMarker()
{
	delete keyNumberNeededHUD;
	delete keyNumberNeededHUDBack;
	//delete keyNumberTotalHUD;
}

void KeyMarker::SetPosition(Vector2f &pos)
{
	neededCenter = pos;//Vector2f(1920- 100, 100 );//Vector2f(62, 33) + pos;
	keyNumberNeededHUD->SetCenter(neededCenter );
	keyNumberNeededHUDBack->SetCenter(neededCenter);


	//keyNumberTotalHUD->SetCenter(neededCenter + Vector2f(-60, -40));
	
}

Vector2f KeyMarker::GetPosition()
{
	return neededCenter;
}

void KeyMarker::UpdateKeyNumbers()
{
	//owner->numKeysCollected++;
	int numKeys = sess->GetPlayer(0)->numKeysHeld;

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

	keyNumberNeededHUD->SetNumber(numKeys);
	keyNumberNeededHUDBack->SetNumber(numKeys);
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
	keyNumberNeededHUD->ts = ts_keyNumDark;

	keyNumberNeededHUD->SetNumber(0);
	keyNumberNeededHUDBack->SetNumber(0);

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
			keyNumberNeededHUD->ts = ts_keyNumDark;
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
			keyNumberNeededHUD->ts = ts_keyNumDark;
		}
		else
		{
			keyNumberNeededHUD->ts = ts_keyNumLight;
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