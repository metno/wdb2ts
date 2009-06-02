#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "matrix.h"
#include "ReadMapFile.h"



using namespace std;

namespace {
typedef struct Header_ {
   char *name;
   bool isFloat;
   int   *ival;
   float *fval;
} Header;


struct HeaderParams{
   int ncols;
   int nrows;
   int cellsize;
   float xllcorner;
   float yllcorner; 
   int   NODATA_value;
   
   HeaderParams()
      : ncols(-1), nrows(-1), cellsize(-1),
        xllcorner(FLT_MIN), yllcorner(FLT_MIN),
        NODATA_value(INT_MIN)
        {}
   
   HeaderParams(const HeaderParams &h)
      : ncols(h.ncols), nrows(h.nrows), cellsize(h.cellsize),
        xllcorner(h.xllcorner), yllcorner(h.yllcorner),
        NODATA_value(h.NODATA_value)
        {}
    
    HeaderParams& operator=(const HeaderParams &rhs){
      if (this!=&rhs){
          ncols=rhs.ncols;
          nrows=rhs.nrows;
          cellsize=rhs.cellsize;
          xllcorner=rhs.xllcorner;
          yllcorner=rhs.yllcorner;
          NODATA_value=rhs.NODATA_value;
      }
      return *this;
    }
    
    bool valid()const { return ncols!=-1 && nrows!=-1 && cellsize!=-1 &&
                               xllcorner!=FLT_MIN && yllcorner!=FLT_MIN &&
                               NODATA_value!=INT_MIN; }
    
    friend std::ostream& operator<<(std::ostream &ost, const HeaderParams &hpar);
};


bool 
readHeader(ifstream &inst, HeaderParams &hpar, std::string &lastline, std::string &error);

}

Map* 
readMapFile(const std::string &filename, std::string &error)
{
   string::size_type i, i1;
   string   line;
   ifstream inst;
   bool hasError=false;
   string  name;
   string  sval;
   int     ival;
   int nNullVals=0;
   int nNotNullVals=0;
   int row=0;
   int col=0;
   Map *map;
   HeaderParams hpar;
   ostringstream ost;

   error.erase();
   
   inst.open(filename.c_str());
   
   if ( !inst ) {
      error += "ERROR: Cant open file <" + filename + ">!\n";
      return 0;
   }
   
   if ( !readHeader(inst, hpar, line, error) ) 
      return false;
    
   //cerr << hpar;
    
   try {
      map=new Map(hpar.ncols, hpar.nrows, hpar.cellsize,
                  hpar.xllcorner, hpar.yllcorner, 
                  hpar.NODATA_value);
      
      if ( !map->matrix.valid() ) {
         error="ERROR: NOMEM\n";
         delete map;
         return 0;
      }
   }
   catch(...){
      error="ERROR: NOMEM\n";
      return false;
   }    
    
    
   do { 
      col=0;
      i=line.find_first_not_of("\r\t ");
         
      while (i!=string::npos) {
         i1=line.find_first_of(" \t", i);
      
         if (i1==string::npos) 
            i1=line.size();

         sval=line.substr(i, i1-i);   
         
         if ( !sval.empty() ) {
            if ( sscanf(sval.c_str(), "%i", &ival) != 1) { 
               error += "ERROR: convert to int failed <" + sval + ">\n";
               hasError=true;
            } else if ( ival == hpar.NODATA_value ) { 
               nNullVals++;
            } else {
               nNotNullVals++;
            }
         } else {
            ost << "line: " << row << " at pos: " << i << ".";
            error += "WARNING: Unexpected empty value! " + ost.str() + "\n";
            ost.str("");
            hasError=true;
            ival = hpar.NODATA_value;
         }
            
         map->matrix.set(row, col, ival); 
         col++;
         i=line.find_first_not_of("\r\t ", i1);
      }
        
      if ( col != hpar.ncols) { 
         ost << "WARNING: line: " << row << ": Collumn Count (" << col << ") expected: " << hpar.ncols << ".";
         error += ost.str();
         ost.str("");
         hasError=true;
      }            
      
      row++;
   }while ( getline(inst, line) && !line.empty() );

   if ( row != hpar.nrows ) {
      ost << "ERROR: Number of rows " << row << ", expected " << hpar.nrows << endl;
      error += ost.str();
      ost.str("");
      hasError=true;
   }
   
   inst.close();
   
   if ( hasError ) {
      delete map;
      return 0;
   }
   
   return map;
}



namespace {
bool 
readHeader(ifstream &inst, HeaderParams &hpar, std::string &lastline, std::string &error)
{
   string::size_type i, i1;
   string  name;
   string  value;
   float fdummy;
   int   idummy;
    
   Header header[]={{"ncols", false, &hpar.ncols, &fdummy},
                   {"nrows", false, &hpar.nrows, &fdummy},
                   {"cellsize", false, &hpar.cellsize, &fdummy},
                   {"NODATA_value", false, &hpar.NODATA_value, &fdummy},
                   {"xllcorner", true, &idummy, &hpar.xllcorner},
                   {"yllcorner", true, &idummy, &hpar.yllcorner},
                   {0, false, &idummy, &fdummy}
                  };
 
   while ( getline(inst, lastline) ) {
      i=lastline.find_first_not_of("\r\t ");
      
      if (i==string::npos) 
         continue;

      i1=lastline.find_first_of(" \t\r", i);
      
      if (i1==string::npos)
         continue;
      
      name=lastline.substr(i, i1-i);
       
      if (!name.empty() && name[0]!='-' && !isdigit(name[0])) {
         i=lastline.find_first_not_of("\r\t ", i1);
      
         if (i==string::npos) 
            continue;

         value=lastline.substr(i);
         
         if (value.empty())
            continue;
            
         int k;
         for ( k=0; header[k].name; ++k ) {
            if ( strcmp(header[k].name, name.c_str()) == 0) {
               if ( header[k].isFloat ) {
                  if( sscanf(value.c_str(), "%f", header[k].fval ) != 1 ) {
                     error += "ERROR: Header (" + string(header[k].name) + "): float error <" + value + ">\n";
                     continue;
                  }
               }else{ 
                  if( sscanf(value.c_str(), "%i", header[k].ival) != 1 ) {
                     error += "ERROR: Header (" + string(header[k].name) + "): int error <" + value + ">\n" ;
                     continue;
                  }     
               }
               
               break; //out of the for loop
            }
         }
         
         if (!header[k].name ) 
            error += "WARNING: Header (" + name + "): unknown name!\n"; 
      } else {
         if ( !hpar.valid() ) 
            error += "ERROR: Missing header params.\n";
            
         return hpar.valid();
      }
   }
   
   error += "ERROR: Unexpected EOF.\n";
   
   return false;
}
   
std::ostream& 
operator<<(std::ostream &ost, const HeaderParams &hpar)
{
   ost << "File header"                         << endl
       << "-----------------------------------" << endl
       << "ncols:        " << hpar.ncols        << endl
       << "nrows:        " << hpar.nrows        << endl
       << "cellsize:     " << hpar.cellsize     << endl
       << "xllcorner:    " << hpar.xllcorner    << endl
       << "yllcorner:    " << hpar.yllcorner    << endl 
       << "NODATA_VALUE: " << hpar.NODATA_value << endl;
 
   return ost;
}
}
