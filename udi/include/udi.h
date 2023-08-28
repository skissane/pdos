/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#if !defined (UDI_VERSION)
 #error "UDI_VERSION must be defined."
#elif (UDI_VERSION != 0x101)
 #error "Unsupported UDI_VERSION."
#endif

#define NULL ((void *)0)

typedef signed char udi_sbit8_t;
typedef signed short udi_sbit16_t;
typedef signed long udi_sbit32_t;
typedef unsigned char udi_ubit8_t;
typedef unsigned short udi_ubit16_t;
typedef unsigned long udi_ubit32_t;
typedef udi_ubit8_t udi_boolean_t;
#define FALSE 0
#define TRUE 1

typedef unsigned long udi_size_t;
typedef unsigned char udi_index_t;

#define _UDI_HANDLE void *
#define _UDI_NULL_HANDLE NULL

typedef _UDI_HANDLE udi_channel_t;
#define UDI_NULL_CHANNEL _UDI_NULL_HANDLE

typedef _UDI_HANDLE udi_buf_path_t;
#define UDI_NULL_BUF_PATH _UDI_NULL_HANDLE

typedef _UDI_HANDLE udi_origin_t;
#define UDI_NULL_ORIGIN _UDI_NULL_HANDLE

typedef unsigned long udi_timestamp_t;

typedef udi_ubit32_t udi_status_t;
#define UDI_STATUS_CODE_MASK     0x0000FFFF
#define UDI_STAT_META_SPECIFIC   0x00008000
#define UDI_SPECIFIC_STATUS_MASK 0x00007FFF
#define UDI_CORRELATE_OFFSET     16
#define UDI_CORRELATE_MASK       0xFFFF0000

#define UDI_OK                     0
#define UDI_STAT_NOT_SUPPORTED     1
#define UDI_STAT_NOT_UNDERSTOOD    2
#define UDI_STAT_INVALID STATE     3
#define UDI_STAT_MISTAKEN_IDENTITY 4
#define UDI_STAT_ABORTED           5
#define UDI_STAT_TIMEOUT           6
#define UDI_STAT_BUSY              7
#define UDI_STAT_RESOURCE_UNAVAIL  8
#define UDI_STAT_HW_PROBLEM        9
#define UDI_STAT_NOT_RESPONDING    10
#define UDI_STAT_DATA_UNDERRUN     11
#define UDI_STAT_DATA_OVERRUN      12
#define UDI_STAT_DATA_ERROR        13
#define UDI_STAT_PARENT_DRV_ERROR  14
#define UDI_STAT_CANNOT_BIND       15
#define UDI_STAT_CANNOT_BIND_EXCL  16
#define UDI_STAT_TOO_MANY_PARENTS  17
#define UDI_STAT_BAD_PARENT_TYPE   18
#define UDI_STAT_TERMINATED        19
#define UDI_STAT_ATTR_MISMATCH     20

typedef const udi_ubit8_t udi_layout_t;
#define UDI_DL_UBIT8_T             1
#define UDI_DL_SBIT8_T             2
#define UDI_DL_UBIT16_T            3
#define UDI_DL_SBIT16_T            4
#define UDI_DL_UBIT32_T            5
#define UDI_DL_SBIT32_T            6
#define UDI_DL_BOOLEAN_T           7
#define UDI_DL_STATUS_T            8

#define UDI_DL_INDEX_T             20

#define UDI_DL_CHANNEL_T           30
#define UDI_DL_ORIGIN_T            32

#define UDI_DL_BUF                 40
#define UDI_DL_CB                  41
#define UDI_DL_INLINE_UNTYPED      42
#define UDI_DL_INLINE_DRIVER_TYPED 43
#define UDI_DL_MOVABLE_UNTYPED     44

#define UDI_DL_INLINE_TYPED        50
#define UDI_DL_MOVABLE_TYPED       51
#define UDI_DL_ARRAY               52
#define UDI_DL_END                 0

#define UDI_HANDLE_IS_NULL(handle, handle_type) \
 ((udi_boolean_t)((handle) == _UDI_NULL_HANDLE))
#define UDI_HANDLE_ID(handle, handle_type) ((void *)(handle))

#include <stdarg.h>
#define UDI_VA_ARG(pvar, type, va_code) \
 _##va_code(pvar, type)
#define _UDI_VA_ARG_INT(pvar, type) ((type)(va_arg(pvar, int))
#define _UDI_VA_UBIT8_T   _UDI_VA_ARG_INT
#define _UDI_VA_SBIT8_T   _UDI_VA_ARG_INT
#define _UDI_VA_UBIT16_T  _UDI_VA_ARG_INT
#define _UDI_VA_SBIT16_T  _UDI_VA_ARG_INT
#define _UDI_VA_UBIT32_T  va_arg
#define _UDI_VA_SBIT32_T  va_arg
#define _UDI_VA_BOOLEAN_T _UDI_VA_UBIT8_T
#define _UDI_VA_INDEX_T   _UDI_VA_UBIT8_T
#define _UDI_VA_SIZE_T    _UDI_VA_UBIT32_T
#define _UDI_VA_STATUS_T  _UDI_VA_UBIT32_T
#define _UDI_VA_CHANNEL_T va_arg
#define _UDI_VA_ORIGIN_T  va_arg
#define _UDI_VA_POINTER   va_arg

#ifndef offsetof
#define offsetof(t, f) ((udi_size_t)(&(((t *)0)->f)))
#endif

typedef struct {
    udi_channel_t channel;
    void *context;
    void *scratch;
    void *initiator_context;
    udi_origin_t origin;
} udi_cb_t;

typedef void udi_cb_alloc_call_t(udi_cb_t *gcb, udi_cb_t *new_cb);

void udi_cb_alloc(udi_cb_alloc_call_t *callback,
                  udi_cb_t *gcb,
                  udi_index_t cb_idx,
                  udi_channel_t default_channel);
void udi_cb_alloc_dynamic(udi_cb_alloc_call_t *callback,
                          udi_cb_t *gcb,
                          udi_index_t cb_idx,
                          udi_channel_t default_channel,
                          udi_size_t inline_size,
                          udi_layout_t *inline_layout);

typedef void udi_cb_alloc_batch_call_t(udi_cb_t *gcb,
                                       udi_cb_t *first_new_cb);

void udi_cb_alloc_batch(udi_cb_alloc_batch_call_t *callback,
                        udi_cb_t *gcb,
                        udi_index_t cb_idx,
                        udi_index_t count,
                        udi_boolean_t with_buf,
                        udi_size_t buf_size,
                        udi_buf_path_t path_handle);

