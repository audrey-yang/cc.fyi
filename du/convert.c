#include "convert.h"

double blocks_to_kb(int num_blocks) {
    return num_blocks / 2.;
}

double blocks_to_mb(int num_blocks) {
    return ceil(blocks_to_kb(num_blocks) / 1024.);
}

double blocks_to_gb(int num_blocks) {
    return ceil(blocks_to_mb(num_blocks) / 1024.);
}

double blocks_to_tb(int num_blocks) {
    return ceil(blocks_to_gb(num_blocks) / 1024.);
}

double get_size_in_unit(int num_blocks, enum Unit unit) {
    switch (unit) {
        case BLOCKS:
            return num_blocks;
        case KB:
            return blocks_to_kb(num_blocks);
        case MB:
            return blocks_to_mb(num_blocks);
        case GB:
            return blocks_to_gb(num_blocks);
        case TB:
            return blocks_to_tb(num_blocks);
    }
    return -1;
}

double autosize(int num_blocks, enum Unit* unit) {
    enum Unit units[] = { BLOCKS, KB, MB, GB, TB };
    if (num_blocks == 0) {
        *unit = BLOCKS;
        return 0;
    }
    double cur_size = blocks_to_kb(num_blocks);

    for (int i = 1; i < 5; i++) {
        if (cur_size < 1000) {
            *unit = units[i];
            return cur_size;
        }
        cur_size /= 1024.;
    }

    *unit = TB;
    return cur_size;
}

char display_unit(enum Unit unit) {
    switch (unit) {
        case BLOCKS:
            return 'B';
        case KB:
            return 'K';
        case MB:
            return 'M';
        case GB:
            return 'G';
        case TB:
            return 'T';
    }
    return ' ';
}