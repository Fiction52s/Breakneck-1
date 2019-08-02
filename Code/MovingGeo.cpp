#include "MovingGeo.h"
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;


sf::Color GetBlendColor(
	sf::Color c0,
	sf::Color c1,
	float progress)
{
	sf::Color blendColor;
	blendColor.r = c0.r * (1.0 - progress) + c1.r * (progress);
	blendColor.g = c0.g * (1.0 - progress) + c1.g * (progress);
	blendColor.b = c0.b * (1.0 - progress) + c1.b * (progress);
	blendColor.a = c0.a * (1.0 - progress) + c1.a * (progress);
	return blendColor;
}


ShapeParticle::ShapeParticle(int p_numPoints, sf::Vertex *v,
	ShapeEmitter *p_emit)
	:numPoints(p_numPoints), points(v), emit( p_emit )
{

}

void ShapeParticle::Activate(float p_radius, sf::Vector2f &p_pos,
	sf::Vector2f &p_vel, float p_angle, int p_ttl)
{
	pos = p_pos;
	vel = p_vel;
	ttl = p_ttl;
	radius = p_radius;
	angle = p_angle;

	if (emit->handler != NULL)
	{
		emit->handler->ActivateShapeParticle(this);
	}

	sf::Transform tr;

	int extraAngle = rand() % 360;
	tr.rotate(angle);//extraAngle);

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

	tileIndex = ti;
	SetRectSubRect(points, emit->ts->GetSubRect( ti ));
}

void ShapeParticle::Update()
{
	if (ttl < 0)
	{
		return;
	}

	Vector2f oldPos = pos;

	if (emit->handler != NULL)
	{
		emit->handler->UpdateShapeParticle(this);
	}

	pos += vel;
	vel += emit->accel;

	for (int i = 0; i < numPoints; ++i)
	{
		points[i].position += vel;
	}

	//UpdatePoints();

	--ttl;

	if (ttl < 0)
	{
		Clear();
	}
}

void ShapeParticle::Clear()
{
	for (int i = 0; i < numPoints; ++i)
	{
		points[i].position = Vector2f(0, 0);
	}
	ttl = -1;
}

ShapeEmitter::ShapeEmitter(int p_pointsPerShape,
	int p_numShapes, float p_angle,
	float p_angleRange,
	float p_minSpeed, float p_maxSpeed, ParticleHandler *h)
	:pointsPerShape( p_pointsPerShape ), numShapesTotal( p_numShapes ),
	angle( p_angle ), angleRange( p_angleRange ), minSpeed( p_minSpeed ),
	maxSpeed( p_maxSpeed ), handle( h )
{
	if (pointsPerShape > 4)
	{
		pointsPerShape *= 3;
	}
	numPoints = numShapesTotal * pointsPerShape;
	points = new Vertex[numPoints];
	particles = new ShapeParticle*[numShapesTotal];
	for (int i = 0; i < numShapesTotal; ++i)
	{
		particles[i] = new ShapeParticle(pointsPerShape, points + i * pointsPerShape, this);
	}
	Color r = Color::Red;
	SetColor(r);
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
	pos = p_pos;
}

void ShapeEmitter::Reset()
{
	for (int i = 0; i < numShapesTotal; ++i)
	{
		particles[i]->Clear();
	}
	frame = 0;
}

void ShapeEmitter::SetColor(sf::Color &c)
{

	for (int i = 0; i < numShapesTotal; ++i)
	{
		sf::Color randColor(rand() % 255, rand() % 255, rand() % 255, 100 + rand() % 155);
		particles[i]->SetColor(randColor);
	}
}

void ShapeEmitter::ActivateParticle( int index )
{
	float a = angle;
	float f = (float)rand() / RAND_MAX * 2.0 - 1.0;
	a += angleRange * f;

	float s = minSpeed;
	float f1 = (float)rand() / RAND_MAX;
	s += (maxSpeed - minSpeed) * f1;

	Vector2f vel(1, 0);
	vel *= s;
	RotateCCW(vel, a);

	int rad = 2 + rand() % 15;

	int extraAngle = rand() % 360;

	particles[index]->Activate( rad, pos, vel, extraAngle, 180 );
	particles[index]->SetTileIndex(0);
}

