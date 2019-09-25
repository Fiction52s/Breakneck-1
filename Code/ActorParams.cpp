//#include "EditSession.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Physics.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "Enemy_Shard.h"
#include "ActorParams.h"

using namespace std;
using namespace sf;

#define cout std::cout

#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )


sf::Font *PoiParams::font = NULL;

ActorParams::ActorParams( ActorType *at)
	:ISelectable( ISelectable::ACTOR ), boundingQuad( sf::Quads, 4 ),
		hasMonitor( false ), group( NULL ), type( at )
{
	groundInfo = NULL;
	lines = NULL;
	enemyLevel = 1;
	loop = false;
	for( int i = 0; i < 4; ++i )
		boundingQuad[i].color = Color( 0, 255, 0, 100);
}

ActorParams::~ActorParams()
{
	if (lines != NULL)
		delete lines;
}

void ActorParams::WriteLevel(std::ofstream &of)
{
	if (enemyLevel < 1)
		enemyLevel = 1;
	of << enemyLevel << endl;
}

bool ActorParams::SetLevel(int lev)
{
	if (lev >= 1 && lev <= type->info.numLevels)
	{
		enemyLevel = lev;
		return true;
	}
	else
	{
		return false;
	}
}

void ActorParams::WriteMonitor(ofstream  &of)
{
	WriteBool(of, hasMonitor);
}

void ActorParams::WriteParamFile(std::ofstream &of)
{
	WriteBasicParamFile(of);
}

void ActorParams::WriteBasicParamFile(std::ofstream &of)
{
	WriteMonitor(of);
	WriteLevel(of);
}

void ActorParams::SetParams()
{
	SetBasicParams();
}

void ActorParams::SetBasicParams()
{
	Panel *p = type->panel;

	int level;

	stringstream ss;
	string s = p->textBoxes["level"]->text.getString().toAnsiString();
	ss << s;

	ss >> level;

	if (!ss.fail() && level > 0 && level < type->info.numLevels)
	{
		enemyLevel = level;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}



void ActorParams::WritePath( std::ofstream &of )
{
	of << localPath.size() << endl;

	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		of << (*it).x << " " << (*it).y << endl;
	}
}

void ActorParams::WriteLoop(std::ofstream &of)
{
	WriteBool(of, loop);
}

void ActorParams::SetPath( std::list<sf::Vector2i> &globalPath )
{	
	if (lines != NULL)
	{
		delete lines;
		lines = NULL;
	}

	localPath.clear();
	if (globalPath.size() > 1)
	{
		int numLines = globalPath.size();

		lines = new VertexArray(sf::LinesStrip, numLines);
		VertexArray &li = *lines;
		li[0].position = Vector2f(0, 0);
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for (; it != globalPath.end(); ++it)
		{

			Vector2i temp((*it).x - position.x, (*it).y - position.y);
			localPath.push_back(temp);
			li[index].position = Vector2f(temp.x, temp.y);
			li[index].color = Color::Magenta;
			++index;
		}
	}
}

std::list<sf::Vector2i> ActorParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back(position);
	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		globalPath.push_back(position + (*it));
	}
	return globalPath;
}

void ActorParams::PlaceAerial(sf::Vector2i &pos)
{
	position = pos;
	image = type->GetSprite(false);
	image.setPosition(position.x, position.y);

	SetBoundingQuad();
}

void ActorParams::PlaceGrounded(TerrainPolygon *tp,
	int edgeIndex, double quant)
{
	AnchorToGround(tp, edgeIndex, quant);
	SetBoundingQuad();
}

void ActorParams::LoadGrounded(std::ifstream &is)
{
	int terrainIndex, edgeIndex;
	double edgeQuantity;
	
	is >> terrainIndex;
	is >> edgeIndex;
	is >> edgeQuantity;

	EditSession *edit = EditSession::GetSession();
	TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);
	PlaceGrounded(terrain, edgeIndex, edgeQuantity);
}

void ActorParams::LoadBool(ifstream &is, bool &b)
{
	int x;
	is >> x;
	b = x;
}

void ActorParams::WriteBool(ofstream &of, bool b)
{
	of << (int)b << "\n";
}

void ActorParams::LoadAerial(std::ifstream &is)
{
	is >> position.x;
	is >> position.y;

	PlaceAerial( position );
}

void ActorParams::LoadGlobalPath( ifstream &is)
{
	list<Vector2i> globalPath;
	int pathLength;
	is >> pathLength;

	globalPath.push_back(Vector2i(position.x, position.y));

	for (int i = 0; i < pathLength; ++i)
	{
		int localX, localY;
		is >> localX;
		is >> localY;
		globalPath.push_back(Vector2i(position.x + localX, position.y + localY));
	}

	SetPath(globalPath);
}

void ActorParams::LoadMonitor(std::ifstream &is)
{
	int ihasMonitor;
	is >> ihasMonitor;
	hasMonitor = ihasMonitor;
}

void ActorParams::LoadEnemyLevel(std::ifstream &is)
{
	is >> enemyLevel;
	if (enemyLevel < 1)
		enemyLevel = 1;
}

void ActorParams::SetBasicPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["level"]->text.setString(boost::lexical_cast<string>(enemyLevel));
	p->checkBoxes["monitor"]->checked = hasMonitor;
}



void ActorParams::SetPanelInfo()
{
	SetBasicPanelInfo();
}

void ActorParams::SetSelected( bool select )
{
	cout << "------selected: " << select << endl;
	selected = select;
}

bool ActorParams::CanApply()
{
	if (type->CanBeGrounded())
	{
		if (groundInfo != NULL)
			return true;
	}
	else if (type->CanBeAerial())
	{
		return true;
	}

	return false;
}

bool ActorParams::CanAdd()
{
	assert( false );
	return false; //shouldn't get called anyway
}

