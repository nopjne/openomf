#include "formats/pcx.h"
#include "formats/error.h"
#include "formats/internal/reader.h"
#include "utils/allocator.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static unsigned decode_next_bytes(uint8_t *dest, sd_reader *reader) {
    uint8_t fst_byte = sd_read_ubyte(reader);
    uint8_t repeat = 0;
    if(fst_byte >= 192) {
        repeat = fst_byte - 192;
    }
    if(repeat == 0) {
        dest[0] = fst_byte;
        // printf("Wrote byte %"PRIx8"\n", fst_byte);
        return 1;
    }
    uint8_t snd_byte = sd_read_ubyte(reader);
    for(unsigned i = 0; i < repeat; ++i) {
        dest[i] = snd_byte;
    }
    // printf("Wrote byte %"PRIx8" %"PRIu8" times\n", snd_byte, repeat);
    return repeat;
}

int pcx_load(pcx_file *pcx, const char *filename) {
    memset(pcx, 0, sizeof(*pcx));

    sd_reader *reader = sd_reader_open(filename);
    if(reader == NULL) {
        return SD_FILE_INVALID_TYPE;
    }

    // The header is 128 bytes.
    long filesize = sd_reader_filesize(reader);
    if(filesize <= 128) {
        sd_reader_close(reader);
        return SD_FILE_INVALID_TYPE;
    }

    pcx->manufacturer = sd_read_ubyte(reader);
    pcx->version = sd_read_ubyte(reader);
    pcx->encoding = sd_read_ubyte(reader);
    pcx->bits_per_plane = sd_read_ubyte(reader);

    pcx->window_x_min = sd_read_uword(reader);
    pcx->window_y_min = sd_read_uword(reader);
    pcx->window_x_max = sd_read_uword(reader);
    pcx->window_x_max = sd_read_uword(reader);

    pcx->horz_dpi = sd_read_uword(reader);
    pcx->vert_dpi = sd_read_uword(reader);

    if(sd_read_buf(reader, (void *)pcx->palette, 48) != 1) {
        sd_reader_close(reader);
        return SD_FILE_READ_ERROR;
    }
    pcx->reserved = sd_read_ubyte(reader);
    pcx->color_planes = sd_read_ubyte(reader);

    pcx->bytes_per_plane_line = sd_read_uword(reader);
    pcx->palette_info = sd_read_uword(reader);
    pcx->hor_scr_size = sd_read_uword(reader);
    pcx->ver_scr_size = sd_read_uword(reader);

    if(sd_reader_set(reader, 128) != 1) {
        sd_reader_close(reader);
        return SD_FILE_INVALID_TYPE;
    }

    for(unsigned j = 0; j < 200; ++j) {
        // printf("Start of line %u\n", j);
        for(unsigned i = 0; i < 320;) {
            i += decode_next_bytes(&pcx->image[j][i], reader);
        }
    }

    sd_reader_close(reader);
    return SD_SUCCESS;
}
