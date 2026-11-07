#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <unistd.h>

void allocateWithMallocFree();
void getMemoryUsage(unsigned long *vmsize, unsigned long *vmrss);
void mmapThreshold();

int main() {
	while (1) {
		banner_call();
		printf("1. Allocate with malloc/free\n");
		printf("2. Allocate with mmap/munmap\n");
		printf("3. Eksekusi Fitur Variasi Kelompok\n");
		printf("4. Exit\n");
		printf("input: ");

		char choice;
		scanf(" %c", &choice);
		
		if (choice == '1') {

		}

		else if (choice == '2') {

		}

		else if (choice == '3') {
            mmapThreshold();
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

void mmapThreshold() {
    int THRESHOLD = 128;
    printf("--- Varian 6: mmap Threshold ---\n");
    printf("M_MMAP_THRESHOLD = %d KB\n\n", THRESHOLD);
    printf("%-9s %-12s %-15s %-15s %s\n", "Ukuran", "delta brk", "alamat", "ujung brk", "Hasil");

    mallopt(M_MMAP_THRESHOLD, THRESHOLD * 1024);
        
    int numbers[] = {1, 2, 4, 8, 16, 32, 64, 96, 120, 127, 128, 129, 192, 256, 512, 1024};
    int length = sizeof(numbers) / sizeof(numbers[0]);
    void *ptr[length];
    int first_mmap_kb = -1;

    for (int i = 0; i < length; i++) {
        size_t bytes = (size_t)numbers[i] * 1024;

        // posisi ujung heap sebelum malloc
        void *brk_awal = sbrk(0);

        ptr[i] = malloc(bytes);

        // posisi ujung heap setelah malloc
        void *brk_akhir = sbrk(0);

        // selisih sebelum dan sesudah malloc
        long delta = (char *)brk_akhir - (char *)brk_awal;
        
        // alamat pointer di bawah ujung brk, maka dialokasikan di heap.
        // di atas ujung brk, maka dialokasikan via mmap.
        const char *lokasi;
        if ((uintptr_t)ptr[i] < (uintptr_t)brk_akhir) {
            lokasi = "heap";
        } else {
            lokasi = "mmap";
            if (first_mmap_kb == -1) {
                first_mmap_kb = numbers[i];
            }
        }

        printf("%-4d KB %-+12ld %-15p %-15p %s\n", numbers[i], delta, ptr[i], brk_akhir, lokasi);
    }

    if (first_mmap_kb != -1) {
        printf("\n[Hasil] malloc mulai beralih ke mmap pada ukuran %d KB (threshold %d KB)\n", first_mmap_kb, THRESHOLD);
    }

    // release memory
    for (int i = 0; i < length; i++) {
        free(ptr[i]);
    }
}