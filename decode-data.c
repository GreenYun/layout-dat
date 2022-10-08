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
  uint32_t field2_start;
  uint32_t field2_end;
  uint32_t length3;
  uint32_t field6;
  uint32_t field7;
};

struct header1 {
  uint32_t magic;
  uint32_t header2_start_at;
  uint32_t group1_count;
};

struct header2 {
  uint32_t field3;
  uint32_t field4;
  uint32_t field5;
  uint32_t field6;
};

int parse_group1(struct group1 *group, FILE *fdata, FILE *fout, int add_comma) {
  fread(group, sizeof(struct group1), 1, fdata);

  if (add_comma)
    fputc(',', fout);

  fprintf(fout,
          "\n"
          "    {\n"
          "      \"field1_start\": %u,\n"
          "      \"field1_end\": %u,\n"
          "      \"field2_start\": %u,\n"
          "      \"field2_end\": %u,\n"
          "      \"length3\": %u,\n"
          "      \"field6\": %u,\n"
          "      \"field7\": %u\n"
          "    }",
          group->field1_start, group->field1_end, group->field2_start,
          group->field2_end, group->length3, group->field6, group->field7);

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
  fseek(fdata, start, SEEK_SET);
  fread(header2, sizeof(struct header2), 1, fdata);

  fprintf(fout,
          "  \"header2\": {\n"
          "    \"field3\": %u,\n"
          "    \"field4\": %u,\n"
          "    \"field5\": %u,\n"
          "    \"field6\": %u\n"
          "  }\n",
          header2->field3, header2->field4, header2->field5, header2->field6);

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

  fprintf(fout, "}");

  fclose(fout);
  fclose(fdata);

  return 0;
}