void udi_cb_free(udi_cb_t *cb);

#define UDI_GCB(mcb) (&(mcb)->gcb)
#define UDI_MCB(gcb, cb_type) ((cb_type *)(gcb))

typedef void udi_cancel_call_t(udi_cb_t *gcb);

void udi_cancel(udi_cancel_call_t *callback,
                udi_cb_t *gcb);


typedef void udi_mem_alloc_call_t(udi_cb_t *gcb, void *new_mem);

void udi_mem_alloc(udi_mem_alloc_call_t *callback,
                   udi_cb_t *gcb,
                   udi_size_t size,
                   udi_ubit8_t flags);
#define UDI_MEM_NOZERO  (1U << 0)
#define UDI_MEM_MOVABLE (1U << 1)

void udi_mem_free(void *target_mem);


typedef struct {
    udi_size_t buf_size;
} udi_buf_t;

typedef struct {
    udi_ubit32_t udi_xfer_max;
    udi_ubit32_t udi_xfer_typical;
    udi_ubit32_t udi_xfer_granularity;
    udi_boolean_t udi_xfer_one_piece;
    udi_boolean_t udi_xfer_exact_size;
    udi_boolean_t udi_xfer_no_reorder;
} udi_xfer_constraints_t;

#define UDI_BUF_ALLOC(callback, gcb, init_data, size, path_handle) \
 udi_buf_write(callback, gcb, init_data, size, NULL, 0, 0, path_handle)
#define UDI_BUF_INSERT(callback, gcb, new_data, size, dst_buf, dst_off) \
 udi_buf_write(callback, gcb, new_data, size, dst_buf, dst_off, 0, \
               UDI_NULL_BUF_PATH)
#define UDI_BUF_DELETE(callback, gcb, size, dst_buf, dst_off) \
 udi_buf_write(callback, gcb, NULL, 0, dst_buf, dst_off, size, \
               UDI_NULL_BUF_PARG)
#define UDI_BUF_DUP(callback, gcb, src_buf, path_handle) \
 udi_buf_copy(callback, gcb, src_buf, 0, (src_buf)->buf_size, NULL, 0, 0, \
              path_handle)

typedef void udi_buf_copy_call_t (udi_cb_t *gcb,
                                  udi_buf_t *new_dst_buf);

void udi_buf_copy(udi_buf_copy_call_t *callback,
                  udi_cb_t *gcb,
                  udi_buf_t *src_buf,
                  udi_size_t src_off,
                  udi_size_t src_len,
                  udi_buf_t *dst_buf,
                  udi_size_t dst_off,
                  udi_size_t dst_len,
                  udi_buf_path_t path_handle);

typedef void udi_buf_write_call_t (udi_cb_t *gcb,
                                   udi_buf_t *new_dst_buf);

void udi_buf_write(udi_buf_write_call_t *callback,
                   udi_cb_t *gcb,
                   const void *src_mem,
                   udi_size_t src_len,
                   udi_buf_t *dst_buf,
                   udi_size_t dst_off,
                   udi_size_t dst_len,
                   udi_buf_path_t path_handle);

void udi_buf_read(udi_buf_t *src_buf,
                  udi_size_t src_off,
                  udi_size_t src_len,
                  void *dst_mem);
void udi_buf_free(udi_buf_t *buf);

void udi_buf_best_path(udi_buf_t *buf,
                       udi_buf_path_t *path_handles,
                       udi_ubit8_t npaths,
                       udi_ubit8_t last_fit,
                       udi_ubit8_t *best_fit_array);
#define UDI_BUF_PATH_END 255

typedef void udi_buf_path_alloc_call_t(udi_cb_t *gcb,
                                       udi_buf_path_t new_buf_path);

void udi_buf_path_alloc(udi_buf_path_alloc_call_t *callback,
                        udi_cb_t *gcb);

void udi_buf_path_free(udi_buf_path_t buf_path);

typedef udi_ubit32_t udi_tagtype_t;
#define UDI_BUFTAG_ALL     0xFFFFFFFF
#define UDI_BUFTAG_VALUES  0x000000FF
#define UDI_BUFTAG_UPDATES 0x0000FF00
#define UDI_BUFTAG_STATUS  0x00FF0000
#define UDI_BUFTAG_DRIVERS 0xFF000000

#define UDI_BUFTAG_BE16_CHECKSUM      (1U << 0)

#define UDI_BUFTAG_SET_iBE16_CHECKSUM (1U << 8)
#define UDI_BUFTAG_SET_TCP_CHECKSUM   (1U << 9)
#define UDI_BUFTAG_SET_UDP_CHECKSUM   (1U << 10)

#define UDI_BUFTAG_TCP_CKSUM_GOOD     (1U << 17)
#define UDI_BUFTAG_UDP_CKSUM_GOOD     (1U << 18)
#define UDI_BUFTAG_IP_CKSUM_GOOD      (1U << 19)
#define UDI_BUFTAG_TCP_CKSUM_BAD      (1U << 21)
#define UDI_BUFTAG_UDP_CKSUM_BAD      (1U << 22)
#define UDI_BUFTAG_IP_CKSUM_BAD       (1U << 23)

#define UDI_BUFTAG_DRIVER1            (1U << 24)
#define UDI_BUFTAG_DRIVER2            (1U << 25)
#define UDI_BUFTAG_DRIVER3            (1U << 26)
#define UDI_BUFTAG_DRIVER4            (1U << 27)
#define UDI_BUFTAG_DRIVER5            (1U << 28)
#define UDI_BUFTAG_DRIVER6            (1U << 29)
#define UDI_BUFTAG_DRIVER7            (1U << 30)
#define UDI_BUFTAG_DRIVER8            (1U << 31)

typedef struct {
    udi_tagtype_t tag_type;
    udi_ubit32_t tag_value;
    udi_size_t tag_off;
    udi_size_t tag_len;
} udi_buf_tag_t;

typedef void udi_buf_tag_set_call_t(udi_cb_t *gcb,
                                    udi_buf_t *new_buf);

void udi_buf_tag_set(udi_buf_tag_set_call_t *callback,
                     udi_cb_t *gcb,
                     udi_buf_t *buf,
                     udi_buf_tag_t *tag_array,
                     udi_ubit16_t tag_array_length);

udi_ubit16_t udi_buf_tag_get(udi_buf_t *buf,
                             udi_tagtype_t tag_type,
                             udi_buf_tag_t *tag_array,
                             udi_ubit16_t tag_array_length,
                             udi_ubit16_t tag_start_idx);

