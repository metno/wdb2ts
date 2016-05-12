/*
  Diana - A Free Meteorological Visualisation Tool

  $Id: parser.cc 3126 2010-03-04 12:06:08Z borgem $

  Copyright (C) 2006 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no
  
  This file is part of Diana

  Diana is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Diana is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with Diana; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include "parser.h"

using namespace std;

namespace {
   typedef enum States { SECTION_ID, KEY, VALUE, KEY_OR_SECTION_ID, COMMENT, END};
   
   const char *StatesStrings[] ={ "SECTION_ID", "KEY", "VALUE", "KEY_OR_SECTION_ID", "COMMENT", "END"};
   
   std::string& replace(std::string &str, const std::string &what, const std::string &with);
   void trimstr(std::string &str, const char *trimset=" \t\r\n");
   int  skipws( string::size_type i, const string &buf );
   int countLines( const string &buf ); 
   
}

void 
miutil::pgpool::
impl::
TokenReader::
skip(const char *skipset)
{
   int i;
   
   while( next<buf.length() ) {
      for( i=0; skipset[i] && skipset[i]!=buf[next]; ++i );
      
      if( ! skipset[i] )
         return;
         
      next++;
   }
}

void
miutil::pgpool::
impl::
TokenReader::
skipComment()
{
   if( next < buf.length() && buf[next]==comment ) 
      while( next < buf.length() && buf[next] != '\n' )
         next++;
}


std::string::size_type
miutil::pgpool::
impl::
TokenReader::  
firstOf(const char *chSet, std::string &out, bool &lastWasComment)
{
   bool hasNewline=false;
   bool hasSpace=false;
   bool hasTab=false;
   int  i;
   
   lastWasComment=false;
   out.erase();
   
   if( next >= buf.length() )
      return string::npos;
   
   for( i=0; chSet[i]; ++i ) {
      if( chSet[i]==' ' )
         hasSpace = true;
      else if( chSet[i]=='\t' )
         hasTab=true;
      else if( chSet[i]=='\n' )
         hasNewline = true;
   }
      
   //Skip space at start      
   while( buf[next]==' ' || buf[next]=='\t' ) {
      if( hasSpace || hasTab )
         return next;
      else {
         out += buf[next];
         next++;
      }
   }
   
   while( next < buf.length() && buf[next]!=comment ) {
      if( buf[next] == '\n' ) {
         lineCount++;
         
         if( hasNewline )
            return next;
      }
            
      for( i=0; chSet[i] && chSet[i]!=buf[next]; ++i ); 
      
      if( chSet[i] )
         return next;
      
      out += buf[next];   
      
      ++next;
   }        

   if( buf[next]==comment ) { 
      skipComment();
      
      if( ! hasNewline )
         lastWasComment=true;
   } 
      
    
   return next<buf.length() ? next : string::npos;
}     
      


bool
miutil::pgpool::
impl::
TokenReader:: 
getToken( string &val, char &token, const char *chFirstof, bool &lastWasComment ) 
{
   string::size_type prev;
   bool ret=true;
            
   val.erase();
   next = firstOf( chFirstof, val, lastWasComment );
   prev=next;
   
   //cerr << "line: " << lineCount << " prev: " << prev << " next: " << next << endl;
            
   if( next == string::npos ) {
      ret = false;     
   } else {
      token = buf[next];
      next++;
   }
   
   replace( val, "\n", "");         
   trimstr( val );  

   return ret;
}

bool
miutil::pgpool::
Keys::
key(const std::string &key, std::string &value )
{
   CIKeyVals it=keys.find(key);
            
   if( it != keys.end() ) {
      value = it->second;
      
      if( value.empty() )
         return false;
      
      return true;
   }

   return false;
}


bool 
miutil::pgpool::
Keys::
key(const std::string &key, float &value )
{
   string buf;
   
   if( ! Keys::key( key, buf ) )
      return false;
      
   if( sscanf(buf.c_str(), "%f", &value) != 1 )
      throw logic_error("Conversion error (float): key: '"+key+"' value: '"+buf+"'.");
      
   return true;
}

bool
miutil::pgpool::
Keys:: 
key(const std::string &key, double &value )
{
   string buf;
   
   if( ! Keys::key( key, buf ) )
      return false;
      
   if( sscanf(buf.c_str(), "%lf", &value) != 1 )
      throw logic_error("Conversion error (double): key: '"+key+"' value: '"+buf+"'.");
      
   return true;
   
}

bool 
miutil::pgpool::
Keys::
key(const std::string &key, int &value )
{
   string buf;
   
   if( ! Keys::key(key, buf ) )
      return false;
      
   if( sscanf(buf.c_str(), "%d", &value) != 1 )
      throw logic_error("Conversion error (int): key: '"+key+"' value: '"+buf+"'.");
      
   return true;
   
}
   

std::list<std::string>
miutil::pgpool::
Keys:: 
getKeys()const
{
   list<string> list;
   
   for(CIKeyVals it=keys.begin(); it!=keys.end(); ++it ) 
      list.push_back( it->first );
      
   return list;
}

bool 
miutil::pgpool::
parseKeysInSections(const std::string &strToParse, 
                    Sections &sections,
                    std::ostream &errors )
{
   miutil::pgpool::impl::TokenReader tokens( strToParse );
   bool comment;
   const char *nextTokens=":";
   string buf;
   char token;
   char count;
   bool sStart=false;
   States state=SECTION_ID;
   string currentSectionId;
   string currentKey;
   
   while ( tokens.getToken( buf, token, nextTokens, comment) ) {
      
      if( comment )
         continue;
      
      if( state == KEY_OR_SECTION_ID ) {
         if( token == ':' ) {
            state = SECTION_ID;
            currentSectionId.erase();
         } else if( token == '=' ) {
            state = KEY;
            currentKey.erase();
         } else 
            state = END;
      }   
          
      switch( state ) {
         case SECTION_ID:
            if( token != ':' ) {
               errors << "line: "  << tokens.line() << " expecting id delimiter ':', got " 
                      << token << endl;
               return false;
            } 
            
            currentSectionId = buf ;
            nextTokens = ":=";          
            state = KEY_OR_SECTION_ID;
            break;      
            
         case KEY:
            if( token != '=' ) {
               errors << "line: "  << tokens.line() << " expecting key/val delimiter '=', got " 
                      << token << endl;
               return false;
            }
            
            currentKey = buf;
            tokens.skip(" \t");
            nextTokens = "\" \t\n";
            state = VALUE;
            break;
 
         case VALUE: {
            bool hasValue=false;
            
            switch( token ) {
               case ' ':
               case '\t':
               case '\n':
                  if( ! sStart ) 
                     hasValue=true;
                  break;
               case '\"':
                  if( ! sStart ) {
                     nextTokens = "\"";
                     sStart = true;
                  } else {
                     sStart = false;
                     hasValue = true;
                  }
                  break;
               default:
                  errors << "line: "  << tokens.line() << " expecting '" << nextTokens << "' , got " 
                         << token << endl;
                  return false;
            }
            
            if( !hasValue )
               break;
            
            sections[currentSectionId].addKey( currentKey, buf );
            state = KEY_OR_SECTION_ID;
            currentKey.erase();
            nextTokens = ":=";
            }
            break;
            
         case KEY_OR_SECTION_ID:
            //Do nothing
            break;
            
         case END:
            //Do nothing
            break;
      }
   }                     
         
   if( ! buf.empty() ) {
      if( ! currentKey.empty() ) {
            sections[currentSectionId].addKey( currentKey, buf );
      } else {
         errors << "line: " << tokens.line() << " state: " << StatesStrings[state] << endl
                << "   expecting token(s) [" << nextTokens << "]" << endl;
              
         return false;
      }
   }
      
   return true;
}     
         


namespace {
   
   std::string& 
   replace(std::string &str, const std::string &what, const std::string &with)
   {
      std::string::size_type pos=str.find(what);
   
      while(pos!=std::string::npos){
         str.replace(pos, what.length(), with);
         pos=str.find(what, pos+with.length());
      }
   
      return str;
   }
   
   
   void trimstr(std::string &str, const char *trimset)
   {
      string::size_type pos;
      int len;

      if(str.length()==0)
         return;
     
      pos=str.find_first_not_of(trimset);

      if(pos==string::npos)
         str.erase();
      else if(pos>0)
         str.erase(0, pos);

      len=str.length();

      if(len>0){  //Trim end
         pos=str.find_last_not_of(trimset);
   
         if(pos==std::string::npos)
            str.erase();
         else if(pos<(len-1))
            str.erase(pos+1, len-pos-1);
      }
   }
   
   int countLines( const string &buf ) 
   {
      string::size_type i=0;
      int count=1;
  
      i = buf.find( "\n", i);   
      
      while ( i != string::npos ) {
         ++i;
         ++count;
         i = buf.find( "\n", i);
      }
      
      return count;
   }
   
   int  skipws( string::size_type i, const string &buf )
   {
      for( ; i!=string::npos && isspace(buf[i]) ; ++i );
      
      return i;
   }
}
