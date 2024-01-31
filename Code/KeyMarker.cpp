#include "KeyMarker.h"
#include "Session.h"
#include "ImageText.h"
#include "Actor.h"
#include "MainMenu.h"
#include "Zone.h"

using namespace std;
using namespace sf;

KeyMarker::KeyMarker(TilesetManager *tm )
{
	sess = NULL;

	showMaxKeys = false;

	frame = 0;

	MainMenu *mm = MainMenu::GetInstance();

	scale = .75;//.75;//1.0;//.5;

	xKeyText.setFont(mm->arial);
	xKeyText.setCharacterSize(40);
	xKeyText.setFillColor(Color::White);
	xKeyText.setString("X");
	xKeyText.setOrigin(xKeyText.getLocalBounds().left + xKeyText.getLocalBounds().width / 2,
		xKeyText.getLocalBounds().top + xKeyText.getLocalBounds().height / 2);
	xKeyText.setScale(scale,scale);

	slashText.setFont(mm->arial);
	slashText.setCharacterSize(80);
	slashText.setFillColor(Color::White);
	slashText.setString("/");
	slashText.setOrigin(slashText.getLocalBounds().left + slashText.getLocalBounds().width / 2,
		slashText.getLocalBounds().top + slashText.getLocalBounds().height / 2);
	slashText.setScale(scale, scale);

	ts_keyNumLight = tm->GetSizedTileset("HUD/keynum_light_80x80.png");
	ts_keyNumDark = tm->GetSizedTileset("HUD/keynum_dark_80x80.png");
	ts_enemyNumLight = tm->GetSizedTileset("HUD/keynum_red_light_80x80.png");
	ts_enemyNumDark = tm->GetSizedTileset("HUD/keynum_red_dark_80x80.png");
	ts_icon = tm->GetSizedTileset("HUD/keymarker_icon_96x96.png");

	float textSpacingFactor = .7;

	keyNumberNeededHUD = new ImageText(2, ts_keyNumDark);
	keyNumberNeededHUD->SetSpacingFactor(textSpacingFactor);
	keyNumberNeededHUDBack = new ImageText(2, ts_keyNumLight);
	keyNumberNeededHUDBack->SetSpacingFactor(textSpacingFactor);

	keyNumberNeededHUD->SetScale(scale);
	keyNumberNeededHUDBack->SetScale(scale);

	keyNumberTotalHUD = new ImageText(2, ts_keyNumDark);
	keyNumberTotalHUD->SetSpacingFactor(textSpacingFactor);
	keyNumberTotalHUD->SetScale(scale);

	SetMarkerType(KEY);

	SetPosition(Vector2f(1920 - 70, 50));
}

KeyMarker::~KeyMarker()
{
	delete keyNumberNeededHUD;
	delete keyNumberNeededHUDBack;

	delete keyNumberTotalHUD;
}

void KeyMarker::SetSession(Session *p_sess)
{
	sess = p_sess;

	SetMarkerType(markerType);
}

void KeyMarker::ShowMaxKeys(int k)
{
	showMaxKeys = true;
	keyNumberTotalHUD->SetNumber(k);
	keyNumberTotalHUD->UpdateSprite();
	RefreshPosition();
	//SetTopLeft(anchor);
}

void KeyMarker::HideMaxKeys()
{
	showMaxKeys = false;
}

