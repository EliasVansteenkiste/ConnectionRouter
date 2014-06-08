#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h> 
#include "util.h"
#include "vpr_types.h"
#include "globals.h"
#include "route_export.h"
#include "route_common.h"
#include "croute_tree_timing.h"
#include "croute_timing.h"
#include "heapsort.h"
#include "path_delay.h"
#include "path_delay2.h"
#include "net_delay.h"
#include "stats.h"
#include "ReadOptions.h"

/******************** Subroutines local to route_timing.c ********************/

static int get_max_pins_per_net(void);

static boolean timing_driven_route_con(int icon,
        float max_criticality,
        float criticality_exp,
        float astar_fac,
        float bend_cost,
        float con_slack,
        float T_crit,
        float *net_delay,
        s_rr_to_rg_node_hash_map* node_map);

static void timing_driven_expand_neighbours_con(struct s_heap *current,
		int inet,
        float bend_cost,
        float criticality_fac,
        int target_node,
        float astar_fac,
        int highfanout_rlim,
        s_rr_to_rg_node_hash_map* node_map);

static float get_timing_driven_expected_cost(int inode,
        int target_node,
        float criticality_fac,
        float R_upstream,
        float C_downstream);

static int get_expected_segs_to_target(int inode,
        int target_node,
        int *num_segs_ortho_dir_ptr);

static void update_rr_base_costs(int inet, float largest_criticality) ;

static void timing_driven_check_net_delays(float **net_delay);

static int mark_node_expansion_by_bin(int inet, 
        int target_node, 
        t_rg_node * rt_node);

static void con_to_net();

static void alloc_timing_driven_route_structs();

static void free_timing_driven_route_structs_td();

static void update_net_delays_from_route_tree_conr(float *net_delay, int inet);

static boolean timing_driven_route_con(int icon,
        float max_criticality,
        float criticality_exp,
        float astar_fac,
        float bend_cost,
        float *net_delay,
        t_slack * slacks,
        s_rr_to_rg_node_hash_map* node_map);

static unsigned long no_nodes_expanded;
static t_rg_node ** rg_sinks;
static t_rg_node ** rg_roots;

static void add_con_td(int con, int net, float *net_delay, s_rr_to_rg_node_hash_map* node_map , float pres_fac);

static void rip_up_con_td(int con, int net, s_rr_to_rg_node_hash_map* node_map, float pres_fac);



/************************ Subroutine definitions *****************************/

