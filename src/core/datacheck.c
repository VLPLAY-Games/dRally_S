#include "datacheck.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* The same files and sizes as the game's own check (src/game/___3e1c4h.c). */
typedef struct {
    const char *    name;
    unsigned long   size;
} datacheck_file_t;

static const datacheck_file_t REQUIRED[] = {
    { "ENGINE.BPA",  0x5e879 },
    { "IBFILES.BPA", 0x14fe5 },
    { "MENU.BPA",    0x305aa5 },
    { "MUSICS.BPA",  0x57615f },
    { "TR0.BPA",     0x52597 },
    { "TR1.BPA",     0x4e772 },
    { "TR2.BPA",     0x47ddd },
    { "TR3.BPA",     0x546fb },
    { "TR4.BPA",     0x3f972 },
    { "TR5.BPA",     0x69778 },
    { "TR6.BPA",     0x57bc8 },
    { "TR7.BPA",     0x65e97 },
    { "TR8.BPA",     0x3dd12 },
    { "TR9.BPA",     0x68623 }
};

static const datacheck_file_t OPTIONAL[] = {
    { "SANIM.HAF",   0x1485040 },
    { "ENDANI.HAF",  0x39ed6b },
    { "ENDANI0.HAF", 0x6dc2fc }
};

static int file_size(const char * path, unsigned long * size){

    struct stat st;

    if(stat(path, &st) != 0) return 0;
    *size = (unsigned long)st.st_size;
    return 1;
}

static void add(char * report, size_t report_size, const char * line){

    const size_t used = strlen(report);

    if(used+strlen(line)+2 < report_size){

        strcat(report, line);
        strcat(report, "\n");
    }
}

/* The folder of the animations: CDROM.INI holds its name (./CINEM), without it they are in `dir`. */
static void cinem_dir(const char * dir, char * out, size_t out_size){

    char ini[512];
    char name[256] = {0};
    FILE * fd;

    snprintf(out, out_size, "%s", dir);
    snprintf(ini, sizeof(ini), "%s/CDROM.INI", dir);

    if((fd = fopen(ini, "rb"))){

        if(fscanf(fd, "%255s", name) == 1){

            snprintf(out, out_size, "%s/%s", dir, name);
        }
        fclose(fd);
    }
}

int DataCheck_scan(const char * dir, char * report, size_t report_size){

    char path[768], line[900], cdir[512];
    int problems = 0;
    size_t n;
    unsigned long size;

    report[0] = 0;

    for(n = 0; n < sizeof(REQUIRED)/sizeof(REQUIRED[0]); n++){

        snprintf(path, sizeof(path), "%s/%s", dir, REQUIRED[n].name);

        if(!file_size(path, &size)){

            snprintf(line, sizeof(line), "%s: not found", REQUIRED[n].name);
            add(report, report_size, line);
            problems++;
        }
        else if(size != REQUIRED[n].size){

            snprintf(line, sizeof(line), "%s: not the original file (%lu bytes, expected %lu)", REQUIRED[n].name, size, REQUIRED[n].size);
            add(report, report_size, line);
            problems++;
        }
    }

    cinem_dir(dir, cdir, sizeof(cdir));

    for(n = 0; n < sizeof(OPTIONAL)/sizeof(OPTIONAL[0]); n++){

        snprintf(path, sizeof(path), "%s/%s", cdir, OPTIONAL[n].name);

        if(file_size(path, &size) && (size != OPTIONAL[n].size)){

            snprintf(line, sizeof(line), "%s: not the original file (%lu bytes, expected %lu)", OPTIONAL[n].name, size, OPTIONAL[n].size);
            add(report, report_size, line);
            problems++;
        }
    }

    return problems;
}
