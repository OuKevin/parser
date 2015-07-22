#include "parser.h"

Parser::Parser(Scanner *s) 
{
  lex = s;
  
  // Create the symbol table
  stab = new Symbol_Table();
  
  // Init the word variable.
  word = lex->next_token();
  
  //Create the emitter
  e = new Emitter();
  
  //Create the register allocator
  ra = new Register_Allocator(e,stab);
  
  // Init the environments to null
  current_env = NULL;
  main_env = NULL;
  parm_pos = -1;
}

Parser::~Parser() 
{
  if (lex != NULL) {
    delete lex;
  }
  if (word != NULL) {
    delete word;
  }
}

// If we have parsed the entire program, then word
// should be the EOF Token.  This function tests
// that condition.
bool Parser::done_with_input()
{
  return word->get_token_type() == TOKEN_EOF;
}

void Parser::parse_error(string *expected, Token *found)
{
  cout << "Expected " << expected->c_str()  << " found " << found->to_string()->c_str()  << endl;
}

void Parser::type_error(Token *where)
{
  cout << "Encountered a type error at " << where->to_string()->c_str()  << endl;
}

void Parser::undeclared_id_error(string *id, string *env)
{
  cout << "Undeclared identifier: " << id->c_str() << " at the environment: " << env->c_str() << endl;
}

bool Parser::parse_program()
{
   // PROGRAM -> program identifier ; DECL_LIST BLOCK ;
   // Predict (program identifier ; DECL_LIST BLOCK ;) == {program}

   // Match keyword program
   if (word->get_token_type() == TOKEN_KEYWORD 
       && static_cast<KeywordToken *>(word)->get_attribute() == KW_PROGRAM) {

     /* ADVANCE  - Notice that we only delete a token on an ADVANCE, and, if
       we ADVANCE, it is the ADVANCE code that is responsible for 
       getting the next token.
     */
     delete word; 
     word = lex->next_token();

     // Match identifier
     if (word->get_token_type() == TOKEN_ID) {	
       string *external= new string ("_EXTERNAL");
       
       //install to symbol table
       stab->install (static_cast<IdToken *>(word)->get_attribute(),
		  external,PROGRAM_T);
       current_env = static_cast<IdToken *>(word)->get_attribute();
       main_env = static_cast<IdToken *>(word)->get_attribute();
      
       string* start_label = static_cast<IdToken *>(word)->get_attribute();
       
        // ADVANCE
        delete word; 
        word = lex->next_token();
	// Match ;
	if (word->get_token_type() == TOKEN_PUNC 
	    && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI) {
	  // ADVANCE
	  delete word; 
	  word = lex->next_token();

	  // Match DECL_LIST - ACTION
	  if (parse_decl_list()) {
	    e->emit_label(start_label);
	    // Match BLOCK - ACTION
	    if (parse_block()) {
	 
	      if (word->get_token_type() == TOKEN_PUNC
		  && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI) {
		// ADVANCE
		delete word;
		word = lex->next_token();
		
	        //halt statement
		e->emit_halt();
	      
		//data directives
	        stab->print_directives(e);
	        //makes sure there are no more input after EOF token
		if(done_with_input())
		{
		  return true;
		} 
		else {
		  string *expected = new string ("'EOF TOKEN'");
		  parse_error (expected, word);
		  delete expected;
		  delete word;
		  return false;
		}
	
		// We failed to match the second semicolon
	      } else {
		string *expected = new string ("';'");
		parse_error (expected, word);
		delete expected;
		delete word;
		return false;
	      }
	      
	      // We failed to parse BLOCK
	    } else {
	      return false;
	    }
	    
	    // We failed to parse DECL_LIST
	  } else {
	    return false;
	  }
	  
	  // We failed to match the first semicolon
	} else {
	  string *expected = new string ("';'");
	  parse_error (expected, word);
	  delete expected;
	  delete word;
	  return false;
	}
	
	// We failed to match an identifier
     } else {
       string *expected = new string ("identifier");
       parse_error (expected, word);
       delete expected;
       delete word;
       return false;
     }
     
     // We failed to match the keyword program
   } else {
     string *expected = new string ("program");
     parse_error (expected, word);
     return false;
   }
   
   // We shouldn't reach this statement.
   return false;
}

bool Parser::parse_decl_list()
{
   /* DECL_LIST -> VARIABLE_DECL_LIST PROCEDURE_DECL_LIST
    * Predict (VARIABLE_DECL_LIST PROCEDURE_DECL_LIST) = 
    *          First (VARIABLE_DECL_LIST)
    *    union First (PROCEDURE_DECL_LIST)
    *    union Follow (DECL_LIST) = {identifier, procedure, begin}
    */
   if (word->get_token_type() == TOKEN_ID 
       || (word->get_token_type() == TOKEN_KEYWORD 
	  && static_cast<KeywordToken *>(word)->get_attribute() == KW_PROCEDURE)
       || (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_BEGIN)) {

     if (parse_variable_decl_list()) {  // ACTION

        if (parse_procedure_decl_list()) { // ACTION
	   return true;
	} else {
	   // parse_procedure_decl_list() failed
	  return false;
	}
      } else {
         // parse_variable_decl_list() failed
         return false;
      }

   } else {
      // word isn't in predict (DECL_LIST)
      string *expected = new string ("identifier, keyword \"begin\" or keyword \"procedure\"");
      parse_error (expected, word);
      return false;
   }
}

bool Parser::parse_variable_decl_list()
{
   /* VARIABLE_DECL_LIST -> VARIABLE_DECL ; VARIABLE_DECL_LIST
    * Predict (VARIABLE_DECL ; VARIABLE_DECL_LIST) = 
    *          First (VARIABLE_DECL) = {identifier}
    */
   if (word->get_token_type() == TOKEN_ID) {
     
     if (parse_variable_decl()) {  // ACTION
       
       // Match ;
	if (word->get_token_type() == TOKEN_PUNC 
	    && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI) {
	  // ADVANCE
	  delete word; 
	  word = lex->next_token();
	
	  if(parse_variable_decl_list())
	  {
	    return true;
	  } else {
	    //parse_variable_decl_list failed
	    return false;
	  }
	  // failed to match semicolon
	}  else {
	  string *expected = new string ("';'");
	  parse_error (expected, word);
	  delete expected;
	  delete word;
	  return false;
	}
     } else {
	  //parse_variable_decl failed
	  return false;
     }
     //If lambda production
   } else if((word->get_token_type() == TOKEN_KEYWORD 
	  && static_cast<KeywordToken *>(word)->get_attribute() == KW_PROCEDURE)
       || (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_BEGIN)) {
	return true;
   } else {
      // word isn't in predict (DECL_LIST)
      string *expected = new string ("identifier, keyword \"begin\", or keyword \"procedure\"");
      parse_error (expected, word);
      return false;
   }	    
	    
}