void ActorParams::Draw( sf::RenderTarget *target )
{
	//cout << "Selected: " << selected << endl;
	

	//temporary checks might make it lag less?

	DrawLevel(target);

	DrawMonitor( target );

	Vector2f viewCenter = target->getView().getCenter();
	Vector2f viewSize = target->getView().getSize();
	if( image.getGlobalBounds().intersects( FloatRect( viewCenter.x - viewSize.x / 2, viewCenter.y - viewSize.y / 2,
		viewSize.x, viewSize.y ) ) )
	{
		target->draw( image );
	}

	DrawBoundary( target );
}

void ActorParams::DrawPreview( sf::RenderTarget *target )
{
}

void ActorParams::DrawMonitor( sf::RenderTarget *target )
{
	

	if( hasMonitor )
	{
		double w = image.getLocalBounds().width;
		double h = image.getLocalBounds().height;

		sf::CircleShape cs;
		cs.setRadius( max( w, h ) );

		cs.setFillColor( Color::White );

		cs.setOrigin( cs.getLocalBounds().width / 2, 
			cs.getLocalBounds().height / 2 );
		cs.setPosition( position.x, position.y );

		target->draw( cs );
	}

	
	
}

void ActorParams::WriteFile( ofstream &of )
{
	of << type->info.name << " ";

	bool canGrounded = type->CanBeGrounded();
	bool canAerial = type->CanBeAerial();
	
	int air = 0;
	int ground = 1;

	if( canGrounded && canAerial )
	{
		if( groundInfo != NULL )
		{
			int edgeIndex = groundInfo->GetEdgeIndex();
			//of << "-air" << " " << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
			of << ground << " " << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
		}
		else
		{
			of << air << " " << position.x << " " << position.y << endl;
		}
	}
	else if( canGrounded )
	{
		assert( groundInfo != NULL );

		int edgeIndex = groundInfo->GetEdgeIndex();

		of << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
	}
	else if( canAerial )
	{
		of << position.x << " " << position.y << endl;
	}
	else
	{
		assert( false );
	}
	
	WriteParamFile( of );
}

void ActorParams::DrawQuad( sf::RenderTarget *target )
{
	target->draw( boundingQuad );
}

GroundInfo::GroundInfo()
	:edgeStart( NULL ), groundQuantity( -1 ), ground( NULL )
{

}

V2d GroundInfo::GetPosition()
{
	V2d start = V2d( edgeStart->pos );
	V2d end;
	if (edgeStart->next == NULL)
	{
		end = V2d( ground->pointStart->pos);
	}
	else
	{
		end = V2d(edgeStart->next->pos );
	}

	V2d dir = normalize(end - start);
	return start + dir * groundQuantity;
}

int GroundInfo::GetEdgeIndex()
{
	int index = 0;
	for( TerrainPoint *curr = ground->pointStart; curr != NULL; curr = curr->next )
	{
		if( curr == edgeStart )
			return index;
		++index;
	}

	
	//assert( false && "could not find correct edge index" );
	return -1;
}

void ActorParams::SetBoundingQuad()
{
	//float note
	int width = type->info.size.x;
	int height = type->info.size.y;

	if( type->CanBeGrounded() && groundInfo != NULL )
	{
		V2d v0( (*groundInfo->edgeStart).pos.x, (*groundInfo->edgeStart).pos.y );
		TerrainPoint *edgeEnd = groundInfo->edgeStart->next;
		if( edgeEnd == NULL )
			edgeEnd = groundInfo->ground->pointStart;
		V2d v1( edgeEnd->pos.x, edgeEnd->pos.y );
		V2d along = normalize( v1 - v0 );
		V2d other( along.y, -along.x );
		

		V2d startGround = v0 + along * groundInfo->groundQuantity;
		V2d leftGround = startGround - along * ( width / 2.0) + other * 1.0;
		V2d rightGround = startGround + along * ( width / 2.0) + other * 1.0;
		V2d leftAir = leftGround + other * (double)(height - 1) ;
		V2d rightAir = rightGround + other * (double)(height - 1 );

		/*if( type->name == "poi" )
		{
			leftGround = startGround - along * ( type->width / 2.0 ) - other * ( type->height / 2.0 );
			rightGround = startGround + along * ( type->width / 2.0 ) - other * ( type->height / 2.0 );
			leftAir = leftGround + other * ( type->height / 2.0 );
			rightAir = rightGround + other * ( type->height / 2.0 );
		}*/

		boundingQuad[0].position = Vector2f( leftGround.x, leftGround.y );
		boundingQuad[1].position = Vector2f( leftAir.x, leftAir.y );
		boundingQuad[2].position = Vector2f( rightAir.x, rightAir.y );
		boundingQuad[3].position = Vector2f( rightGround.x, rightGround.y );

		V2d pos = (leftGround + leftAir + rightAir + rightGround ) / 4.0;
		position = Vector2i( pos.x, pos.y );
	}
	else
	{
		//patroller doesnt need a box because its not physical with the environment
		boundingQuad[0].position = Vector2f( position.x - width / 2, position.y - height / 2);
		boundingQuad[1].position = Vector2f( position.x + width / 2, position.y - height / 2);
		boundingQuad[2].position = Vector2f( position.x + width / 2, position.y + height / 2);
		boundingQuad[3].position = Vector2f( position.x - width / 2, position.y + height / 2);
	}

	UpdateExtraVisuals();
}

void ActorParams::UpdateGroundedSprite()
{	
	assert( groundInfo != NULL && groundInfo->ground != NULL );
	
	TerrainPoint *edge = groundInfo->edgeStart;
	TerrainPoint *next = edge->next;
	if( next == NULL )
		next = groundInfo->ground->pointStart;

	

	V2d pr( edge->pos.x, edge->pos.y );
	V2d cu( next->pos.x, next->pos.y );


	//this shouldn't remain here. i need more detailed checking.
	double groundLength = length( pr - cu );
	int width = type->info.size.x;

	if( groundInfo->groundQuantity + width / 2 > groundLength )
	{
		groundInfo->groundQuantity = groundLength - width / 2;
	}
	else if( groundInfo->groundQuantity - width / 2 < 0 )
	{
		groundInfo->groundQuantity = width / 2;
	}

	V2d newPoint( pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
									(groundInfo->groundQuantity / length( cu - pr ) ) );

	double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;
	image.setPosition( newPoint.x, newPoint.y );
	image.setRotation( angle );
}

