#include <stdio.h>
#include <math.h>
#include <time.h>
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

//static int get_max_pins_per_net(void);

//static void add_route_tree_to_heap(t_rt_node * rt_node,
//        int target_node,
//        float target_criticality,
//        float astar_fac);

static int get_expected_segs_to_target(int inode,
        int target_node,
        int *num_segs_ortho_dir_ptr);

//static int mark_node_expansion_by_bin(int inet, 
//        int target_node, 
//        t_rt_node * rt_node);

//static void con_to_net_old();

static void con_to_net();

static bool ds_route_con(int icon,
        float bend_cost, 
        float pres_fac,
        float astar_fac);

static bool ds_route_con_fast(int icon,
        float bend_cost, 
        float pres_fac,
        float astar_fac);

//static void conr_ds_expand_neighbours(int inode,
//        int icon,
//        int net,
//        int target_node,
//        int fanout,
//        float old_back_cost,
//        float bend_cost,
//        float pres_fac,
//        float astar_fac);

static void conr_ds_fast_expand_neighbours(int inode,
        int target_node,
        float old_back_cost,
        float bend_cost,
        float astar_fac);

//static float get_conr_ds_expected_cost(int inode,
//        int target_node,
//        int fanout);

static float get_conr_ds_fast_expected_cost(int inode,
        int target_node);

unsigned long no_nodes_expanded;

/************************ Subroutine definitions *****************************/

