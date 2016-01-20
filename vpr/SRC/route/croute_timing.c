
#include <cstdio>
#include <ctime>
#include <cmath>

#include <assert.h>

#include <vector>
#include <algorithm>

#include "vpr_utils.h"
#include "util.h"
#include "vpr_types.h"
#include "globals.h"
#include "route_export.h"
#include "route_common.h"
#include "croute_tree_timing.h"
#include "route_tree_timing.h"
#include "croute_timing.h"
#include "route_timing.h"
#include "heapsort.h"
#include "path_delay.h"
#include "path_delay2.h"
#include "net_delay.h"
#include "stats.h"
#include "ReadOptions.h"

using namespace std;



/******************** Subroutines local to route_timing.c ********************/

static int get_max_pins_per_net(void);

static void timing_driven_expand_neighbours_con(struct s_heap *current, 
		int inet,
		float bend_cost, 
		float criticality_fac, 
		int target_node,
		float astar_fac, 
		int highfanout_rlim,
        t_rr_to_rg_node_hash_map* node_map);

static float get_timing_driven_expected_cost(
	int inode, 
	int target_node,
	float criticality_fac,
	float R_upstream);

#ifdef INTERPOSER_BASED_ARCHITECTURE
static int get_num_expected_interposer_hops_to_target(int inode, int target_node);
#endif

static int get_expected_segs_to_target(int inode, 
	int target_node,
	int *num_segs_ortho_dir_ptr);

static void update_rr_base_costs(int inet, float largest_criticality);

static void timing_driven_check_net_delays(float **net_delay);

static int mark_node_expansion_by_bin(int inet, 
	int target_node,
	t_rg_node * rg_node);

static double get_overused_ratio();

static bool should_route_net(int inet);

static bool should_route_con(int inet);

struct more_sinks_than {
	inline bool operator() (const int& net_index1, const int& net_index2) {
		return g_clbs_nlist.net[net_index1].num_sinks() > g_clbs_nlist.net[net_index2].num_sinks();
	}
};



static void congestion_analysis();
static void time_on_fanout_analysis();
constexpr int fanout_per_bin = 5;
static vector<float> time_on_fanout;
static vector<int> itry_on_fanout;

static void con_to_net(float** net_delays);

static void con_to_net_from_timing_rg(float** net_delays);

static void alloc_timing_driven_route_structs_and_arrays();

static void free_timing_driven_route_structs_and_arrays();

static bool timing_driven_route_con(int icon,
        float max_criticality,
        float criticality_exp,
        float astar_fac,
        float bend_cost,
        float *net_delay,
        t_slack * slacks,
        t_rr_to_rg_node_hash_map* node_map);

//static unsigned long no_nodes_expanded;
static t_rg_node ** rg_sinks;
static t_rg_node ** rg_roots;

static void add_con_td(int con, float *net_delay, t_rr_to_rg_node_hash_map* node_map , float pres_fac);

static void rip_up_con_td(int con, t_rr_to_rg_node_hash_map* node_map, float pres_fac);

static void get_num_bends_and_length_conr(int inet,
    t_rr_to_rg_node_hash_map* node_map, 
    int *bends_ptr,
    int *len_ptr,
    int *segments_ptr);

static void print_trace(int inet);

static void print_trace_to_dot(int inet);

static void print_trace_to_dot_from_con(int inet);

static t_rg_node* find_non_visited_rec(t_rg_node* node, bool initial_root_status);


