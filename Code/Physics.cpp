#include "Physics.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>
#include "GameSession.h"
#include <cmath>

using namespace sf;
using namespace std;
using json = nlohmann::json;


bool HitboxInfo::IsAirType(HitPosType hpt)
{
	return hpt == AIRDOWN || hpt == AIRDOWNFORWARD || hpt == AIRFORWARD
		|| hpt == AIRUPFORWARD || hpt == AIRUP;
}

void HitboxInfo::SetupHitboxLevelInfo(
	json &j, HitboxInfo &hi)
{
	hi.damage = j["damage"];
	hi.hitlagFrames = j["hitlag"];
	hi.hitstunFrames = j["hitstun"];
	hi.knockback = j["knockback"];
	double kbAngle = j["knockbackangle"];
	kbAngle = kbAngle / 180.0 * PI;
	hi.kbDir = V2d(cos(kbAngle), -sin(kbAngle));
	hi.invincibleFrames = j["invincibleframes"];
	double gravMult = j["gravmultiplier"];
	hi.gravMultiplier = gravMult;
	hi.extraDefenderHitlag = j["extradefenderhitlag"];
	hi.hitBlockCancelDelay = j["hitblockcanceldelay"];

	string posTypeStr = j["postype"];

	if (posTypeStr == "airup")
	{
		hi.hitPosType = HitboxInfo::HitPosType::AIRUP;
	}
	else if (posTypeStr == "airupforward")
	{
		hi.hitPosType = HitboxInfo::HitPosType::AIRUPFORWARD;
	}
	else if (posTypeStr == "airforward")
	{
		hi.hitPosType = HitboxInfo::HitPosType::AIRFORWARD;
	}
	else if (posTypeStr == "airdownforward")
	{
		hi.hitPosType = HitboxInfo::HitPosType::AIRDOWNFORWARD;
	}
	else if (posTypeStr == "airdown")
	{
		hi.hitPosType = HitboxInfo::HitPosType::AIRDOWN;
	}
	else if (posTypeStr == "ground")
	{
		hi.hitPosType = HitboxInfo::HitPosType::GROUND;
	}
	else if (posTypeStr == "groundlow")
	{
		hi.hitPosType = HitboxInfo::HitPosType::GROUNDLOW;
	}
	else if (posTypeStr == "groundhigh")
	{
		hi.hitPosType = HitboxInfo::HitPosType::GROUNDHIGH;
	}
	else if (posTypeStr == "omni")
	{
		hi.hitPosType = HitboxInfo::HitPosType::OMNI;
	}
	else
	{
		cout << "postypestr is wrong:" << posTypeStr << endl;
		assert(0);
	}

}

V2d HitboxInfo::GetKnockbackDir()
{
	V2d kb = kbDir;
	if (flipHorizontalKB)
	{
		kb.x = -kb.x;
	}
	if (flipVerticalKB)
	{
		kb.y = -kb.y;
	}

	return kb;
}

V2d HitboxInfo::GetKnockbackVector()
{
	return GetKnockbackDir() * knockback;
}

HitboxInfo::HitPosType HitboxInfo::GetAirType(V2d &vel)
{
	assert(vel.x != 0.0 || vel.y != 0.0); //stationary bullet?

	V2d dir = normalize(vel);

	/*double angle = GetVectorAngleCCW(dir);

	cout << "angle: " << angle << endl;

	double division = PI / 8;

	if (angle >= 2 * PI - division || angle < division)
	{
		return AIRFORWARD;
	}
	else if (angle >= division && angle < division * 3)
	{
		return AIRUPFORWARD;
	}
	else if( angle >= division * 3 && angle < )*/


	double primaryThresh = .2;

	if (abs(dir.x) < primaryThresh)
	{
		if (dir.y < 0)
		{
			return AIRUP;
		}
		else
		{
			return AIRDOWN;
		}
	}
	else if (abs(dir.y) < primaryThresh)
	{
		return AIRFORWARD;
	}
	else
	{
		if (dir.y < 0)
		{
			return AIRUPFORWARD;
		}
		else if( dir.y > 0 )
		{
			return AIRDOWNFORWARD;
		}
		else
		{
			cout << " air hit dir: " << dir.x << ", " << dir.y << endl;
			assert(0); //shouldn't happen
			return AIRFORWARD;
		}
	}


	/*bool forward = false;
	if (abs( dir.x ) > .2 )
	{
		forward = true;
	}

	if (dir.y < 0)
	{
		if (forward)
		{
			return AIRUPFORWARD;
		}
		else
		{
			return AIRUP;
		}
	}
	else if (dir.y > 0)
	{
		if (forward)
		{
			return AIRDOWNFORWARD;
		}
		else
		{
			return AIRDOWN;
		}
	}
	else
	{
		return AIRFORWARD;
	}*/
}

//EDGE FUNCTIONS
Edge::Edge()
{
	edge0 = NULL;
	edge1 = NULL;
	edgeType = EdgeType::REGULAR;
	poly = NULL;
	rail = NULL;
}

V2d Edge::Normal()
{
	V2d temp = Along();
	return V2d( temp.y, -temp.x );
}

V2d Edge::Along()
{
	return normalize(v1 - v0);
}

sf::Vector2<double> Edge::FullAlong()
{
	return v1 - v0;
}

V2d Edge::GetPosition( double quantity )
{
	return v0 + quantity * Along();
}

V2d Edge::GetReflectionDir(V2d &dir)
{
	V2d norm = Normal();
	double reflX = cross(-dir, norm);
	double reflY = dot(-dir, norm);
	V2d edgeDir = Along();

	return normalize(reflX * edgeDir + reflY * norm);
}

double Edge::GetQuantity(Vector2<double> &p)
{
	//projects the origin of the line to p onto the edge. if the point is on the edge it will just be 
	//normal to use dot product to get cos(0) =1
	V2d vv = p - v0;
	V2d e = normalize(v1 - v0);
	double result = dot( vv, e );
	double len = length( v1 - v0 );
	if( approxEquals( result, 0 ) )
		return 0;
	else if( approxEquals( result, length( v1 - v0 ) ) )
		return len;
	else
		return result;
}


sf::Vector2<double> Edge::GetRaisedPosition(double quantity, double height)
{
	return GetPosition(quantity) + Normal() * height;
}

double Edge::GetRawQuantity(sf::Vector2<double> &p)
{
	return dot(p - v0, Along());
}

double Edge::GetNormalAngleRadians()
{
	V2d diff = v1 - v0;
	//atan2(testDiff.y, testDiff.x) / PI * 180;
	return atan2(diff.y, diff.x);
}

double Edge::GetNormalAngleDegrees()
{
	return GetNormalAngleRadians() / PI * 180.0;
}

double Edge::GetQuantityGivenX( double x )
{

	V2d e = normalize(v1 - v0);
	double deltax = x - v0.x;
	double factor = deltax / e.y;

	return 0;
}

double Edge::GetDistAlongNormal(sf::Vector2<double> &p)
{
	return (cross(p - v0, Along()));
}

double Edge::GetLength()
{
	return length(v1 - v0);
}

double Edge::GetLengthSqr()
{
	return lengthSqr(v1 - v0);
}

void Edge::CalcAABB()
{
	double left = min(v0.x, v1.x);
	double right = max(v0.x, v1.x);
	double top = min(v0.y, v1.y);
	double bottom = max(v0.y, v1.y);
	aabb = sf::Rect<double>(left, top, right - left, bottom - top);
}

bool Edge::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsEdgeTouchingBox( this, r );
}

bool Edge::IsInvisibleWall()
{
	return edgeType == BORDER || edgeType == BARRIER;
}

sf::Vector2<double> CollisionBox::GetQuadVertex(int index)
{
	double w = 0;
	double h = 0;

	switch (index)
	{
	case 0:
		w = -rw;
		h = -rh;
		break;
	case 1:
		w = rw;
		h = -rh;
		break;
	case 2:
		w = rw;
		h = rh;
		break;
	case 3:
		w = -rw;
		h = rh;
		break;
	}
	assert(w != 0 && h != 0);
	//RotateCCW

	V2d localPos(w, h);
	if (flipHorizontal)
	{
		RotateCCW(localPos, localAngle);
	}
	else
	{
		RotateCW(localPos, localAngle);
	}

	V2d off = GetOffset();

	localPos += off;

	if (flipHorizontal)
	{
		RotateCW(localPos, globalAngle);
	}
	else
	{
		RotateCW(localPos, globalAngle);
	}


	return globalPosition + localPos;
	//return (globalPosition + V2d(w * cos(globalAngle) + h * -sin(globalAngle), w * sin(globalAngle) + h * cos(globalAngle)));
}

void CollisionBox::Scale(double factor)
{
	rw *= factor;
	rh *= factor;
}

void CollisionBox::Move(V2d &move)
{
	globalPosition += move;
}

void CollisionBox::SetRectDir(V2d &dir, double alongSize,
	double otherSize)
{
	isCircle = false;

	double angle = atan2(-dir.y, -dir.x);

	rw = alongSize / 2.0;
	rh = otherSize / 2.0;

	localAngle = angle;
}

V2d CollisionBox::GetOffset()
{
	V2d realOffset = offset;
	if (flipHorizontal)
		realOffset.x = -realOffset.x;
	if (flipVertical)
		realOffset.y = -realOffset.y;

	return realOffset;
}

V2d CollisionBox::GetTrueCenter()
{
	V2d off = GetOffset();
	RotateCW(off, globalAngle);
	return globalPosition + off;
}

