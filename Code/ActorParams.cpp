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

	for( int i = 0; i < 4; ++i )
		boundingQuad[i].color = Color( 0, 255, 0, 100);
}

void ActorParams::SetPath( std::list<sf::Vector2i> &globalPath )
{
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
	AnchorToGround(terrain, edgeIndex, edgeQuantity);
	SetBoundingQuad();
}

void ActorParams::LoadAerial(std::ifstream &is)
{
	is >> position.x;
	is >> position.y;

	image = type->GetSprite(false);
	image.setPosition(position.x, position.y);

	SetBoundingQuad();
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

void ActorParams::SetParams()
{
}

void ActorParams::SetPanelInfo()
{
}

void ActorParams::SetSelected( bool select )
{
	cout << "------selected: " << select << endl;
	selected = select;
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
	//if( params.size() == 0 )
	//{
	//	assert( false && "no params" );
	//}
	
	//dont need number of params because the actortype determines that.
	of << type->info.name << " ";

	bool canGrounded = type->CanBeGrounded();
	bool canAerial = type->CanBeAerial();
	if( canGrounded && canAerial )
	{
		if( groundInfo != NULL )
		{
			int edgeIndex = groundInfo->GetEdgeIndex();
			of << "-air" << " " << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
		}
		else
		{
			of << "+air" << " " << position.x << " " << position.y << endl;
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
	AnchorToGround(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
	SetBoundingQuad();
}


bool GoalParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void GoalParams::WriteParamFile( ofstream &of )
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
	position = pos;

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
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

void PlayerParams::WriteParamFile( std::ofstream &of )
{
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
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
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

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
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
	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
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

	//name = "-";
	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

bool PoiParams::CanApply()
{
	return true;
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
		of << (int)hasCamProperties << endl;
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
	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();

	
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
	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();

	
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

bool KeyParams::CanApply()
{
	return true;
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
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

NexusParams::NexusParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams(at)
{
	nexusIndex = 0;
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

NexusParams::NexusParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	is >> nexusIndex;
}

bool NexusParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
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
	AnchorToGround(p_edgePolygon, p_edgeIndex, p_edgeQuantity);

	SetBoundingQuad();
}

GroundTriggerParams::GroundTriggerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	int ifacingRight;
	is >> ifacingRight;
	facingRight = ifacingRight;

	is >> typeStr;
}

GroundTriggerParams::GroundTriggerParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at)
{
	AnchorToGround(p_edgePolygon, p_edgeIndex, p_edgeQuantity);

	typeStr = "NONE";
	facingRight = true;

	SetBoundingQuad();
}

bool GroundTriggerParams::CanApply()
{
	if (groundInfo != NULL)
		return true;
	//hmm not sure about this now

	return false;
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
	of << (int)facingRight << endl;
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
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

ShipPickupParams::ShipPickupParams(ActorType *at, ifstream &is )
	:ActorParams(at)
{
	LoadGrounded(is);

	int ifacingRight;
	is >> ifacingRight;
	facingRight = ifacingRight;
}

ShipPickupParams::ShipPickupParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams(at)
{
	facingRight = true;
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

bool ShipPickupParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
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
	of << (int)facingRight << endl;
}

ActorParams *ShipPickupParams::Copy()
{
	ShipPickupParams *copy = new ShipPickupParams( *this );
	return copy;
}

ShardParams::ShardParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at)
{
	position = pos;	

	//image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();

	SetShard(0, 0, 0);
	//shardStr = Shard::GetShardString(0, 0);
	
	//shardStr = "SHARD_W1_TEACH_JUMP";//"..no.shard..";
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
	position = pos;

	image = type->GetSprite(false);
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
	image.setPosition(pos.x, pos.y);

	SetBoundingQuad();
	//
	//shardStr = Shard::GetShardString(p_world, p_localIndex);

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

bool ShardParams::CanApply()
{
	return true;
}

ActorParams *ShardParams::Copy()
{
	ShardParams *copy = new ShardParams( *this );
	return copy;
}

RaceFightTargetParams::RaceFightTargetParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at)
{
	position = pos;	

	image = type->GetSprite(false);
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

RaceFightTargetParams::RaceFightTargetParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
}

void RaceFightTargetParams::WriteParamFile( std::ofstream &of )
{
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

bool RaceFightTargetParams::CanApply()
{
	return true;
}

ActorParams *RaceFightTargetParams::Copy()
{
	RaceFightTargetParams *copy = new RaceFightTargetParams( *this );
	return copy;
}

BlockerParams::BlockerParams(ActorType *at, sf::Vector2i pos, list<sf::Vector2i> &globalPath, int p_bType, bool p_armored,
	int p_spacing )
	:ActorParams(at)
{
	lines = NULL;
	//lines = NULL;
	position = pos;

	image = type->GetSprite(false);
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
	image.setPosition(pos.x, pos.y);

	spacing = p_spacing;
	SetPath(globalPath);

	bType = (BlockerType)p_bType;

	armored = p_armored;


	SetBoundingQuad();
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

	int iarmored;
	is >> iarmored;
	armored = iarmored;

	is >> spacing;
	
}

BlockerParams::BlockerParams(ActorType *at,
	sf::Vector2i &pos)
	:ActorParams(at)
{
	lines = NULL;
	position = pos;

	image = type->GetSprite(false);
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
	image.setPosition(pos.x, pos.y);

	armored = false;

	bType = NORMAL;

	spacing = 0;

	SetBoundingQuad();
}

std::list<sf::Vector2i> BlockerParams::GetGlobalChain()
{
	list<Vector2i> globalPath;
	globalPath.push_back(position);
	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		globalPath.push_back(position + (*it));
	}
	return globalPath;
}

void BlockerParams::SetPath(std::list<sf::Vector2i> &globalPath)
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

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f(temp.x, temp.y);
			li[index].color = Color::Magenta;
			++index;
		}
	}
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
	edit->patrolPath = GetGlobalChain();
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool BlockerParams::CanApply()
{
	return true;
	//see note for keyparams
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
	/*int hMon;
	if (hasMonitor)
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;*/

	of << localPath.size() << endl;

	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		of << (*it).x << " " << (*it).y << endl;
	}

	of << bType << "\n";

	of << (int)armored << "\n";
	
	of << spacing << endl;

	/*if (loop)
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}*/

	//of.precision( 5 );
	//of << speed << endl;
	//of << fixed << speed << endl;
}

ActorParams *BlockerParams::Copy()
{
	BlockerParams *bp = new BlockerParams(*this);
	return bp;
	/*ComboerParams *copy = new ComboerParams(*this);
	if (copy->lines != NULL)
	{
		int numVertices = copy->lines->getVertexCount();

		VertexArray &oldli = *copy->lines;
		copy->lines = new VertexArray(sf::LinesStrip, numVertices);
		VertexArray &li = *copy->lines;


		for (int i = 0; i < numVertices; ++i)
		{
			li[i] = oldli[i];
		}
	}
	return copy;*/

}

RailParams::RailParams(ActorType *at, sf::Vector2i pos, list<sf::Vector2i> &globalPath, bool p_energized )
	:ActorParams(at)
{
	lines = NULL;
	//lines = NULL;
	position = pos;

	image = type->GetSprite(false);
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
	image.setPosition(pos.x, pos.y);

	SetPath(globalPath);
	

	energized = p_energized;


	SetBoundingQuad();
}

RailParams::RailParams(ActorType *at, ifstream &is )
	:ActorParams(at)
{
	lines = NULL;
	LoadAerial(is);
	LoadGlobalPath(is);

	int ienergized;
	is >> ienergized;
	energized = ienergized;

	
}

RailParams::RailParams(ActorType *at,
	sf::Vector2i &pos)
	:ActorParams(at)
{
	lines = NULL;
	position = pos;

	image = type->GetSprite(false);
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
	image.setPosition(pos.x, pos.y);

	energized = false;

	SetBoundingQuad();
}

std::list<sf::Vector2i> RailParams::GetGlobalChain()
{
	list<Vector2i> globalPath;
	globalPath.push_back(position);
	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		globalPath.push_back(position + (*it));
	}
	return globalPath;
}

