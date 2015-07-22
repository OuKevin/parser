#include "idtoken.h"

/* The constructor for the ID Token.  We call the base class constructor here.  
   This isn't strictly necessary, because the default constructor of the base
   class will be called anyway, but we do it here for completeness.
*/
IdToken::IdToken() : Token()
{
  set_token_type (TOKEN_ID);
  attribute = new string("UNINITIALIZED IDENTIFIER ATTRIBUTE");
}

//can be initialized with an attribute
IdToken::IdToken (string *attr) : Token()
{
  set_token_type (TOKEN_ID);
  attribute = new string(*attr);
}

//destructor, deletes attribute if there is one
IdToken::~IdToken()
{  if (attribute != NULL) {
    delete attribute;
  }
}

//accessor that returns the IdToken's attribute
string *IdToken::get_attribute() const
{
  string *attr = new string(*attribute);
  return attr;
}

//mutator that sets an attribute for the token
void IdToken::set_attribute(string *attr)
{ 
  if (attribute != NULL) {
    delete attribute;
  }
  attribute = new string (*attr);
}

//returns a string on the type and attribute
string *IdToken::to_string()
{
  string *attribute_name = new string ("ID:" + *attribute);
  return attribute_name;
}
