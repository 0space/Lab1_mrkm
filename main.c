#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdint-gcc.h>
#include <stdlib.h>
#include <string.h>

#define BYTE_SIZE 8
#define BIGINT_MAX_LEN_BYTES SIZE_MAX / BYTE_SIZE

typedef struct BIG_INT_DATA {
    unsigned char *value;
    size_t size_bytes;
} BIG_INT_DATA;

typedef struct BIG_INT {
    BIG_INT_DATA data;
    unsigned char sign;
} BIG_INT;

size_t bits_num_max(size_t n, size_t base) {
    return (size_t) ceil(n * log2(base));
}

size_t bytes_num_max(size_t n, size_t base) {
    return (size_t) ceil((double) bits_num_max(n, base)/BYTE_SIZE);
}

BIG_INT big_int_alloc(size_t n, size_t base) {
    size_t s_n;
    size_t b_max;
    BIG_INT bi;
    b_max = bytes_num_max(n, base);
    if (b_max > BIGINT_MAX_LEN_BYTES) {
        printf("ERROR: LIMIT EXCEEDED.\n");
        goto err;
    }
    bi.data.value = malloc(b_max + 1);
    bi.data.value[b_max] = '\0';
    if (!bi.data.value) {
        printf("ERROR: malloc failed.\n");
        goto err;
    }
    bi.data.size_bytes = b_max;
err:
    return bi;
}

BIG_INT hex_to_big_int(char *s_num) {
    BIG_INT bi;
    int i;
    size_t n;
    unsigned char *cur;
    unsigned char sign;
    cur = s_num;
    if (cur[0] == '-') {
        sign = 1;
        cur++;
    }
    if (cur[0] == '0' && cur[0] == 'x') {
        cur+=2;
    }
    n = strlen(cur);
    bi = big_int_alloc(n, 16);
    i = bi.data.size_bytes - 1;
    if (n & 1) {
        sscanf(cur, "%1hhx", &bi.data.value[i]);
        i--;
        cur++;
    }
    for (i; i >= 0; i--) {
        sscanf(cur, "%2hhx", &bi.data.value[i]);
        cur += 2;
    }
    // printf("Value: ");
    // for (i = 0; i < bi.data.size_bytes; i++) {
    //     printf("%02x", bi.data.value[i]);
    // }
    // printf("\n");
err:
    return bi;
}

char * big_int_to_hex(BIG_INT bi) {
    int i;
    char *out;
    char *cur;
    out = malloc(2 * (bi.data.size_bytes) + 4);
    if (!out) {
        printf("ERROR: malloc failed.\n");
        goto err;
    }
    out[2 * bi.data.size_bytes] = '\0';
    cur = out;
    if (bi.sign == 1) {
        sprintf(cur, "-");
        cur++;
    }
    // sprintf(cur, "0x");
    // cur+=2;
    sprintf(cur, "00");
    for (i = bi.data.size_bytes; i >= 0 ; i--) {
        if (bi.data.value[i] != 0) {
            sprintf(cur, "%02hhx", bi.data.value[i]);
            cur += 2;
        }
    }
    // printf("Value: %s\n", out);
err:
    return out;
}


BIG_INT big_int_create(char *s_num, size_t base) {
    BIG_INT bi;
    switch(base) {
        case 16:
            bi = hex_to_big_int(s_num);
            break;
        default:
            printf("ERROR: wrong base");
    }
    return bi;
}