udi_ubit32_t udi_buf_tag_compute(udi_buf_t *buf,
                                 udi_size_t off,
                                 udi_size_t len,
                                 udi_tagtype_t tag_type);

typedef void udi_buf_tag_apply_call_t(udi_cb_t *gcb,
                                      udi_buf_t *new_buf);

void udi_buf_tag_apply(udi_buf_tag_apply_call_t *callback,
                       udi_cb_t *gcb,
                       udi_buf_t *buf,
                       udi_tagtype_t tag_type);


typedef struct {
    udi_ubit32_t seconds;
    udi_ubit32_t nanoseconds;
} udi_time_t;

typedef void udi_timer_expired_call_t(udi_cb_t *gcb);

void udi_timer_start(udi_timer_expired_call_t *callback,
                     udi_cb_t *gcb,
                     udi_time_t interval);

typedef void udi_timer_tick_call_t(void *context, udi_ubit32_t nmissed);

void udi_timer_start_repeating(udi_timer_tick_call_t *callback,
                               udi_cb_t *gcb,
                               udi_time_t interval);

void udi_timer_cancel(udi_cb_t *gcb);


udi_timestamp_t udi_time_current(void);

udi_time_t udi_time_between(udi_timestamp_t start_time,
                            udi_timestamp_t end_time);

udi_time_t udi_time_since(udi_timestamp_t start_time);


typedef udi_ubit8_t udi_instance_attr_type_t;
#define UDI_ATTR_NONE    0
#define UDI_ATTR_STRING  1
#define UDI_ATTR_ARRAY8  2
#define UDI_ATTR_UBIT32  3
#define UDI_ATTR_BOOLEAN 4
#define UDI_ATTR_FILE    5

typedef void udi_instance_attr_get_call_t(udi_cb_t *gcb,
                                          udi_instance_attr_type_t attr_type,
                                          udi_size_t actual_length);

void udi_instance_attr_get(udi_instance_attr_get_call_t *callback,
                           udi_cb_t *gcb,
                           const char *attr_name,
                           udi_ubit32_t child_ID,
                           void *attr_value,
                           udi_size_t attr_length);

typedef void udi_instance_attr_set_call_t(udi_cb_t *gcb,
                                          udi_status_t status);

void udi_instance_attr_set(udi_instance_attr_set_call_t *callback,
                           udi_cb_t *gcb,
                           const char *attr_name,
                           udi_ubit32_t child_ID,
                           const void *attr_value,
                           udi_size_t attr_length,
                           udi_ubit8_t attr_type);

#define UDI_INSTANCE_ATTR_DELETE(callback, gcb, attr_name) \
 udi_instance_attr_set(callback, gcb, attr_name, NULL, NULL, 0, UDI_ATTR_NONE)    

#define UDI_MAX_ATTR_NAMELEN 32
#define UDI_MAX_ATTR_SIZE 64

typedef struct {
    char attr_name[UDI_MAX_ATTR_NAMELEN];
    udi_ubit8_t attr_value[UDI_MAX_ATTR_SIZE];
    udi_ubit8_t attr_length;
    udi_instance_attr_type_t attr_type;
} udi_instance_attr_list_t;

#define UDI_ATTR32_SET(aval, v) \
 {udi_ubit32_t vtmp = (v); \
  (aval)[0] = (vtmp) & 0xFF; \
  (aval)[1] = ((vtmp) >> 8) & 0xFF; \
  (aval)[2] = ((vtmp) >> 16) & 0xFF; \
  (aval)[3] = ((vtmp) >> 24) & 0xFF;}
#define UDI_ATTR32_GET(aval) \
 ((aval)[0] + ((aval)[1] << 8) + ((aval)[2] << 16) + ((aval)[3] << 24))
#define UDI_ATTR32_INIT(v) \
 {(v) & 0xFF, ((v) >> 8) & 0xFF, ((v) >> 16) & 0xFF, ((v) >> 24) & 0xFF}


typedef void udi_channel_anchor_call_t(udi_cb_t *gcb,
                                       udi_channel_t anchored_channel);

void udi_channel_anchor(udi_channel_anchor_call_t *callback,
                        udi_cb_t *gcb,
                        udi_channel_t channel,
                        udi_index_t ops_idx,
                        void *channel_context);

typedef void udi_channel_spawn_call_t(udi_cb_t *gcb,
                                      udi_channel_t new_channel);

void udi_channel_spawn(udi_channel_spawn_call_t *callback,
                       udi_cb_t *gcb,
                       udi_channel_t channel,
                       udi_index_t spawn_idx,
                       udi_index_t ops_idx,
                       void *channel_context);

void udi_channel_set_context(udi_channel_t target_channel,
                             void *channel_context);

void udi_channel_op_abort(udi_channel_t target_channel,
                          udi_cb_t *orig_cb);

void udi_channel_close(udi_channel_t channel);

typedef struct {
    udi_cb_t gcb;
    udi_ubit8_t event;
    union {
        struct {
            udi_cb_t *bind_cb;
        } internal_bound;
        struct {
            udi_cb_t *bind_cb;
            udi_ubit8_t parent_ID;
            udi_buf_path_t *path_handles;
        } parent_bound;
        udi_cb_t *orig_cb;
    } params;
} udi_channel_event_cb_t;

#define UDI_CHANNEL_CLOSED     0
#define UDI_CHANNEL_BOUND      1
#define UDI_CHANNEL_OP_ABORTED 2

void udi_channel_event_ind(udi_channel_event_cb_t *cb);
typedef void udi_channel_event_ind_op_t(udi_channel_event_cb_t *cb);

void udi_channel_event_complete(udi_channel_event_cb_t *cb,
                                udi_status_t status);


typedef struct {
    udi_size_t max_legal_alloc;
    udi_size_t max_safe_alloc;
    udi_size_t max_trace_log_formatted_len;
    udi_size_t max_instance_attr_len;
    udi_ubit32_t min_curtime_res;
    udi_ubit32_t min_timer_res;
} udi_limits_t;

#define UDI_MIN_ALLOC_LIMIT 4000
#define UDI_MIN_TRACE_LOG_LIMIT 200
#define UDI_MIN_INSTANCE_ATTR_LIMIT 64

typedef struct {
    udi_index_t region_dx;
    udi_limits_t limits;
} udi_init_context_t;