bool Parser::parse_variable_decl()
{
  expr_type standard_type_type;
   /* VARIABLE_DECL -> IDENTIFIER_LIST : STANDARD_TYPE
    * Predict (IDENTIFIER_LIST : STANDARD) = 
    *          First (IDENTIFIER_LIST) = {identifier}
    */
   if (word->get_token_type() == TOKEN_ID) {
     if (parse_identifier_list()) {  // ACTION
       // Match :
	if (word->get_token_type() == TOKEN_PUNC 
	    && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COLON) {
	  // ADVANCE
	  delete word; 
	  word = lex->next_token();
	   if (parse_standard_type(standard_type_type)) {  
	      //updates unknown id to standard_type_type
	      stab->update_type(standard_type_type);
	      return true;
	   } else {
	     //parse_standard_type failed
	     return false;
	   }
	 // failed to match colon
	}  else {
	  string *expected = new string ("':'");
	  parse_error (expected, word);
	  delete expected;
	  delete word;
	  return false;
	}
     } else {
       //parse_identifier_list failed
       return false;
     }
   } else {
      // word isn't in predict (VARIABLE_DECL)
      string *expected = new string ("identifier");
      parse_error (expected, word);
      return false;
   }
}

bool Parser::parse_procedure_decl_list()
{
   /* PROCEDURE_DECL_LIST-> PROCEDURE_DECL ; PROCEDURE_DECL_LIST
    * Predict (PROCEDURE_DECL ; PROCEDURE_DECL_LIST) = 
    *          First (PROCEDURE_DECL) = {procedure}
    */
   if (word->get_token_type() == TOKEN_KEYWORD 
       && static_cast<KeywordToken *>(word)->get_attribute() == KW_PROCEDURE) {
     
     if (parse_procedure_decl()) {  // ACTION
       
       // Match ;
	if (word->get_token_type() == TOKEN_PUNC 
	    && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI) {
	  // ADVANCE
	  delete word; 
	  word = lex->next_token();
	
	  //match parse_procedure_decl_list
	  if(parse_procedure_decl_list()){
	    return true;
	  } else {
	    //parse_procedure_decl_list failed;
	    return false;
	  }
	// failed to match semicolon
	}  else {
	  string *expected = new string ("';'");
	  parse_error (expected, word);
	  delete expected;
	  delete word;
	  return false;
	}
     } else {
       //parse_procedure_decl failed;
       return false;
     }
   } 
   //if lambda production
   else if(word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_BEGIN) {
       return true;
     } else {
      // word isn't in predict (VARIABLE_DECL)
      string *expected = new string ("keyword \"begin\" or keyword \"procedure\"");
      parse_error (expected, word);
      return false;
   }    
}

bool Parser::parse_procedure_decl()
{
   /* PROCEDURE_DECL -> procedure identifier ( ARG_LIST ) VARIABLE_DECL_LIST BLOCK
    * Predict (procedure identifier ( ARG_LIST ) VARIABLE_DECL_LIST BLOCK) = 
    *          First (procedure) = {procedure}
    */
   // Match keyword procedure
   if (word->get_token_type() == TOKEN_KEYWORD 
       && static_cast<KeywordToken *>(word)->get_attribute() == KW_PROCEDURE) {
     delete word; 
     word = lex->next_token();

     // Match identifier
     if (word->get_token_type() == TOKEN_ID) {	
       //install to symbol table
       stab->install (static_cast<IdToken *>(word)->get_attribute(),
		  current_env, PROCEDURE_T);
       current_env = static_cast<IdToken *>(word)->get_attribute();
       parm_pos = 0;
        delete word; 
        word = lex->next_token();
	
	//match (
	if (word->get_token_type() == TOKEN_PUNC
	  && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN) {
	    delete word; //advance
	    word = lex->next_token();
	    
	    //match ARG_LIST
	    if(parse_arg_list()) {
	      
	      //match )
	      if (word->get_token_type() == TOKEN_PUNC
		&& static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE) {
		  delete word; //advance
		  word = lex->next_token();
		
		  //match VARIABLE_DECL_LIST
		if(parse_variable_decl_list()) {
		  //match BLOCK
		  if(parse_block()) {
		    current_env = main_env;
		    return true;
		  } else {
		    //parse_block failed
		    return false;
		  }
		} else {
		  //parse_variable_decl_list failed
		  return false;
		}
		// failed to match )
	      }  else {
		string *expected = new string ("')'");
		parse_error (expected, word);
		delete expected;
		delete word;
		return false;
	      }
	  }  else {
	    //parse_arg_list failed
	    return false;
	  }
      }  else {
	    string *expected = new string ("'('");
	    parse_error (expected, word);
	    delete expected;
	    delete word;
	    return false;
	  }
     } else {
	    string *expected = new string ("'identifier'");
	    parse_error (expected, word);
	    delete expected;
	    delete word;
	    return false;
     }
   } else {
      // word isn't in predict (PROCEDURE_DECL)
      string *expected = new string ("keyword \"procedure\"");
      parse_error (expected, word);
      return false;
   }    
}