BIG_INT big_int_sum(BIG_INT a, BIG_INT b) {
    int i;
    int sum;
    size_t n;
    BIG_INT c, carryover;
    n = (a.data.size_bytes > b.data.size_bytes ? a.data.size_bytes: b.data.size_bytes);
    c = big_int_alloc((n) * 2, 16);
    carryover = big_int_alloc(1, 16);
    carryover.data.value[0] = 0;
    for (i = 0; i < c.data.size_bytes; i++) {
        sum = pow(-1, a.sign) * a.data.value[i] + \
              pow(-1, b.sign) * b.data.value[i] + \
              pow(-1, carryover.sign) * carryover.data.value[0];
        // printf("%hx - %hx - %hx",
        //     a.data.value[i],
        //     b.data.value[i],
        //     carryover.data.value[0]);
        if (sum < 0) {
            carryover.sign = 1;
            carryover.data.value[0] = 1;
            sum += UCHAR_MAX + 1;
        }
        else {
            carryover.sign = 0;
            carryover.data.value[0] = (sum >> 8) & UCHAR_MAX;
        }
        c.data.value[i] = sum & UCHAR_MAX;
        c.sign = carryover.sign;
        // printf(" = %hx (%hx)\n",
        //     c.data.value[i],
        //     carryover.data.value[0]);
    }
    if (carryover.sign == 1) {
        c.data.value[0]--;
        for (i = 0; i < c.data.size_bytes; i++) {
            c.data.value[i] ^= UCHAR_MAX;
        }
    }
    return c;
}

BIG_INT big_int_sub(BIG_INT a, BIG_INT b) {
    int i;
    size_t n;
    int dif;
    BIG_INT c, carryover;
    n = (a.data.size_bytes > b.data.size_bytes ? a.data.size_bytes: b.data.size_bytes);
    c = big_int_alloc((n) * 2, 16);
    carryover.data.value[0] = 0;
    for (i = 0; i < c.data.size_bytes; i++) {
        dif = pow(-1, a.sign) * a.data.value[i] - \
              pow(-1, b.sign) * b.data.value[i] + \
              pow(-1, carryover.sign) * carryover.data.value[0];
        // printf("%hx - %hx - %hx",
        //     a.data.value[i],
        //     b.data.value[i],
        //     carryover.data.value[0]);
        if (dif < 0) {
            carryover.sign = 1;
            carryover.data.value[0] = 1;
            dif += UCHAR_MAX + 1;
        }
        else {
            carryover.sign = 0;
            carryover.data.value[0] = (dif >> 8) & UCHAR_MAX;
        }
        c.data.value[i] = dif & UCHAR_MAX;
        c.sign = carryover.sign;
        // printf(" = %hx (%hx)\n",
        //     c.data.value[i],
        //     carryover.data.value[0]);
    }
    if (carryover.sign == 1) {
        c.data.value[0]--;
        for (i = 0; i < c.data.size_bytes; i++) {
            c.data.value[i] ^= UCHAR_MAX;
        }
    }
    return c;
}

char * big_int_to_bin(BIG_INT bi) {
    unsigned char *bi_bin;
    unsigned char byte;
    size_t i, j;
    bi_bin = malloc(bi.data.size_bytes * BYTE_SIZE + 1);
    bi_bin[bi.data.size_bytes * BYTE_SIZE] = '\0';
    for (i = bi.data.size_bytes - 1; i >= 0; i--) {
        for (j = BYTE_SIZE - 1; j >= 0; j--) {
            // printf("%d__%d", i, j);
            byte = bi.data.value[i] & (1<<j);
            byte >>= j;
            printf("%u",byte);
        }
    }
    puts("");
    return 0;
}

int main(int argc, char **argv) {
    size_t base = 16;
    printf("Len: %zu\n", strlen(argv[1]));
    printf("Bits: %zu\n", bits_num_max(strlen(argv[1]), base));
    printf("Bytes: %zu\n", bytes_num_max(strlen(argv[1]), base));
    BIG_INT a = big_int_create(argv[1], base);
    BIG_INT b = big_int_create(argv[2], base);
    BIG_INT c = big_int_sum(a, b);
    BIG_INT d = big_int_sub(a, b);
    BIG_INT d_i = big_int_sub(b, a);
    printf("Sum: %s\n", big_int_to_hex(c));
    printf("Sub1: %s\n", big_int_to_hex(d));
    printf("Sub2: %s\n", big_int_to_hex(d_i));
    BIG_INT e = big_int_sum(d_i, a);
    printf("Test: %s\n", big_int_to_hex(e));
    // printf("%s\n", a.data.value);
    // big_int_to_bin(a);
    return 0;
}