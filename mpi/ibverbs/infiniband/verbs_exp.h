/*
 * Copyright (c) 2004, 2005 Topspin Communications.  All rights reserved.
 * Copyright (c) 2004, 2011-2012 Intel Corporation.  All rights reserved.
 * Copyright (c) 2005, 2006, 2007 Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2005 PathScale, Inc.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INFINIBAND_VERBS_EXP_H
#define INFINIBAND_VERBS_EXP_H

#include <infiniband/verbs.h>
#include <stdio.h>

#if __GNUC__ >= 3
#  define __attribute_const __attribute__((const))
#else
#  define __attribute_const
#endif

BEGIN_C_DECLS

#define IBV_EXP_TEST_N_SET_COMP_MASK(val, valid_mask)								\
	if (val > (valid_mask)) {										\
		fprintf(stderr, "%s: resetting invalid comp_mask !!! (comp_mask = 0x%x valid_mask = 0x%x)\n",	\
			__FUNCTION__, val, valid_mask);								\
		val = 0;											\
	}


enum ibv_exp_func_name {
	IBV_EXP_POST_SEND_FUNC,
	IBV_EXP_POLL_CQ_FUNC,
	IBV_POST_SEND_FUNC,
	IBV_POLL_CQ_FUNC,
	IBV_POST_RECV_FUNC
};

enum ibv_exp_start_values {
	IBV_EXP_START_ENUM	= 0x40,
	IBV_EXP_START_FLAG_LOC	= 0x20,
	IBV_EXP_START_FLAG	= (1ULL << IBV_EXP_START_FLAG_LOC),
};

/*
 * Capabilities for exp_atomic_cap field in ibv_exp_device_attr struct
 */
enum ibv_exp_atomic_cap {
	IBV_EXP_ATOMIC_NONE		= IBV_ATOMIC_NONE,
	IBV_EXP_ATOMIC_HCA		= IBV_ATOMIC_HCA,
	IBV_EXP_ATOMIC_GLOB		= IBV_ATOMIC_GLOB,

	IBV_EXP_ATOMIC_HCA_REPLY_BE	= IBV_EXP_START_ENUM /* HOST is LE and atomic reply is BE */
};

/*
 * Flags for exp_device_cap_flags field in ibv_exp_device_attr struct
 */
enum ibv_exp_device_cap_flags {
	IBV_EXP_DEVICE_RESIZE_MAX_WR		= IBV_DEVICE_RESIZE_MAX_WR,
	IBV_EXP_DEVICE_BAD_PKEY_CNTR		= IBV_DEVICE_BAD_PKEY_CNTR,
	IBV_EXP_DEVICE_BAD_QKEY_CNTR		= IBV_DEVICE_BAD_QKEY_CNTR,
	IBV_EXP_DEVICE_RAW_MULTI		= IBV_DEVICE_RAW_MULTI,
	IBV_EXP_DEVICE_AUTO_PATH_MIG		= IBV_DEVICE_AUTO_PATH_MIG,
	IBV_EXP_DEVICE_CHANGE_PHY_PORT		= IBV_DEVICE_CHANGE_PHY_PORT,
	IBV_EXP_DEVICE_UD_AV_PORT_ENFORCE	= IBV_DEVICE_UD_AV_PORT_ENFORCE,
	IBV_EXP_DEVICE_CURR_QP_STATE_MOD	= IBV_DEVICE_CURR_QP_STATE_MOD,
	IBV_EXP_DEVICE_SHUTDOWN_PORT		= IBV_DEVICE_SHUTDOWN_PORT,
	IBV_EXP_DEVICE_INIT_TYPE		= IBV_DEVICE_INIT_TYPE,
	IBV_EXP_DEVICE_PORT_ACTIVE_EVENT	= IBV_DEVICE_PORT_ACTIVE_EVENT,
	IBV_EXP_DEVICE_SYS_IMAGE_GUID		= IBV_DEVICE_SYS_IMAGE_GUID,
	IBV_EXP_DEVICE_RC_RNR_NAK_GEN		= IBV_DEVICE_RC_RNR_NAK_GEN,
	IBV_EXP_DEVICE_SRQ_RESIZE		= IBV_DEVICE_SRQ_RESIZE,
	IBV_EXP_DEVICE_N_NOTIFY_CQ		= IBV_DEVICE_N_NOTIFY_CQ,
	IBV_EXP_DEVICE_XRC			= IBV_DEVICE_XRC,

	IBV_EXP_DEVICE_DC_TRANSPORT		= (IBV_EXP_START_FLAG << 0),
	IBV_EXP_DEVICE_QPG			= (IBV_EXP_START_FLAG << 1),
	IBV_EXP_DEVICE_UD_RSS			= (IBV_EXP_START_FLAG << 2),
	IBV_EXP_DEVICE_UD_TSS			= (IBV_EXP_START_FLAG << 3),
	IBV_EXP_DEVICE_MEM_WINDOW		= (IBV_EXP_START_FLAG << 17),
	IBV_EXP_DEVICE_MEM_MGT_EXTENSIONS	= (IBV_EXP_START_FLAG << 21),
	/* Jumping to 23 as of next capability in include/rdma/ib_verbs.h */
	IBV_EXP_DEVICE_MW_TYPE_2A		= (IBV_EXP_START_FLAG << 23),
	IBV_EXP_DEVICE_MW_TYPE_2B		= (IBV_EXP_START_FLAG << 24),
	IBV_EXP_DEVICE_CROSS_CHANNEL		= (IBV_EXP_START_FLAG << 28),
	IBV_EXP_DEVICE_MANAGED_FLOW_STEERING	= (IBV_EXP_START_FLAG << 29),
	IBV_EXP_DEVICE_MR_ALLOCATE		= (IBV_EXP_START_FLAG << 30),
	IBV_EXP_DEVICE_SHARED_MR		= (IBV_EXP_START_FLAG << 31),
};

/*
 * Flags for ibv_exp_device_attr struct comp_mask.
 * Common usage is to set comp_mask to: IBV_EXP_DEVICE_ATTR_RESERVED - 1
 */
enum ibv_exp_device_attr_comp_mask {
	IBV_EXP_DEVICE_ATTR_CALC_CAP		= (1 << 0),
	IBV_EXP_DEVICE_ATTR_WITH_TIMESTAMP_MASK	= (1 << 1),
	IBV_EXP_DEVICE_ATTR_WITH_HCA_CORE_CLOCK	= (1 << 2),
	IBV_EXP_DEVICE_ATTR_EXP_CAP_FLAGS	= (1 << 3),
	IBV_EXP_DEVICE_DC_RD_REQ		= (1 << 4),
	IBV_EXP_DEVICE_DC_RD_RES		= (1 << 5),
	IBV_EXP_DEVICE_ATTR_INLINE_RECV_SZ	= (1 << 6),
	IBV_EXP_DEVICE_ATTR_RSS_TBL_SZ		= (1 << 7),
	/* set supported bits for validity check */
	IBV_EXP_DEVICE_ATTR_RESERVED		= (1 << 8)
};

struct ibv_exp_device_calc_cap {
	uint64_t		data_types;
	uint64_t		data_sizes;
	uint64_t		int_ops;
	uint64_t		uint_ops;
	uint64_t		fp_ops;
};