bool
try_directed_search_route_conr(struct s_router_opts router_opts,
        t_ivec ** clb_opins_used_locally) {

    int itry, inet, icon, i;
    bool success, is_routable, rip_up_local_opins;
//    t_rt_node **rt_node_of_sink; /* [1..max_pins_per_net-1]. */
    float pres_fac;

//    int bends;
//    int wirelength, total_wirelength, available_wirelength;
//    int segments;
    
    clock_t ca, cb;
//    double t_ripup = 0.0, t_route = 0.0, t_add = 0.0;
//    float secs = 0.0;
    
    /* Create Connection Array 
     * 1. Calculate the size of the Connection Array*/
    int maxFO = 0;
    num_cons = 0;
    for (inet = 0; inet < num_nets; inet++) {
        if (clb_net[inet].is_global == false) { /* Skip global nets. */
            num_cons += clb_net[inet].num_sinks;
            if(clb_net[inet].num_sinks > maxFO) maxFO = clb_net[inet].num_sinks;
        }
    }
    /*2. Allocate memory for Connection Array*/
    printf("Allocate memory for Connection Array for %d cons\n", num_cons);
    cons = (t_con*) my_malloc(num_cons * sizeof (t_con));
    /*3. Allocate memory for net-to-con Array*/
    int* first_con_of_net = (int*) my_malloc(num_nets * sizeof (int));
    /*4. Assign values to connection structs*/
    printf("Assigning values to connection structs\n");
    icon = 0;
    int isink;
    for (inet = 0; inet < num_nets; inet++) {
        if (icon > (num_cons - 1))break;
        first_con_of_net[inet] = icon;
        if (clb_net[inet].is_global == false) { /* Skip global nets. */
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
    
    /*6. Allocate hashmaps per net*/
    node_hash_maps = (t_node_hash_map*) my_calloc(num_nets, sizeof(t_node_hash_map));

    /* Usually the first iteration uses a very small (or 0) pres_fac to find  *
     * the shortest path and get a congestion map.  For fast compiles, I set  *
     * pres_fac high even for the first iteration.                            */

    pres_fac = router_opts.first_iter_pres_fac;

    trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    
        
    
    //alloc_timing_driven_route_structs(&pin_criticality, &sink_order,&rt_node_of_sink);
    printf("Start routing iterations.\n");
    printf("router_opts.astar_fac %f\n",router_opts.astar_fac);
    no_nodes_expanded=0;
//    int no_route_con_runs=0;
    for (itry = 1; itry <= router_opts.max_router_iterations; itry++) {
        printf("Routing iteration: %d ...", itry);
        ca = clock();
        
        for (i = 0; i < num_cons; i++) {
            icon = i;
//            icon = con_index[i];
            t_node_hash_map* node_hash_map = &node_hash_maps[cons[icon].net];
            
//           printf("rip_up_con_fast  %d ...\n", icon);
            //c1 = clock();
            rip_up_con_fast(icon, node_hash_map, pres_fac); 
            
            //c2 = clock();
            
//            printf("ds_route_con  %d ...\n", icon);
            //no_route_con_runs++;
            is_routable =
                    ds_route_con(icon,
                    router_opts.bend_cost, pres_fac, router_opts.astar_fac);
            //c3 = clock();

  
            /* Impossible to route? (disconnected rr_graph) */
            if (!is_routable) {
                printf("Routing failed.\n");
                return (false);
            }
            
//            printf("add_con_fast  %d ...\n", icon);
            add_con_fast(icon, node_hash_map, pres_fac);
            //c4 = clock();
            
            //t_ripup += ((double) (c2 - c1)/CLOCKS_PER_SEC);
            //t_route += ((double) (c3 - c2)/CLOCKS_PER_SEC);
            //t_add += ((double) (c4 - c3)/CLOCKS_PER_SEC);
        }
        /* Make sure any CLB OPINs used up by subblocks being hooked directly     *
         * to them are reserved for that purpose.                                 */

        if (itry == 1)
            rip_up_local_opins = false;
        else
            rip_up_local_opins = true;

        reserve_locally_used_opins(pres_fac, router_opts.acc_fac, rip_up_local_opins,
                clb_opins_used_locally);
        
         /* Pathfinder guys quit after finding a feasible route. I may want to keep *
         * going longer, trying to improve timing.  Think about this some.         */
        
        /*Check if the routing is feasible*/
        success = feasible_routing_debug();
        if (success) {
            cb = clock();
            printf(" %f\n",((double) (cb - ca)/CLOCKS_PER_SEC));
            printf("Successfully routed after %d routing iterations.\n",itry);
            //printf("t_ripup %f\n",t_ripup);
            //printf("t_route %f\n",t_route);
            //printf("t_add %f\n",t_add);
            //printf("Number of nodes expanded: %lu\n",no_nodes_expanded);
            //printf("Number of route_con runs: %d\n",no_route_con_runs);
            //printf("Number of nodes expanded per route_net on average: %f", (no_nodes_expanded*1.0/(itry+1)/num_nets));
 
            
            con_to_net();
//            free(con_index);
            free(cons);
//            double overhead  = (float)(c1 - c0)/CLOCKS_PER_SEC +(float)(c3 - c2)/CLOCKS_PER_SEC;
//            printf("Overhead due adaptation to vpr standards: %f sec (CPU time)\n", overhead);
//            printf("Add and rip-up functions: %f sec (CPU time)\n", secs/CLOCKS_PER_SEC);
            return (true);
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
//                if (clb_net[inet].is_global == false && clb_net[inet].num_sinks != 0) { /* Globals don't count. */
//                    get_num_bends_and_length(inet, &bends, &wirelength, &segments);
//                    total_wirelength += wirelength;
//                }
//            }
//            printf("wirelength after first iteration %d, total available wirelength %d, ratio %g\n", total_wirelength, available_wirelength, (float) (total_wirelength) / (float) (available_wirelength));
//            if ((float) (total_wirelength) / (float) (available_wirelength) > FIRST_ITER_WIRELENTH_LIMIT) {
//                printf("Wirelength usage ratio exceeds limit of %g, fail routing\n", FIRST_ITER_WIRELENTH_LIMIT);
//                free(con_index);
//                free(cons);
//                return false;
//                }
            
            pres_fac = router_opts.initial_pres_fac;
            pathfinder_update_cost(pres_fac, 0.); /* Acc_fac=0 for first iter. */
        } else {
            pres_fac *= router_opts.pres_fac_mult;
            /* Avoid overflow for high iteration counts, even if acc_cost is big */
	    pres_fac = std::min(pres_fac, static_cast<float>(HUGE_POSITIVE_FLOAT / 1e5));
            pathfinder_update_cost(pres_fac, router_opts.acc_fac);
        }
        cb = clock();
        printf(" %f\n",((double) (cb - ca)/CLOCKS_PER_SEC));
        //fflush(stdout);
    }

    printf("Routing failed.\n");
//    free(con_index);
    free(cons);
    return (false);
}

bool
try_conr_ds_route_fast(struct s_router_opts router_opts,
        t_ivec ** clb_opins_used_locally) {

    int itry, inet, icon, i;
    bool success, is_routable, rip_up_local_opins;
//    t_rt_node **rt_node_of_sink; /* [1..max_pins_per_net-1]. */
    float pres_fac;

//    int bends;
//    int wirelength, total_wirelength, available_wirelength;
//    int segments;
    
//    clock_t c0, c2, c3, c4;
//    double t_ripup = 0.0, t_route = 0.0, t_add = 0.0;
//    float secs = 0.0;
    
//    c0 = clock();
    /* Create Connection Array 
     * 1. Calculate the size of the Connection Array*/
    int maxFO = 0;
    num_cons = 0;
    for (inet = 0; inet < num_nets; inet++) {
        if (clb_net[inet].is_global == false) { /* Skip global nets. */
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
        if (clb_net[inet].is_global == false) { /* Skip global nets. */
            clb_net[inet].con = icon;
            for (isink = 0; isink < clb_net[inet].num_sinks; isink++) {
                if (icon > (num_cons - 1))break;
                cons[icon].net = inet;
                cons[icon].source = net_rr_terminals[inet][0];
                cons[icon].source_block = clb_net[inet].node_block[0];
                cons[icon].sink_block = clb_net[inet].node_block[isink + 1];
                cons[icon].target_node = net_rr_terminals[inet][isink + 1];
                cons[icon].previous_total_path_cost = -1.0;
                icon++;
            }
        }
    }
    
    int** cons_in_net = (int**) my_malloc(num_nets*sizeof(int*));
    for (inet = 0; inet < num_nets; inet++) {
        cons_in_net[inet] = (int*) my_malloc(clb_net[inet].num_sinks*sizeof(int));
        for(i=0;i<clb_net[inet].num_sinks;i++){
            cons_in_net[inet][i]=-1;
        }
    }
    for(icon = 0; icon < num_cons; icon++){
        inet = cons[icon].net;
        bool found_empty_slot = false;
        for(i=0;i<clb_net[inet].num_sinks;i++){
            if(cons_in_net[inet][i]==-1){
                found_empty_slot = true;
                break;
            }   
        }
        if(!found_empty_slot){
            printf("Error in try_conr_ds_route_fast: could not find an empty slot while constructing cons_in_net array.\n");
            exit(2);
        }else{
            cons_in_net[inet][i] = icon;
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
    
    /*6. Allocate hashmaps per net*/
    printf("Allocating node hash maps for each net\n");
    node_hash_maps = (t_node_hash_map*) my_calloc(num_nets, sizeof(t_node_hash_map));

    /* Usually the first iteration uses a very small (or 0) pres_fac to find  *
     * the shortest path and get a congestion map.  For fast compiles, I set  *
     * pres_fac high even for the first iteration.                            */

    pres_fac = router_opts.first_iter_pres_fac;

    trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    
        
//    congested_cons = (bool*) my_malloc(no_cons * sizeof (bool));
//    
//    for(icon=0; icon<no_cons;icon++){
//        congested_cons[icon]=true;
//    }
//    
    congested_nets = (bool*) my_malloc(num_nets * sizeof (bool));
    
    for(inet=0; inet<num_nets;inet++){
        congested_nets[inet]=true;
    }
    
    no_its_not_congested_con= (int*) my_calloc(num_cons, sizeof(int));    
    no_its_not_congested_net = (int*) my_calloc(num_nets,  sizeof(int));    
    no_its_pathcost_unchanged = (int*) my_calloc(num_cons, sizeof(int));
    
//    int* no_its_identical_trace = (int*) my_calloc(num_cons, sizeof(int));
    
//    int total_correct_predicted_changes = 0;
//    int total_failed_to_predict_changes = 0;
//    int total_correct_predicted_id_traces = 0;
//    int total_failed_to_predict_id_traces = 0;
    
//    cons_with_node_in_bb = (int**) my_malloc( num_rr_nodes * sizeof (int*));
//    int totalIntegersAllocated = 0;
//    
//    printf("Allocating connectionBB - node data structure\n");
//    for(inode=0; inode<num_rr_nodes; inode++){
//        int num_cons_containing_node = 0;
//        int* cons_containing_node;
//        for(icon=0; icon<num_cons; icon++){
//            int xmax = block[cons[icon].source_block].x;
//            int ymax = block[cons[icon].source_block].y;
//            int xmin = xmax;
//            int ymin = ymax;
//            int xsink = block[cons[icon].sink_block].x;
//            int ysink = block[cons[icon].sink_block].y;
//            if(xsink<xmin) xmin = xsink;
//            if(xsink>xmax) xmax = xsink;
//            if(ysink<ymin) ymin = ysink;
//            if(ysink>ymax) ymax = ysink;
//            bool in_bb = rr_node[inode].xlow>xmin && rr_node[inode].xhigh<xmax && rr_node[inode].ylow>ymin && rr_node[inode].yhigh<ymax;
//            if(in_bb){
//                num_cons_containing_node++;
//                cons_containing_node = (int*) realloc (cons_containing_node,num_cons_containing_node*sizeof(int));
//                cons_containing_node[num_cons_containing_node-1] = icon;
//                totalIntegersAllocated++;
//            }
//            
//        }
//        cons_with_node_in_bb[icon] = cons_containing_node;
//    }
//    printf("number of integers allocated: \n",totalIntegersAllocated);
    
//    bool bb_contains_congested_node = false;
    
//    printf("Start building parent data structure...\n");
//    t_parents* parents = (t_parents*) calloc(num_rr_nodes,sizeof(t_parents));
//    for(inode=0;inode<num_rr_nodes;inode++){
//        int num_edges = rr_node[inode].num_edges;
//        for (edge = 0; edge < num_edges; edge++) {
//                int child = rr_node[inode].edges[edge];
//                parents[child].num_parents++;
//                int* reallocated_parents = (int*) realloc (parents[child].parents, parents[child].num_parents * sizeof(int));
//                if(reallocated_parents==NULL){
//                    printf("Allocation failed while building parent data structure.\n");
//                    exit(2);
//                }
//                parents[child].parents = reallocated_parents;
//                parents[child].parents[parents[child].num_parents-1] = inode;
//        }
//    }
//    printf("finished building parent data structure...\n");
//    bool* parent_of_congested_node = calloc(num_rr_nodes,sizeof(bool));
    
    printf("Start routing iterations.\n");
    no_nodes_expanded=0;
//    int old_no_failed = 0;
//    int no_route_con_runs=0;
    for (itry = 1; itry <= router_opts.max_router_iterations; itry++) {
        printf("Routing iteration: %d ... ", itry);
//        printf("unpredicted changes per iteration: %d\n",total_failed_to_predict_changes-old_no_failed);
//        old_no_failed = total_failed_to_predict_changes;
        
        for (i = 0; i < num_cons; i++) {
            icon = i;
//            icon = con_index[i];
            inet = cons[icon].net;
            t_node_hash_map* node_hash_map = &node_hash_maps[inet];
            
            bool ripupandreroute = false;
            float path_cost = 0.0;
//            int total_shares = 0;
//            float neighborhood = 0.0;

            if(itry==1){//||no_its_not_congested_net[cons[icon].net]<1||no_its_identical_trace[icon]<1||no_its_pathcost_unchanged[icon]<1){
                ripupandreroute = true;
            }else{
                    //                    short edge;
                    //                    int neighbor;
                    //                    for (edge = 0; edge < rr_node[trace_head_con[icon]->index].num_edges; edge++) {                            
                    //                        neighbor = rr_node[trace_head_con[icon]->index].edges[edge];
                    //                        if(rr_node[neighbor].occ > rr_node[neighbor].capacity){
                    //                            ripupandreroute = true;
                    //                            break;
                    //                        }
                    //                        neighborhood += rr_node_route_inf[neighbor].acc_cost;
                    //                    }
                    struct s_trace* trace_it = trace_head_con[icon]->next; //Skip the source node !!!
                    for (;;) {
                        int node_index = trace_it->index;
                        short occ = rr_node[node_index].get_occ();
                        short cap = rr_node[node_index].get_capacity();
                        if (occ > cap) {
                            ripupandreroute = true;
                            break;
                        } 
//                        else {// occ <= cap 
//                            float pres_cost = 1.;
//                            short share = get_node_entry(node_hash_map, node_index)->usage;
//                            if(rr_node[node_index].type>2) total_shares += share;
////                            if(icon==100)printf("node %d, type %d\t(%d)\n",node_index,rr_node[node_index].type,share);
//                            short cost_index = rr_node[node_index].cost_index;
//                            path_cost += (rr_indexed_data[cost_index].base_cost
//                                    * rr_node_route_inf[node_index].acc_cost
//                                    * pres_cost / share);
//                        }
//                        short num_edges = rr_node[node_index].num_edges;
//                        short edge;
//                        for (edge = 0; edge < num_edges; edge++) {                            
//                            int neighbor = rr_node[node_index].edges[edge]; 
//                            int next_wire = -1;
//                            if (trace_it->next != NULL) {
//                                next_wire = trace_it->next->index;
//                            }
//                            if(next_wire!=neighbor){
//                                if(rr_node[neighbor].occ > rr_node[neighbor].capacity){
//                                        ripupandreroute = true;
//                                        break;
//                                }
//                                //neighborhood += rr_node_route_inf[neighbor].acc_cost;
//                                
////                                s_node_entry* node_entry_ptr = get_node_entry(node_hash_map,neighbor);
////                                short usage = 0;
////                                if(node_entry_ptr != NULL) usage = node_entry_ptr->usage;
////                                //printf("usage: %d\n", usage);
////                                neighborhood += rr_node[neighbor].occ*1.0/(1+usage);
//                            }
//                        }

                        if (trace_it->next == NULL) {
                            break;
                        } else {
                            trace_it = trace_it->next;
                        }
                    }
                    if (!ripupandreroute) {
                        ripupandreroute = ((path_cost - cons[icon].previous_total_path_cost) > 0.00001);
                        //printf("tot shares: %d, prev tot shares: %d\n",total_shares,cons[icon].previous_total_shares);
                        //ripupandreroute = (total_shares != cons[icon].previous_total_shares);
                        //ripupandreroute = (total_shares < cons[icon].previous_total_shares);
//                        if(!ripupandreroute){
//                                ripupandreroute = ((cons[icon].neighborhood - neighborhood) > 0.0001);   
//                                if(icon==2000)printf("previous neighborhood: %f - new neighborhood: %f\n",cons[icon].neighborhood, neighborhood);
//                        }
                        
                    }
//                    if (!ripupandreroute) {
    //                    int j;
    //                    for (j = 0; j < clb_net[inet].num_sinks; j++) {
    //                        int connection = cons_in_net[inet][j];
    //                        if (!ripupandreroute && connection != icon) {
    //                            trace_it = trace_head_con[connection]->next; //Skip the source node !!!
    //                            for (;;) {
    //                                int node_index = trace_it->index;
    //                                short occ = rr_node[node_index].occ;
    //                                short cap = rr_node[node_index].capacity;
    //                                if (occ > cap) {
    //                                    ripupandreroute = true;
    //                                    break;
    //                                }
    //                                if (trace_it->next == NULL) {
    //                                    break;
    //                                } else {
    //                                    trace_it = trace_it->next;
    //                                }
    //                            }
    //                        }
    //                    }
    //                }
            }
            
            //if(congested[icon]){
//            if(itry==1||no_its_not_congested_net[cons[icon].net]<1||no_its_identical_trace[icon]<1||no_its_pathcost_unchanged[icon]<1||trace_congested||path_cost_changed){
            //if(no_its_not_congested[icon]==0)printf("Connection %d congested.\n",icon);
                

                
//                int xmax = block[cons[icon].source_block].x;
//                int ymax = block[cons[icon].source_block].y;
//                int xmin = xmax;
//                int ymin = ymax;
//                int xsink = block[cons[icon].sink_block].x;
//                int ysink = block[cons[icon].sink_block].y;
//                if(xsink<xmin) xmin = xsink;
//                if(xsink>xmax) xmax = xsink;
//                if(ysink<ymin) ymin = ysink;
//                if(ysink>ymax) ymax = ysink;
//                
//                bb_contains_congested_node = false;
//                for(inode=0; inode<num_rr_nodes; inode++){
//                    if(rr_node[inode].occ>rr_node[inode].capacity){
//                        bool in_bb = rr_node[inode].xlow>xmin && rr_node[inode].xhigh<xmax && rr_node[inode].ylow>ymin && rr_node[inode].yhigh<ymax;
//                        if(in_bb){
//                            bb_contains_congested_node = true;
//                            break;
//                        }
//                    }
//                }

            if(ripupandreroute){
                
//                if(itry==2)printf("rip_up_con_fast  %d ...\n", icon);
                //c1 = clock();
                rip_up_con_fast(icon, node_hash_map, pres_fac); 

                //c2 = clock();
//                struct s_trace* old_tptr = trace_head_con[icon];
//                if(itry==2)printf("ds_route_con_fast  %d ...\n", icon);
//                no_route_con_runs++;
                is_routable = ds_route_con_fast(icon,
                        router_opts.bend_cost, 
                        pres_fac, 
                        router_opts.astar_fac);
                //c3 = clock();

                
                /* Impossible to route? (disconnected rr_graph) */
                if (!is_routable) {
                    printf("Routing failed.\n");
                    return (false);
                }
                
//                if(itry>1){
//                    //Checking if traces are identical
//                    bool identical = true;
//                    struct s_trace* old_it = old_tptr;
//                    struct s_trace* new_it = trace_head_con[icon];
//                    for(;;){
//                        if(old_it->index!=new_it->index){
//                            identical = false;
//                        }
//                        if(old_it->next == NULL){
//                            if(new_it->next != NULL) identical = false;
//                            break;
//                        }else{
//                            if(new_it->next == NULL){
//                                identical = false;
//                                break;
//                            }else{
//                                new_it = new_it->next;
//                                old_it = old_it->next;
//                            }
//                        }                    
//                    }
//                    if(identical){
//                        if(!ripupandreroute){
//                            total_correct_predicted_id_traces++;
//                        }else{
//                            total_failed_to_predict_id_traces++;
//                        }
//                        no_its_identical_trace[icon]++;
//                    }else{
//                       if(!ripupandreroute){
//                            total_failed_to_predict_changes++;
//                        }else{
//                            total_correct_predicted_changes++;
//                        }
//                        no_its_identical_trace[icon]=0;
//                    }
//                    
//                    old_it = old_tptr;
//                    struct s_trace* tempptr;
//                    while (old_it != NULL) {
//                        tempptr = old_it->next;
//                        free_trace_data(old_it);
//                        old_it = tempptr;
//                    } 
//                }

//                if(itry==2)printf("add_con_fast  %d ...\n", icon);
                add_con_fast(icon, node_hash_map, pres_fac);
//                if(itry==2)printf("Done adding con %d ...\n", icon);
                //c4 = clock();

                //t_ripup += ((double) (c2 - c1)/CLOCKS_PER_SEC);
                //t_route += ((double) (c3 - c2)/CLOCKS_PER_SEC);
                //t_add += ((double) (c4 - c3)/CLOCKS_PER_SEC);
            }
             
        }
        /* Make sure any CLB OPINs used up by subblocks being hooked directly     *
         * to them are reserved for that purpose.                                 */

        if (itry == 1)
            rip_up_local_opins = false;
        else
            rip_up_local_opins = true;

        reserve_locally_used_opins(pres_fac, router_opts.acc_fac, rip_up_local_opins,
                clb_opins_used_locally);
        
         /* Pathfinder guys quit after finding a feasible route. I may want to keep *
         * going longer, trying to improve timing.  Think about this some.         */
        
        /*Check if the routing is feasible*/
//        printf("feasible routing\n");
        success = feasible_routing_conr_rco();
//        success = feasible_routing_conr_rco(parents, parent_of_congested_node, itry);
        if (success) {
            printf("Successfully routed after %d routing iterations.\n",itry);
            
//            printf("Nr of predicted changes: %d\n", total_correct_predicted_changes);
//            printf("Nr of unpredicted changes: %d\n", total_failed_to_predict_changes);
//            printf("Nr of predicted identical traces: %d\n", total_correct_predicted_id_traces);
//            printf("Nr of unpredicted identical traces: %d\n", total_failed_to_predict_id_traces);
//            
//            //printf("t_ripup %f\n",t_ripup);
//            //printf("t_route %f\n",t_route);
//            //printf("t_add %f\n",t_add);
//            //printf("Number of nodes expanded: %lu\n",no_nodes_expanded);
//            printf("Number of route_con runs: %d\n",no_route_con_runs);
//            //printf("Number of nodes expanded per route_net on average: %f", (no_nodes_expanded*1.0/(itry+1)/num_nets));
// 
            
//            c2 = clock();
            con_to_net();
//            free(con_index);
            free(cons);
//            c3 = clock();
//            double overhead  = (float)(c1 - c0)/CLOCKS_PER_SEC +(float)(c3 - c2)/CLOCKS_PER_SEC;
//            printf("Overhead due adaptation to vpr standards: %f sec (CPU time)\n", overhead);
//            printf("Add and rip-up functions: %f sec (CPU time)\n", secs/CLOCKS_PER_SEC);
            return (true);
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
//                if (clb_net[inet].is_global == false && clb_net[inet].num_sinks != 0) { /* Globals don't count. */
//                    get_num_bends_and_length(inet, &bends, &wirelength, &segments);
//                    total_wirelength += wirelength;
//                }
//            }
//            printf("wirelength after first iteration %d, total available wirelength %d, ratio %g\n", total_wirelength, available_wirelength, (float) (total_wirelength) / (float) (available_wirelength));
//            if ((float) (total_wirelength) / (float) (available_wirelength) > FIRST_ITER_WIRELENTH_LIMIT) {
//                printf("Wirelength usage ratio exceeds limit of %g, fail routing\n", FIRST_ITER_WIRELENTH_LIMIT);
//                free(con_index);
//                free(cons);
//                return false;
//                }
            
            pres_fac = router_opts.initial_pres_fac;
            pathfinder_update_cost_acc(pres_fac, 0.); /* Acc_fac=0 for first iter. */
        } else {
            pres_fac *= router_opts.pres_fac_mult;
            /* Avoid overflow for high iteration counts, even if acc_cost is big */
            pres_fac = std::min(pres_fac, static_cast<float>(HUGE_POSITIVE_FLOAT / 1e5));
            pathfinder_update_cost_acc(pres_fac, router_opts.acc_fac);
        }
        //fflush(stdout);
    }

    printf("Routing failed.\n");
//    free(con_index);
    free(cons);
    return (false);
}

bool
try_conr_ds_route_history(struct s_router_opts router_opts,
        t_ivec ** clb_opins_used_locally) {

    int itry, inet, icon, i;
    bool success, is_routable, rip_up_local_opins;
    //t_rt_node **rt_node_of_sink; /* [1..max_pins_per_net-1]. */
    float pres_fac;

//    int bends;
//    int wirelength, total_wirelength, available_wirelength;
//    int segments;
    
//    clock_t c0, c2, c3;
//    double t_ripup = 0.0, t_route = 0.0, t_add = 0.0;
//    float secs = 0.0;
    
//    c0 = clock();
    /* Create Connection Array 
     * 1. Calculate the size of the Connection Array*/
    int maxFO = 0;
    num_cons = 0;
    for (inet = 0; inet < num_nets; inet++) {
        if (clb_net[inet].is_global == false) { /* Skip global nets. */
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
        if (clb_net[inet].is_global == false) { /* Skip global nets. */
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
    
    /*6. Allocate hashmaps per net*/
    node_hash_maps = (t_node_hash_map*) my_calloc(num_nets, sizeof(t_node_hash_map));

    /* Usually the first iteration uses a very small (or 0) pres_fac to find  *
     * the shortest path and get a congestion map.  For fast compiles, I set  *
     * pres_fac high even for the first iteration.                            */

    pres_fac = router_opts.first_iter_pres_fac;

    trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    
        
    
    //alloc_timing_driven_route_structs(&pin_criticality, &sink_order,&rt_node_of_sink);
    printf("Start routing iterations.\n");
    no_nodes_expanded=0;
//    int no_route_con_runs=0;
    printf("router_opts.astar_fac %f\n",router_opts.astar_fac);
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
                    ds_route_con(icon, router_opts.bend_cost, pres_fac, router_opts.astar_fac);
            //c3 = clock();

  
            /* Impossible to route? (disconnected rr_graph) */
            if (!is_routable) {
                printf("Routing failed.\n");
                return (false);
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
            rip_up_local_opins = false;
        else
            rip_up_local_opins = true;

        reserve_locally_used_opins(pres_fac, router_opts.acc_fac, rip_up_local_opins,
                clb_opins_used_locally);
        
         /* Pathfinder guys quit after finding a feasible route. I may want to keep *
         * going longer, trying to improve timing.  Think about this some.         */
        
        /*Check if the routing is feasible*/
        printf("feasible routing... ");
        success = feasible_routing();
        printf("OK\n");
        if (success) {
            printf("Successfully routed after %d routing iterations.\n",itry);
            //printf("t_ripup %f\n",t_ripup);
            //printf("t_route %f\n",t_route);
            //printf("t_add %f\n",t_add);
            //printf("Number of nodes expanded: %lu\n",no_nodes_expanded);
            //printf("Number of route_con runs: %d\n",no_route_con_runs);
            //printf("Number of nodes expanded per route_net on average: %f", (no_nodes_expanded*1.0/(itry+1)/num_nets));
 
            
//            c2 = clock();
            con_to_net();
//            free(con_index);
            free(cons);
//            c3 = clock();
//            double overhead  = (float)(c1 - c0)/CLOCKS_PER_SEC +(float)(c3 - c2)/CLOCKS_PER_SEC;
//            printf("Overhead due adaptation to vpr standards: %f sec (CPU time)\n", overhead);
//            printf("Add and rip-up functions: %f sec (CPU time)\n", secs/CLOCKS_PER_SEC);
            return (true);
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
//                if (clb_net[inet].is_global == false && clb_net[inet].num_sinks != 0) { /* Globals don't count. */
//                    get_num_bends_and_length(inet, &bends, &wirelength, &segments);
//                    total_wirelength += wirelength;
//                }
//            }
//            printf("wirelength after first iteration %d, total available wirelength %d, ratio %g\n", total_wirelength, available_wirelength, (float) (total_wirelength) / (float) (available_wirelength));
//            if ((float) (total_wirelength) / (float) (available_wirelength) > FIRST_ITER_WIRELENTH_LIMIT) {
//                printf("Wirelength usage ratio exceeds limit of %g, fail routing\n", FIRST_ITER_WIRELENTH_LIMIT);
//                free(con_index);
//                free(cons);
//                return false;
//                }
            
            pres_fac = router_opts.initial_pres_fac;
            pathfinder_update_cost_acc(pres_fac, 0.); /* Acc_fac=0 for first iter. */
        } else {
            pres_fac *= router_opts.pres_fac_mult;
            /* Avoid overflow for high iteration counts, even if acc_cost is big */
            pres_fac = std::min(pres_fac, static_cast<float>(HUGE_POSITIVE_FLOAT / 1e5));
            pathfinder_update_cost_acc(pres_fac, router_opts.acc_fac);
        }
        //fflush(stdout);
    }

    printf("Routing failed.\n");
//    free(con_index);
    free(cons);
    return (false);
}


//static void con_to_net_old() {
//    int icon, inet;
//    for (inet = 0; inet < num_nets; inet++){
//        trace_head[inet] = NULL;
//    }
//    for (icon = 0; icon < num_cons; icon++) {
//        int netnr = cons[icon].net;
//        if (trace_head[cons[icon].net] == NULL) {
//            trace_head[netnr] = trace_head_con[icon];
//            trace_tail[netnr] = trace_tail_con[icon];
//        } else {
//            struct s_trace* netit;
//            struct s_trace* conit = trace_head_con[icon];
//            struct s_trace* conprevit = NULL;
//            int iteration = 0;
//            int moves = 1;
//            while (moves > 0) {
//                netit = trace_head[netnr];
//                moves = 0;
//                for (; netit != NULL; netit = netit->next) {
//                    if (conit->index == netit->index) {
//                        conprevit = conit;
//                        conit = conit->next;
//                        moves++;
//                    }
//                }
//                iteration++;
//            }
//            trace_tail[netnr]->next = conprevit;
//            trace_tail[netnr] = trace_tail_con[icon];
//        }
//    }
    
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
//}

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

static bool
ds_route_con(int icon, float bend_cost, float pres_fac, float astar_fac) {

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
     * lack of potential paths, rather than congestion), it returns false, as    *
     * routing is impossible on this architecture.  Otherwise it returns true.   */

    int inode, target_node;
//    int net;
    float old_total_path_cost, new_total_path_cost, old_back_path_cost, new_back_path_cost;
    struct s_heap *current;
//    struct s_trace *tptr;

//    net = cons[icon].net;
    target_node = cons[icon].target_node;
//    fanout = clb_net[net].num_sinks;
    free_traceback_con(icon);
    
    /*add source to heap*/
    node_to_heap(cons[icon].source, 0.0, NO_PREVIOUS, NO_PREVIOUS, 0.0, OPEN);
    
//    tptr = NULL;
    current = get_heap_head();
    if (current == NULL) { /* Infeasible routing.  No possible path for net. */
        reset_path_costs(); /* Clean up before leaving. */
        return (false);
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
        
        if (old_total_path_cost>new_total_path_cost  && old_back_path_cost>new_back_path_cost) { /* New path is lowest cost. */
            rr_node_route_inf[inode].prev_node = current->u.prev_node;;
            rr_node_route_inf[inode].prev_edge = current->prev_edge;
            rr_node_route_inf[inode].path_cost = new_total_path_cost;
            rr_node_route_inf[inode].backward_path_cost = new_back_path_cost;


            if (old_total_path_cost > 0.99 * HUGE_POSITIVE_FLOAT) /* First time touched. */
                add_to_mod_list(&rr_node_route_inf[inode].path_cost);
            
            //no_nodes_expanded++;
            conr_ds_fast_expand_neighbours(inode, 
                    target_node,  
                    new_back_path_cost, 
                    bend_cost, 
                    astar_fac);
        }
        
        free_heap_data(current);
        
        current = get_heap_head();
        
        if (current == NULL) { /* Impossible routing. No path for net. */
            reset_path_costs();
            return (false);
        }

        inode = current->index;
    }
    

    update_traceback_con(current, icon);
    free_heap_data(current);

    empty_heap();
    reset_path_costs();
    return (true);
}

static bool
ds_route_con_fast(int icon, float bend_cost, float pres_fac, float astar_fac) {

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
     * lack of potential paths, rather than congestion), it returns false, as    *
     * routing is impossible on this architecture.  Otherwise it returns true.   */

    int inode, target_node;
//    int net;
//    int fanout;
//    int net;
    float old_total_path_cost, new_total_path_cost, old_back_path_cost, new_back_path_cost;
    struct s_heap *current;
//    struct s_trace *tptr;

//    net = cons[icon].net;
    target_node = cons[icon].target_node;
//    fanout = clb_net[net].num_sinks;
    //free_traceback_con(icon);
    
    /*add source to heap*/
    node_to_heap(cons[icon].source, 0.0, NO_PREVIOUS, NO_PREVIOUS, 0.0, OPEN);
    
//    tptr = NULL;
    current = get_heap_head();
    if (current == NULL) { /* Infeasible routing.  No possible path for net. */
        reset_path_costs(); /* Clean up before leaving. */
        return (false);
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
            conr_ds_fast_expand_neighbours(inode, 
                    target_node,
                    new_back_path_cost,
                    bend_cost,
                    astar_fac);
        }
        free_heap_data(current);
        current = get_heap_head();
        
        if (current == NULL) { /* Impossible routing. No path for net. */
            reset_path_costs();
            return (false);
        }

        inode = current->index;
    }    

    update_traceback_con_experimental(current, icon);
    free_heap_data(current);

    empty_heap();
    reset_path_costs();
    return (true);
}

static void
add_route_tree_to_heap(t_rt_node * rt_node,
        int target_node,
        float target_criticality,
        float astar_fac) {

    /* Puts the entire partial routing below and including rt_node onto the heap *
     * (except for those parts marked as not to be expanded) by calling itself   *
     * recursively.                                                              */

    int inode;
    t_rt_node *child_node;
    t_linked_rt_edge *linked_rt_edge;
    float tot_cost, backward_path_cost, R_upstream;

    /* Pre-order depth-first traversal */

    if (rt_node->re_expand) {
        inode = rt_node->inode;
        backward_path_cost = target_criticality * rt_node->Tdel;
        R_upstream = rt_node->R_upstream;
        tot_cost =
                backward_path_cost +
                astar_fac * get_conr_ds_fast_expected_cost(inode,
                target_node);
        node_to_heap(inode, tot_cost, NO_PREVIOUS, NO_PREVIOUS,
                backward_path_cost, R_upstream);
    }

    linked_rt_edge = rt_node->u.child_list;

    while (linked_rt_edge != NULL) {
        child_node = linked_rt_edge->child;
        add_route_tree_to_heap(child_node, target_node,
                target_criticality, astar_fac);
        linked_rt_edge = linked_rt_edge->next;
    }
}

static float
get_conr_ds_fast_expected_cost(int inode,
        int target_node) {

    /* Determines the expected cost (due to both delay and resouce cost) to reach *
     * the target node from inode.  It doesn't include the cost of inode --       *
     * that's already in the "known" path_cost.                                   */

    t_rr_type rr_type;
    int cost_index, ortho_cost_index, num_segs_same_dir, num_segs_ortho_dir;
    float expected_cost, cong_cost;
    int usage = rr_node_route_inf[inode].usage+1;

    rr_type = rr_node[inode].type;

    if (rr_type == CHANX || rr_type == CHANY) {
        num_segs_same_dir = get_expected_segs_to_target(inode, target_node,
                &num_segs_ortho_dir);
        cost_index = rr_node[inode].get_cost_index();
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

//static void
//conr_ds_expand_neighbours(int inode,
//        int icon,
//        int net,
//        int target_node,
//        int fanout,
//        float old_back_cost,
//        float bend_cost,
//        float pres_fac,
//        float astar_fac) {
//
//    /* Puts all the rr_nodes adjacent to inode on the heap.  rr_nodes outside   *
//     * the expanded bounding box specified in route_bb are not added to the     *
//     * heap.  pcost is the path_cost to get to inode.                           */
//
//    int edge, to_node, num_edges, target_x, target_y;;
//    t_rr_type from_type, to_type;
//    float new_tot_path_cost, new_back_cost;
//    
//    target_x = rr_node[target_node].get_xhigh();
//    target_y = rr_node[target_node].get_yhigh();
//
//    num_edges = rr_node[inode].get_num_edges();
//    for (edge = 0; edge < num_edges; edge++) {
//        to_node = rr_node[inode].edges[edge];
//
//        // node is out of the BB + 3tracks, 
////        if (rr_node[to_node].xhigh < route_bb[net].xmin ||
////                rr_node[to_node].xlow > route_bb[net].xmax ||
////                rr_node[to_node].yhigh < route_bb[net].ymin ||
////                rr_node[to_node].ylow > route_bb[net].ymax)
////            continue; /* Node is outside (expanded) bounding box. */
//
//        /* Prune away IPINs that lead to blocks other than the target one.  Avoids  *
//         * the issue of how to cost them properly so they don't get expanded before *
//         * more promising routes, but makes route-throughs (via CLBs) impossible.   *
//         * Change this if you want to investigate route-throughs.                   */
//        to_type = rr_node[to_node].type;
//        if (to_type == IPIN && (rr_node[to_node].get_xhigh() != target_x ||
//                rr_node[to_node].get_yhigh() != target_y))
//            continue;
//        
//        new_back_cost = old_back_cost + get_rr_cong_cost_con(to_node, icon, pres_fac);
//      
//        if (bend_cost != 0.) {
//            from_type = rr_node[inode].type;
//            to_type = rr_node[to_node].type;
//            if ((from_type == CHANX && to_type == CHANY) ||
//                    (from_type == CHANY && to_type == CHANX))
//                new_back_cost += bend_cost;
//        }
//        
//        new_tot_path_cost = new_back_cost + astar_fac * get_conr_ds_expected_cost(to_node, target_node,fanout);
//        
//        node_to_heap(to_node, new_tot_path_cost, inode, edge, new_back_cost, OPEN);
//
//    } /* End for all neighbours */
//}

static void
conr_ds_fast_expand_neighbours(int inode,
        int target_node,
        float old_back_cost,
        float bend_cost,
        float astar_fac) {

    /* Puts all the rr_nodes adjacent to inode on the heap.  rr_nodes outside   *
     * the expanded bounding box specified in route_bb are not added to the     *
     * heap.  pcost is the path_cost to get to inode.                           */

    int edge, to_node, num_edges, target_x, target_y;;
    t_rr_type from_type, to_type;
    float new_tot_path_cost, new_back_cost;
    
    target_x = rr_node[target_node].get_xhigh();
    target_y = rr_node[target_node].get_yhigh();

    num_edges = rr_node[inode].get_num_edges();
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
        if (to_type == IPIN && (rr_node[to_node].get_xhigh() != target_x ||
                rr_node[to_node].get_yhigh() != target_y))
            continue;
        
        new_back_cost = old_back_cost + get_rr_cong_cost(to_node);
      
        if (bend_cost != 0.) {
            from_type = rr_node[inode].type;
            to_type = rr_node[to_node].type;
            if ((from_type == CHANX && to_type == CHANY) ||
                    (from_type == CHANY && to_type == CHANX))
                new_back_cost += bend_cost;
        }
        
        new_tot_path_cost = new_back_cost + astar_fac * get_conr_ds_fast_expected_cost(to_node, target_node);
        
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
        ylow = rr_node[inode].get_ylow();
        yhigh = rr_node[inode].get_yhigh();
        xlow = rr_node[inode].get_xlow();

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

///* Nets that have high fanout can take a very long time to route.  Each sink should be routed contained within a bin instead of the entire bounding box to speed things up */
//static int mark_node_expansion_by_bin(int inet, int target_node, t_rt_node * rt_node) {
//    int target_x, target_y;
//    int rlim = 1;
//    int inode;
//    float area;
//    bool success;
//    t_linked_rt_edge *linked_rt_edge;
//    t_rt_node * child_node;
//
//    target_x = rr_node[target_node].get_xlow();
//    target_y = rr_node[target_node].get_ylow();
//
//    if (clb_net[inet].num_sinks < HIGH_FANOUT_NET_LIM) {
//        /* This algorithm only applies to high fanout nets */
//        return 1;
//    }
//
//    area = (route_bb[inet].xmax - route_bb[inet].xmin) * (route_bb[inet].ymax - route_bb[inet].ymin);
//    if (area <= 0) {
//        area = 1;
//    }
//
//    rlim = ceil(sqrt((float) area / (float) clb_net[inet].num_sinks));
//    if (rt_node == NULL || rt_node->u.child_list == NULL) {
//        /* If unknown traceback, set radius of bin to be size of chip */
//        rlim = std::max(nx + 2, ny + 2);
//        return rlim;
//    }
//
//    success = false;
//    /* determine quickly a feasible bin radius to route sink for high fanout nets 
//       this is necessary to prevent super long runtimes for high fanout nets; in best case, a reduction in complexity from O(N^2logN) to O(NlogN) (Swartz fast router)
//     */
//    linked_rt_edge = rt_node->u.child_list;
//    while (success == false && linked_rt_edge != NULL) {
//        while (linked_rt_edge != NULL && success == false) {
//            child_node = linked_rt_edge->child;
//            inode = child_node->inode;
//            if (!(rr_node[inode].type == IPIN || rr_node[inode].type == SINK)) {
//                if (rr_node[inode].get_xlow() <= target_x + rlim &&
//                        rr_node[inode].get_xhigh() >= target_x - rlim &&
//                        rr_node[inode].get_ylow() <= target_y + rlim &&
//                        rr_node[inode].get_yhigh() >= target_y - rlim) {
//                    success = true;
//                }
//            }
//            linked_rt_edge = linked_rt_edge->next;
//        }
//
//        if (success == false) {
//            if (rlim > std::max(nx + 2, ny + 2)) {
//                printf(ERRTAG "VPR internal error, net %s has paths that are not found in traceback\n", clb_net[inet].name);
//                exit(1);
//            }
//            /* if sink not in bin, increase bin size until fit */
//            rlim *= 2;
//        } else {
//            /* Sometimes might just catch a wire in the end segment, need to give it some channel space to explore */
//            rlim += 4;
//        }
//        linked_rt_edge = rt_node->u.child_list;
//    }
//
//    /* redetermine expansion based on rlim */
//    linked_rt_edge = rt_node->u.child_list;
//    while (linked_rt_edge != NULL) {
//        child_node = linked_rt_edge->child;
//        inode = child_node->inode;
//        if (!(rr_node[inode].type == IPIN || rr_node[inode].type == SINK)) {
//            if (rr_node[inode].get_xlow() <= target_x + rlim &&
//                    rr_node[inode].get_xhigh() >= target_x - rlim &&
//                    rr_node[inode].get_ylow() <= target_y + rlim &&
//                    rr_node[inode].get_yhigh() >= target_y - rlim) {
//                child_node->re_expand = true;
//            } else {
//                child_node->re_expand = false;
//            }
//        }
//        linked_rt_edge = linked_rt_edge->next;
//    }
//    return rlim;
//}
