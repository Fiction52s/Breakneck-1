#include "Shockwave.h"
#include <iostream>
#include "Session.h"

using namespace std;
using namespace sf;

Shockwave::Shockwave()
{
	Session *sess = Session::GetSession();

	if (!shockShader.loadFromFile("Resources/Shader/shockwave_shader.frag", sf::Shader::Fragment))
	{
		cout << "shockwave SHADER NOT LOADING CORRECTLY" << endl;
	}
	shockShader.setUniform("resolution", Vector2f(1920, 1080));
	shockShader.setUniform("texSize", Vector2f(580, 580));
	ts_shock = sess->GetTileset("FX/shockwave_580x580.png");
	shockShader.setUniform("shockwaveTex", *ts_shock->texture);

	frame = 0;
}

void Shockwave::Update(float p_zoom)
{
	zoom = p_zoom;
	shockShader.setUniform("zoom", zoom);

	++frame;
	if (frame == 60)
		frame = 0;
}

void Shockwave::SetPosition(sf::Vector2f &p_pos)
{
	pos = p_pos;
	
}

void Shockwave::Draw( sf::RenderTexture *tex, sf::RenderTarget *target)
{
	Vector2f shockSize(100, 100);//580 / 2, 580 / 2);
	sf::RectangleShape rectPost(shockSize);
	rectPost.setOrigin(rectPost.getLocalBounds().width / 2, rectPost.getLocalBounds().height / 2);
	rectPost.setPosition(pos);

	sprite.setTexture(*ts_shock->texture);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(pos);

	const sf::View &view = target->getView();

	Vector2f botLeft(view.getCenter().x - view.getSize().x / 2, view.getCenter().y + view.getSize().y);

	shockShader.setUniform("underTex", tex->getTexture());
	shockShader.setUniform("shockSize", Vector2f(580, 580));
	shockShader.setUniform("botLeft", Vector2f(rectPost.getPosition().x - rectPost.getSize().x / 2 - botLeft.x,
		rectPost.getPosition().y - rectPost.getSize().y / 2 + rectPost.getSize().y - botLeft.y));
	
	sprite.setScale((1.0 / 60.0) * frame * .4, (1.0 / 60.0) * frame * .4);
	target->draw(sprite, &shockShader);
}