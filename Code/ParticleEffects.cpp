#include "ParticleEffects.h"
#include "Tileset.h"
#include "MovingGeo.h"
#include "GameSession.h"


using namespace std;
using namespace sf;

FadingParticle::FadingParticle(int numPoints,
	sf::Vertex *v,
	ShapeEmitter *emit)
	:ShapeParticle(numPoints, v, emit)
{
	fadeOutThresh = 40;
	fadeInThresh = 40;
}

void FadingParticle::SpecialActivate()
{
	action = FADEIN;
	if (ttl < fadeOutThresh)
	{
		action = FADEOUT;
	}

	startAlpha = color.a;
	SetColor(sf::Color(color.r, color.g, color.b, 0));

	maxTimeToLive = ttl;
}

void FadingParticle::SpecialUpdate()
{
	switch (action)
	{
	case FADEIN:
		if (ttl <= maxTimeToLive - fadeInThresh)
		{
			action = NORMAL;
		}
	case NORMAL:
		if (ttl <= fadeOutThresh)
		{
			action = FADEOUT;
			startAlpha = color.a;
		}
		break;
	case FADEOUT:
		break;
	}

	switch (action)
	{
	case FADEIN:
	{
		float fttl = maxTimeToLive - ttl;
		color.a = startAlpha * fttl / fadeInThresh;
		SetColor(color);
		break;
	}
	case NORMAL:
		break;
	case FADEOUT:
	{
		float fttl = ttl;
		color.a = startAlpha * fttl / fadeOutThresh;
		SetColor(color);
		break;
	}
	}
}

void FadingParticle::SetColorShift(sf::Color &start,
	sf::Color &end, int p_fadeInFrames, int p_fadeOutFrames)
{
	fadeInThresh = p_fadeInFrames;
	fadeOutThresh = p_fadeOutFrames;
	startColor = start;
	endColor = end;
	SetColor(startColor);
}

void FadingParticle::UpdateColor()
{
	if (action == NORMAL)
	{
		float portion = GetNormalPortion();
		SetColor(GetBlendColor(startColor, endColor, 1.f - portion));
	}
}

float FadingParticle::GetNormalPortion()
{
	int nStart = (maxTimeToLive - fadeInThresh);
	int nFrame = nStart - ttl;
	int nLength = nStart - fadeOutThresh;

	return (float)nFrame / nLength;
}


ShapeParticle::ShapeParticle(int p_numPoints, sf::Vertex *v,
	ShapeEmitter *p_emit)
	:numPoints(p_numPoints), points(v), emit(p_emit)
{
	sizeUpdater = NULL;
	colorUpdater = NULL;
	angleUpdater = NULL;
	posUpdater = NULL;
}

ShapeParticle::~ShapeParticle()
{
	if (sizeUpdater != NULL)
	{
		delete sizeUpdater;
	}

	if (colorUpdater != NULL)
	{
		delete colorUpdater;
	}

	if (angleUpdater != NULL)
	{
		delete angleUpdater;
	}

	if (posUpdater != NULL)
	{
		delete posUpdater;
	}
}