typedef udi_ubit32_t udi_trevent_t;
#define UDI_TREVENT_LOCAL_PROC_ENTRY (1U << 0)
#define UDI_TREVENT_LOCAL_PROC_EXIT  (1U << 1)
#define UDI_TREVENT_EXTERNAL_ERROR   (1U << 2)

#define UDI_TREVENT_IO_SCHEDULED     (1U << 6)
#define UDI_TREVENT_IO_COMPLETED     (1U << 7)

#define UDI_TREVENT_META_SPECIFIC_1  (1U << 11)
#define UDI_TREVENT_META_SPECIFIC_2  (1U << 12)
#define UDI_TREVENT_META_SPECIFIC_3  (1U << 13)
#define UDI_TREVENT_META_SPECIFIC_4  (1U << 14)
#define UDI_TREVENT_META_SPECIFIC_5  (1U << 15)

#define UDI_TREVENT_INTERNAL_1       (1U << 16)
#define UDI_TREVENT_INTERNAL_2       (1U << 17)
#define UDI_TREVENT_INTERNAL_3       (1U << 18)
#define UDI_TREVENT_INTERNAL_4       (1U << 19)
#define UDI_TREVENT_INTERNAL_5       (1U << 20)
#define UDI_TREVENT_INTERNAL_6       (1U << 21)
#define UDI_TREVENT_INTERNAL_7       (1U << 22)
#define UDI_TREVENT_INTERNAL_8       (1U << 23)
#define UDI_TREVENT_INTERNAL_9       (1U << 24)
#define UDI_TREVENT_INTERNAL_10      (1U << 25)
#define UDI_TREVENT_INTERNAL_11      (1U << 26)
#define UDI_TREVENT_INTERNAL_12      (1U << 27)
#define UDI_TREVENT_INTERNAL_13      (1U << 28)
#define UDI_TREVENT_INTERNAL_14      (1U << 29)
#define UDI_TREVENT_INTERNAL_15      (1U << 30)

#define UDI_TREVENT_LOG              (1U << 31)

void udi_trace_write(udi_init_context_t *init_context,
                     udi_trevent_t trace_event,
                     udi_index_t meta_idx,
                     udi_ubit32_t msgnum,
                     ...);

typedef void udi_log_write_call_t(udi_cb_t *gcb,
                                  udi_status_t correlated_status);

void udi_log_write(udi_log_write_call_t *callback,
                   udi_cb_t *gcb,
                   udi_trevent_t trace_event,
                   udi_ubit8_t severity,
                   udi_index_t meta_idx,
                   udi_status_t original_status,
                   udi_ubit32_t msgnum,
                   ...);
#define UDI_LOG_DISASTER    1
#define UDI_LOG_ERROR       2
#define UDI_LOG_WARNING     3
#define UDI_LOG_INFORMATION 4


void udi_assert(udi_boolean_t expr);

void udi_debug_break(udi_init_context_t *init_context,
                     const char *message);

void udi_debug_printf(const char *format,
                      ...);


typedef const struct __udi_mgmt_ops_t udi_mgmt_ops_t;

typedef const struct {
    udi_mgmt_ops_t *mgmt_ops;
    const udi_ubit8_t *mgmt_op_flags;
    udi_size_t mgmt_scratch_requirement;
    udi_ubit8_t enumeration_attr_list_length;
    udi_size_t rdata_size;
    udi_size_t child_data_size;
    udi_ubit8_t per_parent_paths;
} udi_primary_init_t;

#define UDI_MAX_SCRATCH 4000
#define UDI_OP_LONG_EXE (1U << 0)

typedef const struct {
    udi_index_t region_idx;
    udi_size_t rdata_size;
} udi_secondary_init_t;

typedef void udi_op_t(void);

typedef udi_op_t * const udi_ops_vector_t;

typedef const struct {
    udi_index_t ops_idx;
    udi_index_t meta_idx;
    udi_index_t meta_ops_num;
    udi_size_t chan_context_size;
    udi_ops_vector_t *ops_vector;
    const udi_ubit8_t *op_flags;
} udi_ops_init_t;

typedef const struct {
    udi_index_t cb_idx;
    udi_index_t meta_idx;
    udi_index_t meta_cb_num;
    udi_size_t scratch_requirement;
    udi_size_t inline_size;
    udi_layout_t *inline_layout;
} udi_cb_init_t;

typedef const struct {
    udi_index_t ops_idx;
    udi_index_t cb_idx;
} udi_cb_select_t;

typedef const struct {
    udi_index_t cb_idx;
    udi_size_t scratch_requirement;
} udi_gcb_init_t;

typedef const struct {
    udi_primary_init_t *primary_init_info;
    udi_secondary_init_t *secondary_init_info;
    udi_ops_init_t *ops_init_list;
    udi_cb_init_t *cb_init_list;
    udi_gcb_init_t *gcb_init_list;
    udi_cb_select_t *cb_select_list;
} udi_init_t;

typedef struct {
    void *rdata;
} udi_chan_context;

typedef struct {
    void *rdata;
    udi_ubit32_t child_ID;
} udi_child_chan_context;


typedef struct {
    char attr_name[UDI_MAX_ATTR_NAMELEN];
    udi_ubit8_t attr_min[UDI_MAX_ATTR_SIZE];
    udi_ubit8_t attr_min_len;
    udi_ubit8_t attr_max[UDI_MAX_ATTR_SIZE];
    udi_ubit8_t attr_max_len;
    udi_instance_attr_type_t attr_type;
    udi_ubit32_t attr_stride;
} udi_filter_element_t;

typedef struct {
    udi_cb_t gcb;
    udi_ubit32_t childID;
    void *child_data;
    udi_instance_attr_list_t *attr_list;
    udi_ubit8_t attr_valid_length;
    const udi_filter_element_t *filter_list;
    udi_ubit8_t filter_list_length;
    udi_ubit8_t parent_ID;
} udi_enumerate_cb_t;

/* Special parent_ID filter value: */
#define UDI_ANY_PARENT_ID 0

typedef struct {
    udi_cb_t gcb;
} udi_mgmt_cb_t;

typedef struct {
    udi_cb_t gcb;
    udi_trevent_t trace_mask;
    udi_index_t meta_idx;
} udi_usage_cb_t;

void udi_usage_ind(udi_usage_cb_t *cb,
                   udi_ubit8_t resource_level);
void udi_usage_res(udi_usage_cb_t *cb);
typedef void udi_usage_ind_op_t(udi_usage_cb_t *cb,
                                udi_ubit8_t resource_level);