void ActorParams::AnchorToGround( TerrainPolygon *poly, int edgeIndex, double quantity )
{
	assert( groundInfo == NULL );
	/*if( groundInfo != NULL )
	{
		cout << "unanchor" << endl;
		
		//UnAnchor( actor );
		cout << "end unanchor" << endl;
		
		//delete groundInfo;
		//groundInfo = NULL;
	}*/



	groundInfo = new GroundInfo;
	
	groundInfo->ground = poly;
	
	groundInfo->groundQuantity = quantity;
	
	int testIndex = 0;

	image = type->GetSprite(true);// .setTexture(*type->ts_image->texture);	

	Vector2i point;

	TerrainPoint *prev = groundInfo->ground->pointEnd;
	TerrainPoint *curr = groundInfo->ground->pointStart;

	for( ; curr != NULL; curr = curr->next )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( prev->pos.x, prev->pos.y );
			V2d cu( curr->pos.x, curr->pos.y );

			V2d newPoint( pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(groundInfo->groundQuantity / length( cu - pr ) ) );

			double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

			groundInfo->edgeStart = prev;

			
			UpdateGroundedSprite();
			SetBoundingQuad();

			break;
		}
		prev = curr;
		++testIndex;
	}

	
}

void ActorParams::AnchorToGround( GroundInfo &gi )
{
	groundInfo = new GroundInfo;
	*groundInfo = gi;

	image = type->GetSprite(true);

	UpdateGroundedSprite();
	SetBoundingQuad();
}

void ActorParams::UnAnchor( ActorPtr &actor )
{
	assert( groundInfo != NULL );
	//if( groundInfo == NULL )
	//	return;

	if( groundInfo != NULL )
	{
		position = Vector2i( image.getPosition().x, image.getPosition().y );

		image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
		image.setRotation( 0 );
		//groundInfo->ground->enemies.clear();
		//i get deleted when i remove myself from the terrain polygon????

		//group->actors.front().use_count();

		//ActorPtr actor( this );
		//group->actors.
		//cout << "use count: " << actor.use_count() << endl;
		
		//groundInfo->ground->enemies.clear();
		
		//cout << 
		//SelectPtr select = boost::dynamic_pointer_cast<ISelectable>		
		groundInfo->ground->enemies[groundInfo->edgeStart].remove( actor );
		//cout << "After remove" << endl;

		//cout << "use count: " << actor.use_count() << endl;

		delete groundInfo;
		groundInfo = NULL;

		//cout << "About to bound" << endl;
		SetBoundingQuad();
		//cout << "just bounded" << endl;
		//image.setPosition( 
	}
}

void ActorParams::DrawBoundary( sf::RenderTarget *target )
{
	if( selected )
	{
		sf::RectangleShape rs;
		rs.setFillColor( Color::Transparent );
		rs.setOutlineColor( Color::Green );
		rs.setOutlineThickness( 3 * EditSession::zoomMultiple );
		rs.setPosition( image.getGlobalBounds().left, image.getGlobalBounds().top );
		rs.setSize( Vector2f( image.getGlobalBounds().width, image.getGlobalBounds().height ) );
		target->draw( rs );
		//cout << "selected draw" << endl;
	}
}

void ActorParams::DrawLevel(sf::RenderTarget *target)
{
	if (type->info.numLevels > 1 && enemyLevel > 1 )
	{
		int extra = 10;
		sf::RectangleShape rs;
		rs.setFillColor(Color::Transparent);
		rs.setOutlineColor(Color( 255, 0, 0, 100));
		rs.setOutlineThickness(3 * EditSession::zoomMultiple);
		rs.setPosition(image.getGlobalBounds().left - extra, image.getGlobalBounds().top - extra);
		rs.setSize(Vector2f(image.getGlobalBounds().width + extra, image.getGlobalBounds().height + extra));
		target->draw(rs);
	}
	if (type->info.numLevels > 2 && enemyLevel > 2)
	{
		int extra = 20;
		sf::RectangleShape rs;
		rs.setFillColor(Color::Transparent);
		rs.setOutlineColor(Color(100, 100, 255, 100));
		rs.setOutlineThickness(3 * EditSession::zoomMultiple);
		rs.setPosition(image.getGlobalBounds().left - extra, image.getGlobalBounds().top - extra);
		rs.setSize(Vector2f(image.getGlobalBounds().width + extra, image.getGlobalBounds().height + extra));
		target->draw(rs);
	}
}

bool ActorParams::ContainsPoint( sf::Vector2f test )
{ 
	sf::Transform trans = image.getTransform();


	/*V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );*/	

	
	V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );

	bool result = QuadContainsPoint( A,B,C,D, V2d( test.x, test.y ) );

	//cout << "result: " << result << endl;
	return result;
}

bool ActorParams::Intersects( sf::IntRect rect )
{
	V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );

	V2d rA( rect.left, rect.top );
	V2d rB( rect.left + rect.width, rect.top );
	V2d rC( rect.left + rect.width, rect.top + rect.height );
	V2d rD( rect.left, rect.top + rect.height );
	if( isQuadTouchingQuad( A, B, C, D, rA, rB, rC, rD ) )
	{
		return true;
	}
	else
		return false;
}

bool ActorParams::IsPlacementOkay()
{
	return false;
}

void ActorParams::Move( SelectPtr me, sf::Vector2i delta )
{
	//assert( groundInfo == NULL );

	//ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( me );
	//if( groundInfo != NULL )
	//{
	//	UnAnchor( actor );
	//}
	if( groundInfo == NULL )
	{
		position.x += delta.x;
		position.y += delta.y;

		SetBoundingQuad();

		image.setPosition( position.x, position.y );
	}
	
	
	/*if( session->worldPosGround.ground != NULL )
	{
		
		AnchorToGround( session->worldPosGround.ground, session->worldPosGround.GetEdgeIndex(), session->worldPosGround.groundQuantity );
		
		SetBoundingQuad();
		UpdateGroundedSprite();
		//session->editStartMove = false;

		//p->enemies[groundInfo->edgeStart].push_back( actor );
		//p->UpdateBounds();
		//cout << "new end" << endl;
	}
	else
	{
		SetBoundingQuad();

		image.setPosition( position.x, position.y );
	}*/
}

