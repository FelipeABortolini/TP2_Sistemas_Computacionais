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

int main(int argc, char **argv) {
    // Verifique se temos os argumentos mínimos necessários
    if (argc < 2) {
        printf("Uso:\n");
        printf("\t%s -c <arquivo_de_saida> <arquivo1> <arquivo2> ... <arquivoN> - Cria arquivo.\n", argv[0]);
        printf("\t%s -l <nome_arquivador> - Lista arquivos.\n", argv[0]);
        printf("\t%s -e <nome_arquivador> <arquivo> - Extrai conteúdo.\n", argv[0]);
        return 1;
    }

    // O primeiro argumento é o arquivo de saída ou opção de listagem -l
    char *opt = argv[1];

    // Listar arquivos
    if (strcmp(opt, "-l") == 0) {
        listFiles(argv[2]);
        return 0;
    }

    // Criar arquivo. 
    if (strcmp(opt, "-c") == 0){
        char *output_file = argv[2];
        char **input_files = &argv[3];
        int num_input_files = argc - 3;

        archiveFile(input_files, num_input_files, output_file);
    }

    // Extrair arquivo. 
    if (strcmp(opt, "-e") == 0){
        char *archive_file = argv[2];
        char *extract_file = argv[3];
        recoverFile(archive_file, extract_file);
    }

    return 0;
}
