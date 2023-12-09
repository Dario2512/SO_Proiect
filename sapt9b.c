#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdint.h>
#include <libgen.h>
 
#define PATH_MAX 4096
 
int pipeWE;
int pipeFd[2];

 
int lines;
 
typedef struct {
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
    uint32_t headerSize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerM;
    int32_t yPixelsPerM;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} BMPsizestats;
 
void readFormat(int inputFileDescriptor, BMPsizestats *header) {
    char sig[2];
    if (read(inputFileDescriptor, sig, 2) != 2 || sig[0] != 'B' || sig[1] != 'M') {
        perror("Not a bmp");
        exit(-1);
    }
 
    if (read(inputFileDescriptor, header, sizeof(BMPsizestats)) != sizeof(*header)) {
        perror("Cannot read the file");
        exit(-1);
    }
}
 
void convert_to_grayscale(const char *input_path) {
    int input_file = open(input_path, O_RDWR);
    if (input_file == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
 
    char header[54];
    if (read(input_file, header, sizeof(header)) != sizeof(header)) {
        perror("read");
        close(input_file);
        exit(EXIT_FAILURE);
    }
 
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int size = width * height * 3;
 
    unsigned char *original_image = (unsigned char*)malloc(size);
    if (original_image == NULL) {
        perror("malloc");
        close(input_file);
        exit(EXIT_FAILURE);
    }
 
    if (read(input_file, original_image, size) != size) {
        perror("read");
        free(original_image);
        close(input_file);
        exit(EXIT_FAILURE);
    }
 
    for (int i = 0; i < size; i += 3) {
        unsigned char blue = original_image[i];
        unsigned char green = original_image[i + 1];
        unsigned char red = original_image[i + 2];
 
        unsigned char grayscale = (unsigned char)(0.299 * red + 0.587 * green + 0.114 * blue);
 
        original_image[i] = original_image[i + 1] = original_image[i + 2] = grayscale;
    }
 
    lseek(input_file, sizeof(header), SEEK_SET);
    if (write(input_file, original_image, size) != size) {
        perror("write");
        free(original_image);
        close(input_file);
        exit(EXIT_FAILURE);
    }
 
    free(original_image);
    close(input_file);
}

void process_bmp_file(const char *input_full_path, const char *output_dir) {
    char input_path_copy[PATH_MAX];
    strcpy(input_path_copy, input_full_path);

    int input_file = open(input_full_path, O_RDWR);
    if (input_file == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char header[54];
    if (read(input_file, header, sizeof(header)) != sizeof(header)) {
        perror("read");
        close(input_file);
        exit(EXIT_FAILURE);
    }

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int size = width * height * 3;

   
    struct stat file_stat;
    if (stat(input_full_path, &file_stat) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    char stat_file_path[PATH_MAX];
    snprintf(stat_file_path, sizeof(stat_file_path), "%s/%s_statistica.txt", output_dir, basename(input_path_copy));

    int stat_file = open(stat_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (stat_file == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char sbuffer[500];

    sprintf(sbuffer, "Nume fisier: %s\n", basename(input_path_copy ));
    write(stat_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Inaltime: %d\n", height);
    write(stat_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Latime: %d\n", width);
    write(stat_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Dimensiune: %ld\n", file_stat.st_size);
    write(stat_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Identificator utilizator: %d\n", file_stat.st_uid);
    write(stat_file, sbuffer, strlen(sbuffer));

    struct tm *tm_info;
    tm_info = localtime(&file_stat.st_mtime);
    strftime(sbuffer, sizeof(sbuffer), "Timpul ultimei modificari: %Y-%m-%d %H:%M:%S\n", tm_info);
    write(stat_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Contorul de legaturi: %ld\n", file_stat.st_nlink);
    write(stat_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Drepturi de acces user: ");
    strcat(sbuffer, ((file_stat.st_mode & S_IRUSR)) ? "R" : "-");
    strcat(sbuffer, ((file_stat.st_mode & S_IWUSR)) ? "W" : "-");
    strcat(sbuffer, ((file_stat.st_mode & S_IXUSR)) ? "X" : "-");
    strcat(sbuffer, "\n");
    write(stat_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Drepturi de acces grup: ");
    strcat(sbuffer, ((file_stat.st_mode & S_IRGRP)) ? "R" : "-");
    strcat(sbuffer, ((file_stat.st_mode & S_IWGRP)) ? "W" : "-");
    strcat(sbuffer, ((file_stat.st_mode & S_IXGRP)) ? "X" : "-");
    strcat(sbuffer, "\n");
    write(stat_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Drepturi de acces altii: ");
    strcat(sbuffer, ((file_stat.st_mode & S_IROTH)) ? "R" : "-");
    strcat(sbuffer, ((file_stat.st_mode & S_IWOTH)) ? "W" : "-");
    strcat(sbuffer, ((file_stat.st_mode & S_IXOTH)) ? "X" : "-");
    strcat(sbuffer, "\n");
    write(stat_file, sbuffer, strlen(sbuffer));

    close(stat_file);
}

void count_valid_sentences(int input_pipe) {
    char buffer[PATH_MAX];
    ssize_t bytes_read;
    int total_sentences = 0;

    while ((bytes_read = read(input_pipe, buffer, sizeof(buffer))) > 0) {
        buffer[bytes_read] = '\0';

        int current_sentences;
        if (sscanf(buffer, "%d", &current_sentences) == 1) {
            total_sentences += current_sentences;
        }
    }

    printf("Au fost identificate in total %d propozitii corecte.\n", total_sentences);
}

void process_regular_file(const char *input_full_path, const struct stat *buffer, const char *entry_name, const char *output_dir) {
    char output_file_path[PATH_MAX];
    snprintf(output_file_path, sizeof(output_file_path), "%s/%s_statistica.txt", output_dir, entry_name);

    int output_file = open(output_file_path, O_WRONLY | O_CREAT | O_TRUNC);
    if (output_file == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    int input_file;
    if ((input_file = open(input_full_path, O_RDONLY)) < 0) {
        perror("cannot open file");
        exit(-1);
    }

    char sbuffer[500];

    sprintf(sbuffer, "Nume fisier: %s\n", entry_name);
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Dimensiune fisier: %ld\n", buffer->st_size);
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "No of hard links: %ld\n", buffer->st_nlink);
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "User ID of owner: %d\n", buffer->st_uid);
    write(output_file, sbuffer, strlen(sbuffer));

    struct tm *tm_info;
    tm_info = localtime(&buffer->st_mtime);
    strftime(sbuffer, sizeof(sbuffer), "Most recent modify time: %Y-%m-%d %H:%M:%S\n", tm_info);
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "User access rights: ");
    strcat(sbuffer, ((buffer->st_mode & S_IRUSR)) ? "R" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IWUSR)) ? "W" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IXUSR)) ? "X" : "-");
    strcat(sbuffer, "\n");
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Group access rights: ");
    strcat(sbuffer, ((buffer->st_mode & S_IRGRP)) ? "R" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IWGRP)) ? "W" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IXGRP)) ? "X" : "-");
    strcat(sbuffer, "\n");
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Other access rights: ");
    strcat(sbuffer, ((buffer->st_mode & S_IROTH)) ? "R" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IWOTH)) ? "W" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IXOTH)) ? "X" : "-");
    strcat(sbuffer, "\n");
    write(output_file, sbuffer, strlen(sbuffer));

    close(output_file);
}