bool Parser::parse_arg_list()
{
  expr_type standard_type_type;
  
   /* ARG_LIST-> IDENTIFIER_LIST : STANDARD_TYPE ARG_LIST_HAT
    * Predict (IDENTIFIER_LIST : STANDARD_TYPE ARG_LIST_HAT) = 
    *          First (IDENTIFIER_LIST) = {identifier}
    */
   if (word->get_token_type() == TOKEN_ID) {
     
     if(parse_identifier_list()) {
       
       // Match :
	if (word->get_token_type() == TOKEN_PUNC 
	    && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COLON) {
	  // ADVANCE
	  delete word; 
	  word = lex->next_token();
	
	  if(parse_standard_type(standard_type_type)) {
	    stab->set_parm_pos(parm_pos);
	    parm_pos++;
	    stab->update_type(standard_type_type);
	    if(parse_arg_list_hat()) {
	      return true;
	    } else {
	      return false; //ARG_LIST_HAT failed
	    }
	  } else {
	    return false; //STANDARD_TYPE failed
	  }
	} else {
	    string *expected = new string ("':'");
	    parse_error (expected, word);
	    delete expected;
	    delete word;
	    return false;
	  }
     } else {
       return false; //parse_identifier_list failed
     }
   } else if (word->get_token_type() == TOKEN_PUNC
	&& static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE) {
      return true;
   } else {
      // word isn't in predict (ARG_LIST)
      string *expected = new string ("identifier or punctuation \")\"");
      parse_error (expected, word);
      return false;
   }    
}
		    
bool Parser::parse_arg_list_hat()
{
   /* ARG_LIST_HAT-> ; ARG_LIST
    * Predict (; ARG_LIST) = 
    *          First (;) = {;}
    */
   if (word->get_token_type() == TOKEN_PUNC 
	    && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI) {
	// ADVANCE
	delete word; 
	word = lex->next_token();
   
      if(parse_arg_list())
      {
	return true;
      } else {
	return false; //parse_arg_list failed
      }
   } //If lambda 
   else if (word->get_token_type() == TOKEN_PUNC
	&& static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE) {
      return true;
   } else {
      // word isn't in predict (ARG_LIST_HAT)
      string *expected = new string ("punctuation \";\" or punctuation \")\"");
      parse_error (expected, word);
      return false;
   }    
}
	     
bool Parser::parse_identifier_list()
{
   /* IDENTIFIER_LIST-> identifier IDENTIFIER_LIST_PRM
    * Predict (identifier IDENTIFIER_LIST_PRM) = 
    *          First (identifier) = {identifier}
    */     
    // Match identifier
     if (word->get_token_type() == TOKEN_ID) {		
        // ADVANCE
       stab->install (static_cast<IdToken *>(word)->get_attribute(),
		  current_env, UNKNOWN_T);
        delete word; 
        word = lex->next_token();
	
	if(parse_identifier_list_prm()){
	  return true;
	} else {
	  return false; //parse_identifier_list_prm failed
	}
     } else {
      // word isn't in predict (IDENTIFIER_LIST)
      string *expected = new string ("identifier");
      parse_error (expected, word);
      return false;
   }    
}

bool Parser::parse_identifier_list_prm()
{
   /* IDENTIFIER_LIST_PRM-> , identifier IDENTIFIER_LIST_PRM
    * Predict (, identifier IDENTIFIER_LIST_PRM) = 
    *      	First (,) = {,}
    */     
   if (word->get_token_type() == TOKEN_PUNC
	  && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COMMA) {
	    delete word; 
	    word = lex->next_token();
    // Match identifier
     if (word->get_token_type() == TOKEN_ID) {		
        // ADVANCE
	stab->install (static_cast<IdToken *>(word)->get_attribute(),
		  current_env, UNKNOWN_T);
        delete word; 
        word = lex->next_token();
	
	if(parse_identifier_list_prm()){
	  return true;
	} else {
	  return false; //parse_identifier_list_prm failed
	}
     } else {
      string *expected = new string ("identifier");
      parse_error (expected, word);
      return false;
      }  
   } //if lambda 
   else if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COLON) {
      return true;  
  } else {
      // word isn't in predict (IDENTIFIER_LIST_PRM)
      string *expected = new string ("punctuation \",\" or punctuation \":\"");
      parse_error (expected, word);
      return false;
   }    
   
}

bool Parser::parse_standard_type(expr_type &standard_type_type)
{
   /* STANDARD_TYPE-> int | bool
    * Predict (int | bool) = {int,bool}
    */     
    if(word->get_token_type() == TOKEN_KEYWORD 
       && static_cast<KeywordToken *>(word)->get_attribute() == KW_INT) {
        standard_type_type = INT_T;
	delete word; 
	word = lex->next_token();
	return true;
    } else if(word->get_token_type() == TOKEN_KEYWORD 
       && static_cast<KeywordToken *>(word)->get_attribute() == KW_BOOL) {
        standard_type_type = BOOL_T;
	delete word; 
	word = lex->next_token();
	return true;
    } else {
      string *expected = new string ("keyword \"int\" or keyword \"bool\"");
      parse_error (expected, word);
      return false;
   }    
}

bool Parser::parse_block()
{
   /* BLOCK-> begin STMT_LIST end
    * Predict (begin STMT_LIST end) = {begin}
    */  
   if(word->get_token_type() == TOKEN_KEYWORD 
       && static_cast<KeywordToken *>(word)->get_attribute() == KW_BEGIN) {
	delete word; 
	word = lex->next_token();
      
	if(parse_stmt_list()) {
	  
	   if(word->get_token_type() == TOKEN_KEYWORD 
	    && static_cast<KeywordToken *>(word)->get_attribute() == KW_END) {
	      delete word; 
	      word = lex->next_token();
	      return true;
	   } else {
	     string *expected = new string ("keyword \"end\"");
	     parse_error (expected, word);
	     return false;
	   }
	} else {
	  return false; //parse_stmt_list failed
	}
   } else {
      string *expected = new string ("keyword \"begin\"");
      parse_error (expected, word);
      return false;
   }
}

bool Parser::parse_stmt_list()
{
   /* STMT_LIST-> STMT ; STMT_LIST_PRM
    * Predict (STMT ; STMT_LIST_PRM) = {if,while,print,identifier}
    */  
      if (word->get_token_type() == TOKEN_ID 
       || (word->get_token_type() == TOKEN_KEYWORD 
	  && static_cast<KeywordToken *>(word)->get_attribute() == KW_WHILE)
       || (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_IF)
       || (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_PRINT)) {
	
	if(parse_stmt()){
	  //match ;
	  if (word->get_token_type() == TOKEN_PUNC
	    && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI) {
	      delete word; //advance 
	      word = lex->next_token();
	      
	      if(parse_stmt_list_prm()) {
		return true;
	      } else {
		return false; //parse_stmt_list_prm failed
	      }
	  } else {
	    string *expected = new string ("';'");
	    parse_error (expected, word);
	    delete expected;
	    delete word;
	    return false;
	  }
	} else {
	  return false; //parse_stmt failed
	}
      } else {
	string *expected = new string ("keyword \"print\" keyword \"if\" keyword \"while\" identifier");
	parse_error (expected, word);
	return false;
      }
}

