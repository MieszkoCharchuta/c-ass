#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512

struct posix_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
};

void parse_arguments(int argc, char *argv[], char **archive, int *list_files, char ***files, int *file_count) {
    if (argc < 3) {
        fprintf(stdout, "mytar: need at least one option\n");
        exit(2);
    }

    *archive = NULL;
    *list_files = 0;
    *file_count = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) {
                *archive = argv[++i];
            } else {
                fprintf(stdout, "mytar: option requires an argument -- 'f'\n");
                exit(2);
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            *list_files = 1;
        } else {
            *file_count = argc - i;
            *files = &argv[i];
            break;
        }
    }

    if (*archive == NULL) {
        fprintf(stdout, "mytar: need at least one option\n");
        exit(2);
    }
}

void handle_tar(FILE *tar_file, int list_files, char **files, int file_count) {
    struct posix_header header;
    int found_files = 0;
    int *printed_files = (int *)calloc(file_count, sizeof(int));
    int zero_block_count = 0;
    long last_valid_block_pos = 0;

    ssize_t read_block = fread(&header, 1, BLOCK_SIZE, tar_file);
    while (read_block == BLOCK_SIZE) {
        if (header.name[0] == '\0') {
            zero_block_count++;
            if (zero_block_count == 2) {
                break; // Valid end of archive
            }
        } else {
            zero_block_count = 0; // Reset zero block count as we have valid data
            last_valid_block_pos = ftell(tar_file) - BLOCK_SIZE;
        }

        // Get file size
        int size;
        sscanf(header.size, "%o", &size);

        // Check for supported typeflag (only regular files)
        if (header.typeflag != '0' && header.typeflag != '\0') {
            fprintf(stdout, "mytar: Unsupported header type: %d\n", header.typeflag);
            exit(2);
        }

        // Check if file is requested
        int should_print = 0;
        if (list_files && file_count > 0) {
            for (int i = 0; i < file_count; i++) {
                if (!printed_files[i] && strcmp(files[i], header.name) == 0) {
                    should_print = 1;
                    printed_files[i] = 1;
                    found_files++;
                    break;
                }
            }
        } else {
            should_print = 1; // Always print when listing all files or no specific files requested
        }

        if (should_print) {
            printf("%s\n", header.name);
        }

        // Seek to next header
        int offset = (size + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;
        if (fseek(tar_file, offset, SEEK_CUR) != 0) {
            fprintf(stdout, "mytar: Unexpected EOF in archive\n");
            fprintf(stdout, "mytar: Error is not recoverable: exiting now\n");
            exit(2);
        }

        read_block = fread(&header, 1, BLOCK_SIZE, tar_file);
    }

    // Handle cases where we might have missed the second zero block
    if (zero_block_count == 1) {
        fprintf(stdout, "mytar: A lone zero block at %ld\n", last_valid_block_pos / BLOCK_SIZE);
    } else if (zero_block_count == 0) {
        fprintf(stdout, "mytar: Unexpected EOF in archive\n");
        fprintf(stdout, "mytar: Error is not recoverable: exiting now\n");
        exit(2);
    }

    // Print files not found in the archive
    if (list_files && file_count > 0 && found_files < file_count) {
        for (int i = 0; i < file_count; i++) {
            if (!printed_files[i]) {
                fprintf(stdout, "mytar: %s: Not found in archive\n", files[i]);
            }
        }
        fprintf(stdout, "mytar: Exiting with failure status due to previous errors\n");
        exit(2);
    }

    free(printed_files);
}


int main(int argc, char *argv[]) {
    char *archive;
    int list_files;
    char **files;
    int file_count;

    parse_arguments(argc, argv, &archive, &list_files, &files, &file_count);

    FILE *tar_file = fopen(archive, "rb");
    if (!tar_file) {
        perror("mytar");
        exit(2);
    }

    handle_tar(tar_file, list_files, files, file_count);

    fclose(tar_file);
    return 0;
}
