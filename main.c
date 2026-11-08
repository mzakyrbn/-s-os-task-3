#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>

void allocateWithMallocFree();
void allocateWithMmapMunmap();
void getMemoryUsage(unsigned long *vmsize, unsigned long *vmrss);
void mmapThreshold();
void mmapThresholdLog(const char *log);
void banner_call(void);
static void mmap_log_printf(FILE *std_log, int sys_log, const char *format, ...);

int main() {
	while (1) {
		banner_call();
		printf("1. Allocate with malloc/free\n");
		printf("2. Allocate with mmap/munmap\n");
		printf("3. Ukuran batas mmap pada malloc\n");
		printf("4. Exit\n");
		printf("input: ");

		char choice;
		scanf(" %c", &choice);
		
		if (choice == '1') {

		}

		else if (choice == '2') {
			allocateWithMmapMunmap();
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

/* Print each mmap run line to the terminal and both required log formats. */
static void mmap_log_printf(FILE *std_log, int sys_log, const char *format, ...) {
    char buffer[1024];
    va_list args;

    va_start(args, format);
    va_list stdout_args;
    va_copy(stdout_args, args);
    vprintf(format, stdout_args);
    va_end(stdout_args);

    va_list file_args;
    va_copy(file_args, args);
    vfprintf(std_log, format, file_args);
    va_end(file_args);

    va_list buffer_args;
    va_copy(buffer_args, args);
    int length = vsnprintf(buffer, sizeof(buffer), format, buffer_args);
    va_end(buffer_args);
    va_end(args);

    if (length > 0) {
        size_t remaining = (size_t)length;
        if (remaining >= sizeof(buffer)) remaining = sizeof(buffer) - 1;
        const char *cursor = buffer;
        while (remaining > 0) {
            ssize_t written = write(sys_log, cursor, remaining);
            if (written < 0) {
                if (errno == EINTR) continue;
                perror("write log");
                break;
            }
            cursor += written;
            remaining -= (size_t)written;
        }
    }
}

void allocateWithMmapMunmap() {
    int input;
    unsigned long vmsize_before = 0, vmrss_before = 0;
    unsigned long vmsize_after = 0, vmrss_after = 0;
    char timestamp[32];
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    if (!local || strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", local) == 0) {
        snprintf(timestamp, sizeof(timestamp), "unknown-time");
    }

    char std_name[96], sys_name[96];
    snprintf(std_name, sizeof(std_name), "06-mmap_std-%s.txt", timestamp);
    snprintf(sys_name, sizeof(sys_name), "06-mmap_sys-%s.txt", timestamp);
    FILE *std_log = fopen(std_name, "w");
    if (!std_log) {
        perror("fopen mmap log");
        return;
    }
    int sys_log = open(sys_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (sys_log < 0) {
        perror("open mmap log");
        fclose(std_log);
        return;
    }

    getMemoryUsage(&vmsize_before, &vmrss_before);
    mmap_log_printf(std_log, sys_log, "You picked mmap/munmap.\n");
    mmap_log_printf(std_log, sys_log, "VmSize Before: %lu KB | VmRSS Before: %lu KB\n",
                    vmsize_before, vmrss_before);

    printf("How many integers to allocate: ");
    if (scanf("%d", &input) != 1 || input <= 0 || (size_t)input > SIZE_MAX / sizeof(int)) {
        printf("Please enter a positive integer count within the supported range.\n");
        close(sys_log);
        fclose(std_log);
        return;
    }
    printf("\n");

    size_t bytes = (size_t)input * sizeof(int);
    char input_log_line[80];
    int input_log_length = snprintf(input_log_line, sizeof(input_log_line),
                                    "Integer count entered: %d\n", input);
    if (input_log_length > 0 && (size_t)input_log_length < sizeof(input_log_line)) {
        fputs(input_log_line, std_log);
        (void)write(sys_log, input_log_line, (size_t)input_log_length);
    }

    int *memory = mmap(NULL, bytes, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        mmap_log_printf(std_log, sys_log, "mmap failed: %s\n", strerror(errno));
        close(sys_log);
        fclose(std_log);
        return;
    }

    for (int i = 0; i < input; i++) memory[i] = i + 1;
    mmap_log_printf(std_log, sys_log, "Starting memory address: %p\n", (void *)memory);
    mmap_log_printf(std_log, sys_log, "Printing first 10 values in reverse order:\n");
    int limit = input < 10 ? input : 10;
    for (int i = 0; i < limit; i++) {
        mmap_log_printf(std_log, sys_log, "%d\n", memory[input - 1 - i]);
    }
    mmap_log_printf(std_log, sys_log, "\n");

    getMemoryUsage(&vmsize_after, &vmrss_after);
    mmap_log_printf(std_log, sys_log, "VmSize After: %lu KB | VmRSS After: %lu KB\n", vmsize_after, vmrss_after);
    if (munmap(memory, bytes) != 0) perror("munmap");
    close(sys_log);
    fclose(std_log);
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
    char log_buffer[4096] = "";
    int offset = 0;

    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset,
                       "--- Varian 6: mmap Threshold ---\n"
                       "M_MMAP_THRESHOLD = 128 KB\n\n"
                       "%-9s %-12s %-15s %-15s %s\n",
                       "Ukuran", "delta brk", "alamat", "ujung brk", "Hasil");

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

        offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset,
                    "%-4d KB %-+12ld %-15p %-15p %s\n",
                    numbers[i], delta, ptr[i], brk_akhir, lokasi);
    }

    if (first_mmap_kb != -1) {
        offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "\n[Hasil] malloc mulai beralih ke mmap pada ukuran %d KB (threshold %d KB)\n", first_mmap_kb, THRESHOLD);
    }

    printf("%s", log_buffer);
    mmapThresholdLog(log_buffer);

    // free memory
    for (int i = 0; i < length; i++) {
        free(ptr[i]);
    }
}

void mmapThresholdLog(const char *log) {
    // get current time
    time_t now = time(NULL);
    struct tm *currtime = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", currtime);

    // naming file
    const char *method = "mmapthreshold";
    char file_std[128];
    char file_sys[128];
    snprintf(file_std, sizeof(file_std), "06-%s_std-%s.txt", method, timestamp);
    snprintf(file_sys, sizeof(file_sys), "06-%s_sys-%s.txt", method, timestamp);

    // write using standard C library
    FILE *pF = fopen(file_std, "w");
    if (pF) {
        fprintf(pF, "%s", log);
        fclose(pF);
    }

    // write using system call
    int fd = open(file_sys, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, log, strlen(log));
        close(fd);
    }


}
