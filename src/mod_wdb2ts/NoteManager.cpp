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

NoteHelper::NoteListenerElement::
NoteListenerElement( INoteUpdateListener *l )
{
	listeners.push_back( l );
}

NoteHelper::NoteListenerElement::
NoteListenerElement()
{
}

void
NoteHelper::NoteListenerElement::
callListeners( const std::string &noteName )
{
	if( ! note ) return;
	for( std::list<INoteUpdateListener*>::iterator it = listeners.begin();
		 it != listeners.end(); ++it )
		(*it)->noteUpdated( noteName, note );
}

void
NoteHelper::NoteListenerElement::
addListener( INoteUpdateListener *l )
{
	for( std::list<INoteUpdateListener*>::iterator it = listeners.begin();
		 it != listeners.end(); ++it ) {
		if( *it == l )
			return;
	}
	listeners.push_back( l );
}

void
NoteHelper::NoteListenerElement::
removeListener( INoteUpdateListener *l )
{
	for( std::list<INoteUpdateListener*>::iterator it = listeners.begin();
	     it != listeners.end(); ++it ) {
		if( *it == l )
			listeners.erase( it );
	}
}

bool
NoteHelper::NoteListenerElement::
updateNote( boost::shared_ptr<NoteTag> n )
{
	if( ! note || (note->version() != n->version()) ) {
		note = n;
		return true;
	} else {
		return false;
	}
}

NoteHelper::
NoteHelper()
{
}


bool
NoteHelper::
updateNote( const std::string &noteName, boost::shared_ptr<NoteTag> note )
{
	std::map< std::string, NoteListenerElement >::iterator nit;

	nit=notes.find( noteName );
	if( nit == notes.end() )
		return false;
	return nit->second.updateNote( note );
}

void
NoteHelper::
callListeners( const std::list<std::string> &noteList )
{
	std::map<std::string, NoteListenerElement >::iterator nit;
	for( std::list<std::string>::const_iterator it=noteList.begin();
		 it != noteList.end(); ++it )
	{
		nit = notes.find( *it );
		if( nit != notes.end() )
			nit->second.callListeners( *it );
	}
}

void
NoteHelper::
addNoteListener( const std::string &noteName,
	                  INoteUpdateListener *noteListener )
{
	std::map<std::string, NoteListenerElement >::iterator it;
	it = notes.find( noteName );

	if( it == notes.end() )
		notes[noteName]=NoteListenerElement( noteListener );
	else
		it->second.addListener( noteListener );
}

void
NoteHelper::
removeNoteListener( const std::string &noteName,
	               	INoteUpdateListener *noteListener )
{
	std::map<std::string, NoteListenerElement >::iterator it;
	it = notes.find( noteName );

	if( it == notes.end() )
		return;

	it->second.removeListener( noteListener );

	if( it->second.listeners.empty() )
		notes.erase( it );
}

void
NoteHelper::
removeAllNoteListener( const std::string &noteName )
{
	std::map<std::string, NoteListenerElement >::iterator it;
	it = notes.find( noteName );

	if( it != notes.end() )
		notes.erase( it );
}



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
	boost::mutex::scoped_lock lock( mutex );
	persistentNotePath = persistentNotePath_;
}

void 
NoteManager::
registerPersistentNote( const std::string &noteName, NoteTag *note )
{
	if( ! note ) {
		cerr << "FATAL: registerPersistentNote: note == 0 '" << noteName << "'!!!\n";
		return;
	}

	boost::mutex::scoped_lock lock( mutex );
	
	std::map<std::string, PersistentNote>::iterator it = persistentNotes.find( noteName );
	
	if( it == persistentNotes.end() )
		persistentNotes[ noteName ] = PersistentNote( persistentNotePath + "/" + noteName + ".note", note );
}

void
NoteManager::
loadPersistentNotes()
{
	boost::posix_time::ptime mtime;
	string buf;
	bool locked;
	NoteTag *note;

	WEBFW_USE_LOGGER( "handler" );
	for( std::map<std::string, PersistentNote>::iterator it = persistentNotes.begin();
			it != persistentNotes.end(); ++ it )
	{
		if( ! it->second.isLoaded ) {
			try {
				mtime = it->second.file->modifiedTime();

				if( mtime.is_special() ) {
					WEBFW_LOG_DEBUG( "loadPersistentNotes:  <" << it->second.file->filename() << "> do not exist." );
					continue;
				}
			}
			catch( std::exception &ex ) {
				WEBFW_LOG_ERROR( "loadPersistentNotes: Exception: " << ex.what() );
				continue;
			}
			catch( ... ) {
				WEBFW_LOG_ERROR( "loadPersistentNotes: Exception: Unexpected exception!" );
				continue;
			}

			if( ! it->second.modifiedTime.is_special() && mtime <= it->second.modifiedTime )
				continue;

			if( ! it->second.file->read( buf, true, locked ) ) {
				WEBFW_LOG_WARN( "loadPersistentNotes: Failed to load note '" << it->first << "' from disk! <" << it->second.file->filename() <<">" );
				continue;
			}

			istringstream sin( buf );

			//WEBFW_LOG_DEBUG( "checkForUpdatedPersistentNotes: note '" << it->first << "' buf '" << buf << "'" );
			note = it->second.note->loadNote( sin );

			if( ! note ) {
				WEBFW_LOG_ERROR( "loadPersistentNotes: Failed to init note '" << it->first << "' from disk!" );
				continue;
			}
			it->second.modifiedTime = mtime;
			boost::shared_ptr<NoteTag> theNote( note );
			theNote->version( 0 );
			it->second.note = theNote;
			it->second.isLoaded = true;
			WEBFW_LOG_DEBUG("NoteManager::loadPersistentNotes: loaded '" << it->first << "' version: " << theNote->version());
		}
	}
}