struct ibv_exp_device_attr {
	char			fw_ver[64];
	uint64_t		node_guid;
	uint64_t		sys_image_guid;
	uint64_t		max_mr_size;
	uint64_t		page_size_cap;
	uint32_t		vendor_id;
	uint32_t		vendor_part_id;
	uint32_t		hw_ver;
	int			max_qp;
	int			max_qp_wr;
	int			reserved; /* place holder to align with ibv_device_attr */
	int			max_sge;
	int			max_sge_rd;
	int			max_cq;
	int			max_cqe;
	int			max_mr;
	int			max_pd;
	int			max_qp_rd_atom;
	int			max_ee_rd_atom;
	int			max_res_rd_atom;
	int			max_qp_init_rd_atom;
	int			max_ee_init_rd_atom;
	enum ibv_exp_atomic_cap	exp_atomic_cap;
	int			max_ee;
	int			max_rdd;
	int			max_mw;
	int			max_raw_ipv6_qp;
	int			max_raw_ethy_qp;
	int			max_mcast_grp;
	int			max_mcast_qp_attach;
	int			max_total_mcast_qp_attach;
	int			max_ah;
	int			max_fmr;
	int			max_map_per_fmr;
	int			max_srq;
	int			max_srq_wr;
	int			max_srq_sge;
	uint16_t		max_pkeys;
	uint8_t			local_ca_ack_delay;
	uint8_t			phys_port_cnt;
	uint32_t                comp_mask;
	struct ibv_exp_device_calc_cap calc_cap;
	uint64_t		timestamp_mask;
	uint64_t		hca_core_clock;
	uint64_t		exp_device_cap_flags; /* use ibv_exp_device_cap_flags */
	int			max_dc_req_rd_atom;
	int			max_dc_res_rd_atom;
	int			inline_recv_sz;
	uint32_t		max_rss_tbl_sz;
};

enum ibv_exp_access_flags {
	IBV_EXP_ACCESS_LOCAL_WRITE	= IBV_ACCESS_LOCAL_WRITE,
	IBV_EXP_ACCESS_REMOTE_WRITE	= IBV_ACCESS_REMOTE_WRITE,
	IBV_EXP_ACCESS_REMOTE_READ	= IBV_ACCESS_REMOTE_READ,
	IBV_EXP_ACCESS_REMOTE_ATOMIC	= IBV_ACCESS_REMOTE_ATOMIC,
	IBV_EXP_ACCESS_MW_BIND		= IBV_ACCESS_MW_BIND,

	IBV_EXP_ACCESS_ALLOCATE_MR		= (IBV_EXP_START_FLAG << 5),
	IBV_EXP_ACCESS_SHARED_MR_USER_READ	= (IBV_EXP_START_FLAG << 6),
	IBV_EXP_ACCESS_SHARED_MR_USER_WRITE	= (IBV_EXP_START_FLAG << 7),
	IBV_EXP_ACCESS_SHARED_MR_GROUP_READ	= (IBV_EXP_START_FLAG << 8),
	IBV_EXP_ACCESS_SHARED_MR_GROUP_WRITE	= (IBV_EXP_START_FLAG << 9),
	IBV_EXP_ACCESS_SHARED_MR_OTHER_READ	= (IBV_EXP_START_FLAG << 10),
	IBV_EXP_ACCESS_SHARED_MR_OTHER_WRITE	= (IBV_EXP_START_FLAG << 11),
	IBV_EXP_ACCESS_NO_RDMA			= (IBV_EXP_START_FLAG << 12),
	IBV_EXP_ACCESS_MW_ZERO_BASED		= (IBV_EXP_START_FLAG << 13),
	/* set supported bits for validity check */
	IBV_EXP_ACCESS_RESERVED			= (IBV_EXP_START_FLAG << 14)
};

/* memory window information struct that is common to types 1 and 2 */
struct ibv_exp_mw_bind_info {
	struct ibv_mr	*mr;
	uint64_t	 addr;
	uint64_t	 length;
	uint64_t	 exp_mw_access_flags; /* use ibv_exp_access_flags */
};

/*
 * Flags for ibv_exp_mw_bind struct comp_mask
 */
enum ibv_exp_bind_mw_comp_mask {
	IBV_EXP_BIND_MW_RESERVED	= (1 << 0)
};

/* type 1 specific info */
struct ibv_exp_mw_bind {
	struct ibv_qp			*qp;
	struct ibv_mw			*mw;
	uint64_t			 wr_id;
	uint64_t			 exp_send_flags; /* use ibv_exp_send_flags */
	struct ibv_exp_mw_bind_info	 bind_info;
	uint32_t			 comp_mask; /* reserved for future growth (must be 0) */
};

enum ibv_exp_calc_op {
	IBV_EXP_CALC_OP_ADD	= 0,
	IBV_EXP_CALC_OP_MAXLOC,
	IBV_EXP_CALC_OP_BAND,
	IBV_EXP_CALC_OP_BXOR,
	IBV_EXP_CALC_OP_BOR,
	IBV_EXP_CALC_OP_NUMBER
};

enum ibv_exp_calc_data_type {
	IBV_EXP_CALC_DATA_TYPE_INT	= 0,
	IBV_EXP_CALC_DATA_TYPE_UINT,
	IBV_EXP_CALC_DATA_TYPE_FLOAT,
	IBV_EXP_CALC_DATA_TYPE_NUMBER
};

enum ibv_exp_calc_data_size {
	IBV_EXP_CALC_DATA_SIZE_64_BIT	= 0,
	IBV_EXP_CALC_DATA_SIZE_NUMBER
};

enum ibv_exp_wr_opcode {
	IBV_EXP_WR_RDMA_WRITE		= IBV_WR_RDMA_WRITE,
	IBV_EXP_WR_RDMA_WRITE_WITH_IMM	= IBV_WR_RDMA_WRITE_WITH_IMM,
	IBV_EXP_WR_SEND			= IBV_WR_SEND,
	IBV_EXP_WR_SEND_WITH_IMM	= IBV_WR_SEND_WITH_IMM,
	IBV_EXP_WR_RDMA_READ		= IBV_WR_RDMA_READ,
	IBV_EXP_WR_ATOMIC_CMP_AND_SWP	= IBV_WR_ATOMIC_CMP_AND_SWP,
	IBV_EXP_WR_ATOMIC_FETCH_AND_ADD	= IBV_WR_ATOMIC_FETCH_AND_ADD,

	IBV_EXP_WR_SEND_WITH_INV	= 8 + IBV_EXP_START_ENUM,
	IBV_EXP_WR_LOCAL_INV		= 10 + IBV_EXP_START_ENUM,
	IBV_EXP_WR_BIND_MW		= 14 + IBV_EXP_START_ENUM,
	IBV_EXP_WR_SEND_ENABLE		= 0x20 + IBV_EXP_START_ENUM,
	IBV_EXP_WR_RECV_ENABLE,
	IBV_EXP_WR_CQE_WAIT
};

enum ibv_exp_send_flags {
	IBV_EXP_SEND_FENCE		= IBV_SEND_FENCE,
	IBV_EXP_SEND_SIGNALED		= IBV_SEND_SIGNALED,
	IBV_EXP_SEND_SOLICITED		= IBV_SEND_SOLICITED,
	IBV_EXP_SEND_INLINE		= IBV_SEND_INLINE,

	IBV_EXP_SEND_IP_CSUM		= (IBV_EXP_START_FLAG << 0),
	IBV_EXP_SEND_WITH_CALC		= (IBV_EXP_START_FLAG << 1),
	IBV_EXP_SEND_WAIT_EN_LAST	= (IBV_EXP_START_FLAG << 2)
};

/*
 * Flags for ibv_exp_send_wr struct comp_mask
 * Common usage is to set comp_mask to: IBV_EXP_QP_ATTR_RESERVED - 1
 */
enum ibv_exp_send_wr_comp_mask {
	IBV_EXP_SEND_WR_ATTR_RESERVED	= 1 << 0
};

struct ibv_exp_send_wr {
	uint64_t		wr_id;
	struct ibv_exp_send_wr *next;
	struct ibv_sge	       *sg_list;
	int			num_sge;
	enum ibv_exp_wr_opcode	exp_opcode; /* use ibv_exp_wr_opcode */
	int			reserved; /* place holder to align with ibv_send_wr */
	union {
		uint32_t	imm_data; /* in network byte order */
		uint32_t	invalidate_rkey;
	} ex;
	union {
		struct {
			uint64_t	remote_addr;
			uint32_t	rkey;
		} rdma;
		struct {
			uint64_t	remote_addr;
			uint64_t	compare_add;
			uint64_t	swap;
			uint32_t	rkey;
		} atomic;
		struct {
			struct ibv_ah  *ah;
			uint32_t	remote_qpn;
			uint32_t	remote_qkey;
		} ud;
	} wr;
	union {
		union {
			struct {
				uint32_t    remote_srqn;
			} xrc;
		} qp_type;