boolean
try_timing_driven_route_conr(struct s_router_opts router_opts,
        float **net_delay,
        t_slack * slacks,
        t_ivec ** clb_opins_used_locally,
        boolean timing_analysis_enabled) {

    /* Timing-driven connection-based routing algorithm.  The timing graph (includes net_slack)   *
     * must have already been allocated, and net_delay must have been allocated. *
     * Returns TRUE if the routing succeeds, FALSE otherwise.                    */
    
    
    int itry, inet, icon, net, ipin, i, j, bends, segments, wirelength, total_wirelength, available_wirelength;
    boolean success, is_routable, rip_up_local_opins;
    float pres_fac, critical_path_delay, init_timing_criticality_val;


    float *sinks;
    int *net_index;
    
    clock_t c0, c1, c2, c3, c4, ca, cb, begin,end;
    double t_ripup = 0.0, t_route = 0.0, t_add = 0.0;
    float secs = 0.0;
    s_rr_to_rg_node_hash_map* node_map = NULL;
   
    
    sinks = (float*) my_malloc(sizeof (float) * num_nets);
    net_index = (int*) my_malloc(sizeof (int) * num_nets);
    for (i = 0; i < num_nets; i++) {
        sinks[i] = clb_net[i].num_sinks;
        net_index[i] = i;
    }
    heapsort(net_index, sinks, num_nets, 1);
    
    c0 = clock();
    /* Create Connection Array 
     * 1. Calculate the size of the Connection Array*/
    int maxFO = 0;
    num_cons = 0;
    for (inet = 0; inet < num_nets; inet++) {
        if (clb_net[inet].is_global == FALSE) { /* Skip global nets. */
            num_cons += clb_net[inet].num_sinks;
            if(clb_net[inet].num_sinks > maxFO) maxFO = clb_net[inet].num_sinks;
        }
    }
    //printf("Net %d has %d sinks\n",278,clb_net[278].num_sinks);
    /*2. Allocate memory for Connection Array*/
    printf("Allocate memory for Connection Array for %d cons\n", num_cons);
    cons = (s_con*) my_malloc(num_cons * sizeof (s_con));
    /*4. Assign values to connection structs*/
    printf("Assigning values to connection structs\n");
    icon = 0;
    int isink;
    for (i = 0; i < num_nets; i++) {
        int net = net_index[i];
        if (clb_net[net].is_global == FALSE) { /* Skip global nets. */
            clb_net[net].con = icon;
            for (isink = 0; isink < clb_net[net].num_sinks; isink++) {
                cons[icon].net = net;
                cons[icon].source = net_rr_terminals[net][0];
                cons[icon].source_block = clb_net[net].node_block[0];
                cons[icon].source_block_pin = clb_net[net].node_block_pin[0];
                //cons[icon].source_block_port = clb_net[net].node_block_port[0];
                cons[icon].sink_block = clb_net[net].node_block[isink + 1];
                cons[icon].sink_block_pin = clb_net[net].node_block_pin[isink + 1];
                //cons[icon].sink_block_port = clb_net[net].node_block_port[isink + 1];
                cons[icon].target_node = net_rr_terminals[net][isink + 1];
                icon++;

            }
        }
    }

    printf("Analyze nets and check if some connections of the same net have the same destination sink\n");
    for (inet = 0; inet < num_nets; inet++) {
    	for(i = 1;i<=clb_net[inet].num_sinks;i++){
    		for(j = 1;j<=clb_net[inet].num_sinks;j++){
    			if(i!=j&&(net_rr_terminals[inet][i]==net_rr_terminals[inet][j])){
    				printf("WARNING: Net %d: sink %d and sink %d have the same terminal (%d)\n",inet,i,j,net_rr_terminals[inet][i]);
    			}
    		}
    	}
    }

    printf("Allocate timing driven route structs.\n");
    alloc_timing_driven_route_structs();

    printf("Allocate hashmaps per net\n");
    node_maps = (s_rr_to_rg_node_hash_map*) my_calloc(num_nets, sizeof(s_rr_to_rg_node_hash_map));
  
    trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    
    back_trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    back_trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));

    if (timing_analysis_enabled) {
            init_timing_criticality_val = 1.;
    } else {
            init_timing_criticality_val = 0.;
    }
    for (inet = 0; inet < num_nets; inet++) {
        rg_roots[inet] = init_graph_to_source(inet, rg_sinks, &node_maps[inet]);
        if (clb_net[inet].is_global == FALSE) {
                for (ipin = 1; ipin <= clb_net[inet].num_sinks; ipin++)
                        slacks->timing_criticality[inet][ipin] = init_timing_criticality_val;
#ifdef PATH_COUNTING
                        slacks->path_criticality[inet][ipin] = init_timing_criticality_val;
#endif		
        } else { 
                /* Set delay of global signals to zero. Non-global net 
                delays are set by update_net_delays_from_route_tree() 
                inside timing_driven_route_net(), which is only called
                for non-global nets. */
                for (ipin = 1; ipin <= clb_net[inet].num_sinks; ipin++) {
                        net_delay[inet][ipin] = 0.;
                }
        }
    }
    
    pres_fac = router_opts.first_iter_pres_fac; /* Typically 0 -> ignore cong. */
    
    for (itry = 1; itry <= router_opts.max_router_iterations; itry++) {
	begin = clock();
	vpr_printf(TIO_MESSAGE_INFO, "\n");
        vpr_printf(TIO_MESSAGE_INFO, "Routing iteration: %d\n", itry);
                
        for (i = 0; i < num_cons; i++) {
            icon = i;
//            icon = con_index[i];
            net = cons[icon].net;
            //printf("Sink node con %d: target %d, block %d\n",icon,cons[icon].target_node,cons[icon].sink_block);

            
            node_map = &node_maps[net];

            //printf("Rip up Con %d\n",icon);
            rip_up_con_td(icon, net, node_map, pres_fac);
            
            //printf("Route con %d (net %d)\n",icon,net);
            is_routable =
                    timing_driven_route_con(icon,
                    router_opts.max_criticality,
                    router_opts.criticality_exp,
                    router_opts.astar_fac,
                    router_opts.bend_cost,
                    net_delay[net],
                    slacks,
                    node_map);


            /* Impossible to route? (disconnected rr_graph) */
            if (!is_routable) {
            	vpr_printf(TIO_MESSAGE_INFO, "Routing failed.\n");
                free_timing_driven_route_structs_td();
                free(net_index);
                free(sinks);
                return (FALSE);
            }
            
            //printf("Add Con %d\n",icon);
            add_con_td(icon, net, net_delay[net], node_map, pres_fac);
        }
        /* Make sure any CLB OPINs used up by subblocks being hooked directly     *
         * to them are reserved for that purpose.                                 */

        /* TODO migrate early exit code from route_timing.c to here*/
        
        if (itry == 1)
            rip_up_local_opins = FALSE;
        else
            rip_up_local_opins = TRUE;

        reserve_locally_used_opins(pres_fac, rip_up_local_opins,
                clb_opins_used_locally);

        /* Pathfinder guys quit after finding a feasible route. I may want to keep *
         * going longer, trying to improve timing.  Think about this some.         */
        print_and_reset_nr_of_stumbles();
        success = feasible_routing_debug();
        printf("Routing iteration: %d ...\n", itry);
        if (success) {
        	s_rr_to_rg_node_entry * node = NULL;
        	struct s_trace * trace_it = NULL;
        	t_linked_rg_edge_ref * edge_it = NULL;
            printf("Successfully routed after %d routing iterations.\n",itry);

//            node_map = &node_maps[278];
//            for(trace_it=trace_head_con[clb_net[278].con+34-1];trace_it!=NULL;trace_it=trace_it->next){
//            	printf("%d - %d ",trace_it->index,rr_node[trace_it->index].type);
//            	node = get_rr_to_rg_node_entry(node_map,trace_it->index);
//            	if(node==NULL||node->rg_node==NULL) break;
//            	printf(" - Tdel %e, parents: ",node->rg_node->Tdel);
//            	for(edge_it = node->rg_node->u.parent_list;edge_it!=NULL;edge_it = edge_it->next){
//            		if(edge_it!=NULL) printf("%d ", edge_it->edge->u.parent->inode);
//
//            	}
//            	printf(", childeren: ");
//            	for(edge_it = node->rg_node->child_list;edge_it!=NULL;edge_it = edge_it->next){
//            		if(edge_it!=NULL) printf("%d ", edge_it->edge->child->inode);
//            	}
//            	printf("\n");
//            }
//            printf("\n");

            printf("Con to net\n");
            con_to_net();
            printf("free_timing_driven_route_structs_td\n");
            free_timing_driven_route_structs_td();
            printf("cons\n");
            free(cons);
#ifdef DEBUG
            printf("Check net delays\n");
            timing_driven_check_net_delays(net_delay);
#endif
            printf("return\n");
            return (TRUE);
        }

        if (itry == 1) {
            pres_fac = router_opts.initial_pres_fac;
            pathfinder_update_cost(pres_fac, 0.); /* Acc_fac=0 for first iter. */
        } else {
            pres_fac *= router_opts.pres_fac_mult;

            /* Avoid overflow for high iteration counts, even if acc_cost is big */
            pres_fac = std::min(pres_fac, static_cast<float>(HUGE_POSITIVE_FLOAT / 1e5));

            pathfinder_update_cost(pres_fac, router_opts.acc_fac);
        }

        if (timing_analysis_enabled) {		
                /* Update slack values by doing another timing analysis.                 *
                 * Timing_driven_route_net updated the net delay values.                 */

                load_timing_graph_net_delays(net_delay);

#ifdef HACK_LUT_PIN_SWAPPING
                do_timing_analysis(slacks, FALSE, TRUE, FALSE);
#else
                do_timing_analysis(slacks, FALSE, FALSE, FALSE);
#endif

                /* Print critical path delay - convert to nanoseconds. */
                critical_path_delay = get_critical_path_delay();
                vpr_printf(TIO_MESSAGE_INFO, "Critical path: %g ns\n", critical_path_delay);
        } else {
                /* If timing analysis is not enabled, make sure that the criticalities and the 	*
                 * net_delays stay as 0 so that wirelength can be optimized. 			*/

                for (inet = 0; inet < num_nets; inet++) {
                        for (ipin = 1; ipin <= clb_net[inet].num_sinks; ipin++) {
                                slacks->timing_criticality[inet][ipin] = 0.;
#ifdef PATH_COUNTING 		
                                slacks->path_criticality[inet][ipin] = 0.; 		
#endif
                                net_delay[inet][ipin] = 0.;
                        }
                }
        }

        end = clock();
        #ifdef CLOCKS_PER_SEC
                vpr_printf(TIO_MESSAGE_INFO, "Routing iteration took %g seconds.\n", (float)(end - begin) / CLOCKS_PER_SEC);
        #else
                vpr_printf(TIO_MESSAGE_INFO, "Routing iteration took %g seconds.\n", (float)(end - begin) / CLK_PER_SEC);
        #endif

        fflush(stdout);
    }

    vpr_printf(TIO_MESSAGE_INFO, "Routing failed.\n");
    free_timing_driven_route_structs_td();
    free(net_index);
    free(sinks);
    return (FALSE);
}

