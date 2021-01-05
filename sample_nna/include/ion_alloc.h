#ifndef _ION_ALLOC_H_
#define _ION_ALLOC_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>
#include <asm-generic/ioctl.h>

#if defined(CONF_KERNEL_VERSION_3_10) || defined(CONF_KERNEL_VERSION_4_4) || defined(CONF_KERNEL_VERSION_4_9)
    typedef int qg_ion_user_handle_t;
#else
    typedef void* qg_ion_user_handle_t;
#endif


typedef struct qg_ion_allocation_info {
    size_t qg_len;
    size_t qg_align;
    unsigned int qg_heap_id_mask;
    unsigned int flags;
    qg_ion_user_handle_t handle;
} qg_ion_allocation_info_t;

typedef struct ion_handle_data {
    qg_ion_user_handle_t handle;
} ion_handle_data_t ;

typedef struct qg_ion_fd_data {
    qg_ion_user_handle_t handle;
    int qg_fd;
} ion_fd_data_t;

typedef struct qg_ion_custom_info {
    unsigned int qg_cmd;
    unsigned long qg_arg;
} ion_custom_data_t;

typedef struct SEAN_PHYS_DATA {
    qg_ion_user_handle_t handle;
    unsigned int  phys_addr;
    unsigned int  size;
} sean_phys_data;


typedef struct {
    long    start;
    long    end;
} sean_cache_range;

#define SZ_64M 0x04000000
#define SZ_4M 0x00400000
#define SZ_1M 0x00100000
#define SZ_64K 0x00010000
#define SZ_4k 0x00001000
#define SZ_1k 0x00000400

enum ion_heap_type {
    QG_ION_SYSTEM_HEAP_TYPE,
    QG_ION_SYSTEM_CONTIG_HEAP_TYPE,
    QG_ION_CARVEOUT_HEAP_TYPE,
    QG_ION_TYPE_HEAP_CHUNK,
    QG_ION_TYPE_HEAP_DMA,
    QG_ION_TYPE_HEAP_CUSTOM,

    QG_ION_TYPE_HEAP_SECURE,

    QG_ION_NUM_HEAPS = 16,
};

#define QG_MEM_ION_IOC_MAGIC 'I'
#define QG_MEM_ION_IOC_ALLOC _IOWR(QG_MEM_ION_IOC_MAGIC, \
		0, struct qg_ion_allocation_info)
#define QG_MEM_ION_IOC_FREE _IOWR(QG_MEM_ION_IOC_MAGIC, 1, \
		struct ion_handle_data)
#define QG_MEM_ION_IOC_MAP _IOWR(QG_MEM_ION_IOC_MAGIC, 2, \
		struct qg_ion_fd_data)
#define QG_MEMION_IOC_SHARE _IOWR(QG_MEM_ION_IOC_MAGIC, 4, \
		struct ion_fd_data)
#define QG_MEMION_IOC_IMPORT _IOWR(QG_MEM_ION_IOC_MAGIC, 5, \
		struct ion_fd_data)
#define QG_MEMION_IOC_SYNC _IOWR(QG_MEM_ION_IOC_MAGIC, 7, \
		struct ion_fd_data)
#define QG_MEM_ION_IOC_CUSTOM _IOWR(QG_MEM_ION_IOC_MAGIC, 6, \
		struct qg_ion_custom_info)

#define QG_MEM_ENGINE_REQ 0x206
#define QG_MEM_ENGINE_REL 0x207
#define QG_MEM_GET_IOMMU_ADDR	0x502
#define QG_MEM_FREE_IOMMU_ADDR	0x503

#define QG_ION_CACHED_FLAG 1
#define QG_ION_CACHED_NEEDS_SYNC_FLAG 2

#define ION_IOC_SEAN_FLUSH_RANGE           5
#define ION_IOC_SEAN_FLUSH_ALL             6
#define ION_IOC_SEAN_PHYS_ADDR             7
#define ION_IOC_SEAN_DMA_COPY              8

#define ION_IOC_SEAN_TEE_ADDR              17
#define QG_ION_SYSTEM_HEAP_MASK        (1 << QG_ION_SYSTEM_HEAP_TYPE)
#define QG_ION_SYSTEM_CONTIG_HEAP_MASK    (1 << QG_ION_SYSTEM_CONTIG_HEAP_TYPE)
#define QG_ION_CARVEOUT_HEAP_MASK        (1 << QG_ION_CARVEOUT_HEAP_TYPE)
#define QG_ION_DMA_HEAP_MASK        (1 << QG_ION_TYPE_HEAP_DMA)
#define QG_ION_SECURE_HEAP_MASK      (1 << QG_ION_TYPE_HEAP_SECURE)

#endif