void ShapeEmitter::Update()
{
	bool activate = false;
	//if( frame == 0 )
	if (frame % 10 == 0)
	{
		activate = true;
	}

	for (int i = 0; i < numShapesTotal; ++i)
	{
		if (particles[i]->ttl < 0 && activate)
		{
			ActivateParticle(i);
			activate = false;
		}
		else
		{
			particles[i]->Update();
		}

		if (particles[i]->ttl < 0 && activate)
		{
			ActivateParticle(i);
			activate = false;
		}
	}

	++frame;
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

void ShapeEmitter::AddForce(sf::Vector2f &f)
{
	accel += f;
}

void ShapeEmitter::ClearForces()
{
	accel = Vector2f(0, 0);
}

void ShapeEmitter::SetForce(sf::Vector2f &f)
{
	accel = f;
}

//void ShapeParticle::UpdatePoints()
//{
//	
//}


MovingGeo::MovingGeo()
	:points( NULL )
{

}

MovingGeo::~MovingGeo()
{

}

void MovingGeo::SetBase(sf::Vector2f &base)
{
	basePos = base;
}

MovingGeoGroup::~MovingGeoGroup()
{
	delete[] points;
}

void MovingGeo::SetColor(sf::Color c)
{
	assert(points != NULL);
	color = c;
	int numPoints = GetNumPoints();
	for (int i = 0; i < numPoints; ++i)
	{
		points[i].color = color;
	}
}

void MovingGeo::SetPoints(sf::Vertex *p)
{
	assert(points == NULL);
	points = p;
}

void MovingGeo::Clear()
{
	int numPoints = GetNumPoints();
	for (int i = 0; i < numPoints; ++i)
	{
		points[i].position = Vector2f(0, 0);
	}
}

void MovingGeoGroup::Reset()
{
	frame = 0;
	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		(*it)->Reset();
	}
}

void MovingGeoGroup::SetBase(sf::Vector2f &pos)
{
	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		(*it)->SetBase(pos);
	}
}

bool MovingGeoGroup::Update()
{
	bool running = false;

	auto wit = waitFrames.begin();
	int wFrames;
	for (auto it = geoList.begin(); it != geoList.end(); ++it, ++wit)
	{
		wFrames = (*wit);
		if (frame >= wFrames)
		{
			(*it)->Update();
			if (!(*it)->done)
			{
				running = true;
			}
		}
		else
		{
			running = true;
		}
	}

	++frame;

	return running;
}

void MovingGeoGroup::Draw(sf::RenderTarget *target)
{
	target->draw(points, numTotalPoints, sf::Quads);
}

void MovingGeoGroup::AddGeo(MovingGeo *mg, int wf)
{
	geoList.push_back(mg);
	waitFrames.push_back(wf);
}

void MovingGeoGroup::Init()
{
	numTotalPoints = 0;
	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		numTotalPoints += (*it)->GetNumPoints();
	}

	points = new Vertex[numTotalPoints];

	sf::Vertex *curr = points;
	int np;
	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		np = (*it)->GetNumPoints();
		(*it)->SetPoints(curr);
		curr += np;
		(*it)->Init();
	}
}




SpinningTri::SpinningTri(float p_startAngle)
	:startAngle( p_startAngle )
{
	stateLength[S_EXPANDING] = 15;
	stateLength[S_GROW] = 15;
	stateLength[S_ROTATE] = 15;
	stateLength[S_ROTATE_AND_FADE] = 45;
	
	maxLength = 2000;
	width = 100;
	startWidth = 100;
	finalWidth = 200;
	startColor = Color::Blue;
	fadeColor = Color(40, 40, 40, 0);
}

void SpinningTri::Reset()
{
	color = startColor;
	length = 0;
	frame = 0;
	state = S_EXPANDING;
	angle = startAngle;
	width = startWidth;
	done = false;
	Clear();
	//UpdatePoints();
}

void SpinningTri::Update()
{
	if (done)
		return;

	if (frame == stateLength[state])
	{
		frame = 0;
		switch (state)
		{
		case S_EXPANDING:
			state = S_GROW;
			break;
		case S_GROW:
			state = S_ROTATE;
			break;
		case S_ROTATE:
			state = S_ROTATE_AND_FADE;
			break;
		case S_ROTATE_AND_FADE:
			done = true;
			Clear();
			return;
			break;
		}
	}

	double fac = (double)frame / stateLength[state];
	float finalAngle = startAngle + PI / 12.0;
	float finalAngle2 = finalAngle + PI / 6.0;
	switch (state)
	{
	case S_EXPANDING:
		length = fac * maxLength;
		break;
	case S_GROW:
		width = startWidth * (1.0 - fac) + finalWidth *(fac);
		break;
	case S_ROTATE:
		angle = startAngle * (1.0 - fac) + finalAngle * (fac);
		break;
	case S_ROTATE_AND_FADE:
		angle = finalAngle * (1.0 - fac) + finalAngle2 * (fac);
		SetColorChange(startColor, fadeColor, fac);
		break;
	}


	UpdatePoints();
	++frame;
}