void ActorParams::BrushDraw( sf::RenderTarget *target, 
		bool valid )
{
	image.setColor( Color( 255, 255, 255, 100 ) );
	target->draw( image );
	image.setColor( Color::White );
}

void ActorParams::Deactivate( EditSession *editsession, SelectPtr select )
{
	cout << "DEACTIVATING ACTOR PARAMS size from: " << group->actors.size() << endl;
	ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( select );
	group->actors.remove( actor );

	
	if( actor->groundInfo != NULL )
	{
		actor->groundInfo->ground
			->enemies[actor->groundInfo->edgeStart].remove( actor );
	}
}

void ActorParams::Activate(EditSession *editsession, SelectPtr select )
{
	cout << "addding to group of size: " << group->actors.size() << endl;
	ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( select );
	group->actors.push_back( actor );

	if( actor->groundInfo != NULL )
	{
		actor->groundInfo->ground
			->enemies[actor->groundInfo->edgeStart].push_back( actor );
	}
}

GoalParams::GoalParams(ActorType *at, std::ifstream &is)
	:ActorParams( at)
{
	LoadGrounded(is);
}

GoalParams::GoalParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at)
{
	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}


void GoalParams::SetParams()
{

}

void GoalParams::SetPanelInfo()
{

}

void GoalParams::WriteParamFile(std::ofstream &of)
{

}

ActorParams *GoalParams::Copy()
{
	GoalParams *copy = new GoalParams( *this );
	return copy;
}




//remnove the postype thing. we have 2 bools for that already
PlayerParams::PlayerParams(ActorType *at, sf::Vector2i pos )
	:ActorParams(at)
{
	PlaceAerial(pos);
}

PlayerParams::PlayerParams(ActorType *at, ifstream &is )
	:ActorParams(at)
{
	LoadAerial(is);
}

bool PlayerParams::CanApply()
{
	EditSession *session = EditSession::GetSession();
	sf::IntRect me( position.x - image.getLocalBounds().width / 2, position.y - image.getLocalBounds().height / 2, 
		image.getLocalBounds().width, image.getLocalBounds().height );
	for( list<PolyPtr>::iterator it = session->polygons.begin(); it != session->polygons.end(); ++it )
	{
		
		if( (*it)->Intersects( me ) )
		{
			return false;
		}
	}
	return true;
}

void PlayerParams::Deactivate(EditSession *editsession, boost::shared_ptr<ISelectable> select)
{
	//nothing
}

void PlayerParams::Activate(EditSession *editsession, boost::shared_ptr<ISelectable> select )
{
	//nothing
}

ActorParams *PlayerParams::Copy()
{
	assert( false );
	return NULL;
}


