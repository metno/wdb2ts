#include <iostream>
#include <sstream>
#include <deque>
#include <State.h>

using namespace std;

namespace wdb2ts {
namespace config {

	
State::
State()
{
}

State::
~State()
{
}

	
bool 
State::
top( std::string &val)
{
	if( stack.empty() )
		return false;

	val = stack.front();
	return true;
}
	
bool 
State::
pop( std::string &val )
{
	if( stack.empty() )
		return false;
	
	val = stack.front();
	stack.pop_front();
	path_.erase();
	
	return true;
}

void 
State::
push( const std::string &val )
{
	path_.erase();
	stack.push_front( val );
}

#if 0
std::string 
State::
path()
{
	ostringstream ost;
	
	if( path_.empty() ) {
		for( std::deque<std::string>::reverse_iterator it = stack.rbegin();
			  it != stack.rend();
			  ++it ) {
			ost << "/" << *it; 
		}
		
		path_ = ost.str();
		
		if( path_.empty() )
			path_="/";
	}
	
	return path_;
}
#endif

std::string
State::
path()const
{
   ostringstream ost;

   for( std::deque<std::string>::const_reverse_iterator it = stack.rbegin();
        it != stack.rend();
        ++it ) {
      ost << "/" << *it;
   }

   return  ost.str();
}


bool
State::
operator==( const std::string &rhs ) const
{
   string p = path();


   if( ! rhs.empty() && rhs[0] == '.' ) {
      string end=rhs.substr( 1 );

      if( p.size() < end.size() )
         return false;

      return p.find( end, p.size() - end.size() ) != string::npos;
   } else {
      return rhs==p;
   }
}

}
}
