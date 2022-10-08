/*
 * Copyright (c) 2022 GreenYun Organization
 * SPDX-License-Identifier: MIT
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct header {
  uint32_t magic;
  uint32_t count;
  uint32_t offset;
  uint32_t unk2;
  uint32_t unk3;
  uint32_t unk4;
  uint32_t unk5;
  uint32_t unk6;
  uint32_t unk7;
};

struct entry {
  uint32_t zero;
  uint32_t key_layout_name;
  uint32_t key_layout_number;
  uint32_t flags;
  uint32_t locale;
  uint32_t flags2;
  uint32_t key_layout_data_size;
  uint32_t key_layout_data_addr;
  uint32_t data2_size;
  uint32_t data2_addr;
  uint32_t icon_size;
  uint32_t icon_addr;
  uint32_t plist1_size;
  uint32_t plist1_addr;
  uint32_t plist2_size;
  uint32_t plist2_addr;
};

int write_blob(const char *filename, uint32_t size, uint32_t addr, FILE *src) {
  if (size == 0) {
    fprintf(stderr, "no data for %s\n", filename);
    return 0;
  }

  FILE *f = fopen(filename, "wb");
  if (f == NULL)
    return -1;

  char *blob = malloc(size * sizeof(char));
  fseek(src, addr, SEEK_SET);
  fread(blob, sizeof(char), size, src);
  fwrite(blob, sizeof(char), size, f);
  fclose(f);

  return 0;
}

int main() {
  const char *path = "/System/Library/Keyboard "
                     "Layouts/AppleKeyboardLayouts.bundle/Contents/Resources/"
                     "AppleKeyboardLayouts-L.dat";

  FILE *dat = fopen(path, "rb");
  if (dat == NULL) {
    return 1;
  }

  struct header header;
  fread(&header, sizeof header, 1, dat);

  struct entry *entries = malloc(header.count * sizeof(struct entry));
  fseek(dat, header.offset, SEEK_SET);
  fread(entries, sizeof(struct entry), header.count, dat);

  for (unsigned i = 0; i < header.count; i++) {
    struct entry *entry = entries + i;

    char name[256];
    fseek(dat, entry->key_layout_name, SEEK_SET);
    fread(name, sizeof(char), 256, dat);

    char locale[256];
    fseek(dat, entry->locale, SEEK_SET);
    fread(locale, sizeof(char), 256, dat);

    mkdir(name, 0777);

    char filename[256];
    strcpy(filename, name);
    strcat(filename, "/meta.txt");

    FILE *meta = fopen(filename, "w");
    if (meta == NULL) {
      fprintf(stderr, "error for %s\n", filename);
      continue;
    }

    fprintf(meta, "%08x\n%s\n%s\n%08x\n%08x\n", entry->key_layout_number, name,
            locale, entry->flags, entry->flags2);
    fclose(meta);

    strcpy(filename, name);
    strcat(filename, "/data.txt");
    if (write_blob(filename, entry->key_layout_data_size,
                   entry->key_layout_data_addr, dat) != 0) {
      fprintf(stderr, "error for %s\n", filename);
      continue;
    }

    strcpy(filename, name);
    strcat(filename, "/data2.txt");
    if (write_blob(filename, entry->data2_size, entry->data2_addr, dat) != 0) {
      fprintf(stderr, "error for %s\n", filename);
      continue;
    }

    strcpy(filename, name);
    strcat(filename, "/icon.icn");
    if (write_blob(filename, entry->icon_size, entry->icon_addr, dat) != 0) {
      fprintf(stderr, "error for %s\n", filename);
      continue;
    }

    strcpy(filename, name);
    strcat(filename, "/info1.plist");
    if (write_blob(filename, entry->plist1_size, entry->plist1_addr, dat) !=
        0) {
      fprintf(stderr, "error for %s\n", filename);
      continue;
    }

    strcpy(filename, name);
    strcat(filename, "/info2.plist");
    if (write_blob(filename, entry->plist2_size, entry->plist2_addr, dat) !=
        0) {
      fprintf(stderr, "error for %s\n", filename);
      continue;
    }
  }

  fclose(dat);

  return 0;
}