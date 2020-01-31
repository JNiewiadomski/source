#ifndef _DOSINT_H
#define _DOSINT_H

// *********************************
// * DOS Kernel Interrupt Services *
// *********************************

#define PGM_TERMINATE	0x20
#define BDOS_REQ 	0x21
#define TERMINATE_ADDR	0x22
#define CB_EXIT_ADDR	0x23
#define CRITICAL_ERR	0x24
#define ABS_DISK_READ	0x25
#define ABS_DISK_WRITE	0x26
#define PGM_TERM_RES	0x27

// *********************
// * Int 21H Functions *
// *********************

#define PGM		0x00
#define KEYIN_ECHO_CB	0x01
#define DSPY_CHAR	0x02
#define AUX_IN		0x03
#define AUX_OUT		0x04
#define PRNT_OUT	0x05
#define KEYIN_ECHO	0x06
#define KEYIN		0x07
#define KEYIN_CB	0x08
#define DSPY_STR	0x09
#define KEYIN_BUF	0x0A
#define CH_READY	0x0B
#define GET_CH		0x0C
#define DISK_RESET	0x0D
#define SELECT_DISK	0x0E
#define OPEN_FILE	0x0F
#define CLOSE_FILE	0x10
#define FIRST_FM	0x11
#define NEXT_FM		0x12
#define DELETE_FILE	0x13
#define READ_SEQ	0x14
#define WRITE_SEQ	0x15
#define CREATE_FILE	0x16
#define RENAME_FILE	0x17

#define CURRENT_DISK	0x19
#define SET_DTA		0x1A
#define DFLT_FAT_INFO	0x1B
#define FAT_INFO	0x1C

#define READ_RANDOM	0x21
#define WRITE_RANDOM	0x22
#define FILE_SIZE	0x23

#define SET_IVEC	0x25
#define NEW_PGM_SEG	0x26
#define RAND_BLK_READ	0x27
#define RAND_BLK_WRITE	0x28
#define PARSE_FILENAME	0x29
#define GET_DATE	0x2A
#define SET_DATE	0x2B
#define GET_TIME	0x2C
#define SET_TIME	0x2D
#define TOGGLE_VERIFY	0x2E
#define GET_DTA		0x2F
#define DOS_VERSION	0x30
#define PGM_TERM_KEEP	0x31

#define CB_CHECK	0x33

#define GET_IVEC	0x35
#define GET_FREE_SPACE	0x36

#define INTL_INFO	0x38
#define MKDIR		0x39
#define RMDIR		0x3A
#define CHDIR		0x3B
#define CREAT		0x3C
#define OPEN_FD		0x3D
#define CLOSE_FD	0x3E

#define WRITE_FD	0x40
#define UNLINK		0x41
#define LSEEK		0x42
#define CHMOD		0x43
#define IOCTL		0x44
#define DUP		0x45
#define FORCE_DUP	0x46
#define GET_CUR_DIR	0x47
#define ALLOC		0x48
#define FREE		0x49
#define SET_BLOCK	0x4A
#define EXEC		0x4B
#define EXIT		0x4C
#define WAIT		0x4D
#define FIND_FIRST	0x4E
#define FIND_NEXT	0x4F

#define VERIFY_STATE	0x54

#define RENAME		0x56
#define FILE_MTIME	0x57

#endif
