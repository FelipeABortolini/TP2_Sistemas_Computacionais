#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 256

#pragma pack(push, 1)
struct file_header {
    char name[MAX_NAME];
    unsigned int file_size;
};
#pragma pack(pop)

void archiveFile(const char *filePaths[], int numFiles, const char *archivePath) {
    int i = 0;
    for(i = 0; i < numFiles; i++){
        FILE *file = fopen(filePaths[i], "rb");
        if (file == NULL) {
            printf("Erro ao abrir o arquivo %s\n", filePaths[i]);
            return;
        }

        FILE *archive = fopen(archivePath, "ab");
        if (archive == NULL) {
            printf("Erro ao abrir o arquivo de arquivamento %s\n", archivePath);
            fclose(file);
            return;
        }

        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        struct file_header header;
        strncpy(header.name, filePaths[i], MAX_NAME);
        header.file_size = (unsigned int)fileSize;

        fwrite(&header, sizeof(struct file_header), 1, archive);

        char *buffer = (char *)malloc(fileSize);
        fread(buffer, fileSize, 1, file);
        fwrite(buffer, fileSize, 1, archive);

        free(buffer);
        fclose(file);
        fclose(archive);
    }
}

void recoverFile(const char *archivePath, const char *fileName) {
    FILE *archive = fopen(archivePath, "rb");
    if (archive == NULL) {
        printf("Erro ao abrir o arquivo de arquivamento %s\n", archivePath);
        return;
    }

    struct file_header header;
    while (fread(&header, sizeof(struct file_header), 1, archive) == 1) {
        if (strcmp(header.name, fileName) == 0) {
            
            char *buffer = (char *)malloc(header.file_size);
            fread(buffer, header.file_size, 1, archive);

            char str[100] = "";
            char *extensao = NULL;
            strcpy(str, fileName);
            
            char *dot = strchr(str, '.');
            
            if (dot != NULL) {
                extensao = dot+sizeof(char);
                *dot = '\0';  // Substitui o ponto e todos os caracteres subsequentes por '\0'
            }

            char newFileName[256]; // ajuste o tamanho conforme necessário
            snprintf(newFileName, sizeof(newFileName), "%s_recovery.%s", str, extensao);

            FILE *recoveredFile = fopen(newFileName, "wb");
            fwrite(buffer, header.file_size, 1, recoveredFile);
            free(buffer);
            fclose(recoveredFile);
            break;
        } else {
            fseek(archive, header.file_size, SEEK_CUR);
        }
    }

    fclose(archive);
}

void listFiles(const char *archivePath) {
    FILE *archive = fopen(archivePath, "rb");
    if (archive == NULL) {
        printf("Erro ao abrir o arquivo de arquivamento %s\n", archivePath);
        return;
    }

    struct file_header header;
    while (fread(&header, sizeof(struct file_header), 1, archive) == 1) {
        printf("Arquivo: %s, Tamanho: %u bytes\n", header.name, header.file_size);
        fseek(archive, header.file_size, SEEK_CUR);
    }

    fclose(archive);
}

int main() {
    const char *filePaths[] = {"file1.txt", "file2.txt", "file3.txt"};
    const char *archivePath = "archive.bin";

    int numFiles = sizeof(filePaths) / sizeof(filePaths[0]);

    archiveFile(filePaths, numFiles, archivePath);

    recoverFile(archivePath, "file1.txt");
    recoverFile(archivePath, "file2.txt");
    recoverFile(archivePath, "file3.txt");

    listFiles(archivePath);

    return 0;
}