/************************ Subroutine definitions *****************************/
bool try_timing_driven_route_conr(struct s_router_opts router_opts,
		float **net_delay, 
		t_slack * slacks, 
		t_ivec ** clb_opins_used_locally, 
		bool timing_analysis_enabled, 
		const t_timing_inf &timing_inf) {

	/* Timing-driven connection-based routing algorithm.  The timing graph (includes slack)   *
	 * must have already been allocated, and net_delay must have been allocated. *
	 * Returns true if the routing succeeds, false otherwise.                    */

	const int max_fanout {get_max_pins_per_net()};
	time_on_fanout.resize((max_fanout / fanout_per_bin) + 1, 0);
	itry_on_fanout.resize((max_fanout / fanout_per_bin) + 1, 0);

	auto sorted_nets = vector<int>(g_clbs_nlist.net.size());

	for (size_t i = 0; i < g_clbs_nlist.net.size(); ++i) {
		sorted_nets[i] = i;
	}

	// sort so net with most sinks is first.
	std::sort(sorted_nets.begin(), sorted_nets.end(), more_sinks_than());

	timing_driven_route_structs route_structs{}; // allocs route strucs (calls default constructor)
	// contains:
	// float *pin_criticality; /* [1..max_pins_per_net-1] */
	// int *sink_order; /* [1..max_pins_per_net-1] */;
	// t_rt_node **rt_node_of_sink; /* [1..max_pins_per_net-1] */

	/* First do one routing iteration ignoring congestion to get reasonable net 
	   delay estimates. Set criticalities to 1 when timing analysis is on to 
	   optimize timing, and to 0 when timing analysis is off to optimize routability. */

	float init_timing_criticality_val = (timing_analysis_enabled ? 1.0 : 0.0);

	/* Variables used to do the optimization of the routing, aborting visibly
	 * impossible Ws */
	double overused_ratio;
	vector<double> historical_overuse_ratio; 
	historical_overuse_ratio.reserve(router_opts.max_router_iterations + 1);

	// for unroutable large circuits, the time per iteration seems to increase dramatically
	vector<float> time_per_iteration;
	time_per_iteration.reserve(router_opts.max_router_iterations + 1);


//    t_rr_to_rg_node_hash_map* node_map = NULL;
    
    num_cons = 0;
    for (unsigned int inet = 0; inet < g_clbs_nlist.net.size(); ++inet) {
        if (g_clbs_nlist.net[inet].is_global == false) {
            num_cons += g_clbs_nlist.net[inet].pins.size() - 1;
//            if ((g_clbs_nlist.net[inet].pins.size() - 1) > maxFO) maxFO = g_clbs_nlist.net[inet].pins.size() - 1;
            for (unsigned int ipin = 1; ipin < g_clbs_nlist.net[inet].pins.size(); ++ipin) {
                slacks->timing_criticality[inet][ipin] = init_timing_criticality_val;
#ifdef PATH_COUNTING
                slacks->path_criticality[inet][ipin] = init_timing_criticality_val;
#endif	
            }
        } else {
            /* Set delay of global signals to zero. Non-global net delays are set by
               update_net_delays_from_route_tree() inside timing_driven_route_net(), 
               which is only called for non-global nets. */
            for (unsigned int ipin = 1; ipin < g_clbs_nlist.net[inet].pins.size(); ++ipin) {
                net_delay[inet][ipin] = 0.;
            }
        }
    }
    
    alloc_timing_driven_route_structs_and_arrays();

	/* Allocate memory for Connection Array*/
    printf("Allocate memory for Connection Array for %d cons\n", num_cons);
    cons = (t_con*) my_malloc(num_cons * sizeof (t_con));
    /* Assign values to connection structs*/
    printf("Assigning values to connection structs\n");
    unsigned int icon = 0;
    for (unsigned int i = 0; i < g_clbs_nlist.net.size(); i++) {
        int net = sorted_nets[i];
        rg_roots[net] = init_graph_to_source(net, rg_sinks, &node_maps[net]);
        if (g_clbs_nlist.net[net].is_global == false) { /* Skip global nets. */
            g_clbs_nlist.net[net].first_con = icon;
            for (unsigned int isink = 1; isink < g_clbs_nlist.net[net].pins.size(); isink++) {
                cons[icon].net = net;
                cons[icon].target_node = net_rr_terminals[net][isink];
                cons[icon].source = net_rr_terminals[net][0];
                cons[icon].source_block = g_clbs_nlist.net[net].pins[0].block;
                cons[icon].source_block_pin = g_clbs_nlist.net[net].pins[0].block_pin;
                cons[icon].source_block_port = g_clbs_nlist.net[net].pins[0].block_port;
                cons[icon].sink_block = g_clbs_nlist.net[net].pins[isink+1].block;
                cons[icon].sink_block_pin = g_clbs_nlist.net[net].pins[isink+1].block_pin;
                cons[icon].sink_block_port = g_clbs_nlist.net[net].pins[isink+1].block_port;
                icon++;

            }
        }
    }

	float pres_fac = router_opts.first_iter_pres_fac; /* Typically 0 -> ignore cong. */

	for (int itry = 1; itry <= router_opts.max_router_iterations; ++itry) {
		clock_t begin = clock();

		/* Reset "is_routed" and "is_fixed" flags to indicate nets not pre-routed (yet) */
		for (unsigned int inet = 0; inet < g_clbs_nlist.net.size(); ++inet) {
			g_clbs_nlist.net[inet].is_routed = false;
			g_clbs_nlist.net[inet].is_fixed = false;
		}

		for (icon = 0; icon < num_cons; ++icon) {
			bool is_routable = try_timing_driven_route_con(
				icon,
				itry,
				pres_fac,
				router_opts,
				net_delay[cons[icon].net],
				slacks,
                                &node_maps[cons[icon].net]
			);

			if (!is_routable) {
				return (false);
			}
		}

		clock_t end = clock();
		float time = static_cast<float>(end - begin) / CLOCKS_PER_SEC;
		time_per_iteration.push_back(time);

		if (itry == 1) {
			/* Early exit code for cases where it is obvious that a successful route will not be found 
			   Heuristic: If total wirelength used in first routing iteration is X% of total available wirelength, exit */
			int total_wirelength = 0;
			int available_wirelength = 0;

			for (int i = 0; i < num_rr_nodes; ++i) {
				if (rr_node[i].type == CHANX || rr_node[i].type == CHANY) {
					available_wirelength += 1 + 
							rr_node[i].get_xhigh() - rr_node[i].get_xlow() + 
							rr_node[i].get_yhigh() - rr_node[i].get_ylow();
				}
			}

			for (unsigned int inet = 0; inet < g_clbs_nlist.net.size(); ++inet) {
				if (g_clbs_nlist.net[inet].is_global == false
						&& g_clbs_nlist.net[inet].num_sinks() != 0) { /* Globals don't count. */
					int bends, wirelength, segments;
					get_num_bends_and_length_conr(inet, &node_maps[inet], &bends, &wirelength, &segments);

					total_wirelength += wirelength;
				}
			}
			vpr_printf_info("Wire length after first iteration %d, total available wire length %d, ratio %g\n",
					total_wirelength, available_wirelength,
					(float) (total_wirelength) / (float) (available_wirelength));
			if ((float) (total_wirelength) / (float) (available_wirelength)> FIRST_ITER_WIRELENTH_LIMIT) {
				vpr_printf_info("Wire length usage ratio exceeds limit of %g, fail routing.\n",
						FIRST_ITER_WIRELENTH_LIMIT);

				time_on_fanout_analysis();
				return false;
			}
			vpr_printf_info("--------- ---------- ----------- ---------------------\n");
			vpr_printf_info("Iteration       Time   Crit Path     Overused RR Nodes\n");
			vpr_printf_info("--------- ---------- ----------- ---------------------\n");
		}

		/* Make sure any CLB OPINs used up by subblocks being hooked directly
		   to them are reserved for that purpose. */

		bool rip_up_local_opins = (itry == 1 ? false : true);
		reserve_locally_used_opins(pres_fac, router_opts.acc_fac, rip_up_local_opins, clb_opins_used_locally);

		/* Pathfinder guys quit after finding a feasible route. I may want to keep 
		   going longer, trying to improve timing.  Think about this some. */

		bool success = feasible_routing();

		/* Verification to check the ratio of overused nodes, depending on the configuration
		 * may abort the routing if the ratio is too high. */
		overused_ratio = get_overused_ratio();
		historical_overuse_ratio.push_back(overused_ratio);

		/* Determine when routing is impossible hence should be aborted */
		if (itry > 5){
			
			int expected_success_route_iter = predict_success_route_iter(historical_overuse_ratio, router_opts);
			if (expected_success_route_iter == UNDEFINED) {
				time_on_fanout_analysis();
				return false;
			}

			if (itry > 15) {
				// compare their slopes over the last 5 iterations
				double time_per_iteration_slope = linear_regression_vector(time_per_iteration, itry-5);
				double congestion_per_iteration_slope = linear_regression_vector(historical_overuse_ratio, itry-5);
				if (router_opts.congestion_analysis)
					vpr_printf_info("%f s/iteration %f %/iteration\n", time_per_iteration_slope, congestion_per_iteration_slope);
				// time is increasing and congestion is non-decreasing (grows faster than 10% per iteration)
				if (congestion_per_iteration_slope > 0 && time_per_iteration_slope > 0.1*time_per_iteration.back()
					&& time_per_iteration_slope > 1) {	// filter out noise
					vpr_printf_info("Time per iteration growing too fast at slope %f s/iteration \n\
									 while congestion grows at %f %/iteration, unlikely to finish.\n",
						time_per_iteration_slope, congestion_per_iteration_slope);

					time_on_fanout_analysis();
					return false;
				}
			}
		}

        //print_usage_by_wire_length();


		if (success) {
                    //print_trace_to_dot_from_con(1);
                    //con_to_net(net_delay);
                    con_to_net_from_timing_rg(net_delay);
   
			if (timing_analysis_enabled) {
				load_timing_graph_net_delays(net_delay);
				do_timing_analysis(slacks, timing_inf, false, false);
				float critical_path_delay = get_critical_path_delay();
                vpr_printf_info("%9d %6.2f sec %8.5f ns   %3.2e (%3.4f %)\n", itry, time, critical_path_delay, overused_ratio*num_rr_nodes, overused_ratio*100);
				vpr_printf_info("Critical path: %g ns\n", critical_path_delay);
			} else {
                vpr_printf_info("%9d %6.2f sec         N/A   %3.2e (%3.4f %)\n", itry, time, overused_ratio*num_rr_nodes, overused_ratio*100);
			}

			vpr_printf_info("Successfully routed after %d routing iterations.\n", itry);
#ifdef DEBUG
			if (timing_analysis_enabled)
				timing_driven_check_net_delays(net_delay);
#endif
			time_on_fanout_analysis();
			return (true);
		}

		if (itry == 1) {
			pres_fac = router_opts.initial_pres_fac;
			pathfinder_update_cost(pres_fac, 0.); /* Acc_fac=0 for first iter. */
		} else {
			pres_fac *= router_opts.pres_fac_mult;

			/* Avoid overflow for high iteration counts, even if acc_cost is big */
			pres_fac = min(pres_fac, static_cast<float>(HUGE_POSITIVE_FLOAT / 1e5));

			pathfinder_update_cost(pres_fac, router_opts.acc_fac);
		}

		if (timing_analysis_enabled) {		
			/* Update slack values by doing another timing analysis.
			   Timing_driven_route_net updated the net delay values. */

			load_timing_graph_net_delays(net_delay);

			do_timing_analysis(slacks, timing_inf, false, false);

		} else {
			/* If timing analysis is not enabled, make sure that the criticalities and the
			   net_delays stay as 0 so that wirelength can be optimized. */
			
			for (unsigned int inet = 0; inet < g_clbs_nlist.net.size(); ++inet) {
				for (unsigned int ipin = 1; ipin < g_clbs_nlist.net[inet].pins.size(); ++ipin) {
					slacks->timing_criticality[inet][ipin] = 0.;
#ifdef PATH_COUNTING 		
					slacks->path_criticality[inet][ipin] = 0.; 		
#endif
					net_delay[inet][ipin] = 0.;
				}
			}
		}

		
		if (timing_analysis_enabled) {
			float critical_path_delay = get_critical_path_delay();
            vpr_printf_info("\n%9d %6.2f sec %8.5f ns   %3.2e (%3.4f %)\n", itry, time, critical_path_delay, overused_ratio*num_rr_nodes, overused_ratio*100);
		} else {
            vpr_printf_info("\n%9d %6.2f sec         N/A   %3.2e (%3.4f %)\n", itry, time, overused_ratio*num_rr_nodes, overused_ratio*100);
		}

        if (router_opts.congestion_analysis) {
        	congestion_analysis();
        }
		fflush(stdout);
	}
	vpr_printf_info("Routing failed.\n");
	time_on_fanout_analysis();
	return (false);
}


void time_on_fanout_analysis() {
	// using the global time_on_fanout and itry_on_fanout
	vpr_printf_info("fanout low           time (s)        attemps\n");
	for (size_t bin = 0; bin < time_on_fanout.size(); ++bin) {
		if (itry_on_fanout[bin]) {	// avoid printing the many 0 bins
			vpr_printf_info("%4d           %14.3f   %12d\n",bin*fanout_per_bin, time_on_fanout[bin], itry_on_fanout[bin]);
		}
	}
}