bool CollisionBox::Intersects(CollisionBox &c)
{
	//first, box with box aabb. can adjust it later
	if (c.isCircle && this->isCircle)
	{
		V2d otherCenter = c.GetTrueCenter();
		V2d center = GetTrueCenter();
		double dist = length(center - otherCenter);

		if (dist <= this->rw + c.rw)
		{
			if (this->isRing == c.isRing)
			{
				return true;
			}
			else if (this->isRing && dist > innerRadius - c.rw)
			{
				return true;
			}
			else if (c.isRing && dist > c.innerRadius - rw)
			{
				return true;
			}

		}

	}
	else if (c.isCircle && !this->isCircle)
	{
		V2d cCenterPos = c.GetTrueCenter();
		V2d pA = GetQuadVertex(0);
		V2d pB = GetQuadVertex(1);
		V2d pC = GetQuadVertex(2);
		V2d pD = GetQuadVertex(3);
		bool intersect = (QuadContainsPoint(pA, pB, pC, pD, cCenterPos)
			|| IsEdgeTouchingCircle(pA, pB, cCenterPos, c.rw)
			|| IsEdgeTouchingCircle(pB, pC, cCenterPos, c.rw)
			|| IsEdgeTouchingCircle(pC, pD, cCenterPos, c.rw)
			|| IsEdgeTouchingCircle(pD, pA, cCenterPos, c.rw));

		if (c.isRing && intersect)
		{
			if (length(pA - cCenterPos) < c.innerRadius
				&& length(pB - cCenterPos) < c.innerRadius
				&& length(pC - cCenterPos) < c.innerRadius
				&& length(pD - cCenterPos) < c.innerRadius )
				{
					return false;
				}
		}

		return intersect;
	}
	else if( !c.isCircle && this->isCircle )
	{
		V2d cCenterPos = GetTrueCenter();
		V2d pA = c.GetQuadVertex(0);
		V2d pB = c.GetQuadVertex(1);
		V2d pC = c.GetQuadVertex(2);
		V2d pD = c.GetQuadVertex(3);
		bool intersect = (QuadContainsPoint(pA, pB, pC, pD, cCenterPos)
			|| IsEdgeTouchingCircle(pA, pB, cCenterPos, rw)
			|| IsEdgeTouchingCircle(pB, pC, cCenterPos, rw)
			|| IsEdgeTouchingCircle(pC, pD, cCenterPos, rw)
			|| IsEdgeTouchingCircle(pD, pA, cCenterPos, rw));

		if (isRing && intersect)
		{
			if (length(pA - cCenterPos) < innerRadius
				&& length(pB - cCenterPos) < innerRadius
				&& length(pC - cCenterPos) < innerRadius
				&& length(pD - cCenterPos) < innerRadius)
			{
				return false;
			}
		}

		return intersect;
	}
	else //both are boxes
	{
		/*Transform test;
		test.rotate( globalAngle / PI * 180 );
		Vector2f transA = test.transformPoint( Vector2f( -rw, -rh ) );
		Vector2f transB = test.transformPoint( Vector2f( rw, -rh ) );
		Vector2f transC = test.transformPoint( Vector2f( rw, rh ) );
		Vector2f transD = test.transformPoint( Vector2f( -rw, rh ) );

		V2d pA0( transA.x, transA.y );
		V2d pB0( transB.x, transB.y );
		V2d pC0( transC.x, transC.y );
		V2d pD0( transD.x, transD.y );

		test = Transform::Identity;
		test.rotate( c.globalAngle / PI * 180 );

		transA = test.transformPoint( Vector2f( -c.rw, -c.rh ) );
		transB = test.transformPoint( Vector2f( c.rw, -c.rh ) );
		transC = test.transformPoint( Vector2f( c.rw, c.rh ) );
		transD = test.transformPoint( Vector2f( -c.rw, c.rh ) );

		V2d pA1( transA.x, transA.y );
		V2d pB1( transB.x, transB.y );
		V2d pC1( transC.x, transC.y );
		V2d pD1( transD.x, transD.y );*/
		//V2d pA0 = globalPosition + test.transformPoint( 
		V2d A0 = GetQuadVertex(0);//globalPosition + V2d( -rw * cos( globalAngle ) + -rh * -sin( globalAngle ), -rw * sin( globalAngle ) + -rh * cos( globalAngle ) );
		V2d B0 = GetQuadVertex(1);//globalPosition + V2d( rw * cos( globalAngle ) + -rh * -sin( globalAngle ), rw * sin( globalAngle ) + -rh * cos( globalAngle ) );
		V2d C0 = GetQuadVertex(2);//globalPosition + V2d( rw * cos( globalAngle ) + rh * -sin( globalAngle ), rw * sin( globalAngle ) + rh * cos( globalAngle ) );
		V2d D0 = GetQuadVertex(3);//globalPosition + V2d( -rw * cos( globalAngle ) + rh * -sin( globalAngle ), -rw * sin( globalAngle ) + rh * cos( globalAngle ) );

		//cout << "rw: " << rw << ", rh: " << rh << ", axis: " << length( pB0 - pA0 ) << ", axis2: " << length( pD0 - pA0 ) << endl;
		

		V2d A1 = c.GetQuadVertex(0);//c.globalPosition + V2d( -c.rw * cos( c.globalAngle ) + -c.rh * -sin( c.globalAngle ), -c.rw * sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		V2d B1 = c.GetQuadVertex(1);//c.globalPosition + V2d( c.rw * cos( c.globalAngle ) + -c.rh * -sin( c.globalAngle ), c.rw * sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		V2d C1 = c.GetQuadVertex(2);//c.globalPosition + V2d( c.rw * cos( c.globalAngle ) + c.rh * -sin( c.globalAngle ), c.rw * sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );
		V2d D1 = c.GetQuadVertex(3);//c.globalPosition + V2d( -c.rw * cos( c.globalAngle ) + c.rh * -sin( c.globalAngle ), -c.rw * sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );

		//cout << "c.rw: " << c.rw << ", c.rh: " << c.rh << endl;

		/*bool touching = isQuadTouchingQuad( pA0, pB0, pC0, pD0, pA1, pB1, pC1, pD1 );*/
		double AB = length( B0 - A0 );
		double AD = length( D0 - A0 );

		V2d realAB = B0 - A0;
		V2d realAD = D0 - A0;

		V2d normalizeAB = normalize( B0 - A0 );
		V2d normalizeAD = normalize( D0 - A0 );
	

		double min1AB = min( dot( A1 - A0, normalizeAB ), min( dot( B1 - A0, normalizeAB ), min( dot( C1 - A0, normalizeAB ),
			dot( D1 - A0, normalizeAB ) ) ) );
		double max1AB = max( dot( A1 - A0, normalizeAB ), max( dot( B1 - A0, normalizeAB ), max( dot( C1 - A0, normalizeAB ),
			dot( D1 - A0, normalizeAB ) ) ) );

		double min1AD = min( dot( A1 - A0, normalizeAD ), min( dot( B1 - A0, normalizeAD ), min( dot( C1 - A0, normalizeAD ),
			dot( D1 - A0, normalizeAD ) ) ) );
		double max1AD = max( dot( A1 - A0, normalizeAD ), max( dot( B1 - A0, normalizeAD ), max( dot( C1 - A0, normalizeAD ),
			dot( D1 - A0, normalizeAD ) ) ) );

		double blaha = dot( A1 - A0, normalizeAD );
		double blahb = dot( B1 - A0, normalizeAD );
		double blahc = dot( C1 - A0, normalizeAD );
		double blahd = dot( D1 - A0, normalizeAD );

	
		double AB1 = length( B1 - A1 );
		double AD1 = length( D1 - A1 );

		V2d normalizeAB1 = normalize( B1 - A1 );
		V2d normalizeAD1 = normalize( D1 - A1 );

		double min0AB = min( dot( A0 - A1, normalizeAB1 ), min( dot( B0 - A1, normalizeAB1 ), min( dot( C0 - A1, normalizeAB1 ),
			dot( D0 - A1, normalizeAB1 ) ) ) );
		double max0AB = max( dot( A0 - A1, normalizeAB1 ), max( dot( B0 - A1, normalizeAB1 ), max( dot( C0 - A1, normalizeAB1 ),
			dot( D0 - A1, normalizeAB1 ) ) ) );

		double min0AD = min( dot( A0 - A1, normalizeAD1 ), min( dot( B0 - A1, normalizeAD1 ), min( dot( C0 - A1, normalizeAD1 ),
			dot( D0 - A1, normalizeAD1 ) ) ) );
		double max0AD = max( dot( A0 - A1, normalizeAD1 ), max( dot( B0 - A1, normalizeAD1 ), max( dot( C0 - A1, normalizeAD1 ),
			dot( D0 - A1, normalizeAD1 ) ) ) );

		

		

		bool a = min1AB <= AB;
		bool b = max1AB >= 0;
		bool cc = min1AD <= AD;
		bool d = max1AD >= 0;
		bool e = min0AB <= AB1;
		bool f = max0AB >= 0;
		bool g = min0AD <= AD1;
		bool h = max0AD >= 0;


		bool touching;
		if( a && b && cc && d && e && f && g && h )
		{
			//cout << "blaha: " << blaha << ", b: " << blahb << ", c: " << blahc << ", d: " << blahd << endl;
			//cout << "A0: " << A0.x << ", " << A0.y << ", B1: " << B1.x << ", " << B1.y << endl;
			//cout << "realAD: " << realAD.x << ", " << realAD.y << endl;
			////cout << "center dist: " << length( globalPosition - c.globalPosition ) << endl;
			//cout << "normad: " << normalizeAD.x << ", " << normalizeAD.y << endl;
			//cout << "AB: " << AB << ", AD: " << AD << ", AB1: " << AB1 << ", AD1: " << AD1 << endl;
			//cout << "globalPosition: " << globalPosition.x << ", " << globalPosition.y << endl;
			//cout << "min1AB: " << min1AB << ", max1AB: " << max1AB << ", min1AD: " << min1AD << ", max1AD: " << max1AD << ", min0AB: " << min0AB <<
			//	", max0AB: " << max0AB << ", min0AD: " << min0AD << ", max0AD: " << max0AD << endl;
			//return true;
			touching = true;
		}
		else
		{
			//return false;
			touching = false;
		}
		
		/*if( touching )
		{
			cout << "TOUCHING" << endl;
		}
		else
		{
			cout << "not " << endl;
		}*/
		return touching;

		//V2d axisAB = normalize( pB0 - pA0 );
		//double maxLen = length( pB0 - pA0 );
		


		//finish this up!

	}
	return false;
}

void CollisionBox::DebugDraw( CollisionBox::BoxType bType, sf::RenderTarget *target )
{
	if( isCircle )
	{
		CircleShape cs;

		if(bType == Physics )
		{
			cs.setFillColor( Color( 255, 0, 0, 100 ) );
		}
		else if(bType == Hit )
		{
			cs.setFillColor( Color( 0, 255, 0, 100 ) );
		}
		else if(bType == Hurt )
		{
			cs.setFillColor( Color( 0, 0, 255, 100 ) );
		}

		V2d off = GetOffset();

		cs.setRadius( rw );
		cs.setOrigin( cs.getLocalBounds().width / 2 - off.x, cs.getLocalBounds().height / 2 - off.y );
		cs.setRotation(globalAngle / PI * 180);
		cs.setPosition( globalPosition.x, globalPosition.y );

		target->draw( cs );

		if (isRing)
		{
			cs.setFillColor(Color(0, 0, 0, 100));
			cs.setRadius(innerRadius);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			target->draw(cs);
		}

	}
	else
	{
		//cout << "Angle?: " << angle << endl;
		sf::RectangleShape r;
		Color col;
		if(bType == Physics )
		{
			col = Color(255, 0, 0, 100);
		}
		else if(bType == Hit )
		{
			col = Color(0, 255, 0, 100);
		}
		else if(bType == Hurt )
		{
			col = Color(0, 0, 255, 100);
		}

		sf::Vertex quad[4];
		for (int i = 0; i < 4; ++i)
		{
			quad[i].position = Vector2f(GetQuadVertex(i));
			quad[i].color = col;
		}
		
		

		/*r.setSize( sf::Vector2f( rw * 2, rh * 2 ) );
		r.setOrigin( r.getLocalBounds().width / 2, r.getLocalBounds().height / 2 );
		r.setRotation( angle / PI * 180 );
		r.setPosition( globalPosition.x, globalPosition.y );*/


		target->draw( quad, 4, sf::Quads );

	}
}

