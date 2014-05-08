#include <stdio.h>
#include <math.h>
#include "util.h"
#include "vpr_types.h"
#include "globals.h"
#include "route_common.h"
#include "croute_tree_timing.h"

/* This module keeps track of the partial routing graph for timing-driven     *
 * connection-based routing.  The normal traceback structure doesn't provide
 *  enough info about the partial routing during timing-driven routing, so 
 * the routines in this module are used to keep a tree representation of the 
 * partial  routing graph during timing-driven routing. This allows rapid incremental *
 * timing analysis.  The net_delay module does timing analysis in one step   *
 * (not incrementally as pieces of the routing are added).  A lot of 
 * net_delay.c could be removed and call the corresponding routines here, 
 * but it's useful to have a from-scratch delay calculator to check    *
 * the results of this one.                                                  */


/********************** Variables local to this module ***********************/

/* Array below allows mapping from any rr_node to any rt_node currently in   *
 * the rt_tree.                                                              */

/* Frees lists for fast addition and deletion of nodes and edges. */

static t_rg_edge *rg_edge_free_list = NULL;
static t_linked_rg_edge_ref *link_free_list = NULL;
static t_rg_node *rg_node_free_list = NULL;
static int stumbles_0 = 0;
static int stumbles_1 = 0;


/********************** Subroutines local to this module *********************/

static t_rg_node *alloc_rg_node(void);

static void free_rg_node(t_rg_node * rt_node);

static t_linked_rg_edge_ref *alloc_linked_rg_edge_ref(void);

static void free_rg_edge(t_rg_edge * edge);

static t_rg_node *
add_con_path_to_route_graph_and_update_R_upstream(int icon, t_rg_node *root, s_rr_to_rg_node_hash_map* node_map);

static void load_new_path_R_upstream(t_rg_node * start_of_new_path_rg_node);

static t_rg_node *update_unbuffered_ancestors_C_downstream(t_rg_node *
							   start_of_new_path_rg_node);

static t_rg_node *update_unbuffered_ancestors_C_downstream_rip_up(t_rg_node *
							   start_of_new_path_rg_node);

static void load_Tdel(t_rg_node * subtree_rg_root,
				 float Tarrival, int icon);

static t_rg_node * unload_branch_and_find_start_of_branch(t_rg_node * sink);

static void recursive_update(t_rg_node * rg_node, float C_downstream_addition);

static t_rg_node *
find_next_joint_and_disconnect(int icon, t_rg_node * branch_point);

static void
update_subtree(t_rg_node * rg_node);

static t_rg_node *
unload_branch(t_rg_node * end);

static float
calculate_C_downstream(t_rg_node * rg_node);

static float
update_C_downstream(t_rg_node * rg_node, int icon);

static void
free_rg_link(t_linked_rg_edge_ref * link);

static void
load_Tdel_in_net_delay_structure(int net, s_rr_to_rg_node_hash_map* node_map, float* delays);


/************************** Subroutine definitions ***************************/



t_rg_node *
update_route_graph(int icon, t_rg_node *root, s_rr_to_rg_node_hash_map* node_map, float* delays)
{

    /* Adds the most recently finished wire segment to the routing tree, and    *
     * updates the Tdel, etc. numbers for the rest of the routing tree.  hptr   *
     * is the heap pointer of the SINK that was reached.  This routine returns  *
     * a pointer to the rt_node of the SINK that it adds to the routing.        */

    t_rg_node *sink_rg_node;
    t_rg_node *unbuffered_subtree_rt_root;

    sink_rg_node = add_con_path_to_route_graph_and_update_R_upstream(icon, root, node_map);

//    if(icon==148){
//    	printf("print routing graph.\n");
//    	print_rg(root,0);
//    }

//    printf("Update C downstream.\n");
    update_C_downstream(root, icon);

//    printf("load_rg_subgraph_Tdel\n");
    load_Tdel(root, 0.,icon);

//    printf("load_Tdel_in_net_delay_structure\n");
    load_Tdel_in_net_delay_structure(cons[icon].net, node_map, delays);

    return (sink_rg_node);
}

