/* Mouse bubble program

Released under Creative-Commons-Zero.

https://creativecommons.org/publicdomain/zero/1.0/legalcode

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined(__32BIT__)
#include <pos.h>
#endif

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

#if defined(__32BIT__)
#define inpb(p) PosInp(p)
#define inportb(p) PosInp(p)
#define outb(p,b) PosOutp(p,b)
#define outportb(p,b) PosOutp(p,b)
/* DJGPP uses inportb and outportb for writes to IO ports */
#elif defined(__DJGPP__) || defined(__GO32__)
#include <pc.h>
#elif !defined(__BORLANDC__) && defined(__TURBOC__)
#define inpb(p) inportb(p)
#define outb(p, v) outportb(p, v)
#endif

#define kbhit() PosKeyboardHit()

/* transfer mode */
#define SB_SINGLE 0x48
#define SB_AUTO 0x58

#define SB_PLAYSOUND 0x00

struct Context
{
    unsigned irq;
    unsigned base;
};

struct Sound
{
    void *data;
    size_t len;
    size_t hz;
};

struct WAV_Header
{
    uint8_t sign[4];          /* "RIFF" */
    uint32_t size;            /* File size - 4 bytes */
    uint8_t file_type[4];     /* "WAVE" */
    uint8_t fmt_mark[4];      /* "fmt" with trailing 0 */
    uint32_t fmt_len;         /* Length of format data */
    uint16_t type;            /* 1 = pcm */
    uint16_t channels;        /* Number of channels */
    uint32_t sample_rate;     /* Sample rate */
    uint32_t sample_len;      /* Bytes per sample */
    uint8_t bits;             /* 1 = 8 bit mono
                                 2 = 8 bit stereo
                                 3 = 16 bit mono
                                 4 = 16 bit stereo */
    uint16_t bits_per_sample; /* Bits per sample */
    uint8_t data_mark[4];     /* Data chunk header marker */
    uint32_t data_len;        /* Length of data */
};

static int ReadWAV(FILE *fp, struct WAV_Header *hdr);
static int ReadFrameWAV(FILE *fp, struct WAV_Header *hdr,
                        void *buf, size_t size);

static int Init(struct Context *ctx, char bits);
static int InitChannel(struct Context *ctx, int channel);
static int Play(struct Context *ctx, struct Sound *sample,
                int channel);
static int CreateSample(struct Sound *sample, unsigned hz,
                        unsigned sec);

static int DriverSB_Init(int base, int irq, char inout, int bits);
static int DriverSB_InitPlay(int base);
static int DriverSB_InitChannel(int base, int channel, int mode);
static int DriverSB_SetChannel(int base, signed int channel, int hz,
                               int transmode, int soundtype, void *data,
                               size_t len);
static int DriverSB_Pause(int base, int bits);
static int DriverSB_Continue(int base, int bits);

#define MAX_SAMPLE_SIZE 48000
static unsigned char *samplebuf = NULL;

static int ReadWAV(FILE *fp, struct WAV_Header *hdr)
{
    if (fread(hdr, 1, sizeof(*hdr), fp) < sizeof(*hdr))
        return -1;
    
    /* Sanity checks */
    if (memcmp(&hdr->sign, "RIFF", 4) || memcmp(&hdr->file_type, "WAVE", 4)
        || hdr->sample_rate > 448000)
        return -1;
    return 0;
}

static int ReadFrameWAV(FILE *fp, struct WAV_Header *hdr,
                 void *buf, size_t size)
{
    return (fread(buf, 1, size, fp) == size) ? 0 : -1;
}

/* Soundblaster IRQ table */
static const int irqToNum[4][2] = {
    {0x01, 0x02}, {0x02, 0x05}, {0x04, 0x07}, {0x08, 0x0A}};
static int DriverSB_Init(int base, int irq, char inout, int bits)
{
    unsigned int i;
    unsigned dspVersion;
    /* Send first DSP */
    outb((0x206 + base), 1);
    /* Wait 3ms */
    PosSleep(1);
#if 0
    for (i = 0; i < 30000; i++)
        ;
#endif
    /* Do final DSP reset */
    outb((0x206 + base), 0);
    /* Read final 0xAA code */
    if (inportb(0x20A + base) != 0xAA)
        return -1;
    /* Obtain Soundblaster DSP version */
    while (inportb(0x20C + base) & 0x80)
        ;
    outportb(0x20C + base, 0xE1);
    dspVersion = inportb(0x20A + base) << 8;
    dspVersion |= inportb(0x20A + base);
    /* Send IRQ to send data to */
    outb(0x204 + base, 0x80);
    outb(0x205 + base, irqToNum[irq][0]);
    switch (inout)
    {
    case 0:
        /* Start the speaker */
        while (inportb(0x20C + base) & 0x80)
            ;
        outb(0x20C + base, 0xD1);
        break;
    case 1:
        break;
    default:
        break;
    }
    return 0;
}

static int DriverSB_InitChannel(int base, int channel, int mode)
{
    /* Initialize playback on channel */
    outb(0x0A, 0x05 + channel);
    outb(0x0C, 0x01);
    outb(0x0B, mode + channel);
    return 0;
}

