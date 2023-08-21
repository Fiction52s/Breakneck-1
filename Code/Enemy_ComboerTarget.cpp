#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_ComboerTarget.h"
#include "Actor.h"

using namespace std;
using namespace sf;

ComboerTarget::ComboerTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_COMBOERTARGET, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(A_IDLE, A_IDLE, 0);

	SetLevel(ap->GetLevel());

	targetType = -1;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);

	if (!myShader.loadFromFile("Resources/Shader/comboertarget_shader.frag", sf::Shader::Fragment))
	{
		cout << "couldnt load comboer target shader" << endl;
		assert(false);
	}

	ts = GetSizedTileset("Enemies/key_comboer_256x256.png");

	ts_palette = GetSizedTileset("Enemies/key_comboer_palette_17x8.png");
	sf::Image paletteImage = ts_palette->texture->copyToImage();

	hasMonitor = true;
	SetKey();

	int skinIndex = 0;

	keyObjectSprite.setTexture(*sess->ts_key->texture);

	const string &typeName = ap->GetTypeName();
	if (typeName == "bluecomboertarget")
	{
		targetType = TARGET_COMBOER_BLUE;
		skinIndex = 0;
	}
	else if (typeName == "greencomboertarget")
	{
		targetType = TARGET_COMBOER_GREEN;
		skinIndex = 1;
	}
	else if (typeName == "yellowcomboertarget")
	{
		targetType = TARGET_COMBOER_YELLOW;
		skinIndex = 2;
	}
	else if (typeName == "orangecomboertarget")
	{
		targetType = TARGET_COMBOER_ORANGE;
		skinIndex = 3;
	}
	else if (typeName == "redcomboertarget")
	{
		targetType = TARGET_COMBOER_RED;
		skinIndex = 4;
	}
	else if (typeName == "magentacomboertarget")
	{
		targetType = TARGET_COMBOER_MAGENTA;
		skinIndex = 5;
	}

	for (int i = 0; i < 17; ++i)
	{
		paletteArray[i] = sf::Glsl::Vec4(paletteImage.getPixel(i, skinIndex));
	}
	myShader.setUniformArray("u_palette", paletteArray, 17);

	keyIdleLength = 16;
	keyAnimFactor = 3;

	actionLength[A_IDLE] = 1;
	animFactor[A_IDLE] = 5;

	actionLength[A_DYING] = 11;//30
	animFactor[A_DYING] = 3;

	assert(targetType != -1);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	ResetEnemy();
}

void ComboerTarget::SetLevel(int lev)
{
	level = lev;
	/*switch (level)
	{
	case 1:
	scale = 4.0;
	break;
	case 2:
	scale = 2.0;
	maxHealth += 2;
	break;
	case 3:
	scale = 4.0;
	maxHealth += 5;
	break;
	}*/
}

void ComboerTarget::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void ComboerTarget::ResetEnemy()
{
	action = A_IDLE;
	frame = 0;

	data.keyFrame = 0;

	if (sess->currWorldDependentTilesetWorldIndex == 5)
	{
		keyAnimFactor = 6;
	}
	else
	{
		keyAnimFactor = 3;
	}

	hasMonitor = true;
	suppressMonitor = false;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void ComboerTarget::Collect()
{
	if (numHealth > 0)
	{
		numHealth = 0;
		sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		ConfirmKill();
	}
}

bool ComboerTarget::IsInteractible()
{
	return action == A_IDLE && !dead;
}

bool ComboerTarget::IsValidTrackEnemy()
{
	return IsInteractible();
}

bool ComboerTarget::IsHomingTarget()
{
	return false;
}

void ComboerTarget::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);

	if (data.keyFrame == keyIdleLength * keyAnimFactor)
	{
		data.keyFrame = 0;
	}

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case A_IDLE:
		{
			frame = 0;
			break;
		}
		case A_DYING:
		{
			dead = true;
			numHealth = 0;
			break;
		}
		}

	}
}