// at the end of a routing iteration, profile how much congestion is taken up by each type of rr_node
// efficient bit array for checking against congested type
struct Congested_node_types {
	uint32_t mask;
	Congested_node_types() : mask{0} {}
	void set_congested(int rr_node_type) {mask |= (1 << rr_node_type);}
	void clear_congested(int rr_node_type) {mask &= ~(1 << rr_node_type);}
	bool is_congested(int rr_node_type) const {return mask & (1 << rr_node_type);}
	bool empty() const {return mask == 0;}
};
void congestion_analysis() {
	static const std::vector<const char*> node_typename {
		"SOURCE",
		"SINK",
		"IPIN",
		"OPIN",
		"CHANX",
		"CHANY",
		"ICE"
	};
	// each type indexes into array which holds the congestion for that type
	std::vector<int> congestion_per_type((size_t) NUM_RR_TYPES, 0);
	// print out specific node information if congestion for type is low enough

	int total_congestion = 0;
	for (int inode = 0; inode < num_rr_nodes; ++inode) {
		const t_rr_node& node = rr_node[inode];
		int congestion = node.get_occ() - node.get_capacity();

		if (congestion > 0) {
			total_congestion += congestion;
			congestion_per_type[node.type] += congestion;
		}
	}

	constexpr int specific_node_print_threshold = 5;
	Congested_node_types congested;
	for (int type = SOURCE; type < NUM_RR_TYPES; ++type) {
		float congestion_percentage = (float)congestion_per_type[type] / (float) total_congestion * 100;
		vpr_printf_info(" %6s: %10.6f %\n", node_typename[type], congestion_percentage); 
		// nodes of that type need specific printing
		if (congestion_per_type[type] > 0 &&
			congestion_per_type[type] < specific_node_print_threshold) congested.set_congested(type);
	}

	// specific print out each congested node
	if (!congested.empty()) {
		vpr_printf_info("Specific congested nodes\nxlow ylow   type\n");
		for (int inode = 0; inode < num_rr_nodes; ++inode) {
			const t_rr_node& node = rr_node[inode];
			if (congested.is_congested(node.type) && (node.get_occ() - node.get_capacity()) > 0) {
				vpr_printf_info("(%3d,%3d) %6s\n", node.get_xlow(), node.get_ylow(), node_typename[node.type]);
			}
		}
	}
}

void get_num_bends_and_length_conr(int inet,
    t_rr_to_rg_node_hash_map* node_map, 
    int *bends_ptr,
    int *len_ptr,
    int *segments_ptr) {

	/* Counts and returns the number of bends, wirelength, and number of routing *
	 * resource segments in net inet's routing.                                  */

	struct s_trace *tptr, *prevptr;
	int inode;
	t_rr_type curr_type, prev_type;
	double bends, length, segments;

	bends = 0;
	length = 0;
	segments = 0;
        
        int icon = g_clbs_nlist.net[inet].first_con;
        for (unsigned int isink = 0; isink < g_clbs_nlist.net[inet].pins.size()-1; isink++) {
            prevptr = trace_head_con[icon]; /* Should always be SOURCE. */
            if (prevptr == NULL) {
                    vpr_throw(VPR_ERROR_OTHER, __FILE__, __LINE__,
                                    "in get_num_bends_and_length: net #%d has no traceback.\n", inet);
            }
            inode = prevptr->index;
            prev_type = rr_node[inode].type;

            tptr = prevptr->next;

            while (tptr != NULL) {
                    inode = tptr->index;
                    curr_type = rr_node[inode].type;

                    if (curr_type == SINK) { /* Starting a new segment */
                            tptr = tptr->next; /* Link to existing path - don't add to len. */
                            if (tptr == NULL)
                                    break;

                            curr_type = rr_node[tptr->index].type;
                    }else if (curr_type == CHANX || curr_type == CHANY) {
                        short usage = get_rr_to_rg_node_entry(node_map, inode)->usage;
                        segments += 1.0/usage;
                        length += 1.0/usage * (1 + (rr_node[inode].get_xhigh() - rr_node[inode].get_xlow()
                                            + rr_node[inode].get_yhigh() - rr_node[inode].get_ylow()));

                        if (curr_type != prev_type && (prev_type == CHANX || prev_type == CHANY))
                                bends += 1.0/usage;
                    }

                    prev_type = curr_type;
                    tptr = tptr->next;
            }
            icon++;
        }

	*bends_ptr = (int)round(bends);
	*len_ptr = (int)round(length);
	*segments_ptr = (int)round(segments);
}

bool try_timing_driven_route_con(int icon,
    int itry,
    float pres_fac,
    struct s_router_opts router_opts,
    float* net_delay,
    t_slack* slacks,
    t_rr_to_rg_node_hash_map* node_map) {

    bool is_routed = false;

    if (g_clbs_nlist.net[cons[icon].net].is_fixed) { /* Skip pre-routed nets. */
        is_routed = true;
    } else if (g_clbs_nlist.net[cons[icon].net].is_global) { /* Skip global nets. */
        is_routed = true;
    } else if (should_route_con(icon) == false) {
        is_routed = true;
    } else {
        // track time spent vs fanout
        //		clock_t net_begin = clock();

        rip_up_con_td(icon, node_map, pres_fac);
        
        is_routed = timing_driven_route_con(icon,
                router_opts.max_criticality,
                router_opts.criticality_exp,
                router_opts.astar_fac,
                router_opts.bend_cost,
                net_delay,
                slacks,
                node_map);
        
        add_con_td(icon, net_delay, node_map, pres_fac);

        //		float time_for_con = static_cast<float>(clock() - net_begin) / CLOCKS_PER_SEC;
        //		int net_fanout = g_clbs_nlist.net[inet].num_sinks();
        //		time_on_fanout[net_fanout / fanout_per_bin] += time_for_net;
        //		itry_on_fanout[net_fanout / fanout_per_bin] += 1;

        /* Impossible to route? (disconnected rr_graph) */
        if (is_routed) {
            cons[icon].is_routed = true;
        } else {
            vpr_printf_info("Routing failed.\n");
        }
    }
    return (is_routed);
}

/*
 * NOTE:
 * Suggest using a timing_driven_route_structs struct. Memory is managed for you
 */
static void
alloc_timing_driven_route_structs_and_arrays() {

    /* Allocates all the structures needed only by the timing-driven connection router.   */

    rg_roots = (t_rg_node **) my_calloc(num_nets, sizeof (t_rg_node *));
    rg_sinks = (t_rg_node **) my_calloc(num_cons, sizeof (t_rg_node *));
    
    node_maps = (t_rr_to_rg_node_hash_map*) my_calloc(num_nets, sizeof(t_rr_to_rg_node_hash_map));

    trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    
    back_trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    back_trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    
}
//alloc_route_tree_timing_structs();

/* This function gets ratio of overused nodes (overused_nodes / num_rr_nodes) */
static double get_overused_ratio(){
	double overused_nodes = 0.0;
	int inode;
	for(inode = 0; inode < num_rr_nodes; inode++){
		if(rr_node[inode].get_occ() > rr_node[inode].get_capacity())
			overused_nodes += (rr_node[inode].get_occ() - rr_node[inode].get_capacity());
	}
	overused_nodes /= (double)num_rr_nodes;
	return overused_nodes;
}

static void
free_timing_driven_route_structs_and_arrays() {

    /* Frees all the stuctures needed only by the timing-driven connection router.        */

    //printf("EVDBG => 1\n");
    int inet;
    for(inet = 0; inet<num_nets; inet++){
    		//print_rg(rg_roots[inet],0);
        free_rg(rg_roots[inet]);
        //printf("EVDBG => 1a\n");
        if(node_maps[inet].node_entries != NULL) free(node_maps[inet].node_entries);
        //printf("EVDBG => 1b\n");
    }
    free(node_maps);
    free_rr_to_rg_node_entries();
    //printf("EVDBG => 2\n");
    free_route_graph_timing_structs();
    //printf("EVDBG => 3\n");
    free(rg_roots);
    //printf("EVDBG => 4\n");
    free(rg_sinks);
    //printf("EVDBG => 5\n");
    /*TODO free all trace elements used while routing */

    free(trace_head_con);
    //printf("EVDBG => 6\n");
    free(trace_tail_con);

    //printf("EVDBG => 7\n");
    free(back_trace_head_con);
    //printf("EVDBG => 8\n");
    free(back_trace_tail_con);
    //printf("EVDBG => 9\n");

}

