#ifndef __INOTEUPDATELISTENER_H__
#define __INOTEUPDATELISTENER_H__

namespace wdb2ts {

class INoteUpdateListener
{
public:
	virtual ~INoteUpdateListener(){}
	
	virtual void noteUpdated( const std::string &noteName, 
			                    boost::shared_ptr<NoteTag> note )=0;
	
};


}


#endif
