#include "enc.h"
#include "hydrogen.h"

static int is_hex(char *in) {
    for (int i = 0; i < strlen(in); i++) {
        if (in[i] >= '0' && in[i] <= '9')
            continue;

        if (in[i] >= 'a' && in[i] <= 'f')
            continue;

        if (in[i] >= 'A' && in[i] <= 'F')
            continue;

        return 0;
    }

    return 1;
}

static void usage() {
    puts("sobfs 32-byte-key-in-hex");
    exit(254);
}

int main(int argc, char **argv) {
    unsigned char key[hydro_random_SEEDBYTES] = {0};

    if (argc < 2) {
        usage();
    }

    if (strlen(argv[1]) != hydro_random_SEEDBYTES*2) {
        fprintf(stderr,
                "key size is invalid (needs to be %d bytes / %d hex digits)\n",
                hydro_random_SEEDBYTES, hydro_random_SEEDBYTES * 2);
        exit(1);
    }

    if (!is_hex(argv[1])) {
        fputs("key is not a valid hexadecimal string\n", stderr);
        exit(1);
    }

    int ret = hydro_hex2bin(key, sizeof(key), argv[1],
                            hydro_random_SEEDBYTES * 2, NULL, NULL);

    unsigned char pad_with_key[4096 + hydro_random_SEEDBYTES];
    unsigned char *pad = pad_with_key + hydro_random_SEEDBYTES;
    const size_t max_pad = sizeof(pad_with_key) - hydro_random_SEEDBYTES;
    size_t pad_used = max_pad;
    char buf[4096];

    while (1) {
        size_t n;

        if ((n = read(STDIN_FILENO, buf, sizeof(buf))) < 0) {
            fputs("error reading from stdin\n", stderr);
            exit(1);
        }

        if (n == 0) {
            break;
        }

        for (int i = 0; i < n; i++) {
            if (pad_used == max_pad) {
                pad_used = 0;
                hydro_random_buf_deterministic(pad_with_key, sizeof(pad_with_key), key);
                memcpy(key, pad_with_key, hydro_random_SEEDBYTES);
            }

            buf[i] ^=  pad[pad_used++];
        }

        size_t len = n;

        while(len > 0) {
            if ((n = write(STDOUT_FILENO, buf, len)) < 0) {
                fputs("error writing to stdout\n", stderr);
                exit(1);
            }

            len -= n;
        }
    }

    return 0;
}