//CONTACT FUNCTIONS
Contact::Contact()
	:edge( NULL )
{
	collisionPriority = 0;
}

Collider::Collider()
	:currentContact(NULL)
{
	currentContact = new Contact;
}

Collider::~Collider()
{
	delete currentContact;
}

Contact *Collider::collideEdge( V2d position, const CollisionBox &b, Edge *e, const V2d &vel, const V2d &tVel )
{
	if( b.isCircle )
	{
		double pointMinTime = 100;

		V2d oldPosition = position - vel;

		V2d v0 = e->v0;
		V2d v1 = e->v1;

		double edgeLength = length( v1 - v0 );
		double radius = b.rw;
		V2d edgeNormal = e->Normal();

		double lineQuantity = dot( position - v0, normalize( v1 - v0 ) );
		double dist = cross(position - v0, normalize( v1 - v0 ) );
		double testD = dot( -vel, edgeNormal );
		bool d = testD > 0;

		//if( d && length( v0 - position ) <= radius )
		//{
		//	V2d pointDir = normalize( v0 - position );
		//	V2d velDir = normalize( vel );
		//	bool hit = dot( vel, pointDir ) <= length( v0 - position );
		//	if( hit )
		//	{
		//		currentContact->position = v0;
		//		currentContact->edge = e;
		//		currentContact->normal = V2d( 0, 0 );
		//		currentContact->collisionPriority = 0;
		//		currentContact->resolution = 
		//		/*CircleShape *cs = new CircleShape;
		//		cs->setFillColor( Color::Cyan );
		//		cs->setRadius( 10 );
		//		cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
		//		cs->setPosition( v0.x, v0.y );

		//		progressDraw.push_back( cs );*/
		//		cout << "first one:" << endl;
		//		return currentContact;
		//	}
		//}
		//else?
		//if( d && dist >= 0 && dist <= radius )
		if (d && dist <= radius && dist > -radius) //changed when i did timestep change. hope everything is fine
		{

			//recently added the dist >= 0 to make sure jumping off of a closed gate didn't trigger the opposite gate edge
			//I think this should still cover the case, it might fail on a border case. need to check.
			/*if (dist < 0)
			{
				cout << "DIST LESS THAN 0 YOU KNOWWWW" << endl;
			}*/
			if( lineQuantity >= 0 && lineQuantity <= edgeLength ) //point is on the circle in the dir of the ege normal
			{
				LineIntersection li;
				lineIntersection( li, oldPosition + radius * -edgeNormal, position
					+ radius * -edgeNormal, e->v0, e->v1 );


				//double testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
				if( li.parallel )//|| abs( testing ) == 1 )
				{
					cout << "returning circle null1" << endl;
					return NULL;
				}


				Vector2<double> intersect = li.position;


				//double intersectQuantity = e->GetQuantity( intersect );


				V2d newPosition = intersect + radius * edgeNormal;

				currentContact->resolution = newPosition - position;
				currentContact->edge = e;
				currentContact->normal= edgeNormal;
				currentContact->position = e->GetPosition( lineQuantity );
				currentContact->collisionPriority = length( intersect - ( oldPosition + radius * -edgeNormal ) );

				/*CircleShape *cs = new CircleShape;
				cs->setFillColor( Color::Cyan );
				cs->setRadius( 10 );
				cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
				cs->setPosition( intersect.x, intersect.y );

				progressDraw.push_back( cs );*/
				//cout << "success dot: " << ( dot( -vel, edgeNormal ) > 0 ) << ", dist: " << dist << ", radius: " << radius << endl;
				//cout << "second one" << endl;
				return currentContact;
			}
			else //special side/hit case for colliding with points
			{
				Edge *next = e->edge1;
				V2d along = normalize( e->v1 - e->v0 );
				V2d nextAlong = normalize( next->v1 - next->v0 );
				double c = cross( nextAlong, along );

				//probably correct but idk if i even need it
				if( c < 0 )
				if( lineQuantity < 0 && lineQuantity > -radius )// && xx >= pr ) //v0
				{
					V2d coll = e->v0 - position;// - e->v0;// - position;
					//cout << "coll: " << coll.x << ", " << coll.y << endl;
					V2d nVel = normalize( vel );
					double yy = cross( coll, nVel );
					double xx = sqrt( radius * radius - yy * yy );
					double pr = dot( coll, nVel );

					//changed this so pr >= 0 but I'm not entirely sure why. I
					//don't understand why my code isn't working T_T
					if( xx >= pr && pr >= 0 )
					{
					//V2d coll = e->v0 - position;
					//cout << "bb" << endl;
					//dist is dot
					double extra = -lineQuantity;
					double y = sqrt( radius * radius - extra * extra );
					//cout << "A" << endl;
					V2d nVel = normalize( vel );
					double lenVel = length( vel );
					
					V2d proj = (xx-pr) * nVel;
					proj = -proj;

					//V2d negvNorm = normalize( -vel );
					currentContact->resolution = proj;//(lenVel - y ) * -nVel;
					currentContact->edge = e;
					currentContact->normal = V2d( 0, 0 );
					currentContact->position = e->v0;
					currentContact->collisionPriority = length( currentContact->resolution ) / length( vel );
					//cout << "third one: " << proj.x << ", " << proj.y << endl;
					return currentContact;
					}
				}
				else if( lineQuantity <= edgeLength + radius )// && xx >= pr )//v1
				{
					V2d coll = e->v1 - position;
					//cout << "coll: " << coll.x << ", " << coll.y << endl;
					V2d nVel = normalize( vel );
					double yy = cross( coll, nVel );
					double xx = sqrt( radius * radius - yy * yy );
					double pr = dot( coll, nVel );

					//pr >= 0 is new. check above comments
					if( xx >= pr && pr >= 0 )
					{
				//	cout << "xx: " << xx << ", pr: " << pr << endl;
					V2d proj = (xx-pr) * nVel;
					proj = -proj;
					//dist is dot
					double extra = lineQuantity - edgeLength;
					double y = sqrt( radius * radius - extra * extra );
					
					double lenVel = length( vel );

					//cout << "ressss: " << proj.x << ", " << proj.y << endl;
					//cout << "B extra: " << extra << ", " << lenVel << ", y: " << y << endl;
					//V2d negvNorm = normalize( -vel );
					currentContact->resolution = proj;//(lenVel - y ) * -nVel;
					currentContact->edge = e;
					currentContact->normal = V2d( 0, 0 );
					currentContact->position = e->v1;
					currentContact->collisionPriority = length( currentContact->resolution ) / length( vel );
					//cout << "fourth one: " << proj.x << ", " << proj.y << endl;
					return currentContact;
					}
				}
				//cout << "UNDONE CASE" << endl;
				//use right triangeles from the vertex to the circle point and cross product to figure out the y. then use
				//radius and the y to find the x value which is the value along the velocity that you should go until you
				//collide. thats how u get resolution here and other stuff. don't need it for this build so do it later
				//if( 
				return NULL;
			}
		}
		//cout << "dot: " << ( dot( -vel, edgeNormal ) > 0 ) << ", dist: " << dist << ", radius: " << radius << endl;
		return NULL;
	}
	else
	{
		//position += V2d(0, -10);
		Vector2<double> oldPosition = position - vel;
		double left = position.x - b.rw;
		double right = position.x + b.rw;
		double top = position.y - b.rh;
		double bottom = position.y + b.rh;

	

		double oldLeft = oldPosition.x - b.rw;
		double oldRight = oldPosition.x + b.rw;
		double oldTop = oldPosition.y - b.rh;
		double oldBottom = oldPosition.y + b.rh;


		double edgeLeft = min( e->v0.x, e->v1.x );
		double edgeRight = max( e->v0.x, e->v1.x ); 
		double edgeTop = min( e->v0.y, e->v1.y ); 
		double edgeBottom = max( e->v0.y, e->v1.y ); 

		V2d en = e->Normal();
		V2d prevEn;
		V2d prevFullAlong;
		V2d prevReverseAlong;
		V2d nextFullAlong;
		if (e->edge0 != NULL)
		{
			prevEn = e->edge0->Normal();
			prevReverseAlong = normalize(e->edge0->v0 - e->v0);	
			prevFullAlong = e->v0 - e->edge0->v0;
			
		}
		else
		{
			prevEn = en;
			prevReverseAlong = -e->Along();
			prevFullAlong = e->FullAlong();
		}

		if (e->edge1 != NULL)
		{
			nextFullAlong = e->edge1->FullAlong();
		}
		else
		{
			nextFullAlong = e->FullAlong();
		}
		V2d point = e->v0;
		//V2d v1 = e->v1;
		//check for point collisions first

		//bool pointInRect = point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;		

		//hopefully will catch any rounding errors
		double ex = .1;//.001;
		bool pointInRect = point.x >= min( left, oldLeft ) - ex  && point.x <= max( right, oldRight ) + ex && point.y >= min( top, oldTop ) - ex && point.y <= max( bottom, oldBottom ) + ex;		


		double unPoint = cross(e->Along(), prevReverseAlong);

		double leftTime = 1, rightTime = 1, bottomTime = 1, topTime = 1; // one whole timestep
		
		V2d intersect;

		double pointMinTime = 100;
		int type = 0;
		V2d pointNormal(0,0);
		
		if( pointInRect && unPoint > -.0001 )
		{
			//cout << "uinpoint: " << unPoint << endl;
			bool rightCond0 = (prevEn.x < 0 && prevEn.y >= 0 && en.x < 0 && en.y <= 0);
			bool rightCond1 = ( prevEn.x >= 0 && prevEn.y > 0 && en.x <= 0 && en.y < 0 );
			bool rightCond2 = prevEn.y > 0 && en.x >= 0 && en.y < 0;//( prevEn.x < 0 && prevEn.y > 0 && en.x >= 0 && en.y < 0 );
			
			bool rightCond3 = prevEn.x < 0 || en.x < 0;
			//bool rightCond4 = prevEn.x <= 0 && prevEn.y <= 0 && en.x >= 0 && en.y <= 0;
			//bool rightCond5 = prevEn.x >= 0 && prevEn.y >= 0 && en.x <= 0 && en.y >= 0;
			//bool rightCond4 = prevEn.x < 0 && prevEn.y <= 0 && en.x >= 0;
			//bool rightCond5 = en.x < 0 && en.y >= 0 && prevEn.x >= 0;
			bool rightCond4 = prevEn.x >= 0 && en.y < 0;
			bool rightCond5 = en.x >= 0 && prevEn.y > 0;
			bool rightCond6 = prevEn.x < 0 && en.x < 0;
			


			rightCond0 = rightCond3 && ( rightCond4 || rightCond5 || rightCond6 );
			rightCond2 = rightCond1 = rightCond0;
			
			//rightCond2 = prevEn.x < 0 && abs( prevEn.y ) > -steepThresh

			//rightCond0 = rightCond3 && !rightCond4 && !rightCond5;
			//rightCond1 = rightCond0;
			//rightCond2 = rightCond1;
			//bool rightCond5 = en.x 
			//bool rightCond3 = ( prevEn.x == 0 && prevEn.y > 0 && en.x < 0 && en.y < 0 );
			
			bool leftCond0 = (prevEn.x > 0 && prevEn.y <= 0 && en.x > 0 && en.y >= 0);
			bool leftCond1 = ( prevEn.x <= 0 && prevEn.y < 0 && en.x > 0 && en.y > 0 );
			bool leftCond2 = prevEn.y < 0 && en.x <= 0 && en.y > 0;//( prevEn.x >= 0 && prevEn.y < 0 && en.x <= 0 && en.y > 0 );
			//cout << "blah: " << (prevEn.x > 0) << ", " << (prevEn.y <= 0) << ", " << (en.x < 0 )<< ", " << (en.y > 0) << endl;

			bool leftCond3 = prevEn.x > 0 || en.x > 0;
			bool leftCond4 = prevEn.x <= 0 && en.y > 0;
			bool leftCond5 = en.x <= 0 && prevEn.y < 0;
			bool leftCond6 = prevEn.x > 0 && en.x > 0;

			leftCond0 = leftCond3 && (leftCond4 || leftCond5 || leftCond6 );
			leftCond1 = leftCond2 = leftCond0;
		//	bool leftCond3 = prevEn.x > 0 || en.x > 0;
		//	bool leftCond4 = 

			bool topCond0 = (prevEn.y > 0 && prevEn.x >= 0 && en.y >= 0 && en.x <= 0);
			//bool topCond0 = (prevEn.y > 0 && en.y >= 0 && en.x <= 0);
			bool topCond1 = ( prevEn.y <= 0 && prevEn.x > 0 && en.y > 0 && en.x < 0 );
			bool topCond2 = prevEn.y > 0 && prevEn.x > 0 && en.x < 0;

			bool topCond3 = prevEn.y > 0 || en.y > 0;
			bool topCond4 = prevEn.y <= 0 && en.x < 0;
			bool topCond5 = en.y <= 0 && prevEn.x > 0;
			bool topCond6 = prevEn.y > 0 && en.y > 0;
			topCond0 = topCond3 && ( topCond4 || topCond5 || topCond6 );
			topCond1 = topCond2 = topCond0;

			//bool bottomCond0 = (prevEn.y <= 0 && prevEn.x <= 0 && en.y < 0 && en.x >= 0);
			bool bottomCond0 = (prevEn.y <= 0 && prevEn.x <= 0 && en.y < 0 && en.x >= 0);
			bool bottomCond1 = ( prevEn.y > 0 && prevEn.x < 0 && en.y <= 0 && en.x > 0 );
			//bool bottomCond2 = ( prevEn.y < 0 && prevEn.x < 0 && en.y >= 0 && en.x > 0 );
			bool bottomCond2 = ( prevEn.y < 0 && prevEn.x < 0 &&  en.x > 0 && en.y > 0 );

			bool bottomCond3 = prevEn.y < 0 || en.y < 0;
			//bool bottomCond4 = prevEn.y <= 0 && prevEn.x >= 0 && en.y >= 0 && en.x >= 0; //4 is done. finish 5 then move onto right
			//bool bottomCond5 = prevEn.y >= 0 && prevEn.x <= 0 && en.y <= 0 && en.x <= 0;

			//bool bottomCond4 = prevEn.y < 0 && prevEn.x >= 0 && en.y >= 0;
			//bool bottomCond5 = en.y < 0 && en.x <= 0 && prevEn.y >= 0;
			//bool bottomCond6 = prevEn.x == -1 && en.y == -1;
			//bool bottomCond7 = prevEn.y == -1 && en.x == 1;

			bool bottomCond4 = prevEn.y >=0 && en.x > 0;
			bool bottomCond5 = en.y >=0 && prevEn.x < 0;
			bool bottomCond6 =  ( prevEn.y <= 0 && en.y <= 0 );

			
			bottomCond0 = bottomCond3 && (bottomCond4 || bottomCond5 || bottomCond6 );
			bottomCond1 = bottomCond2 = bottomCond0;

			//temporary
			bottomCond0 = bottomCond1 = bottomCond2 = false;
			rightCond0 = rightCond1 = rightCond2 = false;
			leftCond0 =leftCond1 = leftCond2 = false;
			topCond0 = topCond1 = topCond2 = false;

			bool topPos = false, botPos = false, leftPos = false, rightPos = false;
			double aaaa =  dot (-prevFullAlong, normalize( e->v1 - e->v0 ) );
			if( aaaa > 0 )
			{
				bool topRight = prevEn.x < 0 && prevEn.y < 0 && en.x > 0 && en.y > 0;
				bool topLeft = prevEn.x < 0 && prevEn.y > 0 && en.x > 0 && en.y < 0;
				bool botLeft = prevEn.x > 0 && prevEn.y > 0 && en.x < 0 && en.y < 0;
				bool botRight = prevEn.x > 0 && prevEn.y < 0 && en.x < 0 && en.y > 0;

				bool topFlat = (prevEn.x == 1 && en.x < 0 && en.y > 0 ) || ( prevEn.x > 0 && prevEn.y > 0 && en.x == -1 );
				bool botFlat = ( prevEn.x < 0 && prevEn.y < 0 && en.x == 1 ) || ( prevEn.x == -1 && en.x > 0 && en.y < 0 );
				bool leftFlat = (prevEn.y == -1 && en.x > 0 && en.y > 0 ) || (prevEn.x > 0 && prevEn.y < 0 && en.y == 1 );
				bool rightFlat = ( prevEn.x < 0 && prevEn.y > 0 && en.y == -1 ) || ( prevEn.y == 1 && en.x < 0 && en.y < 0 );

				bool up = prevEn.x < 0 && prevEn.y < 0 && en.x > 0 && en.y < 0;
				bool r = prevEn.x > 0 && prevEn.y < 0 && en.x > 0 && en.y > 0;
				bool down = prevEn.x > 0 && prevEn.y > 0 && en.x < 0 && en.y > 0;
				bool l = prevEn.x < 0 && prevEn.y > 0 && en.x < 0 && en.y < 0;

				//cout << "up: " << up << ", r: " << r << ", down: " << down << ", l: "<< l << endl;
				//cout << "topleft: " << topLeft << endl;
				if( botLeft || botRight || down || topFlat )
					topCond0 = true;
				if( topLeft || l || botLeft || rightFlat )
					rightCond0 = true;
				if( topRight || up || topLeft || botFlat )
					bottomCond0 = true;
				if( botRight || r || topRight || leftFlat )
					leftCond0 = true;
			}
			else
			{
				//the equals signs are for straight edges connected to slopes. not sure why i need to test those points but i guess it makes sense
				bool up = prevEn.x <= 0 && prevEn.y < 0 && en.x >= 0 && en.y < 0;
				bool r = prevEn.x > 0 && prevEn.y <= 0 && en.x > 0 && en.y >= 0;
				bool down = prevEn.x >= 0 && prevEn.y > 0 && en.x <= 0 && en.y > 0;
				bool l = prevEn.x < 0 && prevEn.y >= 0 && en.x < 0 && en.y <= 0;

				if( down )
					topCond0 = true;
				if( l )
					rightCond0 = true;
				if( r )
					leftCond0 = true;
				if( up )
					bottomCond0 = true;
			}

			if( (rightCond0 || rightCond1 || rightCond2 ) && vel.x > 0 && oldRight <= point.x + .001 && right >= point.x  )
			{
				rightTime = ( point.x - oldRight ) / abs(vel.x);
				V2d testRes =-vel * ( 1 - rightTime );
				if( top + testRes.y <= point.y && bottom + testRes.y >= point.y )
				{
					pointMinTime = rightTime;
					pointNormal.x = -1;
				}

			//	cout << "righttime: " << pointMinTime << endl;
				
			}
			else if( ( leftCond0 || leftCond1 || leftCond2 ) && vel.x < 0 && oldLeft >= point.x - .001 && left <= point.x  )
			{
				leftTime = ( oldLeft - point.x ) / abs( vel.x );
				V2d testRes =-vel * ( 1 - leftTime );
				if( top + testRes.y <= point.y && bottom + testRes.y >= point.y )
				{
					pointMinTime = leftTime;
					pointNormal.x = 1;
				}
			}
			
			if( (bottomCond0 || bottomCond1 || bottomCond2 ) && vel.y > 0 && oldBottom <= point.y + .001 && bottom >= point.y )
			{
				bool okay = true;
				//not sure what this is for. find out the purpose.
				/*if( vel.x > 0 )
				{
					if( oldLeft < edgeRight )
					{
						okay = true;
					}
				}
				else if( vel.x < 0 )
				{
					if( oldRight > edgeLeft )
					{
						okay = true;
					}
				}
				else
				{
					okay = true;
				}*/


				

				
				//cout << "right: " << right << ", px: " << point.x << ", pos: " << position.x << endl;
				
			//cout << "bottom cond okay: " << okay << endl;

				bottomTime = ( point.y - oldBottom ) / abs( vel.y );

				

				if( okay && bottomTime < pointMinTime )
				{
					V2d testRes =-vel * ( 1 - bottomTime );
					//doesnt include the equals stuff because left/right is preferred

					if( right + testRes.x >= point.x && left + testRes.x <= point.x )
					{
						//cout << "bottom vel: " << vel.x << ", " << vel.y << endl;
						pointMinTime = bottomTime;
						pointNormal.x = 0;
						pointNormal.y = -1;
					}
				}
			}
			else if( (topCond0 || topCond1 || topCond2 ) && vel.y < 0 && oldTop >= point.y - .001 && top <= point.y )
			{
			//	cout << "top" << endl;
				topTime = ( oldTop - point.y ) / abs( vel.y );
				
				if( topTime < pointMinTime )
				{
					V2d testRes =-vel * ( 1 - topTime );
					if( right + testRes.x >= point.x && left + testRes.x <= point.x )
					{
						pointMinTime = topTime;
						pointNormal.x = 0;
						pointNormal.y = 1;
					}
				}
			}

		}

		double time = 100;
		if( en.x == 0 )
		{
			//cout << "hitting now" << endl;
			double edgeYPos = edgeTop;
			if( en.y > 0 ) //down
			{
				//cout << "vel.y: " << vel.y << ", oldtop: " << oldTop << ", edgeypos: " << edgeYPos << ", top: " << top << endl;
				if( vel.y < 0 && oldTop >= edgeYPos - .001 && top <= edgeYPos )
				{
					bool hit = true;

					//bool a = left >= edgeLeft && left <= edgeRight;
					//bool b = right >= edgeLeft && right <= edgeRight;
					//cout << "edge l/r: " << edgeLeft << ", " << edgeRight << ", l/r: " << left << ", " << right << endl;
					bool a = left >= edgeLeft - .00001 && left <= edgeRight + .00001;
					bool b = right >= edgeLeft - .00001 && right <= edgeRight + .00001;

					if( a && b )
					{
						intersect.x = (right + left ) / 2.0;
					}
					else if(a  )
					{
						intersect.x = edgeRight;
						//intersect.x = left;
					}
					else if( b )
					{
						intersect.x = edgeLeft;
						//intersect.x = right;
					}
					else if( left <= edgeLeft && right >= edgeRight )
					{
						//cout << "blahhhhh:" << endl;
						intersect.x = (edgeLeft + edgeRight ) / 2.0;
					}
					else
					{
						hit = false;
						
					} 

					if( hit )
					{
						time = ( oldTop - edgeYPos ) / abs( vel.y );


						intersect.y = edgeYPos;
					}
					
				}
			}
			else //up
			{
				if( vel.y > 0 && oldBottom <= edgeYPos + .001 && bottom >= edgeYPos )
				{
					//cout << "this one: " << oldBottom << ", bottom: " << bottom << ", eyp: " << edgeYPos << endl;
					
					//rounding errors are the devil lol. this stops him from falling thru the stage in that one part
					//on the left of 1-4 where you hold right for 2 frames as you fall off the steep slope
					//and go through a corner
					
					
					bool a = left >= edgeLeft - .00001 && left <= edgeRight + .00001;
					bool b = right >= edgeLeft - .00001 && right <= edgeRight + .00001;

					/*bool a = left >= edgeLeft && left <= edgeRight;
					bool b = right >= edgeLeft && right <= edgeRight;*/
					
					
					//cout << "edge l/r: " << edgeLeft << ", " << edgeRight << ", l/r: " << left << ", " << right << endl;
					bool hit = true;

					if( a && b )
					{
						intersect.x = (right + left ) / 2.0;
					}
					else if(a  )
					{
						intersect.x = edgeRight;
						//intersect.x = left;
					}
					else if( b )
					{
						
						intersect.x = edgeLeft;
						//intersect.x = right;
						//cout << "only this!!: " << intersect.x << ", pos: " << position.x << endl;
					}
					else if( left <= edgeLeft && right >= edgeRight )
					{
						//cout << "blahhhhh:" << endl;
						intersect.x = (edgeLeft + edgeRight ) / 2.0;
					}
					else
					{
						hit = false;
						
					} 
					if( hit )
					{
						
						time = ( edgeYPos - oldBottom ) / abs( vel.y );
						//currentContact->resolution = -vel * ( 1 - time );

						intersect.y = edgeYPos;
					}
				}
			}

		}
		else if( en.y == 0 )
		{
			double edgeXPos = edgeLeft;
			if( en.x > 0 ) //right
			{
				//cout << "trying!: oldLeft: " << oldLeft << ", edgeXPos: " << edgeXPos <<", left: " << left << ", vel: " << vel.x << ", " << vel.y << endl;
				//cout << "blah: " << (vel.x < 0 ) << ", " << (oldLeft >= edgeXPos ) << ", " << (left <= edgeXPos ) << endl;
				if( vel.x < 0 && oldLeft >= edgeXPos - .001 && left <= edgeXPos )
				{
					bool a = top >= edgeTop && top <= edgeBottom;
					bool b = bottom >= edgeTop && bottom <= edgeBottom;
					//cout << "edge l/r: " << edgetop << ", " << edgebottom << ", l/r: " << top << ", " << bottom << endl;
					//cout << "in here: " << a << ", " << b << endl;
					bool hit = true;

					if( a && b )
					{
						intersect.y = (bottom + top ) / 2.0;
					}
					else if(a  )
					{
						intersect.y = top;
					}
					else if( b )
					{
						intersect.y = bottom;
					}
					else if( top <= edgeTop && bottom >= edgeBottom )
					{
						//cout << "blahhhhh:" << endl;
						intersect.y = (edgeTop + edgeBottom) / 2.0;
					}
					else
					{
						cout << "miss: 1 0: " << edgeTop << ", " << edgeBottom << ", l/r: " << top << ", " << bottom << endl;
						hit = false;
					} 

					if( hit )
					{
						time = ( oldLeft - edgeXPos ) / abs( vel.x);
						intersect.x = edgeXPos;
					}

				}
			}
			else //left
			{
				//cout << "attempting right: " << oldRight << ", " << edgeXPos << ", " << right << endl;
				if( vel.x > 0 && oldRight <= edgeXPos + .001 && right >= edgeXPos )
				{
					bool a = top >= edgeTop && top <= edgeBottom;
					bool b = bottom >= edgeTop && bottom <= edgeBottom;
					//cout << "edge l/r: " << edgetop << ", " << edgebottom << ", l/r: " << top << ", " << bottom << endl;
					bool hit = true;

					if( a && b )
					{
						intersect.y = (bottom + top ) / 2.0;
					}
					else if(a  )
					{
						intersect.y = top;
					}
					else if( b )
					{
						intersect.y = bottom;
					}
					else if( top <= edgeTop && bottom >= edgeBottom )
					{
						//cout << "blahhhhh:" << endl;
						intersect.y = (edgeTop + edgeBottom) / 2.0;
					}
					else
					{
						
						hit = false;
					} 

					if( hit )
					{
						time = ( edgeXPos - oldRight ) / abs( vel.x );
						intersect.x = edgeXPos;
					}
				}
				
				

			}

		
			
			
			//return NULL;
				
		}
		else
		{
			Vector2<double> corner(0,0);
			V2d opp;
			if( en.x > 0 )
			{
				corner.x = left;
				opp.x = right;
			}
			else if( en.x < 0 )
			{
				corner.x = right;
				opp.x = left;
			}
			
			if( en.y > 0 )
			{
				corner.y = top;
				opp.y = bottom;
			}
			else if( en.y < 0 )
			{
				corner.y = bottom;
				opp.y = top;
			}

			double res = cross( corner - e->v0, e->v1 - e->v0 );
			double oldRes = cross( (corner - vel ) - e->v0, e->v1 - e->v0 );
			double resOpp = cross( opp - e->v0, e->v1 - e->v0 );
			//might remove the opp thing soon

			double measureNormal = dot( en, normalize(-vel) );
			//cout << "oldRes : " << oldRes << endl;
			bool test = res < -.001 && resOpp > 0 && measureNormal > 0 && ( vel.x != 0 || vel.y != 0 ) ;
			//cout << "res: " << res << endl;
			//oldRes >= -.001
			
			//OlD RES WAS .001 BUT I CHANGED IT to .1
			//its so if theres _|\_  and you're coming from below
			//and you airdash into it, you don't pop through

			//and I still need old res so that I don't 
			//mess up some case, although i haven't figured out what
			//case that is yet. good luck collision T_T
			//if( res < -.001 && oldRes >= -.001 && resOpp > 0 && measureNormal > -.001 && ( vel.x != 0 || vel.y != 0 )  )	
			if( res < -.001 && oldRes >= -.1 && resOpp > 0 && measureNormal > -.001 && ( vel.x != 0 || vel.y != 0 )  )	
			
			{
				
				//cout << "normal: " << e->Normal().x << ", " << e->Normal().y << endl;
				LineIntersection li;
				lineIntersection( li, corner, corner - (vel), e->v0, e->v1);
				double testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
				if( li.parallel || abs( testing ) == 1 )
				{
					//cout << "returning null1" << endl;
					return NULL;
				}
				intersect = li.position;

				double intersectQuantity = e->GetQuantity( intersect );

				//cout << "test: " << test << " normal: " << en.x << ", " << en.y << " q: " << intersectQuantity << "len: " << length( e->v1 - e->v0 ) << endl;
				//if( intersectQuantity < 0 )
				//	intersectQuantity = 0;
				//if( intersectQuantity >length( e->v1 - e->v0 ) )
				//	intersectQuantity = length( e->v1 - e->v0 );
				double len = length( e->v1 - e->v0 );
				if( intersectQuantity < -.0001 || intersectQuantity > len + .0001 )
				{
					
					//cout << "bad: " << en.x << ", " << en.y << "  " << intersectQuantity << ", len: " << length( e->v1 - e->v0 ) << endl;
					if( intersectQuantity <= 0 )
					{
					//	point = e->v0;
					//	cout << "adjusting" << endl;
					}
					else
					{
					//	point = e->v1;
					}
				}
				else
				{
					bool okay = true;
					bool a = approxEquals( intersectQuantity, 0 );

					bool b = approxEquals( intersectQuantity, len );
					if( a || b  )
					{
						okay = false;
						double t;
						if( a )
						{
							t = cross( e->v1 - e->v0, prevReverseAlong);
							intersect = e->v0;
						}
						else
						{
							t = -cross( nextFullAlong, normalize(e->v1 - e->v0) ); 
							intersect = e->v1;
						}
						
						okay = true;
						if( t < -.001 )
						{	
							//okay = true;
							currentContact->weirdPoint = false;
							
						}
						else
						{
							currentContact->weirdPoint = true;
						//	cout << "weird point" << endl;
						}
					}

					if( okay )
					{
				//	cout << "using: " << intersectQuantity << ", length: " << length( e->v1 - e->v0 ) << endl;
					//this is prob wrong
					double tempTime = dot( intersect - ( corner - vel ), normalize( vel ) );
					tempTime /= length( vel );
					//cout << "tempTime: " << tempTime << endl;
					//if( tempTime >= -4 )
					{

						//if( tempTime < 0 )
						//	tempTime = 0;
						time = tempTime;
					}
						
					//		cout << "time: " << time << " normal: " << en.x << ", " << en.y << 
			//" vel: " << vel.x << ", " << vel.y << ", q: " << intersectQuantity << ", len: " << length( e->v1 - e->v0 ) << endl;
					}
						
					
						
					
				}
			}
			else
			{
			//	cout << "res: " << res << ", " << oldRes << ", " << resOpp << ", " << measureNormal << endl;
			
				//cout << "baz: " << (res < 0 ) <<", " << ( oldRes >= -.001 ) << ", " << (resOpp > 0 ) << ", " << (measureNormal > -.001 ) << endl;
			}


		}
		//aabb's already collide

		if( pointMinTime <= time )
		{
			if( point == e->v0 )
			{
				Edge *e0 = e->edge0;
				if( e0 != NULL && e0->edgeType == Edge::CLOSED_GATE )
				{

					//return NULL;
					/*Gate *g = (Gate*)e0->info;
					if( e0 == g->edgeA )
					{

					}
					else
					{

					}*/
				}
				
			}
			else if( point == e->v1 )
			{
				Edge *e1 = e->edge1;
				if( e1 != NULL && e1->edgeType == Edge::CLOSED_GATE )
				{
				//	return NULL;
				}
			}
			time = pointMinTime;
			currentContact->position = point;
			currentContact->normal = pointNormal;
			currentContact->weirdPoint = false;

			//cout << "point: " << time << endl;

			if( time == 100 )
			return NULL;

			/*CircleShape *cs = new CircleShape;
			cs->setFillColor( Color::Yellow );
			cs->setRadius( 5 );
			cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
			cs->setPosition( point.x, point.y );

			progressDraw.push_back( cs );	*/


		}
		else
		{
			currentContact->position = intersect;
			currentContact->normal = V2d( 0, 0 );

			/*CircleShape *cs = new CircleShape;
			cs->setFillColor( Color::Yellow );
			cs->setRadius( 5 );
			cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
			cs->setPosition( intersect.x, intersect.y );

			progressDraw.push_back( cs );*/
		}

		if( time == 100 )
			return NULL;
		
		if( time < 0 )
			time = 0;
		if( approxEquals( time, 0 ) )
			time = 0;
		currentContact->collisionPriority = time;
		currentContact->edge = e;
		
		currentContact->resolution = -vel * ( 1 - time );
		//currentContact->resolution += normalize( currentContact->resolution ) * .0001;

	//	cout << "pri: " << currentContact->collisionPriority << " normal: " << en.x << ", " << en.y << 
	//		" res: " << currentContact->resolution.x << ", " << currentContact->resolution.y <<
	//		" vel: " << vel.x << ", " << vel.y << ", pos: " << currentContact->position.x << ", " << currentContact->position.y
	//		<< "old: " << oldPosition.x << ", " << oldPosition.y << endl;

//		cout << "pri: " << currentContact->collisionPriority << " normal: " << en.x << ", " << en.y << endl;
		return currentContact;

		
		
	}
}

