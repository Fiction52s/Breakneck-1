#include "ParticleEffects.h"
#include "Tileset.h"
#include "MovingGeo.h"
#include "GameSession.h"
#include "Actor.h"
#include "MapHeader.h"


using namespace std;
using namespace sf;


ShapeParticle::ShapeParticle(int p_numPoints, sf::Vertex *v,
	ShapeEmitter *p_emit)
	:numPoints(p_numPoints), points(v), emit(p_emit)
{
	data.fadeOutThresh = 0;
	data.fadeInThresh = 0;
	data.startColor = Color::White;
	data.endColor = Color::White;
}

ShapeParticle::~ShapeParticle()
{
}

void ShapeParticle::Activate(float p_radius, sf::Vector2f &p_pos,
	float p_angle, int p_ttl, sf::Color c, int tIndex )
{
	data.pos = p_pos;
	data.ttl = p_ttl;
	data.radius = p_radius;
	data.angle = p_angle;
	SetColor(c);
	
	if (emit->ts != NULL)
	{
		SetTileIndex(tIndex);
	}

	if (data.fadeInThresh == 0)
	{
		data.action = NORMAL;
	}
	else
	{
		data.action = FADEIN;
	}

	if (data.ttl < data.fadeOutThresh)
	{
		data.action = FADEOUT;
	}

	data.startAlpha = data.color.a;
	SetColor(sf::Color(data.color.r, data.color.g, data.color.b, 0));

	data.maxTimeToLive = data.ttl;

	//SpecialActivate();

	//if (emit->handler != NULL)
	//{
	//	emit->handler->ActivateShapeParticle(this);
	//}

	sf::Transform tr;

	//int extraAngle = rand() % 360;
	tr.rotate(data.angle);

	sf::Vector2f dir(0, -1);


	if (numPoints == 3)
	{
		points[0].position = data.pos + tr.transformPoint(dir * data.radius);
		tr.rotate(360.f / numPoints);
		points[1].position = data.pos + tr.transformPoint(dir * data.radius);
		tr.rotate(360.f / numPoints);
		points[2].position = data.pos + tr.transformPoint(dir * data.radius);
	}
	else if (numPoints == 4)
	{
		tr.rotate(-45);
		points[0].position = data.pos + tr.transformPoint(dir * data.radius);
		tr.rotate(90);
		points[1].position = data.pos + tr.transformPoint(dir * data.radius);
		tr.rotate(90);
		points[2].position = data.pos + tr.transformPoint(dir * data.radius);
		tr.rotate(90);
		points[3].position = data.pos + tr.transformPoint(dir * data.radius);
	}
	else if (numPoints > 4)
	{
		int numOuterPoints = numPoints / 3;
		for (int i = 0; i < numOuterPoints; ++i)
		{
			points[i * 3].position = data.pos;
			points[i * 3 + 1].position = data.pos + tr.transformPoint(dir * data.radius);
			tr.rotate(360.f / numOuterPoints);
			points[i * 3 + 2].position = data.pos + tr.transformPoint(dir * data.radius);
		}
	}
	else
	{
		assert(0);
	}
}

void ShapeParticle::SetColor(sf::Color &c)
{
	data.color = c;
	//startColor = color;
	//endColor = color;
	for (int i = 0; i < numPoints; ++i)
	{
		points[i].color = c;
	}
}

void ShapeParticle::SetColorShift(sf::Color start,
	sf::Color end, int p_fadeInFrames, int p_fadeOutFrames)
{
	data.fadeInThresh = p_fadeInFrames;
	data.fadeOutThresh = p_fadeOutFrames;
	data.startColor = start;
	data.endColor = end;
	SetColor(data.startColor);
}

float ShapeParticle::GetNormalPortion()
{
	int nStart = (data.maxTimeToLive - data.fadeInThresh);
	int nFrame = nStart - data.ttl;
	int nLength = nStart - data.fadeOutThresh;

	return (float)nFrame / nLength;
}

void ShapeParticle::SetTileIndex(int ti)
{
	assert(numPoints == 4);

	if (emit->ts == NULL)
		return;

	data.tileIndex = ti;
	SetRectSubRect(points, emit->ts->GetSubRect(ti));
}

