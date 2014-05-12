#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include "util.h"
#include "vpr_types.h"
#include "globals.h"
#include "heapsort.h"
//#include "mst.h"
#include "route_export.h"
#include "route_common.h"
#include "route_breadth_first.h"



/********************* Subroutines local to this module *********************/

static void breadth_first_expand_trace_segment(struct s_trace *start_ptr,
        int
        remaining_connections_to_sink);

static void breadth_first_expand_neighbours(int inode,
        float pcost,
        int inet,
        float bend_cost);

static void breadth_first_add_source_to_heap(int inet);
/* name:  ASCII connection name for informative annotations in the output.
 * source_block: The source block
 * sink_block_port: Port index (on a block) to which the sink terminal connects. 
 * sink_block_pin: Contains the index of the pin (on a block) to 
 *          which sink connects.  */

static boolean breadth_first_route_con(int icon, float bend_cost, float pres_fac, int itry);

static boolean breadth_first_route_con_fast(int icon, float bend_cost, float pres_fac, int itry);

static void
breadth_first_expand_neighbours_con(int inode,
        float pcost,
        int icon,
        int net,
        float bend_cost,
        float pres_fac);
static void
breadth_first_expand_neighbours_con_fast(int inode,
        float pcost,
        int icon,
        int net,
        float bend_cost,
        float pres_fac);

static void
breadth_first_add_source_to_heap_con(int icon);

