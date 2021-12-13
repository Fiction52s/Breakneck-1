#include "Tileset.h"
#include "VisualEffects.h"
#include "MotionGhostEffect.h"

using namespace sf;
using namespace std;

MotionGhostEffect::MotionGhostEffect(int maxGhosts)
	:shader(NULL), ts(NULL)
{
	motionGhostBuffer = new VertexBuf(maxGhosts, sf::Quads);
	oldPositions.resize(maxGhosts);
}

MotionGhostEffect::~MotionGhostEffect()
{
	delete motionGhostBuffer;
}

void MotionGhostEffect::SetDistInBetween(float dist)
{
	distInBetween = dist;
}

void MotionGhostEffect::SetSpread(int p_numGhosts, Vector2f &p_dir, float p_angle)
{
	motionGhostBuffer->SetNumActiveMembers(p_numGhosts);
	dir = p_dir;
	angle = p_angle;
}

void MotionGhostEffect::SetRootPos(Vector2f &pos)
{
	rootPos = pos;
}

void MotionGhostEffect::SetColorGradient(sf::Color &c0,
	sf::Color &c1, CubicBezier &bez)
{
	rootColor = c0;
	tailColor = c1;
	colorBez = bez;
}

void MotionGhostEffect::SetColor(sf::Color &c)
{
	rootColor = c;
	tailColor = c;
	colorBez = CubicBezier();
}

void MotionGhostEffect::ApplyUpdates()
{
	int numActiveMembers = motionGhostBuffer->numActiveMembers;



	for (int i = 0; i < numActiveMembers; ++i)
	{
		motionGhostBuffer->SetRotation(i, angle);
		motionGhostBuffer->SetTile(i, tileIndex);
	}

	//int showMotionGhosts = min((int)round(motionMagnitude), motionGhostBuffer->numMembers );
	//motionGhostBuffer->SetNumActiveMembers(showMotionGhosts);

	Vector2f motionNormal(dir.y, -dir.x);


	//float scaleAmountDown = scaleBez.GetValue(pGhosts) * maxScaleDown;

	Vector2f tempPos;
	float pGhosts;
	float vibrateAmount;
	int iVibrateAmount;
	float scaleAmountUp;
	Color currColor;
	int cr, cg, cb, ca;
	float ta;
	float recip;
	float vGhosts;
	float sGhosts;
	for (int i = 0; i < numActiveMembers; ++i)
	{
		if (numActiveMembers == 1)
		{
			pGhosts = 0;
			vGhosts = 0;
			sGhosts = 0;
		}
		else
		{
			pGhosts = i / (float)(numActiveMembers - 1);
			if (i > startVibrateGhost)
			{
				vGhosts = i / (float)(numActiveMembers - startVibrateGhost - 1);
			}
			else
			{
				vGhosts = 0;
			}

			if (i > startScaleGhost)
			{
				sGhosts = i / (float)(numActiveMembers - startScaleGhost - 1);
			}
			else
			{
				sGhosts = 0;
			}



		}


		ta = colorBez.GetValue(pGhosts);
		recip = 1.f - ta;
		cr = round(rootColor.r * recip + tailColor.r * ta);
		cr = max(cr, 0);
		cr = min(cr, 255);

		cg = round(rootColor.g * recip + tailColor.g * ta);
		cg = max(cg, 0);
		cg = min(cg, 255);

		cb = round(rootColor.b * recip + tailColor.b * ta);
		cb = max(cb, 0);
		cb = min(cb, 255);

		ca = round(rootColor.a * recip + tailColor.a * ta);
		ca = max(ca, 0);
		ca = min(ca, 255);

		vibrateAmount = vibrateBez.GetValue(vGhosts) * maxVibrate;

		iVibrateAmount = round(vibrateAmount);
		if (iVibrateAmount % 2 == 1)
			iVibrateAmount++;

		scaleAmountUp = scaleBez.GetValue(sGhosts) * maxScaleUp;


		tempPos = Vector2f(rootPos.x + dir.x * (i * distInBetween), rootPos.y + dir.y * (i * distInBetween));
		if (iVibrateAmount != 0)
			tempPos += Vector2f(motionNormal * (float)(rand() % iVibrateAmount - iVibrateAmount / 2));
		//motionGhosts[i].setPosition( tempPos );

		/*int tf = 10;
		Vector2f ff(rand() % tf - tf / 2, rand() % tf - tf / 2);
		Vector2f ff1(rand() % tf - tf / 2, rand() % tf - tf / 2);*/
		motionGhostBuffer->SetColor(i, sf::Color(cr, cg, cb, ca));
		motionGhostBuffer->SetPosition(i, tempPos);
		//motionGhostBufferBlue->SetPosition(i, tempPos + ff);
		//motionGhostBufferPurple->SetPosition(i, tempPos + ff1);

		float x = 1.f;

		if (!facingRight)//|| (facingRight && reversed) )
			x = -x;

		if (reversed)
			x = -x;
		float y = 1.f;

		//if (reversed)
		//	y = -y;

		//motionGhostBuffer->SetScale(i, Vector2f(x, y));

		if (i >= startScaleGhost)
		{
			//float blah = ((rand() % testq) - testq / 2) / ((float)testq / 2);
			float blah = ((rand() % 100)) / 100.f;
			blah /= 4.f;//2.f;
						//blah = -blah;
						//motionGhosts[i].setScale( Vector2f(1.f + blah, 1.f + blah));
			motionGhostBuffer->SetScale(i, Vector2f(x + blah, y + blah));
			/*motionGhostBufferBlue->SetScale(i, Vector2f(x + blah, y + blah));
			motionGhostBufferPurple->SetScale(i, Vector2f(x + blah, y + blah));*/
		}
		else
		{
			motionGhostBuffer->SetScale(i, Vector2f(x, y));
			/*motionGhostBufferBlue->SetScale(i, Vector2f(x, y));
			motionGhostBufferPurple->SetScale(i, Vector2f(x, y));*/
		}
	}
	motionGhostBuffer->UpdateVertices();
}

void MotionGhostEffect::SetVibrateParams(CubicBezier &vBez, float p_maxVibrate, int startGhost)
{
	vibrateBez = vBez;
	maxVibrate = p_maxVibrate;
	startVibrateGhost = startGhost;
}

void MotionGhostEffect::SetScaleParams(CubicBezier &sBez, float p_maxScaleUp, float p_maxScaleDown, int p_startGhost)
{
	scaleBez = sBez;
	maxScaleUp = p_maxScaleUp;
	maxScaleDown = p_maxScaleDown;
	startScaleGhost = p_startGhost;
}

void MotionGhostEffect::SetFacing(bool p_facingRight, bool p_reversed)
{
	facingRight = p_facingRight;
	reversed = p_reversed;
}


void MotionGhostEffect::Draw(sf::RenderTarget *target)
{
	if (shader != NULL)
	{
		motionGhostBuffer->Draw(target, shader);
	}
	else
	{
		motionGhostBuffer->Draw(target);
	}
}

void MotionGhostEffect::SetShader(sf::Shader *pShad)
{
	shader = pShad;
}

void MotionGhostEffect::SetTileset(Tileset *p_ts)
{
	ts = p_ts;
	motionGhostBuffer->ts = p_ts;
}
void MotionGhostEffect::SetTile(int p_tileIndex)
{
	tileIndex = p_tileIndex;
}