static int get_max_pins_per_net(void) {

	/* Returns the largest number of pins on any non-global net.    */

	unsigned int inet;
	int max_pins_per_net;

	max_pins_per_net = 0;
	for (inet = 0; inet < g_clbs_nlist.net.size(); inet++) {
		if (g_clbs_nlist.net[inet].is_global == false) {
			max_pins_per_net = max(max_pins_per_net,
					(int) g_clbs_nlist.net[inet].pins.size());
		}
	}

	return (max_pins_per_net);
}


static bool
timing_driven_route_con(int icon,
        float max_criticality,
        float criticality_exp,
        float astar_fac,
        float bend_cost,
        float *net_delay,
        t_slack * slacks,
        t_rr_to_rg_node_hash_map* node_map) {

    /* Returns true as long is found some way to hook up this net, even if that *
     * way resulted in overuse of resources (congestion).  If there is no way   *
     * to route this net, even ignoring congestion, it returns false.  In this  *
     * case the rr_graph is disconnected and you can give up.                   */

    int target_node, inode, net;
    float largest_criticality, old_total_path_cost, con_crit, 
            new_total_path_cost, old_back_path_cost, new_back_path_cost;
    struct s_heap *cheapest;
//    struct s_trace *tptr;
    int highfanout_rlim;
    
    net = cons[icon].net;
    target_node = cons[icon].target_node;
//    int fanout = g_clbs_nlist.net[net].pins.size();

    if (!slacks) {
        /* Use criticality of 1. This makes all nets critical.  Note: There is a big difference between setting pin criticality to 0
        compared to 1.  If pin criticality is set to 0, then the current path delay is completely ignored during routing.  By setting
        pin criticality to 1, the current path delay to the pin will always be considered and optimized for */
        con_crit = 1.0;
    } else { 
#ifdef PATH_COUNTING
        /* Con criticality is based on a weighted sum of timing and path criticalities. */	
        con_crit =	ROUTE_PATH_WEIGHT * slacks->path_criticality[net][ipin]
                    + (1 - ROUTE_PATH_WEIGHT) * slacks->timing_criticality[net][ipin]; 
#else
        /* Con criticality is based on only timing criticality. */
        con_crit = slacks->timing_criticality[net][icon - g_clbs_nlist.net[net].first_con + 1];
#endif
        /* Currently, con criticality is between 0 and 1. Now shift it downwards 
        by 1 - max_criticality (max_criticality is 0.99 by default, so shift down
        by 0.01) and cut off at 0.  This means that all pins with small criticalities 
        (<0.01) get criticality 0 and are ignored entirely, and everything
        else becomes a bit less critical. This effect becomes more pronounced if
        max_criticality is set lower. */
        assert(con_crit > -0.01 && con_crit < 1.01);
        con_crit = max(con_crit - (1.0 - max_criticality), 0.0);

        /* Take con criticality to some power (1 by default). */
        con_crit = pow(con_crit, criticality_exp);

        /* Cut off con criticality at max_criticality. */
        con_crit = min(con_crit, max_criticality);
    }

    /* Update base costs according to fanout */
    update_rr_base_costs(net, largest_criticality);

    highfanout_rlim = mark_node_expansion_by_bin(net, target_node, rg_roots[net]);

    /*add source to heap*/
    node_to_heap(cons[icon].source, 0.0, NO_PREVIOUS, NO_PREVIOUS, 0.0, OPEN);

    cheapest = get_heap_head();
	if (cheapest == NULL) { /* Infeasible routing.  No possible path for net. */
		vpr_printf_info("Cannot route net #%d (%s) to sink #%d -- no possible path.\n",
				   cons[icon].net, g_clbs_nlist.net[cons[icon].net].name, target_node);
		reset_path_costs();
		//free_rg(rg_roots);
		return (false);
	}
    
    inode = cheapest->index;

    while (inode != target_node) {
        old_total_path_cost = rr_node_route_inf[inode].path_cost;
        new_total_path_cost = cheapest->cost;
         
        if (old_total_path_cost > 0.99 * HUGE_POSITIVE_FLOAT){ /* First time touched. */
                old_back_path_cost = HUGE_POSITIVE_FLOAT;
        }else{
                old_back_path_cost = rr_node_route_inf[inode].backward_path_cost;
        }
        
        new_back_path_cost = cheapest->backward_path_cost;

        /* I only re-expand a node if both the "known" backward cost is lower  *
         * in the new expansion (this is necessary to prevent loops from       *
         * forming in the routing and causing havoc) *and* the expected total  *
         * cost to the sink is lower than the old value.  Different R_upstream *
         * values could make a path with lower back_path_cost less desirable   *
         * than one with higher cost.  Test whether or not I should disallow   *
         * re-expansion based on a higher total cost.                          */


        if (old_total_path_cost>new_total_path_cost  && old_back_path_cost>new_back_path_cost) { /* New path is lowest cost. */
            rr_node_route_inf[inode].prev_node = cheapest->u.prev_node;
            rr_node_route_inf[inode].prev_edge = cheapest->prev_edge;
            rr_node_route_inf[inode].path_cost = new_total_path_cost;
            rr_node_route_inf[inode].backward_path_cost = new_back_path_cost;

            if (old_total_path_cost > 0.99 * HUGE_POSITIVE_FLOAT) /* First time touched. */
                add_to_mod_list(&rr_node_route_inf[inode].path_cost);
            
            timing_driven_expand_neighbours_con(cheapest,
                    net,
                    bend_cost,
                    con_crit,
                    target_node,
                    astar_fac,
                    highfanout_rlim,
                    node_map);
        }
        

        free_heap_data(cheapest);
        
        cheapest = get_heap_head();

        if (cheapest == NULL) { /* Impossible routing.  No path for net. */
            reset_path_costs();
            return (false);
        }

        inode = cheapest->index;
    }

        /* NB:  In the code below I keep two records of the partial routing:  the   *
         * traceback and the route_tree.  The route_tree enables fast recomputation *
         * of the Elmore delay to each node in the partial routing.  The traceback  *
         * lets me reuse all the routines written for breadth-first routing, which  *
         * all take a traceback structure as input.  Before this routine exits the  *
         * route_tree structure is destroyed; only the traceback is needed at that  *
         * point.                                                                   */
    update_traceback_td_con(cheapest, icon);
    
    free_heap_data(cheapest);

    empty_heap();

    reset_path_costs();

    return (true);
}


static void 
timing_driven_expand_neighbours_con(struct s_heap *current, 
		int inet, 
		float bend_cost, 
		float criticality_fac, 
		int target_node,
		float astar_fac, 
		int highfanout_rlim,
        t_rr_to_rg_node_hash_map* node_map) {

	/* Puts all the rr_nodes adjacent to current on the heap.  rr_nodes outside *
	 * the expanded bounding box specified in route_bb are not added to the     *
	 * heap.                                                                    */

	int edge, to_node, num_edges, inode, iswitch, target_x, target_y;
	t_rr_type from_type, to_type;
	float new_tot_pcost, old_back_pcost, new_back_pcost, R_upstream;
	float new_R_upstream, Tdel;

	inode = current->index;
	old_back_pcost = current->backward_path_cost;
	R_upstream = current->R_upstream;
	num_edges = rr_node[inode].get_num_edges();

	target_x = rr_node[target_node].get_xhigh();
	target_y = rr_node[target_node].get_yhigh();

	for (edge = 0; edge < num_edges; edge++) {
		to_node = rr_node[inode].edges[edge];

		if (g_clbs_nlist.net[inet].num_sinks() >= HIGH_FANOUT_NET_LIM) {
			if (rr_node[to_node].get_xhigh() < target_x - highfanout_rlim
					|| rr_node[to_node].get_xlow() > target_x + highfanout_rlim
					|| rr_node[to_node].get_yhigh() < target_y - highfanout_rlim
					|| rr_node[to_node].get_ylow() > target_y + highfanout_rlim){
				continue; /* Node is outside high fanout bin. */
			}
		}
		else if (rr_node[to_node].get_xhigh() < route_bb[inet].xmin
				|| rr_node[to_node].get_xlow() > route_bb[inet].xmax
				|| rr_node[to_node].get_yhigh() < route_bb[inet].ymin
				|| rr_node[to_node].get_ylow() > route_bb[inet].ymax)
			continue; /* Node is outside (expanded) bounding box. */


		/* Prune away IPINs that lead to blocks other than the target one.  Avoids  *
		 * the issue of how to cost them properly so they don't get expanded before *
		 * more promising routes, but makes route-throughs (via CLBs) impossible.   *
		 * Change this if you want to investigate route-throughs.                   */

		to_type = rr_node[to_node].type;
		if (to_type == IPIN
				&& (rr_node[to_node].get_xhigh() != target_x
						|| rr_node[to_node].get_yhigh() != target_y))
			continue;

		/* new_back_pcost stores the "known" part of the cost to this node -- the   *
		 * congestion cost of all the routing resources back to the existing route  *
		 * plus the known delay of the total path back to the source.  new_tot_cost *
		 * is this "known" backward cost + an expected cost to get to the target.   */

		new_back_pcost = old_back_pcost
				+ (1. - criticality_fac) * get_rr_cong_cost(to_node);
		
		iswitch = rr_node[inode].switches[edge];
		if (g_rr_switch_inf[iswitch].buffered) {
			new_R_upstream = g_rr_switch_inf[iswitch].R;
		} else {
			new_R_upstream = R_upstream + g_rr_switch_inf[iswitch].R;
		}

		Tdel = rr_node[to_node].C * (new_R_upstream + 0.5 * rr_node[to_node].R);
		Tdel += g_rr_switch_inf[iswitch].Tdel;
		new_R_upstream += rr_node[to_node].R;
		new_back_pcost += criticality_fac * Tdel;

		if (bend_cost != 0.) {
			from_type = rr_node[inode].type;
			to_type = rr_node[to_node].type;
			if ((from_type == CHANX && to_type == CHANY)
					|| (from_type == CHANY && to_type == CHANX))
				new_back_pcost += bend_cost;
		}

		float expected_cost = get_timing_driven_expected_cost(to_node, target_node,
				criticality_fac, new_R_upstream);
		new_tot_pcost = new_back_pcost + astar_fac * expected_cost;

		node_to_heap(to_node, new_tot_pcost, inode, edge, new_back_pcost,
				new_R_upstream);

	} /* End for all neighbours */
}

