#include <math.h>

enum Unit {
    BLOCKS, // 512b blocks
    KB, // 1024b
    MB, // 1024kb
    GB, // 1024mb
    TB // 1024gb
};

double get_size_in_unit(int size, enum Unit unit);
double autosize(int size, enum Unit* unit);
char display_unit(enum Unit unit);
