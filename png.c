#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct chunk_header {
    uint32_t length;
    char type[4];
} chunk_header_t;

typedef struct chunk {
    chunk_header_t header;
    char *data;
    struct chunk *next_chunk;
} chunk_t;

typedef struct ihdr {
    uint32_t width;
    uint32_t height;

    uint8_t depth;
    uint8_t color_type;
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;
} ihdr_t;

typedef struct png {
    int width;
    int height;

    chunk_t *first_chunk;
    ihdr_t *ihdr;
} png_t;

#define SWAP_BYTES(word)    ((((word) & 0x000000ff) << 24) | \
                             (((word) & 0x0000ff00) <<  8) | \
                             (((word) & 0x00ff0000) >>  8) | \
                             (((word) & 0xff000000) >> 24))

int png_read_header(FILE *file)
{
    char buffer[8];
    size_t count = fread(&buffer[0], sizeof(char), 8, file);
    if (count != 8) {
        return 0;
    }
    return strncmp(&buffer[0], "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a", 8) == 0;
}

chunk_t *png_read_chunk(FILE *file)
{
    chunk_t *chunk = malloc(sizeof(chunk_t));
    size_t count = fread(&chunk->header, sizeof(chunk_header_t), 1, file);
    if (count != 1) {
        free(chunk);
        return 0;
    }
    chunk->header.length = SWAP_BYTES(chunk->header.length);
    chunk->data = malloc(chunk->header.length);
    count = fread(chunk->data, sizeof(char), chunk->header.length, file);
    if (count != chunk->header.length) {
        free(chunk->data);
        free(chunk);
        return 0;
    }
    uint32_t crc;
    count = fread(&crc, sizeof(uint32_t), 1, file);
    if (count != 1) {
        free(chunk->data);
        free(chunk);
        return 0;
    }
    return chunk;
}

void png_print_chunk(chunk_t *chunk)
{
    printf("chunk type %c%c%c%c, size %d\n",
        chunk->header.type[0], chunk->header.type[1],
        chunk->header.type[2], chunk->header.type[3],
        chunk->header.length);
}

png_t *png_read(FILE *file)
{
    /* Read the header. */
    if (!png_read_header(file)) {
        return 0;
    }

    /* Allocate the png_t structure. */
    png_t *png = malloc(sizeof(png_t));

    /* Loop through the file and read all the chunks. */
    chunk_t **next_chunk_ptr = &png->first_chunk;

    chunk_t *chunk;
    while (chunk = png_read_chunk(file)) {
        /* Link up the singly linked list of chunks. */
        *next_chunk_ptr = chunk;
        next_chunk_ptr = &chunk->next_chunk;

        /* Check for interesting chunks. */
        if (strncmp(&chunk->header.type[0], "IHDR", 4) == 0) {
            png->ihdr = (ihdr_t *)chunk->data;
        } else if (strncmp(&chunk->header.type[0], "IDAT", 4) == 0) {
            printf("data chunk\n");
        }
    }

    /* Extract some basic image properties. */
    png->width = SWAP_BYTES(png->ihdr->width);
    png->height = SWAP_BYTES(png->ihdr->height);

    printf("width %d height %d\n", png->width, png->height);

    /* Return the png_t structure. */
    return png;
}

void main(int argc, char **argv)
{
    FILE *file = fopen(argv[1], "r");

    png_t *png = png_read(file);
    fclose(file);

    if (!png) {
        printf("Invalid png.\n");
        exit(1);
    }

    chunk_t *chunk = png->first_chunk;
    while (chunk) {
        png_print_chunk(chunk);
        chunk = chunk->next_chunk;
    }

    exit(0);
}