		uint32_t		xrc_remote_srq_num;
	};
	union {
		struct {
			uint64_t		remote_addr;
			uint32_t		rkey;
		} rdma;
		struct {
			uint64_t		remote_addr;
			uint64_t		compare_add;
			uint64_t		swap;
			uint32_t		rkey;
		} atomic;
		struct {
			struct ibv_cq	*cq;
			int32_t  cq_count;
		} cqe_wait;
		struct {
			struct ibv_qp	*qp;
			int32_t  wqe_count;
		} wqe_enable;
	} task;
	union {
		struct {
			enum ibv_exp_calc_op        calc_op;
			enum ibv_exp_calc_data_type data_type;
			enum ibv_exp_calc_data_size data_size;
		} calc;
	} op;
	struct {
		struct ibv_ah   *ah;
		uint64_t        dct_access_key;
		uint32_t        dct_number;
	} dc;
	struct {
		struct ibv_mw			*mw;
		uint32_t			rkey;
		struct ibv_exp_mw_bind_info	bind_info;
	} bind_mw;
	uint64_t	exp_send_flags; /* use ibv_exp_send_flags */
	uint32_t	comp_mask; /* reserved for future growth (must be 0) */
};

/*
 * Flags for ibv_exp_values struct comp_mask
 */
enum ibv_exp_values_comp_mask {
		IBV_EXP_VALUES_HW_CLOCK_NS	= 1 << 0,
		IBV_EXP_VALUES_HW_CLOCK		= 1 << 1,
		IBV_EXP_VALUES_RESERVED		= 1 << 2
};

struct ibv_exp_values {
	uint32_t comp_mask;
	uint64_t hwclock_ns;
	uint64_t hwclock;
};

/*
 * Flags for flags field in the ibv_exp_cq_init_attr struct
 */
enum ibv_exp_cq_create_flags {
	IBV_EXP_CQ_CREATE_CROSS_CHANNEL		= 1 << 0,
	IBV_EXP_CQ_TIMESTAMP			= 1 << 1,
	IBV_EXP_CQ_TIMESTAMP_TO_SYS_TIME	= 1 << 2,
	/*
	 * note: update IBV_EXP_CQ_CREATE_FLAGS_MASK when adding new fields
	 */
};

enum {
	IBV_EXP_CQ_CREATE_FLAGS_MASK	= IBV_EXP_CQ_CREATE_CROSS_CHANNEL |
					  IBV_EXP_CQ_TIMESTAMP |
					  IBV_EXP_CQ_TIMESTAMP_TO_SYS_TIME,
};

/*
 * Flags for ibv_exp_cq_init_attr struct comp_mask
 * Common usage is to set comp_mask to: IBV_EXP_CQ_INIT_ATTR_RESERVED - 1
 */
enum ibv_exp_cq_init_attr_mask {
	IBV_EXP_CQ_INIT_ATTR_FLAGS		= 1 << 0,
	IBV_EXP_CQ_INIT_ATTR_RESERVED		= 1 << 1,
};

struct ibv_exp_cq_init_attr {
	uint32_t	comp_mask;
	uint32_t	flags;

};

/*
 * Flags for ibv_exp_ah_attr struct comp_mask
 */
enum ibv_exp_ah_attr_attr_comp_mask {
	IBV_EXP_AH_ATTR_LL		= 1 << 0,
	IBV_EXP_AH_ATTR_VID		= 1 << 1,
	IBV_EXP_AH_ATTR_RESERVED	= 1 << 2
};

enum ll_address_type {
	LL_ADDRESS_UNKNOWN,
	LL_ADDRESS_IB,
	LL_ADDRESS_ETH,
	LL_ADDRESS_SIZE
};

struct ibv_exp_ah_attr {
	struct ibv_global_route	grh;
	uint16_t		dlid;
	uint8_t			sl;
	uint8_t			src_path_bits;
	uint8_t			static_rate;
	uint8_t			is_global;
	uint8_t			port_num;
	uint32_t		comp_mask;
	struct {
		enum ll_address_type type;
		uint32_t	len;
		char		*address;
	} ll_address;
	uint16_t		vid;
};

/*
 * Flags for exp_attr_mask argument of ibv_exp_modify_qp
 */
enum ibv_exp_qp_attr_mask {
	IBV_EXP_QP_STATE		= IBV_QP_STATE,
	IBV_EXP_QP_CUR_STATE		= IBV_QP_CUR_STATE,
	IBV_EXP_QP_EN_SQD_ASYNC_NOTIFY	= IBV_QP_EN_SQD_ASYNC_NOTIFY,
	IBV_EXP_QP_ACCESS_FLAGS		= IBV_QP_ACCESS_FLAGS,
	IBV_EXP_QP_PKEY_INDEX		= IBV_QP_PKEY_INDEX,
	IBV_EXP_QP_PORT			= IBV_QP_PORT,
	IBV_EXP_QP_QKEY			= IBV_QP_QKEY,
	IBV_EXP_QP_AV			= IBV_QP_AV,
	IBV_EXP_QP_PATH_MTU		= IBV_QP_PATH_MTU,
	IBV_EXP_QP_TIMEOUT		= IBV_QP_TIMEOUT,
	IBV_EXP_QP_RETRY_CNT		= IBV_QP_RETRY_CNT,
	IBV_EXP_QP_RNR_RETRY		= IBV_QP_RNR_RETRY,
	IBV_EXP_QP_RQ_PSN		= IBV_QP_RQ_PSN,
	IBV_EXP_QP_MAX_QP_RD_ATOMIC	= IBV_QP_MAX_QP_RD_ATOMIC,
	IBV_EXP_QP_ALT_PATH		= IBV_QP_ALT_PATH,
	IBV_EXP_QP_MIN_RNR_TIMER	= IBV_QP_MIN_RNR_TIMER,
	IBV_EXP_QP_SQ_PSN		= IBV_QP_SQ_PSN,
	IBV_EXP_QP_MAX_DEST_RD_ATOMIC	= IBV_QP_MAX_DEST_RD_ATOMIC,
	IBV_EXP_QP_PATH_MIG_STATE	= IBV_QP_PATH_MIG_STATE,
	IBV_EXP_QP_CAP			= IBV_QP_CAP,
	IBV_EXP_QP_DEST_QPN		= IBV_QP_DEST_QPN,

	IBV_EXP_QP_GROUP_RSS		= IBV_EXP_START_FLAG << 21,
	IBV_EXP_QP_DC_KEY		= IBV_EXP_START_FLAG << 22,
};

/*
 * Flags for ibv_exp_qp_attr struct comp_mask
 * Common usage is to set comp_mask to: IBV_EXP_QP_ATTR_RESERVED - 1
 */
enum ibv_exp_qp_attr_comp_mask {
	IBV_EXP_QP_ATTR_RESERVED	= 1 << 0
};

struct ibv_exp_qp_attr {
	enum ibv_qp_state	qp_state;
	enum ibv_qp_state	cur_qp_state;
	enum ibv_mtu		path_mtu;
	enum ibv_mig_state	path_mig_state;
	uint32_t		qkey;
	uint32_t		rq_psn;
	uint32_t		sq_psn;
	uint32_t		dest_qp_num;
	int			qp_access_flags; /* use ibv_access_flags form verbs.h */
	struct ibv_qp_cap	cap;
	struct ibv_ah_attr	ah_attr;
	struct ibv_ah_attr	alt_ah_attr;
	uint16_t		pkey_index;
	uint16_t		alt_pkey_index;
	uint8_t			en_sqd_async_notify;
	uint8_t			sq_draining;
	uint8_t			max_rd_atomic;
	uint8_t			max_dest_rd_atomic;
	uint8_t			min_rnr_timer;
	uint8_t			port_num;
	uint8_t			timeout;
	uint8_t			retry_cnt;
	uint8_t			rnr_retry;
	uint8_t			alt_port_num;
	uint8_t			alt_timeout;
	uint64_t		dct_key;
	uint32_t		comp_mask; /* reserved for future growth (must be 0) */
};

