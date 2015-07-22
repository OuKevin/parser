#include "buffer.h"

//Constructs a buffer by opening a file and reading the contents into a list with the fill_buf method
Buffer::Buffer(char *filename)
{
  b = new list<char>;
  source_file= new ifstream;
  source_file->open(filename);
  fill_buf();
}

//closes the file and pops characters from the buffer until it is empty.
Buffer::~Buffer()
{
  source_file->close();
  while(!b->empty())
  {
    b->pop_front();
  }
}

//returns the next character in the buffer and then pops it out 
char Buffer::next_char()
{
  char temp;
  temp = b->front();
  b->pop_front();
  return temp;
}

//puts a character back into the buffer
void Buffer::unread_char(char c)
{
  b->push_front(c);
}

//prints an error if there is a problem
void Buffer::buffer_fatal_error()
{
  cout << "There is an error" << endl;
}

/*fills the buffer until it is full by reading characters in from the source file.
  comments are ignored and whitespace is condensed to a single space,
  a '$' is placed at the end of the buffer to indicate the end of the source file.
*/
void Buffer::fill_buf()
{
  while(b->size() < MAX_BUFFER_SIZE) //fills buffer while its not the maximum size
  {
    if(source_file->is_open()) //checks to see if file is open
    {
      char c;
      int lineLength = 256;
      char comments[lineLength]; //used in the getline method to ignore comments
      bool needSpace = false;  //used to figure out if a space will be needed to separate tokens
      while (source_file->good()) //will keep reading from the file until it reaches the end
      {  
        c = source_file->get();
        if(c == '#')
        {
          source_file->getline(comments,lineLength);
        }
        else if(is_whitespace(c))
        {
          needSpace = true;
        }
        else
        {
           if(needSpace)
           {
              b->push_back(SPACE);
           }
           needSpace = false;
	   b->push_back(c);
	} 
      }
      b->push_back('$'); //if done reading file push the EOF token into the buffer
    }
  }
}

//dumps the content of the buffer onto the screen.
void Buffer::dump_b()
{
  while(!b->empty())
  {
    cout<<b->front()<<endl;
    b->pop_front();
  }
}


