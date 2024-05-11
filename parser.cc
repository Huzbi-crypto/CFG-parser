/**
 * @author Abhisekhar Bharadwaj Gandvarapu
 * StudentID: 1219773724
 */


#include <iostream>
#include <unordered_map>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#include "lexer.h"

LexicalAnalyzer lexer;
std::unordered_map<std::string, int> map;

void syntax_error();
Token ExpectedToken(TokenType token_type);

//3 sections of a program
//var section
void parseVarSection();
void parseIdList();

//body section
InstructionNode* parseBody();
InstructionNode* parseStatementList();
InstructionNode* parseStatement();
InstructionNode* parseAssignStatement();
void parseExpression(InstructionNode* instruction);
Token parsePrimary();
Token parseOperator();
InstructionNode* parseOutputStatement();
InstructionNode* parseInputStatement();
InstructionNode* parseWhileStatement();
InstructionNode* parseIfStatement();
void parseCondition(InstructionNode* instruction);
Token parseRelOp();
InstructionNode* parseSwitchStatement();
InstructionNode* parseForStatement();
InstructionNode* parseCaseList(std::string var_name, InstructionNode* noop);
InstructionNode* parseCase(std::string var_name, InstructionNode* noop);
InstructionNode* parseDefaultCase();

//inputs section
void parseInputs();
void parseNumberList();

//-------------------------------------------------------------------------------------------

void syntax_error(){
    std::cout << "Error: Wrong Syntax!\n";
    exit(1);
}

// Expects the next token from the lexer to be of the specified token type
Token ExpectedToken(TokenType expectedTokenType) {
    // Get the next token from the lexer
    Token token = lexer.GetToken();

    // Check if the token is of the expected token type
    if (token.token_type != expectedTokenType) {
        // If the token is not of the expected token type, output a Syntaxx Error message and exit
        syntax_error();
    }

    // Return the token
    return token;
}

// Parses the input source code and generates an intermediate representation 
// in the form of an InstructionNode linked list named Instructions
struct InstructionNode * parse_generate_intermediate_representation()
{
    // Parse the variable section of the source code
    parseVarSection();

    // Parse the body of the source code and store the head of the InstructionNode linked list
    InstructionNode* Instructions = parseBody();

    // Parse the inputs section of the source code
    parseInputs();

    // Return the head of the InstructionNode linked list
    return Instructions;
}

//------------------------------------------------------------------------------------------------------------------------------------------------
//Variable Section:

// Parses the variable section of the source code
void parseVarSection() {
    // Parse the list of identifiers in the variable section
    parseIdList();

    // Expect a semicolon at the end of the variable section
    ExpectedToken(SEMICOLON);
}


// Parses a comma-separated list of identifiers and adds them to the address map
void parseIdList() {
    // Get the next token, which should be an identifier
    Token newVariable = ExpectedToken(ID);

    // Get the name of the new variable from the token
    std::string variableName = newVariable.lexeme;

    // Assign the variable a memory location and add it to the address map
    mem[next_available] = 0;
    map[variableName] = next_available;
    next_available++;

    // Peek ahead to the next token
    Token next_token = lexer.peek(1);

    if (next_token.token_type == COMMA) {
        // If the next token is a comma, expect it and recursively parse the rest of the identifier list
        ExpectedToken(COMMA);
        parseIdList();
    } else {
        // Otherwise, the end of the identifier list has been reached
        return;
    }
}


//------------------------------------------------------------------------------------------------------------------------------------------


/*Body Section:
  this section of the code deals with body
  of the program where all the instructions
  are performed.*/

// Parses the body of the source code and generates an InstructionNode linked list
InstructionNode* parseBody() {
    // Declare a pointer to an InstructionNode
    InstructionNode* instructionList;

    // Expect an opening brace to begin the body of the source code
    ExpectedToken(LBRACE);

    // Parse the list of statements in the body of the source code and store the head of the InstructionNode linked list
    instructionList = parseStatementList();

    // Expect a closing brace to end the body of the source code
    ExpectedToken(RBRACE);

    // Return the head of the InstructionNode linked list
    return instructionList;
}


// Parses a list of statements and generates an InstructionNode linked list
InstructionNode* parseStatementList() {

    // Parse the first statement in the list
    InstructionNode* instructionList = parseStatement();

    // Check the next token
    Token next_token = lexer.peek(1);
    if (next_token.token_type == RBRACE) {
        // If the next token is a closing brace, the end of the statement list has been reached
        return instructionList;
    } else {
        // Otherwise, recursively parse the rest of the statement list and append it to the first statement list
        InstructionNode* nextInstructionList = parseStatementList();

        // Append instruction_list2 to the end of instruction_list1
        InstructionNode* temp = instructionList;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = nextInstructionList;

        // Return the head of the combined instruction list
        return instructionList;
    }
}