bool Parser::parse_stmt_list_prm()
{
   /* STMT_LIST_PRM-> STMT ; STMT_LIST_PRM
    * Predict (STMT ; STMT_LIST_PRM) = {if,while,print,identifier}
    */  
      if (word->get_token_type() == TOKEN_ID 
       || (word->get_token_type() == TOKEN_KEYWORD 
	  && static_cast<KeywordToken *>(word)->get_attribute() == KW_WHILE)
       || (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_IF)
       || (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_PRINT)) {
	
	if(parse_stmt()){
	  
	  if (word->get_token_type() == TOKEN_PUNC
	    && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI) {
	      delete word; 
	      word = lex->next_token();
	      
	      if(parse_stmt_list_prm()) {
		return true;
	      } else {
		return false; //parse_stmt_list_prm failed
	      }
	  } else {
	    string *expected = new string ("';'");
	    parse_error (expected, word);
	    delete expected;
	    delete word;
	    return false;
	  }
	} else {
	  return false; //parse_stmt failed
	}
      }//if lambda 
      else if(word->get_token_type() == TOKEN_KEYWORD 
	  && static_cast<KeywordToken *>(word)->get_attribute() == KW_END) {
	return true;
      } else {
	string *expected = new string ("keyword \"print\" keyword \"if\" keyword \"while\" keyword \"end\" identifier");
	parse_error (expected, word);
	return false;
      }
}

bool Parser::parse_stmt()
{
  expr_type stmt_ass_proc_tail_type;
  Register *stmt_ass_proc_tail_reg;
   /* STMT -> IF_STMT | WHILE_STMT | PRINT_STMT | identifier STMT_ASS_PROC_TAIL
    * Predict (IF_STMT | WHILE_STMT | PRINT_STMT | identifier STMT_ASS_PROC_TAIL) = {if,while,print,identifier}
    */  
   if (word->get_token_type() == TOKEN_ID 
       || (word->get_token_type() == TOKEN_KEYWORD 
	  && static_cast<KeywordToken *>(word)->get_attribute() == KW_WHILE)
       || (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_IF)
       || (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_PRINT)) {
     
     if(word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_IF) {
	  if(parse_if_stmt()) {
	    return true;
	  } else {
	    return false; //if parse_it_stmt failed
	  }
     }
     else if (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_WHILE) {
       
	  if(parse_while_stmt()){
	    return true;
	  } else {
	    return false; //parse_while_stmt
	  }
     } 
     else if (word->get_token_type() == TOKEN_KEYWORD
	   && static_cast<KeywordToken *>(word)->get_attribute() == KW_PRINT) { 
       
	  if(parse_print_stmt()) {
	    return true;
	  } else {
	    return false; //parse_print_stmt failed
	  }
     } 
     else if (word->get_token_type() == TOKEN_ID) {		
        // ADVANCE
	  bool isDecl = stab->is_decl(static_cast<IdToken *>(word)->get_attribute(),current_env);
	  if(!isDecl)
	  {
	    undeclared_id_error(static_cast<IdToken *>(word)->get_attribute(), current_env);
	    exit(1);
	  }
	  expr_type id_type = stab->get_type(static_cast<IdToken *>(word)->get_attribute(),current_env);  
	  string *the_id = static_cast<IdToken *>(word)->get_attribute();
	  delete word; 
	  word = lex->next_token();
	 
	if(parse_stmt_ass_proc_tail(stmt_ass_proc_tail_type,stmt_ass_proc_tail_reg)) {
	  
	  if(id_type != stmt_ass_proc_tail_type)
	  {
	    type_error(word);
	    exit (1);
	  } 
	  //moves value in register to identifier
	  e->emit_move(the_id,stmt_ass_proc_tail_reg);
	  //deallocate register
	  ra->deallocate_register(stmt_ass_proc_tail_reg);
	  
	  return true;
     } else {
       return false; //parse_stmt_ass_proc_tail failed
     }
    } 
   } else {
	string *expected = new string ("keyword \"print\" keyword \"if\" keyword \"while\" identifier");
	parse_error (expected, word);
	return false;
   }
   
}
	 
bool Parser::parse_stmt_ass_proc_tail(expr_type &stmt_ass_proc_tail_type, Register *&stmt_ass_proc_tail_reg)
{
  expr_type assignment_stmt_tail_type;
   /* STMT_ASS_PROC_TAIL -> ASSIGNMENT_STMT_TAIL
    * Predict (ASSIGNMENT_STMT_TAIL | PROCEDURE_CALL_STMT_TAIL) = {:=, (}
    */  
   if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_ASSIGN){
     
     if(parse_assignment_stmt_tail(assignment_stmt_tail_type,stmt_ass_proc_tail_reg)){
       stmt_ass_proc_tail_type = assignment_stmt_tail_type;
       return true;
     } else {
       return false; //parse_assignment_stmt_tail failed
     }
   } else if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN){
     
      if(parse_procedure_call_stmt_tail()) {
	stmt_ass_proc_tail_type = PROCEDURE_T;
	return true;
      } else {
	return false; //parse_procedure_call_stmt_tail failed
      }
   } else {
	string *expected = new string ("punctuation \":=\" punctuation \"(\" ");
	parse_error (expected, word);
	return false;
   }
   
}

bool Parser::parse_assignment_stmt_tail(expr_type &assignment_stmt_tail_type,Register *&assignment_stmt_tail_reg)
{
  expr_type the_expr_type;
   /* ASSIGNMENT_STMT_TAIL -> := EXPR
    * Predict (:= EXPR) = {:=}
    */  
   if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_ASSIGN){
	delete word; 
	word = lex->next_token();
	
	if(parse_expr(the_expr_type,assignment_stmt_tail_reg)) {
	  assignment_stmt_tail_type = the_expr_type;
	  
	  return true;
	} else {
	  return false; //parse_expr failed
	}
   } else {
	    string *expected = new string ("':='");
	    parse_error (expected, word);
	    delete expected;
	    delete word;
	    return false;
   }
}

