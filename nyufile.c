#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <dirent.h>
#include <openssl/sha.h>

#define SHA_DIGEST_LENGTH 20




void printing(){
  printf("Usage: ./nyufile disk <options>\n");
  printf("  -i                     Print the file system information.\n");
  printf("  -l                     List the root directory.\n");
  printf("  -r filename [-s sha1]  Recover a contiguous file.\n");
  printf("  -R filename -s sha1    Recover a possibly non-contiguous file.\n");

}

bool wheni(char** argv){
        char* filename = argv[optind];



        int fd = open(filename, O_RDONLY);
        if (fd ==-1){
          close(fd);
          return true;
          // break;
        }
        struct stat sb;
        if (fstat(fd,&sb)==-1){


          close(fd);
          return true;
          // break;
        }

        void *addr = mmap(NULL, 512, PROT_READ, MAP_PRIVATE, fd, 0);
      
        if(addr==MAP_FAILED){

          close(fd);
          return true;
          
          // break;
        }
 

        // reserved sector * #bytes + #Fat * #32bitsector
        //4800 - 4000 = 18432 - 16384 = 2048 

        char *boot = (char *)addr;
        // unsigned int num_fat = boot[16];
        short a = *(short*)(boot+11);
        short b = *(short*)(boot+14);






        printf("Number of FATs = %u\n",boot[16]);
        printf("Number of bytes per sector = %u\n", a);
        printf("Number of sectors per cluster = %u\n", boot[13]);
        printf("Number of reserved sectors = %u\n",b);
        
        munmap(addr,512);
        return false;


}

bool whenl(char** argv){
        char* filename = argv[optind];



        int fd = open(filename, O_RDONLY);
        if (fd ==-1){
          close(fd);
          return true;
          // break;
        }
        struct stat sb;
        if (fstat(fd,&sb)==-1){


          close(fd);
          return true;
          // break;
        }

        void *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        


        if(addr==MAP_FAILED){

          close(fd);
          return true;
          

        }

        // reserved sector * #bytes + #Fat * #32bitsector
        //4800 - 4000 = 18432 - 16384 = 2048 = numz32 * bytes per sector *#perclustor


        //reserved sector * *percluster * #bytes  + #Fat * #32z * percluster * persector




        unsigned char *boot = (unsigned char *)addr;
        // unsigned int num_fat = boot[16];
        short persector = *(short*)(boot+11);
        // short numres = *(short*)(boot+14);
        // unsigned int percluster = boot[13];
        // unsigned int z32 = *(unsigned int*)(boot+36);


        // unsigned int rootd = numres * persector  + num_fat * z32 * persector;
        unsigned int rootd = *(unsigned int*)(boot+44)  * persector;


        int num_entry = 0;
        // unsigned char entry = boot[rootd];

        bool first = true;

        while(rootd<sb.st_size){
          

          if(rootd+32 > sb.st_size) break;
          if(!first) rootd+=32;
          first = false;
          


          if(*(boot+rootd) == 0x00) continue;

          if(*(boot+rootd+11) != 0x01 &&*(boot+rootd+11) != 0x02 &&*(boot+rootd+11) != 0x04 &&*(boot+rootd+11) != 0x08 &&*(boot+rootd+11) != 0x10 &&*(boot+rootd+11) != 0x20) continue;

          if(*(boot+rootd) == 0xe5)  continue;


          char fname[13];
          int count =0;
          bool good = true;
          bool good2 = false;
          bool good3 = true;

          for(int i=0;i<11;i++){

            if(boot[rootd+i]=='\n'||boot[rootd+i]=='.') good = false;
            if(boot[rootd+i]==0) good = false;
            if(boot[rootd+i] == ' '||boot[rootd+i]=='\n') good3=false;
            if(boot[rootd+i] == ' '||boot[rootd+i]=='\n') continue;


            if((i<8) && (good3 == false)) good = false;
            if(i==8){
              fname[count++] = '.';
              good2 = true;
            }

            if(((i==9)||(i==10)) && (good2 ==false)) good = false;

            fname[count++]= boot[rootd+i];
          }
          fname[count] = '\0';

          if (good == false) continue;

          printf("%s",fname);

          short high = *(short*)(boot+rootd+20);
          short low = *(short*)(boot+rootd+26);


          short startings[] = {low,high};
          // short startings[] = {high,low};

          unsigned int starting = *(unsigned int*)startings;

          // unsigned int starting = (high<<16) | low;


          unsigned int size = *(unsigned int*)(boot+rootd+28);


  


          if(boot[rootd+11]==0x10){
            printf("/ (starting cluster = %u)\n", starting);
          }

          else{
            printf(" (size = %u",size);
            if(size!=0){
              printf(", starting cluster = %u",starting);
            }
            printf(")\n");
          }

          
          num_entry++;






        }

        printf("Total number of entries = %d\n",num_entry);
        
        munmap(addr,512);
        return false;


}