// Parses a single statement and returns an InstructionNode representing that statement
InstructionNode* parseStatement() {
    // Peek at the next token in the input stream
    Token next_token = lexer.peek(1);

    // Determine the type of statement and call the corresponding parsing function
    if (next_token.token_type == ID) {
        // Assignment statement
        return parseAssignStatement();
    } else if (next_token.token_type == WHILE) {
        // While loop statement
        return parseWhileStatement();
    } else if (next_token.token_type == IF) {
        // If statement
        return parseIfStatement();
    } else if (next_token.token_type == SWITCH) {
        // Switch statement
        return parseSwitchStatement();
    } else if (next_token.token_type == FOR) {
        // For loop statement
        return parseForStatement();
    } else if (next_token.token_type == OUTPUT) {
        // Output statement
        return parseOutputStatement();
    } else if (next_token.token_type == INPUT) {
        // Input statement
        return parseInputStatement();
    } else {
        // Invalid statement
        syntax_error();
    }

}



// Parses an assignment statement and returns an InstructionNode representing that statement
InstructionNode* parseAssignStatement() {
    // Expect an ID token on the left-hand side of the assignment
    Token leftSide = ExpectedToken(ID);

    // Create a new InstructionNode of tpye ASSIGN for the assignment statement
    InstructionNode* instruction = new InstructionNode;
    instruction->type = ASSIGN;

    // Map the variable name to its address in memory and store it in the InstructionNode
    instruction->assign_inst.left_hand_side_index = map[leftSide.lexeme];

    // Expect an equal sign
    ExpectedToken(EQUAL);

    if (lexer.peek(2).token_type == SEMICOLON) {
        // If the assignment has no operator or second operand, create an InstructionNode with none operator
        instruction->assign_inst.op = OPERATOR_NONE;

        // Parse the primary expression on the right-hand side of the assignment and map it to its address in memory
        instruction->assign_inst.operand1_index = map[parsePrimary().lexeme];
    } else {
        // Parse the full expression on the right-hand side of the assignment and store it in the InstructionNode
        parseExpression(instruction);
    }

    // Expect a semicolon at the end of the assignment statement
    ExpectedToken(SEMICOLON);

    // Set the next pointer of the InstructionNode to null and return it
    instruction->next = nullptr;
    return instruction;
}


void parseExpression(InstructionNode* instruction) {

    // set the first operand of the assignment instruction to the address of the primary token
    instruction->assign_inst.operand1_index = map[parsePrimary().lexeme];

    switch (parseOperator().token_type)
    {
        case PLUS:
            instruction->assign_inst.op = OPERATOR_PLUS;
            break;
        case MINUS:
            instruction->assign_inst.op = OPERATOR_MINUS;
            break;
        case MULT:
            instruction->assign_inst.op = OPERATOR_MULT;
            break;
        case DIV:
            instruction->assign_inst.op = OPERATOR_DIV;
            break;
    }

    // set the second operand of the assignment instruction to the address of the primary token
    instruction->assign_inst.operand2_index = map[parsePrimary().lexeme];
}


Token parsePrimary() {

    if (lexer.peek(1).token_type == NUM) {
        // If the token is a number, add it to memory if it's not already there
        if (!map.count(lexer.peek(1).lexeme)) {
            mem[next_available] = std::stoi(lexer.peek(1).lexeme);
            map[lexer.peek(1).lexeme] = next_available;
            next_available++;
        }
        return ExpectedToken(NUM);
    } else {
        // If the token is an identifier, return it
        return ExpectedToken(ID);
    }
}


Token parseOperator() {
    // Check the type of the next token in the input
    if (lexer.peek(1).token_type == PLUS) {
        return ExpectedToken(PLUS);
    }
    else if (lexer.peek(1).token_type == MINUS) {
        return ExpectedToken(MINUS);
    }   
    else if (lexer.peek(1).token_type == MULT) {
        return ExpectedToken(MULT);
    }
    else if (lexer.peek(1).token_type == DIV) {
        return ExpectedToken(DIV);
    }
    else {
        syntax_error();
    }

}


InstructionNode* parseOutputStatement() {
    // Expect the 'output' keyword
    ExpectedToken(OUTPUT);

    // Create a new instruction node of type OUT
    InstructionNode* instruction = new InstructionNode;
    instruction->type = OUT;

    // Expect an ID token, which is the variable to be outputted
    Token variable = ExpectedToken(ID);
    
    // Set the variable index of the output instruction to the index of the variable in the address map
    instruction->output_inst.var_index = map[variable.lexeme];

    // Expect a semicolon
    ExpectedToken(SEMICOLON);

    // Set the next pointer of the instruction node to null
    instruction->next = nullptr;

    // Return the instruction node
    return instruction;
}


