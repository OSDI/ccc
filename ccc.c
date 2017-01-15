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
  while ((c = getc(stdin)) != EOF){
    if (isspace(c))
      continue;
    ungetc(c,stdin);
    return;
  }
}

int get_priority(char c){

  if(c == '+'){
    return 2;
  }
  else if(c == '-'){
    return 2;
  }
  else if(c == '*'){
    return 3;
  }
  else if(c == '/'){
    return 3;
  }
  else{
    return -1;
  }
  
  return 0;
}

Ast *read_expr2(int prec){
    skip_space();
    Ast *ast = read_prim();
    if (!ast) return NULL;

    for(;;){
      skip_space();
      int c = getc(stdin);
      if (c == EOF){
        return ast;
      }
      int prec2=get_priority(c);
      if (prec2 < prec){
        ungetc(c, stdin);
        return ast;
      }

      skip_space();
      ast = make_ast_op(c, ast, read_expr2(prec2+1));
    }

    return ast;
}

Ast *read_prim(void){
  int c=getc(stdin);
  if(isdigit(c)){
    return read_number(c-'0');
  }
  else if (c == '"'){
    return read_string();
  }
  else if (c==EOF){
    return NULL;
  }
  error("Don't know how to handle '%c'",c);
  return 0;
}

void ensure_intexpr(Ast *ast) {
  if (ast->type == '+') return;
  else if (ast->type == '-') return;
  else if (ast->type == '*') return;
  else if (ast->type == '/') return;
  else if (ast->type == AST_INT) return;
  else error("integer or binary operator expected");
}

void emit_intexpr(Ast *ast){
  ensure_intexpr(ast);
  if (ast->type == AST_INT){
    printf("mov $%d, %%eax\n\t", ast->ival);
  }
  else{
    emit_binop(ast);
  }
}

void emit_binop(Ast *ast){
  char *op;
  if(ast->type == '+'){
    op= "add";
  }
  else if(ast->type== '-'){
    op= "sub";
  }
  else if(ast->type== '*'){
    op= "imul";
  }
  else if(ast->type == '/'){
    //do nothing
  }
  else{
    error("invalid operand");
  }

  emit_intexpr(ast->left);
  printf("push %%rax\n\t");
  emit_intexpr(ast->right);

  if (ast->type == '/'){
    printf("mov %%eax, %%ebx\n\t");
    printf("pop %%rax\n\t");
    printf("mov $0, %%edx\n\t");
    printf("idiv %%ebx\n\t");
  }
  else{
    printf("pop %%rbx\n\t");
    printf("%s %%ebx, %%eax\n\t", op);
  }
}

Ast *read_expr(void){
  Ast *r = read_expr2(0);
  if(!r) return NULL;
  skip_space();
  int c = getc(stdin);
  if (c != ';'){
    error("Unterminated experssion");
  }
  return r;
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
    case AST_INT:
      printf("%d", ast->ival);
      break;
    case AST_STR:
      print_quote(ast->sval);
      break;
    default:
      printf("(%c ", ast->type);
      print_ast(ast->left);
      printf(" ");
      print_ast(ast->right);
      printf(")");
  }
}


int main(int argc, char **argv){
  fp = fopen( "debug.txt", "w" );
  int want_ast=(argc > 1 && !strcmp(argv[1], "-a"));
  if(!want_ast){
    printf(".text\n\t"
      ".global mymain\n"
      "mymain:\n\t");
  }
  /* for(;;){ */
    Ast *ast = read_expr();
    /* if(!ast) break; */
    if(want_ast){
      print_ast(ast);
    }
    else{
      compile(ast);
    }
  /* } */
  if(!want_ast){
    printf("ret\n");
  }

  fclose(fp);
  return 0;
}