static int DriverSB_SetChannel(int base, signed int channel,
                        int hz, int transmode, int soundtype,
                        void *data, size_t len)
{
    unsigned long def_addr = (unsigned long)data;
    unsigned long addr;
    if (channel == 0)
        channel = -1;
    /* Get linear address of data pointer */
#ifdef __32BIT__
    /* copy data to lower 640k - address range
       0x80000-0x90000 is hopefully unused */
    addr = 0x80000;
    memcpy((char *)addr, (char *)def_addr, len);
#else
    addr = ((def_addr >> 16) * 16) + (def_addr & 0xFFFF);
#endif
    outb(0x82 + channel, (addr >> 16) & 0xFF);
    addr = (addr & 0xFFFE);
    /* Output the stuff to the SB16 ports */
    outb(0x01 + channel, (addr & 0xFF));
    outb(0x01 + channel, ((addr >> 8) & 0xFF));
    outb(0x02 + channel, (len & 0xFF));
    outb(0x02 + channel, ((len >> 8) & 0xFF));
    outb(0x0A, 1);
    /* Set SB16 sample rates */
    outb(0x20C + base, 0x41);
    hz *= 2;
    outb(0x20C + base, (hz >> 8) & 0xFF);
    outb(0x20C + base, hz & 0xFF);
    /* Send mode to the SB16 */
    outb(0x20C + base, transmode);
    outb(0x20C + base, soundtype);
    /* Lenght of data - 1*/
    outb(0x20C + base, len - 1);
    outb(0x20C + base, (len >> 8) - 1);
    return 0;
}

static int DriverSB_Pause(int base, int bits)
{
    switch (bits)
    {
    case 8:
        outb(0x20C + base, 0xD0);
        break;
    case 16:
        outb(0x20C + base, 0xD5);
        break;
    }
    return 0;
}

static int DriverSB_Continue(int base, int bits)
{
    switch (bits)
    {
    case 8:
        outb(0x20C + base, 0xD4);
        break;
    case 16:
        outb(0x20C + base, 0xD6);
        break;
    }
    return 0;
}

static int Init(struct Context *ctx, char bits)
{
    register unsigned char i, j;
    for (i = 0x00; i < 0xF0; i += 0x10)
    {
        for (j = 0; j < 3; j++)
        {
            if (DriverSB_Init(i, 0, SB_PLAYSOUND, bits))
            {
                ctx->base = i;
                ctx->irq = j;
                return 0;
            }
        }
    }
    return -1;
}

static int InitChannel(struct Context *ctx, int channel)
{
    DriverSB_InitChannel(ctx->base, channel, SB_AUTO);
    return 0;
}

static int Play(struct Context *ctx,
         struct Sound *sample, int channel)
{
    DriverSB_SetChannel(ctx->base, channel, sample->hz, 0xC0, 0x00,
                        samplebuf, sample->len);
    return 0;
}

static int CreateSample(struct Sound *sample, unsigned hz,
                 unsigned sec)
{

    sample->hz = hz;
    sample->len = hz * sec;
    if (sample->len > MAX_SAMPLE_SIZE)
        return -1;
    samplebuf = realloc(samplebuf, sample->len);
    if (samplebuf == NULL)
        return -1;
    sample->data = samplebuf;
    return 0;
}

/* Underlying engine */
static struct Context sctx;
static struct Sound sample;

static FILE *fp = NULL;
static volatile unsigned *clock = (volatile unsigned *)0x0000046C;
static unsigned start;
static float t1;
static float progress;

/* TODO: Have interrupt handler for SB16 requests */
void sb16_handler(void)
{
    abort();
    fread(sample.data, sizeof(char), sample.len, fp);
    Play(&sctx, &sample, 1);
    outb(0x21 + sctx.irq, 0x20); /* Send EOI */
}

int main(int argc, char **argv)
{
    struct WAV_Header wav;

    if(argc <= 1)
    {
        fprintf(stderr, "*** usage: %s [audio file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fp = fopen(argv[1], "rb");
    if (!fp)
    {
        printf("Error opening sound\n");
        goto end;
    }

    printf("Press any key to exit\n");
    if (strstr(argv[1], ".WAV") || strstr(argv[1], ".wav"))
    {
        sctx.base = 0x20;
        sctx.irq = 2;
        DriverSB_InitChannel(sctx.base, 1, SB_AUTO);
        InitChannel(&sctx, 1);
        if (ReadWAV(fp, &wav) < 0)
        {
            fprintf(stderr, "Not a valid WAV file\n");
            goto end;
        }
        printf("Sample rate=%u\n", wav.sample_rate);
        if (CreateSample(&sample, wav.sample_rate, 1))
        {
            printf("Error creating sound sample\n");
            goto end;
        }

        while (!kbhit())
        {
            *clock = 0;
            if (ReadFrameWAV(fp, &wav, sample.data, sample.len) < 0)
            {
                fprintf(stderr, "Audio file ended\n");
                goto end;
            }

            do {
                if (kbhit())
                    break;
            } while (*clock <= 8);
            Play(&sctx, &sample, 1);
        }
    }
    else
    {
        fprintf(stderr, "Unknown file format\n");
        goto end;
    }

end:
    /* Wait until SB16 buffer runs out */
    *clock = 0;
    if (fp)
        fclose(fp);
    if (samplebuf) {
        memset(sample.data, 0, sample.len);
        free(samplebuf);
        do { } while (*clock <= 8);
    }
    return 0;
}