/* resource_level: */
#define UDI_RESOURCES_CRITICAL  1
#define UDI_RESOURCES_LOW       2
#define UDI_RESOURCES_NORMAL    3
#define UDI_RESOURCES_PLENTIFUL 4

void udi_enumerate_req(udi_enumerate_cb_t *cb,
                       udi_ubit8_t enumeration_level);
void udi_enumerate_ack(udi_enumerate_cb_t *cb,
                       udi_ubit8_t enumeration_result,
                       udi_index_t ops_idx);
void udi_devmgmt_req(udi_mgmt_cb_t *cb,
                     udi_ubit8_t mgmt_op,
                     udi_ubit8_t parent_ID);
void udi_devmgmt_ack(udi_mgmt_cb_t *cb,
                     udi_ubit8_t flags,
                     udi_status_t status);
void udi_final_cleanup_req(udi_mgmt_cb_t *cb);
void udi_final_cleanup_ack(udi_mgmt_cb_t *cb);

typedef void udi_enumerate_req_op_t(udi_enumerate_cb_t *cb,
                                    udi_ubit8_t enumeration_level);
typedef void udi_devmgmt_req_op_t(udi_mgmt_cb_t *cb,
                                  udi_ubit8_t mgmt_op,
                                  udi_ubit8_t parent_ID);
typedef void udi_final_cleanup_req_op_t(udi_mgmt_cb_t *cb);

/* enumeration_level: */
#define UDI_ENUMERATE_START        1
#define UDI_ENUMERATE_START_RESCAN 2
#define UDI_ENUMERATE_NEXT         3
#define UDI_ENUMERATE_NEW          4
#define UDI_ENUMERATE_DIRECTED     5
#define UDI_ENUMERATE_RELEASE      6

/* enumeration_result: */
#define UDI_ENUMERATE_OK           0
#define UDI_ENUMERATE_LEAF         1
#define UDI_ENUMERATE_DONE         2
#define UDI_ENUMERATE_RESCAN       3
#define UDI_ENUMERATE_REMOVED      4
#define UDI_ENUMERATE_REMOVED_SELF 5
#define UDI_ENUMERATE_RELEASED     6
#define UDI_ENUMERATE_FAILED       255

/* mgmt_op: */
#define UDI_DMGMT_PREPARE_TO_SUSPEND 1
#define UDI_DMGMT_SUSPEND            2
#define UDI_DMGMT_SHUTDOWN           3
#define UDI_DMGMT_PARENT_SUSPENDED   4
#define UDI_DMGMT_RESUME             5
#define UDI_DMGMT_UNBIND             6

/* flags: */
#define UDI_DMGMT_NONTRANSPARENT (1U << 0)

/* Meta-specific status code: */
#define UDI_DMGMT_STAT_ROUTING_CHANGE (UDI_STAT_META_SPECIFIC | 1)

/* Proxies: */
udi_usage_ind_op_t udi_static_usage;
udi_enumerate_req_op_t udi_enumerate_no_children;

struct __udi_mgmt_ops_t {
    udi_usage_ind_op_t *usage_ind_op;
    udi_enumerate_req_op_t *enumerate_req_op;
    udi_devmgmt_req_op_t *devmgmt_req_op;
    udi_final_cleanup_req_op_t *final_cleanup_req_op;
};


typedef udi_ubit8_t udi_gio_op_t;
#define UDI_GIO_OP_CUSTOM 16
#define UDI_GIO_OP_MAX    64

#define UDI_GIO_DIR_READ  (1U << 6)
#define UDI_GIO_DIR_WRITE (1U << 7)

#define UDI_GIO_OP_READ    UDI_GIO_DIR_READ
#define UDI_GIO_OP_WRITE   UDI_GIO_DIR_WRITE

/* Diagnostic values. */
#define UDI_GIO_OP_DIAG_ENABLE   1
#define UDI_GIO_OP_DIAG_DISABLE  2
#define UDI_GIO_OP_DIAG_RUN_TEST (3 | UDI_GIO_DIR_READ)

typedef struct {
    udi_cb_t gcb;
    udi_xfer_constraints_t xfer_constraints;
} udi_gio_bind_cb_t;

typedef struct {
    udi_cb_t gcb;
    udi_gio_op_t op;
    void *tr_params;
    udi_buf_t *data_buf;
} udi_gio_xfer_cb_t;

typedef struct {
    udi_cb_t gcb;
    udi_ubit8_t event_code;
    void *event_params;
} udi_gio_event_cb_t;

#define UDI_GIO_BIND_CB_NUM  1
#define UDI_GIO_XFER_CB_NUM  2
#define UDI_GIO_EVENT_CB_NUM 3

typedef struct {
    udi_ubit32_t offset_lo;
    udi_ubit32_t offset_hi;
} udi_gio_rw_params_t;

typedef struct {
    udi_ubit8_t test_num;
    udi_ubit8_t test_params_size;
} udi_gio_diag_params_t;

void udi_gio_bind_req(udi_gio_bind_cb_t *cb);
void udi_gio_bind_ack(udi_gio_bind_cb_t *cb,
                      udi_ubit32_t device_size_lo,
                      udi_ubit32_t device_size_hi,
                      udi_status_t status);
void udi_gio_unbind_req(udi_gio_bind_cb_t *cb);
void udi_gio_unbind_ack(udi_gio_bind_cb_t *cb);
void udi_gio_xfer_req(udi_gio_xfer_cb_t *cb);
void udi_gio_xfer_ack(udi_gio_xfer_cb_t *cb);
void udi_gio_xfer_nak(udi_gio_xfer_cb_t *cb,
                      udi_status_t status);
void udi_gio_event_ind(udi_gio_event_cb_t *cb);
void udi_gio_event_res(udi_gio_event_cb_t *cb);

typedef void udi_gio_bind_req_op_t(udi_gio_bind_cb_t *cb);
typedef void udi_gio_bind_ack_op_t(udi_gio_bind_cb_t *cb,
                                   udi_ubit32_t device_size_lo,
                                   udi_ubit32_t device_size_hi,
                                   udi_status_t status);
typedef void udi_gio_unbind_req_op_t(udi_gio_bind_cb_t *cb);
typedef void udi_gio_unbind_ack_op_t(udi_gio_bind_cb_t *cb);
typedef void udi_gio_xfer_req_op_t(udi_gio_xfer_cb_t *cb);
typedef void udi_gio_xfer_ack_op_t(udi_gio_xfer_cb_t *cb);
typedef void udi_gio_xfer_nak_op_t(udi_gio_xfer_cb_t *cb,
                                   udi_status_t status);
