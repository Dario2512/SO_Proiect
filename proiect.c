#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#define DIMENSIUNE_HEADER_FISIER 14
#define DIMENSIUNE_HEADER_INFO 40

typedef struct {
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_m;
    int32_t y_pixels_per_m;
    uint32_t colors_used;
    uint32_t colors_important;
} HeaderFisier;

typedef struct {
    uint32_t size;           
    int32_t width;           
    int32_t height;          
    uint16_t planes;         
    uint16_t bit_count;      
    uint32_t compression;    
    uint32_t image_size;     
    int32_t x_pixels_per_m;  
    int32_t y_pixels_per_m;  
    uint32_t colors_used;    
    uint32_t colors_important;  
} HeaderInfo;

void eroare(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void scrieStatistici(const char *nume_fisier, struct stat info, uint32_t idUtilizator, int isSymbolicLink, const char *director_iesire) {
    char nume_fisier_statistica[512];
    sprintf(nume_fisier_statistica, "%s/%s_statistica.txt", director_iesire, nume_fisier);

    int fd = open(nume_fisier_statistica, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        perror("Eroare la deschiderea fisierului de statistici");
        exit(EXIT_FAILURE);
    }

    char buffer[1024]; 
    time_t t = info.st_mtime;
    struct tm *tm_info = localtime(&t);
    char timp_str[20];
    strftime(timp_str, sizeof(timp_str), "%d.%m.%Y", tm_info);

    char permisiuni_user[4] = "-", permisiuni_group[4] = "-", permisiuni_other[4] = "-";
    if (S_ISDIR(info.st_mode) || S_ISREG(info.st_mode)) {
        if (info.st_mode & S_IRUSR) permisiuni_user[0] = 'R';
        if (info.st_mode & S_IWUSR) permisiuni_user[1] = 'W';
        if (info.st_mode & S_IXUSR) permisiuni_user[2] = 'X';
        if (info.st_mode & S_IRGRP) permisiuni_group[0] = 'R';
        if (info.st_mode & S_IWGRP) permisiuni_group[1] = 'W';
        if (info.st_mode & S_IXGRP) permisiuni_group[2] = 'X';
        if (info.st_mode & S_IROTH) permisiuni_other[0] = 'R';
        if (info.st_mode & S_IWOTH) permisiuni_other[1] = 'W';
        if (info.st_mode & S_IXOTH) permisiuni_other[2] = 'X';
    }

    if (S_ISREG(info.st_mode)) {
        if (isSymbolicLink) {
            struct stat targetInfo;
            if (lstat(nume_fisier, &targetInfo) == -1) {
                perror("Eroare la lstat pentru link-ul simbolic");
                close(fd);
                exit(EXIT_FAILURE);
            }
            sprintf(buffer, "Nume legatura: %s\nDimensiune legatura: %lu\nDimensiune fisier tinta: %lu\nDrepturi de acces user legatura: %s\nDrepturi de acces grup legatura: %s\nDrepturi de acces altii legatura: %s\n\n",
                    nume_fisier, (unsigned long)info.st_size, (unsigned long)targetInfo.st_size, permisiuni_user, permisiuni_group, permisiuni_other);
            write(fd, buffer, strlen(buffer));
        } else if (strstr(nume_fisier, ".bmp") != NULL) {
            int fdBMP = open(nume_fisier, O_RDONLY);
            if (fdBMP == -1) {
                perror("Eroare la deschiderea fisierului BMP");
                close(fd);
                exit(EXIT_FAILURE);
            }
            HeaderFisier headerFisier;
            if (read(fdBMP, &headerFisier, DIMENSIUNE_HEADER_FISIER) == DIMENSIUNE_HEADER_FISIER) {
                if (headerFisier.signature == 0x4D42) { 
                    HeaderInfo headerInfo;
                    if (read(fdBMP, &headerInfo, DIMENSIUNE_HEADER_INFO) != DIMENSIUNE_HEADER_INFO) {
                        perror("Eroare la citirea header-ului INFO");
                        close(fdBMP);
                        close(fd);
                        exit(EXIT_FAILURE);
                    }

                    sprintf(buffer, "Nume fisier: %s\nInaltime: %d\nLatime: %d\nDimensiune: %lu\nIdentificator utilizator: %u\nTimp ultimei modificari: %s\nContor legaturi: %lu\nDrepturi de acces user: %s\nDrepturi de acces grup: %s\nDrepturi de acces altii: %s\n\n",
                            nume_fisier, headerInfo.height, headerInfo.width, (unsigned long)info.st_size, idUtilizator, timp_str, info.st_nlink, permisiuni_user, permisiuni_group, permisiuni_other);
                    write(fd, buffer, strlen(buffer));
                } else {
                    printf("Fisierul nu este un BMP valid.\n");
                }
            }
            close(fdBMP);
        } else {
            sprintf(buffer, "Nume fisier: %s\nDimensiune: %lu\nIdentificator utilizator: %u\nTimp ultimei modificări: %s\nContor legaturi: %lu\nDrepturi de acces user: %s\nDrepturi de acces grup: %s\nDrepturi de acces altii: %s\n\n",
                    nume_fisier, (unsigned long)info.st_size, idUtilizator, timp_str, info.st_nlink, permisiuni_user, permisiuni_group, permisiuni_other);
            write(fd, buffer, strlen(buffer));
        }
    } else if (S_ISLNK(info.st_mode)) {
        char target_path[1024]; 
        ssize_t len = readlink(nume_fisier, target_path, sizeof(target_path) - 1);
        if (len == -1) {
            perror("Eroare la citirea link-ului simbolic");
            close(fd);
            exit(EXIT_FAILURE);
        }
        target_path[len] = '\0'; 

        struct stat targetInfo;
        if (lstat(target_path, &targetInfo) == -1) {
            perror("Eroare la lstat pentru fisierul target");
            close(fd);
            exit(EXIT_FAILURE);
        }

    
        sprintf(buffer, "Nume legatura: %s\nDimensiune legatura: %lu\nDimensiune fisier tinta: %lu\nDrepturi de acces user legatura: %s\nDrepturi de acces grup legatura: %s\nDrepturi de acces altii legatura: %s\n\n",
                nume_fisier, (unsigned long)info.st_size, (unsigned long)targetInfo.st_size, permisiuni_user, permisiuni_group, permisiuni_other);
        write(fd, buffer, strlen(buffer));
    } else if (S_ISDIR(info.st_mode)) {
        sprintf(buffer, "Nume director: %s\nIdentificator utilizator: %u\nDrepturi de acces user: %s\nDrepturi de acces grup: %s\nDrepturi de acces altii: %s\n\n",
                nume_fisier, idUtilizator, permisiuni_user, permisiuni_group, permisiuni_other);
        write(fd, buffer, strlen(buffer));
    }

    if (close(fd) == -1) {
        perror("Eroare la închiderea fisierului de statistici");
        exit(EXIT_FAILURE);
    }
}

void conversieGriBMP(const char *nume_fisier, const char *director_iesire) {
    int fdBMP = open(nume_fisier, O_RDWR);
    if (fdBMP == -1) {
        perror("Eroare la deschiderea fisierului BMP");
        return;
    }

    HeaderFisier headerFisier;
    if (read(fdBMP, &headerFisier, DIMENSIUNE_HEADER_FISIER) != DIMENSIUNE_HEADER_FISIER) {
        perror("Eroare la citirea header-ului fisierului BMP");
        close(fdBMP);
        return;
    }

    if (headerFisier.signature != 0x4D42) {
        printf("Fisierul nu este un BMP valid.\n");
        close(fdBMP);
        return;
    }

    HeaderInfo headerInfo;
    if (read(fdBMP, &headerInfo, DIMENSIUNE_HEADER_INFO) != DIMENSIUNE_HEADER_INFO) {
        perror("Eroare la citirea header-ului INFO");
        close(fdBMP);
        return;
    }

    int32_t width = headerInfo.width;
    int32_t height = headerInfo.height;

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            unsigned char pixel[3];
            if (read(fdBMP, pixel, 3) != 3) {
                perror("Eroare la citirea pixelilor");
                close(fdBMP);
                return;
            }

            unsigned char pixelGri = 0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0];
            lseek(fdBMP, -3, SEEK_CUR);
            write(fdBMP, &pixelGri, 1);
            write(fdBMP, &pixelGri, 1);
            write(fdBMP, &pixelGri, 1);
        }
    }

    close(fdBMP);

    struct stat info;
    if (lstat(nume_fisier, &info) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        return;
    }

    uint32_t idUtilizator = info.st_uid;
    int isSymbolicLink = S_ISLNK(info.st_mode);
    scrieStatistici(nume_fisier, info, idUtilizator, isSymbolicLink, director_iesire);
}


