#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define POTENTIAL -1

#define MAX_SOURCE_CODE_LENGTH 102401 // Max size of file is 100KB
#define MAX_TOKEN_COUNT 102401
#define MAX_TOKEN_LENGTH 100

#define MAX_USER_DEFINED_COUNT 1000 // Assume 1000
#define MAX_USER_DEFINED_NAME_LEN 100 // Assume 100

#define DATATYPES_COUNT 20 // Initial count of data types

char user_defined_variables[MAX_USER_DEFINED_COUNT][MAX_USER_DEFINED_NAME_LEN];
int user_defined_variables_count = 0;
char user_defined_funcs[MAX_USER_DEFINED_COUNT][MAX_USER_DEFINED_NAME_LEN];
int user_defined_funcs_count = 0;

char datatypes[100][20] = {
        "short", "int", "unsigned", "signed", "long", 
        "int8_t", "int16_t", "int32_t", "int64_t",
        "float", "double", "void", "size_t", "char", "bool",
        "struct", "typedef", "union", "enum", "FILE"
};
int datatypes_count = DATATYPES_COUNT;

int isBlank(char* str); // Is 'str' empty?
int isOperator(char c); // Is 'c' an operator?
int isSplit(char c); // Should 'c' be used as split?
int isDataType(char* word); // Is 'word' a data type?
// Return the character at index 'index' in 'word', or the next non-blank character if it's a blank.
char next_char(char* word, int index);

int isAlphabet(char c); // Is 'c' an alphabet character?
int isNum(char* word); // Is 'word' a number (integer or float)?
int isVariable(char *word); // Is 'word' a user-defined variable?
int isFunc(char *word); // Is 'word' a user-defined function?

// Function to preprocess the source code
int preprocessSourceCode(char** token, char* source_code, int source_code_count);

int min(int a, int b); // Returns a smaller value
int max(int a, int b); // Returns a larger value

// Function to find the Longest Common Subsequence (LCS)
int LCS(char** tokens1, int token1_count, char** tokens2, int token2_count);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage:\n %s <source_code_file1> <source_code_file2>\n", argv[0]);
        return 1;
    }

    // Read first source code file
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "File open error (%s)\n", argv[1]);
        return 1;
    }

    char* sourceCode1 = (char *)malloc(sizeof(char) * MAX_SOURCE_CODE_LENGTH);
    int sourceCode1_count = 0;

    char ch;
    while((ch = fgetc(fp)) != EOF) {
        sourceCode1[sourceCode1_count++] = ch;
    }
    sourceCode1[sourceCode1_count] = '\0';
    fclose(fp);
    

    // Read second source code file
    fp = fopen(argv[2], "r");
    if (fp == NULL) {
        fprintf(stderr, "File open error (%s)\n", argv[2]);
        return 1;
    }

    char* sourceCode2 = (char *)malloc(sizeof(char) * MAX_SOURCE_CODE_LENGTH);
    int sourceCode2_count = 0;

    while((ch = fgetc(fp)) != EOF) {
        sourceCode2[sourceCode2_count++] = ch;
    }
    sourceCode2[sourceCode2_count] = '\0';
    fclose(fp);

    /* (1) the token lists for each source code file */
    // Preprocess source code files
    char** token1 = (char **)malloc(sizeof(char *) * MAX_TOKEN_COUNT);
    for(size_t i = 0; i < MAX_TOKEN_COUNT; i++)
        token1[i] = (char *)malloc(sizeof(char) * MAX_TOKEN_LENGTH);

    char** token2 = (char **)malloc(sizeof(char *) * MAX_TOKEN_COUNT);
    for(size_t i = 0; i < MAX_SOURCE_CODE_LENGTH; i++)
        token2[i] = (char *)malloc(sizeof(char) * MAX_TOKEN_LENGTH);

    int token1_count = preprocessSourceCode(token1, sourceCode1, sourceCode1_count);
    int token2_count = preprocessSourceCode(token2, sourceCode2, sourceCode2_count);

    // Write token to the output file
    FILE* outputFile = fopen("hw2_output.txt", "w");

    if (outputFile == NULL) {
        fprintf(stderr, "File open error (%s)\n", "hw2_output.txt");
        return 1;
    }
    
    for (size_t i = 0; i < token1_count; i++) {
        fprintf(outputFile, "%s\n", token1[i]);
    }

    fprintf(outputFile, "*****\n");
    for (size_t i = 0; i < token2_count; i++) {
        fprintf(outputFile, "%s\n", token2[i]);
    }
    fprintf(outputFile, "*****\n");
    fclose(outputFile);


    /* (2) the common tokens found in the longest common subsequence 
        between the token lists of the two source code files */    
    int common_token_count = LCS(token1, token1_count, token2, token2_count);
    

    /*  (3) the plagiarism score, represented as a percentage (rounded to two decimal places)
         into the output file named ‘hw2 output.txt.’ */
    float plagiarism_score = (float)common_token_count / min(token1_count, token2_count) * 100;
    
    // Write plagiarism score to the output file
    outputFile = fopen("hw2_output.txt", "a");

    if (outputFile == NULL) {
        fprintf(stderr, "File open error (%s)\n", "hw2_output.txt");
        return 1;
    }
    fprintf(outputFile, "%.2f", plagiarism_score);
    fclose(outputFile);


    // Memory allocate
    free(sourceCode1);
    free(sourceCode2);
    for(size_t i = 0; i < token1_count; i++)
        free(token1[i]);
    free(token1);
    for(size_t i = 0; i < token2_count; i++)
        free(token2[i]);
    free(token2);

    return 0;
}

