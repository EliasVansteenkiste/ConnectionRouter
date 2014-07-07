#include <assert.h>
#include "util.h"
#include "vpr_types.h"
#include "globals.h"
#include "path_delay2.h"
#include "read_xml_arch_file.h"

/************* Variables (globals) shared by all path_delay modules **********/

int num_tnode_levels; /* Number of levels in the timing graph. */

struct s_ivec *tnodes_at_level;
/* [0..num__tnode_levels - 1].  Count and list of tnodes at each level of    
 * the timing graph, to make topological searches easier. Level-0 nodes are
 * sources to the timing graph (types TN_FF_SOURCE, TN_INPAD_SOURCE
 * and TN_CONSTANT_GEN_SOURCE). Level-N nodes are in the immediate fanout of 
 * nodes with level at most N-1. */

/******************* Subroutines local to this module ************************/

static int *alloc_and_load_tnode_fanin_and_check_edges(int *num_sinks_ptr);
static void show_combinational_cycle_candidates();

/************************** Subroutine definitions ***************************/
static int is_explicit_connection(int inode)
{
	int i;
	//printf("Checking: %d with: \n",inode);
	for(i = 0; i < num_tnodes_explicit; i++ )
	{
	//	printf("%d ",i);
		if(tnode_explicit[i] == &tnode[inode]){
	//		printf("Found: %d\n",inode);
			return i;
		}
	}
	//printf("\n");
	return -1;
}

static int is_the_same_edge(t_tedge edge_a,t_tedge edge_b)
{
	if(edge_a.to_node == edge_b.to_node
		&& edge_a.to_port == edge_b.to_port
		&& edge_a.to_pin == edge_b.to_pin)
		return TRUE;
	return FALSE;
}

static void set_fanin_for_explicit_connections(int *tnode_num_fanin)
{
	int inode,inode_explicit,inode_explicit_trav, iedge,iedge_explicit,iedge_explicit_trav, to_node, num_edges, error, num_sinks,i;
	t_tedge *marked_tedge;
	t_tedge temp_tedge;
	int max_num_marked_tedge = 10,idx_marked_tedge = 0; /*  We will mark the inodes that drive the same tnode in order not to count more
								than once the fanin*/

	marked_tedge = (t_tedge *)my_malloc(max_num_marked_tedge * sizeof(t_tedge));

	for (inode = 0; inode < num_tnodes; inode++) { 																/* For all the nodes */
		if( (inode_explicit = is_explicit_connection(inode)) != -1 ){												/* If the node has more than one edge that is explicit to another node */
			for(inode_explicit_trav = 0; inode_explicit_trav < num_tnodes_explicit; inode_explicit_trav++){		/* Compare the above node with all the other nodes that have an explicit connection */
				for(iedge_explicit = 0; iedge_explicit < tnode[inode_explicit].num_edges; iedge_explicit++)		/* For all the edges of the first node */
					for(iedge_explicit_trav = 0; iedge_explicit_trav < tnode[inode_explicit_trav].num_edges; iedge_explicit_trav++) /* For all the edges of the node that we compare with */
						if(is_the_same_edge(tnode_explicit[inode_explicit]->out_edges[iedge_explicit],tnode_explicit[inode_explicit_trav]->out_edges[iedge_explicit_trav])){ /* If they share an explicit connection to the same node, on the same port and pin	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	do not count the fanin for both of them but only for one of them */
							for(i = 0; i < idx_marked_tedge ;i++)
								if(is_the_same_edge(tnode_explicit[inode_explicit_trav]->out_edges[iedge_explicit_trav],marked_tedge[i]))
									break;
							/* We mark all the edges on each pin and port in order not to count the fanin twice */

							if(i == idx_marked_tedge){
								/* Then these two are driving the same pin */
								idx_marked_tedge++;
								if(idx_marked_tedge == max_num_marked_tedge){
									max_num_marked_tedge = max_num_marked_tedge << 1;
									marked_tedge = (t_tedge *)my_realloc(marked_tedge, max_num_marked_tedge * sizeof(t_tedge));
								}
								temp_tedge = tnode_explicit[inode_explicit]->out_edges[iedge_explicit];
								marked_tedge[idx_marked_tedge - 1] = temp_tedge;

								if (temp_tedge.to_node < 0
										|| temp_tedge.to_node >= num_tnodes) {
									vpr_printf(TIO_MESSAGE_ERROR, "in alloc_and_load_tnode_fanin_and_check_edges:\n");
									vpr_printf(TIO_MESSAGE_ERROR, "\ttnode #%d edge #%d goes to illegal node #%d.\n",inode, iedge, to_node);
									error++;
								}
								printf("Found an explicit edge: to_node:%d to_port:%d to_pin:%d\n",temp_tedge.to_node,temp_tedge.to_port,temp_tedge.to_pin);
								tnode_num_fanin[temp_tedge.to_node]++;
							}
						}
			}
		}
	}
}

