#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void allocateWithMallocFree();
void getMemoryUsage(unsigned long *vmsize, unsigned long *vmrss);

int main() {
	while (1) {
		banner_call();
		printf("1. Allocate with malloc/free\n");
		printf("2. Allocate with mmap/munmap\n");
		printf("3. Eksekusi Fitur Variasi Kelompok\n");
		printf("4. Exit\n");
		printf("input: ");

		char choice;
		scanf("%c", &choice);
		
		if (choice == '1') {

		}

		else if (choice == '2') {

		}

		else if (choice == '3') {

		}

		else if (choice == '4') {
			break;
		}

		else {
			printf("Pilihan tidak valid. Harap masukkan pilihan dengan benar.\n");
		}
	}
    return 0;
}

void allocateWithMallocFree() {
    unsigned long vmsize = 0, vmrss = 0;
    int input;

    getMemoryUsage(&vmsize, &vmrss);

    printf("You picked malloc/free.\n");
    printf("VmSize Before: %lu KB | VmRSS Before: %lu KB\n", vmsize, vmrss);
    printf("How many integers to allocate: ");
    printf("\n");

    scanf("%d", &input);

    int *memory = malloc(input * sizeof(int));

    for (int i = 0; i < input; i++) {
        memory[i] = i + 1;
    }

    printf("Starting memory address: %p\n", memory);
    printf("Printing first 10 values in reverse order:\n");

    int limit = input < 10 ? input : 10;

    for (int i = input - 1; i >= input - limit; i--) {
        printf("%d\n", memory[i]);
    }
    printf("\n");

    getMemoryUsage(&vmsize, &vmrss);

    printf("VmSize After: %lu KB | VmRSS After: %lu KB\n", vmsize, vmrss);

    free(memory);
    memory = NULL;
}

void getMemoryUsage(unsigned long *vmsize, unsigned long *vmrss) {
    FILE *file = fopen("/proc/self/status", "r");
    if (!file) {
        perror("fopen");
        return;
    }
    char line[128];
    *vmsize = 0;
    *vmrss = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "VmSize:", 7) == 0) {
            *vmsize = strtoul(line + 7, NULL, 10);
        } else if (strncmp(line, "VmRSS:", 6) == 0) {
            *vmrss = strtoul(line + 6, NULL, 10);
        }
    }

    fclose(file);
}