/*
 * Flags for ibv_exp_qp_init_attr struct comp_mask
 * Common usage is to set comp_mask to: IBV_EXP_QP_INIT_ATTR_RESERVED - 1
 */
enum ibv_exp_qp_init_attr_comp_mask {
	IBV_EXP_QP_INIT_ATTR_PD			= 1 << 0,
	IBV_EXP_QP_INIT_ATTR_XRCD		= 1 << 1,
	IBV_EXP_QP_INIT_ATTR_CREATE_FLAGS	= 1 << 2,
	IBV_EXP_QP_INIT_ATTR_INL_RECV		= 1 << 3,
	IBV_EXP_QP_INIT_ATTR_QPG		= 1 << 4,
	IBV_EXP_QP_INIT_ATTR_RESERVED		= 1 << 5
};

enum ibv_exp_qpg_type {
	IBV_EXP_QPG_NONE	= 0,
	IBV_EXP_QPG_PARENT	= (1<<0),
	IBV_EXP_QPG_CHILD_RX	= (1<<1),
	IBV_EXP_QPG_CHILD_TX	= (1<<2)
};

struct ibv_exp_qpg_init_attrib {
	uint32_t tss_child_count;
	uint32_t rss_child_count;
};

struct ibv_exp_qpg {
	uint32_t qpg_type;
	union {
		struct ibv_qp *qpg_parent; /* see qpg_type */
		struct ibv_exp_qpg_init_attrib parent_attrib;
	};
};

/*
 * Flags for exp_create_flags field in ibv_exp_qp_init_attr struct
 */
enum ibv_exp_qp_create_flags {
	IBV_EXP_QP_CREATE_CROSS_CHANNEL        = (1 << 2),
	IBV_EXP_QP_CREATE_MANAGED_SEND         = (1 << 3),
	IBV_EXP_QP_CREATE_MANAGED_RECV         = (1 << 4),
	IBV_EXP_QP_CREATE_IGNORE_SQ_OVERFLOW   = (1 << 6),
	IBV_EXP_QP_CREATE_IGNORE_RQ_OVERFLOW   = (1 << 7),
	IBV_EXP_QP_CREATE_ATOMIC_BE_REPLY      = (1 << 8),
	/* set supported bits for validity check */
	IBV_EXP_QP_CREATE_MASK                 = (0x000001DC)
};

struct ibv_exp_qp_init_attr {
	void		       *qp_context;
	struct ibv_cq	       *send_cq;
	struct ibv_cq	       *recv_cq;
	struct ibv_srq	       *srq;
	struct ibv_qp_cap	cap;
	enum ibv_qp_type	qp_type;
	int			sq_sig_all;

	uint32_t		comp_mask; /* use ibv_exp_qp_init_attr_comp_mask */
	struct ibv_pd	       *pd;
	struct ibv_xrcd	       *xrcd;
	uint32_t		exp_create_flags; /* use ibv_exp_qp_create_flags */

	uint32_t		max_inl_recv;
	struct ibv_exp_qpg	qpg;
};

/*
 * Flags for ibv_exp_dct_init_attr struct comp_mask
 */
enum ibv_exp_dct_init_attr_comp_mask {
	IBV_EXP_DCT_INIT_ATTR_RESERVED	= 1 << 0
};

enum {
	IBV_EXP_DCT_CREATE_FLAGS_MASK		= (1 << 0) - 1,
};

struct ibv_exp_dct_init_attr {
	struct ibv_pd	       *pd;
	struct ibv_cq	       *cq;
	struct ibv_srq	       *srq;
	uint64_t		dc_key;
	uint8_t			port;
	uint32_t		access_flags; /* use ibv_access_flags form verbs.h */
	uint8_t			min_rnr_timer;
	uint8_t			tclass;
	uint32_t		flow_label;
	enum ibv_mtu		mtu;
	uint8_t			pkey_index;
	uint8_t			gid_index;
	uint8_t			hop_limit;
	uint32_t		inline_size;
	uint32_t		create_flags;
	uint32_t		comp_mask; /* reserved for future growth (must be 0) */
};

enum {
	IBV_EXP_DCT_STATE_ACTIVE	= 0,
	IBV_EXP_DCT_STATE_DRAINING	= 1,
	IBV_EXP_DCT_STATE_DRAINED	= 2
};

/*
 * Flags for ibv_exp_dct_attr struct comp_mask
 */
enum ibv_exp_dct_attr_comp_mask {
	IBV_EXP_DCT_ATTR_RESERVED	= 1 << 0
};

struct ibv_exp_dct_attr {
	uint64_t		dc_key;
	uint8_t			port;
	uint32_t		access_flags; /* use ibv_access_flags form verbs.h */
	uint8_t			min_rnr_timer;
	uint8_t			tclass;
	uint32_t		flow_label;
	enum ibv_mtu		mtu;
	uint8_t			pkey_index;
	uint8_t			gid_index;
	uint8_t			hop_limit;
	uint32_t		key_violations;
	uint8_t			state;
	struct ibv_srq	       *srq;
	struct ibv_cq	       *cq;
	struct ibv_pd	       *pd;
	uint32_t		comp_mask; /* reserved for future growth (must be 0) */
};

enum {
	IBV_EXP_QUERY_PORT_STATE		= 1 << 0,
	IBV_EXP_QUERY_PORT_MAX_MTU		= 1 << 1,
	IBV_EXP_QUERY_PORT_ACTIVE_MTU		= 1 << 2,
	IBV_EXP_QUERY_PORT_GID_TBL_LEN		= 1 << 3,
	IBV_EXP_QUERY_PORT_CAP_FLAGS		= 1 << 4,
	IBV_EXP_QUERY_PORT_MAX_MSG_SZ		= 1 << 5,
	IBV_EXP_QUERY_PORT_BAD_PKEY_CNTR	= 1 << 6,
	IBV_EXP_QUERY_PORT_QKEY_VIOL_CNTR	= 1 << 7,
	IBV_EXP_QUERY_PORT_PKEY_TBL_LEN		= 1 << 8,
	IBV_EXP_QUERY_PORT_LID			= 1 << 9,
	IBV_EXP_QUERY_PORT_SM_LID		= 1 << 10,
	IBV_EXP_QUERY_PORT_LMC			= 1 << 11,
	IBV_EXP_QUERY_PORT_MAX_VL_NUM		= 1 << 12,
	IBV_EXP_QUERY_PORT_SM_SL		= 1 << 13,
	IBV_EXP_QUERY_PORT_SUBNET_TIMEOUT	= 1 << 14,
	IBV_EXP_QUERY_PORT_INIT_TYPE_REPLY	= 1 << 15,
	IBV_EXP_QUERY_PORT_ACTIVE_WIDTH		= 1 << 16,
	IBV_EXP_QUERY_PORT_ACTIVE_SPEED		= 1 << 17,
	IBV_EXP_QUERY_PORT_PHYS_STATE		= 1 << 18,
	IBV_EXP_QUERY_PORT_LINK_LAYER		= 1 << 19,
	/* mask of the fields that exists in the standard query_port_command */
	IBV_EXP_QUERY_PORT_STD_MASK		= (1 << 20) - 1,
	/* mask of all supported fields */
	IBV_EXP_QUERY_PORT_MASK			= IBV_EXP_QUERY_PORT_STD_MASK,
};

/*
 * Flags for ibv_exp_port_attr struct comp_mask
 * Common usage is to set comp_mask to: IBV_EXP_QUERY_PORT_ATTR_RESERVED - 1,
 */
enum ibv_exp_query_port_attr_comp_mask {
	IBV_EXP_QUERY_PORT_ATTR_MASK1		= 1 << 0,
	IBV_EXP_QUERY_PORT_ATTR_RESERVED	= 1 << 1,

	IBV_EXP_QUERY_PORT_ATTR_MASKS		= IBV_EXP_QUERY_PORT_ATTR_RESERVED - 1
};

