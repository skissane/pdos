/* mainframe support header file */
/* written by Paul Edwards */
/* released to the public domain */

typedef union {
        double dummy;
        struct {
            unsigned int addr;
            unsigned short flags;
            unsigned short len;
        } actual;
    } CCW;

typedef union {
        double dummy;
        struct {
            unsigned short residual_count;
            unsigned short communication_bytes;
            unsigned short csw_status_bytes;
            unsigned char logical_unit_class;
            unsigned char logical_unit;
            void *ccw_address;
            unsigned int status_and_csw_ccw;
        } actual;
    } CCB;

typedef struct {
    int r[16];
} REGS;