typedef void udi_gio_event_ind_op_t(udi_gio_event_cb_t *cb);
typedef void udi_gio_event_res_op_t(udi_gio_event_cb_t *cb);

typedef const struct {
    udi_channel_event_ind_op_t *channel_event_ind_op;
    udi_gio_bind_req_op_t *gio_bind_req_op;
    udi_gio_unbind_req_op_t *gio_unbind_req_op;
    udi_gio_xfer_req_op_t *gio_xfer_req_op;
    udi_gio_event_res_op_t *gio_event_res_op;
} udi_gio_provider_ops_t;

#define UDI_GIO_PROVIDER_OPS_NUM 1

typedef const struct {
    udi_channel_event_ind_op_t *channel_event_ind_op;
    udi_gio_bind_ack_op_t *gio_bind_ack_op;
    udi_gio_unbind_ack_op_t *gio_unbind_ack_op;
    udi_gio_xfer_ack_op_t *gio_xfer_ack_op;
    udi_gio_xfer_nak_op_t *gio_xfer_nak_op;
    udi_gio_event_ind_op_t *gio_event_ind_op;
} udi_gio_client_ops_t;

#define UDI_GIO_CLIENT_OPS_NUM 2

/* Proxies. */
udi_gio_event_ind_op_t udi_gio_event_ind_unused;
udi_gio_event_res_op_t udi_gio_event_res_unused;


typedef void udi_mei_direct_stub_t(udi_op_t *op,
                                   udi_cb_t *gcb,
                                   va_list arglist);
typedef void udi_mei_backend_stub_t(udi_op_t *op,
                                    udi_cb_t *gcb,
                                    void *marshal_space);

typedef udi_ubit8_t udi_mei_enumeration_rank_func_t(
    udi_ubit32_t attr_device_match,
    void **attr_value_list);


typedef const struct {
    const char *op_name;
    udi_ubit8_t op_category;
    udi_ubit8_t op_flags;
    udi_index_t meta_cb_num;
    udi_index_t completion_ops_num;
    udi_index_t completion_vec_idx;
    udi_index_t exception_ops_num;
    udi_index_t exception_vec_idx;
    udi_mei_direct_stub_t *direct_stub;
    udi_mei_backend_stub_t *backend_stub;
    udi_layout_t *visible_layout;
    udi_layout_t *marshal_layout;
} udi_mei_op_template_t;

#define UDI_MEI_OPCAT_REQ 1
#define UDI_MEI_OPCAT_ACK 2
#define UDI_MEI_OPCAT_NAK 3
#define UDI_MEI_OPCAT_IND 4
#define UDI_MEI_OPCAT_RES 5
#define UDI_MEI_OPCAT_RDY 6

#define UDI_MEI_OP_ABORTABLE    (1U << 0)
#define UDI_MEI_OP_RECOVERABLE  (1U << 1)
#define UDI_MEI_OP_STATE_CHANGE (1U << 2)

#define UDI_MEI_MAX_VISIBLE_SIZE 2000
#define UDI_MEI_MAX_MARSHAL_SIZE 4000

typedef const struct {
    udi_index_t meta_ops_num;
    udi_ubit8_t relationship;
    udi_mei_op_template_t *op_template_list;
} udi_mei_ops_vec_template_t;

#define UDI_MEI_REL_INITIATOR (1U << 0)
#define UDI_MEI_REL_BIND      (1U << 1)
#define UDI_MEI_REL_EXTERNAL  (1U << 2)
#define UDI_MEI_REL_INTERNAL  (1U << 3)
#define UDI_MEI_REL_SINGLE    (1U << 4)

typedef const struct {
    udi_mei_ops_vec_template_t *ops_vec_template_list;
    udi_mei_enumeration_rank_func_t *mei_enumeration_rank;
} udi_mei_init_t;

void udi_mei_call(udi_cb_t *gcb,
                  udi_mei_init_t *meta_info,
                  udi_index_t meta_ops_num,
                  udi_index_t vec_idx,
                  ...);
void udi_mei_driver_error(udi_cb_t *gcb,
                          udi_mei_init_t *meta_info,
                          udi_index_t meta_ops_num,
                          udi_index_t vec_idx);

#define UDI_MEI_STUBS\
(op_name, cb_type, argc, args, arg_types, arg_va_list, meta_ops_num, vec_idx) \
 _UDI_MEI_STUBS(op_name, cb_type, argc, args, arg_types, arg_va_list, \
                meta_ops_num, vec_idx, &udi_meta_info)

#define _UDI_ARG_LIST_0()
#define _UDI_ARG_LIST_1(a) , a arg1
#define _UDI_ARG_LIST_2(a, b) , a arg1, b arg2
#define _UDI_ARG_LIST_3(a, b, c) , a arg1, b arg2, c arg3
#define _UDI_ARG_LIST_4(a, b, c, d) , a arg1, b arg2, c arg3, d arg4
#define _UDI_ARG_LIST_5(a, b, c, d, e) , a arg1, b arg2, \
 c arg3, d arg4, e_arg5
#define _UDI_ARG_LIST_6(a, b, c, d, e, f) , a arg1, b arg2, \
 c arg3, d arg4, e_arg5, f arg6
#define _UDI_ARG_LIST_7(a, b, c, d, e, f, g) , a arg1, b arg2, \
 c arg3, d arg4, e_arg5, f arg6, g arg7

#define _UDI_VA_ARGS_0() (void)arglist;
#define _UDI_VA_ARGS_1(a, va_a) \
    a arg1 = UDI_VA_ARG(arglist, a, va_a);
#define _UDI_VA_ARGS_2(a, b, va_a, va_b) \
    a arg1 = UDI_VA_ARG(arglist, a, va_a); \
    b arg2 = UDI_VA_ARG(arglist, b, va_b);
#define _UDI_VA_ARGS_3(a, b, c, va_a, va_b, va_c) \
    a arg1 = UDI_VA_ARG(arglist, a, va_a); \
    b arg2 = UDI_VA_ARG(arglist, b, va_b); \
    c arg3 = UDI_VA_ARG(arglist, c, va_c);
#define _UDI_VA_ARGS_4(a, b, c, d, va_a, va_b, va_c, va_d) \
    a arg1 = UDI_VA_ARG(arglist, a, va_a); \
    b arg2 = UDI_VA_ARG(arglist, b, va_b); \
    c arg3 = UDI_VA_ARG(arglist, c, va_c); \
    d arg4 = UDI_VA_ARG(arglist, d, va_d);