void proceseazaDirector(const char *director_intrare, const char *director_iesire) {
    DIR *dir = opendir(director_intrare);
    if (dir == NULL) {
        perror("Eroare la deschiderea directorului de intrare");
        return;
    }

    struct dirent *intrare;
    while ((intrare = readdir(dir)) != NULL) {
        if (strcmp(intrare->d_name, ".") != 0 && strcmp(intrare->d_name, "..") != 0) {
            char path_intrare[512];
            sprintf(path_intrare, "%s/%s", director_intrare, intrare->d_name);

            struct stat info;
            if (lstat(path_intrare, &info) == -1) {
                perror("Eroare la obtinerea informatiilor despre intrare");
                continue;
            }

            uint32_t idUtilizator = info.st_uid;
            int isSymbolicLink = S_ISLNK(info.st_mode);

            if (S_ISREG(info.st_mode)) {
                if (strstr(intrare->d_name, ".bmp") != NULL) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        conversieGriBMP(path_intrare, director_iesire);
                        exit(EXIT_SUCCESS);
                    } else if (pid < 0) {
                        perror("Eroare la fork");
                        continue;
                    }
                } else {
                    pid_t pid = fork();
                    if (pid == 0) {
                        scrieStatistici(path_intrare, info, idUtilizator, isSymbolicLink, director_iesire);
                        exit(EXIT_SUCCESS);
                    } else if (pid < 0) {
                        perror("Eroare la fork");
                        continue;
                    }
                }
            } else if (S_ISDIR(info.st_mode)) {
                pid_t pid = fork();
                if (pid == 0) {
                    char path_iesire[512];
                    sprintf(path_iesire, "%s/%s", director_iesire, intrare->d_name);
                    mkdir(path_iesire, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    proceseazaDirector(path_intrare, path_iesire);
                    exit(EXIT_SUCCESS);
                } else if (pid < 0) {
                    perror("Eroare la fork");
                    continue;
                }
            }
        }
    }

    closedir(dir);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Utilizare: %s <director_intrare> <director_iesire>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *director_intrare = argv[1];
    const char *director_iesire = argv[2];
    proceseazaDirector(director_intrare, director_iesire);

    return EXIT_SUCCESS;
}