void SpinningTri::UpdatePoints()
{
	Vector2f dir(1, 0);
	RotateCCW(dir, angle);

	Vector2f norm(dir.y, -dir.x);

	Vector2f end = basePos + dir * length;

	points[0].position = basePos;
	points[1].position = basePos;
	points[2].position = end + norm * (width / 2.f);
	points[3].position = end - norm * (width / 2.f);

	SetColor(color);
}

void SpinningTri::SetColorChange(sf::Color &startC,
	sf::Color &endC, float progress)
{
	sf::Color blendColor;
	blendColor.r = startC.r * (1.0 - progress) + endC.r * (progress);
	blendColor.g = startC.g * (1.0 - progress) + endC.g * (progress);
	blendColor.b = startC.b * (1.0 - progress) + endC.b * (progress);
	blendColor.a = startC.a * (1.0 - progress) + endC.a * (progress);
	color = blendColor;
}

void SpinningTri::SetColorGrad(sf::Color startCol,
	sf::Color endCol)
{
	points[0].color = startCol;
	points[1].color = startCol;
	points[2].color = endCol;
	points[3].color = endCol;
}


Laser::Laser(float p_startAngle)
	:startAngle(p_startAngle)
{
	stateLength[S_VERTICALGROW] = 30;// 60 / 20;
	stateLength[S_WIDEN] = 30;
	stateLength[S_SHRINK] = 15;
	stateLength[S_SLOWGROW] = 45;
	stateLength[S_DISAPPEAR] = 40;

	maxHeight = 2000;
	//quad[0].position = center;
	
	width = 100;
	startWidth = 100;
	growWidth = 200;
	shrinkWidth = 150;

	startColor = Color::White;
}

void Laser::Reset()
{
	SetColor(color);
	color = startColor;
	height = 0;
	width = 100;
	frame = 0;
	state = S_VERTICALGROW;
	angle = startAngle;
	width = startWidth;
	done = false;
}

void Laser::SetHeight(float h)
{
	height = h;
	float top = basePos.y - height / 2.f;
	float bot = basePos.y + height / 2.f;
	points[0].position.y = top;
	points[1].position.y = top;
	points[2].position.y = bot;
	points[3].position.y = bot;

	points[4].position.y = top;
	points[5].position.y = top;
	points[6].position.y = bot;
	points[7].position.y = bot;
}

void Laser::SetWidth(float w)
{
	width = w;
	float left = basePos.x - width / 2.f;
	float right = basePos.x + width / 2.f;
	points[0].position.x = left;
	points[3].position.x = left;
	points[1].position.x = right;
	points[2].position.x = right;

	points[4].position.x = basePos.x;
	points[7].position.x = basePos.x;
	points[5].position.x = right + 20;
	points[6].position.x = right + 20;
	
}

void Laser::Update()
{
	if (done)
		return;

	if (frame == stateLength[state])
	{
		frame = 0;

		switch (state)
		{
		case S_VERTICALGROW:
			state = S_WIDEN;
			break;
		case S_WIDEN:
			state = S_SHRINK;
			break;
		case S_SHRINK:
			state = S_SLOWGROW;
			break;
		case S_SLOWGROW:
			state = S_DISAPPEAR;
			break;
		case S_DISAPPEAR:
			done = true;
			Clear();
			return;
			break;
		}
	}

	double fac = (double)frame / stateLength[state];
	//float finalAngle = startAngle + PI / 12.0;
	//float finalAngle2 = finalAngle + PI / 6.0;
	switch (state)
	{
	case S_VERTICALGROW:
		SetWidth(startWidth);
		SetHeight( fac * maxHeight);
		//length = fac * maxLength;
		break;
	case S_WIDEN:
		SetWidth(startWidth * (1.0 - fac) + growWidth *(fac));
		//width = startWidth * (1.0 - fac) + finalWidth *(fac);
		break;
	case S_SHRINK:
		SetWidth(growWidth * (1.0 - fac) + shrinkWidth *(fac));
		//angle = startAngle * (1.0 - fac) + finalAngle * (fac);
		break;
	case S_SLOWGROW:
		SetWidth(shrinkWidth * (1.0 - fac) + growWidth *(fac));
		//angle = finalAngle * (1.0 - fac) + finalAngle2 * (fac);
		//SetColorChange(startColor, fadeColor, fac);
		break;
	case S_DISAPPEAR:
		SetWidth(growWidth * (1.0 - fac));
		//angle = finalAngle * (1.0 - fac) + finalAngle2 * (fac);
		
		//SetColorChange(startColor, fadeColor, fac);
		break;
	}

	//Vector2f dir(1, 0);
	//RotateCCW(dir, angle);
	
	++frame;
}

