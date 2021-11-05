
#include "types.h"
#include "user.h"
#include "fcntl.h"

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int strncmp(const char *, const char *, uint);
int runpipe(char **,int);
int runparallel(char **,int);
int runor(char **,int);
int runand(char **,int);
int runinput(char **,int);
int runoutput(char **,int);
int runexecom(char **);

int seekpipe(char ** tokens){
  for(int i=0;tokens[i]!=0;i++){
    if(strncmp(tokens[i],"|",1)==0){
      return i;
    }
  }
  return 0;
}
int seeksemi(char ** tokens){
  for(int i=0;tokens[i]!=0;i++){
    if(strncmp(tokens[i],";",1)==0){
      return i;
    }
  }
  return 0;
}
int seekinput(char ** tokens){
  for(int i=0;tokens[i]!=0;i++){
    if(strncmp(tokens[i],"<",1)==0){
      return i;
    }
  }
  return 0;
}
int seekoutput(char ** tokens){
  for(int i=0;tokens[i]!=0;i++){
    if(strncmp(tokens[i],">",1)==0){
      return i;
    }
  }
  return 0;
}

int seekand(char ** tokens){
  for(int i=0;tokens[i]!=0;i++){
    if(strncmp(tokens[i],"&&",2)==0){
      return i;
    }
  }
  return 0;
}

int seekor(char ** tokens){
  for(int i=0;tokens[i]!=0;i++){
    if(strncmp(tokens[i],"||",2)==0){
      return i;
    }
  }
  return 0;
}

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){
    char readChar = line[i];
    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
      	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
      	strcpy(tokens[tokenNo++], token);
      	tokenIndex = 0;
      }
    }
    else if(readChar == ';'){
          token[tokenIndex] = '\0';
          if (tokenIndex != 0){
          	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
          	strcpy(tokens[tokenNo++], token);
          	tokenIndex = 0;
          }
          token[0]=readChar;
          token[1]='\0';
          tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
          strcpy(tokens[tokenNo++], token);
    }
    else if(readChar =='|' && (i+1>=strlen(line) || line[i+1]!='|')){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
        tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
        strcpy(tokens[tokenNo++], token);
        tokenIndex = 0;
      }
      token[0]=readChar;
      token[1]='\0';
      tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
      strcpy(tokens[tokenNo++], token);
    }
    else if((i+1<strlen(line) && ((readChar == '|' && line[i+1] == '|')||(readChar == '&' && line[i+1] == '&')))){
          token[tokenIndex] = '\0';
          if (tokenIndex != 0){
          	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
          	strcpy(tokens[tokenNo++], token);
          	tokenIndex = 0;
          }
          token[0]=readChar;
          token[1]=line[i+1];
          token[2]='\0';
          tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
          strcpy(tokens[tokenNo++], token);
          i++;
    }
    else {
      token[tokenIndex++] = readChar;
    }
  }
  free(token);
  tokens[tokenNo] = 0 ;
  return tokens;
}

int execute(char **tokens){
//      printf(1,"%s\n",tokens[0]);

  int pipe_loc=seekpipe(tokens);
  int semi_loc=seeksemi(tokens);
  int input_loc = seekinput(tokens);
  int output_loc = seekoutput(tokens);
  int and_loc=seekand(tokens);
  int or_loc = seekor(tokens);
  if(pipe_loc){
      runpipe(tokens,pipe_loc);
      exit(0);
  }
  else if(and_loc){
    runand(tokens,and_loc);
    exit(0);
  }
  else if (or_loc){
    runor(tokens,or_loc);
    exit(0);
  }
  else if(semi_loc){
    runparallel(tokens,semi_loc);
    exit(0);
  }
  else if(output_loc || input_loc){
    if(output_loc){
      runoutput(tokens,output_loc);
      exit(0);
    }
    if(input_loc){
      runinput(tokens,input_loc);
      exit(0);
    }
  }
  else if (strncmp(tokens[0],"executeCommands",15)==0){
    runexecom(tokens);
    exit(0);
  }
  else if(tokens[0]){
    int d= exec(tokens[0], tokens);
    if(d==-1){
      printf(1,"Illegal command or arguments\n");
      exit(-1);
    }
  }
  return 1;
}

int
strncmp(const char *p, const char *q, uint n)
{
  while(n > 0 && *p && *p == *q)
    n--, p++, q++;
  if(n == 0)
    return 0;
  return (uchar)*p - (uchar)*q;
}

int runand(char **tokens,int and_loc){
  int i,tokenNo= 0;
  char **cmd1=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char **cmd2=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  int exit_status1=-1;
  int exit_status2=-1;
  for(i=0;i<and_loc;i++){
    cmd1[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd1[tokenNo++], tokens[i]);
  }
  cmd1[tokenNo]=0;
  tokenNo=0;
  for(i=and_loc+1;tokens[i]!=0;i++){
    cmd2[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd2[tokenNo++], tokens[i]);
  }
  cmd2[tokenNo]=0;
  if(!fork())
  {
    execute(cmd1);
  }
  wait(&exit_status1);
//  printf(1,"Exit status : %d\n",exit_status1);
  if(exit_status1!=-1){
    if(!fork())
   {
     execute(cmd2);
   }
   wait(&exit_status2);
//   printf(1,"Exit status : %d\n",exit_status2);
  }
  return 1;
}
int runor(char **tokens,int or_loc){
  int i,tokenNo= 0;
  char **cmd1=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char **cmd2=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  int exit_status1=-1;
  int exit_status2=-1;
  for(i=0;i<or_loc;i++){
    cmd1[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd1[tokenNo++], tokens[i]);
  }
  cmd1[tokenNo]=0;
  tokenNo=0;
  for(i=or_loc+1;tokens[i]!=0;i++){
    cmd2[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd2[tokenNo++], tokens[i]);
  }
  cmd2[tokenNo]=0;
  if(!fork())
  {
    execute(cmd1);
  }
  wait(&exit_status1);
  if(exit_status1==-1){
    if(!fork())
   {
     execute(cmd2);
   }
  }
  wait(&exit_status2);
  return 1;
}

