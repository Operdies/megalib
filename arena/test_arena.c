#include "arena.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

void allocate_bigly() {
	arena *a = mk_arena();
	for (int i = 0; i < 30; i++) {
		struct timeval start;
		gettimeofday(&start, NULL);
		if (!arena_alloc(a, 1 << 30, 1)) {
			perror("arena_alloc:");
			break;
		}
		struct timeval end;
		gettimeofday(&end, NULL);
		printf("Allocated %d GB in %ld ms\n", i+1, (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000);
	}
	destroy_arena(a);
}

int main() {
	allocate_bigly();
	int outerest_trials = 10;
	int inner_trials = 10;
	for (int outerest = 0; outerest < outerest_trials; outerest++) {
		arena *a = mk_arena();
		char *allocated_strings[inner_trials];
		char *test_strings[] = {
			"first\n",
			"second\n",
			"a secret third string\n",
			"laaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaarge string\n",
			"getting tired\n",
			"first\n",
			"second\n",
			"a secret third string\n",
			"laaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaarge string\n",
			"getting tired\n",
		};

		size_t expected_cont_length = 0;
		for (int i = 0; i < inner_trials; i++) {
			expected_cont_length += strlen(test_strings[i]) + 1; // +1: joining char
		}

		char *veryfirst = NULL;
		size_t outer_trials = 100;
		for (int j = 0; j < outer_trials; j++) {
			for (int i = 0; i < inner_trials; i++) {
				char *source = test_strings[i];
				int n = strlen(source);
				char *new = arena_alloc(a, n+1, 1);
				if (new == NULL) {
					printf("Alloc failed!\n");
					return 1;
				}
				allocated_strings[i] = new;
				strcpy(allocated_strings[i], source);

				if (!veryfirst)
					veryfirst = allocated_strings[i];
			}

			for (int i = 0; i < inner_trials; i++) {
				char *source = test_strings[i];
				char *dest = allocated_strings[i];
				if (strcmp(source, dest) != 0) {
					printf("Bad copy\n ? %s\n : %s", source, dest);
					exit(0);
				}
				allocated_strings[i][strlen(allocated_strings[i])] = '_';
			}

			size_t cont_length = strlen(allocated_strings[0]);
			if (cont_length != expected_cont_length) {
				printf("Expected length %lu but was %lu\n", expected_cont_length, cont_length);
				exit(1);
			}
		}

		size_t mega_length = strlen(veryfirst);
		size_t expected_mega_length = expected_cont_length * outer_trials;
		if (mega_length != expected_mega_length) {
			printf("Expected mega length %lu but was %lu\n", expected_mega_length, mega_length);
			exit(1);
		}

		destroy_arena(a);
	}
	printf("Tests passing!\n");
}

