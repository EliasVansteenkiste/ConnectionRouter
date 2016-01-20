/*********************************************************************
 *  The following code is part of the power modelling feature of VTR.
 *
 * For support:
 * http://code.google.com/p/vtr-verilog-to-routing/wiki/Power
 *
 * or email:
 * vtr.power.estimation@gmail.com
 *
 * If you are using power estimation for your researach please cite:
 *
 * Jeffrey Goeders and Steven Wilton.  VersaPower: Power Estimation
 * for Diverse FPGA Architectures.  In International Conference on
 * Field Programmable Technology, 2012.
 *
 ********************************************************************/

/**
 * This is the top-level file for power estimation in VTR
 */

#ifndef __POWER_H__
#define __POWER_H__

/************************* INCLUDES *********************************/
#include <list>

#include "vpr_types.h"
#include "PowerSpicedComponent.h"

/************************* DEFINES ***********************************/
/* Maximum size of logs */
#define MAX_LOGS 10000

/* Default clock behaviour */
#define CLOCK_PROB 0.5
#define CLOCK_DENS 2

/************************* ENUMS ************************************/

/* Return code used by power functions */
typedef enum {
	POWER_RET_CODE_SUCCESS = 0, POWER_RET_CODE_ERRORS, POWER_RET_CODE_WARNINGS
} e_power_ret_code;

/* Power log types */
typedef enum {
	POWER_LOG_ERROR, POWER_LOG_WARNING, POWER_LOG_NUM_TYPES
} e_power_log_type;

/* Multiplexer select encoding types */
#if 0
typedef enum {
	ENCODING_ONE_HOT, /* One SRAM bit per mux input */
	ENCODING_DECODER /* Log2(mux_inputs) SRAM bits */
}e_encoding_type;
#endif

/************************* STRUCTS **********************************/
typedef struct s_power_output t_power_output;
typedef struct s_log t_log;
typedef struct s_power_commonly_used t_power_commonly_used;
typedef struct s_power_mux_volt_inf t_power_mux_volt_inf;
typedef struct s_power_mux_volt_pair t_power_mux_volt_pair;
typedef struct s_power_nmos_leakages t_power_nmos_leakages;
typedef struct s_power_nmos_leakage_pair t_power_nmos_leakage_pair;
typedef struct s_power_buffer_sc_levr_inf t_power_buffer_sc_levr_inf;
typedef struct s_power_tech t_power_tech;
typedef struct s_transistor_inf t_transistor_inf;
typedef struct s_transistor_size_inf t_transistor_size_inf;
typedef struct s_rr_node_power t_rr_node_power;
typedef struct s_power_buffer_size_inf t_power_buffer_size_inf;
typedef struct s_power_buffer_strength_inf t_power_buffer_strength_inf;
typedef struct s_mux_node t_mux_node;
typedef struct s_mux_arch t_mux_arch;
typedef struct s_solution_inf t_solution_inf;
typedef struct s_power_nmos_mux_inf t_power_nmos_mux_inf;
typedef struct s_power_nmos_leakage_inf t_power_nmos_leakage_inf;
typedef struct s_power_mux_info t_power_mux_info;

/* Information on the solution obtained by VPR */
struct s_solution_inf {
	float T_crit;
	int channel_width;
};

/* two types of transisters */
typedef enum {
	NMOS, PMOS
} e_tx_type;

/* Information for a given transistor size */
struct s_transistor_size_inf {
	float size;

	/* Subthreshold leakage, for Vds = Vdd */
	float leakage_subthreshold;

	/* Gate leakage for Vgd/Vgs = Vdd */
	float leakage_gate;

	/* Gate, Source, Drain capacitance */
	float C_g;
	float C_s;
	float C_d;
};

/**
 * Transistor information
 */
struct s_transistor_inf {
	int num_size_entries;
	t_transistor_size_inf * size_inf; /* Array of transistor sizes */
	t_transistor_size_inf * long_trans_inf; /* Long transistor (W=1,L=2) */
};

struct s_power_nmos_mux_inf {
	float nmos_size;
	int max_mux_sl_size;
	t_power_mux_volt_inf * mux_voltage_inf;
};

struct s_power_nmos_leakage_inf {
	float nmos_size;
	int num_leakage_pairs;
	t_power_nmos_leakage_pair * leakage_pairs;
};

/* CMOS technology properties, populated from data in xml file */
struct s_power_tech {
	float PN_ratio; /* Ratio of PMOS to NMOS in inverter */
	float Vdd;

	float tech_size; /* Tech size in nm, for example 90e-9 for 90nm */
	float temperature; /* Temp in C */

	t_transistor_inf NMOS_inf;
	t_transistor_inf PMOS_inf;

	/* Pass Multiplexor Voltage Information */
	int num_nmos_mux_info;
	t_power_nmos_mux_inf * nmos_mux_info;

	/* NMOS Leakage by Vds */
	int num_nmos_leakage_info;
	t_power_nmos_leakage_inf * nmos_leakage_info;

