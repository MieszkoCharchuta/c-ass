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
  
  // move cursor to the end
  fseek(tar_file, 0, SEEK_END);
  
  // move cursor two blocks back
  fseek(tar_file, -2 * BLOCK_SIZE, SEEK_CUR);
  
  // Read second to last block?
  read_block = fread(&header, 1, BLOCK_SIZE, tar_file);
  
  // check if zeroblock
  if (!isZeroBlock(&header)) {
    fseek(tar_file, 0, SEEK_END);
    fseek(tar_file, -1 * BLOCK_SIZE, SEEK_CUR);
    read_block = fread(&header, 1, BLOCK_SIZE, tar_file);
    if (isZeroBlock(&header)) {
      fprintf(stdout, "A lone zero block at 22\n");
    }
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
  
  // Check if we've hit an unexpected EOF
  fseek(tar_file, 0, SEEK_END);
  if (ftell(tar_file) < size) {
    if (header.name[0] == '\0') {
      exit(0);
    } else {
      fprintf(stdout, "mytar: Unexpected EOF in archive\n");
      fprintf(stdout, "mytar: Error is not recoverable: exiting now\n");
      exit(2);
    }
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
