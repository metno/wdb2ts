#include <string.h>
#include <iostream>
#include <gettimeofday.h>
#include <Timer.h>
#include <string>
#include <map>

using namespace std;

namespace{
	int MAX_CHAR=64;
}

miutil::
Timer::
Timer(int nTimers_, int nSumMarkers )
	:nTimers(nTimers_), nextMark(0),
	 nSumMark( nSumMarkers ), nextSumMark( 0 )
			
{
	id=0;
	
	if(nTimers<1)
		nTimers=1;
	
	nTimers++;
	timers=0;
	
	
	if( nSumMark < 0 )
		nSumMark = 1;
	
	sumMark = 0;
	nSumMark++;
	
	try{
		timers=new double[nTimers];
		
		id=new char*[nTimers];
		
		for(int i=0; i<nTimers; ++i) 
			id[i]=0;
			
		for(int i=0; i<nTimers; ++i){
			id[i]=new char[MAX_CHAR];
			*id[i]='\0';
			timers[i]=0;
		}
		
		sumMark=new SumMark[nSumMark];
		idSumMark=new char*[nSumMark];

		for( int i=0; i<nSumMark; ++i ) 
			idSumMark[i] = 0;
		
		for( int i=0; i<nSumMark; ++i ) {
			sumMark[i].sum = 0.0;
			sumMark[i].start = -1.0;
			idSumMark[i] = new char[MAX_CHAR];
			*idSumMark[i]='\0';
		}
	}catch(...){
		if(timers)
			delete[] timers;
			
		if(id){
			for(int i=0; i<nTimers; ++i){
				if(id[i])
					delete[] id[i]; 
			}
			
			delete[] id;
		}
		
		if( sumMark )
			delete[] sumMark;
		
		if(idSumMark ){
			for(int i=0; i<nSumMark; ++i){
				if(idSumMark[i])
					delete[] idSumMark[i]; 
			}
					
			delete[] idSumMark;
		}
		
		timers = 0;
		id = 0;
		sumMark = 0;
		idSumMark = 0;
	}
}

miutil::
Timer::
~Timer()
{
	if(timers)
		delete[] timers;
			
	if(id){
		for(int i=0; i<nTimers; ++i){
			if(id[i])
				delete[] id[i]; 
		}
			
		delete[] id;
	}
		
	if( sumMark )
		delete[] sumMark;
		
	if(idSumMark ){
		for(int i=0; i<nSumMark; ++i){
			if(idSumMark[i])
				delete[] idSumMark[i]; 
		}
					
		delete[] idSumMark;
	}
		
	sumMark = 0;
	idSumMark = 0;
	timers=0;
	id=0;
}
			
void 
miutil::
Timer::
reset()
{
	if(timers){
		for(int i=0; i<nTimers; ++i){
			*id[i]='\0';
			timers[i]=0;
		}
		
		nextMark=0;
	}
	
	if( sumMark ) {
		for(int i=0; i<nSumMark; ++i){
			*idSumMark[i]='\0';
			sumMark[i].sum = 0.0;
			sumMark[i].start = -1.0;
		}
			
		nextSumMark = 0;
	}
		
}


int 
miutil::
Timer::
findSumMark( const char *id_, bool mustExist ) 
{
	for( int i=0; i<nSumMark; ++i) 
		if( strcmp( idSumMark[i], id_ ) == 0 )
			return i;
	
	if( mustExist )
		return -1;
	
	if( nextSumMark < nSumMark ) {
		strncpy(idSumMark[nextSumMark], id_, MAX_CHAR-1);
		idSumMark[nextSumMark][MAX_CHAR-1]='\0';
		sumMark[nextSumMark].start = -1.0;
		sumMark[nextSumMark].sum  = 0.0;
		nextSumMark++;
		return nextSumMark - 1;
	}
		
	return -1;
}

void
miutil::
Timer::
markStart( const char *id_ )
{
	int i = findSumMark( id_, false );
	
	if( i < 0 )
		return;
	
	sumMark[ i ].start = gettimeofday();
}

void
miutil::
Timer::
markStop( const char *id_ )
{
	int i = findSumMark( id_, true );
	
	if( i < 0 )
		return;
	
	double stop = gettimeofday();
	
	if( sumMark[ i ].start < stop )
		sumMark[ i ].sum += stop - sumMark[ i ].start;
}


