#ifndef EOFTOKEN_H
#define EOFTOKEN_H

using namespace std;

#include "token.h"
#include <string>

/* EofToken is a derived class (subclass) of Token */
class EofToken : public Token
{
 public:
  //constructor
  EofToken() ;
  
  //destructor
  ~EofToken();
  
   /* This method is overridden from the base class.  It is for
     debugging only.  Return a string that represents a reasonable
     representation of the token, including its type and attribute, if
     any.
  */
  string *to_string();
  
};

#endif
