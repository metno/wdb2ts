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

//#include <sys/stat.h>
//#include <sys/types.h>
//#include <utime.h>
//#include <unistd.h>
#include <fstream>
#include "copyfile.h"
#include "fileutil.h"


bool
miutil::file:: 
copyfile(FILE *srcfd, int frompos, int nByte, 
	 FILE *destfd, int startpos)
{
    int nRead;
    int nWrite;
    int toRead;
    char buf[512];
    char *p;

    if(fseek(srcfd, frompos, SEEK_SET)==-1)
	return false;
    
    if(nByte<0){
      long lastpos;
      long curpos;
      
      curpos=ftell(srcfd);
      
      if(fseek(srcfd, 0L, SEEK_END)==-1)
	return false;
      
      lastpos=ftell(srcfd);
      
      if(fseek(srcfd, curpos, SEEK_SET)==-1)
	return false;
      
      nByte=lastpos-curpos;
    }
    
    if(fseek(destfd, startpos, SEEK_SET)==-1)
      return false;
    
    while(nByte>0){
      toRead=nByte>512?512:nByte;
      
      nRead=fread(buf, 1, toRead, srcfd);
      
      if(nRead<toRead && !feof(srcfd))
	return false;
      
      nByte-=nRead;
      p=buf;
      
      while(nRead>0){
	nWrite=fwrite(p, 1, nRead, destfd);
	nRead-=nWrite;
	p=&p[nWrite];
	
	if(ferror(destfd))
	  return false;
      }
    }
    
    return true;
}

bool
miutil::file::
copyfile(const std::string &fromfile, const std::string &tofile,
	 bool set_mtime)
{
  FILE *from;
  FILE *to;
  char buf[512];
  char *p;
  int  n;
  int written;
  
  from=fopen(fromfile.c_str(), "r");
  
  if(!from)
    return false;
  
  to=fopen(tofile.c_str(), "w");
  
  if(!to){
    fclose(from);
    return false;
  }
  
  while(!feof(from)){
    n=fread(buf, 1, 512, from);
    
    if(n<512 && ferror(from))
      break;
    
    p=buf;
    
    while(n>0 && !ferror(to)){
      written=fwrite(p, 1, n, to);
      p=&p[written];
      n-=written;
    }
	
    if(n>0)
      break;
  }

  if(!feof(from) || ferror(to)){
    //Det har oppst�tt en feil en plass.
    fclose(to);
    fclose(from);
    miutil::file::removefile( tofile );
    return false;
  }

  fclose(to);
  fclose(from);

  if(set_mtime){
      boost::posix_time::ptime t = miutil::file::getmtime( fromfile );

      if( ! t.is_special() )
          miutil::file::setmtime( tofile, t );
  }


  return true;
}


bool
miutil::file::
safecopy(const std::string &fromfile, const std::string &tofile, 
	 bool set_mtime)
{
    std::string tmp(tofile);
    std::string::size_type i;
    std::string tmpextra("_SafeCopy_tmp.tmp");

    i=tmp.find_last_of(".");

    if(i!=std::string::npos){
	if(tmp.find_first_of("/", i)==std::string::npos)
	    tmp.erase(i);
    }

    tmp+=tmpextra;
    
    if(!copyfile(fromfile, tmp, set_mtime))
      return false;
    
    if( ! miutil::file::renamefile( tmp, tofile ) ){
        miutil::file::removefile( tmp );
        return false;
    }
    
    return true;
}

bool
miutil::file::
copyFromStreamToFile( std::istream &ist, const std::string &destFile )
{
	int N(1024*1024);
	char buf[N];
	int n=0;
	std::ofstream ofile;

	ofile.open( destFile.c_str() );

	if( ! ofile.is_open() )
		return false;

   //std::cerr << "copy: to file '" << destFile << "'." << std::endl;

	while( ist.read( buf, N ) ) {
		n = ist.gcount();

		//std::cerr << "copy: [" << std::string( buf, n ) << "]" << std::endl;
		ofile.write( buf, n );

		if( ! ofile ) {
		    miutil::file::removefile( destFile );
			return false;
		}

		n=-1;
	}

	n = ist.gcount();

	if( n > 0 ) {
	   //std::cerr << "copy: n=" << n << " [" << std::string( buf, n ) << "]" << std::endl;
	   ofile.write( buf, n );

	   if( ! ofile ) {
	       miutil::file::removefile( destFile );
	       return false;
	   }
	}

	return true;
}