bool ShapeParticle::Update()
{
	if (data.ttl < 0)
	{
		return false;
	}

	Vector2f oldPos = data.pos;

	/*if (emit->handler != NULL)
	{
	emit->handler->UpdateShapeParticle(this);
	}*/

	data.pos += data.vel;

	//update pos
	//update size
	//update color
	//update angle
	
	

	switch (data.action)
	{
	case FADEIN:
		if (data.ttl <= data.maxTimeToLive - data.fadeInThresh)
		{
			data.action = NORMAL;
		}
	case NORMAL:
		if (data.ttl <= data.fadeOutThresh)
		{
			data.action = FADEOUT;
			data.startAlpha = data.color.a;
		}
		break;
	case FADEOUT:
		break;
	}


	float portion = 1.f - (data.ttl / (float)data.maxTimeToLive);
	Color currColor = GetBlendColor(data.startColor, data.endColor, portion);
	SetColor(currColor);

	switch (data.action)
	{
	case FADEIN:
	{
		float fttl = data.maxTimeToLive - data.ttl;
		currColor.a = data.startAlpha * fttl / data.fadeInThresh;
		SetColor(currColor);
		break;
	}
	case NORMAL:
	{
		/*float portion = 0.f;
		portion = 1.f - (ttl / (float)maxTimeToLive);
		
		SetColor(GetBlendColor(startColor, endColor, portion));*/
		break;
	}
	case FADEOUT:
	{
		float fttl = data.ttl;
		currColor.a = data.startAlpha * fttl / data.fadeOutThresh;
		SetColor(currColor);
		break;
	}
	}


	//SpecialUpdate();

	//pos += vel;
	//vel += emit->accel;

	for (int i = 0; i < numPoints; ++i)
	{
		points[i].position += (data.pos - oldPos);
	}
	//UpdatePoints();

	

	--data.ttl;

	if (data.ttl < 0)
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
	data.ttl = -1;
}

int ShapeParticle::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void ShapeParticle::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &data, sizeof(data));
	bytes += sizeof(data);
}

void ShapeParticle::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(data));
	bytes += sizeof(data);
}


float ShapeEmitter::GetRandomAngle(float baseAngle,
	float angleRange)
{
	//could be a desync issue if this is ever online
	float a = baseAngle;
	float f = (float)sess->GetRand() / RAND_MAX * 2.0 - 1.0;
	a += angleRange * f;

	return a;
}

ShapeEmitter::ShapeEmitter(int p_particleType)
{
	particleType = p_particleType;

	numShapesTotal = -1;

	switch (particleType)
	{
	case PARTICLE_BOOSTER_GRAVITY_INCREASER:
	case PARTICLE_BOOSTER_GRAVITY_DECREASER:
	case PARTICLE_BOOSTER_MOMENTUM:
	case PARTICLE_BOOSTER_TIMESLOW:
	case PARTICLE_BOOSTER_HOMING:
	case PARTICLE_BOOSTER_ANTITIMESLOW:
	case PARTICLE_BOOSTER_FREEFLIGHT:
		numShapesTotal = 200;
		break;
	default:
		numShapesTotal = 500;
		break;
	}

	Init();
}

ShapeEmitter::ShapeEmitter(int p_particleType, int p_maxParticles )
{
	particleType = p_particleType;

	numShapesTotal = p_maxParticles;
	

	Init();
}

void ShapeEmitter::Init()
{
	sess = Session::GetSession();

	pointsPerShape = -1;


	//can easily set this up to do triangles or other shapes
	switch (particleType)
	{
	case PARTICLE_BOOSTER_GRAVITY_INCREASER:
	case PARTICLE_BOOSTER_GRAVITY_DECREASER:
	case PARTICLE_BOOSTER_MOMENTUM:
	case PARTICLE_BOOSTER_TIMESLOW:
	case PARTICLE_BOOSTER_HOMING:
	case PARTICLE_BOOSTER_ANTITIMESLOW:
	case PARTICLE_BOOSTER_FREEFLIGHT:
		pointsPerShape = 4;
		break;
	default:
		pointsPerShape = 4;
		break;
	}

	assert(pointsPerShape >= 0);

	if (pointsPerShape > 4)
	{
		pointsPerShape *= 3;
	}
	numPoints = numShapesTotal * pointsPerShape;
	points = new Vertex[numPoints];
	particles = new ShapeParticle*[numShapesTotal];

	data.active = false;

	data.ratePerSecond = 10; //just for testing defaults

	data.prevID = -1; //initializing here prob does nothing, but its good practice anyway
	data.nextID = -1;

	prev = NULL;
	next = NULL;
	ts = NULL;

	for (int i = 0; i < numShapesTotal; ++i)
	{
		particles[i] = NULL;
	}
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
}