int runinput(char **tokens,int input_loc){
  int i,tokenNo= 0;
  char **cmd1=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  for(i=0;i<input_loc;i++){
    cmd1[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd1[tokenNo++], tokens[i]);
  }
  cmd1[tokenNo]=0;

  int file  = open(tokens[input_loc+1],O_RDONLY);
  if(file==-1){
    printf(1,"Illegal command or arguments\n");
  }
  close(0);
  if(!fork())
  {
    int fd = dup(file);
    execute(cmd1);
    close(fd);
  }
  close(file);
  wait(0);
  return 1;
}
int runoutput(char **tokens,int output_loc){
  int i,tokenNo= 0;
  char **cmd1=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  for(i=0;i<output_loc;i++){
    cmd1[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd1[tokenNo++], tokens[i]);
  }
  cmd1[tokenNo]=0;

  int file  = open(tokens[output_loc+1],O_WRONLY | O_CREATE);
  if(file==-1){
    printf(1,"Illegal command or arguments\n");
  }
  close(1);
  if(!fork())
  {
    int fd = dup(file);
    execute(cmd1);
    close(fd);
  }
  close(file);

  wait(0);
  return 1;
}

int runpipe(char **tokens,int pipe_loc){
  int i,tokenNo= 0;
  int a[2];
  char **cmd1=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char **cmd2=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));

  for(i=0;i<pipe_loc;i++){
    cmd1[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd1[tokenNo++], tokens[i]);
  }
  cmd1[tokenNo]=0;
  tokenNo=0;
  for(i=pipe_loc+1;tokens[i]!=0;i++){
    cmd2[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd2[tokenNo++], tokens[i]);
  }
  cmd2[tokenNo]=0;
  pipe(a);
  if(!fork())
  {
     close(1);
     dup(a[1]);
     close(a[0]);
     execute(cmd1);
  }
  else if(!fork())
  {
     close(0);
     dup(a[0]);
     close(a[1]);
     execute(cmd2);
  }
  close(a[0]);
  close(a[1]);
  wait(0);
  wait(0);
 return 1;
}

int runparallel(char **tokens,int semi_loc){
  int i,tokenNo= 0;
  char **cmd1=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char **cmd2=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));

  for(i=0;i<semi_loc;i++){
    cmd1[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd1[tokenNo++], tokens[i]);
  }
  cmd1[tokenNo]=0;
  tokenNo=0;
  for(i=semi_loc+1;tokens[i]!=0;i++){
    cmd2[tokenNo]= (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(cmd2[tokenNo++], tokens[i]);
  }
  cmd2[tokenNo]=0;
  if(!fork())
  {
    execute(cmd1);
  }
   else if(!fork())
  {
    execute(cmd2);
  }
  wait(0);
  wait(0);
  return 1;
}

int runexecom(char **tokens){
  char *c = (char *) malloc(100*sizeof(char));
  int file  = open(tokens[1],O_RDONLY);
  // char**tok;
  if(file==-1){
    printf(1,"Illegal command or arguments\n");
    return 1;
  }
  int  tokenIndex = 0,i;
    static char tok[100];
    memset(tok,0,sizeof(tok));
  while (read(file, c ,sizeof(c))) {
    for(i =0; i < strlen(c); i++){
      char readChar1 = c[i];
      // printf(1,"%c\n",readChar);
      //printf(1,"tokenIndex : %d\n",tokenIndex);
      if (readChar1 == '\n'){
        // printf(1,"Line break here :1\n");
        tok[tokenIndex++] = ' ';
        tok[tokenIndex++] = '\0';
        if (tokenIndex != 0){
//          printf(1,"Token : %s\n",tok);
          if(fork() == 0){
            char ** cmd;
            cmd = tokenize(tok);
            // for(int j=0;cmd[j]!=0;j++){
            //   printf(1,"%s\n",cmd[j]);
            // }
            execute(cmd);
          }
          wait(0);
          tokenIndex = 0;
        }
      }
      else{
        tok[tokenIndex++] = readChar1;
      }
    }
//    printf(1,"%s\n", c);
 }
   close(file);
   return 1;
}


int
getcmd(char *buf, int nbuf)
{
  printf(2, "MyShell>");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0)
    return -1;
  return 0;
}

int
main(){
  static char buf[100];
	char  **tokens;
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(strncmp(buf,"exit\n",5)==0){
      //wait(0);
      //exit(0);
      break;
    }
    if(fork() == 0)
    {
      tokens = tokenize(buf);
      execute(tokens);
      //exit(0);
    }
    wait(0);
  }
  exit(0);
}