void KeyMarker::SetTopRight(sf::Vector2f &pos)
{
	anchor = pos;
	posType = TOP_RIGHT;

	float currRight = anchor.x;

	if (showMaxKeys && markerType == KEY)
	{
		keyNumberTotalHUD->SetTopRight(Vector2f(currRight, anchor.y));
		float keyNumberTotalWidth = keyNumberTotalHUD->GetWidth();

		currRight -= keyNumberTotalWidth;

		float slashSpacing = 10;

		slashText.setPosition(Vector2f(currRight - (xKeyText.getGlobalBounds().width / 2 + slashSpacing), anchor.y + keyNumberNeededHUD->GetHeight() / 2.f));
		currRight -= xKeyText.getGlobalBounds().width + slashSpacing;

		currRight -= 20;//slashSpacing / 2;
	}

	//new ^


	keyNumberAnchor = Vector2f(currRight, anchor.y);

	keyNumberNeededHUD->SetTopRight(keyNumberAnchor);
	keyNumberNeededHUDBack->SetTopRight(keyNumberAnchor);
	currRight -= keyNumberNeededHUD->GetWidth();

	float keySpacing = 20;
	float iconSpacing = 10;

	xKeyText.setPosition( Vector2f( currRight - (xKeyText.getGlobalBounds().width / 2 + keySpacing ), anchor.y + keyNumberNeededHUD->GetHeight() / 2.f ));

	currRight -= xKeyText.getGlobalBounds().width + keySpacing;

	float iconExtraY = 0;
	if (markerType == ENEMY)
	{
		iconExtraY = 0;
	}
	

	keyIconSpr.setPosition( Vector2f( currRight - keyIconSpr.getGlobalBounds().width / 2, anchor.y + keyNumberNeededHUD->GetHeight() / 2.f + iconExtraY));

}

void KeyMarker::SetTopLeft(sf::Vector2f &pos)
{
	anchor = pos;
	posType = TOP_LEFT;

	float currLeft = anchor.x;

	

	//new ^

	float keyWidth = keyIconSpr.getGlobalBounds().width;
	float keySpacing = 20;
	float iconSpacing = 10;


	Vector2f iconPos = Vector2f(currLeft + keyWidth / 2, anchor.y + keyNumberNeededHUD->GetHeight() / 2.f);

	keyIconSpr.setPosition(iconPos);
	currLeft += keyWidth + keySpacing;

	if (markerType == ENEMY)
	{
		currLeft -= 10;
	}

	xKeyText.setPosition(Vector2f(currLeft + (xKeyText.getGlobalBounds().width / 2), anchor.y + keyNumberNeededHUD->GetHeight() / 2.f));

	currLeft += xKeyText.getGlobalBounds().width + keySpacing;

	keyNumberAnchor = Vector2f(currLeft, anchor.y);

	keyNumberNeededHUD->SetTopLeft(keyNumberAnchor);
	keyNumberNeededHUDBack->SetTopLeft(keyNumberAnchor);
	currLeft += keyNumberNeededHUD->GetWidth();

	if (showMaxKeys && markerType == KEY)
	{
		keyNumberTotalHUD->SetTopLeft(Vector2f(currLeft, anchor.y));
		float keyNumberTotalWidth = keyNumberTotalHUD->GetWidth();

		currLeft += keyNumberTotalWidth;

		float slashSpacing = 10;

		slashText.setPosition(Vector2f(currLeft - (xKeyText.getGlobalBounds().width / 2 + slashSpacing), anchor.y + keyNumberNeededHUD->GetHeight() / 2.f));
		currLeft = xKeyText.getGlobalBounds().width + slashSpacing;

		currLeft += 20;//slashSpacing / 2;
	}
	//currLeft += xKeyText.getGlobalBounds().width + keySpacing;
}