void RailParams::SetPath(std::list<sf::Vector2i> &globalPath)
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

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f(temp.x, temp.y);
			li[index].color = Color::Magenta;
			++index;
		}
	}
}

void RailParams::SetParams()
{
	Panel *p = type->panel;

	energized = p->checkBoxes["energized"]->checked;

	hasMonitor = false;
}

void RailParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	p->checkBoxes["energized"]->checked = energized;

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalChain();
}

bool RailParams::CanApply()
{
	return true;
	//see note for keyparams
}

void RailParams::Draw(sf::RenderTarget *target)
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

void RailParams::WriteParamFile(ofstream &of)
{

	of << localPath.size() << endl;

	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		of << (*it).x << " " << (*it).y << endl;
	}

	of << (int)energized << "\n";

	/*if (loop)
	{
	of << "+loop" << endl;
	}
	else
	{
	of << "-loop" << endl;
	}*/

	//of.precision( 5 );
	//of << speed << endl;
	//of << fixed << speed << endl;
}

ActorParams *RailParams::Copy()
{
	RailParams *rp = new RailParams(*this);
	return rp;
	/*ComboerParams *copy = new ComboerParams(*this);
	if (copy->lines != NULL)
	{
	int numVertices = copy->lines->getVertexCount();

	VertexArray &oldli = *copy->lines;
	copy->lines = new VertexArray(sf::LinesStrip, numVertices);
	VertexArray &li = *copy->lines;


	for (int i = 0; i < numVertices; ++i)
	{
	li[i] = oldli[i];
	}
	}
	return copy;*/
}