static void
alloc_timing_driven_route_structs() {

    /* Allocates all the structures needed only by the timing-driven router.   */
    int icon;
    
    rg_roots = (t_rg_node **) my_calloc(num_nets, sizeof (t_rg_node *));
    
    rg_sinks = (t_rg_node **) my_calloc(num_cons, sizeof (t_rg_node *));
    
}

static void
free_timing_driven_route_structs_td() {

    /* Frees all the stuctures needed only by the timing-driven router.        */
    int inet;
    for(inet = 0; inet<num_nets; inet++){
    		//print_rg(rg_roots[inet],0);
        //printf("free_route_tree %d\n",inet);
        free_rg(rg_roots[inet]);
    }
 
    //printf("\nfree rt_roots\n");
    free(rg_roots);
    
    printf("free_route_tree_timing_structs\n");
    free_route_graph_timing_structs();
}

static int
get_max_pins_per_net(void) {

    /* Returns the largest number of pins on any non-global net.    */

    int inet, max_pins_per_net;

    max_pins_per_net = 0;
    for (inet = 0; inet < num_nets; inet++) {
        if (clb_net[inet].is_global == FALSE) {
            max_pins_per_net =
                    std::max(max_pins_per_net, (clb_net[inet].num_sinks + 1));
        }
    }

    return (max_pins_per_net);
}

