#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_AppearingShape.h"
#include "Actor.h"

using namespace std;
using namespace sf;

AppearingShapePool::AppearingShapePool()
{
	Session *sess = Session::GetSession();

	ts = NULL;
	numShapes = 100;//5;//10;
	shapeVec.resize(numShapes);
	ts = sess->GetSizedTileset("Bosses/Coyote/coyotebullet_32x32.png");
	for (int i = 0; i < numShapes; ++i)
	{
		shapeVec[i] = new AppearingShape(this);
	}
}


AppearingShapePool::~AppearingShapePool()
{
	for (int i = 0; i < numShapes; ++i)
	{
		delete shapeVec[i];
	}

	//delete[] verts;
}

void AppearingShapePool::Reset()
{
	for (int i = 0; i < numShapes; ++i)
	{
		shapeVec[i]->Reset();
	}
}

AppearingShape * AppearingShapePool::Appear(int shapeType, double size, V2d &pos,
	int appearFrames, int hitFrames, int disappearFrames )
{
	AppearingShape *bs = NULL;
	for (int i = 0; i < numShapes; ++i)
	{
		bs = shapeVec[i];
		if (!bs->active)
		{
			bs->Appear(shapeType, size, pos, appearFrames, hitFrames, disappearFrames );
			break;
		}
	}
	return bs;
}


void AppearingShapePool::DrawMinimap(sf::RenderTarget * target)
{
	for (auto it = shapeVec.begin(); it != shapeVec.end(); ++it)
	{
		if ((*it)->active)
		{
			(*it)->DrawMinimap(target);
		}
	}
}

AppearingShape::AppearingShape( AppearingShapePool *sp )
	:Enemy(EnemyType::EN_APPEARINGSHAPE, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 4;
	animFactor[NEUTRAL] = 5;

	actionLength[APPEAR] = 60;
	animFactor[APPEAR] = 1;

	actionLength[HIT] = 10;
	animFactor[HIT] = 1;

	actionLength[DISAPPEAR] = 60;
	animFactor[DISAPPEAR] = 1;

	ts = GetSizedTileset("Enemies/W7/hand_256x256.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	/*cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(21);
	cutObject->SetSubRectBack(20);
	cutObject->SetScale(scale);*/

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	numCirclePoints = 20;
	numVerts = numCirclePoints + 2;
	assert(numCirclePoints >= 3);
	verts = new Vertex[numVerts];

	BasicCircleHitBodySetup(16);
	//BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	Reset();
	//ResetEnemy();
}

AppearingShape::~AppearingShape()
{
	delete[]verts;
}

void AppearingShape::ResetEnemy()
{
	/*if (GetPosition().x < sess->playerOrigPos[0].x)
	facingRight = false;
	else
	facingRight = true;*/

	action = NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();
	//DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void AppearingShape::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case APPEAR:
			action = HIT;
			frame = 0;
			DefaultHitboxesOn();
			break;
		case HIT:
			action = DISAPPEAR;
			frame = 0;
			HitboxesOff();
			break;
		case DISAPPEAR:
			sess->RemoveEnemy(this);
			dead = true;
			numHealth = 0;
			break;
		}
	}
}

void AppearingShape::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();
}

void AppearingShape::UpdateSprite()
{
	/*int trueFrame = 0;

	ts->SetSubRect(sprite, trueFrame, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	sprite.setColor(Color::White);
	switch (action)
	{
	case APPEAR:
		sprite.setColor(Color::Red);
		break;
	}*/

	float factor = (float)frame / (actionLength[action] * animFactor[action]);
	switch (action)
	{
	case APPEAR:
	{
		SetColor(Color(255, 255, 255, factor * 100.f * factor));
		break;
	}
	case HIT:
	{
		SetColor(Color(255, 0, 0, 255));
		break;
	}
	case DISAPPEAR:
	{
		SetColor(Color(0, 255, 0, (1.f - factor) * 255));
		break;
	}
	}

	UpdateVertices();
}

void AppearingShape::EnemyDraw(sf::RenderTarget *target)
{
	switch (shapeType)
	{
	case SHAPE_CIRCLE:
	{
		target->draw(verts, numCirclePoints + 2, sf::TriangleFan);
		break;
	}
	case SHAPE_SQUARE:
	{
		break;
	}
	}
	
	//DrawSprite(target, sprite);
}

void AppearingShape::Appear( int p_shapeType, double p_size, V2d &pos,
	int appearFrames, int hitFrames, int disappearFrames )
{
	if (!active)
	{
		Reset();
		sess->AddEnemy(this);

		action = APPEAR;
		frame = 0;

		actionLength[APPEAR] = appearFrames;
		actionLength[HIT] = hitFrames;
		actionLength[DISAPPEAR] = disappearFrames;


		currPosInfo.position = pos;
		currPosInfo.ground = NULL;

		size = p_size;

		shapeType = p_shapeType;

		switch (shapeType)
		{
		case SHAPE_CIRCLE:
		{
			break;
		}
		}

		hitBody.ResetFrames();

		hitBody.AddBasicCircle(0, size, 0, V2d());

		HitboxesOff();

		UpdateHitboxes();
	}
}

void AppearingShape::UpdateVertices()
{
	if (shapeType == SHAPE_CIRCLE)
	{
		Vector2f currPos = GetPositionF();
		verts[0].position = currPos;
		Vector2f offset(size, 0);
		Vector2f tempOffset;
		Transform t;

		for (int i = 1; i < numVerts; ++i)
		{
			tempOffset = t.transformPoint(offset);
			verts[i].position = currPos + tempOffset;
			t.rotate(360.f / numCirclePoints);
		}
	}
}

void AppearingShape::SetColor(Color c)
{
	for (int i = 0; i < numVerts; ++i)
	{
		verts[i].color = c;
	}
}

int AppearingShape::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void AppearingShape::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void AppearingShape::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}