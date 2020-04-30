#ifndef __EDITORDECORINFO_H__
#define __EDITORDECORINFO_H__

#include "ISelectable.h"

struct TransformTools;
struct EditorDecorInfo : ISelectable
{
	enum DrawMode
	{
		D_NORMAL,
		D_TRANSFORM
	};

	EditorDecorInfo(sf::Sprite &s, int lay,
		const std::string &dName, int p_tile);

	void CancelTransformation();
	DecorPtr CompleteTransformation();
	void UpdateTransformation(TransformTools *tr);
	void StartTransformation();

	bool ContainsPoint(sf::Vector2f test);
	bool Intersects(sf::IntRect rect);
	void Move(sf::Vector2i delta);
	void MoveTo(sf::Vector2i &pos);
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	void Deactivate();
	void Activate();
	void SetSelected(bool select);
	void WriteFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);

	sf::Vector2f currScale;
	float currRotate;

	DrawMode dMode;
	sf::Sprite origSpr;
	sf::Sprite spr;
	int layer;
	std::string decorName;
	int tile;
	std::list<DecorPtr> *myList;
	static bool CompareDecorInfoLayer(EditorDecorInfo &di0, EditorDecorInfo &di1);
};

#endif