void Collider::DebugDraw( RenderTarget *target )
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		target->draw( *(*it) );
	}
	//progressDraw.clear();
}

void Collider::ClearDebug()
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		delete (*it);
	}
	progressDraw.clear();
}

EdgeParentNode::EdgeParentNode( const V2d &poss, double rww, double rhh )
{
	pos = poss;
	rw = rww;
	rh = rhh;
	leaf = false;
	children[0] = new EdgeLeafNode( V2d(pos.x - rw / 2.0, pos.y - rh / 2.0), rw / 2.0, rh / 2.0 );
	children[1] = new EdgeLeafNode( V2d(pos.x + rw / 2.0, pos.y - rh / 2.0), rw / 2.0, rh / 2.0 );
	children[2] = new EdgeLeafNode( V2d(pos.x - rw / 2.0, pos.y + rh / 2.0), rw / 2.0, rh / 2.0 );
	children[3] = new EdgeLeafNode( V2d(pos.x + rw / 2.0, pos.y + rh / 2.0), rw / 2.0, rh / 2.0 );

	
}

EdgeLeafNode::EdgeLeafNode( const V2d &poss, double rww, double rhh )
	:objCount(0)
{
	pos = poss;
	rw = rww;
	rh = rhh;

	leaf = true;
	for( int i = 0; i < 4; ++i )
	{
		edges[i] = NULL;
	}
}

