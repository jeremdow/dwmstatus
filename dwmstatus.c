#define _BSD_SOURCE
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

int
parse_netdev(unsigned long long int *receivedabs, unsigned long long int *sentabs, char *face)
{
	char *buf;
	char *eth0start;
	static int bufsize;
	FILE *devfd;

	buf = (char *) calloc(255, 1);
	bufsize = 255;
	devfd = fopen("/proc/net/dev", "r");

	// ignore the first two lines of the file
        fgets(buf, bufsize, devfd);
        fgets(buf, bufsize, devfd);

	while (fgets(buf, bufsize, devfd)) {

		if ((eth0start = strstr(buf, face)) != NULL) {

			// With thanks to the conky project at http://conky.sourceforge.net/
			sscanf(eth0start + 6, "%llu  %*d     %*d  %*d  %*d  %*d   %*d        %*d       %llu",\
					receivedabs, sentabs);
			fclose(devfd);
			free(buf);
			return 0;
		}
	}
	fclose(devfd);
	free(buf);
	return 1;
}

int
cache_netdev(unsigned long long int *newrec, unsigned long long int *newsent,
		unsigned long long int *oldrec, unsigned long long int *oldsent,
		time_t *mtime)
{
	FILE *cache;
	struct stat statbuf;
	const char *path = "/tmp/status";

	if (stat(path, &statbuf) == 0) {
		*mtime = statbuf.st_mtime;
		cache = fopen(path, "r+");
		fscanf(cache, "%llu %llu", oldrec, oldsent);
		rewind(cache);
	}
	else {
		oldrec = newrec; oldsent = newsent;
		cache = fopen(path, "w");
	}

	fprintf(cache, "%llu %llu", *newrec, *newsent);
	fclose(cache);

	return 0;
}

char *
get_netusage(char *face)
{
	unsigned long long int oldrec, oldsent, newrec, newsent;
	double downspeed, upspeed;
	char *downspeedstr, *upspeedstr;
	char *retstr;
	int retval;
	time_t mtime;
	int interval;

	downspeedstr = (char *) malloc(15);
	upspeedstr = (char *) malloc(15);
	retstr = (char *) malloc(42);

	retval = parse_netdev(&newrec, &newsent, face);
	if (retval) {
		fprintf(stdout, "Error when parsing /proc/net/dev file.\n");
		exit(1);
	}

	cache_netdev(&newrec, &newsent, &oldrec, &oldsent, &mtime);

	if ((interval = (time(NULL) - mtime)) == 0)
		interval = 1;

	downspeed = (newrec - oldrec) / 1024.0 / interval;
	if (downspeed > 1024.0) {
		downspeed /= 1024.0;
		sprintf(downspeedstr, "%.2fMB/s", downspeed);
	} else {
		sprintf(downspeedstr, "%.0fkB/s", downspeed);
	}

	upspeed = (newsent - oldsent) / 1024.0 / interval;
	if (upspeed > 1024.0) {
		upspeed /= 1024.0;
		sprintf(upspeedstr, "%.2fMB/s", upspeed);
	} else {
		sprintf(upspeedstr, "%.0fkB/s", upspeed);
	}

	sprintf(retstr, "↓%s ↑%s", downspeedstr, upspeedstr);

	free(downspeedstr);
	free(upspeedstr);
	return retstr;
}

/*
char *
loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}
*/

char *
get_freespace(char *mntpt)
{
	struct statvfs data;
	double total, used = 0;

	if ( (statvfs(mntpt, &data)) < 0){
		fprintf(stderr, "can't get info on disk.\n");
		return "?";
	}
	total = data.f_blocks;
	used = (data.f_blocks - data.f_bavail);
	return smprintf("%.0f%%", (used/total*100));
}

char *
get_memusage()
{
	FILE *infile;
	unsigned total, free, avail, buffers, cached;

	infile = fopen("/proc/meminfo","r");
	fscanf(infile, "MemTotal: %u kB\nMemFree: %u kB\nMemAvailable: %u kB\nBuffers: %u kB\nCached: %u kB\n",\
		&total, &free, &avail, &buffers, &cached);
	fclose(infile);

	return smprintf("%0.fMB", (float)(total - free - buffers - cached)/1024);
}

int
main(void)
{
	char *status;
	//char *avgs;
	char *netstats;
	char *mem;
	char *rootfs;

	//avgs = loadavg();
	mem = get_memusage();
	netstats = get_netusage("eth0");
	rootfs = get_freespace("/");

	//status = smprintf("%s %s %s /:%s\n", netstats, avgs, mem, rootfs);
	status = smprintf("%s %s /:%s\n", netstats, mem, rootfs);
	printf(status);

	free(rootfs);
	free(mem);
	free(netstats);
	//free(avgs);
	free(status);

	return 0;
}
