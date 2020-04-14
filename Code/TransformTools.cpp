#include "TransformTools.h"
#include "VectorMath.h"
#include "CircleGroup.h"

using namespace std;
using namespace sf;

TransformTools::TransformTools()
{
	tRect.setFillColor(Color::Transparent);
	tRect.setOutlineColor(Color::Red);
	tRect.setOutlineThickness(4);

	int scaleRectWidth = 20;
	int rotateRectWidth = 40;

	Vector2f scaleRectSize(scaleRectWidth, scaleRectWidth);
	Vector2f rotateRectSize(rotateRectWidth, rotateRectWidth);

	circleGroup = new CircleGroup(8, 20, Color::Magenta, 10);
	circleGroup->ShowAll();
	/*for (int i = 0; i < 8; ++i)
	{
		scaleGrabRect[i].setSize(scaleRectSize);
		rotateGrabRect[i].setSize(rotateRectSize);

		scaleGrabRect[i].setFillColor(Color::Transparent);
		rotateGrabRect[i].setFillColor(Color::Transparent);

		scaleGrabRect[i].setOutlineColor(Color::Blue);
		rotateGrabRect[i].setOutlineColor(Color::Magenta);

		scaleGrabRect[i].setOutlineThickness(2);
		rotateGrabRect[i].setOutlineThickness(2);
	}

	
	scaleGrabRect[0].setOrigin(scaleRectSize);
	rotateGrabRect[0].setOrigin(rotateRectSize);

	scaleGrabRect[1].setOrigin(scaleRectSize.x / 2, scaleRectSize.y);
	rotateGrabRect[1].setOrigin(rotateRectSize.x / 2, rotateRectSize.y);

	scaleGrabRect[2].setOrigin(0, scaleRectSize.y);
	rotateGrabRect[2].setOrigin(0, rotateRectSize.y);

	scaleGrabRect[3].setOrigin(0, scaleRectSize.y/2);
	rotateGrabRect[3].setOrigin(0, rotateRectSize.y/2);

	scaleGrabRect[4].setOrigin(0, 0);
	rotateGrabRect[4].setOrigin(0, 0);

	scaleGrabRect[5].setOrigin(scaleRectSize.x / 2, 0);
	rotateGrabRect[5].setOrigin(rotateRectSize.x / 2, 0);

	scaleGrabRect[6].setOrigin(scaleRectSize.x, 0);
	rotateGrabRect[6].setOrigin(rotateRectSize.x, 0);

	scaleGrabRect[7].setOrigin(scaleRectSize.x, scaleRectSize.y / 2);
	rotateGrabRect[7].setOrigin(rotateRectSize.x, rotateRectSize.y / 2);*/
}

TransformTools::~TransformTools()
{
	delete circleGroup;
}

void TransformTools::Reset(Vector2f &p_center, Vector2f &p_size)
{
	mode = NONE;

	center = p_center;
	size = p_size;

	origCenter = center;
	origSize = size;

	originOffset = Vector2f(0, 0);

	tRect.setSize(size);
	tRect.setOrigin(tRect.getLocalBounds().width / 2,
		tRect.getLocalBounds().height / 2);
	tRect.setPosition(center);

	rotation = 0.f;
	scale = 1.f;
	UpdateGrabPoints();
}

void TransformTools::Draw(sf::RenderTarget *target)
{
	target->draw(tRect);
	circleGroup->Draw(target);
}

bool TransformTools::ClickedRotatePoint(sf::Vector2f &pos)
{
	if (RectContainsPoint(pos))
	{
		return false;
	}

	Vector2f circlePos;
	float lenSqr;
	for (int i = 0; i < 8; ++i)
	{
		circlePos = circleGroup->GetPosition(i);
		lenSqr = lengthSqr(pos - circlePos);
		if (lenSqr <= rotatePointRadius * rotatePointRadius)
		{
			return true;
		}
	}

	return false;
}

sf::Vector2f TransformTools::GetRectPoint(int i)
{
	const Transform &tempT = tRect.getTransform();
	return tempT.transformPoint(tRect.getPoint(i));
}

bool TransformTools::RectContainsPoint(Vector2f &pos)
{
	Vector2f A = GetRectPoint(0);
	Vector2f AB = GetRectPoint(1) - A;
	Vector2f AD = GetRectPoint(3) - A;
	Vector2f pointA = pos - A;
	float pointAB = dot(pointA, normalize(AB));
	float pointAD = dot(pointA, normalize(AD));

	if (pointAB >= 0 && pointAB * pointAB <= dot(AB, AB))
	{
		if (pointAD >= 0 && pointAD * pointAD <= dot(AD, AD))
		{
			return true;
		}
	}
}

int TransformTools::GetClickedScalePoint(sf::Vector2f &pos)
{
	Vector2f circlePos;
	float lenSqr;
	for (int i = 0; i < 8; ++i)
	{
		circlePos = circleGroup->GetPosition(i);
		lenSqr = lengthSqr(pos - circlePos);
		if (lenSqr <= scalePointRadius * scalePointRadius)
		{
			return i;
		}
	}

	return -1;
}

void TransformTools::SetScaleAnchor()
{
	int opposite = scalePoint - 4;
	if (opposite < 0)
		opposite + 8;

	scaleAnchor = circleGroup->GetPosition(opposite);
}

void TransformTools::Update( Vector2f &worldPos, bool mouseDown )
{
	if (!mouseDown)
	{
		mode = NONE;
		clickedNothing = false;
		return;
	}

	switch (mode)
	{
	case NONE:
	{
		if (mouseDown && !clickedNothing)
		{
			scalePoint = GetClickedScalePoint(worldPos);
			if (scalePoint >= 0)
			{
				SetScaleAnchor();
				mode = SCALE;
			}
			else if(ClickedRotatePoint(worldPos))
			{
				startClick = worldPos;
				mode = ROTATE;
				rotationStart = rotation;
			}
			else
			{
				clickedNothing = true;
			}
		}
		break;
	}
	case MOVE:
	{

		break;
	}
	case SCALE:
	{
		Vector2f diff = worldPos - center;

		scale = diff.x / (origSize.x / 2.f);

		if (scale < .1)
			scale = .1;

		size = Vector2f(origSize.x * scale, origSize.y * scale);

		tRect.setSize(size);
		tRect.setOrigin(tRect.getLocalBounds().width / 2,
			tRect.getLocalBounds().height / 2);
		tRect.setPosition(center);

		UpdateGrabPoints();
		break;
	}
	case ROTATE:
	{
		Vector2f startDir = normalize(startClick - GetRotationAnchor());
		Vector2f currDir = normalize(worldPos - GetRotationAnchor());
		
		float diffRad = GetVectorAngleDiffCW(startDir, currDir);
		float diffDeg = diffRad / PI * 180;

		rotation = rotationStart + diffDeg;

		tRect.setRotation(rotation);

		UpdateGrabPoints();
		break;
	}
	}
	
}

sf::Vector2f TransformTools::GetRotationAnchor()
{
	return center + originOffset;
}

void TransformTools::UpdateGrabPoints()
{
	Vector2f rectPoint, nextRectPoint, midPoint;
	int nextI;
	for (int i = 0; i < 4; ++i)
	{
		if (i == 3)
			nextI = 0;
		else
			nextI = i + 1;

		rectPoint = GetRectPoint(i);
		nextRectPoint = GetRectPoint(nextI);

		midPoint = (rectPoint + nextRectPoint) / 2.f;

		circleGroup->SetPosition(i*2, rectPoint);
		circleGroup->SetPosition(i*2+1, midPoint);
	}
}