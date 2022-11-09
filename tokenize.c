/**
 * file: tokenize.c
 *
 * build finite state automata given transition matrix and process tokens from
 * input file using created automata.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes.h"

/**
 * Converts string representation of integer to integer,
 * ignoring trailing invalid characters
 *
 * @param str string to convert
 * @return -1 if string does not begin with 0-9 character, inclusive
 *         the string as an integer, ignoring trailing invalid characters
 */
int strInt(char *str) {
    int res = 0, i = 0;
    if (!(str[i] >= 48 && str[i] <= 57)) return -1;
    while (str[i] >= 48 && str[i] <= 57) {
        res = res * 10 + (str[i] - '0');
        i++;
    }
    return res;
}

/**
 * Return character class of character based on classes.h
 *
 * @param c character to lookup
 * @return corresponding character class, an integer
 */
int charClass(char c) {
    if (c == 9 || c == 32) {
	return CC_WS;
    } else if (c == 10) {
	return CC_NEWLINE;
    } else if ((c >= 65 && c <= 90) || c == 95 || (c >= 97 && c <= 122)) {
	return CC_ALPHA;
    } else if (c == 48) {
	return CC_DIG_0;
    } else if (c >= 49 && c <= 55) {
	return CC_DIG_1_7;
    } else if (c == 56 || c == 57) {
	return CC_DIG_8_9;
    } else if (c == 47) {
	return CC_CHAR_SLASH;
    } else if (c == 42) {
	return CC_CHAR_STAR;
    } else if (c == 37 || c == 43 || c == 45) {
	return CC_ARITH_OP;
    } else if (c == -1) {
	return CC_EOF;
    } else {
	return CC_OTHER;
    }
    
    return CC_ERROR;
}

/**
 * Structure for a transition of Finite State Machine.
 */
struct transition {
    int cc;
    int dest;
    char action;
} ;

/**
 * Structure for a state of Finite State Machine.
 */
struct state {
    int state;
    struct transition t[12];
} ;

/**
 * Read first three lines from transition matrix file
 *
 * @param filename transition matrix file to read
 * @param info array to store total, start, and accept state
 */
void readFile(char *filename, int *info) {
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
	perror(filename);
	exit(EXIT_FAILURE);
    }
    
    char buf[16];
    for (int n = 0; n < 3; n++) {
	fgets(buf, 16, fp);
	char delim[] = " ";
	strtok(buf, delim);
	info[n] = strInt(strtok(NULL, delim));
    }

    fclose(fp);
}

/**
 * Build Finite State Machine from transition matrix file.
 *
 * @param filename transition matrix file to read
 * @param machine the finite state machine, an array of states
 * @param states the total number of states
 */
void buildMachine(char *filename, struct state *machine, int states) {
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
	perror(filename);
	exit(EXIT_FAILURE);
    }

    for (int i = 0; i < states; i++) {
	machine[i].state = i;
        for (int j = 0; j < 12; j++) {
            machine[i].t[j].cc = j;
	    machine[i].t[j].dest = 99;
	    machine[i].t[j].action = 'd';
	}
    }

    char *ptr, buf[256], buff[525];
    fgets(buf, 256, fp);
    fgets(buf, 256, fp);
    fgets(buf, 256, fp);
    int b = 0, i = 0;
    while ((ptr = fgets(buf, 256, fp)) != NULL) {
	int flag = 0, idx = 0;
	while (buf[idx]) {
	    if (buf[idx] == ' ') {
	        if (!b) {
		    buff[i] = buf[idx];
		    b = 1;
		    i++;
		}
	    } else {
		buff[i] = buf[idx];
		b = 0;
		i++;
	    }
	    if (buf[idx] == '\n') {
		flag = 1;
		buff[i] = '\0';
	    }
	    idx++;
	}
	if (flag) {
	    int state;
	    char delim[] = " ";
	    char *str = strtok(buff, delim);
            state = strInt(str);
	    while ((str = strtok(NULL, delim)) != NULL) {
	        char action;
	        int cc, dest;
	        sscanf(str, "%d/%d%c", &cc, &dest, &action);
	        machine[state].state = state;
	        machine[state].t[cc].cc = cc;
                machine[state].t[cc].dest = dest;
                machine[state].t[cc].action = action;
	    }
	    i = 0;
	}
    }

    fclose(fp);
}

/**
 * Display transition matrix
 *
 * @param machine the finite state machine, an array of states
 * @param states the total number of states
 */
void displayMachine(struct state *machine, int states) {
    printf("Scanning using the following matrix:\n");
    printf(" ");
    for (int i = 0; i < 12; i++) {
        printf("%5d", i);
    }
    printf("\n");

    for (int i = 0; i < states; i++) {
        printf("%2d", machine[i].state);
	for (int j = 0; j < 12; j++) {
	    printf("%4d%c", machine[i].t[j].dest, machine[i].t[j].action);
	}
	printf("\n");
    }
}

/**
 * Process tokens from input using machine by reading character by character
 * 
 * @param machine the finite state machine, an array of states
 * @param start the starting state
 * @param accept the accepting state
 */
void runMachine(struct state *machine, int start, int accept) {
    char c;
    int i = 0, errorState = 0, state = start;
    char buf[64];
    while (1) {
	c = getchar();
	if (errorState) {
	    int cc = charClass(c);
	    if (cc == CC_WS || cc == CC_NEWLINE) {
		errorState = 0;
		i = 0;
		state = start;
	    }
	    if (cc == CC_EOF) {
		errorState = 0;
		i = 0;
		state = start;
		break;
	    }
	    continue;
	}

	if (i == 0) {
	    printf("%d ", state);
	}
	if (machine[state].t[charClass(c)].action == 's') {
	    buf[i] = c;
	    i++;
	}
	state = machine[state].t[charClass(c)].dest;
	printf("%d ", state);
	
	if (charClass(c) == CC_EOF) {
	    printf("EOF\n");
	    break;
	}
	if (state == accept) {
	    buf[i] = '\0';
	    printf("recognized '%s'\n", buf);
	    i = 0;
	    state = start;
	} else if (state == 99) {
	    printf("rejected\n");
	    errorState = 1;
	}
    }
}

/**
 * Entry for tokenize program. Build finite state machine and display
 * transition matrix being used. Process tokens using machine
 *
 * @param argc number of commandline arguments
 * @param argv commandline arguments
 * @return 0 on successful execution
 */
int main(int argc, char *argv[]) {
    if (argc == 2) {
	int info[3];
        readFile(argv[1], info);
	int states = info[0], start = info[1], accept = info[2];

	struct state machine[states];
	buildMachine(argv[1], machine, states);
	displayMachine(machine, states);
	runMachine(machine, start, accept);
    } else {
	fprintf(stderr, "usage: ./tokenize tmfile\n");
	return 1;
    }

    return 0;
}