/************************ Subroutine definitions ****************************/
boolean
try_breadth_first_route_conr(struct s_router_opts router_opts,
        t_ivec ** clb_opins_used_locally,
        int width_fac) {

    /* Iterated maze router ala Pathfinder Negotiated Congestion algorithm,  *
     * (FPGA 95 p. 111).  Returns TRUE if it can route this FPGA, FALSE if   *
     * it can't.                                                             */

    float pres_fac;
    boolean success, is_routable, rip_up_local_opins;
    int itry, inet, icon, i;
    clock_t c0, c1, c2, c3;
    
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
    cons = (s_con*) my_malloc(num_cons * sizeof (s_con));
    /*3. Assign values to connection structs*/
    printf("Assigning values to connection structs\n");
    icon = 0;
    int isink;
    for (inet = 0; inet < num_nets; inet++) {
        if (icon > (num_cons - 1))break;
        if (clb_net[inet].is_global == FALSE) { /* Skip global nets. */
            for (isink = 0; isink < clb_net[inet].num_sinks; isink++) {
                if (icon > (num_cons - 1))break;
                cons[icon].net = inet;
                /* Copy source information*/
                cons[icon].source = net_rr_terminals[inet][0];
                cons[icon].source_block = clb_net[inet].node_block[0];
                /* Copy sink information*/
                cons[icon].sink_block = clb_net[inet].node_block[isink + 1];

                /*Copy target node (sink node)*/
                cons[icon].target_node = net_rr_terminals[inet][isink + 1];
                /*increment connection iterator*/
                icon++;

            }
        }
    }
    /*4. Calculate bounding boxes of connections*/
    int maxBB = 0;
    int* bbs = (int*) my_calloc(num_cons, sizeof (int));
    int* con_index = (int*) my_calloc(num_cons, sizeof (int));
    for (icon = 0; icon < num_cons; icon++) {
        con_index[icon]=0;
        int xdelta = abs(block[cons[icon].source_block].x - block[cons[icon].sink_block].x);
        int ydelta = abs(block[cons[icon].source_block].y - block[cons[icon].sink_block].y);
        int bb = xdelta + ydelta;
        bbs[icon] = bb;
        if(bb > maxBB) maxBB = bb;
    }
    
    float* sorting_values = (float*) my_calloc(num_cons, sizeof (float));
    for (i = 0; i < num_cons; i++) {
        sorting_values[i]= (maxBB-bbs[i])+clb_net[cons[i].net].num_sinks*1./(maxFO+1);
    }
    heapsort(con_index, sorting_values, num_cons, 1);

    /* Usually the first iteration uses a very small (or 0) pres_fac to find  *
     * the shortest path and get a congestion map.  For fast compiles, I set  *
     * pres_fac high even for the first iteration.                            */

    pres_fac = router_opts.first_iter_pres_fac;

    trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    
    c1 = clock();
    printf("Start routing iterations.\n");
    for (itry = 1; itry <= router_opts.max_router_iterations; itry++) {
        printf("Routing iteration: %d ...\n", itry);

        for (i = 0; i < num_cons; i++) {
            icon = con_index[i];
            //printf("Rip up connection %d\n",icon);
            /*Rip up*/
            rip_up_con(icon, pres_fac);

            //printf("Route connection %d\n",icon);
            /*Reroute*/
            is_routable =
                    breadth_first_route_con(icon,
                    router_opts.
                    bend_cost, pres_fac, itry);

            /* Impossible to route? (disconnected rr_graph) */
            if (!is_routable) {
                printf("Routing failed.\n");
                return (FALSE);
            }

            //printf("Add connection %d\n",icon);
            /*Add */
            add_con(icon, pres_fac);
        }
        /* Make sure any CLB OPINs used up by subblocks being hooked directly     *
         * to them are reserved for that purpose.                                 */

        if (itry == 1)
            rip_up_local_opins = FALSE;
        else
            rip_up_local_opins = TRUE;

        reserve_locally_used_opins(pres_fac, rip_up_local_opins,
                clb_opins_used_locally);

        /*Check if the routing is feasible*/
        success = feasible_routing_debug();
        if (success) {
            printf("Successfully routed after %d routing iterations.\n",
                    itry);
            c2 = clock();
            for (icon = 0; icon < num_cons; icon++) {
                int netnr = cons[icon].net;
                if (trace_head[cons[icon].net] == NULL) {
                    trace_head[netnr] = trace_head_con[icon];
                    trace_tail[netnr] = trace_tail_con[icon];
                } else {
                    struct s_trace* netit;
                    struct s_trace* conit = trace_head_con[icon];
                    struct s_trace* conprevit = NULL;
                    int iteration =0;
                    int moves = 1;
                    while (moves > 0) {
                        netit = trace_head[netnr];
                        moves = 0;
                        for (; netit != NULL; netit = netit->next) {
                            if (conit->index == netit->index) {
                                conprevit = conit;
                                conit = conit->next;
                                moves++;
                            }
                        }
                        iteration++;
                    }

                    trace_tail[netnr]->next = conprevit;
                    trace_tail[netnr] = trace_tail_con[icon];
                }
            }
            free(cons);
            c3 = clock();
            printf("Overhead due adaptation to vpr standards: %f sec (CPU time)\n", (float) ((c1 - c0) + (c2 - c3))/CLOCKS_PER_SEC);
            return (TRUE);
        } 

        if (itry == 1) {
            pres_fac = router_opts.initial_pres_fac;
        } else {
            pres_fac *= router_opts.pres_fac_mult;
        }

        pres_fac = std::min(pres_fac, static_cast<float>(HUGE_POSITIVE_FLOAT / 1e5));

        pathfinder_update_cost(pres_fac, router_opts.acc_fac);
    }
    
    free(cons);
    printf("Routing failed.\n");
    return (FALSE);
}

