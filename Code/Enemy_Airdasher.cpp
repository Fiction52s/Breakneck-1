#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Airdasher.h"
#include "Eye.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

void Airdasher::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

Airdasher::Airdasher( ActorParams *ap )//bool p_hasMonitor, Vector2i pos, int p_level )
	:Enemy(EnemyType::EN_AIRDASHER, ap )//, p_hasMonitor, 1)
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, S_FLOAT, 0);

	actionLength[S_FLOAT] = 11;
	actionLength[S_DASH] = 30;
	actionLength[S_RETURN] = 60;
	actionLength[S_OUT] = 20;

	animFactor[S_FLOAT] = 4;
	animFactor[S_DASH] = 1;
	animFactor[S_RETURN] = 1;
	animFactor[S_OUT] = 1;

	SetLevel(ap->GetLevel());

	speed = 20;
	hitLimit = 5;
	dashRadius = 600;//500;
	dashFrames = 36;
	returnFrames = 30;

	ts = GetSizedTileset("Enemies/W1/airdasher_176x112.png");
	
	sprite.setTexture(*ts->texture);	
	sprite.setScale(scale, scale);

	//BasicCircleHurtBodySetup(48);
	//BasicCircleHitBodySetup(48);

	hitBody.ResetFrames();
	hitBody.SetupNumFrames(2);
	hitBody.SetupNumBoxesOnFrame(0, 1);
	hitBody.SetupNumBoxesOnFrame(1, 1);
	hitBody.AddBasicCircle(0, 48, 0, V2d());
	hitBody.AddBasicRect(1, 60, 25, 0, V2d(0,-10));
	

	hurtBody.ResetFrames();
	hurtBody.SetupNumFrames(2);
	hurtBody.SetupNumBoxesOnFrame(0, 1);
	hurtBody.SetupNumBoxesOnFrame(1, 1);
	hurtBody.AddBasicCircle(0, 48, 0, V2d());
	hurtBody.AddBasicRect(1, 60, 25, 0, V2d(0, -10));

	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 8;
	hitBody.hitboxInfo = hitboxInfo;
	hitboxInfo->kbDir = V2d(1, 0);
	hitboxInfo->hitPosType = HitboxInfo::AIRFORWARD;
	hitboxInfo->hType = HitboxInfo::BLUE;

	maxCharge = 15;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(14);
	cutObject->SetSubRectBack(13);
	cutObject->SetScale(scale);

	ResetEnemy();
}

Airdasher::~Airdasher()
{
}

void Airdasher::ResetEnemy()
{
	SetCurrPosInfo(startPosInfo);

	data.hitFrame = -1;
	data.currHits = 0;
	data.currOrig = startPosInfo.GetPosition();
	action = S_FLOAT;
	frame = 0;
	sprite.setRotation(0);
	
	if (PlayerDir().x >= 0)
		facingRight = true;
	else
		facingRight = false;

	data.physStepIndex = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void Airdasher::UpdateHitboxes()
{
	V2d position = GetPosition();


	double ang = 0;

	if (action == S_DASH)
	{
		ang = GetVectorAngleCW(data.dashDir);
		if (!facingRight)
		{
			ang = ang + PI;
		}
	}
	//can update this with a universal angle at some point
	if (!hurtBody.IsEmpty())
	{
		hurtBody.SetBasicPos(currHurtboxFrame, position, ang);
		hurtBody.GetCollisionBoxes(currHurtboxFrame).at(0).flipHorizontal = !facingRight;
	}

	if (!hitBody.IsEmpty())
	{
		hitBody.SetBasicPos(currHitboxFrame, position, ang);
		hitBody.GetCollisionBoxes(currHitboxFrame).at(0).flipHorizontal = !facingRight;
	}

	auto comboBoxes = GetComboHitboxes();
	if (comboBoxes != NULL)
	{
		for (auto it = comboBoxes->begin(); it != comboBoxes->end(); ++it)
		{
			(*it).globalPosition = position;
		}
	}
	

	//switch (action)
	//{
	//case S_FLOAT:
	//	hitboxInfo->kbDir = normalize(V2d(1, -1));
	//	break;
	//case S_DASH:
	//	hitboxInfo->kbDir = dashDir;//normalize(V2d(1, -1));
	//	break;
	//case S_RETURN:
	//	hitboxInfo->kbDir = -dashDir;//normalize(V2d(1, -1));
	//	break;
	//case S_OUT:
	//	hitboxInfo->kbDir = normalize(V2d(1, -1));
	//	break;
	//}
	/*actionLength[S_FLOAT] = 11;
	actionLength[S_DASH] = 30;
	actionLength[S_RETURN] = 60;
	actionLength[S_OUT] = 20;*/
}

void Airdasher::ProcessState()
{
	V2d playerPos = GetFocusedPlayerPos();//sess->GetPlayerPos();
	
	//cout << "state: " << action << ", frame: " << frame << "\n";

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_DASH:
		{
			//action = S_RETURN;
			break;
		}
		case S_RETURN:
			//action = S_FLOAT;
			break;
		case S_OUT:
		{
			action = S_RETURN;
			sprite.setRotation(0);
			V2d pDir = normalize(playerPos - GetPosition());
			SetFacingSide(pDir);
			break;
		}
		}
	}

	bool withinRange = length(playerPos - GetPosition()) < ( dashRadius + 10.0 );
	if (withinRange)
	{
		if (action == S_FLOAT)
		{
			action = S_CHARGE;
			frame = 0;
			data.chargeFrames = 0;
			data.playerDir = normalize(playerPos - GetPosition());
			SetFacingPlayerAngle();
		}
		else if (action == S_CHARGE)
		{
			data.playerDir = normalize(playerPos - GetPosition());
			SetFacingPlayerAngle();
			if (data.chargeFrames == maxCharge)
			{
				data.dashDir = data.playerDir;
				action = S_DASH;
				frame = 0;
				data.physStepIndex = 0;

				DefaultHitboxesOn(1);
				DefaultHurtboxesOn(1);
				
				//cout << "angle : " << angle << endl;
			}
			else
			{
				data.chargeFrames++;
			}
		}
	}
	else
	{
		if (action == S_CHARGE)
		{
			action = S_FLOAT;
			frame = 0;
			sprite.setRotation(0);
		}
	}
	
}