void
miutil::
Timer::
mark(const char *id_)
{
	if(timers && nextMark<nTimers){
		timers[nextMark]=gettimeofday();
		
		if(id_){
			int n=strlen(id_);
			
			if(n>=MAX_CHAR)
				n=MAX_CHAR-1;
				
			strncpy(id[nextMark], id_, MAX_CHAR-1);
			id[nextMark][MAX_CHAR-1]='\0';
		}
		
		nextMark++;
	}
}
			
			
int 
miutil::
Timer::
getLastmark()const
{
	return nextMark-1;
}
			
double 
miutil::
Timer::
getMark(int i)const
{
	if(i<0 || i>(nextMark-1))
		return -1.0;
		
	return timers[i];
}
			
double 
miutil::
Timer::
getDiff(int i)const
{
	if(i<=0){
		i=nextMark-1;
		
		if(i<0)
			return 0.0;
	}
		
	if(i>(nextMark-1))
		return -1.0;
	
	return timers[i]-timers[0];
}
			
double 
miutil::
Timer::
getTimer(const char *id_)const
{
	if(!id)
		return -1.0;
	
	int first=-1;
	int last=-1;	
		
	int i;
	for(i=0; i<nextMark; ++i){
		if(strcmp(id[i], id_)==0){
			first=i;
			break;
		}
	}
	
	for(i=i+1; i<nextMark; ++i){
		if(strcmp(id[i], id_)==0){
			last=i;
			break;
		}
	}
	
	if(first<0 || last<0)
		return -1.0;
	
	return timers[last]-timers[first];
}

			
double 
miutil::
Timer::
getTimer(int i)const
{
	if(i<0 || i>(nextMark-1))
		return -1.0;
	
	if(i==0)
		return 0.0;
	
	return timers[i]-timers[i-1];
}

std::ostream&
miutil::
Timer::
print( std::ostream &o ) const
{
	string runtime("runtime");
	map<string, int> sMark;
	int maxChar = 0;
	int len;
	double diff;
	double percent;
	
	//Find all marks id's
	for(int i=0; i<nextMark; ++i){
		if(*id[i]) {
			len = strlen( id[i] );
			sMark[ id[i] ] = len;
			if( len>maxChar )
				maxChar = len;
		}
	}
	
	for(int i=0; i<nextSumMark; ++i){
		if(*idSumMark[i]) {
			len = strlen( idSumMark[i] );
			sMark[ idSumMark[i] ] = len;
			if( len>maxChar )
				maxChar = len;
		}
	}

	double tRuntime = getDiff();
	if( runtime.length() > maxChar )
		maxChar = runtime.length();
	
	o.setf(std::ios::floatfield, std::ios::fixed);
	o.precision(6);
	
	o << runtime << string( maxChar-runtime.length(), ' ' ) << " : " << tRuntime << endl;
	for( map<string, int>::const_iterator it = sMark.begin();
		  it!=sMark.end();
		  ++it ) {
		diff = getTimer( it->first.c_str() ); 
		
		if( diff < 0 )
			continue;
		
		percent = (diff/tRuntime)*100;
		o.precision(6);
		o << it->first << string( maxChar-it->second, ' ' ) << " : " << diff << "s ";
		o.precision( 1 );
		o << "(" << percent << "%) " << endl;
	}
	
	for( int i=0; i<nextSumMark; ++i ) {
		percent = (sumMark[i].sum/tRuntime)*100;
		o.precision(6);
		o << "(s) " << idSumMark[i] << string( maxChar-strlen(idSumMark[i]), ' ' ) << " : " << sumMark[i].sum << "s ";
		o.precision( 1 );
		o << "(" << percent << "%) " << endl;
	}
	
	return o;
}

std::ostream& 
miutil::
operator << (std::ostream &o,
           const miutil::Timer &t)
{
	int idindex;
		
	if(t.timers){
		o.setf(std::ios::floatfield, std::ios::fixed);
   		o.precision(6);
   		
		for(int i=0; i<t.nextMark; ++i){
			o << "Timer" << i << ":";
			if(*t.id[i])
				o << " [" << t.id[i] << "]";
				
			o << " diff0: " << t.getDiff(i) << " diffN: " << t.getTimer(i);
			
			idindex=-1;
			
			for(int ii=i-1; ii>=0; --ii){
				if(strcmp(t.id[i], t.id[ii])==0){
					idindex=ii;
					break;
				}
			}
			
			if(idindex>=0)
				o << " diffID: " << t.timers[i]-t.timers[idindex];
				
			o << std::endl; 
		}
	}
	
	return o;
}                         	

