/*
 * Copyright (c) 2022 GreenYun Organization
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct group1 {
  uint32_t field1_start;
  uint32_t field1_end;
  uint32_t field3_start;
  uint32_t field3_end;
  uint32_t length4;
  uint32_t field6;
  uint32_t field7;
};

struct header1 {
  uint32_t magic;
  uint32_t header2_start_at;
  uint32_t group1_count;
};

struct header2 {
  uint32_t magic;
  uint32_t version;
};

struct field3 {
  uint8_t c1;
  uint8_t c2;
  uint8_t c3;
  uint8_t c4;
  uint8_t c5;
  uint8_t c6;
  uint8_t c7;
  uint8_t c8;
};

int parse_group1(struct group1 *group1, FILE *fdata, FILE *fout,
                 int add_comma) {
  fread(group1, sizeof(struct group1), 1, fdata);

  if (add_comma)
    fputc(',', fout);

  fprintf(fout,
          "\n"
          "    {\n"
          "      \"field1_start\": %u,\n"
          "      \"field1_end\": %u,\n"
          "      \"field3_start\": %u,\n"
          "      \"field3_end\": %u,\n"
          "      \"length4\": %u,\n"
          "      \"field6\": %u,\n"
          "      \"field7\": %u\n"
          "    }",
          group1->field1_start, group1->field1_end, group1->field3_start,
          group1->field3_end, group1->length4, group1->field6, group1->field7);

  return 0;
}

int decode_header1(struct header1 *header1, FILE *fdata, FILE *fout) {
  fseek(fdata, 0, SEEK_SET);
  fread(header1, sizeof(struct header1), 1, fdata);

  fprintf(fout,
          "  \"header1\": {\n"
          "    \"magic\": %u,\n"
          "    \"header2_start_at\": %u,\n"
          "    \"group1_count\": %u\n"
          "  },\n",
          header1->magic, header1->header2_start_at, header1->group1_count);

  return 0;
}

int decode_header2(struct header2 *header2, uint32_t start, FILE *fdata,
                   FILE *fout) {
  if (start == 0)
    return 0;

  fseek(fdata, start, SEEK_SET);
  fread(header2, sizeof(struct header2), 1, fdata);

  fprintf(fout,
          "  \"header2\": {\n"
          "    \"magic\": %u,\n"
          "    \"version\": %u\n"
          "  },\n",
          header2->magic, header2->version);

  return 0;
}

int decode_field3(struct field3 *field3, uint32_t start, FILE *fdata,
                  FILE *fout) {
  fseek(fdata, start, SEEK_SET);
  fread(field3, sizeof(struct field3), 1, fdata);

  fprintf(fout,
          "  \"field3\": [\n"
          "    %d, %d, %d, %d, %d, %d, %d, %d\n"
          "  ]\n",
          field3->c1, field3->c2, field3->c3, field3->c4, field3->c5,
          field3->c6, field3->c7, field3->c8);

  return 0;
}

int main(int argc, const char *argv[]) {
  if (argc != 2)
    return 1;

  FILE *fdata = fopen(argv[1], "rb");
  if (fdata == NULL)
    return 1;

  char *out_fname = malloc(strlen(argv[1]) + 6);
  strcpy(out_fname, argv[1]);
  strcat(out_fname, ".json");
  FILE *fout = fopen(out_fname, "w");

  fprintf(fout, "{\n");

  /*
   * header1
   */
  struct header1 header1;
  decode_header1(&header1, fdata, fout);

  /*
   * group1
   */
  struct group1 group1;

  fprintf(fout, "  \"group1\": [");

  for (unsigned i = 0; i < header1.group1_count; i++)
    parse_group1(&group1, fdata, fout, i);

  fprintf(fout, "\n"
                "  ],\n");

  /*
   * header2
   */
  struct header2 header2;
  decode_header2(&header2, header1.header2_start_at, fdata, fout);

  /*
   * field3
   */
  struct field3 field3;
  decode_field3(&field3, group1.field3_start, fdata, fout);

  fprintf(fout, "}");

  fclose(fout);
  fclose(fdata);

  return 0;
}