struct ibv_exp_port_attr {
	union {
		struct {
			enum ibv_port_state	state;
			enum ibv_mtu		max_mtu;
			enum ibv_mtu		active_mtu;
			int			gid_tbl_len;
			uint32_t		port_cap_flags;
			uint32_t		max_msg_sz;
			uint32_t		bad_pkey_cntr;
			uint32_t		qkey_viol_cntr;
			uint16_t		pkey_tbl_len;
			uint16_t		lid;
			uint16_t		sm_lid;
			uint8_t			lmc;
			uint8_t			max_vl_num;
			uint8_t			sm_sl;
			uint8_t			subnet_timeout;
			uint8_t			init_type_reply;
			uint8_t			active_width;
			uint8_t			active_speed;
			uint8_t			phys_state;
			uint8_t			link_layer;
			uint8_t			reserved;
		};
		struct ibv_port_attr		port_attr;
	};
	uint32_t		comp_mask;
	uint32_t		mask1;
};

enum ibv_exp_cq_attr_mask {
	IBV_EXP_CQ_MODERATION                  = 1 << 0,
	IBV_EXP_CQ_CAP_FLAGS                   = 1 << 1
};

enum ibv_exp_cq_cap_flags {
	IBV_EXP_CQ_IGNORE_OVERRUN              = (1 << 0),
	/* set supported bits for validity check */
	IBV_EXP_CQ_CAP_MASK                    = (0x00000001)
};

/*
 * Flags for ibv_exp_cq_attr struct comp_mask
 * Common usage is to set comp_mask to: IBV_EXP_CQ_ATTR_RESERVED - 1
 */
enum ibv_exp_cq_attr_comp_mask {
	IBV_EXP_CQ_ATTR_MODERATION	= (1 << 0),
	IBV_EXP_CQ_ATTR_CQ_CAP_FLAGS	= (1 << 1),
	/* set supported bits for validity check */
	IBV_EXP_CQ_ATTR_RESERVED	= (1 << 2)
};

struct ibv_exp_cq_attr {
	uint32_t                comp_mask;
	struct {
		uint16_t        cq_count;
		uint16_t        cq_period;
	} moderation;
	uint32_t		cq_cap_flags;
};

/*
 * Flags for ibv_exp_reg_shared_mr_in struct comp_mask
 */
enum ibv_exp_reg_shared_mr_comp_mask {
	IBV_EXP_REG_SHARED_MR_RESERVED	= (1 << 0)
};

struct ibv_exp_reg_shared_mr_in {
	uint32_t mr_handle;
	struct ibv_pd *pd;
	void *addr;
	uint64_t exp_access; /* use ibv_exp_access_flags */
	uint32_t comp_mask; /* reserved for future growth (must be 0) */
};

enum ibv_exp_flow_flags {
	IBV_EXP_FLOW_ATTR_FLAGS_ALLOW_LOOP_BACK = 1,
};

enum ibv_exp_flow_attr_type {
	/* steering according to rule specifications */
	IBV_EXP_FLOW_ATTR_NORMAL		= 0x0,
	/* default unicast and multicast rule -
	 * receive all Eth traffic which isn't steered to any QP
	 */
	IBV_EXP_FLOW_ATTR_ALL_DEFAULT	= 0x1,
	/* default multicast rule -
	 * receive all Eth multicast traffic which isn't steered to any QP
	 */
	IBV_EXP_FLOW_ATTR_MC_DEFAULT	= 0x2,
	/* sniffer rule - receive all port traffic */
	IBV_EXP_FLOW_ATTR_SNIFFER		= 0x3,
};

enum ibv_exp_flow_spec_type {
	IBV_EXP_FLOW_SPEC_ETH	= 0x20,
	IBV_EXP_FLOW_SPEC_IB	= 0x21,
	IBV_EXP_FLOW_SPEC_IPV4	= 0x30,
	IBV_EXP_FLOW_SPEC_TCP	= 0x40,
	IBV_EXP_FLOW_SPEC_UDP	= 0x41,
};

struct ibv_exp_flow_eth_filter {
	uint8_t		dst_mac[6];
	uint8_t		src_mac[6];
	uint16_t	ether_type;
	/*
	 * same layout as 802.1q: prio 3, cfi 1, vlan id 12
	 */
	uint16_t	vlan_tag;
};

struct ibv_exp_flow_spec_eth {
	enum ibv_exp_flow_spec_type  type;
	uint16_t  size;
	struct ibv_exp_flow_eth_filter val;
	struct ibv_exp_flow_eth_filter mask;
};

struct ibv_exp_flow_ib_filter {
	uint32_t qpn;
	uint8_t  dst_gid[16];
};

struct ibv_exp_flow_spec_ib {
	enum ibv_exp_flow_spec_type  type;
	uint16_t  size;
	struct ibv_exp_flow_ib_filter val;
	struct ibv_exp_flow_ib_filter mask;
};

struct ibv_exp_flow_ipv4_filter {
	uint32_t src_ip;
	uint32_t dst_ip;
};

struct ibv_exp_flow_spec_ipv4 {
	enum ibv_exp_flow_spec_type  type;
	uint16_t  size;
	struct ibv_exp_flow_ipv4_filter val;
	struct ibv_exp_flow_ipv4_filter mask;
};

struct ibv_exp_flow_tcp_udp_filter {
	uint16_t dst_port;
	uint16_t src_port;
};

struct ibv_exp_flow_spec_tcp_udp {
	enum ibv_exp_flow_spec_type  type;
	uint16_t  size;
	struct ibv_exp_flow_tcp_udp_filter val;
	struct ibv_exp_flow_tcp_udp_filter mask;
};

struct ibv_exp_flow_spec {
	union {
		struct {
			enum ibv_exp_flow_spec_type	type;
			uint16_t			size;
		} hdr;
		struct ibv_exp_flow_spec_ib ib;
		struct ibv_exp_flow_spec_eth eth;
		struct ibv_exp_flow_spec_ipv4 ipv4;
		struct ibv_exp_flow_spec_tcp_udp tcp_udp;
	};
};

struct ibv_exp_flow_attr {
	enum ibv_exp_flow_attr_type type;
	uint16_t size;
	uint16_t priority;
	uint8_t num_of_specs;
	uint8_t port;
	uint32_t flags;
	/* Following are the optional layers according to user request
	 * struct ibv_exp_flow_spec_xxx [L2]
	 * struct ibv_exp_flow_spec_yyy [L3/L4]
	 */
	uint64_t reserved; /* reserved for future growth (must be 0) */
};

struct ibv_exp_flow {
	struct ibv_context *context;
	uint32_t	   handle;
};

struct ibv_exp_dct {
	struct ibv_context     *context;
	uint32_t		handle;
	uint32_t		dct_num;
	struct ibv_pd	       *pd;
	struct ibv_srq	       *srq;
	struct ibv_cq	       *cq;
};

enum ibv_exp_wc_opcode {
	IBV_EXP_WC_SEND,
	IBV_EXP_WC_RDMA_WRITE,
	IBV_EXP_WC_RDMA_READ,
	IBV_EXP_WC_COMP_SWAP,
	IBV_EXP_WC_FETCH_ADD,
	IBV_EXP_WC_BIND_MW,
	IBV_EXP_WC_LOCAL_INV		=	7,
/*
 * Set value of IBV_EXP_WC_RECV so consumers can test if a completion is a
 * receive by testing (opcode & IBV_EXP_WC_RECV).
 */
	IBV_EXP_WC_RECV			= 1 << 7,
	IBV_EXP_WC_RECV_RDMA_WITH_IMM
};

enum ibv_exp_wc_flags {
	IBV_EXP_WC_GRH		= IBV_WC_GRH,
	IBV_EXP_WC_WITH_IMM	= IBV_WC_WITH_IMM,

	IBV_EXP_WC_WITH_INV		= IBV_EXP_START_FLAG << 2,
	IBV_EXP_WC_WITH_SL		= IBV_EXP_START_FLAG << 4,
	IBV_EXP_WC_WITH_SLID		= IBV_EXP_START_FLAG << 5,
	IBV_EXP_WC_WITH_TIMESTAMP	= IBV_EXP_START_FLAG << 6,
	IBV_EXP_WC_QP			= IBV_EXP_START_FLAG << 7,
	IBV_EXP_WC_SRQ			= IBV_EXP_START_FLAG << 8,
	IBV_EXP_WC_DCT			= IBV_EXP_START_FLAG << 9,
};

