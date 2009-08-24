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


#include <iostream>
#include <sstream>
#include <NoteManager.h>
#include <Logger4cpp.h>

using namespace std; 

namespace wdb2ts {

NoteManager::
NoteManager()
{
}

NoteManager::
NoteManager( const std::string &persistentNotePath_ )
	: persistentNotePath( persistentNotePath_ )
{
	
}
	
void 
NoteManager::
setPersistentNotePath( const std::string &persistentNotePath_ )
{
	boost::mutex::scoped_lock lock( notesMutex );
	persistentNotePath = persistentNotePath_;
}

void 
NoteManager::
registerPersistentNote( const std::string &noteName, NoteTag *note )
{
	boost::mutex::scoped_lock lock( notesMutex );
	
	std::map<std::string, PersistentNote>::iterator it = persistentNotes.find( noteName );
	
	if( it == persistentNotes.end() )
		persistentNotes[ noteName ] = PersistentNote( persistentNotePath + "/" + noteName + ".note", note );
}

void
NoteManager::
checkForUpdatedPersistentNotes()
{
	string buf;
	bool locked;
	map<string, boost::shared_ptr<NoteTag> > updNotes;
	NoteTag *note;
	boost::posix_time::ptime mtime;
	
	WEBFW_USE_LOGGER( "handler" );
	{
		boost::mutex::scoped_lock lock( notesMutex );
		for( std::map<std::string, PersistentNote>::iterator it = persistentNotes.begin();
		  it != persistentNotes.end(); ++ it )
		{
			
			try{
				 mtime = it->second.file->modifiedTime();
		
				 if( mtime.is_special() ) {
					 WEBFW_LOG_DEBUG( "checkForUpdatedPersistentNotes:  <" << it->second.file->filename() << "> do not exist." );
					 continue;
				 }
			}
			catch( std::exception &ex ) {
				WEBFW_LOG_ERROR( "checkForUpdatedPersistentNotes:Exception: " << ex.what() );
				continue;
			}

			if( ! it->second.modifiedTime.is_special() &&  mtime <= it->second.modifiedTime )
				continue;
		
			//There is new data on disk.
			
//			WEBFW_LOG_INFO( "checkForUpdatedPersistentNotes: note '" << it->first << "' file '" << it->second.file->filename() << "' changed on disk!" );
			
			if( ! it->second.file->read( buf, true, locked  ) ) {
				WEBFW_LOG_WARN( "getNote: Failed to load note '" << it->first << "' from disk! <" << it->second.file->filename() <<">" );
				continue;
			}
			
			istringstream sin( buf );		
			
//			WEBFW_LOG_DEBUG( "checkForUpdatedPersistentNotes: note '" << it->first << "' buf '" << buf << "'" );
			note = it->second.note->loadNote( sin );
			
			if( ! note ) {
				WEBFW_LOG_ERROR( "getNote: Failed to init note '" << it->first << "' from disk!"  );
				continue;
			}
			it->second.modifiedTime = mtime;
			boost::shared_ptr<NoteTag> tmpNote( note );
			updNotes[it->first] = tmpNote;
			it->second.note = tmpNote;
			it->second.isLoaded = true;
		}
	}

	boost::mutex::scoped_lock lock( listenerMutex );
	
	std::map<std::string, std::list<INoteUpdateListener*> >::iterator it;
	
	for( map<string, boost::shared_ptr<NoteTag> >::iterator updIt = updNotes.begin();
	    updIt != updNotes.end(); ++updIt )
	{
		
//		WEBFW_LOG_DEBUG( "update: " << updIt->first );
		it = noteListener.find( updIt->first );
		
		if( it == noteListener.end() )
			continue;
		
		for( list<INoteUpdateListener*>::iterator itl=it->second.begin();
		     itl != it->second.end();
		     ++itl ) {
//			WEBFW_LOG_DEBUG( "update: send '"<< updIt->first  << "'" );
			(*itl)->noteUpdated( updIt->first, updIt->second );
		}
	}
}

/* Persitent notes is only saved to disk. The note is
 * not updated here, but updated in checkForUpdatedPersistentNotes.
 * The signaling of updated notes is also done in checkForUpdatedPersistentNotes.
 */ 

void 
NoteManager::
setNote( const std::string &noteName, NoteTag *note )
{
	boost::shared_ptr<NoteTag> tmpNote;
	bool isPersistentNote = false;
	
	WEBFW_USE_LOGGER( "handler" );
	{
		boost::mutex::scoped_lock lock( notesMutex );
		
		std::map<std::string, PersistentNote>::iterator it = persistentNotes.find( noteName );
		
		if( it != persistentNotes.end() ) {
			isPersistentNote = true;
			
			ostringstream sar;
			
			try{
				bool locked;
				
				if( ! note->saveNote( sar ) ) {
					WEBFW_LOG_ERROR( "setNote: Failed to save the note <" << note->noteType() << "> to stream!" );
					delete note;
					return;
				}
				
				//WEBFW_LOG_DEBUG( "+++ setNote: file->write" );
				
				if( ! it->second.file->write( sar.str(), true, locked  ) ) {
					WEBFW_LOG_ERROR( "setNote: Failed to save note '" << noteName << "' to disk!" );
				}
				//WEBFW_LOG_DEBUG( "--- setNote: file->write" );
			}
			catch( ... ){
				WEBFW_LOG_ERROR( "setNote: Failed to stream out notaTag <"<< noteName<< ">!" );
			}
			
			delete note;
		} else {
			notes[noteName]=boost::shared_ptr<NoteTag>( note );
			tmpNote = notes[noteName];
		}
	}
	
	if( isPersistentNote )
		return;
	
	boost::mutex::scoped_lock lock( listenerMutex );

	std::map<std::string, std::list<INoteUpdateListener*> >::iterator it;
	it = noteListener.find( noteName );
	
	if( it == noteListener.end() )
		return;
	
	for( list<INoteUpdateListener*>::iterator itl=it->second.begin();
	     itl != it->second.end();
	     ++itl )
		(*itl)->noteUpdated( noteName, tmpNote );
}
	 
boost::shared_ptr<NoteTag> 
NoteManager::
getNote( const std::string &note )
{
	WEBFW_USE_LOGGER( "handler" );

	boost::mutex::scoped_lock lock( notesMutex );

	std::map<std::string, PersistentNote>::iterator pit = persistentNotes.find( note );
			
	if( pit != persistentNotes.end() ) {
		if( pit->second.isLoaded )
			return pit->second.note;
		else
			return boost::shared_ptr<NoteTag>();
	}
	
	std::map<std::string, boost::shared_ptr<NoteTag> >::iterator it=notes.find( note );
		
	if( it != notes.end() ) {
		WEBFW_LOG_DEBUG( "NoteManager::getNote <" << note <<  ">:  found! " );
		return it->second;
	}
	
	WEBFW_LOG_WARN( "NoteManager::getNote <" << note <<  ">: Not found!" );
	return boost::shared_ptr<NoteTag>();
}
	   
void 
NoteManager::
registerNoteListener( const std::string &noteName,
			 	          INoteUpdateListener *noteListener_ )
{
	boost::mutex::scoped_lock lock( listenerMutex );

	std::map<std::string, std::list<INoteUpdateListener*> >::iterator it;
	it = noteListener.find( noteName );
		
	if( it != noteListener.end() ) { 
		for( list<INoteUpdateListener*>::iterator itl=it->second.begin();
		     itl != it->second.end();
		     ++itl )
			if( (*itl)== noteListener_ ) {
				//The noteListener is allready registred
				return;
			}
	}
	
	noteListener[noteName].push_back( noteListener_ );

}

void 
NoteManager::
removeNoteListener( const std::string &noteName,
	 			        INoteUpdateListener *noteListener_ )
{
	boost::mutex::scoped_lock lock( listenerMutex );

	std::map<std::string, std::list<INoteUpdateListener*> >::iterator it;
	it = noteListener.find( noteName );
	
	if( it != noteListener.end() ) { 
		for( list<INoteUpdateListener*>::iterator itl=it->second.begin();
		     itl != it->second.end();
		     ++itl ) {
			if( (*itl)== noteListener_ ) {
				it->second.erase( itl );
				break;
			}
		}
	}
}

}