PoiParams::PoiParams(ActorType *at,
	TerrainPolygon *p_edgePolygon,
	int p_edgeIndex, 
	double p_edgeQuantity )
	:ActorParams(at),
	barrier( NONE )
{
	

	hasCamProperties = false;
	camZoom = 1;

	nameText.setFont( *font );
	nameText.setCharacterSize( 18 );
	nameText.setFillColor( Color::White );
	
	name = "-";


	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

PoiParams::PoiParams(ActorType *at,
	std::ifstream &is)
	:ActorParams( at )
{
	EditSession *edit = EditSession::GetSession();
	string air;
	is >> air;
	Vector2i pos;

	if (air == "+air")
	{
		LoadAerial(is);

		string pname;
		is >> pname;

		string marker;
		is >> marker;

		PoiParams::Barrier b;
		if (marker == "-")
		{
			b = PoiParams::NONE;
		}
		else if (marker == "x")
		{
			b = PoiParams::X;
		}
		else if (marker == "y")
		{
			b = PoiParams::Y;
		}

		int hasCamProps;
		is >> hasCamProps;

		float camZoom = 1;
		if (hasCamProps)
		{
			is >> camZoom;
		}

		camRect.setFillColor(Color::Transparent);
		camRect.setOutlineColor(Color::Red);
		camRect.setOutlineThickness(10);
		camRect.setSize(Vector2f(960, 540));
		camRect.setOrigin(camRect.getLocalBounds().width / 2,
			camRect.getLocalBounds().height / 2);

		nameText.setFont(*font);
		nameText.setCharacterSize(18);
		nameText.setFillColor(Color::White);

		hasCamProperties = false;
		camZoom = 1;

		name = "-";
	}
	else if (air == "-air")
	{
		LoadGrounded(is);

		string pname;
		is >> pname;

		string marker;
		is >> marker;

		PoiParams::Barrier b;
		if (marker == "-")
		{
			b = PoiParams::NONE;
		}
		else if (marker == "x")
		{
			b = PoiParams::X;
		}
		else if (marker == "y")
		{
			b = PoiParams::Y;
		}


		hasCamProperties = false;
		camZoom = 1;

		nameText.setFont(*font);
		nameText.setCharacterSize(18);
		nameText.setFillColor(Color::White);

		name = "-";
	}
	else
	{
		assert(0);
	}
}

PoiParams::PoiParams(ActorType *at,
	TerrainPolygon *p_edgePolygon,
	int p_edgeIndex, 
	double p_edgeQuantity, PoiParams::Barrier bType, const std::string &p_name )
	:ActorParams(at),
	barrier( bType ), name( p_name )
{
	hasCamProperties = false;
	camZoom = 1;

	nameText.setFont( *font );
	nameText.setCharacterSize( 18 );
	nameText.setFillColor( Color::White );

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

PoiParams::PoiParams(ActorType *at,
	sf::Vector2i &pos )
	:ActorParams(at), barrier( NONE )
{
	camRect.setFillColor( Color::Transparent );
	camRect.setOutlineColor( Color::Red );
	camRect.setOutlineThickness( 10 );
	camRect.setSize( Vector2f( 960, 540 ) );
	camRect.setOrigin( camRect.getLocalBounds().width / 2,
		camRect.getLocalBounds().height / 2 );

	nameText.setFont( *font );
	nameText.setCharacterSize( 18 );
	nameText.setFillColor( Color::White );

	hasCamProperties = false;
	camZoom = 1;

	name = "-";

	PlaceAerial(pos);
}

PoiParams::PoiParams(ActorType *at,
	sf::Vector2i &pos, PoiParams::Barrier bType, const std::string &p_name,
	bool hasCam, float cZoom )
	:ActorParams(at), 
	barrier( bType ), name( p_name ), hasCamProperties( hasCam ),
	camZoom( cZoom )
{
	camRect.setFillColor( Color::Transparent );
	camRect.setOutlineColor( Color::Red );
	camRect.setOutlineThickness( 10 );
	camRect.setSize( Vector2f( 960 * cZoom, 540 * cZoom ) );
	camRect.setOrigin( camRect.getLocalBounds().width / 2,
		camRect.getLocalBounds().height / 2 );


	nameText.setFont( *font );
	nameText.setCharacterSize( 18 );
	nameText.setFillColor( Color::White );

	PlaceAerial(pos);
}

ActorParams *PoiParams::Copy()
{
	PoiParams *copy = new PoiParams( *this );
	return copy;
}

void PoiParams::WriteParamFile( std::ofstream &of )
{
	of << name << " ";

	switch( barrier )
	{
	case NONE:
		of << "-" << endl;
		break;
	case X:
		of << "x" << endl;
		break;
	case Y:
		of << "y" << endl;
		break;
	}

	if( groundInfo == NULL )
	{
		WriteBool(of, hasCamProperties);
		if( hasCamProperties )
			of << camZoom << endl;
	}
}

void PoiParams::SetParams()
{
	Panel *p = type->panel;
	
	bool camProps = p->checkBoxes["camprops"]->checked;

	hasCamProperties = camProps;

	name = p->textBoxes["name"]->text.getString().toAnsiString();

	nameText.setString( name );

	stringstream ss;
	string zoomStr = p->textBoxes["camzoom"]->text.getString().toAnsiString();

	ss << zoomStr;

	float zoom;
	ss >> zoom;

	if( !ss.fail() )
	{
		camZoom = zoom;

		camRect.setSize( Vector2f( 960.f * camZoom, 540.f * camZoom ) );
		camRect.setOrigin( camRect.getLocalBounds().width / 2, camRect.getLocalBounds().height / 2 );
	}

	string barStr = p->textBoxes["barrier"]->text.getString().toAnsiString();
	
	if( barStr == "-" )
	{
		barrier = Barrier::NONE;
	}
	else if( barStr == "x" )
	{
		barrier = Barrier::X;
	}
	else if( barStr == "y" )
	{
		barrier = Barrier::Y;
	}
	else
	{
		//do nothing
	}
}

void PoiParams::SetPanelInfo()
{
	Panel *p = type->panel;

	string s;
	switch( barrier )
	{
	case NONE:
		s = "-";
		break;
	case X:
		s = "x";
		break;
	case Y:
		s = "y";
		break;
	}

	p->checkBoxes["camprops"]->checked = hasCamProperties;
	
	p->textBoxes["camzoom"]->text.setString( boost::lexical_cast<string>( camZoom ) );

	p->textBoxes["name"]->text.setString( name );
	if( group != NULL )
	{
		//p->textBoxes["group"]->text.setString( group->name );
	}

	p->textBoxes["barrier"]->text.setString( s );
}

void PoiParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );

	nameText.setString( name );
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2, 
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2 );
	nameText.setPosition( position.x, position.y - 40 );

	target->draw( nameText );

	if( hasCamProperties )
	{
		camRect.setPosition( position.x, position.y );
		target->draw( camRect );
	}
}

KeyParams::KeyParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at)
{
	PlaceAerial(pos);
	
	numKeys = 3;
	zoneType = 0;
}

KeyParams::KeyParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	is >> numKeys;
	is >> zoneType;
}

KeyParams::KeyParams(ActorType *at, sf::Vector2i &pos,
	int p_numKeys, int p_zoneType )
	:ActorParams(at)
{
	PlaceAerial(pos);
	
	numKeys = p_numKeys;
	zoneType = p_zoneType;
}

void KeyParams::WriteParamFile( std::ofstream &of )
{
	of << numKeys << endl;
	of << zoneType << endl;
}

void KeyParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = false;//p->checkBoxes["monitor"]->checked;

	stringstream ss;
	string numkeysString = p->textBoxes["numkeys"]->text.getString().toAnsiString();
	string zoneTypeString =p->textBoxes["zonetype"]->text.getString().toAnsiString();

	ss << numkeysString;

	int numK;
	ss >> numK;

	if( !ss.fail() )
	{
		numKeys = numK;
	}

	ss.clear();

	ss << zoneTypeString;

	int zt;
	ss >> zt;

	if (!ss.fail())
	{
		zoneType = zt;
	}
	
}

void KeyParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->textBoxes["numkeys"]->text.setString( boost::lexical_cast<string>( numKeys ) );

	p->textBoxes["zonetype"]->text.setString(boost::lexical_cast<string>(zoneType));
}

ActorParams *KeyParams::Copy()
{
	KeyParams *copy = new KeyParams( *this );
	return copy;
}

NexusParams::NexusParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity,
	int p_nexusIndex )
	:ActorParams(at), nexusIndex( p_nexusIndex )
{
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

NexusParams::NexusParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams(at)
{
	nexusIndex = 0;
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

NexusParams::NexusParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	is >> nexusIndex;
}

void NexusParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	//p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["nexusindex"]->text.setString( boost::lexical_cast<string>( nexusIndex ) );
}

void NexusParams::SetParams()
{
	Panel *p = type->panel;

	
	int index;

	stringstream ss;
	string s = p->textBoxes["nexusindex"]->text.getString().toAnsiString();
	ss << s;


	ss >> index;

	if( !ss.fail() )
	{
		nexusIndex = index;
	}
}