InstructionNode* parseInputStatement() {
    ExpectedToken(INPUT);

    InstructionNode* instruction = new InstructionNode;
    instruction->type = IN;

    // Expect the name of the variable that will hold the input value
    Token variable = ExpectedToken(ID);

    // Store the index of the variable in the instruction
    instruction->input_inst.var_index = map[variable.lexeme];
    instruction->next = nullptr;

    // Expect a semicolon to end the statement
    ExpectedToken(SEMICOLON);

    return instruction;
}


InstructionNode* parseWhileStatement() {
    ExpectedToken(WHILE);

    // Create a new instruction node of type CJMP (conditional jump)
    InstructionNode* i1 = new InstructionNode;
    i1->type = CJMP;

    // Parse the condition of the while statement
    parseCondition(i1);

    // Parse the body of the while statement
    InstructionNode* body = parseBody();
    i1->next = body;

    // Create a new instruction node of type JMP (unconditional jump)
    InstructionNode* i2 = new InstructionNode;
    i2->type = JMP;
    i2->jmp_inst.target = i1;

    // Append the JMP instruction to the end of the body of the while statement
    while (body->next != nullptr) {
        body = body->next;
    }
    body->next = i2;

    // Create a new instruction node of type NOOP (no operation)
    InstructionNode* i3 = new InstructionNode;
    i3->type = NOOP;
    i3->next = nullptr;

    // Append the NOOP instruction to the end of the JMP instruction
    i2->next = i3;

    // Set the target of the CJMP instruction to the NOOP instruction
    i1->cjmp_inst.target = i3;

    return i1;
}


InstructionNode* parseIfStatement() {
    ExpectedToken(IF);

    // create a new instruction node for the conditional jump
    InstructionNode* i1 = new InstructionNode;
    i1->type = CJMP;

    // parse the condition and set the target of the conditional jump
    parseCondition(i1);
    InstructionNode* i2 = parseBody();
    i1->next = i2;

    // create a new instruction node for the NOOP operation
    InstructionNode* i3 = new InstructionNode;
    i3->type = NOOP;
    i3->next = nullptr;

    // append the NOOP instruction to the end of the body
    while (i2->next != nullptr) {
        i2 = i2->next;
    }
    i2->next = i3;

    // set the target of the conditional jump to the NOOP instruction
    i1->cjmp_inst.target = i3;

    return i1;
}


void parseCondition(InstructionNode* instruction) {
    // Parse the first primary operand and store its address in the instruction node
    instruction->cjmp_inst.operand1_index = map[parsePrimary().lexeme];

    // Parse the relational operator and store its type in the instruction node
    switch (parseRelOp().token_type) {
        case GREATER:
            instruction->cjmp_inst.condition_op = CONDITION_GREATER;
            break;
        case LESS:
            instruction->cjmp_inst.condition_op = CONDITION_LESS;
            break;
        case NOTEQUAL:
            instruction->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
            break;
    }

    // Parse the second primary operand and store its address in the instruction node
    instruction->cjmp_inst.operand2_index = map[parsePrimary().lexeme];
}

Token parseRelOp() {
    // Check the next token in the input and return the corresponding token if it matches one of the three relational operators
    if (lexer.peek(1).token_type == GREATER) {
        return ExpectedToken(GREATER);
    } else if (lexer.peek(1).token_type == LESS) {
        return ExpectedToken(LESS);
    } else if (lexer.peek(1).token_type == NOTEQUAL) {
        return ExpectedToken(NOTEQUAL);
    } else {
    // If the next token is not a relational operator, output an error message and exit the program
        syntax_error();
    }

}


// Parse a switch statement
InstructionNode* parseSwitchStatement() {
    // Expect the 'switch' keyword
    ExpectedToken(SWITCH);

    // Expect the identifier for the switch variable
    Token switch_var = ExpectedToken(ID);

    // Expect an opening curly brace
    ExpectedToken(LBRACE);

    // Create a NOOP instruction to mark the end of the switch statement
    InstructionNode* i1 = new InstructionNode;
    i1->type = NOOP;
    i1->next = nullptr;

    // Parse the case list
    InstructionNode* i2 = parseCaseList(switch_var.lexeme, i1);

    // Check if there is a default case
    if (lexer.peek(1).token_type == DEFAULT) {
        // Parse the default case and append it to the case list
        InstructionNode* i3 = parseDefaultCase();
        InstructionNode* temp = i2;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = i3;
        temp = i3;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = i1;
    } else {
        // Append the ending NOOP to the end of the case list
        InstructionNode* temp = i2;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = i1;
    }

    // Expect a closing curly brace
    ExpectedToken(RBRACE);

    // Return the case list
    return i2;
}

