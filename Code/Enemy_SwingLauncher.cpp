#include "Enemy.h"
#include "Enemy_SwingLauncher.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "MainMenu.h"

using namespace std;
using namespace sf;


void SwingLauncher::UpdateParamsSettings()
{
	SwingLauncherParams *sParams = (SwingLauncherParams*)editParams;
	speed = sParams->speed;
	clockwiseLaunch = sParams->clockwise;
	stunFrames = 600;//ceil(swingRadius / speed);

	V2d along = normalize(GetPosition() - anchor);
	dir = V2d(along.y, -along.x);
	if (clockwiseLaunch)
		dir = -dir;

	UpdateSprite();
}

void SwingLauncher::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	UpdatePath(); //assuming that ap is editparams here
}

void SwingLauncher::UpdatePath()
{
	Vector2i other = Vector2i(0, -10);
	if (editParams->localPath.size() > 0)
		other = editParams->GetLocalPathPos(0);

	V2d dOther = V2d(other.y, -other.x); //perpendicular because swinglauncher
	V2d springVec = normalize(dOther);

	double angle = atan2(springVec.x, -springVec.y);
	sprite.setRotation(angle / PI * 180.0);

	swingRadius = length(V2d(other));

	/*dir = springVec;
	if( !clockwiseLaunch)
		dir = -dir;*/

	anchor = V2d(other) + GetPosition();

	V2d dirToAnchor = normalize(GetPosition() - anchor);
	origAngle = GetVectorAngleCW(dirToAnchor);
	data.currAngle = origAngle;

	UpdateParamsSettings();


	debugLine[0].color = Color::Red;
	debugLine[1].color = Color::Red;
	debugLine[0].position = GetPositionF();
	debugLine[1].position = Vector2f(anchor);

	stringstream ss;
	ss << speed;
	debugSpeed.setString(ss.str());
	debugSpeed.setOrigin(debugSpeed.getLocalBounds().width / 2, debugSpeed.getLocalBounds().height / 2);
	debugSpeed.setPosition(GetPositionF());
}

void SwingLauncher::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

SwingLauncher::SwingLauncher(ActorParams *ap)
	:Enemy(EnemyType::EN_SWINGLAUNCHER, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	launchSoundBuf = GetSound("Enemies/spring_launch");

	debugSpeed.setFont(sess->mainMenu->arial);
	debugSpeed.setFillColor(Color::White);
	debugSpeed.setCharacterSize(30);

	ts = GetSizedTileset("Enemies/W6/swinglauncher_128x128.png");

	if (!armShader.loadFromFile("Resources/Shader/swinglauncherarm_shader.frag", sf::Shader::Fragment))
	{
		assert(0);
	}

	armShader.setUniform("u_texture", *ts->texture);//sf::Shader::CurrentTexture);
	armShader.setUniform("u_quant", 0.f);

	auto ir = ts->GetSubRect(0);


	float width = ts->texture->getSize().x;
	float height = ts->texture->getSize().y;

	armShader.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
		(ir.left + ir.width) / width, (ir.top + ir.height) / height));

	
	/*SetRectSubRect(quads + startIndex * 4, sf::FloatRect(0, 0, 1.0, (wireLength / ir.height) / 2.0));*/

	animationFactor = 10;

	BasicCircleHitBodySetup(64);

	recoverBaseSpeed = .01 * PI;
	recoverAccel = .001 * PI;
	recoverMaxSpeed = .08 * PI;

	actionLength[IDLE] = 12;
	actionLength[SWINGING] = 8;
	actionLength[RECOVERING] = 8;

	animFactor[IDLE] = 4;
	animFactor[SWINGING] = 4;
	animFactor[RECOVERING] = 4;

	SetRectSubRect(quads, ts->GetSubRect(0));
	

	editParams = ap;
	UpdatePath();

	ResetEnemy();
}
void SwingLauncher::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	target->draw(debugLine, 2, sf::Lines);
	target->draw(debugSpeed);
}

sf::FloatRect SwingLauncher::GetAABB()
{
	double halfSprite = 128 / 2;
	V2d pos = GetPosition();
	double left = min(anchor.x, pos.x);
	double top = min(anchor.y, pos.y);
	double right = max(anchor.x, pos.x);
	double bot = max(anchor.y, pos.y);

	return FloatRect(left - halfSprite, top - halfSprite, (right - left) + halfSprite * 2, (bot - top) + halfSprite * 2);
}