static void
load_Tdel_in_net_delay_structure(int net, s_rr_to_rg_node_hash_map* node_map, float* delays){
	int ipin;
	int first_con = clb_net[net].con;

	for(ipin=0; ipin<clb_net[net].num_sinks; ipin++){
		if(back_trace_head_con[first_con+ipin]!=NULL){
			delays[ipin+1] =  get_rr_to_rg_node_entry(node_map,back_trace_head_con[first_con+ipin]->next->index)->rg_node->Tdel;
		}
	}

}

void
print_and_reset_nr_of_stumbles(){
    printf("\nStumbles: %d - %d\n",stumbles_0, stumbles_1);
    stumbles_0=0;
    stumbles_1=0;
}

void print_rg(t_rg_node * node, int level){
	t_linked_rg_edge_ref* child = node->child_list;
	/* Flip visited switch*/
	node->visited = !node->visited;
	/* Print out children */
	while(child!=NULL){
		printf("l%d: %d has child %d\n",level,node->inode,child->edge->child->inode);
		child=child->next;
	}
	child = node->child_list;
	/* Recursive call for all children */
	while(child!=NULL){
		if(node->visited != child->edge->child->visited){
			print_rg(child->edge->child,level+1);
		}else{
			printf("Warning: print_rg is visiting a node that is already been visited. This means that loops are present in the routing graph of connection %d. The node will be discarded.\n");
		}
		child=child->next;
	}

}

void
free_route_graph_timing_structs(void)
{

/* Frees the structures needed to build routing trees, and really frees      *
 * (i.e. calls free) all the data on the free lists.                         */

    t_rg_node *rg_node, *next_node;
    t_linked_rg_edge_ref *rg_edge_ref, *next_edge_ref;
    t_rg_edge *rg_edge, *next_edge;

    rg_node = rg_node_free_list;
    while(rg_node != NULL)
	{
	    next_node = rg_node->u.next;
	    free(rg_node);
	    rg_node = next_node;
	}
    rg_node_free_list = NULL;

    rg_edge_ref = link_free_list;
    while(rg_edge_ref != NULL)
	{
	    next_edge_ref = rg_edge_ref->next;
	    free(rg_edge_ref);
	    rg_edge_ref = next_edge_ref;
	}
    link_free_list = NULL;

    rg_edge = rg_edge_free_list;
    while(rg_edge != NULL)
	{
    	next_edge = rg_edge->u.next;
	    free(rg_edge);
	    rg_edge = next_edge;
	}
    rg_edge_free_list = NULL;
}

static t_rg_node *
alloc_rg_node(void)
{

/* Allocates a new rt_node, from the free list if possible, from the free   *
 * store otherwise.                                                         */

    t_rg_node *rg_node = rg_node_free_list;

    if(rg_node != NULL){
	    rg_node_free_list = rg_node->u.next;
	}else{
	    rg_node = (t_rg_node *) my_malloc(sizeof(t_rg_node));
	}

    rg_node->child_list = NULL;
    rg_node->u.parent_list = NULL;
    rg_node->visited = 0;

    return (rg_node);
}

static void
free_rg_node(t_rg_node * rg_node)
{
    rg_node->no_parents = 0;

/* Adds rt_node to the proper free list.          */

    rg_node->u.next = rg_node_free_list;
    rg_node_free_list = rg_node;
}

static void
free_rg_edge(t_rg_edge * edge)
{

/* Adds the rt_edge to the rt_edge free list.                       */
    edge->u.next = rg_edge_free_list;
    rg_edge_free_list = edge;
}

static void
free_rg_link(t_linked_rg_edge_ref * link){
	link->next = link_free_list;
	link_free_list = link;
}