struct ibv_exp_wc {
	uint64_t		wr_id;
	enum ibv_wc_status	status;
	enum ibv_exp_wc_opcode	exp_opcode;
	uint32_t		vendor_err;
	uint32_t		byte_len;
	uint32_t		imm_data; /* in network byte order */
	uint32_t		qp_num;
	uint32_t		src_qp;
	int			reserved; /* place holder to align with ibv_wc */
	uint16_t		pkey_index;
	uint16_t		slid; /* invalid when TS is used */
	uint8_t			sl; /* invalid when TS is used */
	uint8_t			dlid_path_bits;
	uint64_t		timestamp;
	struct ibv_qp	       *qp;
	struct ibv_srq	       *srq;
	struct ibv_exp_dct     *dct;
	uint64_t		exp_wc_flags; /* use ibv_exp_wc_flags */
};

/*
 * Flags for ibv_exp_reg_mr_in struct comp_mask
 */
enum ibv_exp_reg_mr_in_comp_mask {
	/* set supported bits for validity check */
	IBV_EXP_REG_MR_RESERVED	= (1 << 0)
};
struct ibv_exp_reg_mr_in {
	struct ibv_pd *pd;
	void *addr;
	size_t length;
	uint64_t exp_access; /* use ibv_exp_access_flags */
	uint32_t comp_mask; /* reserved for future growth (must be 0) */
};


enum ibv_exp_task_type {
	IBV_EXP_TASK_SEND         = 0,
	IBV_EXP_TASK_RECV         = 1
};

/*
 * Flags for ibv_exp_task struct comp_mask
 */
enum ibv_exp_task_comp_mask {
	IBV_EXP_TASK_RESERVED	= (1 << 0)
};

struct ibv_exp_task {
	enum ibv_exp_task_type	task_type;
	struct {
		struct ibv_qp  *qp;
		union {
			struct ibv_exp_send_wr  *send_wr;
			struct ibv_recv_wr  *recv_wr;
		};
	} item;
	struct ibv_exp_task    *next;
	uint32_t                comp_mask; /* reserved for future growth (must be 0) */
};

/*
 * Flags for ibv_exp_arm_attr struct comp_mask
 */
enum ibv_exp_arm_attr_comp_mask {
	IBV_EXP_ARM_ATTR_RESERVED	= (1 << 0)
};
struct ibv_exp_arm_attr {
	uint32_t	comp_mask; /* reserved for future growth (must be 0) */
};

struct verbs_context_exp {
	/*  "grows up" - new fields go here */
	int (*drv_exp_arm_dct)(struct ibv_exp_dct *dct, struct ibv_exp_arm_attr *attr);
	int (*lib_exp_arm_dct)(struct ibv_exp_dct *dct, struct ibv_exp_arm_attr *attr);
	int (*drv_exp_bind_mw)(struct ibv_exp_mw_bind *mw_bind);
	int (*lib_exp_bind_mw)(struct ibv_exp_mw_bind *mw_bind);
	int (*drv_exp_post_send)(struct ibv_qp *qp,
				 struct ibv_exp_send_wr *wr,
				 struct ibv_exp_send_wr **bad_wr);
	struct ibv_mr * (*drv_exp_reg_mr)(struct ibv_exp_reg_mr_in *in);
	struct ibv_mr * (*lib_exp_reg_mr)(struct ibv_exp_reg_mr_in *in);
	struct ibv_ah * (*drv_exp_ibv_create_ah)(struct ibv_pd *pd,
						 struct ibv_exp_ah_attr *attr_exp);
	int (*drv_exp_query_values)(struct ibv_context *context, int q_values,
				    struct ibv_exp_values *values);
	struct ibv_cq * (*exp_create_cq)(struct ibv_context *context, int cqe,
					 struct ibv_comp_channel *channel,
					 int comp_vector, struct ibv_exp_cq_init_attr *attr);
	int (*drv_exp_ibv_poll_cq)(struct ibv_cq *ibcq, int num_entries,
				   struct ibv_exp_wc *wc, uint32_t wc_size);
	void * (*drv_exp_get_legacy_xrc) (struct ibv_srq *ibv_srq);
	void (*drv_exp_set_legacy_xrc) (struct ibv_srq *ibv_srq, void *legacy_xrc);
	struct ibv_mr * (*drv_exp_ibv_reg_shared_mr)(struct ibv_exp_reg_shared_mr_in *in);
	struct ibv_mr * (*lib_exp_ibv_reg_shared_mr)(struct ibv_exp_reg_shared_mr_in *in);
	int (*drv_exp_modify_qp)(struct ibv_qp *qp, struct ibv_exp_qp_attr *attr,
				 uint64_t exp_attr_mask);
	int (*lib_exp_modify_qp)(struct ibv_qp *qp, struct ibv_exp_qp_attr *attr,
				 uint64_t exp_attr_mask);
	int (*drv_exp_post_task)(struct ibv_context *context,
				 struct ibv_exp_task *task,
				 struct ibv_exp_task **bad_task);
	int (*lib_exp_post_task)(struct ibv_context *context,
				 struct ibv_exp_task *task,
				 struct ibv_exp_task **bad_task);
	int (*drv_exp_modify_cq)(struct ibv_cq *cq,
				 struct ibv_exp_cq_attr *attr, int attr_mask);
	int (*lib_exp_modify_cq)(struct ibv_cq *cq,
				 struct ibv_exp_cq_attr *attr, int attr_mask);
	int (*drv_exp_ibv_destroy_flow) (struct ibv_exp_flow *flow);
	int (*lib_exp_ibv_destroy_flow) (struct ibv_exp_flow *flow);
	struct ibv_exp_flow * (*drv_exp_ibv_create_flow) (struct ibv_qp *qp,
						      struct ibv_exp_flow_attr
						      *flow_attr);
	struct ibv_exp_flow * (*lib_exp_ibv_create_flow) (struct ibv_qp *qp,
							  struct ibv_exp_flow_attr
							  *flow_attr);

	int (*drv_exp_query_port)(struct ibv_context *context, uint8_t port_num,
				  struct ibv_exp_port_attr *port_attr);
	int (*lib_exp_query_port)(struct ibv_context *context, uint8_t port_num,
				  struct ibv_exp_port_attr *port_attr);
	struct ibv_exp_dct *(*create_dct)(struct ibv_context *context,
					  struct ibv_exp_dct_init_attr *attr);
	int (*destroy_dct)(struct ibv_exp_dct *dct);
	int (*query_dct)(struct ibv_exp_dct *dct, struct ibv_exp_dct_attr *attr);
	int (*drv_exp_query_device)(struct ibv_context *context,
				    struct ibv_exp_device_attr *attr);
	int (*lib_exp_query_device)(struct ibv_context *context,
				    struct ibv_exp_device_attr *attr);
	struct ibv_qp *(*drv_exp_create_qp)(struct ibv_context *context,
					    struct ibv_exp_qp_init_attr *init_attr);
	struct ibv_qp *(*lib_exp_create_qp)(struct ibv_context *context,
					    struct ibv_exp_qp_init_attr *init_attr);
	size_t sz;	/* Set by library on struct allocation,	*/
			/* must be located as last field	*/
};


static inline struct verbs_context_exp *verbs_get_exp_ctx(struct ibv_context *ctx)
{
	size_t sz;
	struct verbs_context *vctx = verbs_get_ctx(ctx);

	if (!vctx || !(vctx->has_comp_mask & VERBS_CONTEXT_EXP))
		return NULL;
	sz = *(size_t *)(((char *)vctx) - sizeof(size_t));
	return (struct verbs_context_exp *)(((char *)vctx) - sz);
}

#define verbs_get_exp_ctx_op(ctx, op) ({ \
	struct verbs_context_exp *vctx = verbs_get_exp_ctx(ctx); \
	(!vctx || (vctx->sz < sizeof(*vctx) - offsetof(struct verbs_context_exp, op)) || \
	!vctx->op) ? NULL : vctx; })

