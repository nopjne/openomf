#include <stdlib.h>
#include <string.h>

#include "shadowdive/error.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/memreader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/chr.h"

#define UNUSED(x) (void)(x)

int sd_chr_create(sd_chr_file *chr) {
    if(chr == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(chr, 0, sizeof(sd_chr_file));
    return SD_SUCCESS;
}

int sd_chr_load(sd_chr_file *chr, const char *filename) {
    if(chr == NULL ||filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Read up pilot block and the unknown data
    sd_mreader *mr = sd_mreader_open_from_reader(r, 448);
    sd_mreader_xor(mr, 0xAC);
    sd_pilot_create(&chr->pilot);
    sd_pilot_load_from_mem(mr, &chr->pilot);
    sd_mread_buf(mr, chr->unknown, 20);
    sd_mreader_close(mr);

    // Read enemies block
    mr = sd_mreader_open_from_reader(r, 68 * chr->pilot.enemies_inc_unranked);
    sd_mreader_xor(mr, (chr->pilot.enemies_inc_unranked * 68) & 0xFF);

    // Handle enemy data
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        // Reserve & zero out
        chr->enemies[i] = malloc(sizeof(sd_chr_enemy));
        sd_pilot_create(&chr->enemies[i]->pilot);
        sd_pilot_load_player_from_mem(mr, &chr->enemies[i]->pilot);
        sd_mread_buf(mr, chr->enemies[i]->unknown, 25);
    }

    // Close memory reader for enemy data block
    sd_mreader_close(mr);

    // Read HAR palette
    sd_palette_create(&chr->pal);
    sd_palette_load_range(r, &chr->pal, 0, 48);

    // No idea what this is. TODO: Find out.
    sd_skip(r, 4);

    // Load sprite
    chr->photo = malloc(sizeof(sd_sprite));
    sd_sprite_create(chr->photo);
    int ret = sd_sprite_load(r, chr->photo);
    if(ret != SD_SUCCESS) {
        goto error_1;
    }

    // Fix photo size
    chr->photo->width++;
    chr->photo->height++;

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;


error_1:
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        if(chr->enemies[i] != NULL) {
            free(chr->enemies[i]);
        }
    }
    sd_sprite_free(chr->photo);
    sd_reader_close(r);
    return SD_FILE_PARSE_ERROR;
}

int sd_chr_save(sd_chr_file *chr, const char *filename) {
    if(chr == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // TODO

    // Close & return
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_chr_free(sd_chr_file *chr) {
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        if(chr->enemies[i] != NULL) {
            free(chr->enemies[i]);
        }
    }
    sd_sprite_free(chr->photo);
}