void NexusParams::WriteParamFile( ofstream &of )
{
	of << nexusIndex << endl;
}

ActorParams *NexusParams::Copy()
{
	NexusParams *copy = new NexusParams( *this );
	return copy;
}

GroundTriggerParams::GroundTriggerParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity,
	bool fr, const std::string &p_typeStr)
	:ActorParams(at), facingRight( fr ), typeStr( p_typeStr )
{
	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

GroundTriggerParams::GroundTriggerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	LoadBool(is, facingRight);

	is >> typeStr;
}

GroundTriggerParams::GroundTriggerParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at)
{
	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
	typeStr = "NONE";
	facingRight = true;

	
}

void GroundTriggerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	//p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["triggertype"]->text.setString(typeStr);
	p->checkBoxes["facingright"]->checked = facingRight;
	
}

void GroundTriggerParams::SetParams()
{
	Panel *p = type->panel;

	string s = p->textBoxes["triggertype"]->text.getString().toAnsiString();

	typeStr = s;

	bool right = p->checkBoxes["facingright"]->checked;
}

void GroundTriggerParams::WriteParamFile(ofstream &of)
{
	WriteBool(of, facingRight);
	of << typeStr << endl;
}

ActorParams *GroundTriggerParams::Copy()
{
	GroundTriggerParams *copy = new GroundTriggerParams(*this);
	return copy;
}

ShipPickupParams::ShipPickupParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity,
	bool p_facingRight )
	:ActorParams(at), facingRight( p_facingRight )
{
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

ShipPickupParams::ShipPickupParams(ActorType *at, ifstream &is )
	:ActorParams(at)
{
	LoadGrounded(is);
	LoadBool(is, facingRight);
}

ShipPickupParams::ShipPickupParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams(at)
{
	facingRight = true;
	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

void ShipPickupParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->checkBoxes["facingright"]->checked = facingRight;
	//p->textBoxes["facing"]->text.setString( boost::lexical_cast<string>( nexusIndex ) );
}

void ShipPickupParams::SetParams()
{
	Panel *p = type->panel;

	
	int index;


	bool right = p->checkBoxes["facingright"]->checked;

	facingRight = right;
}

void ShipPickupParams::WriteParamFile( ofstream &of )
{
	WriteBool(of, facingRight);
}

ActorParams *ShipPickupParams::Copy()
{
	ShipPickupParams *copy = new ShipPickupParams( *this );
	return copy;
}

ShardParams::ShardParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at)
{
	PlaceAerial(pos);
	SetShard(0, 0, 0);
}

void ShardParams::SetShard(int w, int li)
{
	SetShard(w, li % 11, li / 11);
}

void ShardParams::SetShard(int w, int realX, int realY)
{
	world = w;
	sX = realX;
	sY = realY;
	EditSession *session = EditSession::GetSession();
	localIndex = realX + realY * 11;
	Tileset *ts = session->ts_shards[world];
	image.setTexture(*ts->texture);
	image.setTextureRect(ts->GetSubRect(localIndex));
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);

	Panel *p = type->panel;
	GridSelector *gs = p->gridSelectors["shardselector"];
	gs->selectedX = sX;
	gs->selectedY = sY + world * 2;

	shardStr = gs->names[gs->selectedX][gs->selectedY];
}

int ShardParams::GetTotalIndex()
{
	return world * 22 + localIndex;
}

ShardParams::ShardParams(ActorType *at, sf::Vector2i &pos, int p_world,
	int p_localIndex)
	:ActorParams(at)
{
	PlaceAerial(pos);

	SetShard(p_world, p_localIndex);
	//SetShardFromStr();
}

ShardParams::ShardParams(ActorType *at,ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	int w;
	is >> w;

	int li;
	is >> li;

	SetShard(w, li);
}

void ShardParams::SetShardFromStr()
{
	/*ShardType st = Shard::GetShardType(shardStr);
	int sti = st;
	int rem = sti % 21;

	SetShard(sti / 21, rem % 3, rem / 3);*/
}

void ShardParams::WriteParamFile( std::ofstream &of )
{
	of << world << " " << localIndex << endl;
}

void ShardParams::SetParams()
{
	Panel *p = type->panel;

	shardStr = p->labels["shardtype"]->getString();
}

void ShardParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->labels["shardtype"]->setString( shardStr );

	GridSelector *gs = p->gridSelectors["shardselector"];
	gs->selectedX = sX;
	gs->selectedY = sY + world * 2;
}

ActorParams *ShardParams::Copy()
{
	ShardParams *copy = new ShardParams( *this );
	return copy;
}

RaceFightTargetParams::RaceFightTargetParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at)
{
	PlaceAerial(pos);
}

RaceFightTargetParams::RaceFightTargetParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
}

void RaceFightTargetParams::SetParams()
{
	//Panel *p = type->panel;

	//hasMonitor = p->checkBoxes["monitor"]->checked;
}

void RaceFightTargetParams::SetPanelInfo()
{
	//Panel *p = type->panel;

	/*p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->checkBoxes["monitor"]->checked = hasMonitor;*/
}

ActorParams *RaceFightTargetParams::Copy()
{
	RaceFightTargetParams *copy = new RaceFightTargetParams( *this );
	return copy;
}

BlockerParams::BlockerParams(ActorType *at, sf::Vector2i pos, list<sf::Vector2i> &globalPath, int p_bType, bool p_armored,
	int p_spacing, int p_level )
	:ActorParams(at)
{
	lines = NULL;
	
	PlaceAerial(pos);

	spacing = p_spacing;
	SetPath(globalPath);

	bType = (BlockerType)p_bType;

	armored = p_armored;


	enemyLevel = p_level;
}

BlockerParams::BlockerParams(ActorType *at,ifstream &is)
	:ActorParams(at)
{
	lines = NULL;
	LoadAerial(is);
	
	LoadGlobalPath(is);

	int ibType;
	is >> ibType;
	bType = (BlockerType)ibType;

	LoadBool(is, armored);

	is >> spacing;

	is >> enemyLevel;
	
}