// Is 'str' empty?
int isBlank(char *str) {
    size_t length = strlen(str);
    
    for(size_t i = 0; i < length; i++)
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
            return FALSE; // not blank string
    return TRUE; // only blank string
}

// Is 'c' an operator?
int isOperator(char c) {
    char operators[] = "+-*/%%=><!&|^~?:";
    for(size_t i = 0; operators[i] != '\0'; i++)
        if(operators[i] == c)
            return TRUE;
    return FALSE;
}

// Should 'c' be used as split?
int isSplit(char c) {
    char splits[] = " ,;(){}[]\t\n";
    for(size_t i = 0; splits[i] != '\0'; i++)
        if(splits[i] == c)
            return TRUE;
    return FALSE;
}

// Is 'word' a data type?
int isDataType(char* word) {
    for(size_t i = 0; i < datatypes_count; i++)
        if(!strcmp(word, datatypes[i]))
            return i + 1;
    return FALSE;
}

// Return the character at index 'index' in 'word', or the next non-blank character if it's a blank.
char next_char(char* word, int index) {
    if(word[index] == ' ' || word[index] == '\t' || word[index] == '\n')
        return next_char(word, index + 1);
    else
        return word[index];
}

// Is 'c' an alphabet character?
int isAlphabet(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

// Is 'word' a number (integer or float)?
int isNum(char* word) {
    for(size_t i = 0; word[i] != '\0'; i++) {
        if((word[i] >= '0' && word[i] <= '9') || word[i] == '.')
            continue;
        else
            return FALSE;
    }
    return TRUE;
}

// Is 'word' a user-defined variable?
int isVariable(char *word) {
    for(size_t i = 0; i < user_defined_variables_count; i++)
        if(!strcmp(word, user_defined_variables[i]))
            return TRUE;
    return FALSE;
}

// Is 'word' a user-defined function?
int isFunc(char *word) {
    for(size_t i = 0; i < user_defined_funcs_count; i++)
        if(!strcmp(word, user_defined_funcs[i]))
            return TRUE;
    return FALSE;
}

// Function to preprocess the source code
int preprocessSourceCode(char** token, char* source_code, int source_code_count) {
    int token_count = 0;

    // Buffer to store a word
    char* word = (char *)malloc(sizeof(char) * MAX_SOURCE_CODE_LENGTH);
    word[0] = '\0';
    int word_idx = 0;

    // Flags to track various states
    int in_header_file = FALSE; // Inside header file
    int in_define = FALSE;      // Inside macro

    int in_line_comment = FALSE;    // Inside single-line comment
    int in_comment = FALSE;         // Inside multiline comment

    int is_char = FALSE;    // Inside character literal (' ')
    int is_string = FALSE;  // Inside string literal (" ")

    int dot_is_operator = FALSE;    // Dot is an operator

    int after_data_type = FALSE;    // After a data type declaration

    int typeDef_data_type = FALSE;  // After a typedef data type declaration
    int in_typeDef = FALSE;         // Inside typedef block

    int enum_data_type = FALSE;     // Inside an enum data type declaration
    int in_enum = FALSE;            // Inside enum block

    for(size_t i = 0; i < source_code_count; i++) {
        // Remove comments (//)
        if(!in_line_comment && source_code[i] == '/' && source_code[i + 1] == '/') {
            in_line_comment = TRUE;
            continue;
        }
        else if(in_line_comment && source_code[i] == '\n') {
            in_line_comment = FALSE;
            continue;
        }
        else if(in_line_comment) {
            continue;
        }

        // Remove coments (/* */)
        if(!in_comment && source_code[i] == '/' && source_code[i + 1] == '*') {
            in_comment = TRUE;
            continue;
        }
        else if(in_comment && source_code[i - 1] == '*' && source_code[i] == '/') {
            in_comment = FALSE;
            continue;
        }
        else if(in_comment) {
            continue;
        }

        /* If the current character is a delimiter that needs to be split or an operator
         * (including the '.' operator) and it is not within a character literal,
         * string literal, or header file, tokenize the word read so far. */
        if((isSplit(source_code[i]) || isOperator(source_code[i]) || (source_code[i] == '.'
                 && dot_is_operator)) && !is_char && !is_string && !in_header_file) {

            if(typeDef_data_type && source_code[i] == '{')
                in_typeDef++;
            else if(typeDef_data_type && source_code[i] == '}')
                in_typeDef--;

            if(enum_data_type && source_code[i] == '{')
                in_enum++;
            else if(enum_data_type && source_code[i] == '}') {
                in_enum--;
                if(!in_enum)
                    // End of enum block
                    enum_data_type = FALSE;
            }
            
            if(isBlank(word)) { 
                // Just whitespace character case
                ;
            }
            else if(in_enum) {
                // Inside enum, treat word as variable
                strcpy(token[token_count++], "VAR");
                strcpy(user_defined_variables[user_defined_variables_count++], word);
            }
            else if(isNum(word)) { 
                // Numeric literal
                strcpy(token[token_count++], "NUM_LITERAL");
            }
            else if(isDataType(word)) {
                // Word is a data type

                // default data type case
                if(isDataType(word) <= DATATYPES_COUNT)
                    strcpy(token[token_count++], word);
                // user-defined type case
                else
                    strcpy(token[token_count++], "VAR");
                after_data_type = TRUE;

                // word == typedef case
                if(!strcmp(word, "typedef")) {
                    typeDef_data_type = TRUE;
                }
                // word == enum case
                else if(!strcmp(word, "enum")) {
                    enum_data_type = TRUE;
                }
            }
            else if(isVariable(word)) {
                // Word is a variable
                strcpy(token[token_count++], "VAR");
            }
            else if(isFunc(word)) {
                // Word is a function
                strcpy(token[token_count++], "FUNC");
            }
            else if((after_data_type == TRUE || in_define) && next_char(source_code, i) != '(') {
                // Word is a variable (new VAR -> add to user_defined_variables)
                strcpy(token[token_count++], "VAR");
                strcpy(user_defined_variables[user_defined_variables_count++], word);
            }
            else if(after_data_type == TRUE || in_define) {
                // Word is a function (new FUNC -> add to user_defined_funcs)
                strcpy(token[token_count++], "FUNC");
                strcpy(user_defined_funcs[user_defined_funcs_count++], word);
            }
            else if(typeDef_data_type && !in_typeDef) {
                // Word is a variable (typedef new datatype -> add to datatypes)
                strcpy(token[token_count++], "VAR");
                strcpy(datatypes[datatypes_count++], word);
            }
            else {
                // Word is a generic token
                strcpy(token[token_count++], word);
            }

            if(isOperator(source_code[i])) {
                if(source_code[i] == '-' && source_code[i + 1] == '>') {
                    // Special case: '->' operator
                    token[token_count][0] = source_code[i];
                    token[token_count][1] = source_code[i + 1];
                    token[token_count++][2] = '\0';
                    i++;
                }
                else {
                    // Oter operators
                    token[token_count][0] = source_code[i];
                    token[token_count++][1] = '\0';
                }
            }
            else if(source_code[i] == '.' && dot_is_operator) {
                // Dot operator
                token[token_count][0] = source_code[i];
                token[token_count++][1] = '\0';
            }

            if(in_define && source_code[i] == '\n')
                in_define = FALSE;
                
            /* Case 1: If a data type has been declared earlier and it is followed by a ';' or '{' 
                       or ')', it means that no new VAR or FUNC is declared after that point.*/
            if((source_code[i] == ';' || source_code[i] == '{'
                    || source_code[i] == ')') && after_data_type == TRUE)
                after_data_type = FALSE;
            /* Case 2: If a data type has been declared before and '=' is encountered, 
                       the subsequent tokens are not new VAR or FUNC. 
                       However, it is possible for new VAR or FUNC to be declared later, 
                       so it is marked as POTENTIAL. */
            else if(source_code[i] == '=' && after_data_type == TRUE)
                after_data_type = POTENTIAL;
            /* Case 3: If a data type has been declared earlier and it is in the POTENTIAL state, 
                       when a ',' is encountered, it means that new VAR or FUNC can occur again 
                       after that point.*/
            else if(source_code[i] == ',' && after_data_type == POTENTIAL)
                after_data_type = TRUE;

            if(source_code[i] == ';' && !in_typeDef)
                // End of a typedef block
                typeDef_data_type = FALSE;
    
            dot_is_operator = FALSE;

            // Initializing the word
            word_idx = 0;
            strcpy(word, "\0");
        } // end if

        else {
            if(source_code[i] == ' ' || source_code[i] == '\t' || source_code[i] == '\n')
                continue;

            if(!is_string && !strncmp(source_code + i, "#include", 8)) {
                // Preprocessor directive: #include
                strcpy(token[token_count++], "#include");

                i += 7;
                in_header_file = TRUE;
                continue;
            }
            else if(!is_string && !strncmp(source_code + i, "#define", 7)) {
                // Preprocessor directive: #define                
                strcpy(token[token_count++], "#define");

                i += 6;
                in_define = TRUE;
                continue;
            }

            if(isAlphabet(source_code[i]))
                // If there is at least one alphabet character preceding it, 
                // the subsequent occurrence of '.' is considered as an operator.
                dot_is_operator = TRUE;

            // The charactor is stored in the string 'word'
            word[word_idx++] = source_code[i];
            word[word_idx] = '\0';

            if(source_code[i] == '\'' && source_code[i - 1] != '\\' && !is_string) {
                if(is_char) {
                    // End of character literal
                    strcpy(token[token_count++], "STR_LITERAL");
                    word_idx = 0;
                    strcpy(word, "\0");
                    is_char = FALSE;
                }
                else
                    // Start of character literal
                    is_char = TRUE;
            }
            if(source_code[i] == '\"' && source_code[i - 1] != '\\' && !is_char) {
                if(is_string) {
                    // End of string literal
                    strcpy(token[token_count++], "STR_LITERAL");
                    word_idx = 0;
                    strcpy(word, "\0");
                    is_string = FALSE;
                }
                else
                    // Start of string literal
                    is_string = TRUE;
            }

            if(in_header_file && source_code[i] == '>') {
                // End of header file name
                strcpy(token[token_count++], word);
                word_idx = 0;
                strcpy(word, "\0");
                in_header_file = FALSE;
            }

        }
    }

    free(word);
    user_defined_variables_count = 0;
    user_defined_funcs_count = 0;
    return token_count;
}

// Returns a smaller value
int min(int a, int b) {
    return (a < b) ? a : b;
}

// Returns a larger value
int max(int a, int b) {
    return (a > b) ? a : b;
}

// Function to find the Longest Common Subsequence (LCS)
int LCS(char** tokens1, int token1_count, char** tokens2, int token2_count) {
    /* 1. Use dynamic programming to calculate the LCS length */
    // Create a dynamic programming table
    int** dp = (int **)malloc(sizeof(int *) * (token1_count + 1));
    for(size_t i = 0; i <= token1_count; i++)
        dp[i] = (int *)malloc(sizeof(int) * (token2_count + 1));

    for(size_t i = 0; i <= token1_count; i++) {
        for(size_t j = 0; j <= token2_count; j++) {
            // Initialize the dynamic programming table
            if(i == 0 || j == 0)
                dp[i][j] = 0;

            // Using a recurrence relation to solve this problem
            else if(!strcmp(tokens1[i - 1], tokens2[j - 1]))
                dp[i][j] = dp[i - 1][j - 1] + 1;
            else
                dp[i][j] = max(dp[i][j - 1], dp[i - 1][j]);
        }
    }

    /* 2. Use the LCS algorithm to find common tokens */
    // Create an array to store common tokens
    char** common_tokens = (char **)malloc(sizeof(char *) * MAX_TOKEN_COUNT);
    for(size_t i = 0; i < MAX_TOKEN_COUNT; i++)
        common_tokens[i] = (char *)malloc(sizeof(char) * MAX_TOKEN_LENGTH);
    int common_token_count = 0;
    
    // Backtrack through the dynamic programming table to find common tokens
    for(size_t i = token1_count, j = token2_count; i > 0 && j > 0;) {
        if(!strcmp(tokens1[i - 1], tokens2[j - 1])) {
            strcpy(common_tokens[common_token_count++], tokens1[i - 1]);
            i--;
            j--;
        }
        else if(dp[i - 1][j] >= dp[i][j - 1])
            i--;
        else
            j--;
    }
     
    /* 3. Write common token to output file */
    // Open the output file in append mode
    FILE* outputFile = fopen("hw2_output.txt", "a");
    if (outputFile == NULL) {
        fprintf(stderr, "File open error (%s)\n", "hw2_output.txt");
        return 1;
    }

    // Write common tokens to the output file
    for(int i = common_token_count - 1; i >= 0; i--)
        fprintf(outputFile, "%s\n", common_tokens[i]);
    fprintf(outputFile, "*****\n");

    fclose(outputFile);

    // Return the count of common tokens
    return common_token_count;
}