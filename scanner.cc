#include "scanner.h"

//Initializes the buffer that the scanner reads from
Scanner::Scanner(char *filename)
{
  buf = new Buffer(filename);
}

//deletes the buffer that was created
Scanner::~Scanner()
{
  delete buf;
}

/*uses case statements to determine what type of token was read from the buffer and returns it
 * starts in state zero and ends at state 100
 */
Token* Scanner:: next_token()
{
  int state = 0;
  string *attr = new string();
  char c;
  Token *type;
  KeywordToken *key = new KeywordToken();
  PuncToken *punc = new PuncToken();
  RelopToken *relop = new RelopToken(); 
  AddopToken *addop = new AddopToken();
  MulopToken *mulop = new MulopToken(); 
  NumToken *num = new NumToken();
  IdToken *id = new IdToken(); 
  EofToken *eof = new EofToken();
  while(state != 100)
  {
    c =buf->next_char();
    switch(state)
    {
    case 0:
      if(c == ';')
      {
	state = 1;
      }
      else if(c == ',')
      {
        state = 2;
      }
      else if(c == '(')
      { 
        state = 3;
      }
      else if(c == ')')
      {
        state = 4;
      }
      else if(c == ':')
      {
        state = 5;
      }
      else if(c == '=')
      {
        state = 7;
      }
      else if(c == '<')
      {
        state = 8;
      }
      else if(c == '>')
      {
        state = 11;
      }
      else if(c == '+')
      {
        state = 13;
      }
      else if(c == '-')
      {
        state = 14;
      }
      else if(c == '*')
      {
        state = 15;
      }
      else if(c == '/')
      {
        state = 16;
      }
      else if(is_digit(c))
      {
        state = 17;
        attr->push_back(c);
      }
      else if(c == 'a')
      {
        state = 18;
        attr->push_back(c);
      }
      else if(c == 'b' )
      {
        state = 21;
        attr->push_back(c);
      }
      else if( c == 'e')
      {
        state = 31;
        attr->push_back(c);
      }
      else if( c == 'i')
      {
        state = 37;
        attr->push_back(c);
      }
      else if( c == 'n')
      {
        state = 41;
        attr->push_back(c);
      }
      else if( c == 'o')
      {
        state = 44;
        attr->push_back(c);
      }
      else if( c == 'p')
      {
        state = 46;
        attr->push_back(c);
      }
      else if( c == 't')
      {
        state = 64;
        attr->push_back(c);
      }
      else if( c == 'w')
      {
        state = 68;
        attr->push_back(c);
      }
      else if(is_alpha(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else if( c == '$')
      {
        state = 73;
      }
      break;
    case 1:
      state = 100;
      buf->unread_char(c);
      punc->set_attribute(PUNC_SEMI);
      type = punc;
      break;
    case 2:
      state = 100;
      buf->unread_char(c);
      punc->set_attribute(PUNC_COMMA);
      type = punc;
      break;
    case 3:
      state = 100;
      buf->unread_char(c);
      punc->set_attribute(PUNC_OPEN);
      type = punc;  
      break;
    case 4:
      state = 100;
      buf->unread_char(c);
      punc->set_attribute(PUNC_CLOSE);
      type = punc;
      break;
    case 5:
      if(c =='=')
      {
        state = 6;
      }
      else
      {
      state = 100;
      buf->unread_char(c);
      punc->set_attribute(PUNC_COLON);
      type = punc;
      }
      break;
    case 6:
      state = 100;
      buf->unread_char(c);
      punc->set_attribute(PUNC_ASSIGN);
      type = punc;
      break;
    case 7:
      state = 100;
      buf->unread_char(c);
      relop->set_attribute(RELOP_EQ);
      type = relop;
      break;
    case 8:
      if(c =='>')
      {
        state = 9;
      }
      else if(c =='=')
      {
        state = 10;
      }
      else
      {
      state = 100;
      buf->unread_char(c);
      relop->set_attribute(RELOP_LT);
      type = relop;
      }
      break;
    case 9:
      state = 100;
      buf->unread_char(c);
      relop->set_attribute(RELOP_NEQ);
      type = relop;
      break;
    case 10:
      state = 100;
      buf->unread_char(c);
      relop->set_attribute(RELOP_LE);
      type = relop;
      break;
    case 11:
      if(c =='=')
      {
        state = 12;
      }
      else
      {
      state = 100;
      buf->unread_char(c);
      relop->set_attribute(RELOP_GT);
      type = relop;
      }
      break;
     case 12:
      state = 100;
      buf->unread_char(c);
      relop->set_attribute(RELOP_GE);
      type = relop;
      break;
     case 13:
      state = 100;
      buf->unread_char(c);
      addop->set_attribute(ADDOP_ADD);
      type = addop;
      break; 
     case 14:
      state = 100;
      buf->unread_char(c);
      addop->set_attribute(ADDOP_SUB);
      type = addop;
      break; 
     case 15:
      state = 100;
      buf->unread_char(c);
      mulop->set_attribute(MULOP_MUL);
      type = mulop;
      break;
     case 16:
      state = 100;
      buf->unread_char(c);
      mulop->set_attribute(MULOP_DIV);
      type = mulop;
      break;  
     case 17:
      if(is_digit(c))
      {
        attr->push_back(c);
        state = 17;
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        num->set_attribute(attr);
        type = num;
      }
      break;
     case 18:
      if(c == 'n')
      {
        state = 19;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 19:
      if(c == 'd')
      {
        state = 20;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 20:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
      state = 100;
      buf->unread_char(c);
      mulop->set_attribute(MULOP_AND);
      type = mulop;
      }
      break;
     case 21:
      if(c =='e')
      {
        state = 22;
        attr->push_back(c);
      }
      else if(c =='o')
      {
        state = 27;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 22:
      if(c =='g')
      {
        state = 23;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 23:
      if(c =='i')
      {
        state = 24;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 24:
      if(c =='n')
      {
        state = 25;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 25:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
      state = 100;
      buf->unread_char(c);
      key->set_attribute(KW_BEGIN);
      type = key;
      }
      break;
     case 27:
      if(c =='o')
      {
        state = 28;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 28:
      if(c =='l')
      {
        state = 29;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 29:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
      state = 100;
      buf->unread_char(c);
      key->set_attribute(KW_BOOL);
      type = key;
      }
      break;
     case 31:
      if(c =='l')
      {
        state = 32;
        attr->push_back(c);
      }
      else if(c == 'n')
      {
        state = 35;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 32:
      if(c =='s')
      {
        state = 33;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 33:
      if(c =='e')
      {
        state = 34;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 34:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
      state = 100;
      buf->unread_char(c);
      key->set_attribute(KW_ELSE);
      type = key;
      }
      break;
     case 35:
      if(c =='d')
      {
        state = 36;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 36:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
      state = 100;
      buf->unread_char(c);
      key->set_attribute(KW_END);
      type = key;
      }
      break;
     case 37:
      if(c =='f')
      {
        state = 38;
        attr->push_back(c);
      }
      else if(c == 'n')
      {
        state = 39;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 38:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
      state = 100;
      buf->unread_char(c);
      key->set_attribute(KW_IF);
      type = key;
      }
      break;
     case 39:
      if(c =='t')
      {
        state = 40;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 40:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
      state = 100;
      buf->unread_char(c);
      key->set_attribute(KW_INT);
      type = key;
      }
      break;
     case 41:
      if(c =='o')
      {
        state = 42;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 42:
      if(c =='t')
      {
        state = 43;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 43:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
	state = 100;
	buf->unread_char(c);
	key->set_attribute(KW_NOT);
	type = key;
      }
      break;
     case 44:
      if(c =='r')
      {
        state = 45;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 45:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
      state = 100;
      buf->unread_char(c);
      addop->set_attribute(ADDOP_OR);
      type = addop;
      }
      break;
     case 46:
      if(c =='r')
      {
        state = 47;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 47:
      if(c =='i')
      {
        state = 48;
        attr->push_back(c);
      }
      else if(c =='o')
      {
        state = 51;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 48:
      if(c =='n')
      {
        state = 49;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 49:
      if(c =='t')
      {
        state = 50;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 50:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
	state = 100;
	buf->unread_char(c);
	key->set_attribute(KW_PRINT);
	type = key;
      }
      break;
     case 51:
      if(c =='c')
      {
        state = 52;
        attr->push_back(c);
      }
      else if(c =='g')
      {
        state = 59;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
	id->set_attribute(attr);
        type = id;
      }
      break;
     case 52:
      if(c =='e')
      {
        state = 53;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 53:
      if(c =='d')
      {
        state = 54;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 54:
      if(c =='u')
      {
        state = 55;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 55:
      if(c =='r')
      {
        state = 56;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 56:
      if(c =='e')
      {
        state = 57;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 57:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
	state = 100;
	buf->unread_char(c);
	key->set_attribute(KW_PROCEDURE);
	type = key;
      }
      break;
     case 59:
      if(c =='r')
      {
        state = 60;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 60:
      if(c =='a')
      {
        state = 61;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
      }
      break;
     case 61:
     if(c =='m')
      {
        state = 62;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 62:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
	state = 100;
	buf->unread_char(c);
	key->set_attribute(KW_PROGRAM);
	type = key;
      }
      break;
     case 64:
      if(c =='h')
      {
        state = 65;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 65:
      if(c =='e')
      {
        state = 66;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 66:
      if(c =='n')
      {
        state = 67;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 67:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
	state = 100;
	buf->unread_char(c);
	key->set_attribute(KW_THEN);
	type = key;
      }
      break;
     case 68:
      if(c =='h')
      {
        state = 69;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 69:
      if(c =='i')
      {
        state = 70;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 70:
      if(c =='l')
      {
        state = 71;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 71:
      if(c =='e')
      {
        state = 72;
        attr->push_back(c);
      }
      else if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
     case 72:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else{
      state = 100;
      buf->unread_char(c);
      key->set_attribute(KW_WHILE);
      type = key;
      }
      break;
     case 73:
      state = 100;
      buf->unread_char(c);
      type = eof;
      break;
     case 99:
      if(is_alphanumeric(c))
      {
        state = 99;
        attr->push_back(c);
      }
      else
      {
        state = 100;
        buf->unread_char(c);
        id->set_attribute(attr);
        type = id;
      }
      break;
    }
  }
  delete attr;
  return type;
}

