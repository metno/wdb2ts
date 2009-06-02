
#include <string>
#include <File.h>

using namespace std;

std::string  
dnmi::file::
File::
path()const
{
  if(name_.empty())
    return string();

  string::size_type i;
  
  i=name_.find_last_of("/");
  
  if(i==string::npos)
    return string();
  
  string p=name_.substr(0, i);

  while(!p.empty() && (p[p.length()-1]=='/' || p[p.length()-1]==' '))
    p.erase(p.length()-1);

  return p;

}

std::string  
dnmi::file::
File::
file()const
{
  if(name_.empty())
    return string();

  string::size_type i;
  
  i=name_.find_last_of("/");
  
  if(i==string::npos)
    return name_;
  
  string p=name_.substr(i+1);

  while(!p.empty() && p[p.length()-1]==' ')
    p.erase(p.length()-1);

  return p;
}
