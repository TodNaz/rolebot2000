//
// Created by todnaz on 9/1/22.
//

#include "iSerealize.hh"

void strSerealize(std::string str, std::ostream& os)
{
  auto len = str.size();
  os.write(reinterpret_cast<const char*>(&len), sizeof(len));
  os.write(str.data(), len);
}

void strDerealize(std::string& str, std::istream& os)
{
  std::string::size_type len;
  os.read ((std::basic_istream<char>::char_type*) &len, sizeof(len));

  str.resize (len);
  os.read (str.data(), len);
}