sf::Rect<double> GetEdgeBox( Edge *e )
{
	double left = min( e->v0.x, e->v1.x );
	double right = max( e->v0.x, e->v1.x );
	double top = min( e->v0.y, e->v1.y );
	double bottom = max( e->v0.y, e->v1.y );
	return sf::Rect<double>( left, top, right - left, bottom - top );	
}

//to prevent creating these every function call
V2d as[4];
V2d bs[4];
LineIntersection eli;
bool IsEdgeTouchingBox( Edge *e, const sf::Rect<double> & ir )
{
	sf::Rect<double>& er = e->aabb;

	if (er.left >= ir.left 
		&& (er.left + er.width ) <= ir.left + ir.width 
		&& er.top >= ir.top 
		&& ( er.top + er.height ) <= ir.top + ir.height)
		return true;

	as[0].x = ir.left;
	as[0].y = ir.top;

	bs[0].x = ir.left + ir.width;
	bs[0].y = ir.top;

	as[1].x = ir.left;
	as[1].y = ir.top + ir.height;

	bs[1].x = ir.left + ir.width;
	bs[1].y = ir.top + ir.height;

	as[2].x = ir.left;
	as[2].y = ir.top;

	bs[2].x = ir.left;
	bs[2].y = ir.top + ir.height;
	
	as[3].x = ir.left + ir.width;
	as[3].y = ir.top;

	bs[3].x = ir.left + ir.width;
	bs[3].y = ir.top + ir.height;

	double erLeft = er.left;
	double erRight = er.left + er.width;
	double erTop = er.top;
	double erBottom = er.top + er.height;

	
	double e1Left;
	double e1Right;
	double e1Top;
	double e1Bottom;
	for( int i = 0; i < 4; ++i )
	{
		lineIntersection(eli, as[i], bs[i], e->v0, e->v1);
		if( !eli.parallel )
		{
			V2d &a = as[i];
			V2d &b = bs[i];
			e1Left = a.x;
			e1Right = b.x;

			if (b.x < a.x)
			{
				e1Left = b.x;
				e1Right = a.x;
			}

			e1Top = a.y;
			e1Bottom = b.y;

			if (b.y < a.y)
			{
				e1Top = b.y;
				e1Bottom = a.y;
			}

			if( e1Left <= erRight && e1Right >= erLeft && e1Top <= erBottom && e1Bottom >= erTop )
			{
				if( (eli.position.x < e1Right || approxEquals(eli.position.x, e1Right) ) 
					&& (eli.position.x > e1Left || approxEquals(eli.position.x, e1Left ) ) 
					&& (eli.position.y > e1Top || approxEquals(eli.position.y, e1Top ) )
					&& (eli.position.y < e1Bottom || approxEquals(eli.position.y, e1Bottom ) ) )
				{
					if( (eli.position.x < erRight || approxEquals(eli.position.x, erRight )) 
						&& (eli.position.x > erLeft || approxEquals(eli.position.x, erLeft ) ) 
						&& (eli.position.y > erTop || approxEquals(eli.position.y, erTop ) ) 
						&& (eli.position.y < erBottom || approxEquals(eli.position.y, erBottom ) ) )
					{
						return true;
					}
				}
			}
		}
	}
	//cout << "return false" << endl;
	return false;
}