static int *
alloc_and_load_tnode_fanin_and_check_edges(int *num_sinks_ptr) {

	/* Allocates an array and fills it with the number of in-edges (inputs) to   *
	 * each tnode.  While doing this it also checks that each edge in the timing *
	 * graph points to a valid tnode. Also counts the number of sinks.           */

	int inode, iedge, to_node, num_edges, error, num_sinks;
	int *tnode_num_fanin;
	t_tedge *tedge;

	tnode_num_fanin = (int *) my_calloc(num_tnodes, sizeof(int));
	error = 0;
	num_sinks = 0;

	set_fanin_for_explicit_connections(tnode_num_fanin);


	for (inode = 0; inode < num_tnodes; inode++) {
		if(is_explicit_connection(inode) != -1)
			continue;
		num_edges = tnode[inode].num_edges;
		//printf("Node tested for fanin: %s : %d\n",logical_block[tnode[inode].block].name,inode);
		if (num_edges > 0) {
			tedge = tnode[inode].out_edges;

			for (iedge = 0; iedge < num_edges; iedge++) {
				to_node = tedge[iedge].to_node;
				//printf("Edge: %d to_node:%d\n",iedge,to_node);
				//printf("From node: %d to_node: %d\n",inode,to_node);
				if (to_node < 0 || to_node >= num_tnodes) {
					vpr_printf(TIO_MESSAGE_ERROR, "in alloc_and_load_tnode_fanin_and_check_edges:\n");
					vpr_printf(TIO_MESSAGE_ERROR, "\ttnode #%d edge #%d goes to illegal node #%d.\n",inode, iedge, to_node);
					error++;
				}
				//if(tnode[inode].type != TN_PRIMITIVE_IPIN_EXPLICIT)
				tnode_num_fanin[to_node]++;
				//printf("edge: %d fanin: %d node:%s amount:%d from: %d to_node: %d\n",iedge,tnode_num_fanin[to_node],logical_block[tnode[to_node].block].name,tnode_num_fanin[to_node],inode,to_node);
			}
		}

		else if (num_edges == 0) {
			num_sinks++;
		}

		else {
			vpr_printf(TIO_MESSAGE_ERROR, "in alloc_and_load_tnode_fanin_and_check_edges:\n");
			vpr_printf(TIO_MESSAGE_ERROR, "\ttnode #%d has %d edges.\n", inode, num_edges);
			error++;
		}

	}

	if (error != 0) {
		vpr_printf(TIO_MESSAGE_ERROR, "Found %d Errors in the timing graph. Aborting.\n", error);
		exit(1);
	}

	*num_sinks_ptr = num_sinks;
	return (tnode_num_fanin);
}