void ShapeParticle::Activate(float p_radius, sf::Vector2f &p_pos,
	float p_angle, int p_ttl, sf::Color c, int tIndex )
{
	pos = p_pos;
	ttl = p_ttl;
	radius = p_radius;
	angle = p_angle;
	SetColor(c);
	
	if (emit->ts != NULL)
	{
		SetTileIndex(tIndex);
	}

	//SpecialActivate();

	//if (emit->handler != NULL)
	//{
	//	emit->handler->ActivateShapeParticle(this);
	//}

	sf::Transform tr;

	//int extraAngle = rand() % 360;
	tr.rotate(angle);

	sf::Vector2f dir(0, -1);


	if (numPoints == 3)
	{
		points[0].position = pos + tr.transformPoint(dir * radius);
		tr.rotate(360.f / numPoints);
		points[1].position = pos + tr.transformPoint(dir * radius);
		tr.rotate(360.f / numPoints);
		points[2].position = pos + tr.transformPoint(dir * radius);
	}
	else if (numPoints == 4)
	{
		tr.rotate(-45);
		points[0].position = pos + tr.transformPoint(dir * radius);
		tr.rotate(90);
		points[1].position = pos + tr.transformPoint(dir * radius);
		tr.rotate(90);
		points[2].position = pos + tr.transformPoint(dir * radius);
		tr.rotate(90);
		points[3].position = pos + tr.transformPoint(dir * radius);
	}
	else if (numPoints > 4)
	{
		int numOuterPoints = numPoints / 3;
		for (int i = 0; i < numOuterPoints; ++i)
		{
			points[i * 3].position = pos;
			points[i * 3 + 1].position = pos + tr.transformPoint(dir * radius);
			tr.rotate(360.f / numOuterPoints);
			points[i * 3 + 2].position = pos + tr.transformPoint(dir * radius);
		}
	}
	else
	{
		assert(0);
	}
}

void ShapeParticle::SetColor(sf::Color &c)
{
	color = c;
	for (int i = 0; i < numPoints; ++i)
	{
		points[i].color = c;
	}
}

void ShapeParticle::SetTileIndex(int ti)
{
	assert(numPoints == 4);

	if (emit->ts == NULL)
		return;

	tileIndex = ti;
	SetRectSubRect(points, emit->ts->GetSubRect(ti));
}

bool ShapeParticle::Update()
{
	if (ttl < 0)
	{
		return false;
	}

	Vector2f oldPos = pos;

	/*if (emit->handler != NULL)
	{
	emit->handler->UpdateShapeParticle(this);
	}*/

	
	if (posUpdater != NULL)
	{
		posUpdater->PUpdatePos(this);
	}
	if (sizeUpdater != NULL)
	{
		sizeUpdater->PUpdateSize(this);
	}
	if (colorUpdater != NULL )
	{
		colorUpdater->PUpdateColor(this);
	}

	if (angleUpdater != NULL)
	{
		angleUpdater->PUpdateAngle(this);
	}

	//UpdateColor();

	//SpecialUpdate();

	//pos += vel;
	//vel += emit->accel;

	for (int i = 0; i < numPoints; ++i)
	{
		points[i].position += (pos - oldPos);
	}
	//UpdatePoints();

	

	--ttl;

	if (ttl < 0)
	{

		Clear();
		return false;
	}

	return true;
}

void ShapeParticle::Clear()
{
	for (int i = 0; i < numPoints; ++i)
	{
		points[i].position = Vector2f(0, 0);
	}
	ttl = -1;
}

ShapeEmitter::ShapeEmitter(int p_pointsPerShape, int p_numShapes)
	:pointsPerShape(p_pointsPerShape), numShapesTotal(p_numShapes)
{
	if (pointsPerShape > 4)
	{
		pointsPerShape *= 3;
	}
	numPoints = numShapesTotal * pointsPerShape;
	points = new Vertex[numPoints];
	particles = new ShapeParticle*[numShapesTotal];

	//pType = FADE;
	

	//Color r = Color::White;
	//SetColor(r);
	ratePerSecond = 10; //just for testing defaults

	next = NULL;
	ts = NULL;

	posSpawner = NULL;
	colorSpawner = NULL;
	radiusSpawner = NULL;
	angleSpawner = NULL;
	ttlSpawner = NULL;
}

void ShapeEmitter::CreateParticles()
{
	for (int i = 0; i < numShapesTotal; ++i)
	{
		particles[i] = CreateParticle(i);
	}
}

ShapeParticle * ShapeEmitter::CreateParticle(int index)
{
	return new ShapeParticle(pointsPerShape, points + index * pointsPerShape, this);
}

