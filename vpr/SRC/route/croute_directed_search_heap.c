#include <stdio.h>
#include <math.h>
#include <time.h>
//#include <sys/types.h>
#include "util.h"
#include "vpr_types.h"
#include "globals.h"
//#include "mst.h"
#include "route_export.h"
#include "route_common.h"
#include "route_tree_timing.h"
#include "route_timing.h"
#include "heapsort.h"
#include "path_delay.h"
#include "net_delay.h"
#include "stats.h"

/******************** Subroutines local to route_timing.c ********************/

static int get_expected_segs_to_target(int inode,
        int target_node,
        int *num_segs_ortho_dir_ptr);

static void con_to_net();

static boolean route_con(int icon,
        t_node_hash_map* node_hash_map,
        float bend_cost, 
        float pres_fac,
        float astar_fac);

static void expand_neighbours(int inode,
        int net,
        int target_node,
        int fanout,
        float old_back_cost,
        float bend_cost,
        float pres_fac,
        float astar_fac);

static float get_expected_cost(int inode,
        int target_node);

unsigned long no_nodes_expanded;

static struct s_con_heap **con_heap;
static int con_heap_size; /* Number of slots in the heap array */
static int con_heap_tail; /* Index of first unused slot in the heap array */

/************************ Subroutine definitions *****************************/