static t_linked_rg_edge_ref *alloc_linked_rg_edge_ref(void) {

    /* Allocates a new linked_rt_edge, from the free list if possible, from the  *
     * free store otherwise.                                                     */

	t_linked_rg_edge_ref *link = link_free_list;

    if (link != NULL) {
    	link_free_list = link_free_list->next;
    } else {
    	link = (t_linked_rg_edge_ref *) my_malloc(sizeof(t_linked_rg_edge_ref));
    }

    link->edge = NULL;
    link->next = NULL;

    return (link);
}

static t_rg_edge *alloc_rg_edge(void) {

    /* Allocates a new linked_rt_edge, from the free list if possible, from the  *
     * free store otherwise.                                                     */

	t_rg_edge *rg_edge;

    rg_edge = rg_edge_free_list;

    if (rg_edge_free_list != NULL) {
    	rg_edge_free_list = rg_edge_free_list->u.next;
    } else {
    	rg_edge = (t_rg_edge *) my_malloc(sizeof(t_rg_edge));
    }

    rg_edge->u.parent = NULL;
    rg_edge->usage = 0;
    rg_edge->iswitch = -1;
    rg_edge->child = NULL;

    return (rg_edge);
}

t_rg_node *
init_graph_to_source(int inet, t_rg_node** rg_sinks, s_rr_to_rg_node_hash_map* node_map)
{

/* Initializes the routing tree to just the net source, and returns the root *
 * node of the rt_tree (which is just the net source).                       */

    t_rg_node *rg_root;
    t_rg_node *sink_rg_node;
    int inode, isink;

    if(node_map->node_entries==NULL){
        int size = 32;
        node_map->no_entries = 0;
        node_map->size = size;
        node_map->node_entries = my_calloc(size,sizeof(s_node_entry*));
    }

    rg_root = alloc_rg_node();
    rg_root->child_list = NULL;
    rg_root->no_parents = 0;
    rg_root->u.parent_list=NULL;
    rg_root->re_expand = TRUE;
    inode = net_rr_terminals[inet][0];	/* Net source */
    rg_root->inode = inode;
    rg_root->C_downstream = rr_node[inode].C;
    rg_root->R_upstream = rr_node[inode].R;
    rg_root->Tdel = 0.5 * rr_node[inode].R * rr_node[inode].C;
    rg_root->visited = 0;
    add_rr_to_rg_node(node_map,inode,rg_root);
    
//    for(isink = 1; isink <= clb_net[inet].num_sinks; isink++)
//    {
//        sink_rg_node = alloc_rg_node();
//        sink_rg_node->child_list = NULL;
//        sink_rg_node->no_parents = 0;
//        sink_rg_node->parent_list=NULL;
//        inode = net_rr_terminals[inet][isink];	/* Net source */
//        sink_rg_node->inode = inode;
//        sink_rg_node->C_downstream = rr_node[inode].C;
//        sink_rg_node->R_upstream = rr_node[inode].R;
//        rt_root->Tdel =0.0;
//        rg_sinks[clb_net[inet].con+isink-1] = sink_rg_node;
//        add_rr_to_rg_node(node_map,inode,sink_rg_node);
//    }
    return (rg_root);
}