ShapeEmitter::~ShapeEmitter()
{
	delete[] points;
	for (int i = 0; i < numShapesTotal; ++i)
	{
		delete particles[i];
	}
	delete[] particles;

	if (posSpawner != NULL)
	{
		delete posSpawner;
	}
	if (colorSpawner != NULL)
	{
		delete colorSpawner;
	}
	if (radiusSpawner != NULL)
	{
		delete radiusSpawner;
	}
	if (angleSpawner != NULL)
	{
		delete angleSpawner;
	}
	if (ttlSpawner != NULL)
	{
		delete ttlSpawner;
	}
}

void ShapeEmitter::SetPos(sf::Vector2f &p_pos)
{
	pos = p_pos;
}

void ShapeEmitter::SetTileset(Tileset *p_ts)
{
	assert(pointsPerShape == 4);
	ts = p_ts;
}

void ShapeEmitter::Reset()
{
	emitting = true;
	for (int i = 0; i < numShapesTotal; ++i)
	{
		particles[i]->Clear();
	}
	frame = 0;
	lastCreationTime = 0;
	numActive = 0;
}

void ShapeEmitter::ActivateParticle(int index)
{
	particles[index]->Activate(GetSpawnRadius(), GetSpawnPos(), GetSpawnAngle(), GetSpawnTTL(), GetSpawnColor(),
		GetSpawnTile());

	/*float a = angle;
	float f = (float)rand() / RAND_MAX * 2.0 - 1.0;
	a += angleRange * f;

	float s = minSpeed;
	float f1 = (float)rand() / RAND_MAX;
	s += (maxSpeed - minSpeed) * f1;*/

	/*Vector2f vel(1, 0);
	vel *= s;
	RotateCCW(vel, a);*/

	//int rad = 2 + rand() % 15;
	//int rad = 20 + rand() % 30;

	//int extraAngle = rand() % 360;

	//sf::Color randColor(rand() % 255, rand() % 255, rand() % 255, 255); //100 + rand() % 155);
	//sf::Color randColor1(rand() % 255, rand() % 255, rand() % 255, 255); //100 + rand() % 155);


	//switch (pType)
	//{
	//case NORMAL:
	//{
	//	sf::Color randColor(rand() % 255, rand() % 255, rand() % 255, 255); //100 + rand() % 155);
	//	particles[index]->SetColor(randColor);
	//	break;
	//}
	//case FADE:
	//{
	//	FadingParticle *fp = (FadingParticle*)particles[index];
	//	sf::Color leafColor(255, 255, 255, 150);
	//	fp->SetColorShift(leafColor, leafColor, 40, 40);
	//	particles[index]->SetTileIndex(rand() % 5);
	//	//fp->SetColorShift(Color( Color::Blue ), Color(Color::Cyan), 0, 40);

	//	//particles[index]->SetColor(randColor);
	//	break;
	//}
	//}

	//particles[index]->//->Activate( GetSpawnRadius(), GetSpawnPos(),, extraAngle, 200 );

	//cout << "activating: " << index << endl;
}

sf::Vector2f ShapeEmitter::GetSpawnPos()
{
	if (posSpawner != NULL)
	{
		return posSpawner->GetSpawnPos(this);
	}
	else
	{
		return pos;
	}
}

sf::Vector2f ShapeEmitter::GetSpawnVel()
{
	return Vector2f(0, 0);
}

float ShapeEmitter::GetSpawnAngle()
{
	if (angleSpawner != NULL)
	{
		return angleSpawner->GetSpawnAngle(this);
	}
	else
	{
		return 0;
	}
}

int ShapeEmitter::GetSpawnTile() 
{ 
	return 0; 
}

int ShapeEmitter::GetSpawnTTL()
{
	if (ttlSpawner != NULL)
	{
		return ttlSpawner->GetSpawnTTL(this);
	}
	else
	{
		return 120;
	}
}

