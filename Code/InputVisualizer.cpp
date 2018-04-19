#include "InputVisualizer.h"
#include "VectorMath.h"

using namespace sf;

InputVisualizer::InputVisualizer()
{
	pos = Vector2f(400, 400);
	stickRel = Vector2f(0, 0);
	actionRel = Vector2f(100, 0);
	powerRel = Vector2f(-100, 0);
}

void InputVisualizer::Update(ControllerState &currInput)
{
	quads = new Vertex[4 * TOTALQUADS]; 
	sf::Vertex *controlStick = quads;
	


	for (int i = 0; i < TOTALQUADS; ++i)
	{
		quads[i * 4 + 0].color = Color::Black;
		quads[i * 4 + 1].color = Color::Black;
		quads[i * 4 + 2].color = Color::Black;
		quads[i * 4 + 3].color = Color::Black;
	}

	Color c = Color::Red;
	
	if (currInput.LLeft())
	{
		c = Color::Blue;
	}
	if (currInput.LRight())
	{
		c = Color::Green;
	}
	if (currInput.LUp())
	{
		c = Color::Yellow;
	}
	if (currInput.LDown())
	{
		c = Color::Magenta;
	}
	
	SetRectColor(quads + CONTROL, c);

	Color onC = Color::Magenta;
	Color offC = Color::Black;

	if (currInput.A)
	{
		SetRectColor(quads + (ACTION + 0) * 4, onC);
	}
	else
	{
		SetRectColor(quads + (ACTION + 0) * 4, offC);
	}

	if (currInput.B)
	{
		SetRectColor(quads + (ACTION + 1) * 4, onC);
	}
	else
	{
		SetRectColor(quads + (ACTION + 1) * 4, offC);
	}

	if (currInput.rightShoulder)
	{
		SetRectColor(quads + (ACTION + 2) * 4, onC);
	}
	else
	{
		SetRectColor(quads + (ACTION + 2) * 4, offC);
	}

	if (currInput.X)
	{
		SetRectColor(quads + (POWER + 0) * 4, onC);
	}
	else
	{
		SetRectColor(quads + (POWER + 0) * 4, offC);
	}

	if (currInput.Y)
	{
		SetRectColor(quads + (POWER + 1) * 4, onC);
	}
	else
	{
		SetRectColor(quads + (POWER + 1) * 4, offC);
	}

	if (currInput.leftShoulder)
	{
		SetRectColor(quads + (POWER + 2) * 4, onC);
	}
	else
	{
		SetRectColor(quads + (POWER + 2) * 4, offC);
	}

	if (currInput.RightTriggerPressed())
	{
		SetRectColor(quads + (POWER + 3) * 4, onC);
	}
	else
	{
		SetRectColor(quads + (POWER + 3) * 4, offC);
	}

	if (currInput.LeftTriggerPressed())
	{
		SetRectColor(quads + (POWER + 4) * 4, onC);
	}
	else
	{
		SetRectColor(quads + (POWER + 4) * 4, offC);
	}

	
	
	float dist = 10;
	if (currInput.RRight())
	{
		pos.x += dist;
	}
	if (currInput.RLeft())
	{
		pos.x -= dist;
	}
	if (currInput.RDown())
	{
		pos.y += dist;
	}
	if (currInput.RUp())
	{
		pos.y -= dist;
	}

	SetRectCenter(quads + CONTROL, 100, 100, pos + stickRel );

	float controlAngle = 0;
	
	if (currInput.leftStickMagnitude > 0)
	{
		controlAngle = -currInput.leftStickRadians;
	}

	SetRectRotation(quads + CONTROL, controlAngle, 100, 100, pos + stickRel );
	
	Vector2f actionSize(32, 32);
	Vector2f powerSize(32, 32);

	Vector2f actionSizeP = actionSize + Vector2f(4, 4);
	Vector2f powerSizeP = powerSize + Vector2f(4, 4);
	SetRectCenter(quads + (ACTION + 0) * 4, actionSize.x, actionSize.y, pos + actionRel + Vector2f( -actionSizeP.x / 2, actionSizeP.y / 2 ) );
	SetRectCenter(quads + (ACTION + 1) * 4, actionSize.x, actionSize.y, pos + actionRel + Vector2f( actionSizeP.x / 2, actionSizeP.y / 2 ) );
	SetRectCenter(quads + (ACTION + 2) * 4, actionSize.x, actionSize.y, pos + actionRel + Vector2f( 0, -actionSizeP.y / 2));

	SetRectCenter(quads + (POWER + 0) * 4, powerSize.x, powerSize.y, pos + powerRel + Vector2f( -powerSizeP.x / 2, -powerSizeP.x / 2 ));
	SetRectCenter(quads + (POWER + 1) * 4, powerSize.x, powerSize.y, pos + powerRel + Vector2f(powerSizeP.x / 2, -powerSizeP.x / 2));
	SetRectCenter(quads + (POWER + 2) * 4, powerSize.x, powerSize.y, pos + powerRel + Vector2f(powerSizeP.x / 2, powerSizeP.x / 2));
	SetRectCenter(quads + (POWER + 3) * 4, powerSize.x /2, powerSize.y, pos + powerRel + Vector2f(-powerSizeP.x / 4, powerSizeP.x / 2));
	SetRectCenter(quads + (POWER + 4) * 4, powerSize.x/2, powerSize.y, pos + powerRel + Vector2f(-powerSizeP.x / 2 - powerSizeP.x / 4, powerSizeP.x / 2));
}

void InputVisualizer::Draw(sf::RenderTarget *target)
{
	target->draw(quads, 4 * TOTALQUADS, sf::Quads);
}