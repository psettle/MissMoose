/*
file: mm_lidar_region.h
brief:
notes:
*/

#ifndef MM_LIDAR_REGION_H
#define MM_LIDAR_REGION_H

/**********************************************************
                         ENUMS
**********************************************************/

typedef enum
{
    LIDAR_REGION_REGION_NONE, /* The lidar is currently 'detecting' something past the edge of the network */
    
    LIDAR_REGION_REGION_0,    /* The lidar is currently detecting something in the region in front of it */
    LIDAR_REGION_REGION_1,    /* The lidar is currently detecting something in the region 1 past it */
    
    LIDAR_REGION_COUNT
} lidar_region_t;

#endif /* MM_LIDAR_REGION_H */