float ShapeEmitter::GetSpawnRadius()
{
	if (radiusSpawner != NULL)
	{
		radiusSpawner->GetSpawnRadius(this);
	}
	else
	{
		return 20.f;
	}
}

sf::Color ShapeEmitter::GetSpawnColor()
{
	if (colorSpawner != NULL)
	{
		return colorSpawner->GetSpawnColor(this);
	}
	else
	{
		return Color::White;
	}
}

void ShapeEmitter::SetRatePerSecond(int rate)
{
	ratePerSecond = rate;
}

bool ShapeEmitter::IsDone()
{
	return numActive == 0 && !emitting;
}

void ShapeEmitter::Update()
{
	int numActivateThisFrame = 0;

	float rf = 1.f / ratePerSecond;
	float cTime = frame / 60.f;
	float diff = cTime - lastCreationTime;
	float num = diff / rf;

	numActivateThisFrame = floor(num);
	if (numActivateThisFrame > 0)
	{
		lastCreationTime = cTime;
	}

	numActive = 0;
	for (int i = 0; i < numShapesTotal; ++i)
	{
		if (particles[i]->ttl < 0 && numActivateThisFrame > 0 && emitting)
		{
			ActivateParticle(i);
			numActivateThisFrame--;
		}
		else
		{
			bool on = particles[i]->Update();
			if (on)
				numActive++;
		}
	}

	++frame;
}

void ShapeEmitter::SetOn(bool on)
{
	emitting = on;
}

void ShapeEmitter::Draw(sf::RenderTarget *target)
{
	if (pointsPerShape == 4)
	{
		if (ts != NULL)
		{
			target->draw(points, numPoints, sf::Quads, ts->texture);
		}
		else
		{
			target->draw(points, numPoints, sf::Quads);
		}

	}
	else
	{
		target->draw(points, numPoints, sf::Triangles);
	}
}

BoxPosSpawner::BoxPosSpawner(int w, int h)
	:width( w ), height( h )
{

}

void BoxPosSpawner::SetRect(int w, int h)
{
	width = w;
	height = h;
}

sf::Vector2f BoxPosSpawner::GetSpawnPos(ShapeEmitter *emit)
{
	int rw, rh;

	if (width == 0)
	{
		rw = 0;
	}
	else
	{
		rw = (rand() % width) - width / 2;
	}

	if (height == 0)
	{
		rh = 0;
	}
	else
	{
		rh = (rand() % height) - height / 2;
	}

	return emit->pos + Vector2f(rw, rh);
}

LinearVelPPosUpdater::LinearVelPPosUpdater()
{

}

void LinearVelPPosUpdater::PUpdatePos(ShapeParticle* p)
{
	p->pos += vel;
}

LeafEmitter::LeafEmitter()
	:ShapeEmitter( 4, 500)
{
	posSpawner = new BoxPosSpawner(400, 400);
	SetRatePerSecond(120);
}

ShapeParticle *LeafEmitter::CreateParticle(int index)
{
	ShapeParticle *sp = new ShapeParticle(pointsPerShape, points + index * pointsPerShape, this);
	sp->posUpdater = new LinearVelPPosUpdater;
	return sp;
}

void LeafEmitter::ActivateParticle(int index)
{
	ShapeParticle *sp = particles[index];
	Vector2f sPos = GetSpawnPos();
	sp->Activate(GetSpawnRadius(), sPos, GetSpawnAngle(), GetSpawnTTL(), GetSpawnColor(), GetSpawnTile());
	LinearVelPPosUpdater * velUpdater = (LinearVelPPosUpdater*)sp->posUpdater;
	velUpdater->vel = normalize(sPos - pos) * 10.f;
}

int LeafEmitter::GetSpawnTTL()
{
	int variation = 40;
	int r = (rand() % variation) - variation / 2;

	return 90 + r;
}

int LeafEmitter::GetSpawnTile()
{
	return rand() % 5;
}