static t_rg_node *
add_con_path_to_route_graph_and_update_R_upstream(int icon, t_rg_node *root, s_rr_to_rg_node_hash_map* node_map) {

    /* Adds the most recent wire segment, ending at the SINK indicated by hptr, *
     * to the routing tree.  It returns the first (most upstream) new rt_node,  *
     * and (via a pointer) the rt_node of the new SINK.                         */

    struct s_trace *tptr;
    struct s_trace *prev_tptr;
    float R_upstream;
    float R_upstream_max;
    t_linked_rg_edge_ref *link;
    t_linked_rg_edge_ref *prev_link;
    t_linked_rg_edge_ref *parent_link;
    t_linked_rg_edge_ref *it;
    t_rg_edge *edge;
    s_rr_to_rg_node_entry *node_entry;
    boolean sink_found = FALSE;
    
    t_rg_node * rg_node = root;
    t_rg_node * prev_rg_node = NULL;
    int i;
    

    tptr = trace_head_con[icon];
    
    
    prev_tptr = tptr;
    tptr = tptr->next;
    
    link = root->child_list;
    prev_link = NULL;
    
    
    while(!sink_found){
        /* Find branch point, explore the routing time graph until branch point is found. */
    	//printf("Find branch point\n");
        while(link!=NULL){
            //printf("link: %d\n",link->edge->child->inode);
            if(link->edge->child->inode == tptr->index){
            	link->edge->usage++;
            	prev_rg_node = rg_node;
                rg_node = link->edge->child;
                prev_tptr = tptr;
                tptr = tptr->next;
                prev_link = link;
                link = rg_node->child_list;
            }else{
                prev_link = link;
                link = link->next;
            }
        }
        //printf("tptr->index %d, tptr-next->index %d\n",tptr->index,tptr->next->index);
        //printf("get_rr_to_rg_node_entry(node_map,tptr->index)->usage %d, get_rr_to_rg_node_entry(node_map,tptr->next->index)->usage %d\n",get_rr_to_rg_node_entry(node_map,tptr->index)->usage,get_rr_to_rg_node_entry(node_map,tptr->next->index)->usage);
        if(get_rr_to_rg_node_entry(node_map,tptr->index)->usage>1){
        	//printf("Switch between branches.\n");
        	//printf("rg_node %d, prev_rg_node %d\n",rg_node->inode,prev_rg_node->inode);


            /* Switch between branches */
            link = alloc_linked_rg_edge_ref();
            edge = alloc_rg_edge();
            link->edge = edge;
#ifdef DEBUG
            if (prev_link == NULL) {
                printf("Error in add_connection_path_to_route_graph. prev_edge is NULL, while switching between branches.\n", tptr->index ,tptr->next->index);
                exit(1);
            }
#endif
            prev_link->next = link;
            edge->iswitch = prev_tptr->iswitch;
            R_upstream = rg_node->R_upstream;
            //printf("before prev_rg_node->inode %d\n",prev_rg_node->inode);
            //printf("before rg_node->inode %d\n",rg_node->inode);
            prev_rg_node = rg_node;
            rg_node = get_rr_to_rg_node_entry(node_map, tptr->index)->rg_node;
            //printf("after prev_rg_node->inode %d\n",prev_rg_node->inode);
            //printf("rg_node %d\n",rg_node);
            //printf("after rg_node->inode %d\n",rg_node->inode);
            edge->child = rg_node;
            edge->u.parent = prev_rg_node;
            edge->usage=1;
            
            if(switch_inf[link->edge->iswitch].buffered){
                R_upstream = switch_inf[edge->iswitch].R + rr_node[rg_node->inode].R;
            }else{
                R_upstream = R_upstream + switch_inf[edge->iswitch].R + rr_node[rg_node->inode].R;
            }
            
            if(R_upstream>rg_node->R_upstream) {
                rg_node->R_upstream = R_upstream;
            }

            parent_link = alloc_linked_rg_edge_ref();
            parent_link->edge = edge;
            parent_link->next = rg_node->u.parent_list;
            rg_node->u.parent_list = parent_link;
            rg_node->no_parents++;
            prev_tptr = tptr;
            tptr = tptr->next;
            link = rg_node->child_list;
        }else{
        	//printf("A new part has to be added.\n");
            /* A new part has to be added to the routing time graph */
            /* Add new part of the path to the routing time graph */
            /* First, add a child edge to the branch point*/

            R_upstream = rg_node->R_upstream;
            prev_rg_node = rg_node;
            rg_node = alloc_rg_node();
            rg_node->inode = tptr->index;
            //printf("assigning rg_node to entry %d \n", tptr->index);
            get_rr_to_rg_node_entry(node_map, tptr->index)->rg_node = rg_node;
            if(prev_rg_node!=NULL) rg_node->visited = prev_rg_node->visited;

        	edge = alloc_rg_edge();
        	//printf("prev_tptr->index %d\n",prev_tptr->index);
        	edge->iswitch = prev_tptr->iswitch;
            edge->child = rg_node;
            edge->u.parent = prev_rg_node;
            edge->usage = 1;

            if(switch_inf[edge->iswitch].buffered)
                rg_node->R_upstream = switch_inf[edge->iswitch].R + rr_node[rg_node->inode].R;
            else
                rg_node->R_upstream = R_upstream + switch_inf[edge->iswitch].R + rr_node[rg_node->inode].R;
        	
            /* Child link */
            link = alloc_linked_rg_edge_ref();
        	link->edge = edge;
        	link->next = prev_rg_node->child_list;
        	prev_rg_node->child_list = link;
            
            parent_link = alloc_linked_rg_edge_ref();
            parent_link->next =  NULL;
            parent_link->edge = edge;
            rg_node->u.parent_list = parent_link;
            rg_node->no_parents = 1;

            node_entry = get_rr_to_rg_node_entry(node_map, tptr->next->index);
            while(node_entry->rg_node==NULL && rr_node[tptr->next->index].type!=1){
                prev_tptr = tptr;
                tptr = tptr->next;
                prev_rg_node = rg_node;
                rg_node = alloc_rg_node();
                rg_node->inode = tptr->index;
                //printf("assigning rg_node to entry %d \n", tptr->index);
                node_entry->rg_node = rg_node;

                //printf("Add %d\n",rg_node->inode);

                if(switch_inf[prev_tptr->iswitch].buffered)
                    rg_node->R_upstream = switch_inf[prev_tptr->iswitch].R + rr_node[rg_node->inode].R;
                else
                    rg_node->R_upstream = prev_rg_node->R_upstream + switch_inf[prev_tptr->iswitch].R + rr_node[rg_node->inode].R;

                /* Edge */
                edge = alloc_rg_edge();
                edge->child = rg_node;
                edge->u.parent = prev_rg_node;
                edge->usage = 1;
                edge->iswitch = prev_tptr->iswitch;
                /* Child link*/
                link = alloc_linked_rg_edge_ref();
                link->edge = edge;
                link->next = NULL;
                prev_rg_node->child_list = link;
                /* Parent link */
                parent_link = alloc_linked_rg_edge_ref();
                parent_link->edge = edge;
                parent_link->next = NULL;
                rg_node->u.parent_list = parent_link;
                rg_node->no_parents=1;
                node_entry = get_rr_to_rg_node_entry(node_map, tptr->next->index);
                //printf("node %d, type %d\n",tptr->index, rr_node[tptr->next->index].type);

            }
            if(rr_node[tptr->next->index].type==1){
                rg_node->child_list=NULL;
                //printf("sink inode %d, u.child_list %d, type node %d, prev_rg_node(%d)->child_list %d\n",rg_node->inode, rg_node->child_list, rr_node[rg_node->inode].type, prev_rg_node->inode, prev_rg_node->child_list);
                return rg_node;
            }else{
                /* The trace reconnects with the routing graph*/
//            	printf("The trace reconnects with the routing graph\n");
//                printf("before prev_rg_node->inode %d\n",prev_rg_node->inode);
//                printf("before rg_node->inode %d\n",rg_node->inode);
                prev_tptr = tptr;
                tptr = tptr->next;
                prev_rg_node = rg_node;
                rg_node = get_rr_to_rg_node_entry(node_map, tptr->index)->rg_node;
//                printf("after prev_rg_node->inode %d\n",prev_rg_node->inode);
//                printf("after rg_node->inode %d\n",rg_node->inode);
//                printf("after prev_tptr %d\n",prev_tptr->index);
//                printf("after tptr %d\n",tptr->index);
                /* Edge */
                edge = alloc_rg_edge();
                edge->child = rg_node;
                edge->u.parent = prev_rg_node;
                edge->usage = 1;
                edge->iswitch = prev_tptr->iswitch;
                /* Child link*/
                link = alloc_linked_rg_edge_ref();
                link->edge = edge;
                link->next = NULL;
                prev_rg_node->child_list = link;
                /* Parent link */
                parent_link = alloc_linked_rg_edge_ref();
                parent_link->edge = edge;
                parent_link->next = rg_node->u.parent_list;
                rg_node->u.parent_list = parent_link;
                rg_node->no_parents++;
                if(!switch_inf[tptr->iswitch].buffered){
                	link = parent_link;
                	R_upstream_max = 0.0;
                    while(link->next!=NULL){
                        if(!switch_inf[link->edge->iswitch].buffered && (link->edge->child->R_upstream > R_upstream_max)) R_upstream_max = link->edge->child->R_upstream;
                        link = link->next;
                    }
                    /* Update R_upstream */
                    if((prev_rg_node->R_upstream - R_upstream_max)>0.0001){
                        rg_node->R_upstream = prev_rg_node->R_upstream + switch_inf[prev_tptr->iswitch].R + rr_node[rg_node->inode].R;
                        update_subtree(rg_node);
                    }
                }

//                printf("tptr %d\n",tptr->index);
                prev_tptr = tptr;
                tptr = tptr->next;
                link = rg_node->child_list;
            }
        }


    }

}

