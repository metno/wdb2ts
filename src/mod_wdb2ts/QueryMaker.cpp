/*
    wdb - weather and water data storage

    Copyright (C) 2007 met.no

    Contact information:
    Norwegian Meteorological Institute
    Box 43 Blindern
    0313 OSLO
    NORWAY
    E-mail: wdb@met.no

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA  02110-1301, USA
*/
#include <limits.h>
#include <iostream>
#include <sstream>
#include <QueryMaker.h>
#include <ProviderList.h>
#include <wciHelper.h>
#include <Logger4cpp.h>
#include <replace.h>


using namespace std;

namespace {
const string PARAMS="@@PARAMS@@";
}

namespace wdb2ts {


bool
Level::
operator<(const Level &rhs)const
{
	if( (levelparametername_ < rhs.levelparametername_ ) ||
	    (levelparametername_ == rhs.levelparametername_ && from_ < rhs.from_ ) ||
	    (levelparametername_ == rhs.levelparametername_ && from_ == rhs.from_ &&
	      to_ < rhs.to_		) ||
	    (levelparametername_ == rhs.levelparametername_ && from_ == rhs.from_ &&
	     to_ == rhs.to_ && qualifier_ < rhs.qualifier_ )
	      )
		return true;
	else
		return false;
}

bool
Level::
operator==(const Level &rhs )const
{
	return isEqual( *this, false );
}

bool
Level::
isEqual( const Level &toThis, bool ignoreQualifier )const
{
	if( levelparametername_ == toThis.levelparametername_ &&
		from_ == toThis.from_ && to_ == toThis.to_ &&
		unit_ == toThis.unit_ ) {

		if( ignoreQualifier )
			return true;
		else
			return qualifier_ == toThis.qualifier_;
	}

	return false;
}

bool
Level::
isDefined()const
{
	if( to_ == INT_MIN && from_ == INT_MIN  && levelparametername_.empty() )
		return false;
	else
		return true;
}

Level::
Level( int from, int to, const std::string &levelparametername,
		const std::string &unit, Qualifier qualifier)
: from_( from ), to_( to ),
  levelparametername_( levelparametername ),
  unit_( unit ), qualifier_( qualifier)
{
	if( qualifier != udef )
		setQualifier();
}

Level::
Level( const ParamDef &param )
	:from_(INT_MIN),to_(INT_MIN), qualifier_( udef )
{
	if( param.hasLevelparameters() ) {
		qualifier_ = udef;
		from_ = param.levelfrom();
		to_ = param.levelto();
		unit_ = param.levelunitname();
		levelparametername_ = param.levelparametername();
	}
}

void
Level::
setQualifier()
{
	if( from_ == INT_MIN && to_ == INT_MAX)
		qualifier_ = any;
	else if( from_ == INT_MIN && to_ == INT_MIN )
		qualifier_ = udef;
	else if( from_ != INT_MAX && to_ != INT_MAX &&
			 from_ != INT_MIN && to_ != INT_MIN ) {
		if( from_ != to_ ) {
			if(  qualifier_ != exact )
				qualifier_ = inside;
		} else if( qualifier_ != above && qualifier_ != below  ) {
				qualifier_ = exact;
		}
	} else {
		qualifier_ = udef;
	}
}

std::string
Level::
qualifierAsString()const
{
	switch( qualifier_ ) {
	case exact: return "exact";
	case below: return "below";
	case above: return "above";
	case inside: return "inside";
	case any: return "any";
	default:
		return "exact";
	}
}


std::string
Level::
toWdbLevelspec()const
{
	ostringstream ost;

	if( ! isDefined() || levelparametername_.empty() )
		return "NULL";

	if( qualifier_ == any )
		ost << "any";
	else if( to_ == from_ )
		ost << qualifierAsString() << " " << from_;
	else
		ost << qualifierAsString() << " " << from_ << " TO " << to_;

	ost << " " << levelparametername_;

	return ost.str();
}

std::ostream&
operator<<( std::ostream &out, const Level &lvl )
{
	if( ! lvl.isDefined() )
		out << "(LEVEL_UNDEF)";
	else
		out << lvl.toWdbLevelspec();

	return out;
}

namespace qmaker {
std::ostream&
operator<<( std::ostream &out, const QuerysAndParamDefs &q )
{
	out << " ----- QuerysAndParamDefs ----- " << endl;
	out << " ---- BEGIN Querys ----"<< endl;
	for( std::list<Query*>::const_iterator it=q.querys.begin();
	     it != q.querys.end(); ++it ) {
		out << "[" << **it << "]" << endl;
	}
	out << " ---- END Querys ----"<< endl;
	out << " ---- BEGIN params ----" << endl;
	for( ParamDefList::const_iterator it=q.params.begin() ;
	     it != q.params.end(); ++it ) {
		out << " ------- ["<< it->first << "] ---------" << endl;

		for( std::list<ParamDef>::const_iterator pit = it->second.begin();
		    pit != it->second.end(); ++pit )
			out << *pit << endl;

		out << "---------------------------------------" << endl;
	}
	out << " ---- END params ----" << endl;
	out << " ------------------------------ " << endl;

	return out;
}


bool
Query::
operator==( const Query &rhs)const
{
	if( id == rhs.id && provider == rhs.provider )
		return true;
	else
		return false;
}

void
Query::
addParam( const ParamDef &param )
{
	std::list<ParamDef>::const_iterator it;

	for( it=params.begin(); it != params.end(); ++it ) {
		if( it->valueparametername() == param.valueparametername() )
			break;
	}

	if( it == params.end() )
		params.push_back( param );
}


void
Query::
merge( Query *with )
{
	if( !( *with == *this ) )
		return;

	bool found;
	for( std::list<std::string>::const_iterator itWith=with->querys.begin();
	     itWith != with->querys.end(); ++itWith ) {
		found = false;
		for( std::list<std::string>::const_iterator it=querys.begin();
		     !found && it != querys.end(); ++it) {
			if( *itWith == *it )
				found=true;
		}

		if( !found )
			querys.push_back( *itWith );
	}

	for( std::list<ParamDef>::const_iterator itWith=with->params.begin();
			itWith != with->params.end(); ++itWith ) {
		found=false;
		for( std::list<ParamDef>::const_iterator it=params.begin();
			 !found && it != params.end(); ++it ) {
			if( it->alias() == itWith->alias() )
				found = true;
		}

		if( ! found )
			params.push_back( *itWith );
	}
}

std::list<std::string>
Query::
getQuerys( int wciProtocol )const
{
	std::list<std::string> ret;
	std::string sParams = wciValueParameter( wciProtocol, params );

	for( std::list<std::string>::const_iterator it = querys.begin();
		 it != querys.end(); ++it ) {
		ret.push_back( miutil::replaceStringCopy( *it, PARAMS, sParams ) );
	}

	return ret;
}

std::ostream&
operator<<( std::ostream &out, const Query &q ){
	out << "{["<<q.id << "] '"<<q.provider << "'" << endl;
	list<string> qs = q.getQuerys( 3 );
	for(list<string>::iterator it=qs.begin(); it != qs.end(); ++it )
		out << "[" << *it << "]" << endl;
	out << "}" << endl;
	return out;
}



QuerysAndParamDefs::
~QuerysAndParamDefs()
{
	for( std::list<Query*>::iterator it=querys.begin();
			 it != querys.end(); ++it )
		delete *it;
}

void
QuerysAndParamDefs::
addQuery( Query *q )
{
	std::list<Query*>::iterator qit;
	for( qit = querys.begin(); qit != querys.end(); ++qit ) {
		if( **qit == *q )
			break;
	}

	if( qit == querys.end() ) {
		//cerr << "****** NEW QUERY *********\n";
		querys.push_back( q );
	} else {
		//cerr << "****** MERGE QUERY *********\n";
		(*qit)->merge( q );
		delete q;
	}
}

void
QuerysAndParamDefs::
addParamDefs( const ParamDefList &params_ )
{
	params.merge( &params_, false );
}



void
QuerysAndParamDefs::
merge( const QuerysAndParamDefs &with )
{
	for( std::list<Query*>::const_iterator itWith=with.querys.begin();
		 itWith != with.querys.end(); ++itWith ) {
		std::list<Query*>::const_iterator it;
		for( it=querys.begin(); it != querys.end(); ++it) {
			if( *itWith == *it )
				break;
		}

		if( it == querys.end() )
			querys.push_back( new Query( **itWith ) );
		else
			(*it)->merge( *it );

	}

	params.merge( &with.params, false );
}


QueryMaker::
QueryMaker( int wciProtocol_ )
	: wciProtocol( wciProtocol_ )
{
}

QueryMaker*
QueryMaker::
create( const wdb2ts::config::ParamDefConfig &params,
		int wciProtocol )
{
	using namespace wdb2ts::config;

	QueryMaker *query = new QueryMaker( wciProtocol );

	string provider;
	for( ParamDefConfig::ParamDefs::const_iterator itId = params.idParamDefs.begin();
		 itId != params.idParamDefs.end(); ++itId ) {
		for( ParamDefList::const_iterator it = itId->second.begin();
			it != itId->second.end(); ++it  ) {
			provider = it->first;

			if( provider == "__DEFAULT__" )
				provider.erase();
			else {
				ProviderList providerItem = ProviderList::decode( it->first );
				if( ! providerItem.empty() )
					provider=providerItem[0].provider;
			}

			for( ParamDefList::value_type::second_type::const_iterator pit=it->second.begin();
					pit != it->second.end(); ++pit ) {
				//cerr << "Add["<<itId->first<<"]["<<provider<<"]" << endl << Level(*pit)<< endl << *pit << endl;
				query->params[itId->first][provider][Level(*pit)].push_back( *pit );
			}
		}
	}
	return query;
}


void
QueryMaker::
createQuery( Query *query, const ParamDefs &params, const Level &lvl, ParamDefList &parOut )const
{
	WEBFW_USE_LOGGER("querymaker");

	bool disabled;
	int dataversion_;
	boost::posix_time::ptime reftime;

	//cerr << "**** [" << query->provider << "]" << endl;

	if( ! referenceTimes->providerReftimeDisabledAndDataversion( query->provider, reftime, disabled, dataversion_) ) {
		//cerr << "**** NO REFTIME ****" << endl;
		return;
	}

	if( disabled ) {
		//cerr << "**** DISABLED ****" << endl;
		return;
	}

	if( dataVersion != INT_MAX )
		dataversion_ = dataVersion;

	ostringstream q;
	q << "SELECT " << wciReadReturnColoumns( wciProtocol ) << " FROM wci.read(" << endl
	  << "ARRAY['" << query->provider << "'], " << endl
	  << "'nearest POINT(" << longitude << " " << latitude << ")', " << endl
	  << wciTimeSpec( wciProtocol, reftime ) << ", " << endl
	  << wciTimeSpec( wciProtocol, validTime.first, validTime.second ) << ", " << endl
	  << PARAMS /*wciValueParameter( wciProtocol, params )*/ << ", " << endl
	  << "NULL, " << endl
	  << "ARRAY[" << dataversion_ << "], NULL::wci.returnfloat ) ORDER BY referencetime";

	//cerr << "q["<< q.str() << "']" << endl;

	for( ParamDefs::const_iterator it=params.begin();
	     it != params.end(); ++it ) {
		ParamDef par( *it );
		par.levelfrom( lvl.from() );
		par.levelto( lvl.to() );
		par.levelparametername( lvl.levelparametername() );
		par.levelunitname( lvl.unit() );
		query->addParam( par );

		if( ! parOut.addParamDef( par, query->provider ) ) {
			WEBFW_LOG_WARN( "QueryMaker: PARAM '" << it->alias() << "' all ready defined (" << *it <<").");
		}
	}

	query->querys.push_back( q.str() );
}

void
QueryMaker::
getWdbReadQuerys( QuerysAndParamDefsPtr res,
		          const std::string &id,
		          const Level &level,
		          const std::list<std::string> &providerList,
		          const std::map<std::string, LevelParams> &params )const
{
	for( std::list<std::string>::const_iterator it=providerList.begin();
	     it != providerList.end(); ++it ) {
		std::map<std::string, LevelParams>::const_iterator itParams = params.find( *it );

		if( itParams == params.end() )
			itParams = params.find( "" );

		if( itParams == params.end() )
			continue;

		Query *q = new Query( id, *it );
		LevelParams::const_iterator itLevel = itParams->second.begin();

		/*
		void createQuery( Query *query,
					      const ParamDefs &params,
					      const Level &lvl,
					      ParamDefList &parOut ); */
		for( ; itLevel != itParams->second.end(); ++itLevel ) {
			if( itLevel->first.isDefined() )
				createQuery( q, itLevel->second, itLevel->first, res->params );
			else if( level.isDefined() )
				createQuery( q, itLevel->second, level, res->params );
		}

		res->addQuery( q );
	}
}



QuerysAndParamDefsPtr
QueryMaker::
getWdbReadQuerys( const std::string &id,
		          float latitude_, float longitude_,
		          const ProviderList &providerList_,
                  const Level &level,
                  const PtrProviderRefTimes referenceTimes_,
                  const std::pair<boost::posix_time::ptime, boost::posix_time::ptime> &validTime_,
                  int dataVersion_  )
{
	list<string> querys;
	ParamDefList definedParams;
	list<string> providerList( providerList_.providerWithoutPlacename() );
	QuerysAndParamDefsPtr res( new QuerysAndParamDefs( wciProtocol ) );
	Params::const_iterator itId;
	latitude = latitude_;
	longitude = longitude_;
	referenceTimes = referenceTimes_;
	validTime = validTime_;
	dataVersion = dataVersion_;

	res->providerPriority = providerList_;
	res->referenceTimes = referenceTimes;

	itId = params.find( "all" );

	//Must make sure that all providers defined for 'all' is in the providerlist.
	if( itId != params.end() ) {
		list<string> providers;
		for( map<string, LevelParams>::const_iterator it=itId->second.begin();
			 it != itId->second.end(); ++it ) {
			if( ! it->first.empty() )
				providers.push_back( it->first );
		}

		for( list<std::string>::const_iterator itP = providerList.begin();
			 itP != providerList.end(); ++itP ) {
			list<string>::iterator it;
			for( it=providers.begin(); it != providers.end() && *it != *itP; ++it);
			if( it == providers.end() )
				providers.push_back( *itP );
		}

//		//*DEBUG
//		if( providers.empty() )
//			cerr << "**** ALL: (empty) " << endl;
//		else
//			cerr << "**** ALL:";
//		for( list<std::string>::const_iterator it = providers.begin();
//					 it != providers.end(); ++it ) {
//			cerr << " '" << *it << "'";
//		}
//		cerr << endl;
		//DEBUG END
		getWdbReadQuerys( res, itId->first, level, providers, itId->second );
	}


	itId = params.find(id);

	if( itId == params.end() ) {
 		return res;
	}

	getWdbReadQuerys( res, id, level, providerList, itId->second );

	return res;
}

}
}