static float get_timing_driven_expected_cost(int inode, 
	int target_node,
	float criticality_fac, 
	float R_upstream) {

	/* Determines the expected cost (due to both delay and resouce cost) to reach *
	 * the target node from inode.  It doesn't include the cost of inode --       *
	 * that's already in the "known" path_cost.                                   */

	t_rr_type rr_type;
	int cost_index, ortho_cost_index, num_segs_same_dir, num_segs_ortho_dir;
	float expected_cost, cong_cost, Tdel, C_total;

    int usage = rr_node_route_inf[inode].usage+1;
	rr_type = rr_node[inode].type;

	if (rr_type == CHANX || rr_type == CHANY) {

#ifdef INTERPOSER_BASED_ARCHITECTURE		
		int num_interposer_hops = get_num_expected_interposer_hops_to_target(inode, target_node);
#endif

		num_segs_same_dir = get_expected_segs_to_target(inode, target_node, &num_segs_ortho_dir);
		cost_index = rr_node[inode].get_cost_index();
		ortho_cost_index = rr_indexed_data[cost_index].ortho_cost_index;

		cong_cost = num_segs_same_dir * rr_indexed_data[cost_index].base_cost / usage +
					num_segs_ortho_dir * rr_indexed_data[ortho_cost_index].base_cost / usage;

		cong_cost += 	rr_indexed_data[IPIN_COST_INDEX].base_cost +
						rr_indexed_data[SINK_COST_INDEX].base_cost;

        C_total = num_segs_same_dir * rr_indexed_data[cost_index].C_load +
                num_segs_ortho_dir * rr_indexed_data[ortho_cost_index].C_load;

		Tdel = 	num_segs_same_dir * rr_indexed_data[cost_index].T_linear +
				num_segs_ortho_dir * rr_indexed_data[ortho_cost_index].T_linear +
				num_segs_same_dir * num_segs_same_dir * rr_indexed_data[cost_index].T_quadratic +
				num_segs_ortho_dir * num_segs_ortho_dir * rr_indexed_data[ortho_cost_index].T_quadratic+
				+ R_upstream * C_total;

		Tdel += rr_indexed_data[IPIN_COST_INDEX].T_linear;

#ifdef INTERPOSER_BASED_ARCHITECTURE
		float interposer_hop_delay = (float)delay_increase * 1e-12;
		Tdel += num_interposer_hops * interposer_hop_delay;
#endif

		expected_cost = criticality_fac * Tdel + (1. - criticality_fac) * cong_cost;
		return (expected_cost);
	}
	else if (rr_type == IPIN) { /* Change if you're allowing route-throughs */
		return (rr_indexed_data[SINK_COST_INDEX].base_cost);
	}
	else { /* Change this if you want to investigate route-throughs */
		return (0.);
	}
}

#ifdef INTERPOSER_BASED_ARCHITECTURE
static int get_num_expected_interposer_hops_to_target(int inode, int target_node) 
{
	/* 
	Description:
		returns the expected number of times you have to cross the 
		interposer in order to go from inode to target_node.
		This does not include inode itself.
	
	Assumptions: 
		1- Cuts are horizontal
		2- target_node is a terminal pin (hence its ylow==yhigh)
		3- wires that go through the interposer are uni-directional

		---------	y=150
		|		|
		---------	y=100
		|		|
		---------	y=50
		|		|
		---------	y=0

		num_cuts = 2, cut_step = 50, cut_locations = {50, 100}
	*/
	int y_start; /* start point y-coordinate. (y-coordinate at the *end* of wire 'i') */
	int y_end;   /* destination (target) y-coordinate */
	int cut_i, y_cut_location, num_expected_hops;
	t_rr_type rr_type;

	num_expected_hops = 0;
	y_end   = rr_node[target_node].get_ylow(); 
	rr_type = rr_node[inode].type;

	if(rr_type == CHANX) 
	{	/* inode is a horizontal wire */
		/* the ylow and yhigh are the same */
		assert(rr_node[inode].get_ylow() == rr_node[inode].get_yhigh());
		y_start = rr_node[inode].get_ylow();
	}
	else
	{	/* inode is a CHANY */
		if(rr_node[inode].direction == INC_DIRECTION)
		{
			y_start = rr_node[inode].get_yhigh();
		}
		else if(rr_node[inode].direction == DEC_DIRECTION)
		{
			y_start = rr_node[inode].get_ylow();
		}
		else
		{
			/*	this means direction is BIDIRECTIONAL! Error out. */
			y_start = -1;
			assert(rr_node[inode].direction!=BI_DIRECTION);
		}
	}

	/* for every cut, is it between 'i' and 'target'? */
	for(cut_i=0 ; cut_i<num_cuts; ++cut_i) 
	{
		y_cut_location = arch_cut_locations[cut_i];
		if( (y_start < y_cut_location &&  y_cut_location < y_end) ||
			(y_end   < y_cut_location &&  y_cut_location < y_start)) 
		{
			++num_expected_hops;
		}
	}

	/* Make there is no off-by-1 error.For current node i: 
	   if it's a vertical wire, node 'i' itself may be crossing the interposer.
	*/
	if(rr_type == CHANY)
	{	
		/* for every cut, does it cut wire 'i'? */
		for(cut_i=0 ; cut_i<num_cuts; ++cut_i) 
		{
			y_cut_location = arch_cut_locations[cut_i];
			if(rr_node[inode].get_ylow() < y_cut_location && y_cut_location < rr_node[inode].get_yhigh())
			{
				++num_expected_hops;
			}
		}
	}

	return num_expected_hops;
}
#endif

/* Macro used below to ensure that fractions are rounded up, but floating   *
 * point values very close to an integer are rounded to that integer.       */

#define ROUND_UP(x) (ceil (x - 0.001))