boolean
try_breadth_first_route_conr_alloc(struct s_router_opts router_opts,
        t_ivec ** clb_opins_used_locally,
        int width_fac) {

    /* Iterated maze router ala Pathfinder Negotiated Congestion algorithm,  *
     * (FPGA 95 p. 111).  Returns TRUE if it can route this FPGA, FALSE if   *
     * it can't.                                                             */

    float pres_fac;
    boolean success, is_routable, rip_up_local_opins;
    int itry, inet, icon;
    clock_t c0, c1, c2, c3;
    
    c0 = clock();
    /* Create Connection Array 
     * 1. Calculate the size of the Connection Array*/
    num_cons = 0;
    for (inet = 0; inet < num_nets; inet++) {
        if (clb_net[inet].is_global == FALSE) { /* Skip global nets. */
            num_cons += clb_net[inet].num_sinks;
        }
    }
    /*2. Allocate memory for Connection Array*/
    printf("Allocate memory for Connection Array for %d cons\n", num_cons);
    //num_cons = 3000;
    cons = (s_con*) my_malloc(num_cons * sizeof (s_con));
    /*3. Assign values to connection structs*/
    printf("Assigning values to connection structs\n");
    icon = 0;
    int isink;
    for (inet = 0; inet < num_nets; inet++) {
        if (icon > (num_cons - 1))break;
        if (clb_net[inet].is_global == FALSE) { /* Skip global nets. */
            for (isink = 0; isink < clb_net[inet].num_sinks; isink++) {
                if (icon > (num_cons - 1))break;
                cons[icon].net = inet;
                /* Copy source information*/
                cons[icon].source = net_rr_terminals[inet][0];
                cons[icon].source_block = clb_net[inet].node_block[0];
                /* Copy sink information*/
                cons[icon].sink_block = clb_net[inet].node_block[isink + 1];

                /*Copy target node (sink node)*/
                cons[icon].target_node = net_rr_terminals[inet][isink + 1];
                /*increment connection iterator*/
                icon++;

            }
        }
    }
    /*4. Calculate bounding boxes of connections*/
    int* bbs = (int*) my_calloc(num_cons, sizeof (int));
    for (icon = 0; icon < num_cons; icon++) {
        int xdelta = abs(block[cons[icon].source_block].x - block[cons[icon].sink_block].x);
        int ydelta = abs(block[cons[icon].source_block].y - block[cons[icon].sink_block].y);
        bbs[icon] = xdelta + ydelta;
    }

    /*5. Sort Connections in ascending order of bounding box via QuickSort algorithm*/
    int i, j, temp;
    s_con con_temp;
    for (i = 0; i < num_cons; i++) {
        for (j = i + 1; j < num_cons; j++) {
            if (bbs[i] > bbs[j]) {
                temp = bbs[i];
                con_temp = cons[i];
                bbs[i] = bbs[j];
                cons[i] = cons[j];
                bbs[j] = temp;
                cons[j] = con_temp;
            } else if (bbs[i] == bbs[j]) {
                if (clb_net[cons[j].net].num_sinks > clb_net[cons[i].net].num_sinks) {
                    temp = bbs[i];
                    con_temp = cons[i];
                    bbs[i] = bbs[j];
                    cons[i] = cons[j];
                    bbs[j] = temp;
                    cons[j] = con_temp;
                }
            }
        }
    }
    free(bbs);

    /* Usually the first iteration uses a very small (or 0) pres_fac to find  *
     * the shortest path and get a congestion map.  For fast compiles, I set  *
     * pres_fac high even for the first iteration.                            */

    pres_fac = router_opts.first_iter_pres_fac;

    trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));

    c1 = clock();
    printf("Start routing iterations.\n");
    for (itry = 1; itry <= router_opts.max_router_iterations; itry++) {
        printf("Routing iteration: %d ...\n", itry);

        for (icon = 0; icon < num_cons; icon++) {
            //printf("Rip up connection %d\n",icon);
            /*Rip up*/
            rip_up_con(icon, pres_fac);

            //printf("Route connection %d\n",icon);
            /*Reroute*/
            is_routable =
                    breadth_first_route_con(icon,
                    router_opts.
                    bend_cost, pres_fac, itry);

            /* Impossible to route? (disconnected rr_graph) */
            if (!is_routable) {
                printf("Routing failed.\n");
                return (FALSE);
            }

            //printf("Add connection %d\n",icon);
            /*Add */
            add_con(icon, pres_fac);
        }
        /* Make sure any CLB OPINs used up by subblocks being hooked directly     *
         * to them are reserved for that purpose.                                 */

        if (itry == 1)
            rip_up_local_opins = FALSE;
        else
            rip_up_local_opins = TRUE;

        reserve_locally_used_opins(pres_fac, rip_up_local_opins,
                clb_opins_used_locally);

        /*Check if the routing is feasible*/
        success = feasible_routing_debug();
        if (success) {
            printf("Successfully routed after %d routing iterations.\n",
                    itry);
            c2 = clock();
            for (icon = 0; icon < num_cons; icon++) {
                int netnr = cons[icon].net;
                if (trace_head[cons[icon].net] == NULL) {
                    trace_head[netnr] = trace_head_con[icon];
                    trace_tail[netnr] = trace_tail_con[icon];
                } else {
                    struct s_trace* netit;
                    struct s_trace* conit = trace_head_con[icon];
                    struct s_trace* conprevit = NULL;
                    int iteration =0;
                    int moves = 1;
                    while (moves > 0) {
                        netit = trace_head[netnr];
                        moves = 0;
                        for (; netit != NULL; netit = netit->next) {
                            if (conit->index == netit->index) {
                                conprevit = conit;
                                conit = conit->next;
                                moves++;
                            }
                        }
                        iteration++;
                    }

                    trace_tail[netnr]->next = conprevit;
                    trace_tail[netnr] = trace_tail_con[icon];
                }
            }
            free(cons);
            c3 = clock();
            printf("Overhead due adaptation to vpr standards: %f sec (CPU time)\n", (float) ((c1 - c0) + (c2 - c3))/CLOCKS_PER_SEC);
            return (TRUE);
        } 

        if (itry == 1) {
            pres_fac = router_opts.initial_pres_fac;
        } else {
            pres_fac *= router_opts.pres_fac_mult;
        }

        pres_fac = fmin(pres_fac, HUGE_POSITIVE_FLOAT / 1e5);

        pathfinder_update_cost(pres_fac, router_opts.acc_fac);
    }
    
    free(cons);
    printf("Routing failed.\n");
    return (FALSE);
}