// Parses a "for" statement and returns the instruction node for its initialization assignment
InstructionNode* parseForStatement() {
    // Expect "for" keyword and left parenthesis
    ExpectedToken(FOR);
    ExpectedToken(LPAREN);

    // Parse initialization assignment, condition, and final assignment
    InstructionNode* i1 = parseAssignStatement();
    InstructionNode* i2 = new InstructionNode;
    i2->type = CJMP;
    parseCondition(i2);
    ExpectedToken(SEMICOLON);
    InstructionNode* i3 = parseAssignStatement();
    ExpectedToken(RPAREN);

    // Parse body and connect the instruction nodes
    InstructionNode* i4 = parseBody();
    i1->next = i2;
    i2->next = i4;

    // Create a jump instruction node and append it at the end of the body
    InstructionNode* i5 = new InstructionNode;
    i5->type = JMP;
    i5->jmp_inst.target = i2;
    while (i4->next != nullptr) {
        i4 = i4->next;
    }
    i4->next = i3;
    i3->next = i5;

    // Create a noop instruction node and connect the jump and condition instruction nodes to it
    InstructionNode* i6 = new InstructionNode;
    i6->type = NOOP;
    i6->next = nullptr;
    i5->next = i6;
    i2->cjmp_inst.target = i6;

    // Return the initialization assignment instruction node
    return i1;
}


// Parse a list of case statements for a switch statement
InstructionNode* parseCaseList(std::string switch_var, InstructionNode* noop) {

    // Parse the first case in the list
    InstructionNode* i1 = parseCase(switch_var, noop);

    // Check if there are more cases in the list
    Token next_token = lexer.peek(1);
    if (next_token.token_type == RBRACE || next_token.token_type == DEFAULT) {
        // There are no more cases in the list, so return the first case instruction
        return i1;
    } else {
        // Parse the rest of the case statements in the list
        InstructionNode* i2 = parseCaseList(switch_var, noop);

        // Append case_inst_2 to the end of case_inst_1
        InstructionNode* temp = i1;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = i2;

        // Return the linked list of case instructions
        return i1;
    }
}

InstructionNode* parseCase(std::string var_name, InstructionNode* noop) {
    ExpectedToken(CASE);

    Token token = ExpectedToken(NUM);

    // Add number to memory if not already present
    if (!map.count(token.lexeme)) {
        mem[next_available] = std::stoi(token.lexeme);
        map[token.lexeme] = next_available;
        next_available++;
    }

    // Create a new instruction node for the case statement
    InstructionNode* i1 = new InstructionNode;
    i1->type = CJMP;
    i1->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    i1->cjmp_inst.operand1_index = map[var_name];
    i1->cjmp_inst.operand2_index = map[token.lexeme];
    // if not equal, we will go to the next case or instruction
    i1->next = nullptr;

    ExpectedToken(COLON);

    // Parse the body of the case statement
    InstructionNode* i2 = parseBody();

    // if equal, we want to go to the body
    i1->cjmp_inst.target = i2;

    // Create a new instruction node for the jump after the body
    InstructionNode* i3 = new InstructionNode;
    i3->type = JMP;
    
    // Append jmp to end of body
    while (i2->next != nullptr) {
        i2 = i2->next;
    }
    i2->next = i3;

    // Point the jmp to the ending noop
    i3->jmp_inst.target = noop;
    i3->next = nullptr;

    return i1;
}



// Parse the default case of a switch statement
InstructionNode* parseDefaultCase() {
    // Expect the DEFAULT keyword
    ExpectedToken(DEFAULT);
    // Expect the COLON symbol
    ExpectedToken(COLON);

    // Parse the body of the default case
    return parseBody();
}

//-----------------------------------------------------------------------------------------------------------------------------------------


/* Input section:
this section of the code deals with parsing the inputs*/
void parseInputs() {
    // Call parseNumberList() to parse comma-separated list of numbers
    parseNumberList();
}


void parseNumberList() {

    // add the number to the list of inputs
    inputs.push_back(std::stoi(ExpectedToken(NUM).lexeme));

    // if there are more numbers, recursively call this function
    if (lexer.peek(1).token_type == NUM) {
        parseNumberList();
    } else {
        // if there are no more numbers, return
        return;
    }
}