bool whenr(char **argv, char* target){


        int samename =0;
        unsigned int temprootd;
        char* filename = argv[optind];
        char realtarget[12];
        char firstletter = target[0];
        strncpy(realtarget,target+1,strlen(target)-1);
        realtarget[strlen(target)-1] = '\0';
        


        int fd = open(filename, O_RDWR);
        if (fd ==-1){
          close(fd);
          return true;
          // break;
        }
        struct stat sb;
        if (fstat(fd,&sb)==-1){


          close(fd);
          return true;
          // break;
        }

        void *addr = mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        


        if(addr==MAP_FAILED){

          close(fd);
          return true;
          

        }






        unsigned char *boot = (unsigned char *)addr;

        short persector = *(short*)(boot+11);


        unsigned int rootd = *(unsigned int*)(boot+44)  * persector;
        // unsigned int rootc = *(unsigned int*)(boot+44)  * persector;



        bool first = true;

        while(rootd<sb.st_size){
          

          if(rootd+32 > sb.st_size) break;
          if(!first) rootd+=32;
          first = false;
          


          if(*(boot+rootd) == 0x00) continue;

          if(*(boot+rootd+11) != 0x01 &&*(boot+rootd+11) != 0x02 &&*(boot+rootd+11) != 0x04 &&*(boot+rootd+11) != 0x08 &&*(boot+rootd+11) != 0x10 &&*(boot+rootd+11) != 0x20) continue;

          if(*(boot+rootd) != 0xe5)  continue;




          char fname[13];
          int count =0;
          bool good = true;
          bool good2 = false;
          bool good3 = true;

          for(int i=0;i<11;i++){

            if(boot[rootd+i]=='\n'||boot[rootd+i]=='.') good = false;
            if(boot[rootd+i]==0) good = false;
            if(boot[rootd+i] == ' '||boot[rootd+i]=='\n') good3=false;
            if(boot[rootd+i] == ' '||boot[rootd+i]=='\n') continue;


            if((i<8) && (good3 == false)) good = false;
            if(i==8){
              fname[count++] = '.';
              good2 = true;
            }

            if(((i==9)||(i==10)) && (good2 ==false)) good = false;

            fname[count++]= boot[rootd+i];
          }
          fname[count] = '\0';


          if (good == false) continue;

          char realf[12];
          strncpy(realf,fname+1,strlen(fname)-1);
          realf[strlen(fname)-1] = '\0';

          if(strcmp(realtarget,realf)==0){
            samename++;
            temprootd = rootd;
        
          }





        }


        if(samename ==0){
          printf("%s: file not found\n",target);
        
          munmap(addr,512);
          return false;
        }
        else if(samename > 1){
          printf("%s: multiple candidates found\n",target);
          return false;
        }
        else{

            rootd = temprootd;
            boot[rootd] = (unsigned char)firstletter;
            printf("%s: successfully recovered\n",target);


            short high = *(short*)(boot+rootd+20);
            short low = *(short*)(boot+rootd+26);


            short startings[] = {low,high};

            unsigned int starting = *(unsigned int*)startings;


            unsigned int ind = starting*4;


            unsigned int num_fat = boot[16];


            short numres = *(short*)(boot+14);
            unsigned int z32 = *(unsigned int*)(boot+36);


            unsigned int firstfat = numres * persector;

            unsigned int filesize = *(unsigned int*)(boot+rootd+28);
            unsigned int clusters = (filesize+boot[13]*persector -1)/(boot[13]*persector);




            if(filesize!=0){

              // printf("%u\n",clusters);
              for(unsigned int j=0; j< clusters;j++ ){


                unsigned int currentcluster = starting + j;

                unsigned int currfat = firstfat + j*4;



                for(unsigned int i = 0 ; i < num_fat ; i++){


                  if(j==clusters-1){
                    boot[currfat + ind + i * z32 * persector] = 0xff;
                    boot[currfat + 1 + ind + i * z32 * persector] = 0xff;
                    boot[currfat + 2 + ind + i * z32 * persector] = 0xff;
                    boot[currfat + 3 + ind + i * z32 * persector] = 0x0f;
                  }
                  else{
                    unsigned int nextcluster = currentcluster + 1;

                    boot[currfat + ind + i * z32 * persector] = nextcluster%(256);
                    nextcluster /= 256;
                    boot[currfat + 1 + ind + i * z32 * persector] = nextcluster%(256);
                    nextcluster /= 256;
                    boot[currfat + 2 + ind + i * z32 * persector] = nextcluster%(256);
                    nextcluster /= 256;
                    boot[currfat + 3 + ind + i * z32 * persector] = nextcluster%(256);


                  }
                  
                

                }

              }
              

            }


            




            return false;
        }
        

}




