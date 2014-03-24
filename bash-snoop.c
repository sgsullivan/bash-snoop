/*
* This file is part of bash-snoop.
*
* bash-snoop is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* bash-snoop is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with bash-snoop.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


char cache_dir[100] = "/var/cache/BashSnoop";	
float version = 0.1;

void usage( char * arg ) {
	fprintf( stderr, "usage: %s [PID|-help]\n", arg );
	exit(EXIT_FAILURE);
}

void help( char * arg ) {

	printf("\nBashSnoop  Copyright (C) 2012  Scott Sullivan (scottgregorysullivan@gmail.com)\n\
This program comes with ABSOLUTELY NO WARRANTY; for details refer to the GPLv3\n\
license, in the source of this application. This is free software, and you are\n\
welcome to redistribute it under certain conditions; refer to the GPLv3 for\n\
details.\n\n\
** BashSnoop requires root level permissions **\n\n\
BashSnoop version [%.2f] Help\n\n\
%s -all --- Write in memory history of all currently running bash processes to %s.\n\
%s 1986 --- Write in memory history of the bash process belonging to PID 1986 to %s.\n\
%s -help --- Display this help message.\n\n\
",version,arg,cache_dir,arg,cache_dir,arg); 
	exit(EXIT_SUCCESS);
}

int is_numeric( const char *p ) {

	if ( *p ) {
		char c;
		while (( c = *p++ )) {
			if ( ! isdigit(c)) return 0;
		}
		return 1;
	}
	return 0;
}

void getHistSingleBashPid( char * pid ) {

	struct dirent *pDirent;
	
	struct test {
		char foo[5454];
	};
	struct test testing;
	
	int found = 0;
	// Search /proc
	DIR *pDir;
	pDir = opendir ("/proc");
	if ( pDir == NULL ) {
		fprintf( stderr, "Cannot open directory '%s'\n" );
		exit(EXIT_FAILURE);
	}
	while (( pDirent = readdir(pDir)) != NULL ) {
		// We only want numeric entries (PIDs)..
		if ( is_numeric(pDirent->d_name) ) {
			char buf[2068];
			char name[2068];
			const char *token = "Name:";
			// Concatenate to generate /proc/$PID/status for parsing.
			char baseHandle[] = "/proc/";
			char statusHandle[] = "/status";
			strcat(baseHandle, pDirent->d_name);
			strcat(baseHandle, statusHandle);
			// Open file for reading.
			FILE *status = fopen(baseHandle, "r");
			if ( status != NULL ) {
				// Read 'Name:' field from status file..
				while ( fgets(buf, sizeof(buf), status) ) {
					if ( strncmp(buf, token, strlen(token) ) == 0) {
						sscanf(buf, "%*s %s", &name);
						// If it contains 'bash'..
						char toCompare[] = "bash";
						if ( strstr(name, toCompare) ) {

							// Only snoop passed PID.
							if ( strncmp(pid,pDirent->d_name, 10) == 0 ) {
								char s[500];
								sprintf(s, "gdb --batch --pid %s -ex \"call write_history(\\\"%s/%s.bash_history\\\")\" > /dev/null",pDirent->d_name,cache_dir,pDirent->d_name);
								printf("Writing bash_history for %s to %s/%s.bash_history\n", pDirent->d_name, cache_dir, pDirent->d_name);
								system(s);
								found = 1;
								break;
							}
						}
					}
				}
				fclose(status);
			}
		}
	}
	closedir (pDir);
	if ( found == 0 ) {
		printf("Passed PID [%s] doesn't exist or not a valid bash process!\n", pid);
	}
}

void getHistAllBashPids() {

	struct dirent *pDirent;

	struct test {
		char foo[5454];
	};
	struct test testing;

	// Search /proc
	DIR *pDir;
	pDir = opendir ("/proc");
	if ( pDir == NULL ) {
		fprintf( stderr, "Cannot open directory '%s'\n" );
		exit(EXIT_FAILURE);
	}

	// Count how many bash pids are running
	int numBashProcs = 0;
	while (( pDirent = readdir(pDir)) != NULL ) {
		// We only want numeric entries (PIDs)..
		if ( is_numeric(pDirent->d_name) ) {
			char buf[2068];
			char name[2068];
			const char *token = "Name:";
			// Concatenate to generate /proc/$PID/status for parsing.
			char baseHandle[] = "/proc/";
			char statusHandle[] = "/status";
			strcat(baseHandle, pDirent->d_name);
			strcat(baseHandle, statusHandle);
			// Open file for reading.
			FILE *status = fopen(baseHandle, "r");
			if ( status != NULL ) {
				// Read 'Name:' field from status file..
				while ( fgets(buf, sizeof(buf), status) ) {
					if ( strncmp(buf, token, strlen(token) ) == 0) {
						sscanf(buf, "%*s %s", &name);
							// If it contains 'bash'..
							char toCompare[] = "bash";
							if ( strstr(name, toCompare) ) {
								
								char s[500];
								sprintf(s, "gdb --batch --pid %s -ex \"call write_history(\\\"%s/%s.bash_history\\\")\" > /dev/null",pDirent->d_name,cache_dir,pDirent->d_name);	
								
								printf("Writing bash_history for %s to %s/%s.bash_history\n", pDirent->d_name, cache_dir, pDirent->d_name);
								system(s);

								// Increment numBashProcs
								numBashProcs++;
								break;
							}
					}
				}
				fclose(status);
			}
		}
	}
	closedir (pDir);

	printf("Wrote history for %i bash processes\n", numBashProcs);

}

void init() {

	/*
	   Create cache_dir if it doesn't exist.
	*/
	
	struct stat st = {0};
	if ( stat(cache_dir, &st) == -1 ) {
		mkdir(cache_dir, 0755);
	}

	/* 
	   0777 because when we attach to a users bash process, we have the
	   same permissions as the user we attach to.
	*/

	char mode[] = "0777";
	int i;
	i = strtol(mode, 0, 8);
	if ( chmod (cache_dir,i) < 0 ) {
		fprintf(stderr, "error in chmod(%s, %s) - %d (%s)\n", cache_dir, mode, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
}

void cleanup() {

	/*
	   For security reasons, change cache_dir back to 0755 (see comment 
	   section in init()).
	*/

	char mode[] = "0755";
	int i;
	i = strtol(mode, 0, 8);
	if ( chmod (cache_dir,i) < 0 ) {
		fprintf(stderr, "error in chmod(%s, %s) - %d (%s)\n", cache_dir, mode, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

int main (int argc, char** argv) {

	if ( argc != 2 ) { 
		usage( argv[0] );
	}
	
	char helpStr[] = "-help";
	char all[] = "-all";
	if ( ! strcmp(argv[1],helpStr) ) {
		help( argv[0] );
	}
	else if ( ! strcmp(argv[1],all) ) {
		init();
		getHistAllBashPids();
	}
	else {
		init();
		getHistSingleBashPid(argv[1]);
	}

	cleanup();

	return 0;
}