bool Parser::parse_if_stmt()
{
  expr_type the_expr_type;
  
  /* Parse_expr will leave the register containing the value
     of the expression here. */
  Register *expr_reg;
  
   /* IF_STMT -> if EXPR then BLOCK IF_STMT_HAT
    * Predict (if EXPR then BLOCK IF_STMT_HAT) = {if}
    */  
   if(word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_IF) {
	delete word; 
	word = lex->next_token();
	
	if (parse_expr(the_expr_type, expr_reg)) {
	  
	  if(the_expr_type != BOOL_T)
	  {
	    type_error(word);
	    exit (1);
	  }
	  
	   /* code generation */
	  string *if_false = e->get_new_label ("if_false");
	  string *if_done = e->get_new_label ("if_done");

	  e->emit_branch (BREZ, expr_reg, if_false);
	  ra->deallocate_register (expr_reg);
      
	  if(word->get_token_type() == TOKEN_KEYWORD
	    && static_cast<KeywordToken *>(word)->get_attribute() == KW_THEN) {
	      delete word; 
	      word = lex->next_token();
	      
	      if(parse_block()){
		
		/* code generation */
		e->emit_branch (if_done);

		e->emit_label (if_false);
		delete if_false;
		
		if(parse_if_stmt_hat()){
		  
		  /* code generation */
		  e->emit_label (if_done);
		  delete if_done;
		  /**/
	    
		  return true;
		} else {
		  return false; //parse_if_stmt_hat failed
		}
	      } else {
		return false; //parse_block failed
	      }
	  } else {
	      string *expected = new string ("'then'");
	      parse_error (expected, word);
	      delete expected;
	      delete word;
	      return false;
	  }
	} else {
	  return false; //parse_expr failed
	}
   } else {
	string *expected = new string ("Keyword:'if'");
	parse_error (expected, word);
	delete expected;
	delete word;
	return false;
   }
}

bool Parser::parse_if_stmt_hat()
{
   /* IF_STMT_HAT -> else BLOCK
    * Predict (else BLOCK) = {else, ;}
    */  
   if(word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_ELSE) {
	delete word; 
	word = lex->next_token();
	
	if(parse_block())
	{
	  return true;
	} else {
	  return false; //parse_block failed
	}
   } //if lambda
   else if(word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI){
	return true;
   } else {
	string *expected = new string ("keyword \"else\" punctuation \";\" ");
	parse_error (expected, word);
	return false;
   }
}

bool Parser::parse_while_stmt()
{
  expr_type the_expr_type;
  
  Register *expr_reg;
   /* WHILE_STMT -> while EXPR BLOCK
    * Predict (while EXPR BLOCK) = {while}
    */  
   if(word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_WHILE) {
	delete word; 
	word = lex->next_token();
	
	if(parse_expr(the_expr_type,expr_reg)) {
	  
	  if(the_expr_type != BOOL_T)
	  {
	    type_error(word);
	    exit (1);
	  }
	  //code generation for while stmt
	  string *while_true = e->get_new_label("while_true");
	  string *while_done = e->get_new_label("while_done");
	  
	  e->emit_label(while_true);
	  e->emit_branch(BREZ,expr_reg,while_done);
	  //deallocate register
	  ra->deallocate_register(expr_reg);
	 if(parse_block()) {
	   e->emit_branch(while_true);
	   e->emit_label(while_done);
	   return true;
	 } else {
	  return false; //parse_block failed
	 }
	} else {
	  return false; //parse_expr failed
	}
   } else {
      string *expected = new string ("'while'");
      parse_error (expected, word);
      delete expected;
      delete word;
      return false;
   }
}

bool Parser::parse_print_stmt()
{
   expr_type the_expr_type;
   
   Register *expr_reg;
   /* PRINT_STMT -> print EXPR 
    * Predict (print EXPR ) = {print}
    */  
   if(word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_PRINT) {
	delete word; 
	word = lex->next_token();
	
	if(parse_expr(the_expr_type,expr_reg)) {
	  
	  if(the_expr_type != INT_T)
	  {
	    type_error(word);
	    exit (1);
	  } 
	  //prints the register
	  e->emit_1addr(OUTB,expr_reg);
	  ra->deallocate_register(expr_reg);
	  return true;
	} else {
	  return false; //parse_expr failed
	}
   } else {
      string *expected = new string ("'print'");
      parse_error (expected, word);
      delete expected;
      delete word;
      return false;
   }
}

bool Parser::parse_procedure_call_stmt_tail()
{
  
   /* PROCEDURE_CALL_STMT_TAIL -> ( EXPR_LIST ) 
    * Predict (( EXPR_LIST ) ) = {(}
    */  
   if(word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN) {
	delete word; 
	word = lex->next_token();
	
	parm_pos = 0;
	if(parse_expr_list()) {
	  
	  if(word->get_token_type() == TOKEN_PUNC
	    && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE) {
	      delete word; 
	      word = lex->next_token();
	      return true;
	  } else {
	      string *expected = new string ("')'");
	      parse_error (expected, word);
	      delete expected;
	      delete word;
	      return false;
	  }
	} else {
	  return false; //parse_expr_list failed
	}
   } else {
	string *expected = new string ("'('");
	parse_error (expected, word);
	delete expected;
	delete word;
	return false;
   }
}

bool Parser::parse_expr_list()
{
  expr_type the_expr_type;
  
  Register *expr_reg;
   /* EXPR_LIST -> EXPR EXPR_LIST_HAT
    * Predict (EXPR EXPR_LIST_HAT ) = {identifier,num, (, not, +, -}
    */  
   if (word->get_token_type() == TOKEN_ID
     || word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN
     || word->get_token_type() == TOKEN_NUM
     || word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT
     || word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD
     || word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB ) {
     
      if(parse_expr(the_expr_type,expr_reg)) {
	
	if(the_expr_type != (stab->get_type(current_env,parm_pos)))
	{
	  type_error(word);
	  exit (1);
	}
	parm_pos++;
	
	if(parse_expr_list_hat()) {
	  return true;
	} else {
	  return false; //parse_expr_list_hat failed
	}
      } else {
	return false; //parse_expr failed
      } 
   } //if lambda 
   else if(word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE) {  
	return true;
   } else {
	string *expected = new string (" identifier num keyword \"not\" punctuation \"(\"  punctuation \")\"  addop \"+\" addop \"-\" ");
	parse_error (expected, word);
	return false;
   }
}

