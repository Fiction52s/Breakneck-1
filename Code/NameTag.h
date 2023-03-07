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

	NameTag();
	void SetName(const std::string &n);
	void UpdatePixelPos(sf::RenderTarget *target);
	void SetPos(sf::Vector2f pos);
	bool IsActive();
	void SetActive(bool a);
	void Draw(sf::RenderTarget *target);

private:
	bool active;
};

#endif