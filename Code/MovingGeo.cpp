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
	delete[] points;
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
}

void MovingGeoGroup::Reset()
{
	frame = 0;
	for (auto it = geoList.begin(); it != geoList.end(); ++it)
	{
		(*it)->Reset();
	}
	running = false;
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
	bool stillRunning = false;

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
				stillRunning = true;
			}
		}
		else
		{
			stillRunning = true;
		}
	}

	++frame;

	running = stillRunning;
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

void MovingRing::SetBase(sf::Vector2f &base)
{
	basePos = base;

	/*Vector2f diff = endPos - startPos;
	startPos = basePos;
	endPos = startPos + diff;*/
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