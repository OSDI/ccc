/*
 * compact c compiler
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#define BUFLEN 256

FILE *fp;

enum {
  AST_OP_PLUS,
  AST_OP_MINUS,
  AST_INT,
  AST_STR,
};


typedef struct Ast {
  int type;
  union {
    int ival;
    char *sval;
    struct {
      struct Ast *left;
      struct Ast *right;
    };
  };

} Ast;

void error(char *fmt,...){
  va_list args;
  va_start(args,fmt);
  vfprintf(stderr,fmt,args);
  fprintf(stderr,"\n");
  va_end(args);
  exit(1);
}

void compile_string(){
  char buf[BUFLEN];
  int i=0;

  for(;;){
    int c=getc(stdin);
    if (c==EOF){
      error("Unterminated string");
    }
    if (c =='"'){
      break;//finish string
    }

    buf[i++]=c;
    if(i==BUFLEN-1) error("String too long");
  }

  //generate assembler
  buf[i]='\0';
  printf("\t.data\n"
      ".mydata:\n\t"
      ".string \"%s\"\n\t"
      ".text\n\t"
      ".global stringfn\n"
      "stringfn:\n\t"
      "lea .mydata(%%rip), %%rax\n\t"
      "ret\n",buf);
  exit(0);
}

int read_number(int n){
  int c;
  while ((c=getc(stdin)) != EOF){
    if (!isdigit(c)){
      //数字ではないものが来たら、stdinに戻して数字を返す
      ungetc(c, stdin);
      return n;
    }
    n = n * 10 +(c-'0');
  }

  return 0; //not happen
}

void skip_space(void){
  int c;
  while ((c = getc(stdin)) !=EOF){
    if (isspace(c))
      continue;
    ungetc(c,stdin);
    return;
  }
}

void compile_expr2(){
  for(;;){
    skip_space();
    int c = getc(stdin);
    if (c == EOF){
      printf("ret\n");
      exit(0);
    }

    skip_space();
    char *op;
    if (c=='+') op ="add";
    else if (c == '-') op = "sub";
    else error("Operator expected, but got '%c'", c);
    skip_space();

    c = getc(stdin);
    if (!isdigit(c))
      error("Number expected, but got '%c'", c);
    printf("%s $%d, %%rax\n\t", op, read_number(c - '0'));

  }
   
}

void compile_expr(int n){
  n = read_number(n);
  printf(".text\n\t"
      ".global intfn\n"
      "intfn:\n\t"
      "mov $%d, %%rax\n\t", n);
  compile_expr2();
}

void compile(void){
  int c=getc(stdin);

  if(isdigit(c)){
    return compile_expr(c-'0');
  }
  else{
    return compile_string();
  }


  error("Don't know how to handle '%c'",c);
}

int main(int argc, char **argv){
  fp = fopen( "debug.txt", "w" );

  if (argc > 1 && !strcmp(argv[1], "-a")){
    //print_ast(ast);
  }
  else{
    compile();
  }

  fclose(fp);
  return 0;
}

