#ifndef NUMTOKEN_H
#define NUMTOKEN_H

using namespace std;

#include "token.h"
#include <string>

/* IdToken is a derived class (subclass) of Token */
class NumToken : public Token
{
 public:

  NumToken() ;

  NumToken(string *attribute);

  ~NumToken();

  /* 
     Get_Attribute returns the attribute of this token.  If the
     attribute is an object, we return a copy of the object.
  */
  string *get_attribute() const;

  /* 
     Set_attribute() sets the attribute of this token.  If the
     attribute is an object, then we store a pointer to a copy of the
     object with the token.
   */
  void set_attribute(string *attr);

  /* This method is overridden from the base class.  It is for
     debugging only.  Return a string that represents a reasonable
     representation of the token, including its type and attribute, if
     any.
  */
  string *to_string();

 private:
 // Storage for the attribute of the token.
  string *attribute;
};

#endif
