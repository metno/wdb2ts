#include <sstream>
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
	
std::string 
State::
path()
{
	ostringstream ost;
	
	if( path_.empty() ) {
		for( deque<string>::const_reverse_iterator it = stack.rbegin();
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

}
}