int alloc_and_load_timing_graph_levels(void) {

	/* Does a breadth-first search through the timing graph in order to levelize  *
	 * it.  This allows subsequent traversals to be done topologically for speed. *
	 * Also returns the number of sinks in the graph (nodes with no fanout).      */

	t_linked_int *free_list_head, *nodes_at_level_head;
	int inode, num_at_level, iedge, to_node, num_edges, num_sinks, num_levels,i;
	t_tedge *tedge;

	/* [0..num_tnodes-1]. # of in-edges to each tnode that have not yet been    *
	 * seen in this traversal.                                                  */

	int *tnode_fanin_left;

	tnode_fanin_left = alloc_and_load_tnode_fanin_and_check_edges(&num_sinks);

	free_list_head = NULL;
	nodes_at_level_head = NULL;

	/* Very conservative -> max number of levels = num_tnodes.  Realloc later.  *
	 * Temporarily need one extra level on the end because I look at the first  *
	 * empty level.                                                             */

	tnodes_at_level = (struct s_ivec *) my_malloc((num_tnodes + 1) * sizeof(struct s_ivec));

	/* Scan through the timing graph, putting all the primary input nodes (no    *
	 * fanin) into level 0 of the level structure.                               */

	num_at_level = 0;

	for (inode = 0; inode < num_tnodes; inode++) {
		if (tnode_fanin_left[inode] == 0) {
			//if(tnode[inode].type == 0){
			//	printf("name of fanin == 0 : %s %d type:%d\n",logical_block[tnode[inode].block].name,inode,tnode[inode].type);
				num_at_level++;
				nodes_at_level_head = insert_in_int_list(nodes_at_level_head, inode,&free_list_head);
			//}else
				//printf("NOT INCLUDED: name of fanin == 0 : %s %d type:%d\n",logical_block[tnode[inode].block].name,inode,tnode[inode].type);
		}
	}

	alloc_ivector_and_copy_int_list(&nodes_at_level_head, num_at_level,&tnodes_at_level[0], &free_list_head);

	num_levels = 0;

	while (num_at_level != 0) { /* Until there's nothing in the queue. */
		num_levels++;
		num_at_level = 0;

		for (i = 0; i < tnodes_at_level[num_levels - 1].nelem; i++) {
			inode = tnodes_at_level[num_levels - 1].list[i];
			tedge = tnode[inode].out_edges;
			num_edges = tnode[inode].num_edges;

			for (iedge = 0; iedge < num_edges; iedge++) {
				to_node = tedge[iedge].to_node;
				tnode_fanin_left[to_node]--;
				//printf("From node: %s to_node: %s fanin_left:%d\n",logical_block[tnode[iedge].block].name,logical_block[tnode[to_node].block].name,tnode_fanin_left[to_node]);
				if (tnode_fanin_left[to_node] == 0) {
					num_at_level++;
					//printf("Node:%s to_node:%s \n",logical_block[tnode[inode].block].name,logical_block[tnode[to_node].block].name);
					nodes_at_level_head = insert_in_int_list(nodes_at_level_head, to_node, &free_list_head);
				}
			}
		}

		alloc_ivector_and_copy_int_list(&nodes_at_level_head, num_at_level,	&tnodes_at_level[num_levels], &free_list_head);
	}

	tnodes_at_level = (struct s_ivec *) my_realloc(tnodes_at_level,	num_levels * sizeof(struct s_ivec));
	num_tnode_levels = num_levels;

	free(tnode_fanin_left);
	free_int_list(&free_list_head);
	return (num_sinks);
}

void check_timing_graph(int num_sinks) {

	/* Checks the timing graph to see that: (1) all the tnodes have been put    *
	 * into some level of the timing graph; */

	/* Addition error checks that need to be done but not yet implemented: (2) the number of primary inputs    *
	 * to the timing graph is equal to the number of input pads + the number of *
	 * constant generators; and (3) the number of sinks (nodes with no fanout)  *
	 * equals the number of output pads + the number of flip flops.             */

	int num_tnodes_check, ilevel, error;

	error = 0;
	num_tnodes_check = 0;

	/* TODO: Rework error checks for I/Os*/

	for (ilevel = 0; ilevel < num_tnode_levels; ilevel++)
		num_tnodes_check += tnodes_at_level[ilevel].nelem;

	if (num_tnodes_check != num_tnodes) {
		vpr_printf(TIO_MESSAGE_ERROR, "Error in check_timing_graph: %d tnodes appear in the tnode level structure. Expected %d.\n", 
				num_tnodes_check, num_tnodes);
		vpr_printf(TIO_MESSAGE_INFO, "Check the netlist for combinational cycles.\n");
		if (num_tnodes > num_tnodes_check) {
			show_combinational_cycle_candidates();
		}
		//error++;
	}
	/* Todo: Add error checks that # of flip-flops, memories, and other
	 black boxes match # of sinks/sources*/

	if (error != 0) {
		vpr_printf(TIO_MESSAGE_ERROR, "Found %d Errors in the timing graph. Aborting.\n", error);
		exit(1);
	}
}

