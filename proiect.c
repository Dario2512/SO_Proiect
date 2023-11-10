#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "time.h"

typedef struct data_t {
    char numeFisier[40];
    int inaltimeImg;
    int latimeImg;
    int dimensiuneImg;
    int idUtilizator;
    char ultimaModificare[11];
    int contorLegaturi;
    char accesUtilizator[4];
    char accesGrup[4];
    char accesAltii[4];
} data_t;

char *formatData(data_t d);
void printData(data_t d);
void inchideFisier(int descriptorFisier);
void obtineDimensiuniImagine(int descriptorFisier, int *inaltime, int *latime, int *dimensiune);
int deschideImagine(char *caleFisier);
void obtineStatisticiImagine(const char *numeFisier, struct stat *buffer);
char* timespecToData(const struct timespec *ts);
void mod(mode_t mod, char *str, char cine);
void obtinePermisiuni(struct stat *statistici, data_t *data);
void completeazaDataDinStat(struct stat *statistici, data_t *data);
void completeazaProprietatiImagine(data_t *data, int descriptorImagine);
void initializeazaData(data_t *data, const char *numeFisier);
void scrieDataInFisier(const char *numeFisier, const char *data);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Utilizare: %s <cale_fisier>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    data_t data;
    struct stat statistici;

    initializeazaData(&data, argv[1]);

    int descriptorImagine = deschideImagine(data.numeFisier);

    completeazaProprietatiImagine(&data, descriptorImagine);
    
    obtineStatisticiImagine(data.numeFisier, &statistici);
    
    completeazaDataDinStat(&statistici, &data);
    
    inchideFisier(descriptorImagine);
    
    char *formattedData = formatData(data);
    
    scrieDataInFisier("statistica.txt", formatData(data));
    
    free(formattedData);

    return EXIT_SUCCESS;
}


char *formatData(data_t d) {
    char *buffer = (char *)malloc(200 * sizeof(char));
    sprintf(buffer, "Nume fisier: %s\nInaltime: %d\nLatime: %d\nDimensiune: %d\nID Utilizator: %d\nUltima modificare: %10s\nContor legaturi: %d\nDrepturi de acces utilizator: %3s\nDrepturi de acces grup: %3s\nDrepturi de acces alti: %3s\n",
        d.numeFisier,
        d.inaltimeImg,
        d.latimeImg,
        d.dimensiuneImg,
        d.idUtilizator,
        d.ultimaModificare,
        d.contorLegaturi,
        d.accesUtilizator,
        d.accesGrup,
        d.accesAltii
    );
    return buffer;
}

void printData(data_t d) { 
    printf("%s", formatData(d));
}

void inchideFisier(int descriptorFisier) {
    if (close(descriptorFisier) < 0) {
        perror("Eroare la inchiderea fisierului: ");
        exit(EXIT_FAILURE);
    }
}

void obtineDimensiuniImagine(int descriptorFisier, int *inaltime, int *latime, int *dimensiune) {
    if(lseek(descriptorFisier, 18, SEEK_SET) < 0) {
        perror("Eroare mutare cursor fisier: ");
        exit(EXIT_FAILURE);
    }
    
    if(read(descriptorFisier, latime, 4) < 0) {
        perror("Eroare citire din fisier: ");
        exit(EXIT_FAILURE);
    }

    if(lseek(descriptorFisier, 22, SEEK_SET) < 0) {
        perror("Eroare mutare cursor fisier: ");
        exit(EXIT_FAILURE);
    }

    if(read(descriptorFisier, inaltime, 4) < 0) {
        perror("Eroare citire din fisier: ");
        exit(EXIT_FAILURE);
    }

    if(lseek(descriptorFisier, 2, SEEK_SET) < 0) {
        perror("Eroare mutare cursor fisier: ");
        exit(EXIT_FAILURE);
    }
    
    if(read(descriptorFisier, dimensiune, 4) < 0) {
        perror("Eroare citire din fisier: ");
        exit(EXIT_FAILURE);
    }
}

int deschideImagine(char *caleFisier) {
    int descriptorImagine = open(caleFisier, O_RDONLY);
    if (descriptorImagine < 0) {
        perror("Eroare la deschiderea fisierului: ");
        exit(EXIT_FAILURE);
    }
    return descriptorImagine;
}

void obtineStatisticiImagine(const char *numeFisier, struct stat *buffer) {
    if(stat(numeFisier, buffer) < 0) {
        perror("Eroare obtinere statistici fisier: ");
        exit(EXIT_FAILURE);
    }
}

char* timespecToData(const struct timespec *ts) {
    char *dataStr = malloc(11); 
    if (dataStr == NULL) {
        perror("malloc");
        return NULL;
    }

    struct tm lt;
    if (localtime_s(&(ts->tv_sec), &lt) == NULL) {
        free(dataStr);
        return NULL;
    }

    if (strftime(dataStr, 11, "%d-%m-%Y", &lt) == 0) {
        fprintf(stderr, "strftime a returnat 0");
        free(dataStr);
        return NULL;
    }

    return dataStr;
}

void mod(mode_t mod, char *str, char cine) {
    switch (cine) {
        case 'u':

            str[0] = (mod & S_IRUSR) ? 'r' : '-';
            str[1] = (mod & S_IWUSR) ? 'w' : '-';
            str[2] = (mod & S_IXUSR) ? 'x' : '-';
            break;

        case 'g':

            str[0] = (mod & S_IRGRP) ? 'r' : '-';
            str[1] = (mod & S_IWGRP) ? 'w' : '-';
            str[2] = (mod & S_IXGRP) ? 'x' : '-';
            break;

        case 'o':

            str[0] = (mod & S_IROTH) ? 'r' : '-';
            str[1] = (mod & S_IWOTH) ? 'w' : '-';
            str[2] = (mod & S_IXOTH) ? 'x' : '-';
            break;

    }
    str[3] = '\0';
}

void obtinePermisiuni(struct stat *statistici, data_t *data) {
    
    mod(statistici->st_mode, data->accesUtilizator, 'u');
    mod(statistici->st_mode, data->accesGrup, 'g');
    mod(statistici->st_mode, data->accesAltii, 'o');
}

void completeazaDataDinStat(struct stat *statistici, data_t *data) {
    
    data->idUtilizator = (int)statistici->st_uid;
    data->contorLegaturi = (int)statistici->st_nlink;
    strcpy(data->ultimaModificare, timespecToData(&statistici->st_mtime));
    obtinePermisiuni(statistici, data);
}

void completeazaProprietatiImagine(data_t *data, int descriptorImagine) {
    
    obtineDimensiuniImagine(descriptorImagine, &data->inaltimeImg, &data->latimeImg, &data->dimensiuneImg);
}

void initializeazaData(data_t *data, const char *numeFisier) {
    if (strlen(numeFisier) >= sizeof(data->numeFisier)) {
        fprintf(stderr, "Nume fisier prea lung.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(data->numeFisier, numeFisier);
}

void scrieDataInFisier(const char *numeFisier, const char *data) {
    
    int fd = open(numeFisier, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    if (fd < 0) {
        perror("Eroare la deschiderea fisierului de statistici: ");
        exit(EXIT_FAILURE);
    }

    if (write(fd, data, strlen(data)) < 0) {
        perror("Eroare la scrierea in fisierul de statistici: ");
        close(fd); 
        exit(EXIT_FAILURE);
    }
    close(fd);
}

