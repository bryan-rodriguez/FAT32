/*

Name: Bryan Rodriguez
ID: 1001647248

*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

// We want to split our command line up into tokens
// so we need to define what delimits our tokens.
// In this case  white space
// will separate the tokens on our command line
#define MAX_NUM_ARGUMENTS 4
#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255 // The maximum command-line size

struct __attribute__((__packed__)) DirectoryEntry
{
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t unused[8];
  uint16_t ClusterHigh;
  uint8_t unused2[4];
  uint16_t ClusterLow;
  uint32_t size;
};

struct DirectoryEntry dir[16];
FILE * fp = NULL;



int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int32_t BPB_FATSz32;

int LBToOffset(int32_t sector)
{
  return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

int16_t NextLB(uint32_t sector)
{
  uint32_t FATAdress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector * 4);
  int16_t val;
  fseek(fp, FATAdress, SEEK_SET);
  fread(&val, 2, 1, fp);
  return val;
}

void printInfo()
{
  printf("BPB_BytsPerSec: %d %x\n", BPB_BytsPerSec, BPB_BytsPerSec);
  printf("BPB_SecPerClus: %d %x\n", BPB_SecPerClus, BPB_SecPerClus);
  printf("BPB_RsvdSecCnt: %d %x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
  printf("BPB_NumFATs: %d %x\n", BPB_NumFATs, BPB_NumFATs);
  printf("BPB_FATSz32: %d %x\n", BPB_FATSz32, BPB_FATSz32);
}

int compareStr(char * token, char dirName[11])
{
  int charCount = 0;
  char *foundChar = strchr(token, '.');
  if(foundChar)
  {
    int index = foundChar - token;
    memmove(&token[index], &token[index + 1], strlen(token) - index);
  }

  for(int i = 0; i < dirName[i]; i++)
  {
    if(dirName[i] != ' ')
    {
      dirName[charCount++] = tolower(dirName[i]);
    }
  }
  dirName[charCount] = '\0';

  if(strncmp(token, dirName, strlen(token)) == 0)
  {
    return 1;
  }

  return 0;
}



void print_ls()
{

  for(int i = 0; i < 16; i++)
  {
    if((dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 ) || dir[i].DIR_Attr == 0x20)
    {
      char fileName[12];
      strncpy(&fileName[0],&dir[i].DIR_Name[0], 11);
      fileName[12] = '\0';
      printf("%s\n", fileName);
    }
  }
}

int main()
{

  
  char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);


  while (1)
  {

    // Print out the msh prompt
    printf("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));

    //exits program if cmd_str is equal to exit or quit


    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;

    char *working_str = strdup(cmd_str);

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while (((argument_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    //avoiding seg fault by checking if the first token is null, if not then
    //continue running the code
    if (token[0] != NULL)
    {

      
      if (strcmp(token[0], "open") == 0 && fp == NULL)
      {
        fp = fopen(token[1], "r");
        if(fp == NULL)
        {
          perror("Error: File system image not found");
        }
        else
        {
          fseek(fp, 11, SEEK_SET);
          fread(&BPB_BytsPerSec, 2, 1, fp);
          fseek(fp, 13, SEEK_SET);
          fread(&BPB_SecPerClus, 1, 1, fp);
          fseek(fp, 14, SEEK_SET);
          fread(&BPB_RsvdSecCnt, 2, 1, fp);
          fseek(fp, 16, SEEK_SET);
          fread(&BPB_NumFATs, 1, 1, fp);
          fseek(fp, 36, SEEK_SET);
          fread(&BPB_FATSz32, 4, 1, fp);
          
          fseek(fp, (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec), SEEK_SET);
          fread(&dir, sizeof(struct DirectoryEntry), 16, fp);
          
        }
      }
      else if(strcmp(token[0], "open") == 0 && fp != NULL)
      {
        printf("Error: File system image already opened\n");
      }
      else if(strcmp(token[0], "close") == 0 && fp == NULL)
      {
        printf("Error: File system not open\n");
      }
      else if(strcmp(token[0], "close") == 0 && fp != NULL)
      {
        fclose(fp);
        fp = NULL;
      }
      else if(fp == NULL)
      {
        printf("Error: File system must be opened\n");
      }
      else
      {
        if(strcmp(token[0], "info") == 0)
        {
          printInfo();
        }
        if(strcmp(token[0], "ls") == 0)
        {
          print_ls();
        }
        if(strcmp(token[0], "get") == 0)
        {
          for(int i = 0; i < 16; i++)
          {
            char dirNameCpy[11];
            char * cpyToken = (char * )malloc(MAX_COMMAND_SIZE);
            strcpy(cpyToken, token[1]);
            strncpy(dirNameCpy, dir[i].DIR_Name, 11);
            unsigned char str[512];
            if(compareStr(cpyToken, dirNameCpy) && dir[i].DIR_Attr == 0x20)
            {
              int bytesRemaining = dir[i].size;
              FILE * ofp = fopen(token[1], "w");
              int cluster = dir[i].ClusterLow;
              while(cluster != -1)
              {
                int offset = LBToOffset(cluster);
                fseek(fp, offset, SEEK_SET);
                fread(str, 512, 1, fp);
                fwrite(str, 512, 1,ofp);
                cluster = NextLB(cluster);
                bytesRemaining = bytesRemaining - 512;
              }
              if(bytesRemaining != 0)
              {
                fread(str, bytesRemaining, 1, fp);
                fwrite(str, bytesRemaining, 1,ofp);
              }
            }
          }
        }
        if(strcmp(token[0], "read") == 0)
        {
          for(int i = 0; i < 16; i++)
          {
            char dirNameCpy[11];
            strncpy(dirNameCpy, dir[i].DIR_Name, 11);
            unsigned char str[512];
            if(compareStr(token[1], dirNameCpy) && dir[i].DIR_Attr == 0x20)
            {
              int cluster = dir[i].ClusterLow;
              int offset = LBToOffset(cluster);
              fseek(fp, offset + atoi(token[2]), SEEK_SET);
              fread(str, 1, atoi(token[3]), fp);
              for(int i = 0; i < atoi(token[3]); i++)
              {
                printf("%x ", str[i]);
              }
              printf("\n");
            }
          }
        }
        if(strcmp(token[0], "cd") == 0)
        {
          if(strcmp(token[1], "..") != 0)
          {
            for(int i = 0; i < 16; i++)
            {
              char dirNameCpy[11];
              strncpy(dirNameCpy, dir[i].DIR_Name, 11);
              if(compareStr(token[1], dirNameCpy) == 1 && dir[i].DIR_Attr == 0x10)
              {
                int cluster = dir[i].ClusterLow;
                int offset = LBToOffset(cluster);
                fseek(fp, offset, SEEK_SET);
                fread(&dir, sizeof(struct DirectoryEntry), 16, fp);
              }
            }
          }
          else
          {
            for(int i = 0; i < 16; i++)
            {
              if(strncmp(token[1], dir[i].DIR_Name, 2) == 0 && dir[i].DIR_Attr == 0x10)
              {    
                int cluster = dir[i].ClusterLow;
                if(cluster == 0)
                  cluster = 2;
                int offset = LBToOffset(cluster);
                fseek(fp, offset, SEEK_SET);
                fread(&dir, sizeof(struct DirectoryEntry), 16, fp);
              }
            }
          }
        }
        if(strcmp(token[0], "stat") == 0)
        {
          for(int i = 0; i < 16; i++)
          {
            char dirNameCpy[11];
            strncpy(dirNameCpy, dir[i].DIR_Name, 11);
            if(compareStr(token[1], dirNameCpy) == 1 )
            {
              if(dir[i].DIR_Attr == 0x10)
              {
                printf("File Size: 0\n");
              }
              else
              {
                printf("File Size: %d\n", dir[i].size);
              }
              printf("First Cluster Low: %d\n", dir[i].ClusterLow);
              printf("DIR_ATTR: %d\n", dir[i].DIR_Attr);
              printf("First Cluster High: %d\n", dir[i].ClusterHigh);
            }
          }
        }

      }
    }

    free(working_root);
  }
  return 0;
}