#include "Sequence.h"

using namespace std;
using namespace sf;

FlashGroup::FlashGroup()
{
	currBGTileset = NULL;
	bg = NULL;
	inFrontOfFade = false;
}

void FlashGroup::Reset()
{
	frame = 0;
	done = false;
}

void FlashGroup::SetBG(SceneBG *p_bg)
{
	bg = p_bg;
	if (bg != NULL)
	{
		UpdateBG();
		SetRectCenter(bgQuad, currBGTileset->tileWidth, currBGTileset->tileHeight, Vector2f(960, 540));
		SetRectSubRect(bgQuad, currBGTileset->GetSubRect(0));
	}
}

void FlashGroup::UpdateBG()
{
	if (bg != NULL)
	{
		Tileset *bgTS = bg->GetCurrTileset(frame);
		if (currBGTileset != bgTS)
		{
			currBGTileset = bgTS;
		}

	}
}

void FlashGroup::DrawBG(RenderTarget *target)
{
	if (currBGTileset != NULL)
	{
		target->draw(bgQuad, 4, sf::Quads, currBGTileset->texture);
	}
}

void FlashGroup::TryCurrFlashes()
{
	if (fMap.count(frame) > 0)
	{
		auto &listRef = fMap[frame];
		for (auto it = listRef.begin(); it != listRef.end(); ++it)
		{
			(*it)->image->Flash();
		}
	}

	if (infEndMap.count(frame) > 0)
	{
		auto &listRef = infEndMap[frame];
		for (auto it = listRef.begin(); it != listRef.end(); ++it)
		{
			(*it)->image->StopHolding();
		}
	}
}

void FlashGroup::Update()
{
	if (frame == numFrames)
	{
		done = true;
	}

	if (!done)
	{
		TryCurrFlashes();

		UpdateBG();

		++frame;
	}
	//SetRectSubRect(bgQuad, bgTilesets[0]->)
	//might want to put the frame == numFrames down here, not sure yet.
}

void FlashGroup::Init()
{
	numFrames = 0;

	if (fList.size() > 0)
	{
		int counter = 0;
		for (auto it = fList.begin(); it != fList.end(); ++it)
		{
			counter += (*it)->delayedStart;
			if (counter < 0)
			{
				assert(0);
				counter = 0;
			}

			(*it)->startFrame = counter;
			fMap[counter].push_back((*it));

			if (!(*it)->simul)
			{
				if (!(*it)->image->infiniteHold)
				{
					counter += (*it)->image->GetNumFrames();
				}
			}

		}

		FlashInfo *lastFlash = fList.back();

		assert(!lastFlash->image->infiniteHold);
		numFrames = lastFlash->startFrame + lastFlash->image->GetNumFrames();

		for (auto it = fList.begin(); it != fList.end(); ++it)
		{
			if ((*it)->image->infiniteHold)
			{
				infEndMap[(numFrames - 1) - (*it)->image->infiniteHoldEarlyEnd].push_back((*it));
			}
		}
	}
}

void FlashGroup::AddSimulFlash(FlashedImage *fi, int delayedFrames)
{
	fList.push_back(new FlashInfo(fi, delayedFrames, true));
}

bool FlashGroup::IsDone()
{
	return done;
}

void FlashGroup::AddSeqFlash(FlashedImage *fi,
	int delayedStart)
{
	//delayed start by a negative amount
	fList.push_back(new FlashInfo(fi, delayedStart, false));
}

FlashGroup::~FlashGroup()
{
	for (auto it = fList.begin(); it != fList.end(); ++it)
	{
		delete (*it);
	}
}