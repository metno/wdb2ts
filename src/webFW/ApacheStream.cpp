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
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <ApacheStream.h>
#include <apr_file_io.h>
#include <apr_file_info.h>


#define DEFAULT_CHUNC_SIZE 1024

using namespace std;

webfw::
ApacheStream::
ApacheStream(request_rec *request) :
   chunckSize(DEFAULT_CHUNC_SIZE), 
   r( request ), size_( 0 ), nChunck( 0 ), directOutput_( false ), bb( 0 )
{
}   

webfw::
ApacheStream::
ApacheStream( const ApacheStream &cp )
   : chunckSize( cp.chunckSize ), r( cp.r ), size_( cp.size_ ),
     nChunck( cp.nChunck ), directOutput_( cp.directOutput_ ), bb( cp.bb )
{
}

webfw::
ApacheStream::
~ApacheStream() 
{
}

void 
webfw::
ApacheStream::
contentType( const char *content_type )
{
   ap_set_content_type ( r, content_type );
}

void 
webfw::
ApacheStream::
expire( const char *exp )
{
	//cerr << "ApacheStream::expire '"<<exp<<"'." << endl;
   if( exp )
      apr_table_setn(r->headers_out, "Expires", apr_pstrdup( r->pool, exp ));
}

void 
webfw::
ApacheStream::
serviceUnavailable( const char *retryAfter )
{
	if( retryAfter )
		apr_table_add(r->err_headers_out,"Retry-After", apr_pstrdup(r->pool, retryAfter) );
}

void 
webfw::
ApacheStream::
contentLength( long content_length )
{
   ap_set_content_length (r, content_length );
}

void
webfw::
ApacheStream::
checkIOStatusAndThrow( apr_status_t status )
{
   if( r->connection->aborted )
      throw IOError( "The client aborted the request." );
   
   if( status == APR_SUCCESS )
      return;
      
   char buf[256];
   char *err;
         
   err = apr_strerror( status, buf, 256 );
         
   if( err ) {
      std::cerr << "-- WARNING: Write: " << err << std::endl;
   }else {
      sprintf( buf, "IOError: error code (%d)", status );
      std::cerr << "-- WARNING: Write (ERROR (" << status << ")): <NO ERROR MESSAGE>" << status <<  std::endl;
   }

   throw IOError( buf, true );
}

void 
webfw::
ApacheStream::
sendToClient( const char *buf )
{
   if( ! bb )
      throw IOError("ERROR: No output buffer (bucket_brigade)!", true );
   
   checkIOStatusAndThrow( ap_fputs(r->output_filters, bb, buf ) );
   checkIOStatusAndThrow( ap_fflush( r->output_filters, bb ) );
   
   nChunck = 0;
   ost.str("");
}


std::streamsize 
webfw::
ApacheStream::
write(const char_type* s, std::streamsize n)
{
   if( directOutput_ ) {
      if( nChunck > chunckSize ) {
         sendToClient( ost.str().c_str() );
    //   sleep(1);
      }
   } 
   
   ost.write( s, n );
   size_ += n;
   nChunck += n;
   return n;
}  

void 
webfw::
ApacheStream::
directOutput( bool flag )
{
   //If we allready is in directOutput mode, just return,
   //we have now way to get back to buffred mode.
   if( directOutput_ )
      return;
   
   //If we try to set the flag to false, just return,
   //the false state is default.
   if( ! flag )
      return;
   
   directOutput_ = true;

   bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
   
   if( ! bb )
      throw IOError("ERROR: Cant create output buffer (bucket_brigade)!", true );
      
   if( nChunck > chunckSize ) 
      sendToClient( ost.str().c_str() );
}

void 
webfw::
ApacheStream::
flushStream()
{
   if( r->connection->aborted )
      return;
   
   if( ! directOutput_ ) {
      contentLength( ost.str().length() );
      ap_rputs( ost.str().c_str(), r );
      ap_rflush( r );
      ost.str("");
   } else  if( ost.str().length() > 0 ) {
      if( ! bb )
         return;

      sendToClient( ost.str().c_str() );
   }
}

long
webfw::
ApacheStream::
sendFile( const std::string &file, bool deleteFile )
{
	apr_file_t *fd;
	apr_finfo_t fInfo;
	apr_size_t nBytes;
	apr_int32_t flag = APR_FOPEN_READ | APR_FOPEN_SENDFILE_ENABLED;

	apr_status_t st = apr_stat( &fInfo, file.c_str(), APR_FINFO_MIN, r->pool );

	if( st != APR_SUCCESS || st != APR_INCOMPLETE ) {
		if( deleteFile )
			unlink( file.c_str() );

		throw IOError("ERROR: sendFile: Cant get information about the file <" + file + ">.", true );
	}

	if( (fInfo.valid & APR_FINFO_SIZE) == 0 ) {
		if( deleteFile )
			unlink( file.c_str() );

		throw IOError("ERROR: sendFile: Cant get the length of the file <" + file + ">.", true );
	}

	if( deleteFile )
		flag |= APR_FOPEN_DELONCLOSE;

	if( apr_file_open( &fd, file.c_str(), flag,	APR_FPROT_OS_DEFAULT, r->pool ) != APR_SUCCESS ) {
		throw IOError("ERROR: sendFile: Cant open the file <" + file + ">.", true );
	}

	contentLength( fInfo.size );

	if( ap_send_fd( fd, r, 0, fInfo.size, &nBytes) != APR_SUCCESS )
		throw IOError("ERROR: sendFile: Failed to send the file <" + file + "> to the client.", true );

	return nBytes;
}


