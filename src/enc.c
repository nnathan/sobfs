#include "enc.h"
#include "gimli.h"

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
    puts("sobfs 32-byte-key-in-hex [16-byte-iv-in-hex]");
    exit(254);
}

int main(int argc, char **argv) {
    unsigned char key[32] = {0};
    unsigned char iv[16] = {0};

    if (argc < 2) {
        usage();
    }

    if (strlen(argv[1]) != 64) {
        fputs("key size is invalid", stderr);
        exit(1);
    }

    if (!is_hex(argv[1])) {
        fputs("key is not a valid hexadecimal string", stderr);
        exit(1);
    }

    hex_to_bin(key, argv[1]);

    if (argc > 2) {
      if (strlen(argv[2]) != 32) {
          fputs("iv size is invalid", stderr);
          exit(1);
      }

      if (!is_hex(argv[2])) {
          fputs("iv is not a valid hexadecimal string", stderr);
          exit(1);
      }

      hex_to_bin(iv, argv[2]);
    }

    uint32_t state[12];
    memcpy(&state[0], key, 32);
    memcpy(&state[8], iv, 16);

    gimli(state);

    char buf[4096] = {0};
    size_t n;
    size_t state_n = 0;
    unsigned char *state_buf = (unsigned char *) state;

    while (1) {
        if ((n = read(STDIN_FILENO, buf, sizeof(buf))) < 0) {
            fputs("error reading from stdin", stderr);
            exit(1);
        }

        if (n == 0) {
            break;
        }

        for (int i = 0; i < n; i++, state_n++) {
            if (state_n == 48) {
                state_n = 0;
                gimli(state);
            }

            buf[i] ^=  state_buf[state_n];
        }

        size_t len = n;

        while(len > 0) {
            size_t write_n;

            if ((write_n = write(STDOUT_FILENO, buf, len)) < 0) {
                fputs("error writing to stdout", stderr);
                exit(1);
            }

            len -= write_n;
        }
    }

    return 0;
}