void
NoteManager::
checkForUpdatedNotes( NoteHelper *noteHelper )
{
	std::list<string> updNotes;

	WEBFW_USE_LOGGER( "handler" );

	{
		boost::mutex::scoped_lock lock( mutex );
		loadPersistentNotes();

		if( ! noteHelper )
			return;

		for( std::map<std::string, PersistentNote>::iterator it = persistentNotes.begin();
				it != persistentNotes.end(); ++ it )
			if( noteHelper->updateNote( it->first, it->second.note ) )
				updNotes.push_back( it->first );

		for( std::map<std::string, boost::shared_ptr<NoteTag> >::iterator it = notes.begin();
             it != notes.end(); ++ it )
			if( noteHelper->updateNote( it->first, it->second ) )
				updNotes.push_back( it->first );
	}

	noteHelper->callListeners( updNotes );
}



/* Persistent notes is only saved to disk. The note is
 * not updated here, but updated in checkForUpdatedNotes.
 */ 

void 
NoteManager::
setNote( const std::string &noteName, NoteTag *note )
{
	boost::shared_ptr<NoteTag> theNote( note );
	theNote->version( 0 );
	
	WEBFW_USE_LOGGER( "handler" );
	boost::mutex::scoped_lock lock( mutex );
		
	std::map<std::string, PersistentNote>::iterator pit = persistentNotes.find( noteName );
		
	if( pit != persistentNotes.end() ) {
		ostringstream sar;
			
		try{
			bool locked;
				
			if( ! theNote->saveNote( sar ) ) {
				WEBFW_LOG_ERROR( "setNote: Failed to save the note <" << note->noteType() << "> to stream!" );
				return;
			}
				
			//WEBFW_LOG_DEBUG( "+++ setNote: file->write" );
				
			if( ! pit->second.file->write( sar.str(), true, locked  ) ) {
				WEBFW_LOG_ERROR( "setNote: Failed to save note '" << noteName << "' to disk!" );
			}
			//WEBFW_LOG_DEBUG( "--- setNote: file->write" );
		}
		catch( ... ){
			WEBFW_LOG_ERROR( "setNote: Failed to stream out notaTag <"<< noteName<< ">!" );
		}

		if( pit->second.note )
			theNote->version( pit->second.note->version() + 1 );

		WEBFW_LOG_DEBUG("NoteManager::setNote: persistent '" << noteName << "' version: " << theNote->version());
		pit->second.note = theNote;
	} else {
		std::map<std::string, boost::shared_ptr<NoteTag> >::iterator it;
			
		it = notes.find( noteName );

		if( it != notes.end() && it->second )
			theNote->version( it->second->version() + 1 );

		WEBFW_LOG_DEBUG("NoteManager::setNote: '" << noteName << "' version: " << theNote->version());

		notes[noteName]= theNote;
	}
}
	 
boost::shared_ptr<NoteTag> 
NoteManager::
getNote( const std::string &note )
{
	WEBFW_USE_LOGGER( "handler" );

	boost::mutex::scoped_lock lock( mutex );

	std::map<std::string, PersistentNote>::iterator pit = persistentNotes.find( note );
			
	if( pit != persistentNotes.end() ) {
		if( pit->second.isLoaded && pit->second.note )
			return pit->second.note;
		else
			return boost::shared_ptr<NoteTag>();
	}
	
	std::map<std::string, boost::shared_ptr<NoteTag> >::iterator it=notes.find( note );
		
	if( it != notes.end() && it->second ) {
		WEBFW_LOG_DEBUG( "NoteManager::getNote <" << note <<  ">:  found! " );
		return it->second;
	}
	
	WEBFW_LOG_WARN( "NoteManager::getNote <" << note <<  ">: Not found!" );
	return boost::shared_ptr<NoteTag>();
}
}