static int 
get_expected_segs_to_target(int inode, 
		int target_node,
		int *num_segs_ortho_dir_ptr) {

	/* Returns the number of segments the same type as inode that will be needed *
	 * to reach target_node (not including inode) in each direction (the same    *
	 * direction (horizontal or vertical) as inode and the orthogonal direction).*/

	t_rr_type rr_type;
	int target_x, target_y, num_segs_same_dir, cost_index, ortho_cost_index;
	int no_need_to_pass_by_clb;
	float inv_length, ortho_inv_length, ylow, yhigh, xlow, xhigh;

	#ifdef INTERPOSER_BASED_ARCHITECTURE
	int num_expected_hops = get_num_expected_interposer_hops_to_target(inode, target_node);
	#endif
	
	target_x = rr_node[target_node].get_xlow();
	target_y = rr_node[target_node].get_ylow();
	cost_index = rr_node[inode].get_cost_index();
	inv_length = rr_indexed_data[cost_index].inv_length;
	ortho_cost_index = rr_indexed_data[cost_index].ortho_cost_index;
	ortho_inv_length = rr_indexed_data[ortho_cost_index].inv_length;
	rr_type = rr_node[inode].type;

	if (rr_type == CHANX) {
		ylow = rr_node[inode].get_ylow();
		xhigh = rr_node[inode].get_xhigh();
		xlow = rr_node[inode].get_xlow();

		/* Count vertical (orthogonal to inode) segs first. */

		if (ylow > target_y) { /* Coming from a row above target? */
			*num_segs_ortho_dir_ptr =
					(int)(ROUND_UP((ylow - target_y + 1.) * ortho_inv_length));
			no_need_to_pass_by_clb = 1;
		} else if (ylow < target_y - 1) { /* Below the CLB bottom? */
			*num_segs_ortho_dir_ptr = (int)(ROUND_UP((target_y - ylow) *
					ortho_inv_length));
			no_need_to_pass_by_clb = 1;
		} else { /* In a row that passes by target CLB */
			*num_segs_ortho_dir_ptr = 0;
			no_need_to_pass_by_clb = 0;
		}

#ifdef INTERPOSER_BASED_ARCHITECTURE		
		(*num_segs_ortho_dir_ptr) = (*num_segs_ortho_dir_ptr) + 2*num_expected_hops;
#endif
		
		/* Now count horizontal (same dir. as inode) segs. */

		if (xlow > target_x + no_need_to_pass_by_clb) {
			num_segs_same_dir = (int)(ROUND_UP((xlow - no_need_to_pass_by_clb -
							target_x) * inv_length));
		} else if (xhigh < target_x - no_need_to_pass_by_clb) {
			num_segs_same_dir = (int)(ROUND_UP((target_x - no_need_to_pass_by_clb -
							xhigh) * inv_length));
		} else {
			num_segs_same_dir = 0;
		}
	}

	else { /* inode is a CHANY */
		ylow = rr_node[inode].get_ylow();
		yhigh = rr_node[inode].get_yhigh();
		xlow = rr_node[inode].get_xlow();

		/* Count horizontal (orthogonal to inode) segs first. */

		if (xlow > target_x) { /* Coming from a column right of target? */
			*num_segs_ortho_dir_ptr = (int)(
					ROUND_UP((xlow - target_x + 1.) * ortho_inv_length));
			no_need_to_pass_by_clb = 1;
		} else if (xlow < target_x - 1) { /* Left of and not adjacent to the CLB? */
			*num_segs_ortho_dir_ptr = (int)(ROUND_UP((target_x - xlow) *
					ortho_inv_length));
			no_need_to_pass_by_clb = 1;
		} else { /* In a column that passes by target CLB */
			*num_segs_ortho_dir_ptr = 0;
			no_need_to_pass_by_clb = 0;
		}

		/* Now count vertical (same dir. as inode) segs. */

		if (ylow > target_y + no_need_to_pass_by_clb) {
			num_segs_same_dir = (int)(ROUND_UP((ylow - no_need_to_pass_by_clb -
							target_y) * inv_length));
		} else if (yhigh < target_y - no_need_to_pass_by_clb) {
			num_segs_same_dir = (int)(ROUND_UP((target_y - no_need_to_pass_by_clb -
							yhigh) * inv_length));
		} else {
			num_segs_same_dir = 0;
		}
		
#ifdef INTERPOSER_BASED_ARCHITECTURE		
		num_segs_same_dir = num_segs_same_dir + 2*num_expected_hops;
#endif
	}

	return (num_segs_same_dir);
}

static void update_rr_base_costs(int inet, float largest_criticality) {

	/* Changes the base costs of different types of rr_nodes according to the  *
	 * criticality, fanout, etc. of the current net being routed (inet).       */

	float fanout, factor;
	int index;

	fanout = g_clbs_nlist.net[inet].num_sinks();

	/* Other reasonable values for factor include fanout and 1 */
	factor = sqrt(fanout);

	for (index = CHANX_COST_INDEX_START; index < num_rr_indexed_data; index++) {
		if (rr_indexed_data[index].T_quadratic > 0.) { /* pass transistor */
			rr_indexed_data[index].base_cost =
					rr_indexed_data[index].saved_base_cost * factor;
		} else {
			rr_indexed_data[index].base_cost =
					rr_indexed_data[index].saved_base_cost;
		}
	}
}

/* Nets that have high fanout can take a very long time to route. Routing for sinks that are part of high-fanout nets should be
   done within a rectangular 'bin' centered around a target (as opposed to the entire net bounding box) the size of which is returned by this function. */
static int mark_node_expansion_by_bin(int inet, int target_node,
		t_rg_node * rg_node) {
	int target_xlow, target_ylow, target_xhigh, target_yhigh;
	int rlim = 1;
	int inode;
	float area;
	bool success;
	t_linked_rg_edge_ref *link;
	t_rg_node * child_node;

	target_xlow = rr_node[target_node].get_xlow();
	target_ylow = rr_node[target_node].get_ylow();
	target_xhigh = rr_node[target_node].get_xhigh();
	target_yhigh = rr_node[target_node].get_yhigh();

	if (g_clbs_nlist.net[inet].num_sinks() < HIGH_FANOUT_NET_LIM) {
		/* This algorithm only applies to high fanout nets */
		return 1;
	}
	if (rg_node == NULL || rg_node->child_list == NULL) {
		/* If unknown traceback, set radius of bin to be size of chip */
		rlim = max(nx + 2, ny + 2);
		return rlim;
	}

	area = (route_bb[inet].xmax - route_bb[inet].xmin)
			* (route_bb[inet].ymax - route_bb[inet].ymin);
	if (area <= 0) {
		area = 1;
	}

	rlim = (int)(ceil(sqrt((float) area / (float) g_clbs_nlist.net[inet].num_sinks())));

	success = false;
	/* determine quickly a feasible bin radius to route sink for high fanout nets 
	 this is necessary to prevent super long runtimes for high fanout nets; in best case, a reduction in complexity from O(N^2logN) to O(NlogN) (Swartz fast router)
	 */
	link = rg_node->child_list;
	while (success == false && link != NULL) {
		while (link != NULL && success == false) {
			child_node = link->edge->child;
			inode = child_node->inode;
			if (!(rr_node[inode].type == IPIN || rr_node[inode].type == SINK)) {
				if (rr_node[inode].get_xlow() <= target_xhigh + rlim
						&& rr_node[inode].get_xhigh() >= target_xhigh - rlim
						&& rr_node[inode].get_ylow() <= target_yhigh + rlim
						&& rr_node[inode].get_yhigh() >= target_yhigh - rlim) {
					success = true;
				}
			}
			link = link->next;
		}

		if (success == false) {
			if (rlim > max(nx + 2, ny + 2)) {
				vpr_throw(VPR_ERROR_ROUTE, __FILE__, __LINE__, 
					 "VPR internal error, net %s has paths that are not found in traceback.\n", g_clbs_nlist.net[inet].name);
			}
			/* if sink not in bin, increase bin size until fit */
			rlim *= 2;
		} else {
			/* Sometimes might just catch a wire in the end segment, need to give it some channel space to explore */
			rlim += 4;
		}
		link = rg_node->child_list;
	}

	/* adjust rlim to account for width/height of block containing the target sink */
	int target_span = max( target_xhigh - target_xlow, target_yhigh - target_ylow );
	rlim += target_span;

	/* redetermine expansion based on rlim */
	link = rg_node->child_list;
	while (link != NULL) {
		child_node = link->edge->child;
		inode = child_node->inode;
		if (!(rr_node[inode].type == IPIN || rr_node[inode].type == SINK)) {
			if (rr_node[inode].get_xlow() <= target_xhigh + rlim
					&& rr_node[inode].get_xhigh() >= target_xhigh - rlim
					&& rr_node[inode].get_ylow() <= target_yhigh + rlim
					&& rr_node[inode].get_yhigh() >= target_yhigh - rlim) {
				child_node->re_expand = true;
			} else {
				child_node->re_expand = false;
			}
		}
		link = link->next;
	}
	return rlim;
}

static void con_to_net_from_timing_rg(float ** net_delays) {
    unsigned int inet;
    for (inet = 0; inet < g_clbs_nlist.net.size(); inet++) {
        if(!g_clbs_nlist.net[inet].is_global){
            finalize_route_graph(inet,rg_roots[inet],net_delays[inet]);
            trace_head[inet] = trace_from_timing_rg_dbg(rg_roots[inet],inet);
        }
    }
    
}