void Airdasher::IHitPlayer(int index)
{

}

void Airdasher::SetFacingSide( V2d pDir )
{
	if (pDir.x < 0)
	{
		facingRight = false;
	}
	else if (pDir.x > 0)
	{
		facingRight = true;
	}
	else if (pDir.y == 1)
	{
		facingRight = true;
	}
	else if (pDir.y == -1)
	{
		facingRight = false;
	}
	else
	{
		assert(0);
	}
}

double Airdasher::SetFacingPlayerAngle()
{
	double angle = -atan2(data.playerDir.y, -data.playerDir.x) / PI * 180.0;
	

	if (data.playerDir.x < 0)
	{
		facingRight = false;
	}
	else if (data.playerDir.x > 0)
	{
		facingRight = true;
		angle += 180;
	}
	else if (data.playerDir.y == 1)
	{
		facingRight = true;
	}
	else if (data.playerDir.y == -1)
	{
		facingRight = false;
	}
	else
	{
		assert(0);
	}

	sprite.setRotation(angle);

	return angle;
}

void Airdasher::UpdateEnemyPhysics()
{
	V2d playerPos = sess->GetPlayerPos();
	V2d dest = data.currOrig + data.playerDir * dashRadius;
	switch (action)
	{
	case S_DASH:
	{
		double a = (double)data.physStepIndex / (dashFrames * NUM_MAX_STEPS * 5);
		if (a > 1.0)
		{
			action = S_OUT;
			frame = 0;
			data.physStepIndex = 0;

			DefaultHitboxesOn(0);
			DefaultHurtboxesOn(0);
			break;
		}
		double f = dashBez.GetValue(a);
		double rf = 1.0 - f;

		currPosInfo.position = data.currOrig * rf + dest * f;


		int steps = (5 / slowMultiple) * NUM_MAX_STEPS / numPhysSteps;

		data.physStepIndex += steps;
		break;
	}
	case S_RETURN:
	{
		V2d d = data.currOrig + data.playerDir * ( dashRadius / 2.0 );
		double a = (double)data.physStepIndex / (returnFrames * NUM_MAX_STEPS * 5);
		if (a > 1.0)
		{
			action = S_CHARGE;
			data.playerDir = normalize(playerPos - GetPosition());
			SetFacingPlayerAngle();
			data.chargeFrames = maxCharge - 5;
			frame = 0;
			data.currOrig = GetPosition();
			break;
		}
		double f = returnBez.GetValue(a);
		double rf = 1.0 - f;

		currPosInfo.position = dest * rf + d * f;
		

		int steps = (5 / slowMultiple) * NUM_MAX_STEPS / numPhysSteps;

		data.physStepIndex += steps;
		break;
	}
		
	}
}

void Airdasher::FrameIncrement()
{
	/*if (action == CHARGEDFLAP)
	{
	++fireCounter;
	}*/
}

void Airdasher::UpdateSprite()
{
	int tIndex = 0;

	
	switch (action)
	{
	case S_FLOAT:
		tIndex = frame / animFactor[S_FLOAT];
		break;
	case S_CHARGE:
		//sprite.setRotation(angle);
		tIndex = 11;
		break;
	case S_DASH:
		tIndex = 12;
		break;
	case S_OUT:
		tIndex = 11;
		break;
	case S_RETURN:
		//tIndex = 0;
		tIndex = (frame / animFactor[S_FLOAT]) % actionLength[S_FLOAT];
		break;
	}

	IntRect ir = ts->GetSubRect(tIndex);
	if (!facingRight)
	{
		ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
	}
	sprite.setTextureRect(ir);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

int Airdasher::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Airdasher::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Airdasher::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}