bool IsBoxTouchingBox( const sf::Rect<double> & r0, const sf::Rect<double> & r1 )
{
	//bool test = r0.intersects( r1 );
	bool test2 =r0.left <= r1.left + r1.width 
		&& r0.left + r0.width >= r1.left 
		&& r0.top <= r1.top + r1.height
		&& r0.top + r0.height >= r1.top;
	return test2;
}

bool IsBoxTouchingBox(const sf::Rect<int> & r0, const sf::Rect<int> & r1)
{
	//bool test = r0.intersects(r1);
	bool test2 = r0.left <= r1.left + r1.width
		&& r0.left + r0.width >= r1.left
		&& r0.top <= r1.top + r1.height
		&& r0.top + r0.height >= r1.top;
	return test2;
}

bool IsCircleTouchingCircle( V2d pos0, double rad_0, V2d pos1, double rad_1 )
{
	return length( pos1 - pos0 ) <= rad_0 + rad_1;
}

bool IsEdgeTouchingCircle(sf::Vector2<double> &v0, sf::Vector2<double> &v1, 
	sf::Vector2<double> &pos, double rad )
{	
	if (v0 == v1)
	{
		cout << "edge points cant be the same" << endl;
		assert(0);
		return false;
	}
	double q = dot( pos - v0, normalize( v1 - v0 ) );

	if( q < 0 )
	{
		return length(v0 - pos) < rad;
	}
	else if( q > length(v1 - v0))
	{
		return length(v1 - pos) < rad;
	}

	double dist = cross(pos - v0, normalize(v1 - v0));
	return abs(dist) < rad;
}

bool IsQuadTouchingCircle( V2d &A, V2d &B, V2d &C, V2d &D, V2d &pos, double rad )
{
	if (QuadContainsPoint(A, B, C, D, pos))
	{
		return true;
	}

	bool a = IsEdgeTouchingCircle( A,B, pos, rad );
	bool b = IsEdgeTouchingCircle( B,C, pos, rad );
	bool c = IsEdgeTouchingCircle( C,D, pos, rad );
	bool d = IsEdgeTouchingCircle( D,A, pos, rad );

	if( a || b || c || d )
	{
		//cout << a << ", " << b << ", " << c << ", " << d << endl;
		return true;
	}

	return false;
}
//top left is A then clockwise
bool isQuadTouchingQuad( V2d &A0, V2d &B0, V2d &C0, V2d &D0, V2d &A1, V2d &B1, V2d &C1, V2d &D1 )
{
	double AB = length( B0 - A0 );
	double AD = length( D0 - A0 );

	V2d normalizeAB = normalize( B0 - A0 );
	V2d normalizeAD = normalize( D0 - A0 );
	

	double min1AB = min( dot( A1 - A0, normalizeAB ), min( dot( B1 - A0, normalizeAB ), min( dot( C1 - A0, normalizeAB ),
		dot( D1 - A0, normalizeAB ) ) ) );
	double max1AB = max( dot( A1 - A0, normalizeAB ), max( dot( B1 - A0, normalizeAB ), max( dot( C1 - A0, normalizeAB ),
		dot( D1 - A0, normalizeAB ) ) ) );

	double min1AD = min( dot( A1 - A0, normalizeAD ), min( dot( B1 - A0, normalizeAD ), min( dot( C1 - A0, normalizeAD ),
		dot( D1 - A0, normalizeAD ) ) ) );
	double max1AD = max( dot( A1 - A0, normalizeAD ), max( dot( B1 - A0, normalizeAD ), max( dot( C1 - A0, normalizeAD ),
		dot( D1 - A0, normalizeAD ) ) ) );

	
	double AB1 = length( B1 - A1 );
	double AD1 = length( D1 - A1 );

	V2d normalizeAB1 = normalize( B1 - A1 );
	V2d normalizeAD1 = normalize( D1 - A1 );

	double min0AB = min( dot( A0 - A1, normalizeAB1 ), min( dot( B0 - A1, normalizeAB1 ), min( dot( C0 - A1, normalizeAB1 ),
		dot( D0 - A1, normalizeAB1 ) ) ) );
	double max0AB = max( dot( A0 - A1, normalizeAB1 ), max( dot( B0 - A1, normalizeAB1 ), max( dot( C0 - A1, normalizeAB1 ),
		dot( D0 - A1, normalizeAB1 ) ) ) );

	double min0AD = min( dot( A0 - A1, normalizeAD1 ), min( dot( B0 - A1, normalizeAD1 ), min( dot( C0 - A1, normalizeAD1 ),
		dot( D0 - A1, normalizeAD1 ) ) ) );
	double max0AD = max( dot( A0 - A1, normalizeAD1 ), max( dot( B0 - A1, normalizeAD1 ), max( dot( C0 - A1, normalizeAD1 ),
		dot( D0 - A1, normalizeAD1 ) ) ) );

	bool a = min1AB <= AB;
	bool b = max1AB >= 0;
	bool c = min1AD <= AD;
	bool d = max1AD >= 0;
	bool e = min0AB <= AB1;
	bool f = max0AB >= 0;
	bool g = min0AD <= AD1;
	bool h = max0AD >= 0;

	if( a && b && c && d && e && f && g && h )
	{
		/*cout << "AB: " << AB << ", AD: " << AD << ", AB1: " << AB1 << ", AD1: " << AD1 << endl;
		cout << "min1AB: " << min1AB << ", max1AB: " << max1AB << ", min1AD: " << min1AD << ", max1AD: " << max1AD << ", min0AB: " << min0AB <<
			", max0AB: " << max0AB << ", min0AD: " << min0AD << ", max0AD: " << max0AD << endl;*/
		return true;
	}
	else
	{
		return false;
	}
}