BoosterParams::BoosterParams(ActorType *at, sf::Vector2i &pos, int p_strength )
	:ActorParams(at), strength( p_strength )
{
	position = pos;

	image = type->GetSprite(false);
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
	image.setPosition(pos.x, pos.y);

	SetBoundingQuad();
}


BoosterParams::BoosterParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> strength;
}

BoosterParams::BoosterParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	position = pos;

	image = type->GetSprite(false);
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
	image.setPosition(pos.x, pos.y);

	SetBoundingQuad();
	
	strength = 10;
}

void BoosterParams::WriteParamFile(std::ofstream &of)
{
	of << strength << "\n";
}

void BoosterParams::SetParams()
{
	Panel *p = type->panel;

	string strengthStr = p->textBoxes["strength"]->text.getString().toAnsiString();

	stringstream ss;
	ss << strengthStr;

	int t_strength;
	ss >> t_strength;

	if (!ss.fail())
	{
		strength = t_strength;
	}
	//hasMonitor = p->checkBoxes["monitor"]->checked;
}

void BoosterParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

	p->textBoxes["strength"]->text.setString(boost::lexical_cast<string>(strength));
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool BoosterParams::CanApply()
{
	return true;
}

ActorParams *BoosterParams::Copy()
{
	BoosterParams *copy = new BoosterParams(*this);
	return copy;
}

SpringParams::SpringParams(ActorType *at, sf::Vector2i &pos, std::list<sf::Vector2i> &globalPath,
	int p_moveFrames)
	:ActorParams(at), moveFrames( p_moveFrames )
{
	position = pos;

	image = type->GetSprite(false);
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
	image.setPosition(pos.x, pos.y);

	SetBoundingQuad();

	lines = NULL;

	SetPath(globalPath);	
}

SpringParams::SpringParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	int moveFrames;
	is >> moveFrames;

	Vector2i other;
	is >> other.x;
	is >> other.y;

	list<Vector2i> globalPath;
	globalPath.push_back(Vector2i(position.x, position.y));
	globalPath.push_back(position + other);
	SetPath(globalPath);

	lines = NULL;
}

SpringParams::SpringParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	position = pos;

	image = type->GetSprite(false);
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
	image.setPosition(pos.x, pos.y);

	SetBoundingQuad();

	moveFrames = 60;

	lines = NULL;


}

void SpringParams::WriteParamFile(std::ofstream &of)
{

	of << moveFrames << "\n";
	of << localPath.front().x << " " << localPath.front().y << endl;

	
}

void SpringParams::SetPath(std::list<sf::Vector2i> &globalPath)
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

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f(temp.x, temp.y);
			li[index].color = Color::Magenta;
			++index;
		}

		Vector2f diff = li[1].position - li[0].position;
		float f = GetVectorAngleCW(diff);
		float rot = f / PI * 180.f + 90;
		image.setRotation(rot);
		cout << "blah: " << rot << endl;

	}

	
	//Vector2i diff = globalPath. - globalPath[0];
}

void SpringParams::SetParams()
{
	Panel *p = type->panel;

	string moveFrameStr = p->textBoxes["moveframes"]->text.getString().toAnsiString();

	stringstream ss;
	ss << moveFrameStr;

	int t_moveFrames;
	ss >> t_moveFrames;

	if (!ss.fail())
	{
		moveFrames = t_moveFrames;
	}

	
	hasMonitor = false;

}

void SpringParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

	p->textBoxes["moveframes"]->text.setString((boost::lexical_cast<string>(moveFrames)));

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}

bool SpringParams::CanApply()
{
	return true;
}

ActorParams *SpringParams::Copy()
{
	SpringParams *copy = new SpringParams(*this);
	return copy;
}

void SpringParams::Draw(sf::RenderTarget *target)
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

		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position -= Vector2f(position.x, position.y);
		}
	}

	ActorParams::Draw(target);
}


std::list<sf::Vector2i> SpringParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back(position);
	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		globalPath.push_back(position + (*it));
	}
	return globalPath;
}

