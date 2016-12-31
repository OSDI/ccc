#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#define BUFLEN 256


FILE *fp;

void error(char *fmt,...){
  va_list args;
  va_start(args,fmt);
  vfprintf(stderr,fmt,args);
  fprintf(stderr,"\n");
  va_end(args);
  exit(1);
}

/**
 * @brief 
 *
 * @param n
 *
 */
void compile_number(int n){
  int c;
  while ((c=getc(stdin))!=EOF){
    if(isspace(c))
      break;
    if(!isdigit(c))
      error("Invalid character in number:'%c'",c);

    fprintf(fp,"%d\n",c);
    fprintf(fp,"%d\n",n);

    n=n*10+(c-'0');

    fprintf(fp,"%d\n",n);
  }

  printf("\t.text\n\t"
      ".global intfn\n"
      "intfn:\n\t"
      "mov $%d, %%rax\n\t"
      "ret\n",n);
}

void compile_string(){
  char buf[BUFLEN];
  int i=0;

  for(;;){
    int c=getc(stdin);
    fprintf(fp,"%d\n",c);
    if (c==EOF){
      error("Unterminated string");
    }
    if (c =='"'){
      break;//finish string
    }

    buf[i++]=c;
    if(i==BUFLEN-1) error("String too long");
  }

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
      ungetc(c, stdin);
      return n;
    }
    n = n * 10 +(c-'0');
  }
}


void compile_expr(int n){
  n = read_number(n);
}

void Compile(void){
  int c=getc(stdin);

  if(isdigit(c)){
    /* return compile_expr(c-'0'); */
    return compile_number(c-'0');
  }
  else{
    return compile_string();
  }


  error("Don't know how to handle '%c'",c);
}

int main(int argc, char **argv){
  fp = fopen( "debug.txt", "w" );
  Compile();
  fclose(fp);
  return 0;
}