#define _UDI_VA_ARGS_5(a, b, c, d, e, va_a, va_b, va_c, va_d, va_e) \
    a arg1 = UDI_VA_ARG(arglist, a, va_a); \
    b arg2 = UDI_VA_ARG(arglist, b, va_b); \
    c arg3 = UDI_VA_ARG(arglist, c, va_c); \
    d arg4 = UDI_VA_ARG(arglist, d, va_d); \
    e arg5 = UDI_VA_ARG(arglist, e, va_e);
#define _UDI_VA_ARGS_6(a, b, c, d, e, f, va_a, va_b, va_c, va_d, va_e, va_f) \
    a arg1 = UDI_VA_ARG(arglist, a, va_a); \
    b arg2 = UDI_VA_ARG(arglist, b, va_b); \
    c arg3 = UDI_VA_ARG(arglist, c, va_c); \
    d arg4 = UDI_VA_ARG(arglist, d, va_d); \
    e arg5 = UDI_VA_ARG(arglist, e, va_e); \
    f arg6 = UDI_VA_ARG(arglist, f, va_f);
#define _UDI_VA_ARGS_7\
(a, b, c, d, e, f, g, va_a, va_b, va_c, va_d, va_e, va_f, va_g) \
    a arg1 = UDI_VA_ARG(arglist, a, va_a); \
    b arg2 = UDI_VA_ARG(arglist, b, va_b); \
    c arg3 = UDI_VA_ARG(arglist, c, va_c); \
    d arg4 = UDI_VA_ARG(arglist, d, va_d); \
    e arg5 = UDI_VA_ARG(arglist, e, va_e); \
    f arg6 = UDI_VA_ARG(arglist, f, va_f); \
    g arg7 = UDI_VA_ARG(arglist, g, va_g);

#define _UDI_L_0() (
#define _UDI_L_1(a) (a,
#define _UDI_L_2(a, b) (a, b,
#define _UDI_L_3(a, b, c) (a, b, c,
#define _UDI_L_4(a, b, c, d) (a, b, c, d,
#define _UDI_L_5(a, b, c, d, e) (a, b, c, d, e,
#define _UDI_L_6(a, b, c, d, e, f) (a, b, c, d, e, f,
#define _UDI_L_7(a, b, c, d, e, f, g) (a, b, c, d, e, f, g,
 
#define _UDI_R_0() )
#define _UDI_R_1(a) a)
#define _UDI_R_2(a, b) a, b)
#define _UDI_R_3(a, b, c) a, b, c)
#define _UDI_R_4(a, b, c, d) a, b, c, d)
#define _UDI_R_5(a, b, c, d, e) a, b, c, d, e)
#define _UDI_R_6(a, b, c, d, e, f) a, b, c, d, e, f)
#define _UDI_R_7(a, b, c, d, e, f, g) a, b, c, d, e, f, g)

#define __UDI_VA_ARGLIST(argc, list) _UDI_VA_ARGS_##argc list
#define _UDI_CONCAT_LIST(argc, list1, list2) \
 _UDI_L_##argc list1 _UDI_R_##argc list2
#define _UDI_VA_ARGLIST(argc, list1, list2) \
 __UDI_VA_ARGLIST(argc, _UDI_CONCAT_LIST(argc, list1, list2))

#define _UDI_ARG_VARS_0
#define _UDI_ARG_VARS_1 , arg1
#define _UDI_ARG_VARS_2 , arg1, arg2
#define _UDI_ARG_VARS_3 , arg1, arg2, arg3
#define _UDI_ARG_VARS_4 , arg1, arg2, arg3, arg4
#define _UDI_ARG_VARS_5 , arg1, arg2, arg3, arg4, arg5
#define _UDI_ARG_VARS_6 , arg1, arg2, arg3, arg4, arg5, arg6
#define _UDI_ARG_VARS_7 , arg1, arg2, arg3, arg4, arg5, arg6, arg7

#define _UDI_ARG_MEMBERS_0() \
 char ignored;
#define _UDI_ARG_MEMBERS_1(a) \
 a arg1;
#define _UDI_ARG_MEMBERS_2(a, b) \
 a arg1; b arg2;
#define _UDI_ARG_MEMBERS_3(a, b, c) \
 a arg1; b arg2; c arg3;
#define _UDI_ARG_MEMBERS_4(a, b, c, d) \
 a arg1; b arg2; c arg3; d arg4;
#define _UDI_ARG_MEMBERS_5(a, b, c, d, e) \
 a arg1; b arg2; c arg3; d arg4; e arg5;
#define _UDI_ARG_MEMBERS_6(a, b, c, d, e, f) \
 a arg1; b arg2; c arg3; d arg4; e arg5; f arg6;
#define _UDI_ARG_MEMBERS_7(a, b, c, d, e, f, g) \
 a arg1; b arg2; c arg3; d arg4; e arg5; f arg6; g arg7;

#define _UDI_MP_ARGS_0
#define _UDI_MP_ARGS_1 , mp->arg1
#define _UDI_MP_ARGS_2 , mp->arg1, mp->arg2
#define _UDI_MP_ARGS_3 , mp->arg1, mp->arg2, mp->arg3
#define _UDI_MP_ARGS_4 , mp->arg1, mp->arg2, mp->arg3, mp->arg4
#define _UDI_MP_ARGS_5 , mp->arg1, mp->arg2, mp->arg3, mp->arg4, mp->arg5
#define _UDI_MP_ARGS_6 , mp->arg1, mp->arg2, mp->arg3, mp->arg4, mp->arg5, \
 mp->arg6
#define _UDI_MP_ARGS_7 , mp->arg1, mp->arg2, mp->arg3, mp->arg4, mp->arg5, \
 mp->arg6, mp->arg7

#define _UDI_MEI_STUBS\
(op_name, cb_type, argc, args, arg_types, arg_va_list,\
 meta_ops_num, vec_idx, meta_idx) \
 void op_name(cb_type *cb _UDI_ARG_LIST_##argc arg_types \
              ) \
 { \
     udi_mei_call(UDI_GCB(cb), meta_idx, meta_ops_num, vec_idx \
                  _UDI_ARG_VARS_##argc \
                  ); \
 } \
 static void op_name##_direct(udi_op_t *op, udi_cb_t *gcb, va_list arglist)\
 { \
    _UDI_VA_ARGLIST(argc, arg_types, arg_va_list); \
    (*(op_name##_op_t *)op)(UDI_MCB(gcb, cb_type) _UDI_ARG_VARS_##argc); \
 } \
 static void op_name##_backend(udi_op_t *op, udi_cb_t *gcb, \
                               void *marshal_space)\
 { \
    struct op_name##_marshal { \
        _UDI_ARG_MEMBERS_##argc arg_types \
    } *mp = marshal_space; \
    (*(op_name##_op_t *\
       )op)(UDI_MCB(gcb, cb_type) _UDI_MP_ARGS_##argc \
       ); \
 }