void KeyMarker::SetCenter(sf::Vector2f &pos)
{
	anchor = pos;
	posType = CENTER;


	float keyWidth = keyIconSpr.getGlobalBounds().width;
	float halfNumberHeight = keyNumberNeededHUD->GetHeight() / 2.f;
	float halfXWidth = xKeyText.getGlobalBounds().width / 2;
	float keySpacing = 10;
	float iconSpacing = 10;

	

	xKeyText.setPosition(Vector2f(anchor.x, anchor.y));// +keyNumberNeededHUD->GetHeight() / 2.f));

	Vector2f iconPos = Vector2f(anchor.x - (keyWidth / 2 + keySpacing + halfXWidth), anchor.y);

	if (markerType == ENEMY)
	{
		iconPos.x += 10;
	}

	keyIconSpr.setPosition(iconPos);// +keyNumberNeededHUD->GetHeight() / 2.f));

	Vector2f numTopLeft = anchor;
	numTopLeft += Vector2f(halfXWidth + keySpacing, -halfNumberHeight);

	keyNumberAnchor = numTopLeft;

	keyNumberNeededHUD->SetTopLeft(keyNumberAnchor);
	keyNumberNeededHUDBack->SetTopLeft(keyNumberAnchor);

	numTopLeft.x += keyNumberNeededHUD->GetWidth();

	if (showMaxKeys && markerType == KEY)
	{
		float slashSpacing = 10;
		numTopLeft.x += slashSpacing + 20;
		
		slashText.setPosition(numTopLeft.x, anchor.y);

		numTopLeft.x += slashText.getGlobalBounds().width;// +slashSpacing;

		keyNumberTotalHUD->SetTopLeft(numTopLeft);
	}

	//SetPosition(pos);
}

void KeyMarker::SetPosition(Vector2f &pos)
{
	//neededCenter = pos;//Vector2f(1920- 100, 100 );//Vector2f(62, 33) + pos;
	anchor = pos;
	posType = CENTER;

	keyNumberNeededHUD->SetCenter(anchor);
	keyNumberNeededHUDBack->SetCenter(anchor);
	keyNumberTotalHUD->SetCenter(anchor + Vector2f(100 * scale, 0));

	keyIconSpr.setPosition(anchor + Vector2f( -150 * scale, 0 ));
	xKeyText.setPosition(anchor + Vector2f(-75 * scale, 0));
	slashText.setPosition(anchor + Vector2f(50 * scale, 0));
	//keyNumberTotalHUD->SetCenter(neededCenter + Vector2f(-60, -40));
	
}

Vector2f KeyMarker::GetPosition()
{
	return neededCenter;
}

void KeyMarker::SetMarkerType(int k)
{
	markerType = (MarkerType)k;

	if (markerType == KEY)
	{
		keyNumberNeededHUD->ts = ts_keyNumDark;
		keyNumberNeededHUDBack->ts = ts_keyNumLight;
	}
	else if (markerType == ENEMY)
	{
		keyNumberNeededHUD->ts = ts_enemyNumDark;
		keyNumberNeededHUDBack->ts = ts_enemyNumLight;
	}

	if (sess != NULL)
	{
		keyIconSpr.setTexture(*ts_icon->texture);
		keyIconSpr.setTextureRect(ts_icon->GetSubRect(0));

		keyIconSpr.setScale(scale / .75, scale / .75);

		if (markerType == ENEMY)
		{
			keyIconSpr.setTextureRect(ts_icon->GetSubRect(8));
			//keyIconSpr.setScale(scale * 2, scale * 2);
		}
		else
		{
			assert(sess->currWorldDependentTilesetWorldIndex >= 0);
			keyIconSpr.setTextureRect(ts_icon->GetSubRect(sess->currWorldDependentTilesetWorldIndex));
		}

		keyIconSpr.setOrigin(keyIconSpr.getLocalBounds().width / 2,
			keyIconSpr.getLocalBounds().height / 2);

	}
}

