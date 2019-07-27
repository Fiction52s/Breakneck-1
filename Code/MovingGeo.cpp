#include "MovingGeo.h"
#include "VectorMath.h"

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

SpinningTri::SpinningTri(float p_startAngle, sf::Vector2f &p_center)
	:center( p_center ), startAngle( p_startAngle )
{
	stateLength[S_EXPANDING] = 15;
	stateLength[S_GROW] = 15;
	stateLength[S_ROTATE] = 15;
	stateLength[S_ROTATE_AND_FADE] = 45;
	
	maxLength = 2000;
	tri[0].position = center;
	Reset();
	width = 100;
	startWidth = 100;
	finalWidth = 200;
	startColor = Color::White;
	fadeColor = Color(40, 40, 40, 80);
}

void SpinningTri::Reset()
{
	currColor = startColor;
	length = 0;
	frame = 0;
	state = S_EXPANDING;
	angle = startAngle;
	width = startWidth;
}

void SpinningTri::Update()
{
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
			Reset();
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


	Vector2f dir(1, 0);
	RotateCCW(dir, angle);

	Vector2f norm(dir.y, -dir.x);

	Vector2f end = center + dir * length;

	tri[1].position = end + norm * (width / 2.f);
	tri[2].position = end - norm * (width / 2.f);

	SetColor(currColor);
	++frame;
}

void SpinningTri::SetColorChange(sf::Color &startC,
	sf::Color &endC, float progress)
{
	sf::Color blendColor;
	blendColor.r = startC.r * (1.0 - progress) + endC.r * (progress);
	blendColor.g = startC.g * (1.0 - progress) + endC.g * (progress);
	blendColor.b = startC.b * (1.0 - progress) + endC.b * (progress);
	blendColor.a = startC.a * (1.0 - progress) + endC.a * (progress);
	currColor = blendColor;
}

void SpinningTri::SetColor(sf::Color c)
{
	for (int i = 0; i < 3; ++i)
	{
		tri[i].color = c;
	}
}

void SpinningTri::SetColorGrad(sf::Color startCol,
	sf::Color endCol)
{
	tri[0].color = startCol;
	tri[1].color = endCol;
	tri[2].color = endCol;
}

void SpinningTri::Draw(RenderTarget *target)
{
	//target->draw(tri, 3, sf::Triangles);
}

Laser::Laser(float p_startAngle, sf::Vector2f &p_center)
	:center(p_center), startAngle(p_startAngle)
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

	Reset();
}

void Laser::Reset()
{
	SetHeight(0);
	SetWidth(startWidth);
	currColor = startColor;
	SetColor(currColor);
	height = 0;
	width = 100;
	frame = 0;
	state = S_VERTICALGROW;
	angle = startAngle;
	width = startWidth;
}

void Laser::SetHeight(float h)
{
	height = h;
	float top = center.y - height / 2.f;
	float bot = center.y + height / 2.f;
	quad[0].position.y = top;
	quad[1].position.y = top;
	quad[2].position.y = bot;
	quad[3].position.y = bot;

	quad[4].position.y = top;
	quad[5].position.y = top;
	quad[6].position.y = bot;
	quad[7].position.y = bot;
}

void Laser::SetWidth(float w)
{
	width = w;
	float left = center.x - width / 2.f;
	float right = center.x + width / 2.f;
	quad[0].position.x = left;
	quad[3].position.x = left;
	quad[1].position.x = right;
	quad[2].position.x = right;

	quad[4].position.x = center.x;
	quad[7].position.x = center.x;
	quad[5].position.x = right + 20;
	quad[6].position.x = right + 20;
	
}

void Laser::Update()
{
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
			Reset();
			break;
		}
	}

	double fac = (double)frame / stateLength[state];
	//float finalAngle = startAngle + PI / 12.0;
	//float finalAngle2 = finalAngle + PI / 6.0;
	switch (state)
	{
	case S_VERTICALGROW:
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
	currColor = GetBlendColor( startC, endC, progress );
}

void Laser::SetColor(sf::Color c)
{
	for (int i = 0; i < 4; ++i)
	{
		quad[i].color = c;
	}

	Color test = Color::Cyan;
	test.a = 10;
	for (int i = 0; i < 4; ++i)
	{
		quad[i+4].color = test;
	}

}

void Laser::Draw(RenderTarget *target)
{
	//target->draw(quad, 8, sf::Quads);
}

//const int Ring::circlePoints = 32;

Ring::Ring(int p_circlePoints)
	:innerRadius(100), outerRadius(200), shader(NULL), circlePoints(p_circlePoints)
{
	position = Vector2f(0, 0);
	color = Color::White;
	circle = new Vertex[circlePoints * 4];
	UpdatePoints();
}

Ring::~Ring()
{
	delete[] circle;
}

void Ring::SetShader(sf::Shader *sh)
{
	shader = sh;
}

void Ring::SetColor(sf::Color c)
{
	color = c;
	for (int i = 0; i < circlePoints * 4; ++i)
	{
		circle[i].color = color;
	}
	
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
		circle[i * 4 + 0].position = position + tr.transformPoint(offsetInner);
		circle[i * 4 + 1].position = position + tr.transformPoint(offsetOuter);

		tr.rotate(360.f / circlePoints);

		circle[i * 4 + 2].position = position + tr.transformPoint(offsetOuter);
		circle[i * 4 + 3].position = position + tr.transformPoint(offsetInner);
	}
}

void Ring::Draw(sf::RenderTarget *target)
{
	if (shader == NULL)
	{
		target->draw(circle, circlePoints * 4, sf::Quads);
	}
	else
	{
		target->draw(circle, circlePoints * 4, sf::Quads, shader);
	}
	
}

MovingRing::MovingRing(int p_circlePoints,
	float p_startInner,
	float p_startWidth,
	float p_endInner,
	float p_endWidth,
	Vector2f p_startPos,
	Vector2f p_endPos,
	sf::Color p_startCol,
	sf::Color p_endCol,
	int p_totalFrames)
	:startInner(p_startInner), startWidth(p_startWidth), endInner(p_endInner), endWidth(p_endWidth),
	startPos(p_startPos), endPos(p_endPos), totalFrames( p_totalFrames ),
	startColor(p_startCol), endColor(p_endCol)
{
	ring = new Ring(p_circlePoints);
	Reset();
}

void MovingRing::Reset()
{
	ring->Set(startPos, startInner, startWidth);
	frame = 0;
}

void MovingRing::Update()
{
	if( frame > totalFrames )
	{
		Reset(); //debugging
	}

	float fac = (float)frame / totalFrames;
	float sa = sizeBez.GetValue(fac);
	float ca = colorBez.GetValue(fac);
	float pa = posBez.GetValue(fac);
	float ia = innerBez.GetValue(fac);

	Vector2f currPos = startPos * (1.f - pa) + endPos * pa;
	Color currColor = GetBlendColor(startColor, endColor, ca);
	float currInner = startInner * (1.f - ia) + endInner * ia;
	float currWidth = startWidth * (1.f - sa) + endWidth * sa;

	ring->Set(currPos, currInner, currWidth);
	ring->SetColor(currColor);

	++frame;
}

void MovingRing::Draw(sf::RenderTarget *target)
{
	ring->Draw(target);
}