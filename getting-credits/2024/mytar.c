#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

bool isZeroBlock(const struct posix_header *header) {
  const unsigned char *bytes = (const unsigned char *)header;
  for (size_t i = 0; i < BLOCK_SIZE; i++) {
    if (bytes[i] != 0) {
      return false;
    }
  }
  return true;
}

void parse_arguments(int argc, char *argv[], char **archive, int *list_files, int *extract_files, char ***files, int *file_count) {
  if (argc < 3) {
    fprintf(stdout, "mytar: need at least one option\n");
    exit(2);
  }
  
  *archive = NULL;
  *list_files = 0;
  *extract_files = 0;
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
    } else if (strcmp(argv[i], "-x") == 0) {
      *extract_files = 1;
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

void extract_file(FILE *tar_file, struct posix_header *header, int size) {
  FILE *out_file = fopen(header->name, "wb");
  if (!out_file) {
    perror("mytar");
    exit(2);
  }
  
  char buffer[BLOCK_SIZE];
  int bytes_to_read = size;
  while (bytes_to_read > 0) {
    int bytes_read = fread(buffer, 1, bytes_to_read < BLOCK_SIZE ? bytes_to_read : BLOCK_SIZE, tar_file);
    fwrite(buffer, 1, bytes_read, out_file);
    bytes_to_read -= bytes_read;
  }
  
  fclose(out_file);
}

void handle_tar(FILE *tar_file, int list_files, int extract_files, char **files, int file_count) {
  struct posix_header header;
  int found_files = 0;
  int *printed_files = (int *)calloc(file_count, sizeof(int));
  int size;  
  ssize_t read_block = fread(&header, 1, BLOCK_SIZE, tar_file);
  while (read_block == BLOCK_SIZE) {
    if (header.name[0] == '\0') {
      break; // End of archive
    }
    
    // Get file size
    sscanf(header.size, "%o", &size);
    
    // Check for supported typeflag (only regular files)
    if (header.typeflag != '0' && header.typeflag != '\0') {
      fprintf(stdout, "mytar: Unsupported header type: %d\n", header.typeflag);
      exit(2);
    }
    
    // Check if file is requested
    int should_process = 0;
    if (file_count > 0) {
      for (int i = 0; i < file_count; i++) {
        if (!printed_files[i] && strcmp(files[i], header.name) == 0) {
          should_process = 1;
          printed_files[i] = 1;
          found_files++;
          break;
        }
      }
    } else {
      should_process = 1; // Always process when no specific files requested
    }
    
    if (list_files && should_process) {
      printf("%s\n", header.name);
    }
    
    if (extract_files && should_process) {
      extract_file(tar_file, &header, size);
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
  
  // Print files not found in the archive
  if (file_count > 0 && found_files < file_count) {
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
  int extract_files;
  char **files;
  int file_count;
  
  parse_arguments(argc, argv, &archive, &list_files, &extract_files, &files, &file_count);
  
  FILE *tar_file = fopen(archive, "rb");
  if (!tar_file) {
    perror("mytar");
    exit(2);
  }
  
  handle_tar(tar_file, list_files, extract_files, files, file_count);
  
  fclose(tar_file);
  return 0;
}