void ShapeEmitter::SetPos(sf::Vector2f &p_pos)
{
	data.pos = p_pos;
}

void ShapeEmitter::SetTileset(Tileset *p_ts)
{
	assert(pointsPerShape == 4);
	ts = p_ts;
}

void ShapeEmitter::Reset()
{
	data.active = false;
	data.emitting = true;
	if (particles[0] != NULL)
	{
		for (int i = 0; i < numShapesTotal; ++i)
		{
			particles[i]->Clear();
		}
	}
	data.frame = 0;
	data.lastCreationTime = 0;
	data.numActive = 0;
	prev = NULL;
	next = NULL;
}

void ShapeEmitter::ActivateParticle(int index)
{
	particles[index]->Activate(20, data.pos, 0, 120, Color::White, 0);
}

void ShapeEmitter::SetRatePerSecond(int rate)
{
	data.ratePerSecond = rate;
}

bool ShapeEmitter::IsDone()
{
	return data.numActive == 0 && !data.emitting;
}

void ShapeEmitter::SetIDAndAddToAllEmittersVec()
{
	emitterID = sess->allEmittersVec.size();
	sess->allEmittersVec.push_back(this);
}

void ShapeEmitter::Update()
{
	SpecialUpdate();


	int numActivateThisFrame = 0;

	float rf = 1.f / data.ratePerSecond;
	float cTime = data.frame / 60.f;
	float diff = cTime - data.lastCreationTime;
	float num = diff / rf;

	numActivateThisFrame = floor(num);
	if (numActivateThisFrame > 0)
	{
		data.lastCreationTime = cTime;
	}

	data.numActive = 0;
	for (int i = 0; i < numShapesTotal; ++i)
	{
		if (particles[i]->data.ttl < 0 && numActivateThisFrame > 0 && data.emitting)
		{
			ActivateParticle(i);
			numActivateThisFrame--;
		}
		else
		{
			bool on = particles[i]->Update();
			if (on)
				data.numActive++;
		}
	}

	++data.frame;
}

