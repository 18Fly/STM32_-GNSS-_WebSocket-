#include "WebSocket.h"

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

const char *magic_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// SHA-1 context structure
typedef struct
{
    uint32_t state[5];  // state (ABCDE)
    uint32_t count[2];  // number of bits, modulo 2^64 (high, low)
    uint8_t buffer[64]; // input buffer
} SHA1_CTX;

void SHA1Transform(uint32_t state[5], const uint8_t buffer[64]);
void SHA1Init(SHA1_CTX *context);
void SHA1Update(SHA1_CTX *context, const uint8_t *data, uint32_t len);
void SHA1Final(uint8_t digest[20], SHA1_CTX *context);
void SHA1Encode(uint8_t *output, uint32_t *input, uint32_t len);
void SHA1Decode(uint32_t *output, const uint8_t *input, uint32_t len);

// Circular left shift macro
#define ROTLEFT(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

// Constants defined in SHA-1
#define K0 0x5A827999
#define K1 0x6ED9EBA1
#define K2 0x8F1BBCDC
#define K3 0xCA62C1D6

// Base64 编码函数
void Base64_Encode(const unsigned char *input, int len, char *output)
{
    int i = 0, j = 0;

    // 每三个字节编码为四个 Base64 字符
    for (int n = 0; n < len; n += 3) {
        unsigned char byte0 = input[n];
        unsigned char byte1 = (n + 1 < len) ? input[n + 1] : 0;
        unsigned char byte2 = (n + 2 < len) ? input[n + 2] : 0;

        unsigned int triple = (byte0 << 16) | (byte1 << 8) | byte2;

        output[j++] = base64_chars[(triple >> 18) & 0x3F];
        output[j++] = base64_chars[(triple >> 12) & 0x3F];
        output[j++] = (n + 1 < len) ? base64_chars[(triple >> 6) & 0x3F] : '=';
        output[j++] = (n + 2 < len) ? base64_chars[triple & 0x3F] : '=';
    }

    // 添加字符串终止符
    output[j] = '\0';
}

// Initial hash values for SHA-1
void SHA1Init(SHA1_CTX *context)
{
    context->count[0] = context->count[1] = 0;
    context->state[0]                     = 0x67452301;
    context->state[1]                     = 0xEFCDAB89;
    context->state[2]                     = 0x98BADCFE;
    context->state[3]                     = 0x10325476;
    context->state[4]                     = 0xC3D2E1F0;
}

// Update the hash with a new chunk of data
void SHA1Update(SHA1_CTX *context, const uint8_t *data, uint32_t len)
{
    uint32_t i, j;
    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3)) {
        context->count[1]++;
    }
    context->count[1] += (len >> 29);
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        SHA1Transform(context->state, context->buffer);
        for (; i + 63 < len; i += 64) {
            SHA1Transform(context->state, &data[i]);
        }
        j = 0;
    } else {
        i = 0;
    }
    memcpy(&context->buffer[j], &data[i], len - i);
}

// Finalize the hash and produce the output digest
void SHA1Final(uint8_t digest[20], SHA1_CTX *context)
{
    uint8_t finalcount[8];
    uint8_t c;

    for (int i = 0; i < 8; i++) {
        finalcount[i] = (uint8_t)((context->count[(i >= 4 ? 0 : 1)] >>
                                   ((3 - (i & 3)) * 8)) &
                                  255); // Endian independent
    }

    c = 0x80;
    SHA1Update(context, &c, 1);
    while ((context->count[0] & 504) != 448) {
        c = 0x00;
        SHA1Update(context, &c, 1);
    }

    SHA1Update(context, finalcount, 8); // Should cause a SHA1Transform()
    SHA1Encode(digest, context->state, 20);
    memset(context, 0, sizeof(*context)); // Wipe the context
}

// Helper function to encode uint32_t to uint8_t
void SHA1Encode(uint8_t *output, uint32_t *input, uint32_t len)
{
    for (uint32_t i = 0, j = 0; j < len; i++, j += 4) {
        output[j]     = (uint8_t)((input[i] >> 24) & 255);
        output[j + 1] = (uint8_t)((input[i] >> 16) & 255);
        output[j + 2] = (uint8_t)((input[i] >> 8) & 255);
        output[j + 3] = (uint8_t)(input[i] & 255);
    }
}

// Helper function to decode uint8_t to uint32_t
void SHA1Decode(uint32_t *output, const uint8_t *input, uint32_t len)
{
    for (uint32_t i = 0, j = 0; j < len; i++, j += 4) {
        output[i] = ((uint32_t)input[j] << 24) | ((uint32_t)input[j + 1] << 16) |
                    ((uint32_t)input[j + 2] << 8) | ((uint32_t)input[j + 3]);
    }
}

// Core transformation function for processing a 512-bit block
void SHA1Transform(uint32_t state[5], const uint8_t buffer[64])
{
    uint32_t a, b, c, d, e, t, m[80];

    SHA1Decode(m, buffer, 64);
    for (int i = 16; i < 80; i++) {
        m[i] = ROTLEFT(m[i - 3] ^ m[i - 8] ^ m[i - 14] ^ m[i - 16], 1);
    }

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    for (int i = 0; i < 80; i++) {
        if (i < 20)
            t = ROTLEFT(a, 5) + ((b & c) | (~b & d)) + e + m[i] + K0;
        else if (i < 40)
            t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + m[i] + K1;
        else if (i < 60)
            t = ROTLEFT(a, 5) + ((b & c) | (b & d) | (c & d)) + e + m[i] + K2;
        else
            t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + m[i] + K3;

        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

void repeatWSRaw(char *Key, char *Buf)
{
    char requestBody[130] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
    MyDMA_Init((uint32_t)requestBody, (uint32_t)Buf, 129);

    uint8_t tmp[61] = {0};
    uint8_t hash[20];
    uint8_t encoded[32] = {0};
    SHA1_CTX context;

    strcat(tmp, Key);
    strcat(tmp, magic_string);

    SHA1Init(&context);
    SHA1Update(&context, tmp, 60);
    SHA1Final(hash, &context);

    Base64_Encode(hash, 20, encoded);

    strcat(requestBody, encoded);

    strcat(requestBody, "\r\n\r\n");

    MyDMA_Transfer();
}