bool Parser::parse_expr_list_hat()
{
   /* EXPR_LIST_HAT -> , EXPR_LIST
    * Predict (, EXPR_LIST ) = {,}
    */ 
   if(word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COMMA) {
	delete word; 
	word = lex->next_token();
	
	if(parse_expr_list()) {
	  return true;
	} else {
	  return false; //parse_expr_list failed
	}
   } //if lambda
   else if(word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE) {  
	return true;
   } else {
	string *expected = new string (" punctuation \",\"  punctuation \")\"  ");
	parse_error (expected, word);
	return false;
   }
}

bool Parser::parse_expr(expr_type &the_expr_type,Register *&expr_reg)
{
   expr_type simple_expr_type, expr_hat_type;
   /* EXPR -> , SIMPLE_EXPR EXPR_HAT
    * Predict (SIMPLE_EXPR EXPR_HAT ) = {identifier, num, (, +, -, not}
    */ 
   if (word->get_token_type() == TOKEN_ID
     || word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN
     || word->get_token_type() == TOKEN_NUM
     || word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT
     || word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD
     || word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB ) {
     
     if(parse_simple_expr(simple_expr_type,expr_reg)) {
       
       if(parse_expr_hat(expr_hat_type,expr_reg)) {
	 if(expr_hat_type == NO_T)
	 {
	   the_expr_type = simple_expr_type;
	 } else if(simple_expr_type == INT_T && expr_hat_type == INT_T){
	   the_expr_type = BOOL_T;
	 } else {
	   type_error(word);
	   exit (1);
	 }
	 return true;
       } else {
	 return false; //parse_expr_hat failed
       }
     } else {
       return false; //parse_simple_expr failed
     }
   } else {
	string *expected = new string (" identifier num keyword \"not\" punctuation \"(\" addop \"+\" addop \"-\" ");
	parse_error (expected, word);
	return false;
   }
}

bool Parser::parse_expr_hat(expr_type &expr_hat_type,Register *&expr_hat_reg)
{
  expr_type simple_expr_type;
  Register *simple_expr_reg;
  relop_attr_type the_relop;
   /* EXPR_HAT() -> ,relop SIMPLE_EXPR
    * Predict (relop SIMPLE_EXPR) = {relop}
    */ 
   if (word->get_token_type() == TOKEN_RELOP) {
     the_relop = static_cast<RelopToken*>(word)->get_attribute();
     delete word; 
     word = lex->next_token();
     
     if(parse_simple_expr(simple_expr_type,simple_expr_reg)) {
 
       if(simple_expr_type==INT_T)
       {
	 expr_hat_type = INT_T;
       } else {
	 type_error(word);
	 exit (1);
       }
       
       //code generation
        string *is_true = e->get_new_label("is_true");
	string *is_false = e->get_new_label("is_false");
       if(the_relop == RELOP_EQ) //code for :=
       {
	e->emit_2addr(SUB,expr_hat_reg,simple_expr_reg);
	e->emit_branch(BREZ,expr_hat_reg,is_true);
	e->emit_branch(is_false);
       }
       else if(the_relop == RELOP_NEQ) // code for <>
       {
	e->emit_2addr(SUB,expr_hat_reg,simple_expr_reg);
	e->emit_branch(BREZ,expr_hat_reg,is_false);
	e->emit_branch(is_true);
       }
       else if(the_relop == RELOP_GT) // code for >
       {
	e->emit_2addr(SUB,expr_hat_reg,simple_expr_reg);
	e->emit_branch(BRPO,expr_hat_reg,is_true);
	e->emit_branch(BREZ,expr_hat_reg,is_false);
	e->emit_branch(BRNE,expr_hat_reg,is_false);
       }
       else if(the_relop == RELOP_GE) // for for >=
       {
	e->emit_2addr(SUB,expr_hat_reg,simple_expr_reg);
	e->emit_branch(BREZ,expr_hat_reg,is_true);
	e->emit_branch(BRPO,expr_hat_reg,is_true);
	e->emit_branch(BRNE,expr_hat_reg,is_false);
	
       }
       else if(the_relop == RELOP_LT) // code for <
       {
	e->emit_2addr(SUB,expr_hat_reg,simple_expr_reg);
	e->emit_branch(BRNE,expr_hat_reg,is_true);
	e->emit_branch(BREZ,expr_hat_reg,is_false);
	e->emit_branch(BRPO,expr_hat_reg,is_false);
       }
       else if(the_relop == RELOP_LE) // code for <=
       {
	e->emit_2addr(SUB,expr_hat_reg,simple_expr_reg);
	e->emit_branch(BREZ,expr_hat_reg,is_true);
	e->emit_branch(BRNE,expr_hat_reg,is_true);
	e->emit_branch(BRPO,expr_hat_reg,is_false);
       }
       //moves the value 1 to register if true, 0 otherwise
       string *is_done = e->get_new_label("is_done");
       e->emit_label(is_true);
       e->emit_move(expr_hat_reg,1);
       e->emit_branch(is_done);
       e->emit_label(is_false);
       e->emit_move(expr_hat_reg,0);
       e->emit_branch(is_done);
       e->emit_label(is_done);
       //free up register
       ra->deallocate_register(simple_expr_reg);
       return true;
     } else {
       return false; //parse_simple_expr failed
     }
   } //if lamda
   else if (word->get_token_type() == TOKEN_PUNC && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI
	|| word->get_token_type() == TOKEN_PUNC && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COMMA
	|| word->get_token_type() == TOKEN_PUNC && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE
	|| word->get_token_type() == TOKEN_KEYWORD && static_cast<KeywordToken *>(word)->get_attribute() == KW_THEN
	|| word->get_token_type() == TOKEN_KEYWORD && static_cast<KeywordToken *>(word)->get_attribute() == KW_BEGIN) {
      expr_hat_type = NO_T;
      return true;
   } else {
	string *expected = new string (" relop  keyword \"then\" keyword \"begin\" punctuation \";\" punctuation \",\" punctuation \")\" ");
	parse_error (expected, word);
	return false;
   }
}