static t_rg_node* find_non_visited_rec(t_rg_node* node, bool initial_root_status){
    if(node->visited == initial_root_status){
        return node;
    }else{
        if(node->child_list==NULL) return NULL;
        for(t_linked_rg_edge_ref* link= node->child_list; link != NULL; link = link->next){
            t_rg_node* ret = find_non_visited_rec(link->edge->child,initial_root_status);
            if(ret != NULL) return ret;
        }
        return NULL;
    }
}

static void con_to_net(float ** net_delays) {
    unsigned int icon, inet;
    for (inet = 0; inet < g_clbs_nlist.net.size(); inet++) {
        bool found = find_nodes_with_multiple_parents(rg_roots[inet]);
        if(found) printf("inet %d\n",inet);
        trace_head[inet] = NULL;
    }
    
    for (inet = 0; inet < g_clbs_nlist.net.size(); inet++) {
        if(!g_clbs_nlist.net[inet].is_global){
            finalize_route_graph(inet,rg_roots[inet],net_delays[inet]);
        }
    }
    
    printf("Second check starts now...\n");
    for (inet = 0; inet < g_clbs_nlist.net.size(); inet++) {
        bool found = find_nodes_with_multiple_parents(rg_roots[inet]);
        if(found) printf("inet %d\n",inet);
        trace_head[inet] = NULL;
    }
    for (icon = 0; icon < num_cons; icon++) {
        //printf("EVDBG -> con %d\n", icon);
        int netnr = cons[icon].net;
        if (trace_head_con[icon] == NULL) {
            printf("EVDBG -> Warning: trace_head_con of con %d is NULL\n", icon);
        } else {
            if (trace_head[cons[icon].net] == NULL) {
                trace_head[netnr] = trace_head_con[icon];
                //printf("EVDBG -> dbg 2b\n", icon);
                trace_tail[netnr] = trace_tail_con[icon];
            } else {
                //printf("EVDBG -> dbg 3\n", icon);
                //printf("con %d\n", icon);
                struct s_trace* netit;
                struct s_trace* conit = trace_head_con[icon];
                struct s_trace* conprevit = NULL;
                for (; conit->next != NULL; conit = conit->next) {
                    for (netit = trace_head[netnr]; netit != NULL; netit = netit->next) {
                        if (netit->index == conit->index) {
                            conprevit = conit;
                            break;
                        }
                    }
                }
                trace_tail[netnr]->next = conprevit;
                trace_tail[netnr] = trace_tail_con[icon];
            }
        }
    }
    
    
//    struct s_trace* netit = trace_head[1];
//    for (; netit->next != NULL; netit = netit->next) {
//        printf("%d, ", netit->index);
//    }
//    printf("\n");
//#ifdef DEBUG
//    printf("Check if every node in the connection traces is present in the net traces.\n");
//    for (icon = 0; icon < num_cons; icon++) {
//    	int netnr = cons[icon].net;
//        if(trace_head_con[icon]==NULL){
//        	printf("Warning: trace_head_con of con %d is NULL\n",icon);
//        }else{
//			struct s_trace* netit;
//			struct s_trace* conit = trace_head_con[icon];
//			bool found;
//			for(;conit->next!=NULL;conit=conit->next){
//				found = false;
//				for(netit = trace_head[netnr];netit!=NULL;netit = netit->next){
//					if(netit->index == conit->index){
//						found = true;
//						break;
//					}
//				}
//				if(!found) printf("Warning: con %d: node %d is not in the net %d trace.\nThis means that the routing graph of the net is not a tree\n",icon,conit->index,netnr);
//			}
//		}
//    }
//#endif

    
//    int maximum = 0;
//    int totalnodes = 0;
//    for (inet = 0; inet < g_clbs_nlist.net.size(); inet++) {
//        struct s_trace* it = trace_head[inet];
//        int nonodes = 0;
//        while (it != NULL) {
//            it = it->next;
//            nonodes++;
//        }
//        if (nonodes > maximum) maximum = nonodes;
//        totalnodes += nonodes;
//    }
//    printf("Maximum: %d, avg: %f", maximum, (1.0 * totalnodes / g_clbs_nlist.net.size()));
}


static void add_con_td(int con, float *net_delay, t_rr_to_rg_node_hash_map* node_map, float pres_fac) {
    struct s_trace* route_segment_start = trace_head_con[con];
    struct s_trace* tptr;
    int inode, check_sum = 0;
    
    int net = cons[con].net;

    if (route_segment_start == NULL) {/* No routing yet. */
        vpr_printf_error(__FILE__, __LINE__, "no routing in route segment/trace_head_con[con], con %d\n",con);
    }

#ifdef DEBUG
    bool usage_increases = false;
    int previous_usage =  -1;
#endif
    //if(net == 0) printf("\nPath found for con %d (net %d): ",con,net);
    for (tptr = route_segment_start;;) {
        inode = tptr->index;
        check_sum += inode;
        //if(net == 0) printf("%d (%d) (%d) (%d) -> ",inode,rr_node[inode].type, rr_node_route_inf[inode].usage, tptr->iswitch);
#ifdef DEBUG
        if(previous_usage != -1 && rr_node_route_inf[inode].usage > previous_usage){
            usage_increases = true;
        }
        previous_usage = rr_node_route_inf[inode].usage;
#endif
        if (rr_node[inode].type == SINK) {
            if (tptr->next == NULL) {
                break;
            } else {
                tptr = tptr->next;
            }
        }
        tptr = tptr->next;
    } /* End while loop -- did an entire traceback. */
    //if(net == 1) printf("| check_sum %d \n",check_sum);
#ifdef DEBUG
    if(usage_increases){
            //printf("Warning: usage goes up again, loops are introduced in the routing graph!\n");
            printf("*%d-%d*",con,cons[con].net);
    }
#endif


    /*Load out usages*/
    int it=0, node_entries_visited=0;
    for(;it<node_map->size&&node_entries_visited<node_map->no_entries;it++){
        if(node_map->node_entries[it]!=NULL){
        	int node = node_map->node_entries[it]->rr_node;
            rr_node_route_inf[node].usage = 0;
            rr_node_route_inf[node].pres_cost = rr_node_route_inf[node].pres_cost_old;
            node_entries_visited++;
        }
    }

    for (tptr = route_segment_start;;) {
        inode = tptr->index;
        t_rr_to_rg_node_entry* node_entry;
        int occupancy;
        if(rr_node[inode].type == SOURCE && rr_node[inode].type == SINK){
        	occupancy = rr_node[inode].get_occ();
        }else{
        	node_entry = add_rr_to_rg_node(node_map,inode, NULL);
            if(rr_node[inode].get_occ()>0 && node_entry->usage>1){
                occupancy = rr_node[inode].get_occ();
            }else{
                occupancy = rr_node[inode].get_occ() + 1;
            }
        }

        rr_node[inode].set_occ(occupancy);
        int capacity = rr_node[inode].get_capacity();
        if (occupancy < capacity) {
            rr_node_route_inf[inode].pres_cost = 1.;
        } else {
            rr_node_route_inf[inode].pres_cost =
                    1. + (occupancy + 1 - capacity) * pres_fac;
        }
        if (rr_node[inode].type == SINK) {
            if (tptr->next == NULL) {
                trace_tail_con[con] = tptr;
                break;
            } else {
                tptr = tptr->next;
            }
        }
        tptr = tptr->next;
    } /* End while loop -- did an entire traceback. */

    rg_sinks[con] = update_route_graph(con, rg_roots[net], node_map, net_delay);

    ///* For later timing analysis. */
    //update_net_delays_from_route_tree_conr(net_delay, net);


}