boolean
try_conr_ds_route_heap(struct s_router_opts router_opts,
        t_ivec ** clb_opins_used_locally) {

    int itry, inet, icon, ipin, i;
    boolean success, is_routable, rip_up_local_opins;
    t_rt_node **rt_node_of_sink; /* [1..max_pins_per_net-1]. */
    float pres_fac;

    int bends;
    int wirelength, total_wirelength, available_wirelength;
    int segments;
    
    clock_t c0, c1, c2, c3, c4, ca, cb;
    double t_ripup = 0.0, t_route = 0.0, t_add = 0.0;
    float secs = 0.0;
    
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
    /*2. Allocate memory for Connection Array*/
    printf("Allocate memory for Connection Array for %d cons\n", num_cons);
    //num_cons = 3000;
    cons = (t_con*) my_malloc(num_cons * sizeof (t_con));
    /*3. Assign values to connection structs*/
    printf("Assigning values to connection structs\n");
    icon = 0;
    int isink;
    for (inet = 0; inet < num_nets; inet++) {
        if (icon > (num_cons - 1))break;
        if (clb_net[inet].is_global == FALSE) { /* Skip global nets. */
            clb_net[inet].con = icon;
            for (isink = 0; isink < clb_net[inet].num_sinks; isink++) {
                if (icon > (num_cons - 1))break;
                cons[icon].net = inet;
                cons[icon].source = net_rr_terminals[inet][0];
                cons[icon].source_block = clb_net[inet].node_block[0];
                cons[icon].sink_block = clb_net[inet].node_block[isink + 1];
                cons[icon].target_node = net_rr_terminals[inet][isink + 1];
                icon++;

            }
        }
    }
//    /*4. Calculate bounding boxes of connections*/
//    int maxBB = 0;
//    int* bbs = (int*) my_calloc(num_cons, sizeof (int));
//    int* con_index = (int*) my_calloc(num_cons, sizeof (int));
//    for (icon = 0; icon < num_cons; icon++) {
//        con_index[icon]=0;
//        int xdelta = abs(block[cons[icon].source_block].x - block[cons[icon].sink_block].x);
//        int ydelta = abs(block[cons[icon].source_block].y - block[cons[icon].sink_block].y);
//        int bb = xdelta + ydelta;
//        bbs[icon] = bb;
//        if(bb > maxBB) maxBB = bb;
//    }
//    
//    /*5. Calculate boundaries BB*/
////        route_bb_con[icon].xmin = max(xmin - bb_factor, 0);
////        route_bb_con[icon].xmax = min(xmax + bb_factor, nx + 1);
////        route_bb_con[icon].ymin = max(ymin - bb_factor, 0);
////        route_bb_con[icon].ymax = min(ymax + bb_factor, ny + 1);
//    
//    float* sorting_values = (float*) my_calloc(num_cons, sizeof (float));
//    for (i = 0; i < num_cons; i++) {
//        sorting_values[i]= (maxBB-bbs[i])+clb_net[cons[i].net].num_sinks*1./(maxFO+1);
//    }
//    heapsort(con_index, sorting_values, num_cons, 1);
    
    //allocate_data_structures();
    /*6. Allocate hashmaps per net*/
    node_hash_maps = (t_node_hash_map *) my_calloc(num_nets, sizeof(t_node_hash_map));

    /* Usually the first iteration uses a very small (or 0) pres_fac to find  *
     * the shortest path and get a congestion map.  For fast compiles, I set  *
     * pres_fac high even for the first iteration.                            */

    pres_fac = router_opts.first_iter_pres_fac;

    trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    
        
    
    //alloc_timing_driven_route_structs(&pin_criticality, &sink_order,&rt_node_of_sink);
    printf("Start routing iterations.\n");
    no_nodes_expanded=0;
    int no_route_con_runs=0;
    for (itry = 1; itry <= router_opts.max_router_iterations; itry++) {
        printf("Routing iteration: %d ...\n", itry);
        
        
        for (i = 0; i < num_cons; i++) {
            icon = i;
//            icon = con_index[i];
            inet = cons[icon].net;
            t_node_hash_map* node_hash_map = &node_hash_maps[inet];
            
           //printf("rip_up_con_fast  %d ...\n", icon);
            //c1 = clock();
            rip_up_con_fast(icon, node_hash_map, pres_fac); 
            
            //c2 = clock();
            
            //printf("ds_route_con_fast  %d ...\n", icon);
            //no_route_con_runs++;
            is_routable =
                    route_con(icon, node_hash_map,
                    router_opts.bend_cost, pres_fac, router_opts.astar_fac);
            //c3 = clock();

  
            /* Impossible to route? (disconnected rr_graph) */
            if (!is_routable) {
                printf("Routing failed.\n");
                return (FALSE);
            }
            
            //printf("add_con_fast  %d ...\n", icon);
            add_con_fast(icon, node_hash_map, pres_fac);
            //c4 = clock();
            
            //t_ripup += ((double) (c2 - c1)/CLOCKS_PER_SEC);
            //t_route += ((double) (c3 - c2)/CLOCKS_PER_SEC);
            //t_add += ((double) (c4 - c3)/CLOCKS_PER_SEC);
        }
        /* Make sure any CLB OPINs used up by subblocks being hooked directly     *
         * to them are reserved for that purpose.                                 */

        if (itry == 1)
            rip_up_local_opins = FALSE;
        else
            rip_up_local_opins = TRUE;

        reserve_locally_used_opins(pres_fac, rip_up_local_opins,
                clb_opins_used_locally);
        
         /* Pathfinder guys quit after finding a feasible route. I may want to keep *
         * going longer, trying to improve timing.  Think about this some.         */
        
        /*Check if the routing is feasible*/
        printf("feasible routing\n");
        success = feasible_routing();
        if (success) {
            printf("Successfully routed after %d routing iterations.\n",itry);
            //printf("t_ripup %f\n",t_ripup);
            //printf("t_route %f\n",t_route);
            //printf("t_add %f\n",t_add);
            //printf("Number of nodes expanded: %lu\n",no_nodes_expanded);
            //printf("Number of route_con runs: %d\n",no_route_con_runs);
            //printf("Number of nodes expanded per route_net on average: %f", (no_nodes_expanded*1.0/(itry+1)/num_nets));
 
            
            c2 = clock();
            con_to_net();
//            free(con_index);
            free(cons);
            c3 = clock();
//            double overhead  = (float)(c1 - c0)/CLOCKS_PER_SEC +(float)(c3 - c2)/CLOCKS_PER_SEC;
//            printf("Overhead due adaptation to vpr standards: %f sec (CPU time)\n", overhead);
//            printf("Add and rip-up functions: %f sec (CPU time)\n", secs/CLOCKS_PER_SEC);
            return (TRUE);
        } 

        if (itry == 1) {
             /* Early exit code for cases where it is obvious that a successful route will not be found 
               Heuristic: If total wirelength used in first routing iteration is X% of total available wirelength, exit
             */
//            total_wirelength = 0;
//            available_wirelength = 0;
//
//            for (i = 0; i < num_rr_nodes; i++) {
//                if (rr_node[i].type == CHANX || rr_node[i].type == CHANY) {
//                    available_wirelength += 1 + rr_node[i].xhigh - rr_node[i].xlow +
//                            rr_node[i].yhigh - rr_node[i].ylow;
//                }
//            }
//            con_to_net();
//            for (inet = 0; inet < num_nets; inet++) {
//                if (clb_net[inet].is_global == FALSE && clb_net[inet].num_sinks != 0) { /* Globals don't count. */
//                    get_num_bends_and_length(inet, &bends, &wirelength, &segments);
//                    total_wirelength += wirelength;
//                }
//            }
//            printf("wirelength after first iteration %d, total available wirelength %d, ratio %g\n", total_wirelength, available_wirelength, (float) (total_wirelength) / (float) (available_wirelength));
//            if ((float) (total_wirelength) / (float) (available_wirelength) > FIRST_ITER_WIRELENTH_LIMIT) {
//                printf("Wirelength usage ratio exceeds limit of %g, fail routing\n", FIRST_ITER_WIRELENTH_LIMIT);
//                free(con_index);
//                free(cons);
//                return FALSE;
//                }
            
            pres_fac = router_opts.initial_pres_fac;
            pathfinder_update_cost(pres_fac, 0.); /* Acc_fac=0 for first iter. */
        } else {
            pres_fac *= router_opts.pres_fac_mult;
            /* Avoid overflow for high iteration counts, even if acc_cost is big */
            pres_fac = std::min(pres_fac, static_cast<float>(HUGE_POSITIVE_FLOAT / 1e5));
            pathfinder_update_cost(pres_fac, router_opts.acc_fac);
        }
        //fflush(stdout);
    }

    printf("Routing failed.\n");
//    free(con_index);
    free(cons);
    return (FALSE);
}