bool Parser::parse_simple_expr(expr_type &simple_expr_type,Register *&simple_expr_reg)
{
  expr_type simple_expr_prm_type, term_type;
  
  
   /* SIMPLE_EXPR -> ,TERM SIMPLE_EXPR_PRM
    * Predict (,TERM SIMPLE_EXPR_PRM) = {identifier, num, (, +, -, not}
    */ 
   if (word->get_token_type() == TOKEN_ID
     || word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN
     || word->get_token_type() == TOKEN_NUM
     || word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT
     || word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD
     || word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB ) {
     
     if(parse_term(term_type,simple_expr_reg)) {
       
       if(parse_simple_expr_prm(simple_expr_prm_type,simple_expr_reg)) {
	 
	 if(simple_expr_prm_type == NO_T)
	 {
	   simple_expr_type = term_type;
	 } else if(term_type == simple_expr_prm_type) {
	   simple_expr_type = term_type;
	 } else {
	   type_error(word);
	   exit (1);
	 }
	
	 return true;
       } else {
	 return false; //parse_simple_expr_prm failed
       }
     } else {
       return false; //parse_term failed
     }
   } else {
	string *expected = new string (" identifier num keyword \"not\" punctuation \"(\" addop \"+\" addop \"-\" ");
	parse_error (expected, word);
	return false;
   }
}

bool Parser::parse_simple_expr_prm(expr_type &simple_expr_prm_type, 
				   Register *&simple_expr_prm_reg)
{
  // For semantic analysis
  expr_type addop_type, term_type, simple_expr_prm_1_type;
  // For code generation
  Register *term_reg;
  addop_attr_type the_addop;

  // SIMPLE_EXPR_PRM -> addop TERM SIMPLE_EXPR_PRM
  // Predict (SIMPLE_EXPR_PRM -> addop TERM SIMPLE_EXPR_PRM) = {addop}
  if (word->get_token_type() == TOKEN_ADDOP) {

    // semantic analysis
    if (static_cast<AddopToken *>(word)->get_attribute() == ADDOP_OR) {
      addop_type = BOOL_T;
    } else {
      addop_type = INT_T;
    }

    // Remember the actual addop operation for code generation.
    the_addop = static_cast<AddopToken *>(word)->get_attribute();
		
    delete word; 
    word = lex->next_token();

    /* Parse_term will tell us the register where it left the result
       of evaluating the term.  Generate code.
    */
    if (parse_term(term_type, term_reg)) {

      /* Code generation */
      if (the_addop == ADDOP_SUB) { // integer subtraction
	e->emit_2addr (SUB, simple_expr_prm_reg, term_reg);
      } else { // integer addition or boolean or
	e->emit_2addr (ADD, simple_expr_prm_reg, term_reg);
      }
      // done with term_reg
      ra->deallocate_register (term_reg);

      if (parse_simple_expr_prm(simple_expr_prm_1_type, simple_expr_prm_reg)) {

	/* semantics */
	if (simple_expr_prm_1_type == NO_T) {
	  if (addop_type == term_type) {
	    simple_expr_prm_type = addop_type;
	  } else {
	    type_error(word);
	  }
	} else if (addop_type == term_type 
		   && term_type == simple_expr_prm_1_type) {
	  simple_expr_prm_type = term_type;
	} else {
	  type_error(word);
	}

	return true;
      } else {
	return false;
      }

    } else {
      return false;
    }

    // SIMPLE_EXPR_PRM -> lambda
    // Follow (SIMPLE_EXPR_PRM) = {relop, ;, then, begin, ,, )}
  } else if ((word->get_token_type() == TOKEN_RELOP) 
	     || (word->get_token_type() == TOKEN_PUNC 
		 && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI) 
	     || (word->get_token_type() == TOKEN_KEYWORD 
		 && static_cast<KeywordToken *>(word)->get_attribute() == KW_THEN) 
	     || (word->get_token_type() == TOKEN_KEYWORD 
		 && static_cast<KeywordToken *>(word)->get_attribute() == KW_BEGIN) 
	     || (word->get_token_type() == TOKEN_PUNC 
		 && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COMMA)
	     || (word->get_token_type() == TOKEN_PUNC 
		 && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE)) {
    /* sematics */
    simple_expr_prm_type = NO_T;
    /* not code generation required */
    return true;

  } else {
    expected = new string ("additive op, relational op, ';', then, begin, ',' or ')'");
    parse_error (expected, word);
    return false;
  }
}

bool Parser::parse_term(expr_type &term_type,Register *&term_reg)
{
  expr_type term_prm_type, factor_type;
   /* TERM -> ,FACTOR TERM_PRM
    * Predict (,FACTOR TERM_PRM) = {identifier, num, (, +, -, not}
    */ 
   if (word->get_token_type() == TOKEN_ID
     || word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN
     || word->get_token_type() == TOKEN_NUM
     || word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT
     || word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD
     || word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB ) {
     
     if(parse_factor(factor_type,term_reg)) {
       
       if(parse_term_prm(term_prm_type,term_reg)) {
	 
	 if(term_prm_type == NO_T)
	 {
	   term_type = factor_type;
	 } else if(factor_type == term_prm_type) {
	   term_type = factor_type;
	 } else {
	   type_error(word);
	   exit (1);
	 }
	 return true;
       } else {
	 return false; //parse_term_prm failed
       }
     } else {
       return false; //parse_factor failed
     }
   } else {
	string *expected = new string (" identifier num keyword \"not\" punctuation \"(\" addop \"+\" addop \"-\" ");
	parse_error (expected, word);
	return false;
   }
  
}