void SwingLauncher::DirectKill()
{
}

void SwingLauncher::ResetEnemy()
{
	action = IDLE;
	frame = 0;

	data.recoverAngDist = 0;

	
	data.currAngle = origAngle;
	
	DefaultHitboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void SwingLauncher::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case SWINGING:
			//action = RECOVERING;
			break;
		case RECOVERING:
			//action = IDLE;
			break;
		}
	}
}

void SwingLauncher::Launch()
{
	assert(action == IDLE);
	action = SWINGING;
	frame = 0;
	sess->ActivateSound(launchSoundBuf);
}

void SwingLauncher::Recover()
{
	if (action == SWINGING)
	{
		action = RECOVERING;
		frame = 0;

		V2d test(1, 0);
		RotateCW(test, data.currAngle);

		V2d orig = normalize(GetPosition() - anchor);

		if (clockwiseLaunch)
		{
			data.recoverAngDist = GetVectorAngleDiffCCW(test, orig);
		}
		else
		{
			data.recoverAngDist = GetVectorAngleDiffCW(test, orig);
		}



		data.recoverSpeed = recoverBaseSpeed;
	}
}

void SwingLauncher::ProcessState()
{
	ActionEnded();

	if (action == RECOVERING)
	{
		//double rotSpeed = .1 * PI;
		
		if (data.recoverAngDist < data.recoverSpeed)
		{
			data.recoverAngDist = 0;
			data.currAngle = origAngle;
			action = IDLE;
			frame = 0;
		}
		else
		{
			if (clockwiseLaunch)
			{
				data.currAngle -= data.recoverSpeed;
			}
			else
			{
				data.currAngle += data.recoverSpeed;
			}
			
			data.recoverAngDist -= data.recoverSpeed;
		}

		data.recoverSpeed += recoverAccel;
		if (data.recoverSpeed > recoverMaxSpeed)
		{
			data.recoverSpeed = recoverMaxSpeed;
		}
	}
}


void SwingLauncher::UpdateSprite()
{
	SetRectCenter(quads, ts->tileWidth, ts->tileHeight, Vector2f(anchor));

	V2d offset(swingRadius, 0);
	RotateCW(offset, data.currAngle);

	ts->SetQuadSubRect(quads + 4, 2, clockwiseLaunch);

	SetRectRotation(quads + 4, data.currAngle - PI / 2, ts->tileWidth, ts->tileHeight, Vector2f(anchor + offset));

	//SetRectColor(armQuad, Color::White);
	//SetRectRotation(armQuad, data.currAngle, swingRadius, 128, Vector2f(anchor + offset / 2.0));
	SetRectRotation(armQuad, data.currAngle - PI/2, 128, swingRadius, Vector2f(anchor + offset / 2.0));


	auto ir = ts->GetSubRect(1);
	Vector2f texSize(ts->texture->getSize());


	float left = ir.left / texSize.x;
	float top = ir.top / texSize.y;
	float right = (ir.left + ir.width) / texSize.x;
	float bot = swingRadius / texSize.y;;//(ir.top + ir.height) / texSize.y;
	armQuad[0].texCoords = Vector2f(left, top);
	armQuad[1].texCoords = Vector2f(right, top);
	armQuad[2].texCoords = Vector2f(right, bot);
	armQuad[3].texCoords = Vector2f(left, bot);

	//SetRectSubRectGL(armQuad, ir, Vector2f(ts->texture->getSize()));

	//SetRectSubRect(armQuad, sf::FloatRect(0, 0, 1.0 / 3.0, 1.0));//sf::FloatRect(0, 0, 1.0, (swingRadius / ir.height) / 2.0));
}

void SwingLauncher::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(armQuad, 4, sf::Quads, &armShader);
	target->draw(quads, 3 * 4, sf::Quads, ts->texture );


	/*sf::CircleShape testCircle;
	testCircle.setFillColor(Color(255, 0, 0, 100));
	testCircle.setRadius(swingRadius);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2, testCircle.getLocalBounds().height / 2);
	testCircle.setPosition(Vector2f(anchor));

	target->draw(testCircle);*/
}

int SwingLauncher::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void SwingLauncher::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void SwingLauncher::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}