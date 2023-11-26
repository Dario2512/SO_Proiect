/* void proceseazaDirector(const char *director_intrare, const char *director_iesire, char c) {
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
                perror("Eroare la obtinerea informatilor despre intrare");
                continue;
            }

            uint32_t idUtilizator = info.st_uid;
            int isSymbolicLink = S_ISLNK(info.st_mode);

            if (S_ISREG(info.st_mode)) {
                if (strstr(intrare->d_name, ".bmp") != NULL) {
                    char path_iesire_bmp[512];
                    sprintf(path_iesire_bmp, "%s/%s", director_iesire, intrare->d_name);
                    pid_t pid = fork();
                    if (pid == 0) {
                        conversieGriBMP(path_intrare, path_iesire_bmp);
                        exit(EXIT_SUCCESS);
                    } else if (pid < 0) {
                        perror("Eroare la fork");
                        continue;
                    }

                    struct stat bmp_info;
                    if (lstat(path_intrare, &bmp_info) == -1) {
                        perror("Eroare la obținerea informatiilor despre fișierul BMP");
                        continue;
                    }
                    scrieStatistici(path_iesire_bmp, bmp_info, idUtilizator, isSymbolicLink, director_iesire);
                } else {
                    FILE *file = fopen(path_intrare, "r");
                    if (file == NULL) {
                        perror("Eroare la deschiderea fisierului");
                        continue;
                    }

                    int found = 0;
                    char ch;
                    while ((ch = fgetc(file)) != EOF) {
                        if (ch == c) {
                            found = 1;
                            break;
                        }
                    }
                    fclose(file);

                    if (found) {
                        pid_t pid = fork();
                        if (pid == 0) {
                            scrieStatistici(path_intrare, info, idUtilizator, isSymbolicLink, director_iesire);
                            exit(EXIT_SUCCESS);
                        } else if (pid < 0) {
                            perror("Eroare la fork");
                            continue;
                        }
                    }
                }
            } else if (S_ISDIR(info.st_mode)) {
                char path_iesire[512];
                sprintf(path_iesire, "%s/%s", director_iesire, intrare->d_name);
                mkdir(path_iesire, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                pid_t pid = fork();
                if (pid == 0) {
                    proceseazaDirector(path_intrare, path_iesire, c);
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
    if (argc != 4) {
        fprintf(stderr, "Utilizare: %s <director_intrare> <director_iesire> <caracter>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *director_intrare = argv[1];
    const char *director_iesire = argv[2];
    char caracter = argv[3][0];

    proceseazaDirector(director_intrare, director_iesire, caracter);

    return EXIT_SUCCESS;
}

*/
