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


#ifndef __NOTEMANAGER_H__
#define __NOTEMANAGER_H__

#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <NoteTag.h>
#include <INoteUpdateListener.h>
#include <FileLockHelper.h>

namespace wdb2ts {

class NoteManager {
	
	struct PersistentNote {
		boost::shared_ptr<miutil::FileLockHelper> file;
		boost::posix_time::ptime modifiedTime;
		boost::shared_ptr<NoteTag> note;
		bool isLoaded;
		
		PersistentNote():isLoaded( false ){}
		PersistentNote( const std::string &filename_, NoteTag  *note_ ) 
			: file( new miutil::FileLockHelper( filename_ ) ), 
			  note( note_ ),
			  isLoaded( false ){}
		PersistentNote(const PersistentNote &pn )
			: file( pn.file ), modifiedTime( pn.modifiedTime ), 
			  note( pn.note ), isLoaded( pn.isLoaded ){}
		PersistentNote& operator=(const PersistentNote &rhs ) {
			if( this != &rhs ){
				file = rhs.file;
				modifiedTime = rhs.modifiedTime;
				note = rhs.note;
				isLoaded = rhs.isLoaded;
			}
			return *this;
		}
	};
	
	std::string persistentNotePath;
	
	///Maintain a list of persistent notes
	std::map<std::string, PersistentNote> persistentNotes;
	
	///Maintain a list of named notes.
	std::map<std::string, boost::shared_ptr<NoteTag> >      notes;
	
	///Maintain a list of noteListeners for every noteName.
	std::map<std::string, std::list<INoteUpdateListener*> > noteListener;

	///Serialize access to the notes
 	boost::mutex        notesMutex;
 	
 	///Serialize access to the noteListener
 	boost::mutex        listenerMutex;

public:
	NoteManager( const std::string &persistentNotePath );
	NoteManager( );
	
	void setPersistentNotePath( const std::string &persistentNotePath );
	
	void registerPersistentNote( const std::string &noteName, NoteTag *note );
	void checkForUpdatedPersistentNotes();
	
	/**
	 * Set a note and broadcast it to all noteListeners. 
	 * 
	 * Don't use the note ptr after this function have returned.
	 * In the case of persistent notes the pointer is deleted.
	 */
	void setNote( const std::string &noteName, NoteTag *note );
	 
	///Retrive a named note.
	boost::shared_ptr<NoteTag> getNote( const std::string &note );
	   
	///Register a note listener.
	void registerNoteListener( const std::string &noteName,
			 	                   INoteUpdateListener *noteListener );

	///Remove a note listener.
	void removeNoteListener( const std::string &noteName,
	 			 	              INoteUpdateListener *noteListener );

};

}


#endif
