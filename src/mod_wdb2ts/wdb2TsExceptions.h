#ifndef __WDB2TSEXCEPTIONS_H__
#define __WDB2TSEXCEPTIONS_H__

#include <exception>
#include <string>

namespace wdb2ts {

	class NoReftime : public std::exception
	{
		std::string message;
	public:
		explicit NoReftime() throw(): message( "NoReftime" ) 
		{}
		
		explicit NoReftime( const char *msg ) throw(): message( msg ) 
				{}
		explicit NoReftime( const std::string &msg ) throw(): message( msg ) 
						{}
		
		virtual ~NoReftime() throw() {}
		
		const char *what() const throw (){ return message.c_str(); }
		
	};

	class InInit : public std::exception
	{
		std::string message;
	public:
		explicit InInit() throw(): message( "InInit" ) 
		{}
		
		explicit InInit( const char *msg ) throw(): message( msg ) 
				{}
		explicit InInit( const std::string &msg ) throw(): message( msg ) 
						{}
		
		virtual ~InInit() throw() {}
		
		const char *what() const throw (){ return message.c_str(); }
		
	};
}

#endif