static boolean
timing_driven_route_con(int icon,
        float max_criticality,
        float criticality_exp,
        float astar_fac,
        float bend_cost,
        float *net_delay,
        t_slack * slacks,
        s_rr_to_rg_node_hash_map* node_map) {

    /* Returns TRUE as long is found some way to hook up this net, even if that *
     * way resulted in overuse of resources (congestion).  If there is no way   *
     * to route this net, even ignoring congestion, it returns FALSE.  In this  *
     * case the rr_graph is disconnected and you can give up.                   */

    int i, ipin, num_sinks, itarget, target_node, inode, net, fanout;
    float largest_criticality, pin_crit, old_total_path_cost, con_crit, 
            new_total_path_cost, old_back_path_cost, new_back_path_cost;
    struct s_heap *current;
    struct s_trace *tptr;
    int highfanout_rlim;
    
    net = cons[icon].net;
    target_node = cons[icon].target_node;
    fanout = clb_net[net].num_sinks;

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
        con_crit = slacks->timing_criticality[net][icon - clb_net[net].con+1];
#endif
        /* Currently, con criticality is between 0 and 1. Now shift it downwards 
        by 1 - max_criticality (max_criticality is 0.99 by default, so shift down
        by 0.01) and cut off at 0.  This means that all pins with small criticalities 
        (<0.01) get criticality 0 and are ignored entirely, and everything
        else becomes a bit less critical. This effect becomes more pronounced if
        max_criticality is set lower. */
        assert(con_crit > -0.01 && con_crit < 1.01);
        con_crit = std::max(con_crit - (1.0 - max_criticality), 0.0);

        /* Take con criticality to some power (1 by default). */
        con_crit = pow(con_crit, criticality_exp);

        /* Cut off con criticality at max_criticality. */
        con_crit = std::min(con_crit, max_criticality);
    }

    /* Update base costs according to fanout */
    update_rr_base_costs(net, largest_criticality);

    highfanout_rlim = mark_node_expansion_by_bin(net, target_node, rg_roots[net]);

    /*add source to heap*/
    node_to_heap(cons[icon].source, 0.0, NO_PREVIOUS, NO_PREVIOUS, 0.0, OPEN);
    //? nodig was aanwezig in croute_ds
    tptr = NULL;
    current = get_heap_head();
    if (current == NULL) { /* Infeasible routing.  No possible path for net. */
        reset_path_costs();
        return (FALSE);
    }
    
    inode = current->index;

    //printf("icon %d, con_crit %f\n",icon,con_crit);
    while (inode != target_node) {
        old_total_path_cost = rr_node_route_inf[inode].path_cost;
        new_total_path_cost = current->cost;
         
        if (old_total_path_cost > 0.99 * HUGE_POSITIVE_FLOAT){ /* First time touched. */
                old_back_path_cost = HUGE_POSITIVE_FLOAT;
        }else{
                old_back_path_cost = rr_node_route_inf[inode].backward_path_cost;
        }
        
        new_back_path_cost = current->backward_path_cost;
        /* I only re-expand a node if both the "known" backward cost is lower  *
         * in the new expansion (this is necessary to prevent loops from       *
         * forming in the routing and causing havoc) *and* the expected total  *
         * cost to the sink is lower than the old value.  Different R_upstream *
         * values could make a path with lower back_path_cost less desirable   *
         * than one with higher cost.  Test whether or not I should disallow   *
         * re-expansion based on a higher total cost.                          */


        if (old_total_path_cost>new_total_path_cost  && old_back_path_cost>new_back_path_cost) { /* New path is lowest cost. */
            rr_node_route_inf[inode].prev_node = current->u.prev_node;
            rr_node_route_inf[inode].prev_edge = current->prev_edge;
            rr_node_route_inf[inode].path_cost = new_total_path_cost;
            rr_node_route_inf[inode].backward_path_cost = new_back_path_cost;

            if (old_total_path_cost > 0.99 * HUGE_POSITIVE_FLOAT) /* First time touched. */
                add_to_mod_list(&rr_node_route_inf[inode].path_cost);
            
            timing_driven_expand_neighbours_con(current,
                    net,
                    bend_cost,
                    con_crit,
                    target_node,
                    astar_fac,
					highfanout_rlim,
                    node_map);
        }
        

        free_heap_data(current);
        
        current = get_heap_head();

        if (current == NULL) { /* Impossible routing.  No path for net. */
            reset_path_costs();
            return (FALSE);
        }

        inode = current->index;
    }

        /* NB:  In the code below I keep two records of the partial routing:  the   *
         * traceback and the route_tree.  The route_tree enables fast recomputation *
         * of the Elmore delay to each node in the partial routing.  The traceback  *
         * lets me reuse all the routines written for breadth-first routing, which  *
         * all take a traceback structure as input.  Before this routine exits the  *
         * route_tree structure is destroyed; only the traceback is needed at that  *
         * point.                                                                   */
    update_traceback_td_con(current, icon);
    
    free_heap_data(current);

    empty_heap();

    reset_path_costs();

    return (TRUE);
}