/* Utility functions. */
udi_size_t udi_strlen(const char *s);

char *udi_strcat(char *s1, const char *s2);
char *udi_strncat(char *s1, const char *s2, udi_size_t n);

udi_sbit8_t udi_strcmp(const char *s1, const char *s2);
udi_sbit8_t udi_strncmp(const char *s1, const char *s2, udi_size_t n);
udi_sbit8_t udi_memcmp(const char *s1, const char *s2, udi_size_t n);

char *udi_strcpy(char *s1, const char *s2);
char *udi_strncpy(char *s1, const char *s2, udi_size_t n);
void *udi_memcpy(void *s1, const void *s2, udi_size_t n);
void *udi_memmove(void *s1, const void *s2, udi_size_t n);

char *udi_strncpy_rtrim(char *s1, const char *s2, udi_size_t n);

char *udi_strchr(const char *s, char c);
char *udi_strrchr(const char *s, char c);
void *udi_memchr(const void *s, udi_ubit8_t c, udi_size_t n);

void *udi_memset(void *s, udi_ubit8_t c, udi_size_t n);

udi_ubit32_t udi_strtou32(const char *s, char **endptr, int base);

udi_size_t udi_snprintf(char *s,
                        udi_size_t max_bytes,
                        const char *format,
                        ...);
udi_size_t udi_vsnprintf(char *s,
                         udi_size_t max_bytes,
                         const char *format,
                         va_list ap);


typedef struct udi_queue {
    struct udi_queue *next;
    struct udi_queue *prev;
} udi_queue_t;

void udi_enqueue(udi_queue_t *new_el, udi_queue_t *old_el);
udi_queue_t *udi_dequeue(udi_queue_t *element);

#define UDI_QUEUE_INIT(listhead) \
 ((listhead)->next = (listhead)->prev = (listhead))
#define UDI_QUEUE_EMPTY(listhead) \
 ((listhead)->next == (listhead))

#define UDI_ENQUEUE_HEAD(listhead, element) \
 udi_enqueue((element), (listhead))
#define UDI_ENQUEUE_TAIL(listhead, element) \
 udi_enqueue((element), (listhead)->prev)
#define UDI_INSERT_AFTER(old_el, new_el) \
 udi_enqueue((new_el), (old_el))
#define UDI_INSERT_BEFORE(old_el, new_el) \
 udi_enqueue((new_el), (old_el)->prev)

#define UDI_DEQUEUE_HEAD(listhead) \
 udi_dequeue((listhead)->next)
#define UDI_DEQUEUE_TAIL(listhead) \
 udi_dequeue((listhead)->prev)
#define UDI_QUEUE_REMOVE(element) \
 ((void)udi_dequeue(element))

#define UDI_FIRST_ELEMENT(listhead) \
 ((listhead)->next)
#define UDI_LAST_ELEMENT(listhead) \
 ((listhead)->prev)
#define UDI_NEXT_ELEMENT(element) \
 ((element)->next)
#define UDI_PREV_ELEMENT(element) \
 ((element)->prev)

#define UDI_QUEUE_FOREACH(listhead, element, tmp) \
 for ((element) = UDI_FIRST_ELEMENT(listhead); \
      ((tmp) = UDI_NEXT_ELEMENT(element)), \
      ((element) != (listhead)); \
      (element) = (tmp))

#define UDI_BASE_STRUCT(memberp, struct_type, member_name) \
 ((struct_type *)((char *)(memberp) - offsetof(struct_type, member_name)))


#define UDI_BFMASK(p, len) \
 (((1U << (len)) - 1) << p)
#define UDI_BFGET(val, p, len) \
 (((udi_ubit8_t)(val) >> (p)) & ((1U << (len)) - 1))
#define UDI_BFSET(val, p, len, dst) \
 ((dst) = (((dst) & ~UDI_BFMASK(p, len)) \
           | (((udi_ubit8_t)(val) << (p)) & UDI_BFMASK(p, len)))

#define UDI_MBGET(N, structp, field) \
 UDI_MBGET_##N(structp, field)
#define UDI_MBGET_2(structp, field) \
 ((structp)->field##0 | ((structp)->field##1 << 8))
#define UDI_MBGET_3(structp, field) \
 ((structp)->field##0 | ((structp)->field##1 << 8) \
  | ((structp)->field##2 << 16))
#define UDI_MBGET_4(structp, field) \
 ((structp)->field##0 | ((structp)->field##1 << 8) \
  | ((structp)->field##2 << 16) | ((structp)->field##3 << 24))

#define UDI_MBSET(N, structp, field, val) \
 UDI_MBSET_##N(structp, field, val)
#define UDI_MBSET_2(structp, field, val) \
 ((structp)->field##0 = (val) & 0xFF, \
  (structp)->field##1 = ((val) >> 8) & 0xFF)
#define UDI_MBSET_3(structp, field, val) \
 ((structp)->field##0 = (val) & 0xFF, \
  (structp)->field##1 = ((val) >> 8) & 0xFF, \
  (structp)->field##2 = ((val) >> 16) & 0xFF)
#define UDI_MBSET_4(structp, field, val) \
 ((structp)->field##0 = (val) & 0xFF, \
  (structp)->field##1 = ((val) >> 8) & 0xFF, \
  (structp)->field##2 = ((val) >> 16) & 0xFF, \
  (structp)->field##3 = ((val) >> 24) & 0xFF)

#define UDI_ENDIAN_SWAP_16(data16) \
  ((((data16) & 0x00FF) << 8) \
   | (((data16) >> 8) & 0x00FF))
#define UDI_ENDIAN_SWAP_32(data32) \
  ((((data32) & 0x000000FF) << 24) \
   | (((data32) & 0x0000FF00) << 8) \
   | (((data32) >> 8) & 0x0000FF00) \
   | (((data32) >> 24) & 0x000000FF))

void udi_endian_swap(const void *src,
                     void *dst,
                     udi_ubit8_t swap_size,
                     udi_ubit8_t stride,
                     udi_ubit16_t rep_count);

#define UDI_ENDIAN_SWAP_ARRAY(src, element_size, count) \
 udi_endian_swap(src, src, element_size, element_size, count)
