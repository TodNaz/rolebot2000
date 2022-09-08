//
// Created by todnaz on 9/1/22.
//

#ifndef _ISEREALIZE_HH_
#define _ISEREALIZE_HH_

#include <ostream>
#include <istream>

class ISerealize {
 protected:
  ~ISerealize() = default;

 public:
  virtual void seralize(std::ostream& os) const = 0;
  virtual void derealize(std::istream& is)  = 0;
};

template<typename T>
void baseSerealize(T data, std::ostream& os)
{
  os.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

template<typename T>
void baseDerealize(T* data, std::istream& os)
{
  os.read((std::basic_istream<char>::char_type*) data, sizeof(data));
}

void strSerealize(std::string str, std::ostream& os);

void strDerealize(std::string& str, std::istream& os);

#endif //_ISEREALIZE_HH_
