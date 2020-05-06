#include "TransformTools.h"
#include "VectorMath.h"
#include "CircleGroup.h"

using namespace std;
using namespace sf;

TransformTools::TransformTools()
{
	tRect.setFillColor(Color::Transparent);
	tRect.setOutlineColor(Color::Red);
	tRect.setOutlineThickness(-2);

	scalePointRadius = 8;
	rotatePointRadius = 30;

	circleGroup = new CircleGroup(8, scalePointRadius, Color::Magenta, 10);
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

void TransformTools::Reset(Vector2f &p_center, Vector2f &p_size,float startAngle )
{
	mode = NONE;

	origCenter = p_center;

	rotationAnchor = origCenter;
	
	baseRotation = startAngle;

	extra = 0;//create extra space surrounding polys
	origSize = p_size;
	rectSize = p_size + Vector2f(extra, extra);
	size = origSize;

	originOffset = Vector2f(0, 0);

	tRect.setSize(rectSize);
	tRect.setOrigin(rectSize.x / 2.f, rectSize.y / 2.f);
	tRect.setRotation(baseRotation);
	tRect.setPosition(origCenter);

	rotation = 0.f;
	scale = Vector2f(1.f, 1.f);
	UpdateGrabPoints();

	clickedNothing = false;
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

	return false;
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
	scaleAnchorIndex = scalePoint - 4;
	if (scaleAnchorIndex < 0)
		scaleAnchorIndex += 8;

	scaleAnchor = circleGroup->GetPosition(scaleAnchorIndex);

	if (scaleAnchorIndex % 2 == 0)
	{
		int anchorNext = scaleAnchorIndex + 1;
		if (anchorNext == 8)
			anchorNext = 0;
		int anchorPrev = scaleAnchorIndex - 1;
		if (anchorPrev == -1)
			anchorPrev = 7;

		scaleAlong = normalize(circleGroup->GetPosition(anchorNext) - scaleAnchor);
		scaleOther = normalize(circleGroup->GetPosition(anchorPrev) - scaleAnchor);
	}
	else
	{
		scaleAlong = normalize(circleGroup->GetPosition(scalePoint) - scaleAnchor);
		scaleOther = Vector2f(0, 0);
	}

	UpdateScaleOrigin();

	tRect.setPosition(scaleAnchor);
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

				UpdateRotationAnchor();

				Vector2f scaledOffset = GetScaledOffset();
				tRect.setOrigin(rectSize.x / 2 + scaledOffset.x, rectSize.y / 2 + scaledOffset.y);
				tRect.setPosition(rotationAnchor);
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
		Vector2f diff = worldPos - scaleAnchor;

		float along, other;
		//Vector2f currDiff = normalize(worldPos - center);

		along = dot(diff, scaleAlong) - extra;
		if (scalePoint % 2 == 0)
		{
			other = dot(diff, scaleOther) - extra;

			if (scalePoint == 0 || scalePoint == 4)
			{
				scale.x = along / origSize.x;
				scale.y = other / origSize.y;
			}
			else
			{
				scale.x = other / origSize.x;
				scale.y = along / origSize.y;
			}
		}
		else
		{
			if (scalePoint == 3 || scalePoint == 7)
			{
				scale.x = along / origSize.x;
			}
			else
			{
				scale.y = along / origSize.y;
			}
		}

		size = Vector2f(origSize.x * scale.x, origSize.y * scale.y);
		rectSize = size + Vector2f(extra, extra);
		tRect.setSize(rectSize);

		UpdateScaleOrigin();

		UpdateGrabPoints();

		UpdateRotationAnchor();
		break;
	}
	case ROTATE:
	{
		Vector2f startDir = normalize(startClick - rotationAnchor);
		Vector2f currDir = normalize(worldPos - rotationAnchor);
		
		float diffRad = GetVectorAngleDiffCW(startDir, currDir);
		float diffDeg = diffRad / PI * 180;

		rotation = rotationStart + diffDeg;

		tRect.setRotation(rotation + baseRotation);

		UpdateGrabPoints();
		break;
	}
	}
	
}

sf::Vector2f TransformTools::GetCenter()
{
	return Vector2f(GetRectPoint(0) + GetRectPoint(1) + GetRectPoint(2) + GetRectPoint(3)) / 4.f;
}

void TransformTools::UpdateRotationAnchor()
{
	rotationAnchor = GetCenter() + GetTransformedOffset();
}

void TransformTools::UpdateScaleOrigin()
{
	if (scaleAnchorIndex % 2 == 0)
	{
		tRect.setOrigin(tRect.getPoint(scaleAnchorIndex / 2));
	}
	else
	{
		tRect.setOrigin((tRect.getPoint(scaleAnchorIndex / 2) + tRect.getPoint(scaleAnchorIndex / 2 + 1)) / 2.f);
	}
}

sf::Vector2f TransformTools::GetTransformedOffset()
{
	Transform t;
	t.rotate(rotation);
	t.scale(scale);
	return t.transformPoint(originOffset);
}

sf::Vector2f TransformTools::GetScaledOffset()
{
	return Vector2f(originOffset.x * scale.x, originOffset.y * scale.y);
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