static void con_to_net() {
    int icon, inet;
    for (inet = 0; inet < num_nets; inet++){
        trace_head[inet] = NULL;
    }
    for (icon = 0; icon < num_cons; icon++) {
        int netnr = cons[icon].net;
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

static boolean
route_con(int icon, t_node_hash_map* node_hash_map, float bend_cost, float pres_fac, float astar_fac) {

    /* Uses a maze routing (Dijkstra's) algorithm to route a connection.  The net       *
     * begins at the net output, and expands outward until it hits a target      *
     * pin.  The algorithm is then restarted with the entire first wire segment  *
     * included as part of the source this time.  For an n-pin net, the maze     *
     * router is invoked n-1 times to complete all the connections.  Inet is     *
     * the index of the net to be routed.  Bends are penalized by bend_cost      *
     * (which is typically zero for detailed routing and nonzero only for global *
     * routing), since global routes with lots of bends are tougher to detailed  *
     * route (using a detailed router like SEGA).                                *
     * If this routine finds that a net *cannot* be connected (due to a complete *
     * lack of potential paths, rather than congestion), it returns FALSE, as    *
     * routing is impossible on this architecture.  Otherwise it returns TRUE.   */

    int i, inode, net, target_node, fanout;
    float old_total_path_cost, new_total_path_cost, old_back_path_cost, new_back_path_cost;
    struct s_heap *current;
    struct s_trace *tptr;

    net = cons[icon].net;
    target_node = cons[icon].target_node;
    fanout = clb_net[net].num_sinks;
    free_traceback_con(icon);
    
    /*add source to heap*/
    node_to_heap(cons[icon].source, 0.0, NO_PREVIOUS, NO_PREVIOUS, 0.0, OPEN);
    
    tptr = NULL;
    current = get_heap_head();
    if (current == NULL) { /* Infeasible routing.  No possible path for net. */
        reset_path_costs(); /* Clean up before leaving. */
        return (FALSE);
    }

    inode = current->index;
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
        
        if ((old_total_path_cost-new_total_path_cost)>0.001  && (old_back_path_cost-new_back_path_cost)>0.001) { /* New path is lowest cost. */
            rr_node_route_inf[inode].prev_node = current->u.prev_node;;
            rr_node_route_inf[inode].prev_edge = current->prev_edge;
            rr_node_route_inf[inode].path_cost = new_total_path_cost;
            rr_node_route_inf[inode].backward_path_cost = new_back_path_cost;


            if (old_total_path_cost > 0.99 * HUGE_POSITIVE_FLOAT) /* First time touched. */
                add_to_mod_list(&rr_node_route_inf[inode].path_cost);
            
            //no_nodes_expanded++;
            expand_neighbours(inode, net, target_node, fanout, 
                    new_back_path_cost, bend_cost, pres_fac, astar_fac);
        }
        free_heap_data(current);
        current = get_heap_head();
        
        if (current == NULL) { /* Impossible routing. No path for net. */
            reset_path_costs();
            return (FALSE);
        }

        inode = current->index;
    }
    

    update_traceback_con(current, icon);
    free_heap_data(current);

    empty_heap();
    reset_path_costs();
    return (TRUE);
}

static float
get_expected_cost(int inode,
        int target_node) {

    /* Determines the expected cost (due to both delay and resouce cost) to reach *
     * the target node from inode.  It doesn't include the cost of inode --       *
     * that's already in the "known" path_cost.                                   */

    t_rr_type rr_type;
    int cost_index, ortho_cost_index, num_segs_same_dir, num_segs_ortho_dir;
    float expected_cost, cong_cost, Tdel;
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

        expected_cost = cong_cost;
        return (expected_cost);
    }
    else if (rr_type == IPIN) { /* Change if you're allowing route-throughs */
        return (rr_indexed_data[SINK_COST_INDEX].base_cost);
    }
    else { /* Change this if you want to investigate route-throughs */
        return (0.);
    }
}

