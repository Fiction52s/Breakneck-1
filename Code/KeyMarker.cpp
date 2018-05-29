#include "KeyMarker.h"
#include "GameSession.h"
#include "ImageText.h"

using namespace std;
using namespace sf;

KeyMarker::KeyMarker( GameSession *p_owner )
{
	owner = p_owner;
	//ts_keys = owner->GetTileset( "keys_256x256.png", 256, 256 );
	//ts_keyEnergy = owner->GetTileset( "keys_energy_256x256.png", 256, 256 );

	//backSprite.setTexture( *ts_keys->texture );
	//energySprite.setTexture( *ts_keyEnergy->texture );

	//backSprite.setPosition( 1920 - 256 - 40, 1080 - 256 - 40 );
	//energySprite.setPosition( backSprite.getPosition() );

	

	//state = ZERO;
	startKeys = 0;
	keysRequired = 0;
	frame = 0;

	ts_keyRing = owner->GetTileset("keyring_40x40.png", 40, 40);
	keyRingSpr.setTexture(*ts_keyRing->texture);
	keyRingSpr.setTextureRect(ts_keyRing->GetSubRect(0));
	keyRingSpr.setOrigin(keyRingSpr.getLocalBounds().width / 2, 
		keyRingSpr.getLocalBounds().height / 2);
	


	Tileset *scoreTS = owner->GetTileset("Menu/keynum_small_32x32.png", 32, 32);
	ts_keyNumLight = owner->GetTileset("Menu/keynum_light_80x80.png", 80, 80);
	ts_keyNumDark = owner->GetTileset("Menu/keynum_dark_80x80.png", 80, 80);

	keyNumberNeededHUD = new ImageText(2, ts_keyNumDark);
	keyNumberNeededHUDBack = new ImageText(2, ts_keyNumLight);
	keyNumberTotalHUD = new ImageText(2, scoreTS);

	Tileset *keyBGTS = owner->GetTileset("keyframe_81x81.png", 81, 81);

	keyNumberHUDBG.setTexture(*keyBGTS->texture);

	SetPosition(Vector2f(226 + 10, 141 + 10));
	//keyNumberNeededHUD->SetCenter(Vector2f(400, 400));
	//keyNumberTotalHUD->SetCenter(Vector2f(600, 400));
	
}

void KeyMarker::SetPosition(Vector2f &pos)
{
	keyNumberHUDBG.setPosition(pos);
	neededCenter = Vector2f(1920- 100, 100 );//Vector2f(62, 33) + pos;
	keyNumberNeededHUD->SetCenter(neededCenter );
	keyNumberNeededHUDBack->SetCenter(neededCenter);
	keyRingSpr.setPosition(neededCenter);

	keyNumberTotalHUD->SetCenter(neededCenter + Vector2f(-60, -40));
}

void KeyMarker::CollectKey()
{
	owner->numKeysCollected++;
	if( keysRequired == 0 )
	{
		return;
	}
	//assert( keysRequired > 0 );
	
	
	//int f = ((maxKeys-1) * 4) + (frame / growMult);

	--keysRequired;

	if( keysRequired == 0 )
	{
		int soundIndex = GameSession::SoundType::S_KEY_COMPLETE_W1 + ( startKeys - 1 );
		owner->soundNodeList->ActivateSound( owner->gameSoundBuffers[soundIndex] );
		//state = TOZERO;
		frame = 0;
	}
	else
	{
		//SetEnergySprite();
	}

	keyNumberNeededHUD->SetNumber(keyNumberNeededHUD->value-1);
	keyNumberNeededHUDBack->SetNumber(keyNumberNeededHUD->value);
	keyNumberTotalHUD->SetNumber(keyNumberTotalHUD->value-1);

	keyNumberNeededHUD->UpdateSprite();
	keyNumberNeededHUDBack->UpdateSprite();
	keyNumberTotalHUD->UpdateSprite();
}

void KeyMarker::SetStartKeysZone(Zone *z)
{
	SetStartKeys(z->requiredKeys, z->totalStartingKeys);
}

void KeyMarker::SetStartKeys( int neededKeys, int totalKeys )
{
	if (neededKeys > totalKeys)
		neededKeys = totalKeys;

	startKeys = neededKeys;
	keysRequired = neededKeys;
	
	keyNumberNeededHUD->SetNumber(neededKeys);
	keyNumberNeededHUDBack->SetNumber(neededKeys);
	keyNumberTotalHUD->SetNumber(totalKeys);


	keyNumberNeededHUD->UpdateSprite();
	keyNumberNeededHUDBack->UpdateSprite();
	keyNumberTotalHUD->UpdateSprite();
	//set bg sprite
	
	if(neededKeys > 0 )
	{
		//state = FROMZERO;
		//frame = 0;
	}
	else
	{
		//state = ZERO;
		//backSprite.setTextureRect( ts_keys->GetSubRect( 24 ) );
	}

	Reset();
	//SetEnergySprite();
}

void KeyMarker::Reset()
{
	action = IDLE;
	frame = 0;
	keyNumberNeededHUD->ts = ts_keyNumDark;

	keyNumberNeededHUDBack->SetCenter(neededCenter);
	keyNumberNeededHUDBack->UpdateSprite();
	keyNumberNeededHUD->SetCenter(neededCenter);
	keyNumberNeededHUD->UpdateSprite();
}

void KeyMarker::Draw( sf::RenderTarget *target )
{
	//target->draw(keyNumberHUDBG);

	if (action == VIBRATING && frame < 20 )
	{
		//cout << "blah: " << keyNumberNeededHUDBack->center.x << ", " << keyNumberNeededHUDBack->center.y << endl;
		keyNumberNeededHUDBack->Draw(target);
	}
	

	keyNumberTotalHUD->Draw(target);
	
	keyNumberNeededHUD->Draw(target);
	
	int val = keyNumberNeededHUD->value;
	if ( val >= 1 && val <= 3)
	{
		//keyRingSpr.setTextureRect(ts_keyRing->GetSubRect(val -1));
		//target->draw(keyRingSpr);
	}
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