static void
timing_driven_expand_neighbours_con(struct s_heap *current,
		int inet,
        float bend_cost,
        float criticality_fac,
        int target_node,
        float astar_fac,
        int highfanout_rlim,
        s_rr_to_rg_node_hash_map* node_map) {

    /* Puts all the rr_nodes adjacent to current on the heap.  rr_nodes outside *
     * the expanded bounding box specified in route_bb are not added to the     *
     * heap.                                                                    */

    int edge, to_node, num_edges, inode, iswitch, target_x, target_y;
    short current_node_usage;
    t_rr_type from_type, to_type;
    float new_tot_path_cost, old_back_pcost, new_back_pcost, R_upstream;
    float new_R_upstream, Tdel;

    inode = current->index;
    old_back_pcost = current->backward_path_cost;
    R_upstream = current->R_upstream;
    num_edges = rr_node[inode].num_edges;

    target_x = rr_node[target_node].xhigh;
    target_y = rr_node[target_node].yhigh;
    

    current_node_usage = rr_node_route_inf[inode].usage;
//    if(current_node_usage == 0){
        for (edge = 0; edge < num_edges; edge++) {
            to_node = rr_node[inode].edges[edge];
            

            if (rr_node[to_node].xhigh < route_bb[inet].xmin ||
                    rr_node[to_node].xlow > route_bb[inet].xmax ||
                    rr_node[to_node].yhigh < route_bb[inet].ymin ||
                    rr_node[to_node].ylow > route_bb[inet].ymax)
                continue; /* Node is outside (expanded) bounding box. */

            if (clb_net[inet].num_sinks >= HIGH_FANOUT_NET_LIM) {
                if (rr_node[to_node].xhigh < target_x - highfanout_rlim ||
                        rr_node[to_node].xlow > target_x + highfanout_rlim ||
                        rr_node[to_node].yhigh < target_y - highfanout_rlim ||
                        rr_node[to_node].ylow > target_y + highfanout_rlim)
                    continue; /* Node is outside high fanout bin. */
            }


            /* Prune away IPINs that lead to blocks other than the target one.  Avoids  *
             * the issue of how to cost them properly so they don't get expanded before *
             * more promising routes, but makes route-throughs (via CLBs) impossible.   *
             * Change this if you want to investigate route-throughs.                   */

            to_type = rr_node[to_node].type;
            if (to_type == IPIN && (rr_node[to_node].xhigh != target_x ||
                    rr_node[to_node].yhigh != target_y))
                continue;




            /* new_back_pcost stores the "known" part of the cost to this node -- the   *
             * congestion cost of all the routing resources back to the existing route  *
             * plus the known delay of the total path back to the source.  new_tot_cost *
             * is this "known" backward cost + an expected cost to get to the target.   */

            new_back_pcost = old_back_pcost + (1. - criticality_fac) * get_rr_cong_cost(to_node);

            iswitch = rr_node[inode].switches[edge];
            if (switch_inf[iswitch].buffered) {
                new_R_upstream = switch_inf[iswitch].R;
            } else {
                new_R_upstream = R_upstream + switch_inf[iswitch].R;
            }

            Tdel = rr_node[to_node].C * (new_R_upstream +
                    0.5 * rr_node[to_node].R);
            Tdel += switch_inf[iswitch].Tdel;
            new_R_upstream += rr_node[to_node].R;
            new_back_pcost += criticality_fac * Tdel;

            if (bend_cost != 0.) {
                from_type = rr_node[inode].type;
                to_type = rr_node[to_node].type;
                if ((from_type == CHANX && to_type == CHANY) ||
                        (from_type == CHANY && to_type == CHANX))
                    new_back_pcost += bend_cost;
            }

            float C_downstream;
            if(get_rr_to_rg_node_entry(node_map,to_node) != NULL){
            	if(get_rr_to_rg_node_entry(node_map,to_node)->rg_node==NULL && to_type == SINK){
            		C_downstream = 0.0;
            	}else{
            		C_downstream = get_rr_to_rg_node_entry(node_map,to_node)->rg_node->C_downstream;
            	}
            }
            else C_downstream = 0.0;
            new_tot_path_cost = new_back_pcost + astar_fac *
                    get_timing_driven_expected_cost(to_node, target_node,
                    criticality_fac,
                    new_R_upstream,
                    C_downstream);

            node_to_heap(to_node, new_tot_path_cost, inode, edge, new_back_pcost,
                    new_R_upstream);

        } /* End for all neighbours */
}