	/* Buffer Info */
	int max_buffer_size;
	t_power_buffer_size_inf * buffer_size_inf;
};

/* Buffer information for a given size (# of stages) */
struct s_power_buffer_size_inf {
	int num_strengths;
	t_power_buffer_strength_inf * strength_inf;
};

/* Set of I/O Voltages for a single-level multiplexer */
struct s_power_mux_volt_inf {
	int num_voltage_pairs;
	t_power_mux_volt_pair * mux_voltage_pairs;
};

/* Single I/O voltage for a single-level multiplexer */
struct s_power_mux_volt_pair {
	float v_in;
	float v_out_min;
	float v_out_max;
};

/* Buffer information for a given buffer stength */
struct s_power_buffer_strength_inf {
	float stage_gain;

	/* Short circuit factor - no level restorer */
	float sc_no_levr;

	/* Short circuit factors - level restorers */
	int num_levr_entries;
	t_power_buffer_sc_levr_inf * sc_levr_inf;
};

/* Buffer short-circuit information for a given input mux size */
struct s_power_buffer_sc_levr_inf {
	int mux_size;

	/* Short circuit factor */
	float sc_levr;
};

/* Vds/Ids subthreshold leakage pair */
struct s_power_nmos_leakage_pair {
	float v_ds;
	float i_ds;
};

/* Output details of the power estimation */
struct s_power_output {
	FILE * out;
	t_log * logs;
	int num_logs;
};

struct s_log {
	char * name;
	char ** messages;
	int num_messages;
};

struct s_power_mux_info {
	/* Mux architecture information for 0..mux_arch_max_size */
	int mux_arch_max_size;
	t_mux_arch * mux_arch;
};

/**
 * Commonly used values that are cached here instead of recalculting each time,
 * also includes statistics.
 */
struct s_power_commonly_used {

	/* Capacitances */
	float NMOS_1X_C_g;
	float NMOS_1X_C_d;
	float NMOS_1X_C_s;

	float PMOS_1X_C_g;
	float PMOS_1X_C_d;
	float PMOS_1X_C_s;

	float INV_1X_C_in;
	float INV_1X_C;
	float INV_2X_C;

	/* Component Callibrations Array [0..POWER_CALLIB_COMPONENT_MAX-1] */
	PowerSpicedComponent ** component_callibration;

	/* Subthreshold leakages */
	float NMOS_1X_st_leakage;
	float NMOS_2X_st_leakage;
	float PMOS_1X_st_leakage;
	float PMOS_2X_st_leakage;

	std::map<float, t_power_mux_info*> mux_info;

	/* Routing stats */
	int max_routing_mux_size;
	int max_IPIN_fanin;
	int max_seg_fanout;
	int max_seg_to_IPIN_fanout;
	int max_seg_to_seg_fanout;

	/* Physical length of a tile (meters) */
	float tile_length;

	/* Size of switch and connection box buffers */
	int num_sb_buffers;
	float total_sb_buffer_size;

	int num_cb_buffers;
	float total_cb_buffer_size;
};

/* 1-to-1 data structure with t_rr_node
 */
struct s_rr_node_power {
	bool visited; /* When traversing netlist, need to track whether the node has been processed */
	float * in_dens; /* Switching density of inputs */
	float * in_prob; /* Static probability of inputs */
	short num_inputs; /* Number of inputs */
	short selected_input; /* Input index that is selected */
	short driver_switch_type; /* Switch type that drives this resource */
};

/* Architecture information for a multiplexer.
 * This is used to specify transistor sizes, encoding, etc.
 */
struct s_mux_arch {
	int levels;
	int num_inputs;
	float transistor_size;
	//enum e_encoding_type * encoding_types;
	t_mux_node * mux_graph_head;
};

/* A single-level multiplexer data structure.
 * These are combined in a hierarchy to represent
 * multi-level multiplexers
 */
struct s_mux_node {
	int num_inputs; /* Number of inputs */
	t_mux_node * children; /* Multiplexers that drive the inputs [0..num_inputs-1] */
	int starting_pin_idx; /* Applicable to level 0 only, the overall mux primary input index */
	int level; /* Level in the full multilevel mux - 0 = primary inputs to mux */
	bool level_restorer; /* Whether the output of this mux is level restored */
};

/************************* GLOBALS **********************************/
extern t_solution_inf g_solution_inf;
extern t_power_output * g_power_output;
extern t_power_commonly_used * g_power_commonly_used;
extern t_power_tech * g_power_tech;
extern t_power_arch * g_power_arch;
extern t_net_power * clb_net_power;

/************************* FUNCTION DECLARATIONS ********************/

/* Call before using power module */
bool power_init(char * power_out_filepath,
		char * cmos_tech_behavior_filepath, t_arch * arch,
		t_det_routing_arch * routing_arch);

bool power_uninit(void);

/* Top-Level Function */
e_power_ret_code power_total(float * run_time_s, t_vpr_setup vpr_setup,
		t_arch * arch, t_det_routing_arch * routing_arch);

#endif /* __POWER_H__ */
