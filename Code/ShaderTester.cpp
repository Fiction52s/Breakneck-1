#include "ShaderTester.h"
#include "GameSession.h"
#include "VectorMath.h"

ShaderTester::ShaderTester(ShaderType t, GameSession *owner )
{
	type = t;

	sf::FloatRect fr;
	fr.left = 0;
	fr.top = 0;
	fr.width = 1.f;
	fr.height = 1.f;

	SetRectSubRect(quad, fr);
	SetRectCenter( quad, 400, 400, sf::Vector2f( 960, 540 ));
	frame = 0;

	switch (type)
	{
		case FIRE:
		{
			Tileset *ts_perlin = owner->GetTileset("Shader/perlin01.png", 400, 400);
			Tileset *ts_grad = owner->GetTileset("Shader/gradient01.png", 400, 400);
			if (!sh.loadFromFile("Shader/fire.frag", sf::Shader::Fragment))
			{
				assert(0);
			}

			sh.setUniform("u_noiseTex", *ts_perlin->texture);
			sh.setUniform("u_gradTex", *ts_grad->texture);
			break;
		}
	}
}

void ShaderTester::Update()
{
	switch (type)
	{
	case FIRE:
		if (frame == 180)
		{
			frame = 0;
		}
		break;
	}


	switch (type)
	{
	case FIRE:
	{
		float ff = frame / 180.f;
		sh.setUniform("u_time", ff);
		break;
	}
	}

	++frame;
}

void ShaderTester::Draw( sf::RenderTarget *target )
{
	target->draw(quad, 4, sf::Quads, &sh);
}