static float
get_timing_driven_expected_cost(int inode,
        int target_node,
        float criticality_fac,
        float R_upstream,
        float C_downstream) {

    /* Determines the expected cost (due to both delay and resouce cost) to reach *
     * the target node from inode.  It doesn't include the cost of inode --       *
     * that's already in the "known" path_cost.                                   */

    t_rr_type rr_type;
    int cost_index, ortho_cost_index, num_segs_same_dir, num_segs_ortho_dir;
    float expected_cost, cong_cost, Tdel, C_total;
    int usage = rr_node_route_inf[inode].usage+1;

    rr_type = rr_node[inode].type;

    if (rr_type == CHANX || rr_type == CHANY) {
        num_segs_same_dir = get_expected_segs_to_target(inode, target_node,
                &num_segs_ortho_dir);
        cost_index = rr_node[inode].cost_index;
        ortho_cost_index = rr_indexed_data[cost_index].ortho_cost_index;

        cong_cost =
                num_segs_same_dir * rr_indexed_data[cost_index].base_cost / usage +
                num_segs_ortho_dir * rr_indexed_data[ortho_cost_index].base_cost / usage;
        cong_cost +=
                rr_indexed_data[IPIN_COST_INDEX].base_cost +
                rr_indexed_data[SINK_COST_INDEX].base_cost;

        C_total = num_segs_same_dir * rr_indexed_data[cost_index].C_load +
                num_segs_ortho_dir * rr_indexed_data[ortho_cost_index].C_load;

//        if((C_downstream-C_total)>(1.0e-14)){
//        	C_total = C_downstream;
//                printf("C_downstream - C_total %e\n",(C_downstream-C_total));
//                printf("1.0e-14 %e\n",1.0e-14);
//        	printf("C_downstream %e, C_total %e\n",C_downstream,C_total); 
//                printf("((C_downstream-C_total)>(1.0e-14)) %d\n",((C_downstream-C_total)>(1.0e-14)));
//        }
    	//printf("rr_indexed_data[ortho_cost_index].C_load %e\n",rr_indexed_data[cost_index].C_load);

        Tdel = num_segs_same_dir * rr_indexed_data[cost_index].T_linear +
                num_segs_ortho_dir * rr_indexed_data[ortho_cost_index].T_linear +
                num_segs_same_dir * num_segs_same_dir * rr_indexed_data[cost_index].T_quadratic +
                num_segs_ortho_dir * num_segs_ortho_dir * rr_indexed_data[ortho_cost_index].T_quadratic +
                R_upstream * (C_total);


        Tdel += rr_indexed_data[IPIN_COST_INDEX].T_linear;

        expected_cost =
                criticality_fac * Tdel + (1. - criticality_fac) * cong_cost;

        return (expected_cost);
    }
    else if (rr_type == IPIN) { /* Change if you're allowing route-throughs */
        return (rr_indexed_data[SINK_COST_INDEX].base_cost);
    }
    else { /* Change this if you want to investigate route-throughs */
        return (0.);
    }
}

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

    target_x = rr_node[target_node].xlow;
    target_y = rr_node[target_node].ylow;
    cost_index = rr_node[inode].cost_index;
    inv_length = rr_indexed_data[cost_index].inv_length;
    ortho_cost_index = rr_indexed_data[cost_index].ortho_cost_index;
    ortho_inv_length = rr_indexed_data[ortho_cost_index].inv_length;
    rr_type = rr_node[inode].type;

    if (rr_type == CHANX) {
        ylow = rr_node[inode].ylow;
        xhigh = rr_node[inode].xhigh;
        xlow = rr_node[inode].xlow;

        /* Count vertical (orthogonal to inode) segs first. */

        if (ylow > target_y) { /* Coming from a row above target? */
            *num_segs_ortho_dir_ptr =
                    ROUND_UP((ylow - target_y + 1.) * ortho_inv_length);
            no_need_to_pass_by_clb = 1;
        } else if (ylow < target_y - 1) { /* Below the CLB bottom? */
            *num_segs_ortho_dir_ptr = ROUND_UP((target_y - ylow) *
                    ortho_inv_length);
            no_need_to_pass_by_clb = 1;
        } else { /* In a row that passes by target CLB */
            *num_segs_ortho_dir_ptr = 0;
            no_need_to_pass_by_clb = 0;
        }

        /* Now count horizontal (same dir. as inode) segs. */

        if (xlow > target_x + no_need_to_pass_by_clb) {
            num_segs_same_dir =
                    ROUND_UP((xlow - no_need_to_pass_by_clb -
                    target_x) * inv_length);
        } else if (xhigh < target_x - no_need_to_pass_by_clb) {
            num_segs_same_dir =
                    ROUND_UP((target_x - no_need_to_pass_by_clb -
                    xhigh) * inv_length);
        } else {
            num_segs_same_dir = 0;
        }
    }
    else { /* inode is a CHANY */
        ylow = rr_node[inode].ylow;
        yhigh = rr_node[inode].yhigh;
        xlow = rr_node[inode].xlow;

        /* Count horizontal (orthogonal to inode) segs first. */

        if (xlow > target_x) { /* Coming from a column right of target? */
            *num_segs_ortho_dir_ptr =
                    ROUND_UP((xlow - target_x + 1.) * ortho_inv_length);
            no_need_to_pass_by_clb = 1;
        } else if (xlow < target_x - 1) { /* Left of and not adjacent to the CLB? */
            *num_segs_ortho_dir_ptr = ROUND_UP((target_x - xlow) *
                    ortho_inv_length);
            no_need_to_pass_by_clb = 1;
        } else { /* In a column that passes by target CLB */
            *num_segs_ortho_dir_ptr = 0;
            no_need_to_pass_by_clb = 0;
        }

        /* Now count vertical (same dir. as inode) segs. */

        if (ylow > target_y + no_need_to_pass_by_clb) {
            num_segs_same_dir =
                    ROUND_UP((ylow - no_need_to_pass_by_clb -
                    target_y) * inv_length);
        } else if (yhigh < target_y - no_need_to_pass_by_clb) {
            num_segs_same_dir =
                    ROUND_UP((target_y - no_need_to_pass_by_clb -
                    yhigh) * inv_length);
        } else {
            num_segs_same_dir = 0;
        }
    }

    return (num_segs_same_dir);
}