bool QuadContainsPoint( V2d &A, V2d &B, V2d&C, V2d&D, V2d&point )
{
	V2d AB = B - A;
	V2d AD = D - A;
	V2d pointA = point - A;
	double pointAB = dot( pointA, normalize( AB ) );
	double pointAD = dot( pointA, normalize( AD ) );

	if( pointAB >= 0 && pointAB * pointAB <= dot( AB,AB ) )
	{
		if( pointAD >= 0 && pointAD * pointAD <= dot( AD, AD ) )
		{
			return true;
		}
	}

	return false;
}

bool QuadContainsPoint(sf::Vector2<float> &A,
	sf::Vector2<float> &B,
	sf::Vector2<float> &C,
	sf::Vector2<float> &D,
	sf::Vector2<float> &point)
{
	Vector2f AB = B - A;
	Vector2f AD = D - A;
	Vector2f pointA = point - A;
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

bool QuadContainsPoint(sf::Vertex *quad, sf::Vector2f &point)
{
	return QuadContainsPoint(quad[0].position, 
		quad[1].position, 
		quad[2].position, 
		quad[3].position,
		point);
}

sf::Vector2f GetQuadCenter(sf::Vertex *v)
{
	return (v[0].position + v[1].position + v[2].position + v[3].position) / 4.f;
}

sf::FloatRect GetQuadAABB(sf::Vertex *v)
{
	float left, right, top, bot;
	left = v[0].position.x;
	right = left;
	top = v[0].position.y;
	bot = top;

	for (int i = 1; i < 4; ++i)
	{
		left = min(left, v[i].position.x);
		right = max(right, v[i].position.x);
		top = min(top, v[i].position.y);
		bot = max(bot, v[i].position.y);
	}

	return FloatRect(left, top, right - left, bot - top);
	//return (v[0].position + v[1].position + v[2].position + v[3].position) / 4.f;
}

bool IsEdgeTouchingQuad(sf::Vector2<double> &v0,
	sf::Vector2<double> &v1,
	sf::Vector2<double> &A,
	sf::Vector2<double> &B,
	sf::Vector2<double> &C,
	sf::Vector2<double> &D)
{
	LineIntersection li = SegmentIntersect(v0, v1, A, B);
	if (!li.parallel)
	{
		return true;
	}
	li = SegmentIntersect(v0, v1, B, C);
	if (!li.parallel)
	{
		return true;
	}
	li = SegmentIntersect(v0, v1, C, D);
	if (!li.parallel)
	{
		return true;
	}
	li = SegmentIntersect(v0, v1, D, A);
	if (!li.parallel)
	{
		return true;
	}

	return false;
}

EdgeQNode *Insert( EdgeQNode *node, Edge* e )
{
	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;
		if( n->objCount == 4 ) //full
		{
		//	cout << "splitting" << endl;	
			EdgeParentNode *p = new EdgeParentNode( n->pos, n->rw, n->rh );
			p->parent = n->parent;
			p->debug = n->debug;

		/*	for( int i = 0; i < 4; ++i )
			{
				//EdgeLeafNode *inner = (EdgeLeafNode*)p->children[i];
				Edge * tempEdge = n->edges[i];
				sf::IntRect nw( node->pos.x - node->rw, node->pos.y - node->rh, node->rw, node->rh);
				sf::IntRect ne( node->pos.x + node->rw, node->pos.y - node->rh, node->rw, node->rh );
				sf::IntRect sw( node->pos.x - node->rw, node->pos.y + node->rh, node->rw, node->rh );
				sf::IntRect se( node->pos.x + node->rw, node->pos.y + node->rh, node->rw, node->rh );

				if( IsEdgeTouchingBox( tempEdge, nw ) )
					p->children[0] = Insert( p->children[0], tempEdge );
				if( IsEdgeTouchingBox( tempEdge, ne ) )
					p->children[1] = Insert( p->children[1], tempEdge );
				if( IsEdgeTouchingBox( tempEdge, sw ) )
					p->children[2] = Insert( p->children[2], tempEdge );
				if( IsEdgeTouchingBox( tempEdge, se ) )
					p->children[3] = Insert( p->children[3], tempEdge );
			}*/

			for( int i = 0; i < 4; ++i )
			{
			//	cout << "test: " << n->edges[i]->Normal().x << ", " << n->edges[i]->Normal().y << endl;
				Insert( p, n->edges[i] );
			}


			delete node;

			 

			return Insert( p, e );
		}
		else
		{
		//	cout << "inserting into leaf . " << n->objCount << endl;
		//	cout << "norm: " << e->Normal().x << ", " << e->Normal().y << endl;
			n->edges[n->objCount] = e;
			++(n->objCount);
			return node;
		}
	}
	else
	{
	//	cout << "inserting into parent" << endl;
		EdgeParentNode *n = (EdgeParentNode*)node;
		sf::Rect<double> nw( node->pos.x - node->rw, node->pos.y - node->rh, node->rw, node->rh);
		sf::Rect<double> ne( node->pos.x, node->pos.y - node->rh, node->rw, node->rh );
		sf::Rect<double> sw( node->pos.x - node->rw, node->pos.y, node->rw, node->rh );
		sf::Rect<double> se( node->pos.x, node->pos.y, node->rw, node->rh );

		if( IsEdgeTouchingBox( e, nw ) )
		{
	//		cout << "calling northwest insert" << endl;
			n->children[0] = Insert( n->children[0], e );
		}
		if( IsEdgeTouchingBox( e, ne ) )
		{
	//		cout << "calling northeast insert" << endl;
			n->children[1] = Insert( n->children[1], e );
		}
		if( IsEdgeTouchingBox( e, sw ) )
		{
	//		cout << "calling southwest insert" << endl;
			n->children[2] = Insert( n->children[2], e );
		}
		if( IsEdgeTouchingBox( e, se ) )
		{
	//		cout << "calling southeast insert" << endl;
			n->children[3] = Insert( n->children[3], e );
		}
	}

	


	return node;
}

void DebugDrawQuadTree( sf::RenderWindow *w, EdgeQNode *node )
{
	//cout << "pos: " << node->pos.x << ", " << node->pos.y << " , rw: " << node->rw << ", rh: " << node->rh << endl;
	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;

		sf::RectangleShape rs( sf::Vector2f( node->rw * 2, node->rh * 2 ) );
		int trans = 100;
		if( n->objCount == 0 )
			rs.setFillColor( Color( 100, 100, 100, trans ) ); //
		else if( n->objCount == 1 )
			rs.setFillColor( Color( 255, 0, 0, trans) ); // red == 1
		else if( n->objCount == 2 )
			rs.setFillColor( Color( 0, 255, 0, trans ) ); // green == 2
		else if( n->objCount == 3 )
			rs.setFillColor( Color( 0, 0, 255, trans ) ); //blue == 3
		else
		{
			rs.setFillColor( Color( 0, 100, 255, trans ) ); //blah == 4
		}
		
		//rs.setFillColor( Color::Green );
		//rs.setOutlineColor( Color::Blue );
	//	rs.setOutlineThickness( 3 );
		//rs.setFillColor( Color::Transparent );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setPosition( node->pos.x, node->pos.y );
		//rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );

		w->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );
		//w->draw( cs );
	}
	else
	{
		EdgeParentNode *n = (EdgeParentNode*)node;
		sf::RectangleShape rs( sf::Vector2f( node->rw * 2, node->rh * 2 ) );
		//rs.setOutlineColor( Color::Red );
		rs.setOrigin( rs.getLocalBounds().width / 2.0, rs.getLocalBounds().height / 2.0 );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setPosition( node->pos.x, node->pos.y );
		rs.setFillColor( Color::Transparent );
		//rs.setOutlineThickness( 10 );
		w->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );

		//w->draw( cs );

		for( int i = 0; i < 4; ++i )
			DebugDrawQuadTree( w, n->children[i] );
	}
	

	
}

void Query( EdgeQuadTreeCollider *qtc, EdgeQNode *node, const sf::Rect<double> &r )
{
	sf::Rect<double> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );

	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;

		if( IsBoxTouchingBox( r, nodeBox ) )
		{
			for( int i = 0; i < n->objCount; ++i )
			{
				qtc->HandleEdge( n->edges[i] );
			}
		}
	}
	else
	{
		//shouldn't this check for box touching box right here??
		EdgeParentNode *n = (EdgeParentNode*)node;

		if( r.intersects( nodeBox ) )
		{
			for( int i = 0; i < 4; ++i )
			{
				Query( qtc, n->children[i], r );
			}
		}
	}
	
}

bool WithinDistance(sf::Vector2<double> &A,
	sf::Vector2<double> &B, double rad)
{
	double lenSqr = lengthSqr(A - B);
	if (rad * rad > lenSqr)
		return true;

	return false;
}

//only works on edges
void RayCast(RayCastHandler *handler, QNode *node,Edge &e)
{
	if (node->leaf)
	{
		LeafNode *n = (LeafNode*)node;

		sf::Rect<double> nodeBox(node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2);

		if (IsEdgeTouchingBox(&e, nodeBox))
		{
			for (int i = 0; i < n->objCount; ++i)
			{
				LineIntersection li = SegmentIntersect(e.v0, e.v1, ((Edge*)(n->entrants[i]))->v0, ((Edge*)(n->entrants[i]))->v1);
				if (!li.parallel)
				{
					handler->HandleRayCollision(((Edge*)(n->entrants[i])), ((Edge*)(n->entrants[i]))->GetQuantity(li.position),
						dot(V2d(li.position - e.v0), e.Along()));
				}
			}
		}
	}
	else
	{
		ParentNode *n = (ParentNode*)node;

		sf::Rect<double> nodeBox(node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2);

		if (IsEdgeTouchingBox(&e, nodeBox))
		{
			int extraChildrenSize = n->extraChildren.size();
			Edge *currEdge;
			for (int i = 0; i < extraChildrenSize; ++i)
			{
				currEdge = (Edge*)(n->extraChildren[i]);
				LineIntersection li = SegmentIntersect(e.v0, e.v1, currEdge->v0, (currEdge->v1));
				if (!li.parallel)
				{
					handler->HandleRayCollision(currEdge, currEdge->GetQuantity(li.position),
						dot(V2d(li.position - e.v0), e.Along()));
				}
			}

			for (int i = 0; i < 4; ++i)
			{
				RayCast(handler, n->children[i], e);
			}
		}
	}
}

void RayCastHandler::HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion)
{
	double len = length(edge->GetPosition(edgeQuantity) - rayCastInfo.rayStart);
	if (rayCastInfo.rcEdge == NULL
		|| len < rayCastInfo.rcPortion)
	{
		rayCastInfo.rcEdge = edge;
		rayCastInfo.rcPortion = len;
		rayCastInfo.rcQuant = edgeQuantity;
	}
}

