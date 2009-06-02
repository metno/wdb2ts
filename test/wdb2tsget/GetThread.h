#ifndef __GETTHREAD_H__
#define __GETTHREAD_H__

#include <float.h>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <UrlQuery.h>

namespace wdb2ts {

class GetThread
{
	struct Results {
		int nSuccess;
		int nFailed;
		int nCrcFail;
	};
	
	boost::shared_ptr<Results> results;
	boost::shared_ptr<webfw::UrlQuery> query;
	int nRuns;
	int id;
	unsigned int randState;
	std::string url;
	float lat, lon;
	
protected:
	void nextLatLong( float &lat, float &lon );
	
public:
	GetThread(const std::string &url_, float lat_, float lon_, int runs, unsigned int randInitState, int id_ )
		: results( new Results ), query( new webfw::UrlQuery), nRuns( runs ), id( id_ ), randState( randInitState ),
		  url( url_ ), lat( lat_ ), lon( lon_ )
	{ 
		results->nSuccess = 0;
		results->nFailed  = 0;
		results->nCrcFail = 0; 
	};
	
	GetThread( const GetThread &gt )
			: nRuns( gt.nRuns ), results( gt.results ), query( gt.query ), id( gt.id ), randState( gt.randState ),
			  url( gt.url ), lat( gt.lat ), lon( gt.lon )
		{};

	void operator()();
	
	int runs()const { return nRuns; };
	int success() const { return results->nSuccess; }
	int failed() const { return results->nFailed;}
	int crcFail() const { return results->nCrcFail; }
};

}


#endif 
