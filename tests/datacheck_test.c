/* Host test of the game file check. Build and run: tests/run.sh */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/datacheck.h"

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while(0)

static char dir[256];

static void make_file(const char * sub, const char * name, unsigned long size){

    char path[512];
    FILE * f;

    snprintf(path, sizeof(path), "%s/%s", dir, sub);
    if(sub[0]) mkdir(path, 0755);
    snprintf(path, sizeof(path), "%s/%s%s%s", dir, sub, sub[0] ? "/" : "", name);
    f = fopen(path, "wb");
    fseek(f, (long)size-1, SEEK_SET);   /* a sparse file of that size */
    fputc(0, f);
    fclose(f);
}

static const struct { const char * name; unsigned long size; } REQ[] = {
    {"ENGINE.BPA",0x5e879},{"IBFILES.BPA",0x14fe5},{"MENU.BPA",0x305aa5},{"MUSICS.BPA",0x57615f},
    {"TR0.BPA",0x52597},{"TR1.BPA",0x4e772},{"TR2.BPA",0x47ddd},{"TR3.BPA",0x546fb},{"TR4.BPA",0x3f972},
    {"TR5.BPA",0x69778},{"TR6.BPA",0x57bc8},{"TR7.BPA",0x65e97},{"TR8.BPA",0x3dd12},{"TR9.BPA",0x68623}
};

int main(void){

    char report[4096], cmd[300];
    size_t i;

    snprintf(dir, sizeof(dir), "/tmp/drally_datacheck_XXXXXX");
    if(!mkdtemp(dir)){ printf("mkdtemp failed\n"); return 1; }

    /* an empty folder: every required file is reported */
    CHECK(DataCheck_scan(dir, report, sizeof(report)) == 14);
    CHECK(strstr(report, "MENU.BPA: not found") != NULL);

    /* the original files: nothing to report, the animations are optional */
    for(i = 0; i < sizeof(REQ)/sizeof(REQ[0]); i++) make_file("", REQ[i].name, REQ[i].size);
    CHECK(DataCheck_scan(dir, report, sizeof(report)) == 0);
    CHECK(report[0] == 0);

    /* a file of the wrong size (another version, a damaged copy) */
    make_file("", "TR3.BPA", 12345);
    CHECK(DataCheck_scan(dir, report, sizeof(report)) == 1);
    CHECK(strstr(report, "TR3.BPA: not the original file (12345 bytes, expected 345851)") != NULL);
    make_file("", "TR3.BPA", 0x546fb);

    /* animations in CINEM/, named by CDROM.INI: fine when original, reported when not */
    {
        char ini[300];
        FILE * f;

        snprintf(ini, sizeof(ini), "%s/CDROM.INI", dir);
        f = fopen(ini, "wb"); fputs("./CINEM\n", f); fclose(f);
    }
    make_file("CINEM", "SANIM.HAF", 0x1485040);
    CHECK(DataCheck_scan(dir, report, sizeof(report)) == 0);
    make_file("CINEM", "SANIM.HAF", 1000);
    CHECK(DataCheck_scan(dir, report, sizeof(report)) == 1);
    CHECK(strstr(report, "SANIM.HAF") != NULL);

    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    if(system(cmd) != 0) printf("could not remove %s\n", dir);

    if(failures) printf("%d check(s) failed\n", failures);
    else printf("datacheck: all checks passed\n");
    return failures != 0;
}
