/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2011 Infineon Technologies
 *
 * Authors:
 * Peter Huewe <huewe.external@infineon.com>
 *
 * Version: 2.1.1
 *
 * Description:
 * Device driver for TCG/TCPA TPM (trusted platform module).
 * Specifications at www.trustedcomputinggroup.org
 *
 * It is based on the Linux kernel driver tpm.c from Leendert van
 * Dorn, Dave Safford, Reiner Sailer, and Kyleen Hall.
 */

#ifndef _TPM_TIS_I2C_H
#define _TPM_TIS_I2C_H

#include <linux/compiler.h>
#include <linux/types.h>

struct tpm_tis_phy_ops {
	int (*read_bytes)(struct udevice *udev, u32 addr, u16 len,
			  u8 *result);
	int (*write_bytes)(struct udevice *udev, u32 addr, u16 len,
			   const u8 *value);
	int (*read16)(struct udevice *udev, u32 addr, u16 *result);
	int (*read32)(struct udevice *udev, u32 addr, u32 *result);
	int (*write32)(struct udevice *udev, u32 addr, u32 src);
};

enum tis_int_flags {
	TPM_GLOBAL_INT_ENABLE = 0x80000000,
	TPM_INTF_BURST_COUNT_STATIC = 0x100,
	TPM_INTF_CMD_READY_INT = 0x080,
	TPM_INTF_INT_EDGE_FALLING = 0x040,
	TPM_INTF_INT_EDGE_RISING = 0x020,
	TPM_INTF_INT_LEVEL_LOW = 0x010,
	TPM_INTF_INT_LEVEL_HIGH = 0x008,
	TPM_INTF_LOCALITY_CHANGE_INT = 0x004,
	TPM_INTF_STS_VALID_INT = 0x002,
	TPM_INTF_DATA_AVAIL_INT = 0x001,
};

#define TPM_ACCESS(l)                   (0x0000 | ((l) << 12))
#define TPM_INT_ENABLE(l)               (0x0008 | ((l) << 12))
#define TPM_STS(l)                      (0x0018 | ((l) << 12))
#define TPM_DATA_FIFO(l)                (0x0024 | ((l) << 12))
#define TPM_DID_VID(l)                  (0x0F00 | ((l) << 12))
#define TPM_RID(l)                      (0x0F04 | ((l) << 12))
#define TPM_INTF_CAPS(l)                (0x0014 | ((l) << 12))

enum tpm_timeout {
	TPM_TIMEOUT_MS			= 5,
	TIS_SHORT_TIMEOUT_MS		= 750,
	TIS_LONG_TIMEOUT_MS		= 2000,
	SLEEP_DURATION_US		= 60,
	SLEEP_DURATION_LONG_US		= 210,
};

/* Size of external transmit buffer (used in tpm_transmit)*/
#define TPM_BUFSIZE 4096

/* Index of Count field in TPM response buffer */
#define TPM_RSP_SIZE_BYTE	2
#define TPM_RSP_RC_BYTE		6

struct tpm_chip {
	int is_open;
	int locality;
	u32 vend_dev;
	u8 rid;
	unsigned long timeout_a, timeout_b, timeout_c, timeout_d;  /* msec */
	ulong chip_type;
	struct tpm_tis_phy_ops *phy_ops;
};

struct tpm_input_header {
	__be16 tag;
	__be32 length;
	__be32 ordinal;
} __packed;

struct tpm_output_header {
	__be16 tag;
	__be32 length;
	__be32 return_code;
} __packed;

struct timeout_t {
	__be32 a;
	__be32 b;
	__be32 c;
	__be32 d;
} __packed;

struct duration_t {
	__be32 tpm_short;
	__be32 tpm_medium;
	__be32 tpm_long;
} __packed;

union cap_t {
	struct timeout_t timeout;
	struct duration_t duration;
};

struct tpm_getcap_params_in {
	__be32 cap;
	__be32 subcap_size;
	__be32 subcap;
} __packed;

struct tpm_getcap_params_out {
	__be32 cap_size;
	union cap_t cap;
} __packed;

union tpm_cmd_header {
	struct tpm_input_header in;
	struct tpm_output_header out;
};

union tpm_cmd_params {
	struct tpm_getcap_params_out getcap_out;
	struct tpm_getcap_params_in getcap_in;
};

struct tpm_cmd_t {
	union tpm_cmd_header header;
	union tpm_cmd_params params;
} __packed;

/* Max number of iterations after i2c NAK */
#define MAX_COUNT		3

#ifndef __TPM_V2_H
/*
 * Max number of iterations after i2c NAK for 'long' commands
 *
 * We need this especially for sending TPM_READY, since the cleanup after the
 * transtion to the ready state may take some time, but it is unpredictable
 * how long it will take.
 */
#define MAX_COUNT_LONG		50

enum tis_access {
	TPM_ACCESS_VALID		= 0x80,
	TPM_ACCESS_ACTIVE_LOCALITY	= 0x20,
	TPM_ACCESS_REQUEST_PENDING	= 0x04,
	TPM_ACCESS_REQUEST_USE		= 0x02,
};

enum tis_status {
	TPM_STS_VALID			= 0x80,
	TPM_STS_COMMAND_READY		= 0x40,
	TPM_STS_GO			= 0x20,
	TPM_STS_DATA_AVAIL		= 0x10,
	TPM_STS_DATA_EXPECT		= 0x08,
};
#endif

int tpm_tis_open(struct udevice *udev);
int tpm_tis_close(struct udevice *udev);
int tpm_tis_cleanup(struct udevice *udev);
int tpm_tis_send(struct udevice *udev, const u8 *buf, size_t len);
int tpm_tis_recv(struct udevice *udev, u8 *buf, size_t count);
int tpm_tis_get_desc(struct udevice *udev, char *buf, int size);
int tpm_tis_init(struct udevice *udev);
void tpm_tis_ops_register(struct udevice *udev, struct tpm_tis_phy_ops *ops);
#endif