#define verbs_set_exp_ctx_op(_vctx, op, ptr) ({ \
	struct verbs_context_exp *vctx = _vctx; \
	if (vctx && (vctx->sz >= sizeof(*vctx) - offsetof(struct verbs_context_exp, op))) \
		vctx->op = ptr; })


static inline struct ibv_qp *
ibv_exp_create_qp(struct ibv_context *context, struct ibv_exp_qp_init_attr *qp_init_attr)
{
	struct verbs_context_exp *vctx;
	uint32_t mask = qp_init_attr->comp_mask;

	if (mask == IBV_EXP_QP_INIT_ATTR_PD)
		return ibv_create_qp(qp_init_attr->pd,
				     (struct ibv_qp_init_attr *) qp_init_attr);

	vctx = verbs_get_exp_ctx_op(context, lib_exp_create_qp);
	if (!vctx) {
		errno = ENOSYS;
		return NULL;
	}
	IBV_EXP_TEST_N_SET_COMP_MASK(qp_init_attr->comp_mask, IBV_EXP_QP_INIT_ATTR_RESERVED - 1);

	return vctx->lib_exp_create_qp(context, qp_init_attr);
}

static inline int ibv_exp_query_device(struct ibv_context *context,
				       struct ibv_exp_device_attr *attr)
{
	struct verbs_context_exp *vctx = verbs_get_exp_ctx_op(context,
							      lib_exp_query_device);
	if (!vctx)
		return ENOSYS;

	IBV_EXP_TEST_N_SET_COMP_MASK(attr->comp_mask, IBV_EXP_DEVICE_ATTR_RESERVED - 1);
	return vctx->lib_exp_query_device(context, attr);
}

static inline struct ibv_exp_dct *
ibv_exp_create_dct(struct ibv_context *context,
		   struct ibv_exp_dct_init_attr *attr)
{
	struct verbs_context_exp *vctx;
	struct ibv_exp_dct *dct;

	vctx = verbs_get_exp_ctx_op(context, create_dct);
	if (!vctx) {
		errno = ENOSYS;
		return NULL;
	}

	pthread_mutex_lock(&context->mutex);
	IBV_EXP_TEST_N_SET_COMP_MASK(attr->comp_mask, IBV_EXP_DCT_INIT_ATTR_RESERVED - 1);
	dct = vctx->create_dct(context, attr);
	if (dct)
		dct->context = context;

	pthread_mutex_unlock(&context->mutex);

	return dct;
}

static inline int ibv_exp_destroy_dct(struct ibv_exp_dct *dct)
{
	struct verbs_context_exp *vctx;
	struct ibv_context *context = dct->context;
	int err;

	vctx = verbs_get_exp_ctx_op(context, destroy_dct);
	if (!vctx) {
		errno = ENOSYS;
		return errno;
	}

	pthread_mutex_lock(&context->mutex);
	err = vctx->destroy_dct(dct);
	pthread_mutex_unlock(&context->mutex);

	return err;
}

static inline int ibv_exp_query_dct(struct ibv_exp_dct *dct,
				    struct ibv_exp_dct_attr *attr)
{
	struct verbs_context_exp *vctx;
	struct ibv_context *context = dct->context;
	int err;

	vctx = verbs_get_exp_ctx_op(context, query_dct);
	if (!vctx) {
		errno = ENOSYS;
		return errno;
	}

	pthread_mutex_lock(&context->mutex);
	IBV_EXP_TEST_N_SET_COMP_MASK(attr->comp_mask, IBV_EXP_DCT_ATTR_RESERVED - 1);
	err = vctx->query_dct(dct, attr);
	pthread_mutex_unlock(&context->mutex);

	return err;
}

static inline int ibv_exp_arm_dct(struct ibv_exp_dct *dct,
				  struct ibv_exp_arm_attr *attr)
{
	struct verbs_context_exp *vctx;
	struct ibv_context *context = dct->context;
	int err;

	vctx = verbs_get_exp_ctx_op(context, lib_exp_arm_dct);
	if (!vctx) {
		errno = ENOSYS;
		return errno;
	}

	pthread_mutex_lock(&context->mutex);
	IBV_EXP_TEST_N_SET_COMP_MASK(attr->comp_mask, IBV_EXP_ARM_ATTR_RESERVED - 1);
	err = vctx->lib_exp_arm_dct(dct, attr);
	pthread_mutex_unlock(&context->mutex);

	return err;
}

static inline int ibv_exp_query_port(struct ibv_context *context,
				     uint8_t port_num,
				     struct ibv_exp_port_attr *port_attr)
{
	struct verbs_context_exp *vctx;

	if (0 == port_attr->comp_mask)
		return ibv_query_port(context, port_num,
				      &port_attr->port_attr);

	/* Check that only valid flags were given */
	if ((!port_attr->comp_mask & IBV_EXP_QUERY_PORT_ATTR_MASK1) ||
	    (port_attr->comp_mask & ~IBV_EXP_QUERY_PORT_ATTR_MASKS) ||
	    (port_attr->mask1 & ~IBV_EXP_QUERY_PORT_MASK)) {
		errno = EINVAL;
		return -errno;
	}

	vctx = verbs_get_exp_ctx_op(context, lib_exp_query_port);

	if (!vctx) {
		/* Fallback to legacy mode */
		if (port_attr->comp_mask == IBV_EXP_QUERY_PORT_ATTR_MASK1 &&
		    !(port_attr->mask1 & ~IBV_EXP_QUERY_PORT_STD_MASK))
			return ibv_query_port(context, port_num,
					      &port_attr->port_attr);

		/* Unsupported field was requested */
		errno = ENOSYS;
		return -errno;
	}
	IBV_EXP_TEST_N_SET_COMP_MASK(port_attr->comp_mask, IBV_EXP_QUERY_PORT_ATTR_RESERVED - 1);

	return vctx->lib_exp_query_port(context, port_num, port_attr);
}

/**
 * ibv_exp_post_task - Post a list of tasks to different QPs.
 */
static inline int ibv_exp_post_task(struct ibv_context *context,
				    struct ibv_exp_task *task,
				    struct ibv_exp_task **bad_task)
{
	struct verbs_context_exp *vctx = verbs_get_exp_ctx_op(context,
							      lib_exp_post_task);
	if (!vctx)
		return ENOSYS;

	IBV_EXP_TEST_N_SET_COMP_MASK(task->comp_mask, IBV_EXP_TASK_RESERVED - 1);

	return vctx->lib_exp_post_task(context, task, bad_task);
}

static inline int ibv_exp_query_values(struct ibv_context *context, int q_values,
				       struct ibv_exp_values *values)
{
	struct verbs_context_exp *vctx = verbs_get_exp_ctx_op(context,
							      drv_exp_query_values);
	if (!vctx) {
		errno = ENOSYS;
		return -errno;
	}
	IBV_EXP_TEST_N_SET_COMP_MASK(values->comp_mask, IBV_EXP_VALUES_RESERVED - 1);

	return vctx->drv_exp_query_values(context, q_values, values);
}

static inline struct ibv_exp_flow *ibv_exp_create_flow(struct ibv_qp *qp,
						       struct ibv_exp_flow_attr *flow)
{
	struct verbs_context_exp *vctx = verbs_get_exp_ctx_op(qp->context,
							      lib_exp_ibv_create_flow);
	if (!vctx || !vctx->lib_exp_ibv_create_flow)
		return NULL;

	if (flow->reserved != 0L) {
		fprintf(stderr, "%s:%d: flow->reserved must be 0\n", __FUNCTION__, __LINE__);
		flow->reserved = 0L;
	}

	return vctx->lib_exp_ibv_create_flow(qp, flow);
}

static inline int ibv_exp_destroy_flow(struct ibv_exp_flow *flow_id)
{
	struct verbs_context_exp *vctx = verbs_get_exp_ctx_op(flow_id->context,
							      lib_exp_ibv_destroy_flow);
	if (!vctx || !vctx->lib_exp_ibv_destroy_flow)
		return -ENOSYS;

	return vctx->lib_exp_ibv_destroy_flow(flow_id);
}