void KeyMarker::RefreshPosition()
{
	switch (posType)
	{
	case TOP_RIGHT:
		SetTopRight(anchor);
		break;
	case TOP_LEFT:
		SetTopLeft(anchor);
		break;
	case CENTER:
		SetCenter(anchor);
		break;
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

	SetMarkerType(markerType);

	VibrateNumbers();

	if (markerType == KEY)
	{
		int numKeys = sess->GetPlayer(0)->numKeysHeld;

		keyNumberNeededHUD->SetNumber(numKeys);
		keyNumberNeededHUDBack->SetNumber(numKeys);
		RefreshPosition();

		bool makeRing = false;
		for (auto it = sess->currentZone->gates.begin(); it != sess->currentZone->gates.end(); ++it)
		{
			Gate *g = (Gate*)(*it)->info;
			if (g->IsLockedForever() )
			{
				continue;
			}

			if (g->category == Gate::ALLKEY || g->category == Gate::NUMBER_KEY)
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
		RefreshPosition();

		if (numEnemiesRemaining == 0)
		{
			sess->GetPlayer(0)->CreateEnemiesClearedRing();
		}
	}
	
	//keyNumberTotalHUD->SetNumber(numKeys);

	keyNumberNeededHUD->UpdateSprite();
	keyNumberNeededHUDBack->UpdateSprite();

	RefreshPosition();
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

	RefreshPosition();


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

		RefreshPosition();

		keyNumberNeededHUD->ts = ts_keyNumDark;
	}
	else if (markerType == ENEMY)
	{
		if (sess->currentZone != NULL)
		{
			int numEnemiesRemaining = sess->currentZone->GetNumRemainingKillableEnemies();
			keyNumberNeededHUD->SetNumber(numEnemiesRemaining);
			keyNumberNeededHUDBack->SetNumber(numEnemiesRemaining);

			RefreshPosition();
		}

		keyNumberNeededHUD->ts = ts_enemyNumDark;
	}
	

	//keyNumberNeededHUDBack->SetCenter(neededCenter);
	keyNumberNeededHUDBack->UpdateSprite();
	//keyNumberNeededHUD->SetCenter(neededCenter);
	keyNumberNeededHUD->UpdateSprite();
}

void KeyMarker::Draw( sf::RenderTarget *target )
{

	if (action == VIBRATING && frame < 20 )
	{
		keyNumberNeededHUDBack->Draw(target);
	}
	

	
	
	keyNumberNeededHUD->Draw(target);
	
	int val = keyNumberNeededHUD->value;

	target->draw(xKeyText);
	target->draw(keyIconSpr);

	if (showMaxKeys && markerType == KEY)
	{
		target->draw(slashText);
		keyNumberTotalHUD->Draw(target);
	}
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
			int amount = 5;
			rx *= amount;
			ry *= amount;
			//keyNumberNeededHUDBack->SetTopRight(keyNumberNeededHUD->anchor + Vector2f(rx, ry));

			//keyNumberNeededHUDBack->SetTopLeft(keyNumberNeededHUD->anchor + Vector2f(rx, ry));
			
			if (keyNumberNeededHUDBack->posType == ImageText::PositionType::TOP_LEFT)
			{
				keyNumberNeededHUDBack->SetTopLeft(keyNumberAnchor + Vector2f(rx, ry));
			}
			if (keyNumberNeededHUDBack->posType == ImageText::PositionType::TOP_RIGHT)
			{
				keyNumberNeededHUDBack->SetTopRight(keyNumberAnchor + Vector2f(rx, ry));
			}
			if (keyNumberNeededHUDBack->posType == ImageText::PositionType::CENTER)
			{
				keyNumberNeededHUDBack->SetCenter(keyNumberAnchor + Vector2f(rx, ry));
			}
			
			keyNumberNeededHUDBack->UpdateSprite();

			int rx1 = (rand() % 3) - 1;
			int ry1 = (rand() % 3) - 1;
			rx1 *= 2;
			ry1 *= 2;


			if (keyNumberNeededHUD->posType == ImageText::PositionType::TOP_LEFT)
			{
				keyNumberNeededHUD->SetTopLeft(keyNumberAnchor + Vector2f(rx1, ry1));
			}
			if (keyNumberNeededHUD->posType == ImageText::PositionType::TOP_RIGHT)
			{
				keyNumberNeededHUD->SetTopRight(keyNumberAnchor + Vector2f(rx1, ry1));
			}
			if (keyNumberNeededHUD->posType == ImageText::PositionType::CENTER)
			{
				keyNumberNeededHUD->SetCenter(keyNumberAnchor + Vector2f(rx1, ry1));
			}

			keyNumberNeededHUD->UpdateSprite();
		}
		else if (frame == 20)
		{
			RefreshPosition();
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