static void
expand_neighbours(int inode,
        int net,
        int target_node,
        int fanout,
        float old_back_cost,
        float bend_cost,
        float pres_fac,
        float astar_fac) {

    /* Puts all the rr_nodes adjacent to inode on the heap.  rr_nodes outside   *
     * the expanded bounding box specified in route_bb are not added to the     *
     * heap.  pcost is the path_cost to get to inode.                           */

    int edge, to_node, num_edges, target_x, target_y;;
    t_rr_type from_type, to_type;
    float new_tot_path_cost, new_back_cost;
    
    target_x = rr_node[target_node].xhigh;
    target_y = rr_node[target_node].yhigh;

    num_edges = rr_node[inode].num_edges;
    for (edge = 0; edge < num_edges; edge++) {
        to_node = rr_node[inode].edges[edge];

//        // node is out of the BB + 3tracks, 
//        if (rr_node[to_node].xhigh < route_bb[net].xmin ||
//                rr_node[to_node].xlow > route_bb[net].xmax ||
//                rr_node[to_node].yhigh < route_bb[net].ymin ||
//                rr_node[to_node].ylow > route_bb[net].ymax)
//            continue; /* Node is outside (expanded) bounding box. */

        /* Prune away IPINs that lead to blocks other than the target one.  Avoids  *
         * the issue of how to cost them properly so they don't get expanded before *
         * more promising routes, but makes route-throughs (via CLBs) impossible.   *
         * Change this if you want to investigate route-throughs.                   */
        to_type = rr_node[to_node].type;
        if (to_type == IPIN && (rr_node[to_node].xhigh != target_x ||
                rr_node[to_node].yhigh != target_y))
            continue;
        
        new_back_cost = old_back_cost + get_rr_cong_cost(to_node);
      
        if (bend_cost != 0.) {
            from_type = rr_node[inode].type;
            to_type = rr_node[to_node].type;
            if ((from_type == CHANX && to_type == CHANY) ||
                    (from_type == CHANY && to_type == CHANX))
                new_back_cost += bend_cost;
        }
        
        new_tot_path_cost = new_back_cost + astar_fac * get_expected_cost(to_node, target_node);
        
        node_to_heap(to_node, new_tot_path_cost, inode, edge, new_back_cost, OPEN);

    } /* End for all neighbours */
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

//static void allocate_data_structures(){
//   con_heap = (struct s_heap **) my_malloc(heap_size * sizeof (struct s_heap *));
//   con_heap--; /* heap stores from [1..heap_size] */
//   heap_tail = 1;
//}