void Laser::SetColorChange(sf::Color &startC,
	sf::Color &endC, float progress)
{
	color = GetBlendColor( startC, endC, progress );
}

void Laser::SetColor(sf::Color c)
{
	for (int i = 0; i < 4; ++i)
	{
		points[i].color = c;
	}

	Color test = Color::Cyan;
	test.a = 10;
	for (int i = 0; i < 4; ++i)
	{
		points[i+4].color = test;
	}

}


Ring::Ring(int p_circlePoints)
	:innerRadius(100), outerRadius(200), shader(NULL), circlePoints(p_circlePoints)
{
	ownsPoints = false;
	color = Color::White;
}

Ring::~Ring()
{
	if (ownsPoints)
	{
		delete[]points;
	}
}

void Ring::CreatePoints()
{
	points = new Vertex[circlePoints * 4];
	ownsPoints = true;
}

void Ring::SetPoints(sf::Vertex *p)
{
	points = p;
	UpdatePoints();
}

void Ring::SetShader(sf::Shader *sh)
{
	shader = sh;
}

void Ring::Set(sf::Vector2f pos,
	float innerR, float ringWidth)
{
	position = pos;
	innerRadius = innerR;
	outerRadius = innerR + ringWidth;
	UpdatePoints();
}

void Ring::UpdatePoints()
{
	Transform tr;
	Vector2f offsetInner(0, -innerRadius);
	Vector2f offsetOuter(0, -outerRadius);
	for (int i = 0; i < circlePoints; ++i)
	{
		points[i * 4 + 0].position = position + tr.transformPoint(offsetInner);
		points[i * 4 + 1].position = position + tr.transformPoint(offsetOuter);

		tr.rotate(360.f / circlePoints);

		points[i * 4 + 2].position = position + tr.transformPoint(offsetOuter);
		points[i * 4 + 3].position = position + tr.transformPoint(offsetInner);
	}
}

void Ring::Draw(sf::RenderTarget *target)
{
	if (shader == NULL)
	{
		target->draw(points, circlePoints * 4, sf::Quads);
	}
	else
	{
		target->draw(points, circlePoints * 4, sf::Quads, shader);
	}
}

MovingRing::MovingRing(int p_circlePoints,
	float p_startInner,
	float p_endInner,
	float p_startWidth,
	float p_endWidth,
	Vector2f p_startPos,
	Vector2f p_endPos,
	sf::Color p_startCol,
	sf::Color p_endCol,
	int p_totalFrames)
	:Ring( p_circlePoints ), startInner(p_startInner), startWidth(p_startWidth), endInner(p_endInner), endWidth(p_endWidth),
	startPos(p_startPos), endPos(p_endPos), totalFrames( p_totalFrames ),
	startColor(p_startCol), endColor(p_endCol)
{
}

void MovingRing::Reset()
{
	Clear();//Set(startPos, startInner, startWidth);
	frame = 0;
	done = false;
}

void MovingRing::Update()
{
	if (done)
		return;

	if (frame > totalFrames)
	{
		done = true;
		Clear();
		return;
		//Reset(); //debugging
	}

	float fac = (float)frame / totalFrames;
	float sa = sizeBez.GetValue(fac);
	float ca = colorBez.GetValue(fac);
	float pa = posBez.GetValue(fac);
	float ia = innerBez.GetValue(fac);

	Vector2f currPos = startPos * (1.f - pa) + endPos * pa + basePos;
	Color currColor = GetBlendColor(startColor, endColor, ca);
	float currInner = startInner * (1.f - ia) + endInner * ia;
	float currWidth = startWidth * (1.f - sa) + endWidth * sa;

	Set(currPos, currInner, currWidth);
	SetColor(currColor);

	++frame;
}