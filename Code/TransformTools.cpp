#include "TransformTools.h"

using namespace std;
using namespace sf;

TransformTools::TransformTools()
{
	tRect.setFillColor(Color::Transparent);
	tRect.setOutlineColor(Color::Red);
	tRect.setOutlineThickness(4);
}

void TransformTools::Reset(Vector2f &p_center, Vector2f &p_size)
{
	center = p_center;
	size = p_size;

	origCenter = center;
	origSize = size;

	originOffset = Vector2f(0, 0);

	tRect.setSize(size);
	tRect.setOrigin(tRect.getLocalBounds().width / 2,
		tRect.getLocalBounds().height / 2);
	tRect.setPosition(center);
}

void TransformTools::Draw(sf::RenderTarget *target)
{
	target->draw(tRect);
}

void TransformTools::Update( Vector2f &worldPos, bool mouseDown )
{
	if (mouseDown)
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
	}
}