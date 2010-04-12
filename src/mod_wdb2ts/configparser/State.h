#ifndef __STATE_H__
#define __STATE_H__

#include <string>
#include <deque>


namespace wdb2ts {
namespace config {

class State
{
	std::deque<std::string> stack;
	
	std::string path_;
	
public:
	State();
	~State();
	
	bool top( std::string &val);
	bool pop( std::string &val );
	void push( const std::string &val );
	
	std::string path();
};

}
}


#endif 