BlockerParams::BlockerParams(ActorType *at,
	sf::Vector2i &pos)
	:ActorParams(at)
{
	lines = NULL;
	PlaceAerial(pos);
	armored = false;

	bType = NORMAL;

	spacing = 0;
}

void BlockerParams::SetParams()
{
	Panel *p = type->panel;

	armored = p->checkBoxes["armored"]->checked;


	string typeStr = p->textBoxes["btype"]->text.getString().toAnsiString();

	stringstream ss;
	ss << typeStr;

	int t_type;
	ss >> t_type;

	if (!ss.fail())
	{
		bType = (BlockerType)t_type;
	}

	hasMonitor = false;

	string spacingStr = p->textBoxes["spacing"]->text.getString().toAnsiString();

	
	ss << spacingStr;

	int t_spacing;
	ss >> t_spacing;

	if (!ss.fail())
	{
		spacing = t_spacing;
	}
	//hasMonitor = p->checkBoxes["monitor"]->checked;
	//try
	//{
	//	speed = boost::lexical_cast<int>( p->textBoxes["speed"]->text.getString().toAnsiString() );
	//}
	//catch(boost::bad_lexical_cast &)
	//{
	//	//error
	//}
}

void BlockerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	p->textBoxes["btype"]->text.setString(boost::lexical_cast<string>(bType));
	p->checkBoxes["armored"]->checked = armored;
	p->textBoxes["spacing"]->text.setString(boost::lexical_cast<string>(spacing));

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}

void BlockerParams::Draw(sf::RenderTarget *target)
{
	int localPathSize = localPath.size();

	if (localPathSize > 0)
	{
		VertexArray &li = *lines;


		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position += Vector2f(position.x, position.y);
		}


		target->draw(li);



		//if (loop)
		//{

		//	//draw the line between the first and last
		//	sf::Vertex vertices[2] =
		//	{
		//		sf::Vertex(li[localPathSize].position, Color::Magenta),
		//		sf::Vertex(li[0].position, Color::White)
		//	};

		//	target->draw(vertices, 2, sf::Lines);
		//}


		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position -= Vector2f(position.x, position.y);
		}
	}

	ActorParams::Draw(target);
}

void BlockerParams::WriteParamFile(ofstream &of)
{
	WritePath(of);

	of << bType << "\n";

	WriteBool(of, armored);
	
	of << spacing << endl;

	of << level << endl;
}

ActorParams *BlockerParams::Copy()
{
	BlockerParams *bp = new BlockerParams(*this);
	return bp;
}


AirTriggerParams::AirTriggerParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	PlaceAerial(pos);
	triggerRect.setFillColor(Color(200, 200, 200, 150));

	rectWidth = 50;
	rectHeight = 50;
	SetRect(rectWidth, rectHeight, pos);

	trigType = "none";//"..no.shard..";
}

AirTriggerParams::AirTriggerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> trigType;

	is >> rectWidth;

	is >> rectHeight;

	triggerRect.setFillColor(Color(200, 200, 200, 150));

	SetRect(rectWidth, rectHeight, position);
}

AirTriggerParams::AirTriggerParams(ActorType *at, sf::Vector2i &pos, const std::string &typeStr, int w,int h)
	:ActorParams(at)
{
	PlaceAerial(pos);

	triggerRect.setFillColor(Color(200, 200, 200, 150));//Color::Transparent);
	//triggerRect.setOutlineColor(Color::Red);
	//triggerRect.setOutlineThickness(10);
	
	rectWidth = w;
	rectHeight = h;

	SetRect( w, h, pos);

	trigType = typeStr;
}

void AirTriggerParams::SetRect(int width, int height, Vector2i &center)
{
	triggerRect.setSize(Vector2f(width, height));
	triggerRect.setOrigin(triggerRect.getLocalBounds().width / 2,
		triggerRect.getLocalBounds().height / 2);
	position = center;
	triggerRect.setPosition(position.x, position.y);
	image.setPosition(position.x, position.y);
	SetBoundingQuad();

	rectWidth = width;
	rectHeight = height;
}

void AirTriggerParams::WriteParamFile(std::ofstream &of)
{
	of << trigType << endl;
	of << rectWidth << endl;
	of << rectHeight << endl;
}

void AirTriggerParams::SetParams()
{
	Panel *p = type->panel;

	trigType = p->textBoxes["triggertype"]->text.getString();
}

void AirTriggerParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	//p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["triggertype"]->text.setString(trigType);
}

ActorParams *AirTriggerParams::Copy()
{
	AirTriggerParams *copy = new AirTriggerParams(*this);
	return copy;
}

void AirTriggerParams::Draw(RenderTarget *target)
{
	ActorParams::Draw(target);

	nameText.setString(trigType);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);
	nameText.setPosition(position.x, position.y - 40);

	target->draw(nameText);

	triggerRect.setPosition(position.x, position.y);
	target->draw(triggerRect);
	
}

FlowerPodParams::FlowerPodParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity,
	const std::string &p_typeStr)
	:ActorParams(at), facingRight(true), typeStr(p_typeStr)
{
	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

FlowerPodParams::FlowerPodParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	is >> typeStr;
}

FlowerPodParams::FlowerPodParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at)
{
	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);

	typeStr = "NONE";
	facingRight = true;
}

void FlowerPodParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	//p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["podtype"]->text.setString(typeStr);
}

void FlowerPodParams::SetParams()
{
	Panel *p = type->panel;

	string s = p->textBoxes["podtype"]->text.getString().toAnsiString();

	typeStr = s;

	//bool right = p->checkBoxes["facingright"]->checked;
}

void FlowerPodParams::WriteParamFile(ofstream &of)
{
	//of << (int)facingRight << endl;
	of << typeStr << endl;
}

ActorParams *FlowerPodParams::Copy()
{
	FlowerPodParams *copy = new FlowerPodParams(*this);
	return copy;
}

template<typename X> ActorParams *MakeParams(
	ActorType *type)
