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


void archiveFile(const char *filePath, const char *archivePath) {
    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo %s\n", filePath);
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
    strncpy(header.name, filePath, MAX_NAME);
    header.file_size = (unsigned int)fileSize;

    fwrite(&header, sizeof(struct file_header), 1, archive);

    char *buffer = (char *)malloc(fileSize);
    fread(buffer, fileSize, 1, file);
    fwrite(buffer, fileSize, 1, archive);

    free(buffer);
    fclose(file);
    fclose(archive);
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
            FILE *recoveredFile = fopen(fileName, "wb");
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

    for (int i = 0; i < 3; i++) {
        archiveFile(filePaths[i], archivePath);
    }

    recoverFile(archivePath, "file2.txt");

    listFiles(archivePath);

    return 0;
}