float print_critical_path_node(FILE * fp, t_linked_int * critical_path_node) {

	/* Prints one tnode on the critical path out to fp. Returns the delay to the next node. */

	int inode, iblk, inet, downstream_node;
	t_pb_graph_pin * pb_graph_pin;
	e_tnode_type type;
	static const char *tnode_type_names[] = { "TN_INPAD_SOURCE", "TN_INPAD_OPIN",
			"TN_OUTPAD_IPIN", "TN_OUTPAD_SINK", "TN_CB_IPIN", "TN_CB_OPIN",
			"TN_INTERMEDIATE_NODE", "TN_PRIMITIVE_IPIN", "TN_PRIMITIVE_OPIN", "TN_FF_IPIN",
			"TN_FF_OPIN", "TN_FF_SINK", "TN_FF_SOURCE", "TN_FF_CLOCK", "TN_CONSTANT_GEN_SOURCE" };

	t_linked_int *next_crit_node;
	float Tdel;

	inode = critical_path_node->data;
	type = tnode[inode].type;
	iblk = tnode[inode].block;
	pb_graph_pin = tnode[inode].pb_graph_pin;

	fprintf(fp, "Node: %d  %s Block #%d (%s)\n", inode, tnode_type_names[type],
		iblk, block[iblk].name);

	if (pb_graph_pin == NULL) {
		assert(
				type == TN_INPAD_SOURCE || type == TN_OUTPAD_SINK || type == TN_FF_SOURCE || type == TN_FF_SINK);
	}

	if (pb_graph_pin != NULL) {
		fprintf(fp, "Pin: %s.%s[%d] pb (%s)", pb_graph_pin->parent_node->pb_type->name, pb_graph_pin->port->name, pb_graph_pin->pin_number, block[iblk].pb->rr_node_to_pb_mapping[pb_graph_pin->pin_count_in_cluster]->name);
	}
	if (type != TN_INPAD_SOURCE && type != TN_OUTPAD_SINK) {
		fprintf(fp, "\n");
	}

	fprintf(fp, "T_arr: %g  T_req: %g  ", tnode[inode].T_arr, tnode[inode].T_req);

	next_crit_node = critical_path_node->next;
	if (next_crit_node != NULL) {
		downstream_node = next_crit_node->data;
		Tdel = tnode[downstream_node].T_arr - tnode[inode].T_arr;
		fprintf(fp, "Tdel: %g\n", Tdel);
	} else { /* last node, no Tdel. */
		Tdel = 0.;
		fprintf(fp, "\n");
	}

	if (type == TN_CB_OPIN) {
		inet = block[iblk].pb->rr_graph[pb_graph_pin->pin_count_in_cluster].net_num;
		inet = vpack_to_clb_net_mapping[inet];
		fprintf(fp, "External-to-Block Net: #%d (%s).  Pins on net: %d.\n",
				inet, clb_net[inet].name, (clb_net[inet].num_sinks + 1));
	} else if (pb_graph_pin != NULL) {
		inet =
				block[iblk].pb->rr_graph[pb_graph_pin->pin_count_in_cluster].net_num;
		fprintf(fp, "Internal Net: #%d (%s).  Pins on net: %d.\n", inet,
				vpack_net[inet].name, (vpack_net[inet].num_sinks + 1));
	}

	fprintf(fp, "\n");
	return (Tdel);
}

/* Display nodes that are likely in combinational cycles */
static void show_combinational_cycle_candidates() {
	boolean *found_tnode;
	int ilevel, i, inode;

	found_tnode = (boolean*) my_calloc(num_tnodes, sizeof(boolean));

	for (ilevel = 0; ilevel < num_tnode_levels; ilevel++) {
		for (i = 0; i < tnodes_at_level[ilevel].nelem; i++) {
			inode = tnodes_at_level[ilevel].list[i];
			found_tnode[inode] = TRUE;
		}
	}

	vpr_printf(TIO_MESSAGE_INFO, "\tProblematic nodes:\n");
	for (i = 0; i < num_tnodes; i++) {
		if (found_tnode[i] == FALSE) {
			vpr_printf(TIO_MESSAGE_INFO, "\t\ttnode %d ", i);
			if (tnode[i].pb_graph_pin == NULL) {
				vpr_printf(TIO_MESSAGE_INFO, "block %s port %d pin %d\n", logical_block[tnode[i].block].name, tnode[i].prepacked_data->model_port, tnode[i].prepacked_data->model_pin);
			} else {
				vpr_printf(TIO_MESSAGE_INFO, "\n");
			}
		}
	}

	free(found_tnode);
}

