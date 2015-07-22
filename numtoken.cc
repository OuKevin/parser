#include "numtoken.h"

/* The constructor for the NUM Token.  We call the base class constructor here.  
   This isn't strictly necessary, because the default constructor of the base
   class will be called anyway, but we do it here for completeness.
*/
NumToken::NumToken() : Token()
{
  set_token_type (TOKEN_NUM);
  attribute = new string("UNINITIALIZED NUM ATTRIBUTE");
}

//initializes the token with an attribute
NumToken::NumToken (string *attr) : Token()
{
  set_token_type (TOKEN_NUM);
  attribute = new string(*attr);
}

//destructor, deletes an attribute if the token has one
NumToken::~NumToken()
{  if (attribute != NULL) {
    delete attribute;
  }
}

//accessor that returns a token's attribute
string *NumToken::get_attribute() const
{
  string *attr = new string(*attribute);
  return attr;
}

//mutator that sets the attribute
void NumToken::set_attribute(string *attr)
{ 
  if (attribute != NULL) {
    delete attribute;
  }
  attribute = new string (*attr);
}

//returns a string of the type and attribute
string *NumToken::to_string()
{
  string *attribute_name = new string ("NUM:" + *attribute);
  return attribute_name;
}
