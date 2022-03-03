#include "enc.h"
#include "monocypher.h"

static void hex_to_bin(unsigned char *out, char *in) {
    size_t len = strlen(in);

    if (len % 2 != 0) {
        return;
    }

    for (int i = 0; i < len; i += 2) {
        unsigned char l, r;
        l = tolower(in[i]);
        r = tolower(in[i+1]);

        if (l >= 'a' && l <= 'f') {
            l -= 'a';
            l += 10;
        }

        if (l >= '0' && l <= '9') {
            l -= '0';
        }

        if (r >= 'a' && r <= 'f') {
            r -= 'a';
            r += 10;
        }

        if (r >= '0' && r <= '9') {
            r -= '0';
        }

        out[i/2] = (l << 4) | r;
    }
}

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
    unsigned char key[32] = {0};
    unsigned char nonce[12] = {0};

    if (argc < 2) {
        usage();
    }

    if (strlen(argv[1]) != 64) {
        fputs("key size is invalid (needs to be 32 bytes / 64 hex digits)\n", stderr);
        exit(1);
    }

    if (!is_hex(argv[1])) {
        fputs("key is not a valid hexadecimal string\n", stderr);
        exit(1);
    }

    hex_to_bin(key, argv[1]);

    unsigned char pad[4096];
    uint32_t ctr = 0;
    size_t pad_used = sizeof(pad);
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
            if (pad_used == sizeof(pad)) {
                pad_used = 0;
                memset(pad, 0, sizeof(pad));
                ctr = crypto_ietf_chacha20_ctr(pad, pad, sizeof(pad), key, nonce, ctr);
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
