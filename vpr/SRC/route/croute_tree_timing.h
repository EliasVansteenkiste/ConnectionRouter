/************** Types and defines exported by route_tree_timing.c ************/

//struct s_linked_rg_edge
//{
//    struct s_rg_node *close_relative;
//    short iswitch;
//    struct s_linked_rg_edge *next;
//};
//
//typedef struct s_linked_rg_edge t_linked_rg_edge;
//
///* Linked list listing the children of an rt_node.                           *
// * child:  Pointer to an rt_node (child of the current node).                *
// * iswitch:  Index of the switch type used to connect to the child node.     *
// * next:   Pointer to the next linked_rt_edge in the linked list (allows     *
// *         you to get the next child of the current rt_node).                */
//
//
//
//struct s_rg_node
//{
//	union{
//    	t_linked_rg_edge *parentlist;
//        t_linked_rg_edge *child_list;
//        struct s_rg_node *next;
//    }u;
//    int no_parents;
//    short re_expand;
//    int inode;
//    float C_downstream;
//    float R_upstream;
//    float Tdel;
//};
//
//typedef struct s_rg_node t_rg_node;

/* Structure describing one node in a routing tree (used to get net delays   *
 * incrementally during routing, as pieces are being added).                 *
 * u.child_list:  Pointer to a linked list of linked_rt_edge.  Each one of   *
 *                the linked list entries gives a child of this node.        *
 * u.next:  Used only when this node is on the free list.  Gives the next    *
 *          node on the free list.                                           *
 * parent_node:  Pointer to the rt_node that is this node's parent (used to  *
 *               make bottom to top traversals).                             *
 * re_expand:  (really boolean).  Should this node be put on the heap as     *
 *             part of the partial routing to act as a source for subsequent *
 *             connections?  TRUE->yes, FALSE-> no.                          *
 * parent_switch:  Index of the switch type driving this node (by its        *
 *                 parent).                                                  *
 * inode:  index (ID) of the rr_node that corresponds to this rt_node.       *
 * C_downstream:  Total downstream capacitance from this rt_node.  That is,  *
 *                the total C of the subtree rooted at the current node,     *
 *                including the C of the current node.                       *
 * R_upstream:  Total upstream resistance from this rt_node to the net       *
 *              source, including any rr_node[].R of this node.              *
 * Tdel:  Time delay for the signal to get from the net source to this node. *
 *        Includes the time to go through this node.                         */



/**************** Subroutines exported by route_tree_timing.c ***************/

void alloc_route_graph_timing_structs(void);

void free_route_graph_timing_structs(void);

t_rg_node *init_graph_to_source(int inet, t_rg_node** rg_sinks, t_rr_to_rg_node_hash_map* node_map);

void free_rg(t_rg_node * rg_node);

t_rg_node *update_route_graph(int icon, t_rg_node *root, t_rr_to_rg_node_hash_map* node_map, float* net_delay);

void update_net_delays_from_route_graph(float *net_delay,
				       t_rg_node ** rg_node_of_sink,
				       int inet);
float get_C_downstream(int inode);

void load_rt_subtree(t_rg_node * subtree_rg_root);

void remove_connection_from_route_timing_graph(int icon, t_rg_node *root, t_rg_node *sink_rg_node);

void print_and_reset_nr_of_stumbles();

void print_rg(t_rg_node * node, int level);