static void
update_subtree(t_rg_node * rg_node){
    float R_upstream_max;
    
    t_linked_rg_edge_ref *child_link = rg_node->child_list;
    t_rg_node * child = NULL;
    t_linked_rg_edge_ref *parent_link = NULL;
    t_rg_node * parent = NULL;
    
    while(child_link!=NULL){
        child = child_link->edge->child;
        if(!switch_inf[child_link->edge->iswitch].buffered){
            R_upstream_max = 0.0;
            parent_link = child->u.parent_list;
            while(parent_link!=NULL){
                parent = parent_link->edge->u.parent;
                if(parent->inode!=rg_node->inode && !switch_inf[parent_link->edge->iswitch].buffered && parent->R_upstream > R_upstream_max){
                    R_upstream_max = parent->R_upstream;
                }
                parent_link = parent_link->next;    
            }
            if(rg_node->R_upstream>R_upstream_max){
                child->R_upstream = rg_node->R_upstream + switch_inf[child_link->edge->iswitch].R + rr_node[child->inode].R;
                update_subtree(child);
            }
        }
        child_link = child_link->next;
    }
    
}

static void 
recursive_update(t_rg_node * rg_node, float C_downstream_addition)
{

/* Updates the C_downstream values for the ancestors of the new path.  Once *
 * a buffered switch is found amongst the ancestors, no more ancestors are  *
 * affected.  Returns the root of the "unbuffered subtree" whose Tdel       *
 * values are affected by the new path's addition.                          */

    t_rg_node  *parent_rg_node;
    t_linked_rg_edge_ref * parent_it;
    short iswitch;
    
    parent_it = rg_node->u.parent_list;
    rg_node->C_downstream += C_downstream_addition;
    while(parent_it!=NULL) {
        parent_rg_node = parent_it->edge->u.parent;
        iswitch = parent_it->edge->iswitch;
        if (switch_inf[iswitch].buffered == FALSE) {
            recursive_update(parent_rg_node,C_downstream_addition/parent_rg_node->no_parents);
        }
        parent_it = parent_it->next;
    }
}

