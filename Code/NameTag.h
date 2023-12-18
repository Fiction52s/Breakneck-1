#ifndef __NAMETAG_H__
#define __NAMETAG_H__

#include <SFML\Graphics.hpp>
#include <string>

struct NameTag
{
	sf::Text nameText;
	sf::Vertex bgQuad[4];
	sf::Vector2f trackingPos;
	sf::Vector2f pixelPos;
	sf::Vector2f hoverDelta;

	NameTag();
	void SetName(const std::string &n);
	void UpdatePixelPos(sf::RenderTarget *target);
	void SetPos(sf::Vector2f pos);
	bool IsActive();
	void SetActive(bool a);
	void Draw(sf::RenderTarget *target);
	void MapDraw(sf::RenderTarget *target, sf::Vector2f offset, sf::Vector2f p_pixelPos);
	void SetHoverDelta(sf::Vector2f offset);

private:
	bool active;
};

#endif