#include "parser.h"

int main(int argc, char *argv[])
{
  Scanner *my_scan = new Scanner(argv[1]);  //creates a scanner object 
  Parser *my_parser = new Parser(my_scan);  //creates a parser object 
  if(my_parser->parse_program()) {
    cout << "The parse has succeeded " << endl;
  }

  else
  {
    cout << "The parse has failed " << endl;
  }
  return 0;
}
