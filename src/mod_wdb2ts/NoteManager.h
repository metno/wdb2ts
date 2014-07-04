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


#ifndef __NOTEMANAGER1_H__
#define __NOTEMANAGER1_H__

#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <NoteTag.h>
#include <INoteUpdateListener.h>
#include <FileLockHelper.h>

namespace wdb2ts {

class NoteHelper {
	friend class NoteManager;
	NoteHelper& operator=(const NoteHelper &);
	NoteHelper(const NoteHelper &);

	struct NoteListenerElement {
		std::list<INoteUpdateListener*> listeners;
		boost::shared_ptr<NoteTag> note;

		NoteListenerElement( INoteUpdateListener *l );
		NoteListenerElement();

		void callListeners( const std::string &noteName );
		void addListener( INoteUpdateListener *l );
		void removeListener( INoteUpdateListener *l );
		bool updateNote( boost::shared_ptr<NoteTag> n );
	};

	///List of notes
	std::map<std::string, NoteListenerElement > notes;

	bool updateNote( const std::string &noteName, boost::shared_ptr<NoteTag> note );
	void callListeners( const std::list<std::string> &noteList );

public:
	NoteHelper();

	///Register a note listener.
	void addNoteListener( const std::string &noteName,
	                  	       INoteUpdateListener *noteListener );

	void removeNoteListener( const std::string &noteName,
	               	       INoteUpdateListener *noteListener );

	void removeAllNoteListener( const std::string &noteName );
};


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
			  isLoaded( false ){ note->version( 0 );		}
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
	
	boost::mutex        mutex;

	void loadPersistentNotes();

public:
	NoteManager( const std::string &persistentNotePath );
	NoteManager( );
	
	void setPersistentNotePath( const std::string &persistentNotePath );
	
	/**
	 * Mark the noteName as a persistent note, ie a note that is saved to disk.
	 */
	void registerPersistentNote( const std::string &noteName, NoteTag *note );

	/**
	 * Check for updated notes and signal the listener registered in noteHelper.
	 * It also reads the persistent loads saved to disk.
	 */
	void checkForUpdatedNotes( NoteHelper *noteHelper );


	/**
	 * Set a note. If it is a persistent note it is also saved
	 * to disk.
	 * 
	 * Don't use the note pointer after this function have returned.
	 */
	void setNote( const std::string &noteName, NoteTag *note );
	 
	/**
	 * Retrieve a note. To ensure that a persistent note can be
	 * retrieved checkForUpdatedNotes must be called first.
	 */
	boost::shared_ptr<NoteTag> getNote( const std::string &note );

};

}


#endif