void
remove_connection_from_route_timing_graph(int icon, t_rg_node *root, t_rg_node *sink_rg_node) {

    /* Adds the most recently finished wire segment to the routing tree, and    *
     * updates the Tdel, etc. numbers for the rest of the routing tree.  hptr   *
     * is the heap pointer of the SINK that was reached.  This routine returns  *
     * a pointer to the rt_node of the SINK that it adds to the routing.        */

    t_rg_node *rg_node, *parent_rg_node;
    t_linked_rg_edge_ref *parent_link, *child_link, *it, *prev, *sel;
    t_rg_edge *edge;
    struct s_trace *btptr;
    float R_upstream_max;
    boolean found;

    if (root->child_list == NULL || sink_rg_node == NULL) return;

    it = NULL;
    prev = NULL;

    rg_node = sink_rg_node;
    btptr = back_trace_head_con[icon]->next->next;
    parent_link = rg_node->u.parent_list;
    child_link = NULL;
#ifdef DEBUG
    if(parent_link == NULL){
        printf("Error: parent of IPIN is NULL, should be a wire node.\n");
        exit(5);
    }
#endif
    while(parent_link != NULL){
    	edge = parent_link->edge;
    	parent_rg_node = edge->u.parent ;

    	if(edge->usage==1){
			//printf("inode %d, type %d, parent %d\n", rg_node->inode, rr_node[rg_node->inode].type,parent_rg_node->inode);

			/* Select and remove parent link */
			//printf("Select and remove parent link\n");
			R_upstream_max = 0.0;
		    sel = NULL;
		    prev = NULL;
		    found = FALSE;
			it = rg_node->u.parent_list;
			while(it!=NULL){
				//printf("it->edge->u.parent->inode %d, parent_rg_node->inode %d\n",it->edge->u.parent->inode,parent_rg_node->inode);
				if(!found && it->edge->u.parent->inode==parent_rg_node->inode){
					sel = it;
					found =  TRUE;
				}else if((it->edge->u.parent->R_upstream>R_upstream_max)){
					R_upstream_max = it->edge->u.parent->R_upstream;
				}
				if(!found) prev = it;
				it = it->next;
			}
		    if(sel->edge->u.parent->R_upstream>R_upstream_max){
		        rg_node->R_upstream = R_upstream_max;
		        update_subtree(rg_node);
		    }
			if(prev == NULL){
				rg_node->u.parent_list = sel->next;
			}else{
				prev->next = sel->next;
			}
			free_rg_link(sel);

			/* Select and remove child link of parent */
			//printf("Select and remove child link of parent\n");
			prev = NULL;
			it = parent_rg_node->child_list;
			//printf("it->edge->child->inode %d, rg_node->inode %d\n",it->edge->child->inode,rg_node->inode);
			while(it->edge->child->inode!=rg_node->inode){
				prev = it;
				it=it->next;
				//printf("it->edge->child->inode %d, rg_node->inode %d\n",it->edge->child->inode,rg_node->inode);
			}
			if(prev == NULL){
				parent_rg_node->child_list = it->next;
			}else{
				prev->next = it->next;
			}
			free_rg_link(it);
			if(rr_node_route_inf[rg_node->inode].usage == 0){
				free_rg_node(rg_node);
			}

			free_rg_edge(edge);
    	}else{
    		edge->usage--;
    	}

		rg_node = parent_rg_node;
		btptr = btptr->next;
		parent_link = rg_node->u.parent_list;
//		printf("rg_node->inode %d\n",rg_node->inode);
//		if(btptr!=NULL) printf("btptr->index %d, parent_link %d\n",btptr->index,parent_link);
//		else printf("btptr NULL, parent_link %d\n",parent_link);

		while(parent_link != NULL && parent_link->edge->u.parent->inode != btptr->index){
			//printf("parent_link %d, parent_link->edge->u.parent->inode %d\n",parent_link,parent_link->edge->u.parent->inode);
			parent_link = parent_link->next;
		}
//		if(parent_link==NULL){
//			printf("parent_link null\n");
//		}else{
//			printf("parent_link->edge->u.parent->inode %d, btptr->index %d\n",parent_link->edge->u.parent->inode, btptr->index);
//		}
//		printf("a\n");
    }
//    printf("return rg node inode %d, type %d child list %d, no_parents %d\n", rg_node->inode, rr_node[rg_node->inode].type,rg_node->child_list,rg_node->no_parents);

//    printf("Update C downstream.\n");
    //update_C_downstream(root,icon);
}