void ShapeEmitter::SetOn(bool on)
{
	data.emitting = on;
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



sf::Vector2f ShapeEmitter::GetBoxSpawnPos(int width, int height)
{
	int rw, rh;

	if (width == 0)
	{
		rw = 0;
	}
	else
	{
		
		//use rand() if you don't want this included in rollback
		//GetRand is bugged with decently large numbers, like 2000. find a fix for that soon.

		rw = (rand() % width) - width / 2;
		//rw = (sess->GetRand() % width) - width / 2;
	}

	if (height == 0)
	{
		rh = 0;
	}
	else
	{
		rh = (rand() % height) - height / 2;
		//rh = (sess->GetRand() % height) - height / 2;
	}

	return data.pos + Vector2f(rw, rh);
}

int ShapeEmitter::GetNumStoredBytes()
{
	//return sizeof(bool);

	int total = sizeof(data);
	for (int i = 0; i < numShapesTotal; ++i)
	{
		total += particles[i]->GetNumStoredBytes();
	}
	return total;
}

void ShapeEmitter::StoreBytes(unsigned char *bytes)
{
	//memcpy(bytes, &data, sizeof(bool));
	//return;

	data.prevID = sess->GetEmitterID(prev);
	data.nextID = sess->GetEmitterID(next);

	memcpy(bytes, &data, sizeof(data));

	bytes += sizeof(data);

	for (int i = 0; i < numShapesTotal; ++i)
	{
		particles[i]->StoreBytes(bytes);
		bytes += particles[i]->GetNumStoredBytes();
	}
	//StoreBasicEnemyData(data);
	//memcpy(bytes, &data, sizeof(MyData));
	//bytes += sizeof(MyData);
}

void ShapeEmitter::SetFromBytes(unsigned char *bytes)
{
	//memcpy(&data, bytes, sizeof(bool));
	//return;

	memcpy(&data, bytes, sizeof(data));
	prev = sess->GetEmitterFromID(data.prevID);
	next = sess->GetEmitterFromID(data.nextID);

	bytes += sizeof(data);

	for (int i = 0; i < numShapesTotal; ++i)
	{
		particles[i]->SetFromBytes(bytes);
		bytes += particles[i]->GetNumStoredBytes();
	}
}

LeafEmitter::LeafEmitter()
	:ShapeEmitter( 4, 500)
{
	//posSpawner = new BoxPosSpawner(400, 400);
	SetRatePerSecond(120);
}

void LeafEmitter::ActivateParticle(int index)
{
	ShapeParticle *sp = particles[index];
	/*Vector2f sPos = GetSpawnPos();
	sp->Activate(GetSpawnRadius(), sPos, GetSpawnAngle(), GetSpawnTTL(), GetSpawnColor(), GetSpawnTile());
	LinearVelPPosUpdater * velUpdater = (LinearVelPPosUpdater*)sp->posUpdater;

	velUpdater->vel = normalize(sPos - pos) * 10.f;*/
}

//int LeafEmitter::GetSpawnTTL()
//{
//	int variation = 40;
//	int r = (rand() % variation) - variation / 2;
//
//	return 90 + r;
//}
//
//int LeafEmitter::GetSpawnTile()
//{
//	return rand() % 5;
//}

PlayerBoosterEffectEmitter::PlayerBoosterEffectEmitter( Actor *p_player, int p_particleType )
	:ShapeEmitter(p_particleType)
{
	player = p_player;
	SetRatePerSecond(120);
}

void PlayerBoosterEffectEmitter::ActivateParticle(int index)
{
	ShapeParticle *sp = particles[index];
	Vector2f sPos = GetBoxSpawnPos(20, 20);

	int minRad = 10;
	int maxRad = 32;

	

	int angI = 0;//sess->GetRand() % 360;
	//int angI = rand() % 360;
	float ang = angI;

	
	Color aColor = Color::White;
	Color bColor = Color::White;//Color( 40, 0, 0 );

	
	Color sColor = GetBlendColor(aColor, bColor, data.boostPortion);


	int ttlVariation = 40;
	//int ttlValue = (rand() % ttlVariation) - ttlVariation / 2;
	int ttlValue = (sess->GetRand() % ttlVariation) - ttlVariation / 2;
	int finalTimeToLive = 90 + ttlValue;

	int tile = 0;

	switch (particleType)
	{
	case PARTICLE_BOOSTER_GRAVITY_INCREASER:
	{
		maxRad = 64;
		minRad = 20;
		tile = 0;
		ang = 0;

		int tileR = sess->GetRand() % 3;
		if (tileR == 0)
		{
			tile += 1;
			maxRad = 32;
			minRad = 20;
		}

		break;
	}
	case PARTICLE_BOOSTER_GRAVITY_DECREASER:
	{
		maxRad = 64;
		minRad = 20;
		tile = 5;
		ang = 0;

		int tileR = sess->GetRand() % 3;
		if (tileR == 0)
		{
			tile += 1;
			maxRad = 32;
			minRad = 20;
		}
		//int tileR = sess->GetRand() % 2;
		
		break;
	}
	default:
		tile = 0;
		ang = 0;
		break;
	}

	//int r = rand() % ((maxRad - minRad) + 1) + minRad;
	int r = sess->GetRand() % ((maxRad - minRad) + 1) + minRad;
	float rad = r;

	sp->Activate(rad, sPos, ang, finalTimeToLive, Color::White, tile);
	sp->data.vel = normalize(sPos - data.pos) * .1f;//10.f;
	//360

	Color sColorTransParent = sColor;
	sColorTransParent.a = 70;

	sp->SetColorShift(sColor, sColorTransParent, 20, 20);

}


ForegroundTestEmitter::ForegroundTestEmitter(int p_particleType)
	:ShapeEmitter(p_particleType, 2000)
{
	SetRatePerSecond(20);

	extraZoom = 1.0;

	depth = 1.3;//2.0;
	
}

void ForegroundTestEmitter::ActivateParticle(int index)
{
	ShapeParticle *sp = particles[index];

	Vector2f center(sess->mapHeader->leftBounds + sess->mapHeader->boundsWidth / 2.f, sess->mapHeader->topBounds + sess->mapHeader->boundsHeight / 2.f);
	data.pos = center;

	/*return (((p.x >= mapHeader->leftBounds)
		&& (p.y >= mapHeader->topBounds)
		&& (p.x <= mapHeader->leftBounds + mapHeader->boundsWidth)
		&& (p.y <= mapHeader->topBounds + mapHeader->boundsHeight)));*/

	//Vector2f sPos = GetBoxSpawnPos(20, 20);
	Vector2f sPos = GetBoxSpawnPos(sess->mapHeader->boundsWidth, sess->mapHeader->boundsHeight);

	int minRad = 5;
	int maxRad = 16;



	int angI = 0;//sess->GetRand() % 360;
				 //int angI = rand() % 360;
	float ang = angI;


	Color aColor = Color::White;
	Color bColor = Color::White;//Color( 40, 0, 0 );


	Color sColor = GetBlendColor(aColor, bColor, data.boostPortion);

	int ttlVariation = 40;
	//int ttlValue = (rand() % ttlVariation) - ttlVariation / 2;
	int ttlValue = 60;//(sess->GetRand() % ttlVariation) - ttlVariation / 2;
	int finalTimeToLive = 300 + ttlValue;

	int tile = 0;

	switch (particleType)
	{
	case PARTICLE_BOOSTER_GRAVITY_INCREASER:
	{
		maxRad = 64;
		minRad = 20;
		tile = 0;
		ang = 0;

		int tileR = sess->GetRand() % 3;
		if (tileR == 0)
		{
			tile += 1;
			maxRad = 32;
			minRad = 20;
		}

		break;
	}
	case PARTICLE_BOOSTER_GRAVITY_DECREASER:
	{
		maxRad = 64;
		minRad = 20;
		tile = 5;
		ang = 0;

		int tileR = sess->GetRand() % 3;
		if (tileR == 0)
		{
			tile += 1;
			maxRad = 32;
			minRad = 20;
		}
		//int tileR = sess->GetRand() % 2;

		break;
	}
	default:
		tile = 0;
		ang = 0;
		break;
	}

	//int r = rand() % ((maxRad - minRad) + 1) + minRad;
	int r = sess->GetRand() % ((maxRad - minRad) + 1) + minRad;
	float rad = r;

	sp->Activate(rad, sPos, ang, finalTimeToLive, Color::White, tile);
	sp->data.vel = normalize(sPos - data.pos) * .1f;//10.f;
													//360

	Color sColorTransParent = sColor;
	sColorTransParent.a = 70;

	sp->SetColorShift(sColor, sColorTransParent, 0, 60);

}

void ForegroundTestEmitter::Set(sf::Vector2f &pos, float zoom )
{
	extraOffset = pos;
	extraZoom = zoom;
}

void ForegroundTestEmitter::SetExtra(sf::Vector2f &p_extra)
{
	extra = p_extra;
}

void ForegroundTestEmitter::SpecialUpdate()
{
	scrollOffset = 0;
	Vector2f cPos = sess->cam.GetPos();
	//float testScrollOffset = extra.x + scrollOffset;
	//cPos.x -= testScrollOffset;

	//cPos = cPos * depth;
	
	//Vector2f realPos(cPos.x + off.x, camPos.y);
	//realPos = camPos;
	//realPos.x -= 960;
	//SetLeftPos(cPos);//camPos.x );//realPos.x );
}

void ForegroundTestEmitter::Draw(sf::RenderTarget *target)
{
	oldView = target->getView();

	Vector2f cTest = oldView.getCenter() * depth;//sess->cam.GetPos() * depth;
	//Set(cTest, sess->cam.GetZoom() * depth);

	newView.setCenter(cTest);//Vector2f(oldView.getCenter().x, oldView.getCenter().y) - extraOffset);
	//newView.setSize(Vector2f(1920, 1080) / extraZoom * .5f);
	//newView.setSize(Vector2f(960, 540) * sess->cam.GetZoom() / depth);// / extraZoom);
	newView.setSize(oldView.getSize() / depth);

	target->setView(newView);

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

	target->setView(oldView);



	
}