//TerrainPolygon *tp, int edgeIndex,
//double quant, sf::Vector2i worldPos)
{
	EditSession *edit = EditSession::GetSession();
	if (type->canBeGrounded)
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			return new X(edit->enemyEdgePolygon,
				edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);
		}
	}
	else if( type->canBeAerial )
	{
		return new X(sf::Vector2i(edit->worldPos));
	}

	return NULL;
}

BasicGroundEnemyParams::BasicGroundEnemyParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity,
	int level)
	:ActorParams(at)
{
	enemyLevel = level;
	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

BasicGroundEnemyParams::BasicGroundEnemyParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	ParamsInfo &pi = at->info;

	if (pi.writeMonitor)
	{
		LoadMonitor(is);
	}

	if (pi.writeLevel)
	{
		LoadEnemyLevel(is);
	}
}

void BasicGroundEnemyParams::WriteParamFile(std::ofstream &of)
{
	ParamsInfo &pi = type->info;

	if (pi.writeMonitor)
	{
		WriteMonitor(of);
	}

	if (pi.writeLevel)
	{
		WriteLevel(of);
	}
}

ActorParams *BasicGroundEnemyParams::Copy()
{
	BasicGroundEnemyParams *copy = new BasicGroundEnemyParams(*this);
	return copy;
}

BasicAirEnemyParams::BasicAirEnemyParams(ActorType *at, sf::Vector2i &pos, int level )
	:ActorParams(at)
{
	enemyLevel = level;
	PlaceAerial(pos);
}

BasicAirEnemyParams::BasicAirEnemyParams(ActorType *at, ifstream &is)
	: ActorParams(at)
{
	LoadAerial(is);

	ParamsInfo &pi = at->info;
	if (pi.writeMonitor)
	{
		LoadMonitor(is);
	}
	
	if (pi.writePath)
	{
		LoadGlobalPath(is);
	}

	if (pi.writeLoop)
	{
		LoadBool(is, loop);
	}

	if (pi.writeLevel)
	{
		LoadEnemyLevel(is);
	}
}

void BasicAirEnemyParams::WriteParamFile(std::ofstream &of)
{
	ParamsInfo &pi = type->info;
	if (pi.writeMonitor)
	{
		WriteMonitor(of);
	}

	if (pi.writePath)
	{
		WritePath(of);
	}

	if (pi.writeLoop)
	{
		WriteLoop(of);
	}

	if (pi.writeLevel)
	{
		WriteLevel(of);
	}
}

void BasicAirEnemyParams::SetPanelInfo()
{
	//SetBasicPanelInfo();

	Panel *p = type->panel;

	ParamsInfo &pi = type->info;
	if (pi.writeLevel)
	{
		p->textBoxes["level"]->text.setString(boost::lexical_cast<string>(enemyLevel));
	}

	if (pi.writeMonitor)
	{
		p->checkBoxes["monitor"]->checked = hasMonitor;
	}
	
	if (pi.writePath)
	{
		EditSession *edit = EditSession::GetSession();
		edit->patrolPath = GetGlobalPath();
	}

	if (pi.writeLoop)
	{
		p->checkBoxes["loop"]->checked = loop;
	}
}

void BasicAirEnemyParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;

	ParamsInfo &pi = type->info;

	if (pi.writeLevel)
	{
		int level;
		string s = p->textBoxes["level"]->text.getString().toAnsiString();
		ss << s;

		ss >> level;

		if (!ss.fail() && level > 0 && level <= pi.numLevels)
		{
			enemyLevel = level;
		}
	}
	
	if (pi.writeMonitor)
	{
		hasMonitor = p->checkBoxes["monitor"]->checked;
	}
}

ActorParams *BasicAirEnemyParams::Copy()
{
	BasicAirEnemyParams *copy = new BasicAirEnemyParams(*this);
	return copy;
}

//AirPathEnemyParamsLoop::AirPathEnemyParamsLoop(ActorType *at, sf::Vector2i &pos, int level)
//	:ActorParams(at)
//{
//	enemyLevel = level;
//	PlaceAerial(pos);
//}
//
//AirPathEnemyParamsLoop::AirPathEnemyParamsLoop(ActorType *at, ifstream &is)
//	: ActorParams(at)
//{
//	LoadAerial(is);
//
//	LoadMonitor(is);
//
//	LoadGlobalPath(is);
//
//	LoadBool(is, loop);
//
//	LoadEnemyLevel(is);
//}
//
//ActorParams *AirPathEnemyParamsLoop::Copy()
//{
//	AirPathEnemyParamsLoop *copy = new AirPathEnemyParamsLoop(*this);
//	return copy;
//}
//
//void AirPathEnemyParamsLoop::WriteParamFile(std::ofstream &of)
//{
//	WriteMonitor(of);
//	WritePath(of);
//	WriteLoop(of);
//	WriteLevel(of);
//}
//
//void AirPathEnemyParamsLoop::SetPanelInfo()
//{
//	SetBasicPanelInfo();
//
//	Panel *p = type->panel;
//
//	EditSession *edit = EditSession::GetSession();
//	edit->patrolPath = GetGlobalPath();
//
//	p->checkBoxes["loop"]->checked = loop;
//}

void BasicAirEnemyParams::Draw(sf::RenderTarget *target)
{
	if (type->info.writePath)
	{
		int localPathSize = localPath.size();

		if (localPathSize > 0)
		{

			VertexArray &li = *lines;


			for (int i = 0; i < localPathSize + 1; ++i)
			{
				li[i].position += Vector2f(position.x, position.y);
			}


			target->draw(li);

			if (loop)
			{

				//draw the line between the first and last
				sf::Vertex vertices[2] =
				{
					sf::Vertex(li[localPathSize].position, Color::Magenta),
					sf::Vertex(li[0].position, Color::White)
				};

				target->draw(vertices, 2, sf::Lines);
			}


			for (int i = 0; i < localPathSize + 1; ++i)
			{
				li[i].position -= Vector2f(position.x, position.y);
			}
		}
	}

	ActorParams::Draw(target);
}