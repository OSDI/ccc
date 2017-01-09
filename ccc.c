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

Ast *read_prim(void);
void emit_binop(Ast *ast);

void error(char *fmt,...){
  va_list args;
  va_start(args,fmt);
  vfprintf(stderr,fmt,args);
  fprintf(stderr,"\n");
  va_end(args);
  exit(1);
}

Ast *make_ast_str(char *str){
  Ast *r = malloc(sizeof(Ast));
  r->type=AST_STR;
  r->sval=str;
  return r;
}

Ast *read_string(){
  char *buf=malloc(BUFLEN);
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
  return make_ast_str(buf);
}

Ast *make_ast_op(int type, Ast *left, Ast *right){
  Ast *r = malloc(sizeof(Ast));
  r->type=type;
  r->left=left;
  r->right=right;
  return r;
}

Ast *make_ast_int(int val){
  Ast *r = malloc(sizeof(Ast));
  r->type=AST_INT;
  r->ival=val;
  return r;
}

Ast *read_number(int n){
  for (;;){
    int c = getc(stdin);
    if (!isdigit(c)){
      //数字ではないものが来たら、stdinに戻して数字を返す
      ungetc(c, stdin);
      return make_ast_int(n);
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

Ast *read_expr2(Ast *left){
    skip_space();
    int c = getc(stdin);
    if (c == EOF){
      return left;
    }
    int op;
    if (c == '+') op =AST_OP_PLUS;
    else if (c=='-') op =AST_OP_MINUS;
    else error("Operator expected, but got '%c'", c);
    skip_space();
    Ast *right = read_prim();
    return read_expr2(make_ast_op(op, left, right));
}

Ast *read_prim(void){
  int c=getc(stdin);
  if(isdigit(c)){
    return read_number(c-'0');
  }
  else if (c == '"'){
    return read_string();
  }
  error("Don't know how to handle '%c'",c);
  return 0;
}

void ensure_intexpr(Ast *ast) {
  if (ast->type != AST_OP_PLUS &&
      ast->type != AST_OP_MINUS &&
      ast->type != AST_INT){
    error("integer or binary operator expected");
  }
}

void emit_intexpr(Ast *ast){
  ensure_intexpr(ast);
  if (ast->type == AST_INT){
    printf("mov $%d, %%eax\n\t", ast->ival);
  }
  else if(ast->type == AST_OP_MINUS ||
          ast->type == AST_OP_PLUS){
    emit_binop(ast);
  }
  else{
    error("unknown ast type %d", ast->type);
  }
}

void emit_binop(Ast *ast){
  char *op;
  if(ast->type == AST_OP_PLUS){
    op= "add";
  }
  else if(ast->type==AST_OP_MINUS){
    op= "sub";
  }
  else{
    error("invalid operand");
  }
  emit_intexpr(ast->left);
  printf("mov %%eax, %%ebx\n\t");
  emit_intexpr(ast->right);
  printf("%s %%ebx, %%eax\n\t",op);
}

Ast *read_expr(void){
  Ast *left = read_prim();
  return read_expr2(left);
}

void print_quote(char *p){
  while(*p){
    if (*p=='\"' || *p == '\\')
      printf("\\");
    printf("%c",*p);
    p++;
  }
}

void emit_string(Ast *ast) {
  printf("\t.data\n"
        ".mydata:\n\t"
        ".string \"");
  print_quote(ast->sval);
  printf("\"\n\t"
      ".text\n\t"
      ".global stringfn\n"
      "stringfn:\n\t"
      "lea .mydata(%%rip), %%rax\n\t"
      "ret\n"
      );
  return;
}

void compile(Ast *ast){
  if (ast->type == AST_STR){
    emit_string(ast);
  }
  else{
    printf(".text\n\t"
        ".global intfn\n"
        "intfn:\n\t");
    emit_intexpr(ast);
    printf("ret\n");
  }
}

void print_ast(Ast *ast){
  switch(ast->type){
    case AST_OP_PLUS:
      printf("(+ ");
      print_ast(ast->left);
      printf(" ");
      print_ast(ast->right);
      printf(")");
      break;
    case AST_OP_MINUS:
      printf("(- ");
      print_ast(ast->left);
      printf(" ");
      print_ast(ast->right);
      printf(")");
      break;
    case AST_INT:
      printf("%d", ast->ival);
      break;
    case AST_STR:
      print_quote(ast->sval);
      break;
    default:
      error("shold not reach hear");
  }
}


int main(int argc, char **argv){
  fp = fopen( "debug.txt", "w" );

  Ast *ast = read_expr();

  if (argc > 1 && !strcmp(argv[1], "-a")){
    print_ast(ast);
  }
  else{
    compile(ast);
  }

  fclose(fp);
  return 0;
}