static void
load_Tdel(t_rg_node * subtree_rt_root,
		     float Tarrival, int con)
{

/* Updates the Tdel values of the subtree rooted at subtree_rt_root by      *
 * by calling itself recursively.  The C_downstream values of all the nodes *
 * must be correct before this routine is called.  Tarrival is the time at  *
 * at which the signal arrives at this node's *input*.                      */

    int inode;
    short iswitch;
    t_rg_node *child_node;
    t_linked_rg_edge_ref *link;
    float Tdel, Tchild;

    inode = subtree_rt_root->inode;

/* Assuming the downstream connections are, on average, connected halfway    *
 * along a wire segment's length.  See discussion in net_delay.c if you want *
 * to change this.                                                           */

    Tdel = Tarrival + 0.5 * subtree_rt_root->C_downstream * rr_node[inode].R;
    subtree_rt_root->Tdel = Tdel;

    subtree_rt_root->visited = !subtree_rt_root->visited;

/* Now expand the children of this node to load their Tdel values (depth-   *
 * first pre-order traversal).                                              */

    link = subtree_rt_root->child_list;

    while(link != NULL)
	{
	    child_node = link->edge->child;

	    if(child_node->visited != subtree_rt_root->visited){
		    iswitch = link->edge->iswitch;
		    Tchild = Tdel + switch_inf[iswitch].R * child_node->C_downstream;
		    Tchild += switch_inf[iswitch].Tdel;	/* Intrinsic switch delay. */
			load_Tdel(child_node, Tchild, con);
		}else{
//			printf("Warning: load_rg_subgraph_Tdel is visiting a node that is already been visited. This means that loops are present in the routing graph of connection %d. The node will be discarded.\n");
			stumbles_1++;

		}
	    link = link->next;
	}
}