bool whenrs(char** argv, char* target, char* sha1){




        char* filename = argv[optind];
        char realtarget[12];
        char firstletter = target[0];
        strncpy(realtarget,target+1,strlen(target)-1);
        realtarget[strlen(target)-1] = '\0';
        


        int fd = open(filename, O_RDWR);
        if (fd ==-1){
          close(fd);
          return true;
          // break;
        }
        struct stat sb;
        if (fstat(fd,&sb)==-1){


          close(fd);
          return true;
          // break;
        }

        void *addr = mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        


        if(addr==MAP_FAILED){

          close(fd);
          return true;
          

        }






        unsigned char *boot = (unsigned char *)addr;

        short persector = *(short*)(boot+11);


        unsigned int rootd = *(unsigned int*)(boot+44)  * persector;
        // unsigned int rootc = *(unsigned int*)(boot+44)  * persector;



        bool first = true;

        while(rootd<sb.st_size){
          

          if(rootd+32 > sb.st_size) break;
          if(!first) rootd+=32;
          first = false;
          


          if(*(boot+rootd) == 0x00) continue;

          if(*(boot+rootd+11) != 0x01 &&*(boot+rootd+11) != 0x02 &&*(boot+rootd+11) != 0x04 &&*(boot+rootd+11) != 0x08 &&*(boot+rootd+11) != 0x10 &&*(boot+rootd+11) != 0x20) continue;

          if(*(boot+rootd) != 0xe5)  continue;




          char fname[13];
          int count =0;
          bool good = true;
          bool good2 = false;
          bool good3 = true;

          for(int i=0;i<11;i++){

            if(boot[rootd+i]=='\n'||boot[rootd+i]=='.') good = false;
            if(boot[rootd+i]==0) good = false;
            if(boot[rootd+i] == ' '||boot[rootd+i]=='\n') good3=false;
            if(boot[rootd+i] == ' '||boot[rootd+i]=='\n') continue;


            if((i<8) && (good3 == false)) good = false;
            if(i==8){
              fname[count++] = '.';
              good2 = true;
            }

            if(((i==9)||(i==10)) && (good2 ==false)) good = false;

            fname[count++]= boot[rootd+i];
          }
          fname[count] = '\0';


          if (good == false) continue;

          char realf[12];
          strncpy(realf,fname+1,strlen(fname)-1);
          realf[strlen(fname)-1] = '\0';

          if(strcmp(realtarget,realf)==0){



            


            short high = *(short*)(boot+rootd+20);
            short low = *(short*)(boot+rootd+26);


            short startings[] = {low,high};

            unsigned int starting = *(unsigned int*)startings;


            


            unsigned int ind = starting*4;


            unsigned int num_fat = boot[16];


            short numres = *(short*)(boot+14);
            unsigned int z32 = *(unsigned int*)(boot+36);


            unsigned int firstfat = numres * persector;

            unsigned int filesize = *(unsigned int*)(boot+rootd+28);
            unsigned int clusters = (filesize+boot[13]*persector -1)/(boot[13]*persector);



            unsigned char hash[SHA_DIGEST_LENGTH];


            unsigned int data = numres*persector + num_fat * z32 *persector;
            data += (starting-2) * boot[13]*persector;

            SHA1(boot+data, filesize, hash);


            

            char hash2[41];
            for(int k=0;k<SHA_DIGEST_LENGTH;k++){
              sprintf(&hash2[k*2],"%02x",hash[k]);
            }

    
            hash2[40]='\0';

            // printf("hash2: %s\n",hash2);
            // printf("sha1: %s\n",sha1);


            if(strcmp(hash2,sha1)==0){

              boot[rootd] = (unsigned char)firstletter;
              printf("%s: successfully recovered with SHA-1\n",target);

              if(filesize!=0){


              for(unsigned int j=0; j< clusters;j++ ){


                unsigned int currentcluster = starting + j;

                unsigned int currfat = firstfat + j*4;



                for(unsigned int i = 0 ; i < num_fat ; i++){


                  if(j==clusters-1){
                    boot[currfat + ind + i * z32 * persector] = 0xff;
                    boot[currfat + 1 + ind + i * z32 * persector] = 0xff;
                    boot[currfat + 2 + ind + i * z32 * persector] = 0xff;
                    boot[currfat + 3 + ind + i * z32 * persector] = 0x0f;
                  }
                  else{
                    unsigned int nextcluster = currentcluster + 1;

                    boot[currfat + ind + i * z32 * persector] = nextcluster%(256);
                    nextcluster /= 256;
                    boot[currfat + 1 + ind + i * z32 * persector] = nextcluster%(256);
                    nextcluster /= 256;
                    boot[currfat + 2 + ind + i * z32 * persector] = nextcluster%(256);
                    nextcluster /= 256;
                    boot[currfat + 3 + ind + i * z32 * persector] = nextcluster%(256);


                  }
                  
                

                }

              }
              

            }

            return false;


            }
            continue;


            


            




            
          }



        }

        printf("%s: file not found\n",target);
        
        munmap(addr,sb.st_size);
        return false;





}






int main(int arg, char **argv) {

  bool toprint = true;
  int ch;
  int tell =0;
  char* temp;
  char* temp2;

  while((ch = getopt(arg, argv, "ilr:s:R:")) != -1){
    switch(ch) {
      default:{
        
        toprint = false;
        printing();
        break;
      }
        

      case 'i':{

        if (strcmp("-i",argv[1])==0){
          toprint = true;
          break;
        }
        tell = 1;
        break;

      }

        
      case 'l':{
        tell =2;
        break;
      }

      case 'r':{
        // printf("%s",optarg);
        temp = optarg;
        tell =3;
        break;
      } 

      case 's':{
        temp2 = optarg;
        tell =4;
        break;
      }

      case 'R':{
        temp = optarg;
        tell =5;
        break;
      }
      
    }

    

  }


  if(tell==1){
    toprint = wheni(argv);
  }

  if(tell==2){
    toprint = whenl(argv);
  }

  
  if(tell==3){
    // printf("%s\n",temp);
    toprint = whenr(argv,temp);
  }

  if(tell==4){
    toprint = whenrs(argv,temp,temp2);
  }

  if(tell==5){
    toprint = whenrs(argv,temp,temp2);

  }
  if(toprint){

    printing();

  }



}