static void update_rr_base_costs(int inet, float largest_criticality) {

	/* Changes the base costs of different types of rr_nodes according to the  *
	 * criticality, fanout, etc. of the current net being routed (inet).       */

	float fanout, factor;
	int index;

	fanout = clb_net[inet].num_sinks;

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

/* Nets that have high fanout can take a very long time to route.  Each sink should be routed contained within a bin instead of the entire bounding box to speed things up */
static int mark_node_expansion_by_bin(int inet, int target_node, t_rg_node * rg_node) {
    int target_x, target_y;
    int rlim = 1;
    int inode;
    float area;
    boolean success;
    t_linked_rg_edge_ref *link;
    t_rg_node * child_node;

    target_x = rr_node[target_node].xlow;
    target_y = rr_node[target_node].ylow;

    if (clb_net[inet].num_sinks < HIGH_FANOUT_NET_LIM) {
        /* This algorithm only applies to high fanout nets */
        return 1;
    }

    area = (route_bb[inet].xmax - route_bb[inet].xmin) * (route_bb[inet].ymax - route_bb[inet].ymin);
    if (area <= 0) {
        area = 1;
    }

    rlim = ceil(sqrt((float) area / (float) clb_net[inet].num_sinks));
    if (rg_node == NULL || rg_node->child_list == NULL) {
        /* If unknown traceback, set radius of bin to be size of chip */
        rlim = std::max(nx + 2, ny + 2);
        return rlim;
    }

    success = FALSE;
    /* determine quickly a feasible bin radius to route sink for high fanout nets 
       this is necessary to prevent super long runtimes for high fanout nets; in best case, a reduction in complexity from O(N^2logN) to O(NlogN) (Swartz fast router)
     */
    link = rg_node->child_list;
    while (success == FALSE && link != NULL) {
        while (link != NULL && success == FALSE) {
            child_node = link->edge->child;
            inode = child_node->inode;
            if (!(rr_node[inode].type == IPIN || rr_node[inode].type == SINK)) {
                if (rr_node[inode].xlow <= target_x + rlim &&
                        rr_node[inode].xhigh >= target_x - rlim &&
                        rr_node[inode].ylow <= target_y + rlim &&
                        rr_node[inode].yhigh >= target_y - rlim) {
                    success = TRUE;
                }
            }
            link = link->next;
        }

        if (success == FALSE) {
            if (rlim > std::max(nx + 2, ny + 2)) {
                printf(ERRTAG "VPR internal error, net %s has paths that are not found in traceback\n", clb_net[inet].name);
                exit(1);
            }
            /* if sink not in bin, increase bin size until fit */
            rlim *= 2;
        } else {
            /* Sometimes might just catch a wire in the end segment, need to give it some channel space to explore */
            rlim += 4;
        }
        link = rg_node->child_list;
    }

    /* redetermine expansion based on rlim */
    link = rg_node->child_list;
    while (link != NULL) {
        child_node = link->edge->child;
        inode = child_node->inode;
        if (!(rr_node[inode].type == IPIN || rr_node[inode].type == SINK)) {
            if (rr_node[inode].xlow <= target_x + rlim &&
                    rr_node[inode].xhigh >= target_x - rlim &&
                    rr_node[inode].ylow <= target_y + rlim &&
                    rr_node[inode].yhigh >= target_y - rlim) {
                child_node->re_expand = TRUE;
            } else {
                child_node->re_expand = FALSE;
            }
        }
        link = link->next;
    }
    return rlim;
}

//static void
//compute_con_slacks(float **con_slack)
//{
//
///* Puts the slack of each source-sink pair of block pins in net_slack.     */
//
//    int inet, iedge, inode, to_node, num_edges, first_con;
//    t_tedge *tedge;
//    float T_arr, Tdel, T_req;
//
//    for(inet = 0; inet < num_nets; inet++){
//        inode = net_to_driver_tnode[inet];
//	T_arr = tnode[inode].T_arr;
//	num_edges = tnode[inode].num_edges;
//	tedge = tnode[inode].out_edges;
//        first_con = clb_net[inet].con;
//        for(iedge = 0; iedge < num_edges; iedge++){
//            to_node = tedge[iedge].to_node;
//            Tdel = tedge[iedge].Tdel;
//            T_req = tnode[to_node].T_req;
//            *con_slack[first_con+iedge] = T_req - T_arr - Tdel;
//        }
//    }
//}

static void
update_net_delays_from_route_tree_conr(float *net_delays, int inet)
{

/* Goes through all the sinks of this net and copies their delay values from *
 * the route_tree to the net_delay array.                                    */

    int isink;
    t_rg_node *sink_rt_node;
    int con = clb_net[inet].con;

    for(isink = 1; isink <= clb_net[inet].num_sinks; isink++){
        sink_rt_node = rg_sinks[con];
        if(sink_rt_node!=NULL){
            net_delays[isink] = sink_rt_node->Tdel;
            con++;
        }
    }
}


static void con_to_net() {
    int icon, inet;
    for (inet = 0; inet < num_nets; inet++){
        trace_head[inet] = NULL;
    }
    for (icon = 0; icon < num_cons; icon++) {
        int netnr = cons[icon].net;
        if(trace_head_con[icon]==NULL){
        	printf("Warning: trace_head_con of con %d is NULL\n",icon);
        }else{
			if (trace_head[cons[icon].net] == NULL) {
				trace_head[netnr] = trace_head_con[icon];
				trace_tail[netnr] = trace_tail_con[icon];
			} else {
				struct s_trace* netit;
				struct s_trace* conit = trace_head_con[icon];
				struct s_trace* conprevit = NULL;
				for(;conit->next!=NULL;conit=conit->next){
					for(netit = trace_head[netnr];netit!=NULL;netit = netit->next){
						if(netit->index == conit->index){
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
//#ifdef DEBUG
//    printf("Check if every node in the connection traces is present in the net traces.\n");
//    for (icon = 0; icon < num_cons; icon++) {
//    	int netnr = cons[icon].net;
//        if(trace_head_con[icon]==NULL){
//        	printf("Warning: trace_head_con of con %d is NULL\n",icon);
//        }else{
//			struct s_trace* netit;
//			struct s_trace* conit = trace_head_con[icon];
//			boolean found;
//			for(;conit->next!=NULL;conit=conit->next){
//				found = FALSE;
//				for(netit = trace_head[netnr];netit!=NULL;netit = netit->next){
//					if(netit->index == conit->index){
//						found = TRUE;
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
//    for (inet = 0; inet < num_nets; inet++) {
//        struct s_trace* it = trace_head[inet];
//        int nonodes = 0;
//        while (it != NULL) {
//            it = it->next;
//            nonodes++;
//        }
//        if (nonodes > maximum) maximum = nonodes;
//        totalnodes += nonodes;
//    }
//    printf("Maximum: %d, avg: %f", maximum, (1.0 * totalnodes / num_nets));
}

static void add_con_td(int con, int net, float *net_delay, s_rr_to_rg_node_hash_map* node_map, float pres_fac) {
    struct s_trace* route_segment_start = trace_head_con[con];
    struct s_trace* tptr;
    int inode;

    if (route_segment_start == NULL) {/* No routing yet. */
        printf("Error in add_con_fast: no routing in route segment.\n");
        exit(10);
    }

#ifdef DEBUG
    boolean usage_increases = FALSE;
    int previous_usage =  -1;
#endif
    //printf("Path found: ");
    for (tptr = route_segment_start;;) {
        inode = tptr->index;
        //printf("%d (%d) (%d) -> ",inode,rr_node[inode].type, rr_node_route_inf[inode].usage);
#ifdef DEBUG
        if(previous_usage != -1 && rr_node_route_inf[inode].usage > previous_usage){
            usage_increases = TRUE;
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
    //printf("\n");
#ifdef DEBUG
    if(usage_increases){
            //printf("Warning: usage goes up again, loops are induced in the routing graph!\n");
            printf("*%d*",con);
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
        s_rr_to_rg_node_entry* node_entry;
        int occupancy;
        if(rr_node[inode].type == SOURCE && rr_node[inode].type == SINK){
        	occupancy = rr_node[inode].occ;
        }else{
        	node_entry = add_rr_to_rg_node(node_map,inode, NULL);
            if(rr_node[inode].occ>0 && node_entry->usage>1){
                occupancy = rr_node[inode].occ;
            }else{
                occupancy = rr_node[inode].occ + 1;
            }
        }

        rr_node[inode].occ = occupancy;
        int capacity = rr_node[inode].capacity;
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

static void rip_up_con_td(int con, int net, s_rr_to_rg_node_hash_map* node_map, float pres_fac) {
    struct s_trace* tptr = trace_head_con[con];
    int inode, occupancy, capacity;
    if (tptr != NULL){
        for (;;) {
            inode = tptr->index;
            s_rr_to_rg_node_entry* node_entry;
            if(rr_node[inode].type == SOURCE && rr_node[inode].type == SINK){
            	node_entry = get_rr_to_rg_node_entry(node_map,inode);
            }else{
            	node_entry = remove_rr_to_rg_node(node_map,inode);
            }
            if(node_entry == NULL){
                occupancy = rr_node[inode].occ -1;
            }else{
                occupancy = rr_node[inode].occ;
            }
            rr_node[inode].occ = occupancy;
            capacity = rr_node[inode].capacity;
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
            int occ = rr_node[node].occ;
            int cap = rr_node[node].capacity;
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

	int inet, ipin;
	float **net_delay_check;

	t_chunk list_head_net_delay_check_ch = {NULL, 0, NULL};

	/*struct s_linked_vptr *ch_list_head_net_delay_check;*/

	net_delay_check = alloc_net_delay(&list_head_net_delay_check_ch, clb_net,
			num_nets);
	load_net_delay_from_routing(net_delay_check, clb_net, num_nets);

	for (inet = 0; inet < num_nets; inet++) {
		for (ipin = 1; ipin <= clb_net[inet].num_sinks; ipin++) {
			if (net_delay_check[inet][ipin] == 0.) { /* Should be only GLOBAL nets */
				if (fabs(net_delay[inet][ipin]) > ERROR_TOL) {
					vpr_printf(TIO_MESSAGE_ERROR, "in timing_driven_check_net_delays: net %d pin %d.\n",
							inet, ipin);
					vpr_printf(TIO_MESSAGE_ERROR, "\tIncremental calc. net_delay is %g, but from scratch net delay is %g.\n",
							net_delay[inet][ipin], net_delay_check[inet][ipin]);
					exit(1);
				}
			} else {
				if (fabs(1.0 - net_delay[inet][ipin] / net_delay_check[inet][ipin]) > ERROR_TOL) {
					vpr_printf(TIO_MESSAGE_ERROR, "in timing_driven_check_net_delays: net %d pin %d.\n",
							inet, ipin);
					vpr_printf(TIO_MESSAGE_ERROR, "\tIncremental calc. net_delay is %g, but from scratch net delay is %g.\n",
							net_delay[inet][ipin], net_delay_check[inet][ipin]);
					exit(1);
				}
			}
		}
	}

	free_net_delay(net_delay_check, &list_head_net_delay_check_ch);
	vpr_printf(TIO_MESSAGE_INFO, "Completed net delay value cross check successfully.\n");
}
