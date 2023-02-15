#include "MovingGeo.h"
#include "VectorMath.h"
#include <assert.h>
#include <iostream>

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
	RemoveAll();
}

void MovingGeo::SetColor(sf::Color c)
{
	if (points == NULL)
	{
		assert(points != NULL);
	}
	
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
	data.done = false;
	data.state = 0;
	data.frame = 0;
}

int MovingGeo::GetNumStoredBytes()
{
	return sizeof(MovingGeoBasicData);
}

void MovingGeo::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &data, sizeof(MovingGeoBasicData));
	bytes += sizeof(MovingGeoBasicData);
}

void MovingGeo::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MovingGeoBasicData));
	bytes += sizeof(MovingGeoBasicData);
}

MovingGeoGroup::MovingGeoGroup()
{
	data.frame = 0;
	points = NULL;
	numTotalPoints = 0;
}

void MovingGeoGroup::RemoveAll()
{
	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		delete (*it);
	}
	geoList.clear();
	delete[] points;
	points = NULL;
}

void MovingGeoGroup::Reset()
{
	data.frame = 0;
	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		(*it)->Reset();
	}
	data.running = false;
}

void MovingGeoGroup::Start()
{
	data.running = true;
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
	if (!data.running)
		return false;

	bool stillRunning = false;

	auto wit = waitFrames.begin();
	int wFrames;
	for (auto it = geoList.begin(); it != geoList.end(); ++it, ++wit)
	{
		wFrames = (*wit);
		if (data.frame >= wFrames)
		{
			(*it)->Update();
			if (!(*it)->data.done)
			{
				stillRunning = true;
			}
		}
		else
		{
			stillRunning = true;
		}
	}

	++data.frame;

	data.running = stillRunning;
	return stillRunning;
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

	Reset();
}


int MovingGeoGroup::GetNumStoredBytes()
{
	int geoTotalSize = 0;
	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		geoTotalSize += (*it)->GetNumStoredBytes();
	}

	return sizeof(MyData) + geoTotalSize;
}

void MovingGeoGroup::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		(*it)->StoreBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}
}

void MovingGeoGroup::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	bytes += sizeof(MyData);

	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		(*it)->SetFromBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
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

	Clear();

	data.state = S_EXPANDING;

	length = 0;
	angle = startAngle;
	width = startWidth;
}

void SpinningTri::Update()
{
	if (data.done)
		return;

	if (data.frame == stateLength[data.state])
	{
		data.frame = 0;
		switch (data.state)
		{
		case S_EXPANDING:
			data.state = S_GROW;
			break;
		case S_GROW:
			data.state = S_ROTATE;
			break;
		case S_ROTATE:
			data.state = S_ROTATE_AND_FADE;
			break;
		case S_ROTATE_AND_FADE:
		{
			Clear();
			data.done = true;
			return;
			break;
		}
		}
	}

	double fac = (double)data.frame / stateLength[data.state];
	float finalAngle = startAngle + PI / 12.0;
	float finalAngle2 = finalAngle + PI / 6.0;
	switch (data.state)
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
	++data.frame;
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

Laser::Laser(float p_startWidth, float p_growWidth, float p_shrinkWidth, float p_startAngle,
	sf::Color p_startColor )
	:startAngle(p_startAngle),startWidth( p_startWidth),
	growWidth( p_growWidth ), shrinkWidth( p_shrinkWidth ),
	startColor( p_startColor )
{
	stateLength[S_VERTICALGROW] = 30;// 60 / 20;
	stateLength[S_WIDEN] = 30;
	stateLength[S_SHRINK] = 15;
	stateLength[S_SLOWGROW] = 45;
	stateLength[S_DISAPPEAR] = 40;

	maxHeight = 2000;
	
	//startWidth = 100
	//growWidth = 200;
	//shrinkWidth = 150;

	//startColor = Color::White;

	//Reset();
}

void Laser::Reset()
{
	SetColor(color);
	color = startColor;

	Clear();

	height = 0;
	data.state = S_VERTICALGROW;
	angle = startAngle;
	currWidth = startWidth;
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
	currWidth = w;
	float left = basePos.x - currWidth / 2.f;
	float right = basePos.x + currWidth / 2.f;
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
	if (data.done)
		return;

	if (data.frame == stateLength[data.state])
	{
		data.frame = 0;

		switch (data.state)
		{
		case S_VERTICALGROW:
			data.state = S_WIDEN;
			break;
		case S_WIDEN:
			data.state = S_SHRINK;
			break;
		case S_SHRINK:
			data.state = S_SLOWGROW;
			break;
		case S_SLOWGROW:
			data.state = S_DISAPPEAR;
			break;
		case S_DISAPPEAR:
			Clear();
			data.done = true;
			return;
			break;
		}
	}

	double fac = (double)data.frame / stateLength[data.state];
	//float finalAngle = startAngle + PI / 12.0;
	//float finalAngle2 = finalAngle + PI / 6.0;
	switch (data.state)
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
	
	++data.frame;
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
	Clear();
}

void MovingRing::Update()
{
	if (data.done)
		return;

	if (data.frame > totalFrames)
	{
		Clear();
		data.done = true;
		return;
		//Reset(); //debugging
	}

	float fac = (float)data.frame / totalFrames;
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

	++data.frame;
}

PokeTri::PokeTri(sf::Vector2f &p_offset)
{
	offset = p_offset;
	pokeAngle = GetVectorAngleCW(offset) / PI * 180;

	stateLength[S_POKING] = 6;
	stateLength[S_SHRINKING] = 6;

	startWidth = 30;
	startColor = Color( 255, 255, 255, 100 );//Color::Blue;
	fadeColor = Color(40, 40, 40, 0);
}

void PokeTri::Reset()
{
	Clear();

	color = startColor;
	length = 0;
	data.state = S_POKING;
	angle = pokeAngle;
	width = startWidth;
	lengthFactor = 1;

	maxLength = 500 + ( rand() % 100 );
	width = startWidth + (rand() % 20);
	//UpdatePoints();
}

void PokeTri::SetLengthFactor(float f)
{
	lengthFactor = f;
}

void PokeTri::Update()
{
	if (data.done)
		return;

	if (data.frame == stateLength[data.state])
	{
		data.frame = 0;
		switch (data.state)
		{
		case S_POKING:
			data.state = S_SHRINKING;
			break;
		case S_SHRINKING:
			Clear();
			data.done = true;
			return;
		}
	}

	double fac = (double)data.frame / stateLength[data.state];
	switch (data.state)
	{
	case S_POKING:
		length = fac * maxLength;
		break;
	case S_SHRINKING:
		length = (1.0 - fac) * maxLength;
		break;
	}


	UpdatePoints();
	++data.frame;
}

void PokeTri::UpdatePoints()
{
	//Vector2f dir(1, 0);
	//RotateCCW(dir, angle);

	Vector2f dir = normalize(-offset);

	Vector2f norm(dir.y, -dir.x);

	Vector2f end = basePos + offset + dir * length * lengthFactor;

	Vector2f triBase = basePos + offset;

	points[0].position = end;
	points[1].position = end;
	points[2].position = triBase + norm * (width / 2.f);
	points[3].position = triBase - norm * (width / 2.f);

	SetColor(color);
}