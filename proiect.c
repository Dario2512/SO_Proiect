#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void close_file(int file_description);
void image_size(int file_description, int *h, int *w);

typedef struct data_t{
    char file_name[40];
    int h;
    int w;
    int file_size;
    int user_id;
    char last_modified[11];
    int link_count;
    char user_access[4];
    char group_access[4];
    char others_access[4];
}data_t;

void print_data(data_t d){
    printf("Nume: %39s,Inaltime:%d,Lungime:%d,File_size:%d,User_id:%d,last_modified:%s,Link_count:%d,user_access:%s,group_access:%s,others_access:%s",d.file_name, d.h, d.w, d.file_size, d.user_id, d.last_modified,d.link_count, d.user_access, d.group_access, d.others_access);
}

int main(){
    char file_path[] = "imagine.bmp";
    int open_flags = O_RDONLY;
    int height = 0;
    int width = 0;
    int size = 0;
    data_t data;
   // struct stat stats;


    int image_description = open(file_path, open_flags);
    if(image_description < 0){
       perror("error deschidere fisier\n"); 
    } 

    if(lseek(image_description,18, SEEK_SET) < 0){
        perror("Error cursor\n");
    }

    if(read(image_description, &width, 4) < 0){
       perror("error deschidere fisier\n");
       exit(-1); 
    }
    
    if(lseek(image_description,22, SEEK_SET) < 0){
        perror("Error cursor\n");
    }

    if(read(image_description, &height, 4) < 0){
       perror("error deschidere fisier\n");
       exit(-1); 
    }

      if(lseek(image_description,2, SEEK_SET) < 0){
        perror("Error cursor\n");
    }

    if(read(image_description, &size, 4) < 0){
       perror("error deschidere fisier\n");
       exit(-1); 
    }

    printf("Width: %d\nHeight: %d\nSize: %d Bytes\n", width, height, size);

   // image_size(image_description,height,width);

    return 0;
}

void close_file(int file_description){
    if(close(file_description) < 0) {
        perror("error close");
        exit(-1);
    }
}
/*
void image_size(int file_description, int *h, int *w){
    if(lseek(file_description,18, SEEK_SET) < 0){
        perror("Error cursor\n");
    }

    if(read(file_description, &w, 4) < 0){
       perror("error deschidere fisier\n");
       exit(-1); 
    }
    
    if(lseek(file_description, 22, SEEK_SET) < 0){
        perror("Error cursor\n");
    }

    if(read(file_description, &h, 4) < 0){
       perror("error deschidere fisier\n");
       exit(-1); 
    }

    printf("Width: %d\n Height: %d\n", h, w);

    close_file(file_description);
}
*/