bool Parser::parse_term_prm(expr_type &term_prm_type, Register *&term_prm_reg)
{
  expr_type term_prm1_type, factor_type, mulop_type;
  mulop_attr_type the_mulop;
  
  Register *factor_reg;
   /* TERM_PRM -> ,mulop FACTOR TERM_PRM
    * Predict (,FACTOR TERM_PRM) = {mulop}
    */ 
   if (word->get_token_type() == TOKEN_MULOP) {
     
     if(word->get_token_type() == TOKEN_MULOP
	  && static_cast<MulopToken *>(word)->get_attribute() == MULOP_MUL
	  || word->get_token_type() == TOKEN_MULOP
	  && static_cast<MulopToken *>(word)->get_attribute() == MULOP_DIV) 
      {
	mulop_type = INT_T;
      } 
      else if(word->get_token_type() == TOKEN_MULOP
	&& static_cast<MulopToken *>(word)->get_attribute() == MULOP_AND) 
	{
	  mulop_type = BOOL_T;
	}
     the_mulop = static_cast<MulopToken *>(word)->get_attribute(); //the operation
     
     delete word; 
     word = lex->next_token();
     
     if(parse_factor(factor_type,factor_reg)) {
       
       if(the_mulop == MULOP_MUL) { //integer multiplication or boolean AND
	 e->emit_2addr(MUL,term_prm_reg,factor_reg);
       } else {//integer division
	 e->emit_2addr(DIV,term_prm_reg,factor_reg);
       }
       //deallocates register
       ra->deallocate_register(factor_reg);
       if(parse_term_prm(term_prm1_type,term_prm_reg)) {
	 
	 if(term_prm1_type == NO_T && mulop_type == factor_type)
	 {
	   term_prm_type = mulop_type;
	 } 
	 else if(mulop_type == factor_type && factor_type == term_prm1_type)
	 {
	   term_prm_type = mulop_type;
	 }
	 else
	 {
	   type_error(word);
	   exit (1);
	 }
	 return true;
       } else {
	 return false; //parse_term_prm failed
       }
     } else {
       return false; //parse_factor failed
     }
   } else if(word->get_token_type() == TOKEN_RELOP
	|| word->get_token_type() == TOKEN_ADDOP
	|| word->get_token_type() == TOKEN_PUNC
	 && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI
	|| word->get_token_type() == TOKEN_PUNC
	 && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COMMA
	|| word->get_token_type() == TOKEN_PUNC
	  && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE
	|| word->get_token_type() == TOKEN_KEYWORD
	  && static_cast<KeywordToken *>(word)->get_attribute() == KW_THEN
	|| word->get_token_type() == TOKEN_KEYWORD
	  && static_cast<KeywordToken *>(word)->get_attribute() == KW_BEGIN) {
     term_prm_type = NO_T;
     return true;
   } else {
	string *expected = new string (" mulop addop relop  keyword \"then\" keyword \"begin\" punctuation \";\" punctuation \",\" punctuation \")\" ");
	parse_error (expected, word);
	return false;
   }
     
}

bool Parser::parse_factor(expr_type &factor_type,Register *&factor_reg)
{
  expr_type the_expr_type, factor1_type;
  
   /* FACTOR-> ,identifier | num | ( EXPR ) | not FACTOR | SIGN FACTOR
    * Predict (,identifier | num | ( EXPR ) | not FACTOR | SIGN FACTOR) = {identifier, num, (, +, -, not}
    */ 
   if(word->get_token_type() == TOKEN_ID) {
     bool isDecl = stab->is_decl(static_cast<IdToken *>(word)->get_attribute(), current_env);
     if(isDecl) {
       expr_type id_type =  stab->get_type(static_cast<IdToken *>(word)->get_attribute(), current_env);
       factor_type = id_type;
     } else {
        undeclared_id_error(static_cast<IdToken *>(word)->get_attribute(), current_env);
	exit(1);
     }
     //allocates a register
     factor_reg = ra->allocate_register();
     e->emit_move(factor_reg,static_cast<IdToken *>(word)->get_attribute()); //moves id to register
     delete word; 
       word = lex->next_token();
     return true;
   } else if(word->get_token_type() == TOKEN_NUM) {
       factor_type = INT_T;
       //allocates a register
       factor_reg = ra->allocate_register();
       e->emit_move(factor_reg,static_cast<NumToken *>(word)->get_attribute()); //moves num to register
       delete word; 
       word = lex->next_token();
       return true;
     }
   else if(word->get_token_type() == TOKEN_PUNC
	  && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN)
   {
     delete word; 
     word = lex->next_token();
     if(parse_expr(the_expr_type, factor_reg))
     {
       
       if(word->get_token_type() == TOKEN_PUNC
	  && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE) {
	 factor_type = the_expr_type;
	 delete word; 
	 word = lex->next_token();
	 return true;
       } else {
	  string *expected = new string ("')'");
	  parse_error (expected, word);
	  delete expected;
	  delete word;
	  return false;
       }
     } else {
       return false; //parse_expr failed
     }
   } else if(word->get_token_type() == TOKEN_KEYWORD
	  && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT) {
	  
	  delete word; 
	  word = lex->next_token();
	  
	  if(parse_factor(factor1_type,factor_reg)) {
	    
	    if(factor1_type == BOOL_T) {
	      factor_type = BOOL_T;
	    } else {
	      type_error(word);
	      exit (1);
	    }   
	    //Code to swap false to true and true to false
	    e->emit_1addr(NOT,factor_reg);
	    string *is_done = e->get_new_label("is_done");
	    string *is_false = e->get_new_label("is_false");
	    e->emit_branch(BREZ,factor_reg,is_false);
	    e->emit_move(factor_reg,0);
	    e->emit_branch(is_done);
	    e->emit_label(is_false);
	    e->emit_move(factor_reg,1);
	    e->emit_label(is_done);
	    return true;
	  } else {
	    return false; //parse_factor failed
	  }
   } else if(word->get_token_type() == TOKEN_ADDOP
	    && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD
	    || word->get_token_type() == TOKEN_ADDOP
	    && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB ) {
	    
	    addop_attr_type the_addop = static_cast<AddopToken *>(word)->get_attribute();
	    if(parse_sign()) {
	      
	      if(parse_factor(factor1_type,factor_reg)) {
		 if(factor1_type == INT_T) {
		    factor_type = INT_T;
		  } else {
		    type_error(word);
		    exit (1);
		  }   
		  //emits negative if the sign is -
		  if(the_addop == ADDOP_SUB)
		  {
		    e->emit_1addr(NEG,factor_reg);
		  }
		return true;
	      } else { 
		return false; //parse favor failed
	      }
	    } else {
	      return false; //parse sign failed
	    }
   } else {
	string *expected = new string (" identifier num keyword \"not\" punctuation \"(\" addop \"+\" addop \"-\" ");
	parse_error (expected, word);
	return false;
   }
}
	
bool Parser::parse_sign()
{
   /* SIGN-> ,+ | -
    * Predict (+ | -) = {+,-)
    */ 
   if(word->get_token_type() == TOKEN_ADDOP
	    && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD
	    || word->get_token_type() == TOKEN_ADDOP
	    && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB ) {
	  delete word; 
	  word = lex->next_token();
	  return true;
   } else {
	string *expected = new string (" addop \"+\" addop \"-\" ");
	parse_error (expected, word);
	return false;
   }
}
     
 
