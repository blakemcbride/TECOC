/*
 * ZFirst.c
 *
 * This function is used to implement memory validity checking
 * for TURBO-C.  It is only needed if CHECKSUM_CODE is defined.
 *
 * Link this as the first object module in the TLINK command line.
 * The CHECKSUM_CODE routines use the address of ZFirst() to get
 * an idea of where the first function is in memory.
 *
 * To include this routine, specify -DMAP on the MAKE command line.
 */
 void ZFirst (void)
 {
 }