bool RayCastHandler::ExecuteRayCast(V2d &start, V2d& end)
{
	rayCastInfo.Reset();
	rayCastInfo.rayStart = start;
	rayCastInfo.rayEnd = end;
	RayCast(this, rayCastInfo.tree->startNode, rayCastInfo.rayStart, rayCastInfo.rayEnd);

	if (rayCastInfo.rcEdge != NULL)
	{
		return true;
	}

	return false;
}

//only works on edges
void RayCast( RayCastHandler *handler, QNode *node, V2d &startPoint, V2d &endPoint )
{
	Edge e;
	e.v0 = startPoint;
	e.v1 = endPoint;
	e.CalcAABB();

	RayCast(handler, node, e);
}

void Edge::HandleQuery(QuadTreeCollider * qtc)
{
	qtc->HandleEntrant(this);
}

Edge * Edge::GetPrevEdge()
{
	return edge0;
}

Edge * Edge::GetNextEdge()
{
	return edge1;
}

bool Edge::IsFlatGround()
{
	return (Normal().x == 0);
}

bool Edge::IsSlopedGround()
{
	V2d normal = Normal();
	return (abs(normal.y) > GetSteepThresh() && normal.x != 0);
}

bool Edge::IsSteepGround()
{
	V2d normal = Normal();
	return (abs(normal.y) <= GetSteepThresh() && abs(normal.x) < 1.0);
}

bool Edge::IsWall()
{
	return Normal().y == 0;
}

sf::Rect<double> CollisionBox::GetAABB()
{
	if (isCircle)
	{
		V2d truePos = GetTrueCenter();
		return sf::Rect<double>(truePos.x - rw, truePos.y - rw, rw * 2, rw * 2);
	}
	else
	{
		V2d quads[4];
		for (int i = 0; i < 4; ++i)
		{
			quads[i] = GetQuadVertex(i);
		}

		double temp;
		double left = quads[0].x;
		double right = left;
		double top = quads[0].y;
		double bot = top;

		for (int i = 1; i < 4; ++i)
		{
			temp = quads[i].x;
			if (temp < left)
				left = temp;
			if (temp > right)
				right = temp;
			temp = quads[i].y;
			if (temp < top)
				top = temp;
			if (temp > bot)
				bot = temp;
		}
		return sf::Rect<double>(left, top, right - left, bot - top);
	}
}

CollisionBody::CollisionBody()
	:hitboxInfo( NULL ), numFrames( 0 ), boxType( CollisionBox::Hit )
{

}

CollisionBody::CollisionBody(CollisionBox::BoxType bType)
	:hitboxInfo(NULL), numFrames(0)
{
	boxType = bType;
	//collisionBoxVectors = new std::list<CollisionBox>*[numFrames];
	//memset(collisionBoxVectors, 0, sizeof(collisionBoxVectors) * numFrames);
}

void CollisionBody::ResetFrames()
{
	for (int i = 0; i < numFrames; ++i)
	{
		collisionBoxVectors[i].clear(); //keeps capacity the same
	}
}

void CollisionBody::SetupNumFrames(int p_numFrames)
{
	numFrames = p_numFrames;
	collisionBoxVectors.resize(numFrames);
}

void CollisionBody::SetupNumBoxesOnFrame(int frame, int numBoxes)
{
	assert(frame >= 0 && frame < collisionBoxVectors.size());

	collisionBoxVectors[frame].reserve(numBoxes);
}

void CollisionBody::OffsetFrame(int frame, Vector2f &offset)
{
	auto &li = GetCollisionBoxes(0);
	for (auto it = li.begin(); it != li.end(); ++it)
	{
		(*it).offset += V2d( offset );
	}
}

void CollisionBody::OffsetAllFrames(Vector2f &offset)
{
	for (int i = 0; i < numFrames; ++i)
	{
		auto &boxVec = collisionBoxVectors[i];
		if (boxVec.empty())
			continue;
		else
		{
			for (auto it = boxVec.begin(); it != boxVec.end(); ++it)
			{
				(*it).offset += V2d(offset);
			}
		}
	}
}

CollisionBody::CollisionBody(int p_numFrames, std::map<int, std::list<CollisionBox>> & hListMap,
	HitboxInfo *hInfo )
	:hitboxInfo( hInfo )
{
	boxType = CollisionBox::Hit;
	SetupNumFrames(p_numFrames);

	int frame;
	for (auto it = hListMap.begin(); it != hListMap.end(); ++it)
	{
		frame = (*it).first;

		auto & cbList = collisionBoxVectors[frame];

		std::list<CollisionBox> &hList = (*it).second;
		cbList.reserve(hList.size());
		for (auto hit = hList.begin(); hit != hList.end(); ++hit)
		{
			cbList.push_back((*hit));
		}
	}
}

bool CollisionBody::Empty()
{
	return collisionBoxVectors.empty();
}

void CollisionBody::BasicSetup() //one frame and one hitbox
{
	SetupNumFrames(1);
	SetupNumBoxesOnFrame(0, 1);
}

void CollisionBody::AddBasicCircle( int frame, double radius, double angle,
	V2d &offset )
{
	CollisionBox box;
	box.isCircle = true;
	box.globalAngle = angle;
	box.offset.x = offset.x;
	box.offset.y = offset.y;
	box.rw = radius;
	box.rh = radius;
	AddCollisionBox(frame, box);
}

void CollisionBody::AddBasicRect( int frame, double hw, double hh,
	double angle, V2d &offset )
{
	CollisionBox box;
	box.isCircle = false;
	box.globalAngle = angle;
	box.offset.x = offset.x;
	box.offset.y = offset.y;
	box.rw = hw;
	box.rh = hh;
	AddCollisionBox(frame, box);
}

void CollisionBody::BasicCircleSetup( double radius, double angle, V2d &offset ) //one frame and one hitbox
{
	BasicSetup();
	
	AddBasicCircle( 0, radius, angle, offset);
}

void CollisionBody::BasicCircleSetup(double radius)
{
	BasicSetup();
	AddBasicCircle(0, radius, 0, V2d());
}

void CollisionBody::BasicCircleSetup(double radius, double angle, V2d &offset, V2d &pos) //one frame and one hitbox
{
	BasicCircleSetup(radius, angle, offset);
	SetBasicPos(pos);
}

void CollisionBody::BasicCircleSetup(double radius, V2d &pos)
{
	BasicCircleSetup(radius);
	SetBasicPos(pos);
}

V2d CollisionBody::GetBasicPos()
{
	return collisionBoxVectors[0][0].globalPosition;
}

int CollisionBody::GetFirstNonEmptyFrame()
{
	int size = collisionBoxVectors.size();
	for (int i = 0; i < size; ++i)
	{
		if (!collisionBoxVectors[i].empty())
		{
			return i;
		}
	}
}

void CollisionBody::SetBasicPos(V2d &pos)
{
	collisionBoxVectors[0][0].globalPosition = pos;
}

void CollisionBody::SetBasicPos(V2d &pos, double angle )
{
	auto &box = collisionBoxVectors[0][0];
	box.globalAngle = angle;
	box.globalPosition = pos;
}

void CollisionBody::SetBasicPos(int frame, V2d &pos, double angle)
{
	collisionBoxVectors[frame][0].globalPosition = pos;
	collisionBoxVectors[frame][0].globalAngle = angle;
}

void CollisionBody::SetLastPos( int frame, V2d &pos)
{
	collisionBoxVectors[frame].back().globalPosition = pos;
}

void CollisionBody::BasicRectSetup(double w, double h, double angle, V2d &offset)
{
	BasicSetup();
	AddBasicRect( 0, w, h, angle, offset);
}

CollisionBody::~CollisionBody()
{
}

sf::Rect<double> CollisionBody::GetAABB( int frame )
{
	bool first = true;
	double left, right, top, bot;

	std::vector<CollisionBox> &cbVec = collisionBoxVectors[frame];

	if (cbVec.empty())
		return sf::Rect<double>();
	
	sf::Rect<double> tAABB;
	for (auto bList = cbVec.begin(); bList != cbVec.end(); ++bList)
	{
		tAABB = (*bList).GetAABB();
		if (first)
		{
			first = false;
			left = tAABB.left;
			top = tAABB.top;
			right = tAABB.left + tAABB.width;
			bot = tAABB.top + tAABB.height;
		}
		else
		{
			if (tAABB.left < left)
			{
				left = tAABB.left;
			}
			if (tAABB.top < top)
			{
				top = tAABB.top;
			}
			if (tAABB.left + tAABB.width > right)
			{
				right = tAABB.left + tAABB.width;
			}
			if (tAABB.top + tAABB.height > bot )
			{
				bot = tAABB.top + tAABB.height;
			}
		}
	}

	return sf::Rect<double>(left, top, right - left, bot - top);
}

void CollisionBody::AddCollisionBox(int frame, CollisionBox &cb)
{
	collisionBoxVectors[frame].push_back(cb);
}

std::vector<CollisionBox> &CollisionBody::GetCollisionBoxes(int frame)
{
	if (frame >= numFrames || frame < 0 )
	{
		assert(0);
		return collisionBoxVectors[0];
	}

	return collisionBoxVectors[frame];
}

int CollisionBody::GetNumBoxes(int frame)
{
	return collisionBoxVectors[frame].size();
}

void CollisionBody::DebugDraw( int frame, sf::RenderTarget *target)
{
	if( collisionBoxVectors.empty() )
		return;

	auto &cbVec = collisionBoxVectors[frame];
	if (cbVec.empty() )
		return;

	for (auto it = cbVec.begin(); it != cbVec.end(); ++it)
	{
		(*it).DebugDraw( boxType, target);
	}
}

void CollisionBody::Move(V2d &move)
{
	vector<CollisionBox> *myVec;
	for (int i = 0; i < numFrames; ++i)
	{
		myVec = &GetCollisionBoxes(i);
		auto end = myVec->end();
		for (auto it = myVec->begin(); it != end; ++it)
		{
			(*it).Move(move);
		}
	}
}

bool CollisionBody::Intersects( int frame, CollisionBody *other, int otherFrame )
{
	if (!GetAABB(frame ).intersects(other->GetAABB( otherFrame )))
		return false;

	vector<CollisionBox> &myVec = GetCollisionBoxes(frame);
	vector<CollisionBox> &otherVec = other->GetCollisionBoxes(otherFrame);

	if (myVec.empty() || otherVec.empty())
		return false;

	for (auto it = myVec.begin(); it != myVec.end(); ++it)
	{
		for (auto oit = otherVec.begin(); oit != otherVec.end(); ++oit)
		{
			if ((*it).Intersects((*oit)))
			{
				return true;
			}
		}
	}

	return false;
}

bool CollisionBody::Intersects( int frame, CollisionBox *box)
{
	if (!GetAABB(frame).intersects(box->GetAABB()) )
		return false;

	auto &myVec = GetCollisionBoxes(frame);
	if (myVec.empty() )
		return false;

	for (auto it = myVec.begin(); it != myVec.end(); ++it)
	{
		if ((*it).Intersects(*box))
		{
			return true;
		}
	}

	return false;
}