void ComboerTarget::UpdateSprite()
{
	sess->ts_key->SetSpriteTexture(keyObjectSprite);

	switch (action)
	{
	case A_IDLE:
	{
		sprite.setTextureRect(ts->GetSubRect(0));
		break;
	}
	case A_DYING:
	{
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[A_DYING] + 1));
		break;
	}
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());


	keyObjectSprite.setTextureRect(sess->ts_key->GetSubRect(data.keyFrame / keyAnimFactor));

	keyObjectSprite.setOrigin(keyObjectSprite.getLocalBounds().width / 2, keyObjectSprite.getLocalBounds().height / 2);
	keyObjectSprite.setPosition(GetPositionF());
}

void ComboerTarget::EnemyDraw(sf::RenderTarget *target)
{
	myShader.setUniform("u_texture", *ts->texture);

	sess->ts_key->SetSpriteTexture(keyObjectSprite);

	bool drawHurtShader = (pauseFrames >= 2 && !pauseFramesFromAttacking) && currShield == NULL;


	if(drawHurtShader)
	{
		myShader.setUniform("u_hurt", 1);
		target->draw(sprite, &myShader);
	}
	else
	{
		myShader.setUniform("u_hurt", 0);
		target->draw(sprite, &myShader);// , &keyShader);
	}

	if (action == A_IDLE)
	{
		DrawSprite(target, keyObjectSprite);
	}

	if (hasMonitor && !suppressMonitor)
	{
		target->draw(keySprite);
	}
}

HitboxInfo * ComboerTarget::IsHit(int pIndex)
{
	if (currHurtboxes == NULL)
		return NULL;

	Actor *player = sess->GetPlayer(pIndex);

	if (CanBeHitByComboer())
	{
		ComboObject *co = player->IntersectMyComboHitboxes(this, currHurtboxes, currHurtboxFrame);
		if (co != NULL)
		{
			HitboxInfo *hi = co->enemyHitboxInfo;

			Enemy *en = co->enemy;

			bool validHit = false;
			switch (targetType)
			{
			case TARGET_COMBOER_BLUE:
				if (en->type == EN_COMBOER || en->type == EN_SPLITCOMBOER)
				{
					validHit = true;
				}
				break;
			case TARGET_COMBOER_GREEN:
				if (en->type == EN_GRAVITYJUGGLER)
				{
					validHit = true;
				}
				break;
			case TARGET_COMBOER_YELLOW:
				if (en->type == EN_BOUNCEJUGGLER || en->type == EN_BALL || en->type == EN_EXPLODINGBARREL)
				{
					validHit = true;
				}
				break;
			case TARGET_COMBOER_ORANGE:
				if (en->type == EN_GRINDJUGGLER || en->type == EN_GROUNDEDGRINDJUGGLER)
				{
					validHit = true;
				}
				break;
			case TARGET_COMBOER_RED:
				if (en->type == EN_RELATIVECOMBOER || en->type == EN_HUNGRYCOMBOER)
				{
					validHit = true;
				}
				break;
				//case TARGET_COMBOER_MAGENTA:
				//	if( en->type == EN_)
			}

			if (validHit)
			{
				co->enemy->ComboHit();
				comboHitEnemyID = co->enemy->enemyIndex;

				return hi;
			}
			else
			{
				return NULL;
			}

		}
	}

	return NULL;
}

void ComboerTarget::FrameIncrement()
{
	++data.keyFrame;
}

void ComboerTarget::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		//sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		//ConfirmHitNoKill();
		sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		ConfirmKill();
		if (hasMonitor)
		{
			suppressMonitor = true;
		}
		dead = false;

		action = A_DYING;
		frame = 0;
		HitboxesOff();
		HurtboxesOff();

		numHealth = maxHealth;

		receivedHit.SetEmpty();
	}
}

int ComboerTarget::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void ComboerTarget::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void ComboerTarget::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}