boolean
try_breadth_first_route_conr_fast(struct s_router_opts router_opts,
        t_ivec ** clb_opins_used_locally,
        int width_fac) {

    /* Iterated maze router ala Pathfinder Negotiated Congestion algorithm,  *
     * (FPGA 95 p. 111).  Returns TRUE if it can route this FPGA, FALSE if   *
     * it can't.                                                             */

    float pres_fac;
    boolean success, is_routable, rip_up_local_opins;
    int itry, inet, icon, i;
    clock_t c0, c1, c2, c3;
    
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
    cons = (s_con*) my_malloc(num_cons * sizeof (s_con));
    /*3. Assign values to connection structs*/
    printf("Assigning values to connection structs\n");
    icon = 0;
    int isink;
    for (inet = 0; inet < num_nets; inet++) {
        if (icon > (num_cons - 1))break;
        if (clb_net[inet].is_global == FALSE) { /* Skip global nets. */
            for (isink = 0; isink < clb_net[inet].num_sinks; isink++) {
                if (icon > (num_cons - 1))break;
                cons[icon].net = inet;
                /* Copy source information*/
                cons[icon].source = net_rr_terminals[inet][0];
                cons[icon].source_block = clb_net[inet].node_block[0];
                /* Copy sink information*/
                cons[icon].sink_block = clb_net[inet].node_block[isink + 1];

                /*Copy target node (sink node)*/
                cons[icon].target_node = net_rr_terminals[inet][isink + 1];
                /*increment connection iterator*/
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
//    float* sorting_values = (float*) my_calloc(num_cons, sizeof (float));
//    for (i = 0; i < num_cons; i++) {
//        sorting_values[i]= (maxBB-bbs[i])+clb_net[cons[i].net].num_sinks*1./(maxFO+1);
//    }
//    heapsort(con_index, sorting_values, num_cons, 1);
    
    /*6. Allocate hashmaps per net*/
    node_hash_maps = (s_node_hash_map*) my_calloc(num_nets, sizeof(s_node_hash_map));

    /* Usually the first iteration uses a very small (or 0) pres_fac to find  *
     * the shortest path and get a congestion map.  For fast compiles, I set  *
     * pres_fac high even for the first iteration.                            */

    pres_fac = router_opts.first_iter_pres_fac;

    trace_head_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    trace_tail_con = (struct s_trace **) my_calloc(num_cons, sizeof (struct s_trace *));
    
//    congested = (boolean*) my_malloc(num_cons * sizeof (boolean));
//    for(icon=0; icon<num_cons;icon++){
//        congested[icon]=TRUE;
//    }
    
    c1 = clock();
    printf("Start routing iterations.\n");
    for (itry = 1; itry <= router_opts.max_router_iterations; itry++) {
        printf("Routing iteration: %d ...\n", itry);

        for (i = 0; i < num_cons; i++) {
            icon = i;
//            icon = con_index[i];
            inet = cons[icon].net;
            s_node_hash_map* node_hash_map = &node_hash_maps[inet];
            
            //printf("Rip up connection %d\n",icon);
            /*Rip up*/
            rip_up_con_fast(icon, node_hash_map, pres_fac);

            //printf("Route connection %d\n",icon);
            /*Reroute*/
            is_routable =
                    breadth_first_route_con_fast(icon,
                    router_opts.
                    bend_cost, pres_fac, itry);

            /* Impossible to route? (disconnected rr_graph) */
            if (!is_routable) {
                printf("Routing failed.\n");
                return (FALSE);
            }

            //printf("Add connection %d\n",icon);
            /*Add */
            add_con_fast(icon, node_hash_map, pres_fac);
        }
        /* Make sure any CLB OPINs used up by subblocks being hooked directly     *
         * to them are reserved for that purpose.                                 */
        //printf("Ending routing iteration\n");
        if (itry == 1)
            rip_up_local_opins = FALSE;
        else
            rip_up_local_opins = TRUE;

        reserve_locally_used_opins(pres_fac, rip_up_local_opins,
                clb_opins_used_locally);

        /*Check if the routing is feasible*/
        printf("checkfeasible routing.\n");
        success = feasible_routing();
        printf("done checking feasible routing \n");
        if (success) {
            printf("Successfully routed after %d routing iterations.\n",
                    itry);
            c2 = clock();
            for (icon = 0; icon < num_cons; icon++) {
                int netnr = cons[icon].net;
                if (trace_head[cons[icon].net] == NULL) {
                    trace_head[netnr] = trace_head_con[icon];
                    trace_tail[netnr] = trace_tail_con[icon];
                } else {
                    struct s_trace* netit;
                    struct s_trace* conit = trace_head_con[icon];
                    struct s_trace* conprevit = NULL;
                    int iteration =0;
                    int moves = 1;
                    while (moves > 0) {
                        netit = trace_head[netnr];
                        moves = 0;
                        for (; netit != NULL; netit = netit->next) {
                            if (conit->index == netit->index) {
                                conprevit = conit;
                                conit = conit->next;
                                moves++;
                            }
                        }
                        iteration++;
                    }

                    trace_tail[netnr]->next = conprevit;
                    trace_tail[netnr] = trace_tail_con[icon];
                }
            }
            free(cons);
            c3 = clock();
            printf("Overhead due adaptation to vpr standards: %f sec (CPU time)\n", (float) ((c1 - c0) + (c2 - c3))/CLOCKS_PER_SEC);
            return (TRUE);
        } 

        if (itry == 1) {
            pres_fac = router_opts.initial_pres_fac;
        } else {
            pres_fac *= router_opts.pres_fac_mult;
        }

        pres_fac = fmin(pres_fac, HUGE_POSITIVE_FLOAT / 1e5);

        pathfinder_update_cost(pres_fac, router_opts.acc_fac);
    }
    
    free(cons);
    printf("Routing failed.\n");
    return (FALSE);
}

//boolean
//try_breadth_first_route_rco(struct s_router_opts router_opts,
//        t_ivec ** clb_opins_used_locally,
//        int width_fac) {
//
//    /* Iterated maze router ala Pathfinder Negotiated Congestion algorithm,  *
//     * (FPGA 95 p. 111).  Returns TRUE if it can route this FPGA, FALSE if   *
//     * it can't.                                                             */
//
//    float pres_fac;
//    boolean success, is_routable, rip_up_local_opins;
//    int itry, inet;
//
//    /* Usually the first iteration uses a very small (or 0) pres_fac to find  *
//     * the shortest path and get a congestion map.  For fast compiles, I set  *
//     * pres_fac high even for the first iteration.                            */
//
//    pres_fac = router_opts.first_iter_pres_fac;
//    
//    congested = (boolean*) my_malloc(num_nets * sizeof (boolean));
//    
//    for(inet=0; inet<num_nets;inet++){
//        congested[inet]=TRUE;
//    }
//
//    for (itry = 1; itry <= router_opts.max_router_iterations; itry++) {
//        for (inet = 0; inet < num_nets; inet++) {
//            if (!clb_net[inet].is_global) { /* Skip global nets and nets that are not congested*/
//
//                pathfinder_update_one_cost(trace_head[inet], -1,
//                        pres_fac);
//
//                if(congested[inet]){
//                    is_routable =
//                            breadth_first_route_net(inet,
//                            router_opts.
//                            bend_cost);
//                }
//                /* Impossible to route? (disconnected rr_graph) */
//
//                if (!is_routable) {
//                    printf("Routing failed.\n");
//                    return (FALSE);
//                }
//
//                pathfinder_update_one_cost(trace_head[inet], 1,
//                        pres_fac);
//
//            }
//        }
//
//
//        /* Make sure any CLB OPINs used up by subblocks being hooked directly     *
//         * to them are reserved for that purpose.                                 */
//
//        if (itry == 1)
//            rip_up_local_opins = FALSE;
//        else
//            rip_up_local_opins = TRUE;
//
//        reserve_locally_used_opins(pres_fac, rip_up_local_opins,
//                clb_opins_used_locally);
//
//        success = feasible_routing_rco();
//        if (success) {
//            printf
//                    ("Successfully routed after %d routing iterations.\n",
//                    itry);
//            return (TRUE);
//        }
//
//        if (itry == 1)
//            pres_fac = router_opts.initial_pres_fac;
//        else
//            pres_fac *= router_opts.pres_fac_mult;
//
//        pres_fac = min(pres_fac, HUGE_FLOAT / 1e5);
//
//        pathfinder_update_cost(pres_fac, router_opts.acc_fac);
//    }
//
//    printf("Routing failed.\n");
//    return (FALSE);
//}

//void
//pathfinder_add_alloc(int icon, float pres_fac) {
//    struct s_trace *route_segment_start = trace_head_con[icon];
//    int source = cons[icon].source;
//    int inode, occ, capacity;
//    struct s_trace* tptr = route_segment_start;
//    if (tptr == NULL) {/* No routing yet. */
//        printf("no routing in route segment.\n");
//        exit(10);
//    }
//    for (;;) {
//        inode = tptr->index;
//        /*Check if node is already used by source*/
//        boolean source_found = FALSE;
////        struct s_linked_vptr* current_element;
////        struct s_linked_vptr* prev_element = NULL;
//        struct s_source* current_element;
//        struct s_source* prev_element = NULL;
//        int old_capacity = rr_node[inode].occ;
//        if (old_capacity > 0) {
//            current_element = rr_node_route_inf[inode].source_list_head;
//            if (current_element != NULL) {
//                for (;;) {
////                    source_entry* so = (source_entry*) current_element->data_vptr;
////                    if (so->source == source) {
//                    if(current_element->source == source){
//                        source_found = TRUE;
//                        break;
//                    }
//                    if (current_element->next == NULL) {
//                        break;
//                    } else {
//                        prev_element = current_element;
//                        current_element = current_element->next;
//                    }
//                }
//            }
//        }
//        if (source_found) {
////            source_entry* se = (source_entry*) current_element->data_vptr;
////            se->usage++;
//            current_element->usage++;
//        } else {
//            /*Add source to source Array*/
////            source_entry* se = (source_entry*) my_malloc(sizeof (source_entry));
////            se->source = source;
////            se->usage = 1;
////            rr_node_route_inf[inode].source_list_head = insert_in_vptr_list(rr_node_route_inf[inode].source_list_head, se);
//            s_source* ssptr = alloc_source_data();
//            ssptr->source = source;
//            ssptr->usage = 1;
//            ssptr->next = rr_node_route_inf[inode].source_list_head;
//            rr_node_route_inf[inode].source_list_head = ssptr;
//            rr_node_route_inf[inode].source_list_head = current
//            rr_node_route_inf[inode].size_source_list++;
//            if (rr_node_route_inf[inode].size_source_list > rr_node_route_inf[inode].max_size_source_list)rr_node_route_inf[inode].max_size_source_list = rr_node_route_inf[inode].size_source_list;
//            /*Adapt occupation field and calculate present congestion factor*/
//            occ = rr_node[inode].occ + 1;
//            capacity = rr_node[inode].capacity;
//            rr_node[inode].occ = occ;
//            if (occ < capacity) {
//                rr_node_route_inf[inode].pres_cost = 1.;
//            } else {
//                rr_node_route_inf[inode].pres_cost =
//                        1. + (occ + 1 - capacity) * pres_fac;
//            }
//        }
//
//        if (rr_node[inode].type == SINK) {
//            if (tptr->next == NULL) {
//                trace_tail_con[icon] = tptr;
//                break;
//            } else {
//                tptr = tptr->next;
//            }
//        }
//
//        tptr = tptr->next;
//
//    } /* End while loop -- did an entire traceback. */
//}
static boolean
breadth_first_route_con(int icon, float bend_cost, float pres_fac, int itry) {

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

    int i, inode, prev_node, net;
    float pcost, new_pcost;
    struct s_heap *current;
    struct s_trace *tptr;

    net = cons[icon].net;
    
    free_traceback_con(icon);
    
    breadth_first_add_source_to_heap_con(cons[icon].source);
    /*Set sink target flag*/
    rr_node_route_inf[cons[icon].target_node].target_flag++;
    tptr = NULL;
    current = get_heap_head();
    if (current == NULL) { /* Infeasible routing.  No possible path for net. */
        reset_path_costs(); /* Clean up before leaving. */
        return (FALSE);
    }

    inode = current->index;
    while (!rr_node_route_inf[inode].target_flag) {
        pcost = rr_node_route_inf[inode].path_cost;
        new_pcost = current->cost;
        if (pcost > new_pcost) { /* New path is lowest cost. */
            rr_node_route_inf[inode].path_cost = new_pcost;
            prev_node = current->u.prev_node;
            rr_node_route_inf[inode].prev_node = prev_node;
            rr_node_route_inf[inode].prev_edge =
                    current->prev_edge;

            if (pcost > 0.99 * HUGE_POSITIVE_FLOAT) /* First time touched. */
                add_to_mod_list(&rr_node_route_inf[inode].path_cost);

            breadth_first_expand_neighbours_con(inode, new_pcost,
                    icon, net, bend_cost, pres_fac);
        }
        free_heap_data(current);
        current = get_heap_head();
        if (current == NULL) { /* Impossible routing. No path for net. */
            reset_path_costs();
            return (FALSE);
        }

        inode = current->index;
    }

    
    rr_node_route_inf[inode].target_flag--; /* Connected to this SINK. */
    update_traceback_con(current, icon);
    free_heap_data(current);

    empty_heap();
    //printf("%f\n",rr_node_route_inf[prev_node].path_cost);
    reset_path_costs();
    return (TRUE);
}

static boolean
breadth_first_route_con_fast(int icon, float bend_cost, float pres_fac, int itry) {

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

    int i, inode, prev_node, net;
    float pcost, new_pcost;
    struct s_heap *current;
    struct s_trace *tptr;

    net = cons[icon].net;
    
    free_traceback_con(icon);
    
    breadth_first_add_source_to_heap_con(cons[icon].source);
    /*Set sink target flag*/
    rr_node_route_inf[cons[icon].target_node].target_flag++;
    tptr = NULL;
    current = get_heap_head();
    if (current == NULL) { /* Infeasible routing.  No possible path for net. */
        reset_path_costs(); /* Clean up before leaving. */
        return (FALSE);
    }

    inode = current->index;
    while (!rr_node_route_inf[inode].target_flag) {
        pcost = rr_node_route_inf[inode].path_cost;
        new_pcost = current->cost;
        if (pcost > new_pcost) { /* New path is lowest cost. */
            rr_node_route_inf[inode].path_cost = new_pcost;
            prev_node = current->u.prev_node;
            rr_node_route_inf[inode].prev_node = prev_node;
            rr_node_route_inf[inode].prev_edge =
                    current->prev_edge;

            if (pcost > 0.99 * HUGE_POSITIVE_FLOAT) /* First time touched. */
                add_to_mod_list(&rr_node_route_inf[inode].path_cost);

            breadth_first_expand_neighbours_con_fast(inode, new_pcost,
                    icon, net, bend_cost, pres_fac);
        }
        free_heap_data(current);
        current = get_heap_head();
        if (current == NULL) { /* Impossible routing. No path for net. */
            reset_path_costs();
            return (FALSE);
        }

        inode = current->index;
    }

    
    rr_node_route_inf[inode].target_flag--; /* Connected to this SINK. */
    update_traceback_con(current, icon);
    free_heap_data(current);

    empty_heap();
    //printf("%f\n",rr_node_route_inf[prev_node].path_cost);
    reset_path_costs();
    return (TRUE);
}

static void
breadth_first_expand_neighbours_con(int inode,
        float pcost,
        int icon,
        int net,
        float bend_cost,
        float pres_fac) {

    /* Puts all the rr_nodes adjacent to inode on the heap.  rr_nodes outside   *
     * the expanded bounding box specified in route_bb are not added to the     *
     * heap.  pcost is the path_cost to get to inode.                           */

    int edge, to_node, num_edges;
    t_rr_type from_type, to_type;
    float tot_cost;

    num_edges = rr_node[inode].num_edges;
    for (edge = 0; edge < num_edges; edge++) {
        to_node = rr_node[inode].edges[edge];

        //doesn't work, getting empty heap errors
        if (rr_node[to_node].xhigh < route_bb[net].xmin ||
                rr_node[to_node].xlow > route_bb[net].xmax ||
                rr_node[to_node].yhigh < route_bb[net].ymin ||
                rr_node[to_node].ylow > route_bb[net].ymax)
            continue; /* Node is outside (expanded) bounding box. */

        float getcost = get_rr_cong_cost_con(to_node, icon, pres_fac);
        
        tot_cost = pcost + getcost;
        if (bend_cost != 0.) {
            from_type = rr_node[inode].type;
            to_type = rr_node[to_node].type;
            if ((from_type == CHANX && to_type == CHANY) ||
                    (from_type == CHANY && to_type == CHANX))
                tot_cost += bend_cost;
        }
        node_to_heap(to_node, tot_cost, inode, edge, OPEN, OPEN);
    }
}

static void
breadth_first_expand_neighbours_con_fast(int inode,
        float pcost,
        int icon,
        int net,
        float bend_cost,
        float pres_fac) {

    /* Puts all the rr_nodes adjacent to inode on the heap.  rr_nodes outside   *
     * the expanded bounding box specified in route_bb are not added to the     *
     * heap.  pcost is the path_cost to get to inode.                           */

    int edge, to_node, num_edges;
    t_rr_type from_type, to_type;
    float tot_cost;

    num_edges = rr_node[inode].num_edges;
    for (edge = 0; edge < num_edges; edge++) {
        to_node = rr_node[inode].edges[edge];

        //doesn't work, getting empty heap errors
        if (rr_node[to_node].xhigh < route_bb[net].xmin ||
                rr_node[to_node].xlow > route_bb[net].xmax ||
                rr_node[to_node].yhigh < route_bb[net].ymin ||
                rr_node[to_node].ylow > route_bb[net].ymax)
            continue; /* Node is outside (expanded) bounding box. */

        float getcost = get_rr_cong_cost(to_node);
        
        tot_cost = pcost + getcost;
        if (bend_cost != 0.) {
            from_type = rr_node[inode].type;
            to_type = rr_node[to_node].type;
            if ((from_type == CHANX && to_type == CHANY) ||
                    (from_type == CHANY && to_type == CHANX))
                tot_cost += bend_cost;
        }
        node_to_heap(to_node, tot_cost, inode, edge, OPEN, OPEN);
    }
}

static void
breadth_first_add_source_to_heap_con(int inode) {

    node_to_heap(inode, 0.0, NO_PREVIOUS, NO_PREVIOUS, OPEN, OPEN);
}