static void rip_up_con_td(int con, t_rr_to_rg_node_hash_map* node_map, float pres_fac) {
    struct s_trace* tptr = trace_head_con[con];
    int net  = cons[con].net;
    int inode;
    if (tptr != NULL){
        for (;;) {
            inode = tptr->index;
            t_rr_to_rg_node_entry* node_entry;
            if(rr_node[inode].type == SOURCE && rr_node[inode].type == SINK){
            	node_entry = get_rr_to_rg_node_entry(node_map,inode);
            }else{
            	node_entry = remove_rr_to_rg_node(node_map,inode);
            }
            int occupancy;
            if(node_entry == NULL){
                occupancy = rr_node[inode].get_occ() -1;
            }else{
                occupancy = rr_node[inode].get_occ();
            }
            rr_node[inode].set_occ(occupancy);
            int capacity = rr_node[inode].get_capacity();
            if (occupancy < capacity) {
                rr_node_route_inf[inode].pres_cost = 1.;
            } else {
                rr_node_route_inf[inode].pres_cost =
                        1. + (occupancy + 1 - capacity) * pres_fac;
            }
            if (rr_node[inode].type == SINK) {
                if (tptr->next == NULL) {
                    trace_tail_con[con] = tptr;
                    break;
                } else {
                    tptr = tptr->next;
                }
            }
            tptr = tptr->next;
        } /* End while loop -- did an entire traceback. */
    }

    int it = 0, node_entries_visited = 0;
    for (; it < node_map->size && node_entries_visited < node_map->no_entries; it++) {
        if (node_map->node_entries[it] != NULL) {
            int node = node_map->node_entries[it]->rr_node;
            int usage = node_map->node_entries[it]->usage;
            rr_node_route_inf[node].usage = usage;
            int occ = rr_node[node].get_occ();
            int cap = rr_node[node].get_capacity();
            float pres_cost;
            if (occ < cap) {
                pres_cost = 1.;
            } else {
                pres_cost = 1. + (occ - cap) * pres_fac;
            }
            rr_node_route_inf[node_map->node_entries[it]->rr_node].pres_cost_old = rr_node_route_inf[node_map->node_entries[it]->rr_node].pres_cost;
            rr_node_route_inf[node].pres_cost = pres_cost / (usage + 1);
            rr_node_route_inf[node].usage = usage;
            node_entries_visited++;
        }
    }

    //printf("Remove Connection from Timing Graph...\n");
    remove_connection_from_route_timing_graph(con, rg_roots[net], rg_sinks[con]);

    //printf("free_traceback_td_con...\n");
    free_traceback_td_con(con);

}


#define ERROR_TOL 0.0001

static void timing_driven_check_net_delays(float **net_delay) {

	/* Checks that the net delays computed incrementally during timing driven    *
	 * routing match those computed from scratch by the net_delay.c module.      */

	unsigned int inet, ipin;
	float **net_delay_check;

	t_chunk list_head_net_delay_check_ch = {NULL, 0, NULL};

	/*struct s_linked_vptr *ch_list_head_net_delay_check;*/

	net_delay_check = alloc_net_delay(&list_head_net_delay_check_ch, g_clbs_nlist.net,
		g_clbs_nlist.net.size());
	load_net_delay_from_routing(net_delay_check, g_clbs_nlist.net, g_clbs_nlist.net.size());

	for (inet = 0; inet < g_clbs_nlist.net.size(); inet++) {
		for (ipin = 1; ipin < g_clbs_nlist.net[inet].pins.size(); ipin++) {
			if (net_delay_check[inet][ipin] == 0.) { /* Should be only GLOBAL nets */
				if (fabs(net_delay[inet][ipin]) > ERROR_TOL) {
					vpr_throw(VPR_ERROR_ROUTE, __FILE__, __LINE__, 
						"in timing_driven_check_net_delays: net %d pin %d.\n"
						"\tIncremental calc. net_delay is %g, but from scratch net delay is %g.\n",
						inet, ipin, net_delay[inet][ipin], net_delay_check[inet][ipin]);
				}
			} else {
				if (fabs(1.0 - net_delay[inet][ipin] / net_delay_check[inet][ipin]) > ERROR_TOL) {
                                    //print_trace_to_dot(inet);
					vpr_throw(VPR_ERROR_ROUTE, __FILE__, __LINE__, 
						"in timing_driven_check_net_delays: net %d pin %d (con %d).\n"
						"\tIncremental calc. net_delay is %g, but from scratch net delay is %g.\n",
						inet, ipin, g_clbs_nlist.net[inet].first_con+ipin-1, net_delay[inet][ipin], net_delay_check[inet][ipin]);
				}
			}
		}
	}

	free_net_delay(net_delay_check, &list_head_net_delay_check_ch);
	vpr_printf_info("Completed net delay value cross check successfully.\n");
}

static void print_trace(int inet) {
        printf("Printing trace of net %d\n",inet);
	t_trace * tptr = trace_head[inet];
        printf("tptr %d\n",tptr);
	if (tptr == NULL) {
		/* No routing yet. */
		printf("NULL\n");
	}
    printf("tptr->index %d\n",tptr->index);
	for (;;) {
		int inode = tptr->index;
		//int occ = rr_node[inode].get_occ();
		//int capacity = rr_node[inode].get_capacity();

		if (rr_node[inode].type == SINK) {
                    printf("sink(%d) ",inode);
			tptr = tptr->next; /* Skip next segment. */
                    //printf("hook(%d) -> ",tptr->index);
			if (tptr == NULL)
				break;
		}else{
                    printf("(%d) -> \n",inode);
                }

		tptr = tptr->next;

	} /* End while loop -- did an entire traceback. */
}

static void print_trace_to_dot_from_con(int inet) {
	t_trace * tptr;
        t_trace * prev;
        int pairs[2000];
        for(int i=0; i<2000;i++){
            pairs[i]=-1;
        }
        
        printf("digraph {\n");
        for(unsigned int ipin = 0; ipin < g_clbs_nlist.net[inet].pins.size()-1; ipin++){
            int con = g_clbs_nlist.net[inet].first_con + ipin;
            tptr = trace_head_con[con];
            prev = tptr;
            tptr = tptr->next;
            for (;;) {
                int inode = tptr->index;
                int prev_inode = prev->index;
                int hash = prev_inode*100000+inode;
                bool found = false;
                int i = 0;
                for(; i<2000;i++){
                    if(pairs[i] == -1) break;
                    if(pairs[i] == hash){
                        found = true;
                        break;
                    }
                }
                
                if(!found){
                    printf("\tn%d -> n%d;\n",prev_inode,inode);
                    pairs[i]=hash;
                }
//                else{
//                    printf("found\n",prev_inode,inode);
//                }

                if (rr_node[inode].type == SINK) {
                    break;
                }

                prev = tptr;
                tptr = tptr->next;

            } /* End while loop -- did an entire traceback. */
        }
        printf("}\n");
}

static void print_trace_to_dot(int inet) {
    printf("Printing trace of net %d\n",inet);
    t_trace * tptr = trace_head[inet];
    
    if (tptr == NULL) {
        /* No routing yet. */
        printf("No routing. NULL\n");
        return;
    }
    
    t_trace * prev = tptr;
    tptr = tptr->next;
    
    
    printf("digraph {\n");
    for (;;) {
        
        int inode = tptr->index;
        int prev_inode = prev->index;
        
        printf("\tn%d -> n%d;\n",prev_inode,inode);
        
        if (rr_node[inode].type == SINK) {
            prev = tptr;
            tptr = tptr->next; /* Skip next segment. */
            if (tptr == NULL)
                break;
        }
        
        prev = tptr;
        tptr = tptr->next;
        
    } /* End while loop -- did an entire traceback. */
    printf("}\n");
}


/* Detect if net should be routed or not */
static bool should_route_net(int inet) {
	t_trace * tptr = trace_head[inet];

	if (tptr == NULL) {
		/* No routing yet. */
		return true;
	} 

	for (;;) {
		int inode = tptr->index;
		int occ = rr_node[inode].get_occ();
		int capacity = rr_node[inode].get_capacity();

		if (occ > capacity) {
			return true; /* overuse detected */
		}

		if (rr_node[inode].type == SINK) {
			tptr = tptr->next; /* Skip next segment. */
			if (tptr == NULL)
				break;
		}

		tptr = tptr->next;

	} /* End while loop -- did an entire traceback. */

	return false; /* Current route has no overuse */
}

/* Detect if net should be routed or not */
static bool should_route_con(int icon) {
	t_trace * tptr = trace_head_con[icon];

	if (tptr == NULL) {
		/* No routing yet. */
		return true;
	} 

	for (;;) {
		int inode = tptr->index;
		int occ = rr_node[inode].get_occ();
		int capacity = rr_node[inode].get_capacity();

		if (occ > capacity) {
			return true; /* overuse detected */
		}

		if (rr_node[inode].type == SINK) {
			tptr = tptr->next; /* Skip next segment. */
			if (tptr == NULL)
				break;
		}

		tptr = tptr->next;

	} /* End while loop -- did an entire traceback. */

	return false; /* Current route has no overuse */
}