static float
update_C_downstream(t_rg_node * rg_node, int icon){
	float C, C_downstream;
    t_linked_rg_edge_ref *child_link;
    short iswitch;

    rg_node->visited = !rg_node->visited;
    C = rr_node[rg_node->inode].C;
    child_link = rg_node->child_list;

    while(child_link!=NULL){
		if(rg_node->visited != child_link->edge->child->visited){
		    C_downstream = update_C_downstream(child_link->edge->child, icon);
		    if(switch_inf[child_link->edge->iswitch].buffered == FALSE){
		    	C += C_downstream;
		    }
		}else{
//			printf("Warning: update_C_downstream is visiting a node that is already been visited. This means that loops are present in the routing graph of connection %d. The node will be discarded.\n", icon);
			stumbles_0++;
		}
        child_link = child_link->next;
    }

    rg_node->C_downstream = C;
    return (C);
}

void
free_rg(t_rg_node * rg_node)
{

/* Puts the rt_nodes and edges in the tree rooted at rt_node back on the    *
 * free lists.  Recursive, depth-first post-order traversal.                */

    t_rg_node *child_node;
    t_rg_edge *edge;
    t_linked_rg_edge_ref *edge_ref, *next_edge_ref;

    rg_node->visited = !rg_node->visited;
    edge_ref = rg_node->child_list;

    while(edge_ref != NULL){
    		/* For all children */
    		edge = edge_ref->edge;
	    child_node = edge->child;
	    next_edge_ref = edge_ref->next;
	    if(rg_node->visited != child_node->visited){
			free_rg(child_node);
	    }
		free_rg_edge(edge);
		free_rg_link(edge_ref);
		edge_ref = next_edge_ref;
	}

    free_rg_node(rg_node);
}

//t_rg_node* get_rg_node(int rr_node){
//    return rr_node_to_rt_node[rr_node];
//}