ComboerParams::ComboerParams(ActorType *at, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop)
	:ActorParams(at)
{
	lines = NULL;
	position = pos;

	image = type->GetSprite(false);
	image.setPosition(pos.x, pos.y);

	//list<Vector2i> localPath;
	SetPath(globalPath);

	loop = p_loop;
	speed = p_speed;

	SetBoundingQuad();
}

ComboerParams::ComboerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	lines = NULL;

	LoadAerial(is);

	LoadMonitor(is);
	
	LoadGlobalPath(is);

	string loopStr;
	is >> loopStr;
	if (loopStr == "+loop")
		loop = true;
	else if (loopStr == "-loop")
		loop = false;
	else
		assert(false && "should be a boolean");

	is >> speed;
}

ComboerParams::ComboerParams(ActorType *at,
	sf::Vector2i &pos)
	:ActorParams(at)
{
	lines = NULL;

	position = pos;
	image = type->GetSprite(false);
	image.setPosition(position.x, position.y);

	loop = false;
	speed = 10;

	SetBoundingQuad();
}

void ComboerParams::SetParams()
{
	Panel *p = type->panel;

	bool loop = p->checkBoxes["loop"]->checked;


	string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();

	stringstream ss;
	ss << speedStr;

	int t_speed;
	ss >> t_speed;

	if (!ss.fail())
	{
		speed = t_speed;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
	//try
	//{
	//	speed = boost::lexical_cast<int>( p->textBoxes["speed"]->text.getString().toAnsiString() );
	//}
	//catch(boost::bad_lexical_cast &)
	//{
	//	//error
	//}
}

void ComboerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	p->textBoxes["speed"]->text.setString(boost::lexical_cast<string>(speed));
	p->checkBoxes["loop"]->checked = loop;
	p->checkBoxes["monitor"]->checked = hasMonitor;

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
}

bool ComboerParams::CanApply()
{
	return true;
	//see note for keyparams
}

void ComboerParams::SetPath(std::list<sf::Vector2i> &globalPath)
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

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f(temp.x, temp.y);
			li[index].color = Color::Magenta;
			++index;
		}
	}



}

void ComboerParams::Draw(sf::RenderTarget *target)
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

	ActorParams::Draw(target);
	//target->draw( image );

	//DrawBoundar
}

std::list<sf::Vector2i> ComboerParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back(position);
	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		globalPath.push_back(position + (*it));
	}
	return globalPath;
}

void ComboerParams::WriteParamFile(ofstream &of)
{
	int hMon;
	if (hasMonitor)
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;

	of << localPath.size() << endl;

	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		of << (*it).x << " " << (*it).y << endl;
	}

	if (loop)
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}

	//of.precision( 5 );
	of << speed << endl;
	//of << fixed << speed << endl;
}

ActorParams *ComboerParams::Copy()
{
	ComboerParams *copy = new ComboerParams(*this);
	if (copy->lines != NULL)
	{
		int numVertices = copy->lines->getVertexCount();

		VertexArray &oldli = *copy->lines;
		copy->lines = new VertexArray(sf::LinesStrip, numVertices);
		VertexArray &li = *copy->lines;


		for (int i = 0; i < numVertices; ++i)
		{
			li[i] = oldli[i];
		}
	}
	return copy;
}


AirTriggerParams::AirTriggerParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	position = pos;

	image = type->GetSprite(false);
	image.setPosition(pos.x, pos.y);

	SetBoundingQuad();

	//triggerRect.setFillColor(Color::Transparent);
	//triggerRect.setOutlineColor(Color::Red);
	//triggerRect.setOutlineThickness(10);
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
	position = pos;

	image = type->GetSprite(false);
	image.setPosition(pos.x, pos.y);


	triggerRect.setFillColor(Color(200, 200, 200, 150));//Color::Transparent);
	//triggerRect.setOutlineColor(Color::Red);
	//triggerRect.setOutlineThickness(10);
	
	rectWidth = w;
	rectHeight = h;

	SetRect( w, h, pos);
	SetBoundingQuad();

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

bool AirTriggerParams::CanApply()
{
	return true;
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
	AnchorToGround(p_edgePolygon, p_edgeIndex, p_edgeQuantity);

	SetBoundingQuad();
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
	AnchorToGround(p_edgePolygon, p_edgeIndex, p_edgeQuantity);

	typeStr = "NONE";
	facingRight = true;

	SetBoundingQuad();
}

bool FlowerPodParams::CanApply()
{
	if (groundInfo != NULL)
		return true;
	//hmm not sure about this now

	return false;
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