static inline int ibv_exp_poll_cq(struct ibv_cq *ibcq, int num_entries,
				  struct ibv_exp_wc *wc, uint32_t wc_size)
{
	struct verbs_context_exp *vctx = verbs_get_exp_ctx_op(ibcq->context,
							      drv_exp_ibv_poll_cq);
	if (!vctx)
		return -ENOSYS;

	return vctx->drv_exp_ibv_poll_cq(ibcq, num_entries, wc, wc_size);
}

/**
 * ibv_exp_post_send - Post a list of work requests to a send queue.
 */
static inline int ibv_exp_post_send(struct ibv_qp *qp,
				    struct ibv_exp_send_wr *wr,
				    struct ibv_exp_send_wr **bad_wr)
{
	struct verbs_context_exp *vctx = verbs_get_exp_ctx_op(qp->context,
							      drv_exp_post_send);
	if (!vctx)
		return -ENOSYS;

	return vctx->drv_exp_post_send(qp, wr, bad_wr);
}

/**
 * ibv_exp_reg_shared_mr - Register to an existing shared memory region
 * @in - Experimental register shared MR input data.
 */
static inline struct ibv_mr *ibv_exp_reg_shared_mr(struct ibv_exp_reg_shared_mr_in *mr_in)
{
	struct verbs_context_exp *vctx = verbs_get_exp_ctx_op(mr_in->pd->context,
							      lib_exp_ibv_reg_shared_mr);
	if (!vctx) {
		errno = ENOSYS;
		return NULL;
	}
	IBV_EXP_TEST_N_SET_COMP_MASK(mr_in->comp_mask, IBV_EXP_REG_SHARED_MR_RESERVED - 1);

	return vctx->lib_exp_ibv_reg_shared_mr(mr_in);
}

/**
 * ibv_exp_modify_cq - Modifies the attributes for the specified CQ.
 * @cq: The CQ to modify.
 * @cq_attr: Specifies the CQ attributes to modify.
 * @cq_attr_mask: A bit-mask used to specify which attributes of the CQ
 *   are being modified.
 */
static inline int ibv_exp_modify_cq(struct ibv_cq *cq,
				    struct ibv_exp_cq_attr *cq_attr,
				    int cq_attr_mask)
{
	struct verbs_context_exp *vctx = verbs_get_exp_ctx_op(cq->context,
							      lib_exp_modify_cq);
	if (!vctx)
		return ENOSYS;

	IBV_EXP_TEST_N_SET_COMP_MASK(cq_attr->comp_mask, IBV_EXP_CQ_ATTR_RESERVED - 1);

	return vctx->lib_exp_modify_cq(cq, cq_attr, cq_attr_mask);
}

static inline struct ibv_cq *ibv_exp_create_cq(struct ibv_context *context,
					       int cqe,
					       void *cq_context,
					       struct ibv_comp_channel *channel,
					       int comp_vector,
					       struct ibv_exp_cq_init_attr *attr)
{
	struct verbs_context_exp *vctx;
	struct ibv_cq *cq;

	vctx = verbs_get_exp_ctx_op(context, exp_create_cq);
	if (!vctx) {
		errno = ENOSYS;
		return NULL;
	}

	pthread_mutex_lock(&context->mutex);
	IBV_EXP_TEST_N_SET_COMP_MASK(attr->comp_mask, IBV_EXP_CQ_INIT_ATTR_RESERVED - 1);
	cq = vctx->exp_create_cq(context, cqe, channel, comp_vector, attr);
	if (cq) {
		cq->context		   = context;
		cq->channel		   = channel;
		if (channel)
			++channel->refcnt;
		cq->cq_context		   = cq_context;
		cq->comp_events_completed  = 0;
		cq->async_events_completed = 0;
		pthread_mutex_init(&cq->mutex, NULL);
		pthread_cond_init(&cq->cond, NULL);
	}

	pthread_mutex_unlock(&context->mutex);

	return cq;
}

/**
 * ibv_exp_modify_qp - Modify a queue pair.
 * The argument exp_attr_mask specifies the QP attributes to be modified.
 */
static inline int
ibv_exp_modify_qp(struct ibv_qp *qp, struct ibv_exp_qp_attr *attr, uint64_t exp_attr_mask)
{
	struct verbs_context_exp *vctx;

	vctx = verbs_get_exp_ctx_op(qp->context, lib_exp_modify_qp);
	if (!vctx) {
		errno = ENOSYS;
		return errno;
	}
	IBV_EXP_TEST_N_SET_COMP_MASK(attr->comp_mask, IBV_EXP_QP_ATTR_RESERVED - 1);

	return vctx->lib_exp_modify_qp(qp, attr, exp_attr_mask);
}

/**
 * ibv_exp_reg_mr - Register a memory region
 */
static inline struct ibv_mr *ibv_exp_reg_mr(struct ibv_exp_reg_mr_in *in)
{
	struct verbs_context_exp *vctx;

	vctx = verbs_get_exp_ctx_op(in->pd->context, lib_exp_reg_mr);
	if (!vctx) {
		errno = ENOSYS;
		return NULL;
	}
	IBV_EXP_TEST_N_SET_COMP_MASK(in->comp_mask, IBV_EXP_REG_MR_RESERVED - 1);

	return vctx->lib_exp_reg_mr(in);
}


/**
 * ibv_exp_bind_mw - Bind a memory window to a region
 */
static inline int ibv_exp_bind_mw(struct ibv_exp_mw_bind *mw_bind)
{
	struct verbs_context_exp *vctx;

	vctx = verbs_get_exp_ctx_op(mw_bind->mw->context, lib_exp_bind_mw);
	if (!vctx) {
		errno = ENOSYS;
		return errno;
	}
	IBV_EXP_TEST_N_SET_COMP_MASK(mw_bind->comp_mask, IBV_EXP_BIND_MW_RESERVED - 1);

	return vctx->lib_exp_bind_mw(mw_bind);
}

typedef int (*drv_exp_post_send_func)(struct ibv_qp *qp,
				 struct ibv_exp_send_wr *wr,
				 struct ibv_exp_send_wr **bad_wr);
typedef int (*drv_post_send_func)(struct ibv_qp *qp, struct ibv_send_wr *wr,
				struct ibv_send_wr **bad_wr);
typedef int (*drv_exp_poll_cq_func)(struct ibv_cq *ibcq, int num_entries,
				   struct ibv_exp_wc *wc, uint32_t wc_size);
typedef int (*drv_poll_cq_func)(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc);
typedef int (*drv_post_recv_func)(struct ibv_qp *qp, struct ibv_recv_wr *wr,
			 struct ibv_recv_wr **bad_wr);

static inline void *ibv_exp_get_provider_func(struct ibv_context *context,
						enum ibv_exp_func_name name)
{
	struct verbs_context_exp *vctx;

	switch (name) {
	case IBV_EXP_POST_SEND_FUNC:
		vctx = verbs_get_exp_ctx_op(context, drv_exp_post_send);
		if (!vctx)
			goto error;

		return (void *)vctx->drv_exp_post_send;

	case IBV_EXP_POLL_CQ_FUNC:
		vctx = verbs_get_exp_ctx_op(context, drv_exp_ibv_poll_cq);
		if (!vctx)
			goto error;

		return (void *)vctx->drv_exp_ibv_poll_cq;

	case IBV_POST_SEND_FUNC:
		if (!context->ops.post_send)
			goto error;

		return (void *)context->ops.post_send;

	case IBV_POLL_CQ_FUNC:
		if (!context->ops.poll_cq)
			goto error;

		return (void *)context->ops.poll_cq;

	case IBV_POST_RECV_FUNC:
		if (!context->ops.post_recv)
			goto error;

		return (void *)context->ops.post_recv;

	default:
		break;
	}

error:
	errno = ENOSYS;
	return NULL;
}

END_C_DECLS

#  undef __attribute_const


#endif /* INFINIBAND_VERBS_EXP_H */