void process_symbolic_link(const char *input_full_path, const struct stat *buffer, const char *entry_name, const char *output_dir) {
    char output_file_path[PATH_MAX];
    snprintf(output_file_path, sizeof(output_file_path), "%s/%s_statistica.txt", output_dir, entry_name);

    int output_file = open(output_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_file == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char sbuffer[500];

    sprintf(sbuffer, "Nume legatura: %s\n", entry_name);
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Dimensiune legatura: %ld\n", buffer->st_size);
    write(output_file, sbuffer, strlen(sbuffer));

    struct stat target_buffer;
    if (stat(input_full_path, &target_buffer) == 0) {
        sprintf(sbuffer, "Dimensiune fisier target: %ld\n", target_buffer.st_size);
        write(output_file, sbuffer, strlen(sbuffer));
    }

    sprintf(sbuffer, "Drepturi de acces user legatura: %s\n", ((buffer->st_mode & S_IRUSR) ? "R" : "-"));
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Drepturi de acces grup legatura: %s\n", ((buffer->st_mode & S_IRGRP) ? "R" : "-"));
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Drepturi de acces altii legatura: %s\n", ((buffer->st_mode & S_IROTH) ? "R" : "-"));
    write(output_file, sbuffer, strlen(sbuffer));

    close(output_file);
}

void process_directory(const char *input_full_path, const struct stat *buffer, const char *entry_name, const char *output_dir) {
    char output_file_path[PATH_MAX];
    snprintf(output_file_path, sizeof(output_file_path), "%s/%s_statistica.txt", output_dir, entry_name);

    int output_file = open(output_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_file == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char sbuffer[500];

    sprintf(sbuffer, "Nume director: %s\n", entry_name);
    write(output_file, sbuffer, strlen(sbuffer));

    struct passwd *user_info = getpwuid(buffer->st_uid);
    if (user_info != NULL) {
        sprintf(sbuffer, "Identificatorul utilizatorului: %s\n", user_info->pw_name);
        write(output_file, sbuffer, strlen(sbuffer));
    }

    sprintf(sbuffer, "User access rights: ");
    strcat(sbuffer, ((buffer->st_mode & S_IRUSR)) ? "R" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IWUSR)) ? "W" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IXUSR)) ? "X" : "-");
    strcat(sbuffer, "\n");
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Group access rights: ");
    strcat(sbuffer, ((buffer->st_mode & S_IRGRP)) ? "R" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IWGRP)) ? "W" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IXGRP)) ? "X" : "-");
    strcat(sbuffer, "\n");
    write(output_file, sbuffer, strlen(sbuffer));

    sprintf(sbuffer, "Other access rights: ");
    strcat(sbuffer, ((buffer->st_mode & S_IROTH)) ? "R" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IWOTH)) ? "W" : "-");
    strcat(sbuffer, ((buffer->st_mode & S_IXOTH)) ? "X" : "-");
    strcat(sbuffer, "\n");
    write(output_file, sbuffer, strlen(sbuffer));

    close(output_file);
}
/*
void process_entry(const char *input_path, const struct dirent *entry, const char *output_dir, const char *character, int pipeFd[2]) {
    char input_full_path[PATH_MAX];
    int status;
    snprintf(input_full_path, sizeof(input_full_path), "%s/%s", input_path, entry->d_name);
    
    struct stat buffer;
    if (lstat(input_full_path, &buffer) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }
 
    if (S_ISREG(buffer.st_mode)) {
        if (strlen(entry->d_name) > 4 && strcmp(entry->d_name + strlen(entry->d_name) - 4, ".bmp") == 0) {
            pid_t bmp_child_pid = fork();
            if (bmp_child_pid == 0) {
                convert_to_grayscale(input_full_path);
                process_bmp_file(input_full_path, output_dir);
                exit(EXIT_SUCCESS);
            } else if (bmp_child_pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            waitpid(bmp_child_pid, &status, 0);
            printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", bmp_child_pid, WEXITSTATUS(status));
        } else {
            pid_t child_pid = fork();
            if (child_pid == 0) {
                char cmd[100];
                snprintf(cmd, sizeof(cmd), "bash script.sh %c", character[0]);
                FILE *script_output = popen(cmd, "r");
                if (script_output == NULL) {
                    perror("popen");
                    exit(EXIT_FAILURE);
                }

                char line[256];
                int local_correct_sentences = 0;
                while (fgets(line, sizeof(line), script_output)) {
                    local_correct_sentences++;
                }

                pclose(script_output);

                close(pipeFd[0]);
                write(pipeFd[1], &local_correct_sentences, sizeof(int));
                close(pipeFd[1]);

                exit(EXIT_SUCCESS);
            } else if (child_pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else {
                waitpid(child_pid, &status, 0);
                if (WIFEXITED(status)) {
                    int local_result;
                    close(pipeFd[1]);
                    read(pipeFd[0], &local_result, sizeof(int));
                    close(pipeFd[0]);

                    printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", child_pid, WEXITSTATUS(status));

                    int local_correct_sentences = local_result; 
                    
                }
            }
        }
    } else if (S_ISLNK(buffer.st_mode)) {
        pid_t bmp_child_pid = fork();
        if (bmp_child_pid == 0) {
            process_symbolic_link(input_full_path, &buffer, entry->d_name, output_dir);
            exit(EXIT_SUCCESS);
        } else if (bmp_child_pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        waitpid(bmp_child_pid, &status, 0);
        printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", bmp_child_pid, WEXITSTATUS(status));
    } else if (S_ISDIR(buffer.st_mode)) {
        pid_t bmp_child_pid = fork();
        if (bmp_child_pid == 0) {
            process_directory(input_full_path, &buffer, entry->d_name, output_dir);
            exit(EXIT_SUCCESS);
        } else if (bmp_child_pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        waitpid(bmp_child_pid, &status, 0);
        printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", bmp_child_pid, WEXITSTATUS(status));
    }
}
*/

int main(int argc, char *argv[]) {
if (argc != 4) {
        fprintf(stderr, "Usage: %s <director_intrare> <director_iesire> <caracter>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_dir = argv[1];
    const char *output_dir = argv[2];
    const char *character = argv[3];

    DIR *dir = opendir(input_dir);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    int pipeFd[2];
    if (pipe(pipeFd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            process_entry(input_dir, entry, output_dir, character, pipeFd);
        }
    }

    closedir(dir);

    close(pipeFd[0]);
    close(pipeFd[1]);

    return 0;
}