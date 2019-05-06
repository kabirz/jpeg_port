#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../src/jpeglib.h"

void print_help(const char *str)
{
	printf("Usage:%s <-i input> <-o output> <-s wxh> [-g]\n", str);
}
int read_jpg_file(const char *jpgname, char **data, int *w, int *h)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *fp, *out;
	unsigned char *cur;

	// open
	fp = fopen(jpgname, "rb");
	if (fp == NULL) {
		perror("Error:");
		return -1;
	}

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	*w = cinfo.output_width;
	*h = cinfo.output_height;
	printf("w:%d, h:%d\n", *w, *h);
	*data = (unsigned char *)malloc((*w) * (*h) * cinfo.output_components);
	cur = *data;
	while (cinfo.output_scanline < cinfo.image_height) {
		jpeg_read_scanlines(&cinfo, &cur, 1);
		cur += *w * cinfo.output_components;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(fp);
	return 0;
}

int write_jpg(const char *output, char **data, int w, int h, int compents)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *fp;
	unsigned char *cur;

	// open
	fp = fopen(output, "wb");
	if (fp == NULL) {
		perror("Error:");
		return -1;
	}
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);
	cinfo.image_height = h;
	cinfo.image_width = w;
	cinfo.in_color_space = JCS_RGB;
	cinfo.input_components = compents;
	jpeg_set_defaults(&cinfo);
	jpeg_start_compress(&cinfo, TRUE);
	cur = *data;
	while (cinfo.next_scanline < cinfo.image_height) {
		jpeg_write_scanlines(&cinfo, &cur, 1);
		cur += w * compents;
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(fp);
	return 0;
}

int main(int argc, char *const *argv)
{
	int ch;
	int w = 0, h = 0;
	int is_generate_jpg = 0;
	char *buf = NULL;
	char input[64] = { 0 }, output[64] = { 0 };
	char first[16], second[16];
	char *pur;
	FILE *fp;

	while ((ch = getopt(argc, argv, "i:o:s:gh")) != -1) {
		switch (ch) {
		case 'i':
			strncpy(input, (const char *)optarg, 64);
			break;
		case 'o':
			strncpy(output, (const char *)optarg, 64);
			break;
		case 'g':
			is_generate_jpg = 1;
			break;
		case 's':
			pur = strchr(optarg, 'x');
			if (pur == NULL) {
				print_help(argv[0]);
				return 0;
			}
			strncpy(first, optarg, pur - optarg);
			strncpy(second, pur + 1, 16);
			w = atoi(first);
			h = atoi(second);
			break;
		default:
			print_help(argv[0]);
			return 0;
		}
	}
	if (input[0] == 0 || output[0] == 0) {
		print_help(argv[0]);
		return 0;
	}
	buf = (char *)malloc(w * h * 3);
	if (buf == NULL) {
		printf("malloc error\n");
		exit(-1);
	}
	if (is_generate_jpg) {
		if (w * h == 0) {
			print_help(argv[0]);
			return 0;
		}
		fp = fopen(input, "rb");
		if (fp == NULL) {
			perror("Error:");
			exit(-1);
		}
		fread(buf, w * h * 3, 1, fp);
		fclose(fp);
		write_jpg(output, &buf, w, h, 3);
	} else if (!read_jpg_file(input, &buf, &w, &h)) {
		fp = fopen(output, "wb");
		if (fp == NULL) {
			perror("Error:");
			exit(-2);
		}
		fwrite(buf, w * h * 3, 1, fp);
		fclose(fp);
	}
	return 0;
}
