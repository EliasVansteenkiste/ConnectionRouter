/**
 * Author: Jason Luu
 * Date: May 2009
 * 
 * Read a circuit netlist in XML format and populate the netlist data structures for VPR
 */

#include <cstdio>
#include <cstring>
using namespace std;

#include <assert.h>

#include "util.h"
#include "hash.h"
#include "vpr_types.h"
#include "vpr_utils.h"
#include "ReadLine.h"
#include "globals.h"
#include "ezxml.h"
#include "read_xml_util.h"
#include "read_netlist.h"
#include "pb_type_graph.h"
#include "token.h"
#include "netlist.h"

static const char* netlist_file_name = NULL;

static void processPorts(INOUTP ezxml_t Parent, INOUTP t_pb* pb, INOUTP t_pb_route *pb_route,		
		INP struct s_hash **vpack_net_hash);

static void processPb(INOUTP ezxml_t Parent, INOUTP t_block *cb, INP int index,
		INOUTP t_pb* pb, INOUTP t_pb_route *pb_route, INOUTP int *num_primitives,
		INP struct s_hash **vpack_net_hash,
		INP struct s_hash **logical_block_hash);

static void processComplexBlock(INOUTP ezxml_t Parent, INOUTP t_block *cb,
		INP int index, INOUTP int *num_primitives, INP const t_arch *arch,
		INP struct s_hash **vpack_net_hash,
		INP struct s_hash **logical_block_hash);
static struct s_net *alloc_and_init_netlist_from_hash(INP int ncount,
		INOUTP struct s_hash **nhash);

static int add_net_to_hash(INOUTP struct s_hash **nhash, INP char *net_name,
		INOUTP int *ncount);

static void load_external_nets_and_cb(INP int L_num_blocks,
		INP struct s_block block_list[], INP int ncount,
		INP struct s_net nlist[], OUTP int *ext_ncount,
		OUTP struct s_net **ext_nets, INP char **circuit_clocks);

static void load_interal_to_block_net_nums(INP t_type_ptr type, INOUTP t_pb_route *pb_route);

static void load_atom_index_for_pb_pin(t_pb_route *pb_route, int ipin);

static void mark_constant_generators(INP int L_num_blocks,
		INP struct s_block block_list[], INP int ncount,
		INOUTP struct s_net nlist[]);

static void mark_constant_generators_rec(INP t_pb *pb, INP t_pb_route *pb_route, INOUTP struct s_net nlist[]);

static t_pb_route *alloc_pb_route(t_pb_graph_node *pb_graph_node);

void readCon(ezxml_t *Cur,char **source,char **sink,char **condition)
{
	ezxml_t Con;
	char **sourceTemp,**sinkTemp,**conditionTemp;

	Con = FindElement(*Cur, "source", true);
	sourceTemp = GetNodeTokens(Con);
	*source = my_strdup(*sourceTemp);
	FreeNode(Con);
	printf("source: %s\n",source[0]);
	Con = FindElement(*Cur, "sink", true);
	sinkTemp = GetNodeTokens(Con);
	*sink = my_strdup(*sinkTemp);
	FreeNode(Con);
	printf("sink: %s\n",*sink);
	Con = FindElement(*Cur, "condition", false);
	if(Con != NULL)
	{
		conditionTemp = GetNodeTokens(Con);
		*condition = my_strdup(*conditionTemp);
		FreeNode(Con);
		printf("condition: %s\n",*condition);
	}

}

/**
 * Initializes the block_list with info from a netlist 
 * net_file - Name of the netlist file to read
 * num_blocks - number of CLBs in netlist 
 * block_list - array of blocks in netlist [0..num_blocks - 1]
 * num_nets - number of nets in netlist
 * net_list - nets in netlist [0..num_nets - 1]
 */
void read_netlist(INP const char *net_file, INP const t_arch *arch,
		OUTP int *L_num_blocks, OUTP struct s_block *block_list[],
		OUTP int *L_num_nets, OUTP struct s_net *net_list[]) {
	ezxml_t Cur, Prev, Top;
	int i;
	const char *Prop;
	int bcount;
	struct s_block *blist;
	int ext_ncount;
	struct s_net *ext_nlist;
	struct s_hash **vpack_net_hash, **logical_block_hash, *temp_hash;
	char **circuit_inputs, **circuit_outputs, **circuit_clocks;
	int Count, Len;

	int num_primitives = 0;

	/* Parse the file */
	vpr_printf_info("Begin parsing packed FPGA netlist file.\n");
	Top = ezxml_parse_file(net_file);
	if (NULL == Top) {
		vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
				"Unable to load netlist file '%s'.\n", net_file);
	}
	vpr_printf_info("Finished parsing packed FPGA netlist file.\n");

	/* Save netlist file's name in file-scoped variable */
	netlist_file_name = net_file;

	/* Root node should be block */
	CheckElement(Top, "block");

	/* Check top-level netlist attributes */
	Prop = FindProperty(Top, "name", true);
	vpr_printf_info("Netlist generated from file '%s'.\n", Prop);
	ezxml_set_attr(Top, "name", NULL);

	Prop = FindProperty(Top, "instance", true);
	if (strcmp(Prop, "FPGA_packed_netlist[0]") != 0) {
		vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Top->line,
				"Expected instance to be \"FPGA_packed_netlist[0]\", found %s.",
				Prop);
	}
	ezxml_set_attr(Top, "instance", NULL);

	/* Parse top-level netlist I/Os */
	Cur = FindElement(Top, "inputs", true);
	circuit_inputs = GetNodeTokens(Cur);
	FreeNode(Cur);
	Cur = FindElement(Top, "outputs", true);
	circuit_outputs = GetNodeTokens(Cur);
	FreeNode(Cur);

	Cur = FindElement(Top, "clocks", true);
	CountTokensInString(Cur->txt, &Count, &Len);
	if (Count > 0) {
		circuit_clocks = GetNodeTokens(Cur);
	} else {
		circuit_clocks = NULL;
	}
	FreeNode(Cur);

	/* Parse all CLB blocks and all nets*/
	bcount = CountChildren(Top, "block", 1);
	blist = (struct s_block *) my_calloc(bcount, sizeof(t_block));

	/* create quick hash look up for vpack_net and logical_block 
	 Also reset logical block data structure for pb
	 */
	vpack_net_hash = alloc_hash_table();
	logical_block_hash = alloc_hash_table();
	/* Imports the input and output nets on the hash table */
	for (i = 0; i < num_logical_nets; i++) {
		temp_hash = insert_in_hash_table(vpack_net_hash, vpack_net[i].name, i);
		assert(temp_hash->count == 1);
	}

	/* Imports the names of the available logic blocks into the hash table */
	/* logical_block is the data structure that holds all the blocks. It is GLOBAL */
	for (i = 0; i < num_logical_blocks; i++) {
		temp_hash = insert_in_hash_table(logical_block_hash,
				logical_block[i].name, i);
		logical_block[i].pb = NULL;
		if(temp_hash->count != 1) {
			vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
					"Found duplicate block in netlist file named: %s.\n",
					logical_block[i].name);
        }
	}

	/* Process netlist */

	Cur = Top->child;
	i = 0;
	while (Cur) {
		if (0 == strcmp(Cur->name, "block")) {
			CheckElement(Cur, "block");
		/*  v_pack_net_hash: hash table of the nets
			logical_block_hash: hash table of the blocks */
			processComplexBlock(Cur, blist, i, &num_primitives,	arch, vpack_net_hash, logical_block_hash);
			Prev = Cur;
			Cur = Cur->next;
			FreeNode(Prev);
			i++;
		} else {
			Cur = Cur->next;
		}
	}
	assert(i == bcount);
	assert(num_primitives == num_logical_blocks);
/*
	int conCount;
	t_con* connections;
	conCount = CountChildren(Top, "con", 1);
	connections = (t_con *) my_calloc(bcount, sizeof(t_con));

	 Process netlist in order to read the <con>*/
	/* TODO: We have to include error control that will check if there is a
	 * connection between two sinks or two sources

	Cur = Top->child;
	i = 0;
	while (Cur) {
		if(0 == strcmp(Cur->name, "con")){
			CheckElement(Cur, "con");
			readCon(&Cur,&connections[i].source,&connections[i].sink,&connections[i].condition);
			printf("%s %s\n",connections[i].source,connections[i].sink);
			if(connections[i].source == NULL || connections[i].sink == NULL)	{
				vpr_printf(TIO_MESSAGE_ERROR, "reading <con> type failed. Missing either source, sink or both\n");
				break;
			}
			Prev = Cur;
			Cur = Cur->next;
			FreeNode(Prev);
			i++;
		} else {
			Cur = Cur->next;
		}
	}
*/


	/* Error check */
	for (i = 0; i < num_logical_blocks; i++) {
		if (logical_block[i].pb == NULL) {
			vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
					".blif file and .net file do not match, .net file missing atom %s.\n",
					logical_block[i].name);
		}
	}
	/* TODO: Add additional check to make sure net connections match */
	/* vpack_net is the data structure that holds all the information about the nets. It is GLOBAL */
	/* blist is a clustered unit block */
	mark_constant_generators(bcount, blist, num_logical_nets, vpack_net);
	load_external_nets_and_cb(bcount, blist, num_logical_nets, vpack_net,
			&ext_ncount, &ext_nlist, circuit_clocks);

	/* TODO: create this function later
	 check_top_IO_matches_IO_blocks(circuit_inputs, circuit_outputs, circuit_clocks, blist, bcount);
	 */

	FreeTokens(&circuit_inputs);
	FreeTokens(&circuit_outputs);
	if (circuit_clocks)
		FreeTokens(&circuit_clocks);
	FreeNode(Top);

	/* load mapping between external nets and all nets */
	/* jluu TODO: Should use local variables here then assign to globals later, clean up later */
	clb_to_vpack_net_mapping = (int *) my_malloc(ext_ncount * sizeof(int));
	vpack_to_clb_net_mapping = (int *) my_malloc(
			num_logical_nets * sizeof(int));
	for (i = 0; i < num_logical_nets; i++) {
		vpack_to_clb_net_mapping[i] = OPEN;
	}

	for (i = 0; i < ext_ncount; i++) {
		temp_hash = get_hash_entry(vpack_net_hash, ext_nlist[i].name);
		assert(temp_hash != NULL);
		clb_to_vpack_net_mapping[i] = temp_hash->index;
		vpack_to_clb_net_mapping[temp_hash->index] = i;
	}

	/* Return blocks and nets */
	*L_num_blocks = bcount;
	*block_list = blist;
	*L_num_nets = ext_ncount;
	*net_list = ext_nlist;

	//Added August 2013, Daniel Chen for loading post-pack netlist into new data structures
	load_global_net_from_array(ext_nlist, ext_ncount, &g_clbs_nlist);
	//echo_global_nlist_net(&g_clbs_nlist, ext_nlist);

	free_hash_table(logical_block_hash);
	free_hash_table(vpack_net_hash);
}

/**
 * XML parser to populate CLB info and to update nets with the nets of this CLB 
 * Parent - XML tag for this CLB
 * clb - Array of CLBs in the netlist
 * index - index of the CLB to allocate and load information into
 * vpack_net_hash - hashtable of all nets in blif netlist
 * logical_block_hash - hashtable of all atoms in blif netlist
 */
static void processComplexBlock(INOUTP ezxml_t Parent, INOUTP t_block *cb,
		INP int index, INOUTP int *num_primitives, INP const t_arch *arch,
		INP struct s_hash **vpack_net_hash,
		INP struct s_hash **logical_block_hash) {

	const char *Prop;
	bool found;
	int num_tokens = 0;
	t_token *tokens;
	int i;
	const t_pb_type * pb_type = NULL;

	/* parse cb attributes */
	cb[index].pb = (t_pb*) my_calloc(1, sizeof(t_pb));

	Prop = FindProperty(Parent, "name", true);
	cb[index].name = my_strdup(Prop);
	cb[index].pb->name = my_strdup(Prop);
	ezxml_set_attr(Parent, "name", NULL);

	Prop = FindProperty(Parent, "instance", true);
	tokens = GetTokensFromString(Prop, &num_tokens);
	ezxml_set_attr(Parent, "instance", NULL);
	if (num_tokens != 4 || tokens[0].type != TOKEN_STRING
			|| tokens[1].type != TOKEN_OPEN_SQUARE_BRACKET
			|| tokens[2].type != TOKEN_INT
			|| tokens[3].type != TOKEN_CLOSE_SQUARE_BRACKET) {
		vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Parent->line,
				"Unknown syntax for instance %s in %s. Expected pb_type[instance_number].\n",
				Prop, Parent->name);
	}
	assert(my_atoi(tokens[2].data) == index);
	found = false;
	for (i = 0; i < num_types; i++) {
		if (strcmp(type_descriptors[i].name, tokens[0].data) == 0) {
			cb[index].type = &type_descriptors[i];
			pb_type = cb[index].type->pb_type;
			found = true;
			break;
		}
	}
	if (!found) {
		vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Parent->line,
				"Unknown cb type %s for cb %s #%d.\n", Prop, cb[index].name,
				index);
	}

	/* Parse all pbs and CB internal nets*/
	cb[index].pb->logical_block = OPEN;
	cb[index].pb->pb_graph_node = cb[index].type->pb_graph_head;
	cb[index].pb_route = alloc_pb_route(cb[index].pb->pb_graph_node);
	
	Prop = FindProperty(Parent, "mode", true);
	ezxml_set_attr(Parent, "mode", NULL);

	found = false;
	for (i = 0; i < pb_type->num_modes; i++) {
		if (strcmp(Prop, pb_type->modes[i].name) == 0) {
			cb[index].pb->mode = i;
			found = true;
		}
	}
	if (!found) {
		vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Parent->line,
				"Unknown mode %s for cb %s #%d.\n", Prop, cb[index].name,
				index);
	}

	processPb(Parent, cb, index, cb[index].pb, cb[index].pb_route, num_primitives, vpack_net_hash,
			logical_block_hash);

	cb[index].nets = (int *) my_malloc(cb[index].type->num_pins * sizeof(int));
	for (i = 0; i < cb[index].type->num_pins; i++) {
		cb[index].nets[i] = OPEN;
	}
	load_interal_to_block_net_nums(cb[index].type, cb[index].pb_route);
	freeTokens(tokens, num_tokens);
}

/**
 * XML parser to populate pb info and to update internal nets of the parent CLB
 * Parent - XML tag for this pb_type
 * pb - physical block to use
 * vpack_net_hash - hashtable of original blif net names and indices
 * logical_block_hash - hashtable of original blif atom names and indices
 */
static void processPb(INOUTP ezxml_t Parent, INOUTP t_block *cb, INP int index,
	INOUTP t_pb* pb, INOUTP t_pb_route *pb_route, INOUTP int *num_primitives,
		INP struct s_hash **vpack_net_hash,
		INP struct s_hash **logical_block_hash) {
	ezxml_t Cur, Prev, lookahead;
	const char *Prop;
	const char *instance_type;
	int i, j, pb_index;
	bool found;
	const t_pb_type *pb_type;
	t_token *tokens;
	int num_tokens;
	struct s_hash *temp_hash;

	Cur = FindElement(Parent, "inputs", true);
	processPorts(Cur, pb, pb_route, vpack_net_hash);
	FreeNode(Cur);
	Cur = FindElement(Parent, "outputs", true);
	processPorts(Cur, pb, pb_route, vpack_net_hash);
	FreeNode(Cur);
	Cur = FindElement(Parent, "clocks", true);
	processPorts(Cur, pb, pb_route, vpack_net_hash);
	FreeNode(Cur);
	pb_type = pb->pb_graph_node->pb_type;
	if (pb_type->num_modes == 0) {
		temp_hash = get_hash_entry(logical_block_hash, pb->name);
		if (temp_hash == NULL) {
			vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
					".net file and .blif file do not match, encountered unknown primitive %s in .net file.\n",
					pb->name);
		}
		pb->logical_block = temp_hash->index;
		assert(logical_block[temp_hash->index].pb == NULL);
		logical_block[temp_hash->index].pb = pb;
		logical_block[temp_hash->index].clb_index = index;
		(*num_primitives)++;
	} else {
		/* process children of child if exists */

		pb->child_pbs = (t_pb **) my_calloc(
				pb_type->modes[pb->mode].num_pb_type_children, sizeof(t_pb*));
		for (i = 0; i < pb_type->modes[pb->mode].num_pb_type_children; i++) {
			pb->child_pbs[i] = (t_pb *) my_calloc(
					pb_type->modes[pb->mode].pb_type_children[i].num_pb,
					sizeof(t_pb));
			for (j = 0; j < pb_type->modes[pb->mode].pb_type_children[i].num_pb;
					j++) {
				pb->child_pbs[i][j].logical_block = OPEN;
			}
		}

		/* Populate info for each physical block  */
		Cur = Parent->child;
		while (Cur) {
			if (0 == strcmp(Cur->name, "block")) {
				CheckElement(Cur, "block");

				instance_type = FindProperty(Cur, "instance", true);
				tokens = GetTokensFromString(instance_type, &num_tokens);
				ezxml_set_attr(Cur, "instance", NULL);
				if (num_tokens != 4 || tokens[0].type != TOKEN_STRING
						|| tokens[1].type != TOKEN_OPEN_SQUARE_BRACKET
						|| tokens[2].type != TOKEN_INT
						|| tokens[3].type != TOKEN_CLOSE_SQUARE_BRACKET) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
							"Unknown syntax for instance %s in %s. Expected pb_type[instance_number].\n",
							instance_type, Cur->name);
				}

				found = false;
				pb_index = OPEN;
				for (i = 0; i < pb_type->modes[pb->mode].num_pb_type_children;
						i++) {
					if (strcmp(
							pb_type->modes[pb->mode].pb_type_children[i].name,
							tokens[0].data) == 0) {
						if (my_atoi(tokens[2].data)
								>= pb_type->modes[pb->mode].pb_type_children[i].num_pb) {
							vpr_throw(VPR_ERROR_NET_F, netlist_file_name,
									Cur->line,
									"Instance number exceeds # of pb available for instance %s in %s.\n",
									instance_type, Cur->name);
						}
						pb_index = my_atoi(tokens[2].data);
						if (pb->child_pbs[i][pb_index].pb_graph_node != NULL) {
							vpr_throw(VPR_ERROR_NET_F, netlist_file_name,
									Cur->line,
									"node is used by two different blocks %s and %s.\n",
									instance_type,
									pb->child_pbs[i][pb_index].name);
						}
						pb->child_pbs[i][pb_index].pb_graph_node =
								&pb->pb_graph_node->child_pb_graph_nodes[pb->mode][i][pb_index];
						found = true;
						break;
					}
				}
				if (!found) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
							"Unknown pb type %s.\n", instance_type);
				}

				Prop = FindProperty(Cur, "name", true);
				ezxml_set_attr(Cur, "name", NULL);
				if (0 != strcmp(Prop, "open")) {
					pb->child_pbs[i][pb_index].name = my_strdup(Prop);

					/* Parse all pbs and CB internal nets*/
					pb->child_pbs[i][pb_index].logical_block = OPEN;

					Prop = FindProperty(Cur, "mode", false);
					if (Prop) {
						ezxml_set_attr(Cur, "mode", NULL);
					}
					pb->child_pbs[i][pb_index].mode = 0;
					found = false;
					for (j = 0;
							j
									< pb->child_pbs[i][pb_index].pb_graph_node->pb_type->num_modes;
							j++) {
						if (strcmp(Prop,
								pb->child_pbs[i][pb_index].pb_graph_node->pb_type->modes[j].name)
								== 0) {
							pb->child_pbs[i][pb_index].mode = j;
							found = true;
						}
					}
					if (!found
							&& pb->child_pbs[i][pb_index].pb_graph_node->pb_type->num_modes
									!= 0) {
						vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
								"Unknown mode %s for cb %s #%d.\n", Prop,
								pb->child_pbs[i][pb_index].name, pb_index);
					}
					pb->child_pbs[i][pb_index].parent_pb = pb;

					processPb(Cur, cb, index, &pb->child_pbs[i][pb_index], pb_route,
							num_primitives,	vpack_net_hash, logical_block_hash);
				} else {
					/* physical block has no used primitives but it may have used routing */
					pb->child_pbs[i][pb_index].name = NULL;
					pb->child_pbs[i][pb_index].logical_block = OPEN;
					lookahead = FindElement(Cur, "outputs", false);
					if (lookahead != NULL) {
						lookahead = FindFirstElement(lookahead, "port", true);
						Prop = FindProperty(Cur, "mode", false);
						if (Prop) {
							ezxml_set_attr(Cur, "mode", NULL);
						}
						pb->child_pbs[i][pb_index].mode = 0;
						found = false;
						for (j = 0;
								j
										< pb->child_pbs[i][pb_index].pb_graph_node->pb_type->num_modes;
								j++) {
							if (strcmp(Prop,
									pb->child_pbs[i][pb_index].pb_graph_node->pb_type->modes[j].name)
									== 0) {
								pb->child_pbs[i][pb_index].mode = j;
								found = true;
							}
						}
						if (!found
								&& pb->child_pbs[i][pb_index].pb_graph_node->pb_type->num_modes
										!= 0) {
							vpr_throw(VPR_ERROR_NET_F, netlist_file_name,
									Cur->line,
									"Unknown mode %s for cb %s #%d.\n", Prop,
									pb->child_pbs[i][pb_index].name, pb_index);
						}
						pb->child_pbs[i][pb_index].parent_pb = pb;
						processPb(Cur, cb, index, &pb->child_pbs[i][pb_index], pb_route,
								num_primitives, vpack_net_hash, logical_block_hash);
					}
				}
				Prev = Cur;
				Cur = Cur->next;
				FreeNode(Prev);
				freeTokens(tokens, num_tokens);
			} else {
				Cur = Cur->next;
			}
		}
	}
}

/**
 * Allocates memory for nets and loads the name of the net so that it can be identified and loaded with
 * more complete information later
 * ncount - number of nets in the hashtable of nets
 * nhash - hashtable of nets
 * returns array of nets stored in hashtable
 */
static struct s_net *alloc_and_init_netlist_from_hash(INP int ncount,
		INOUTP struct s_hash **nhash) {
	struct s_net *nlist;
	struct s_hash_iterator hash_iter;
	struct s_hash *curr_net;
	int i;

	nlist = (struct s_net *) my_calloc(ncount, sizeof(struct s_net));

	hash_iter = start_hash_table_iterator();
	curr_net = get_next_hash(nhash, &hash_iter);
	while (curr_net != NULL) {
		assert(nlist[curr_net->index].name == NULL);
		nlist[curr_net->index].name = my_strdup(curr_net->name);
		nlist[curr_net->index].num_sinks = curr_net->count - 1;

		nlist[curr_net->index].node_block = (int *) my_malloc(
				curr_net->count * sizeof(int));
		nlist[curr_net->index].node_block_pin = (int *) my_malloc(
				curr_net->count * sizeof(int));
		nlist[curr_net->index].is_routed = false;
		nlist[curr_net->index].is_fixed = false;
		nlist[curr_net->index].is_global = false;
		for (i = 0; i < curr_net->count; i++) {
			nlist[curr_net->index].node_block[i] = OPEN;
			nlist[curr_net->index].node_block_pin[i] = OPEN;
		}
		curr_net = get_next_hash(nhash, &hash_iter);
	}
	return nlist;
}

/**
 * Adds net to hashtable of nets.  If the net is "open", then this is a keyword so do not add it.  
 * If the net already exists, increase the count on that net 
 */
static int add_net_to_hash(INOUTP struct s_hash **nhash, INP char *net_name,
		INOUTP int *ncount) {
	struct s_hash *hash_value;

	if (strcmp(net_name, "open") == 0) {
		return OPEN;
	}

	hash_value = insert_in_hash_table(nhash, net_name, *ncount);
	if (hash_value->count == 1) {
		assert(*ncount == hash_value->index);
		(*ncount)++;
	}
	return hash_value->index;
}

/* Function countPorts:
 * 	This function parses the <port> blocks in the input,output and global category in order to count
 * 	the number of ports in each category.
 */
void countPorts(ezxml_t Cur,const char **Prop,t_pb* pb,int *in_port,int *out_port,int *clock_port)
{
	int i, found;

	*Prop = FindProperty(Cur, "name", true);
	ezxml_set_attr(Cur, "name", NULL);

	*in_port = *out_port = *clock_port = 0;
	found = false;

	for (i = 0; i < pb->pb_graph_node->pb_type->num_ports; i++) {
		if (0 == strcmp(pb->pb_graph_node->pb_type->ports[i].name,*Prop)) {
			found = true;
			break;
		}
		if (pb->pb_graph_node->pb_type->ports[i].is_clock
				&& pb->pb_graph_node->pb_type->ports[i].type
						== IN_PORT) {
			(*clock_port)++;
		} else if (!pb->pb_graph_node->pb_type->ports[i].is_clock
				&& pb->pb_graph_node->pb_type->ports[i].type
						== IN_PORT) {
			(*in_port)++;
		} else {
			assert(
					pb->pb_graph_node->pb_type->ports[i].type == OUT_PORT);
			(*out_port)++;
		}
	}
	if (!found) {
		vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
			"Unknown port %s for pb %s[%d].\n", Prop,
			pb->pb_graph_node->pb_type->name,
			pb->pb_graph_node->placement_index);
	}
}

/* Function errorCheckPins:
 * 	This function does an error checking on the connections that reach and leave a block so
 * 	to be sure that they do not exceed the available number of pins
 */
void errorCheckPins(const char *Prop, ezxml_t Cur, char **pins, int num_tokens, ezxml_t Parent, t_pb* pb, int in_port, int out_port,int clock_port)
{

	if (0 == strcmp(Parent->name, "inputs")) {
				if (num_tokens != pb->pb_graph_node->num_input_pins[in_port]) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
							"Incorrect # pins %d found for port %s for pb %s[%d].\n",
							num_tokens, Prop, pb->pb_graph_node->pb_type->name,
							pb->pb_graph_node->placement_index);
				}
	} else if (0 == strcmp(Parent->name, "outputs")) {
				if (num_tokens
						!= pb->pb_graph_node->num_output_pins[out_port]) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
							"Incorrect # pins %d found for port %s for pb %s[%d].\n",
							num_tokens, Prop, pb->pb_graph_node->pb_type->name,
							pb->pb_graph_node->placement_index);
				}
	} else {
		if (num_tokens
				!= pb->pb_graph_node->num_clock_pins[clock_port]) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
							"Incorrect # pins %d found for port %s for pb %s[%d].\n",
							num_tokens, Prop, pb->pb_graph_node->pb_type->name,
							pb->pb_graph_node->placement_index);
		}
	}
}

void place_connections_ports(ezxml_t Cur, INOUTP ezxml_t Parent, INOUTP t_pb* pb, int num_tokens, int in_port, int out_port,int clock_port,
						char **pins,INP struct s_hash **vpack_net_hash, t_pb_route *pb_route)
{
	int i,j;
	int rr_node_index;
	struct s_hash *temp_hash;
	char *port_name, *interconnect_name;
	int *num_ptrs, num_sets;
	t_pb_graph_pin *** pin_node;
	bool found;

	if (0 == strcmp(Parent->name, "inputs")
			|| 0 == strcmp(Parent->name, "clocks")) {
		if (pb->parent_pb == NULL) {
			/* top-level, connections are nets to route */
			for (i = 0; i < num_tokens; i++) {
				if (0 == strcmp(Parent->name, "inputs")){
					rr_node_index =
							pb->pb_graph_node->input_pins[in_port][i].pin_count_in_cluster;
				}else
					rr_node_index =
							pb->pb_graph_node->clock_pins[clock_port][i].pin_count_in_cluster;
				if (strcmp(pins[i], "open") != 0) {
					temp_hash = get_hash_entry(vpack_net_hash, pins[i]);
					if (temp_hash == NULL) {
						vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
										".blif and .net do not match, unknown net %s found in .net file.\n.",
										pins[i]);
							}
					/*rr_graph[rr_node_index].net_num = temp_hash->index;*/
				}
				/*rr_node_to_pb_mapping[rr_node_index] = pb;*/
			}
		} else {
			for (i = 0; i < num_tokens; i++) {
				if (0 == strcmp(pins[i], "open")) {
					continue;
				}
				interconnect_name = strstr(pins[i], "->");
				*interconnect_name = '\0';
				interconnect_name += 2;
				port_name = pins[i];
				pin_node =
						alloc_and_load_port_pin_ptrs_from_string(
								pb->pb_graph_node->pb_type->parent_mode->interconnect[0].line_num,
								pb->pb_graph_node->parent_pb_graph_node,
								pb->pb_graph_node->parent_pb_graph_node->child_pb_graph_nodes[pb->parent_pb->mode],
								port_name, &num_ptrs, &num_sets, true,
								true);
				assert(num_sets == 1 && num_ptrs[0] == 1);
				if (0 == strcmp(Parent->name, "inputs"))
					rr_node_index =
							pb->pb_graph_node->input_pins[in_port][i].pin_count_in_cluster;
				else
					rr_node_index =
							pb->pb_graph_node->clock_pins[clock_port][i].pin_count_in_cluster;
				pb_route[rr_node_index].prev_pb_pin_id =
						pin_node[0][0]->pin_count_in_cluster;
				//rr_node_to_pb_mapping[rr_node_index] = pb;
				found = false;
				for (j = 0; j < pin_node[0][0]->num_output_edges; j++) {
					if (0
							== strcmp(interconnect_name,
									pin_node[0][0]->output_edges[j]->interconnect->name)) {
						found = true;
						break;
					}
				}
				for (j = 0; j < num_sets; j++) {
					free(pin_node[j]);
				}
				free(pin_node);
				free(num_ptrs);
				if (!found) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name,
								Cur->line,
								"Unknown interconnect %s connecting to pin %s.\n",
								interconnect_name, port_name);
				}
			}
		}
	}

	if (0 == strcmp(Parent->name, "outputs")) {
		if (pb->pb_graph_node->pb_type->num_modes == 0) {
			/* primitives are drivers of nets */
			for (i = 0; i < num_tokens; i++) {
				rr_node_index =
						pb->pb_graph_node->output_pins[out_port][i].pin_count_in_cluster;
				if (strcmp(pins[i], "open") != 0) {
					temp_hash = get_hash_entry(vpack_net_hash, pins[i]);
							if (temp_hash == NULL) {
								vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
										".blif and .net do not match, unknown net %s found in .net file.\n",
										pins[i]);
							}
					pb_route[rr_node_index].atom_net_idx = temp_hash->index;
				}
				//rr_node_to_pb_mapping[rr_node_index] = pb;
			}
		} else {
			for (i = 0; i < num_tokens; i++) {
				if (0 == strcmp(pins[i], "open")) {
					continue;
				}
				interconnect_name = strstr(pins[i], "->");
				*interconnect_name = '\0';
				interconnect_name += 2;
				port_name = pins[i];
				pin_node =
						alloc_and_load_port_pin_ptrs_from_string(
								pb->pb_graph_node->pb_type->modes[pb->mode].interconnect->line_num,
								pb->pb_graph_node,
								pb->pb_graph_node->child_pb_graph_nodes[pb->mode],
								port_name, &num_ptrs, &num_sets, true,
								true);
				assert(num_sets == 1 && num_ptrs[0] == 1);
				rr_node_index =
						pb->pb_graph_node->output_pins[out_port][i].pin_count_in_cluster;
				pb_route[rr_node_index].atom_net_idx =
						pin_node[0][0]->pin_count_in_cluster;
				pb_route[rr_node_index].prev_pb_pin_id = pin_node[0][0]->pin_count_in_cluster;
				found = false;
				for (j = 0; j < pin_node[0][0]->num_output_edges; j++) {
					if (0
							== strcmp(interconnect_name,
									pin_node[0][0]->output_edges[j]->interconnect->name)) {
						found = true;
						break;
					}
				}
				for (j = 0; j < num_sets; j++) {
					free(pin_node[j]);
				}
				free(pin_node);
				free(num_ptrs);
				if (!found) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name,
								Cur->line,
								"Unknown interconnect %s connecting to pin %s.\n",
								interconnect_name, port_name);
				}
				interconnect_name -= 2;
				*interconnect_name = '-';
			}
		}
	}

}

//static void processPorts(INOUTP ezxml_t Parent, INOUTP t_pb* pb, INOUTP t_pb_route *pb_route,
//		INP struct s_hash **vpack_net_hash) {
//
//	int in_port, out_port, clock_port, num_tokens;
//	ezxml_t Cur, Prev;
//	const char *Prop;
//	char **pins;
//	//char *interconnect_name;
//
//	Cur = Parent->child;
//	while (Cur) {
//		if (0 == strcmp(Cur->name, "port")) {
//			CheckElement(Cur, "port");
//
//			pins = GetNodeTokens(Cur);
//			num_tokens = CountTokens(pins);
//			errorCheckPins(Prop,Cur, pins,num_tokens, Parent,pb, in_port,  out_port, clock_port);
//
//			place_connections_ports(Cur, Parent, pb, num_tokens, in_port, out_port,clock_port, pins, vpack_net_hash, pb_route);
//
//			FreeTokens(&pins);
//
//			Prev = Cur;
//			Cur = Cur->next;
//			FreeNode(Prev);
//		} else {
//			Cur = Cur->next;
//		}
//	}
//}

static void processPorts(INOUTP ezxml_t Parent, INOUTP t_pb* pb, INOUTP t_pb_route *pb_route,
		INP struct s_hash **vpack_net_hash) {

	int i, j, in_port, out_port, clock_port, num_tokens;
	ezxml_t Cur, Prev;
	const char *Prop;
	char **pins;
	char *port_name, *interconnect_name;
	int rr_node_index;
	t_pb_graph_pin *** pin_node;
	int *num_ptrs, num_sets;
	struct s_hash *temp_hash;
	bool found;

	Cur = Parent->child;
	while (Cur) {
		if (0 == strcmp(Cur->name, "port")) {
			CheckElement(Cur, "port");

			Prop = FindProperty(Cur, "name", true);
			ezxml_set_attr(Cur, "name", NULL);

			in_port = out_port = clock_port = 0;
			found = false;
			for (i = 0; i < pb->pb_graph_node->pb_type->num_ports; i++) {
				if (0
						== strcmp(pb->pb_graph_node->pb_type->ports[i].name,
								Prop)) {
					found = true;
					break;
				}
				if (pb->pb_graph_node->pb_type->ports[i].is_clock
						&& pb->pb_graph_node->pb_type->ports[i].type
								== IN_PORT) {
					clock_port++;
				} else if (!pb->pb_graph_node->pb_type->ports[i].is_clock
						&& pb->pb_graph_node->pb_type->ports[i].type
								== IN_PORT) {
					in_port++;
				} else {
					assert(
							pb->pb_graph_node->pb_type->ports[i].type
									== OUT_PORT);
					out_port++;
				}
			}
			if (!found) {
				vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
						"Unknown port %s for pb %s[%d].\n", Prop,
						pb->pb_graph_node->pb_type->name,
						pb->pb_graph_node->placement_index);
			}

			pins = GetNodeTokens(Cur);
			num_tokens = CountTokens(pins);
			if (0 == strcmp(Parent->name, "inputs")) {
				if (num_tokens != pb->pb_graph_node->num_input_pins[in_port]) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
							"Incorrect # pins %d found for port %s for pb %s[%d].\n",
							num_tokens, Prop, pb->pb_graph_node->pb_type->name,
							pb->pb_graph_node->placement_index);
				}
			} else if (0 == strcmp(Parent->name, "outputs")) {
				if (num_tokens
						!= pb->pb_graph_node->num_output_pins[out_port]) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
							"Incorrect # pins %d found for port %s for pb %s[%d].\n",
							num_tokens, Prop, pb->pb_graph_node->pb_type->name,
							pb->pb_graph_node->placement_index);
				}
			} else {
				if (num_tokens
						!= pb->pb_graph_node->num_clock_pins[clock_port]) {
					vpr_throw(VPR_ERROR_NET_F, netlist_file_name, Cur->line,
							"Incorrect # pins %d found for port %s for pb %s[%d].\n",
							num_tokens, Prop, pb->pb_graph_node->pb_type->name,
							pb->pb_graph_node->placement_index);
				}
			}
			if (0 == strcmp(Parent->name, "inputs")
					|| 0 == strcmp(Parent->name, "clocks")) {
				if (pb->parent_pb == NULL) {
					/* top-level, connections are nets to route */
					for (i = 0; i < num_tokens; i++) {
						if (0 == strcmp(Parent->name, "inputs"))
							rr_node_index =
									pb->pb_graph_node->input_pins[in_port][i].pin_count_in_cluster;
						else
							rr_node_index =
									pb->pb_graph_node->clock_pins[clock_port][i].pin_count_in_cluster;
						if (strcmp(pins[i], "open") != 0) {
							temp_hash = get_hash_entry(vpack_net_hash, pins[i]);
							if (temp_hash == NULL) {
								vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
										".blif and .net do not match, unknown net %s found in .net file.\n.",
										pins[i]);
							}
							pb_route[rr_node_index].atom_net_idx = temp_hash->index;
						}						
					}
				} else {
					for (i = 0; i < num_tokens; i++) {
						if (0 == strcmp(pins[i], "open")) {
							continue;
						}
						interconnect_name = strstr(pins[i], "->");
						*interconnect_name = '\0';
						interconnect_name += 2;
						port_name = pins[i];
						pin_node =
								alloc_and_load_port_pin_ptrs_from_string(
										pb->pb_graph_node->pb_type->parent_mode->interconnect[0].line_num,
										pb->pb_graph_node->parent_pb_graph_node,
										pb->pb_graph_node->parent_pb_graph_node->child_pb_graph_nodes[pb->parent_pb->mode],
										port_name, &num_ptrs, &num_sets, true,
										true);
						assert(num_sets == 1 && num_ptrs[0] == 1);
						if (0 == strcmp(Parent->name, "inputs"))
							rr_node_index =
									pb->pb_graph_node->input_pins[in_port][i].pin_count_in_cluster;
						else
							rr_node_index =
									pb->pb_graph_node->clock_pins[clock_port][i].pin_count_in_cluster;
						pb_route[rr_node_index].prev_pb_pin_id = pin_node[0][0]->pin_count_in_cluster;
						found = false;
						for (j = 0; j < pin_node[0][0]->num_output_edges; j++) {
							if (0
									== strcmp(interconnect_name,
											pin_node[0][0]->output_edges[j]->interconnect->name)) {
								found = true;
								break;
							}
						}
						for (j = 0; j < num_sets; j++) {
							free(pin_node[j]);
						}
						free(pin_node);
						free(num_ptrs);
						if (!found) {
							vpr_throw(VPR_ERROR_NET_F, netlist_file_name,
									Cur->line,
									"Unknown interconnect %s connecting to pin %s.\n",
									interconnect_name, port_name);
						}
					}
				}
			}

			if (0 == strcmp(Parent->name, "outputs")) {
				if (pb->pb_graph_node->pb_type->num_modes == 0) {
					/* primitives are drivers of nets */
					for (i = 0; i < num_tokens; i++) {
						rr_node_index =
								pb->pb_graph_node->output_pins[out_port][i].pin_count_in_cluster;
						if (strcmp(pins[i], "open") != 0) {
							temp_hash = get_hash_entry(vpack_net_hash, pins[i]);
							if (temp_hash == NULL) {
								vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
										".blif and .net do not match, unknown net %s found in .net file.\n",
										pins[i]);
							}
							pb_route[rr_node_index].atom_net_idx = temp_hash->index;
						}
					}
				} else {
					for (i = 0; i < num_tokens; i++) {
						if (0 == strcmp(pins[i], "open")) {
							continue;
						}
						interconnect_name = strstr(pins[i], "->");
						*interconnect_name = '\0';
						interconnect_name += 2;
						port_name = pins[i];
						pin_node =
								alloc_and_load_port_pin_ptrs_from_string(
										pb->pb_graph_node->pb_type->modes[pb->mode].interconnect->line_num,
										pb->pb_graph_node,
										pb->pb_graph_node->child_pb_graph_nodes[pb->mode],
										port_name, &num_ptrs, &num_sets, true,
										true);
						assert(num_sets == 1 && num_ptrs[0] == 1);
						rr_node_index =
								pb->pb_graph_node->output_pins[out_port][i].pin_count_in_cluster;
						pb_route[rr_node_index].prev_pb_pin_id = pin_node[0][0]->pin_count_in_cluster;
						found = false;
						for (j = 0; j < pin_node[0][0]->num_output_edges; j++) {
							if (0
									== strcmp(interconnect_name,
											pin_node[0][0]->output_edges[j]->interconnect->name)) {
								found = true;
								break;
							}
						}
						for (j = 0; j < num_sets; j++) {
							free(pin_node[j]);
						}
						free(pin_node);
						free(num_ptrs);
						if (!found) {
							vpr_throw(VPR_ERROR_NET_F, netlist_file_name,
									Cur->line,
									"Unknown interconnect %s connecting to pin %s.\n",
									interconnect_name, port_name);
						}
						interconnect_name -= 2;
						*interconnect_name = '-';
					}
				}
			}

			FreeTokens(&pins);

			Prev = Cur;
			Cur = Cur->next;
			FreeNode(Prev);
		} else {
			Cur = Cur->next;
		}
	}
}

void writeToDebugFile(char *info,int last)
{
	static FILE* file = NULL;
	static int open_file = 0;

	if(!open_file)
	{
		file = fopen("debug_file","w");
		if(file == NULL)
		{
			fprintf(stderr,"Failed to open the debug file\n");
		}
		open_file = 1;
	}
	fprintf(file,"%s",info);

	if(last)
	{
		if(fclose(file))
			printf("file did not close succesfuly\n");
	}

}

/**  
 * This function updates the nets list and the connections between that list and the complex block
 */
static void load_external_nets_and_cb(INP int L_num_blocks,
		INP struct s_block block_list[], INP int ncount,
		INP struct s_net nlist[], OUTP int *ext_ncount,
		OUTP struct s_net **ext_nets, INP char **circuit_clocks) {
	int i, j, k, ipin;
	struct s_hash **ext_nhash;
	t_pb_graph_pin *pb_graph_pin;
	int *count;
	int netnum, num_tokens;

	*ext_ncount = 0;
	ext_nhash = alloc_hash_table();

	/* Assumes that complex block pins are ordered inputs, outputs, globals */

	/* Determine the external nets of complex block */
	for (i = 0; i < L_num_blocks; i++) {
		ipin = 0;
		if (block_list[i].type->pb_type->num_input_pins
				+ block_list[i].type->pb_type->num_output_pins
				+ block_list[i].type->pb_type->num_clock_pins
				!= block_list[i].type->num_pins
						/ block_list[i].type->capacity) {

			assert(0);
		}

		/* First determine nets external to complex blocks */
		assert(
				block_list[i].type->pb_type->num_input_pins + block_list[i].type->pb_type->num_output_pins + block_list[i].type->pb_type->num_clock_pins == block_list[i].type->num_pins / block_list[i].type->capacity);
        char tempWriteBuffer[500]; /*->EV_DBG: When this number is too low it causes hard to debug memory errors, that only pop up at the end of this function, symptoms were sigabrt in xcode, abort trap 6 mentioning when running program, valgrind gives __stack_chk_fail ! TODO maybe change this static buffer to a dynamic one */

		/* we start by iterating through the complex blocks */
		for (j = 0; j < block_list[i].pb->pb_graph_node->num_input_ports; j++) { /* we run through all the ports */
			for (k = 0; k < block_list[i].pb->pb_graph_node->num_input_pins[j];k++) {  /* we also run through all the pins of the ports */

				sprintf(tempWriteBuffer,"Number of input ports: %d\n",block_list[i].pb->pb_graph_node->num_input_ports);
				writeToDebugFile(tempWriteBuffer,0);

				sprintf(tempWriteBuffer,"Number of input pins: %d\n",block_list[i].pb->pb_graph_node->num_input_pins[j]);
				writeToDebugFile(tempWriteBuffer,0);

				pb_graph_pin =
						&block_list[i].pb->pb_graph_node->input_pins[j][k]; /* We go through all the pins of all the blocks */
				assert(pb_graph_pin->pin_count_in_cluster == ipin);
				if (block_list[i].pb_route[pb_graph_pin->pin_count_in_cluster].atom_net_idx
						!= OPEN) {
					block_list[i].nets[ipin] =
							add_net_to_hash(ext_nhash,
									nlist[block_list[i].pb_route[pb_graph_pin->pin_count_in_cluster].atom_net_idx].name,
									ext_ncount);
				} else {
					block_list[i].nets[ipin] = OPEN;
				}
				/* It puts the nets in an order inside the nets[] structure */
				ipin++;
			}
		}
		for (j = 0; j < block_list[i].pb->pb_graph_node->num_output_ports;
				j++) {
			for (k = 0; k < block_list[i].pb->pb_graph_node->num_output_pins[j];
					k++) {
				pb_graph_pin =
						&block_list[i].pb->pb_graph_node->output_pins[j][k];
				assert(pb_graph_pin->pin_count_in_cluster == ipin);
				if (block_list[i].pb_route[pb_graph_pin->pin_count_in_cluster].atom_net_idx
						!= OPEN) {
					block_list[i].nets[ipin] =
							add_net_to_hash(ext_nhash,
							nlist[block_list[i].pb_route[pb_graph_pin->pin_count_in_cluster].atom_net_idx].name,
									ext_ncount);
				} else {
					block_list[i].nets[ipin] = OPEN;
				}
				ipin++;
			}
		}
		for (j = 0; j < block_list[i].pb->pb_graph_node->num_clock_ports; j++) {
			for (k = 0; k < block_list[i].pb->pb_graph_node->num_clock_pins[j];
					k++) {
				pb_graph_pin =
						&block_list[i].pb->pb_graph_node->clock_pins[j][k];
				assert(pb_graph_pin->pin_count_in_cluster == ipin);
				if (block_list[i].pb_route[pb_graph_pin->pin_count_in_cluster].atom_net_idx
						!= OPEN) {
					block_list[i].nets[ipin] =
							add_net_to_hash(ext_nhash,
								nlist[block_list[i].pb_route[pb_graph_pin->pin_count_in_cluster].atom_net_idx].name,
									ext_ncount);
				} else {
					block_list[i].nets[ipin] = OPEN;
				}
				ipin++;
			}
		}
		for (j = ipin; j < block_list[i].type->num_pins; j++) {
			block_list[i].nets[ipin] = OPEN;
		}
	}
	writeToDebugFile("\n",1);
	/* alloc and partially load the list of external nets */
	(*ext_nets) = alloc_and_init_netlist_from_hash(*ext_ncount, ext_nhash);
	/* Load global nets */
	num_tokens = CountTokens(circuit_clocks);

	count = (int *) my_calloc(*ext_ncount, sizeof(int));

	/* complete load of external nets so that each net points back to the blocks */
	for (i = 0; i < L_num_blocks; i++) {
		ipin = 0;
		for (j = 0; j < block_list[i].type->num_pins; j++) {
			netnum = block_list[i].nets[j]; /* the number of available nets (maybe)*/
			if (netnum != OPEN) {
				if (RECEIVER
						== block_list[i].type->class_inf[block_list[i].type->pin_class[j]].type) {
					count[netnum]++;
					if (count[netnum] > (*ext_nets)[netnum].num_sinks) {
						vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
								"net %s #%d inconsistency, expected %d terminals but encountered %d terminals, it is likely net terminal is disconnected in netlist file.\n",
								(*ext_nets)[netnum].name, netnum, count[netnum],
								(*ext_nets)[netnum].num_sinks);
					}

					(*ext_nets)[netnum].node_block[count[netnum]] = i;
					(*ext_nets)[netnum].node_block_pin[count[netnum]] = j;

					(*ext_nets)[netnum].is_global =
							block_list[i].type->is_global_pin[j]; /* Error check performed later to ensure no mixing of global and non-global signals */
				} else {
					assert(
							DRIVER
									== block_list[i].type->class_inf[block_list[i].type->pin_class[j]].type);
					assert((*ext_nets)[netnum].node_block[0] == OPEN);
					(*ext_nets)[netnum].node_block[0] = i;
					(*ext_nets)[netnum].node_block_pin[0] = j;
				}
			}
		}
	}
	/* Error check global and non global signals */
	for (i = 0; i < *ext_ncount; i++) {
		for (j = 1; j <= (*ext_nets)[i].num_sinks; j++) {
			bool is_global_net =
					static_cast<bool>((*ext_nets)[i].is_global);
			if (block_list[(*ext_nets)[i].node_block[j]].type->is_global_pin[(*ext_nets)[i].node_block_pin[j]]
					!= is_global_net) {
				vpr_throw(VPR_ERROR_NET_F, __FILE__, __LINE__,
						"Netlist attempts to connect net %s to both global and non-global pins.\n",
						(*ext_nets)[i].name);
			}
		}
		for (j = 0; j < num_tokens; j++) {
			if (strcmp(circuit_clocks[j], (*ext_nets)[i].name) == 0) {
				assert((*ext_nets)[i].is_global == true); /* above code should have caught this case, if not, then bug in code */
			}
		}
	}
	free(count);
	free_hash_table(ext_nhash);
}

/* Recursive function that fills rr_graph of cb with net numbers starting at the given rr_node */
static int count_sinks_internal_cb_rr_graph_net_nums(
		INP t_rr_node * cur_rr_node, INP t_rr_node * rr_graph) {
	int i;
	int count = 0;

	for (i = 0; i < cur_rr_node->get_num_edges(); i++) {
		if (&rr_graph[rr_graph[cur_rr_node->edges[i]].prev_node]
				== cur_rr_node) {
			assert(
					rr_graph[cur_rr_node->edges[i]].net_num == OPEN || rr_graph[cur_rr_node->edges[i]].net_num == cur_rr_node->net_num);
			count += count_sinks_internal_cb_rr_graph_net_nums(
					&rr_graph[cur_rr_node->edges[i]], rr_graph);
		}
	}
	if (count == 0) {
		return 1; /* terminal node */
	} else {
		return count;
	}
}
//
///* Recursive function that fills rr_graph of cb with net numbers starting at the given rr_node */
//static void load_internal_cb_rr_graph_net_nums(INP t_rr_node * cur_rr_node,
//		INP t_pb_route *pb_route, INOUTP struct s_net * nets,
//		INOUTP int * curr_net, INOUTP int * curr_sink) {
//	int i;
//
//	bool terminal;
//	terminal = true;
//
//	for (i = 0; i < cur_rr_node->get_num_edges(); i++) {
//		if (&pb_route[pb_route[cur_rr_node->edges[i]].prev_pb_pin_id]
//				== cur_rr_node) {
//			/* TODO: If multiple edges to same node (should not happen in reasonable design) this always
//			 selects the last edge, need to be smart about it in future (ie. select fastest edge */
//			assert(
//					pb_route[cur_rr_node->edges[i]].atom_net_idx == OPEN || pb_route[cur_rr_node->edges[i]].atom_net_idx == cur_rr_node->net_num);
//			pb_route[cur_rr_node->edges[i]].atom_net_idx = cur_rr_node->net_num;
//			pb_route[cur_rr_node->edges[i]].prev_pb_pin_id = i;
//			load_internal_cb_rr_graph_net_nums(&pb_route[cur_rr_node->edges[i]],
//					pb_route, nets, curr_net, curr_sink);
//			terminal = false;
//		}
//	}
//	if (terminal == true) {
//		/* Since the routing node index is known, assign that instead of the more obscure node block */
//		nets[*curr_net].node_block[*curr_sink] =
//				cur_rr_node->pb_graph_pin->pin_count_in_cluster;
//		nets[*curr_net].node_block_pin[*curr_sink] = OPEN;
//		nets[*curr_net].node_block_port[*curr_sink] = OPEN;
//		(*curr_sink)++;
//	}
//}
//
///* Load internal cb nets and fill rr_graph of cb with net numbers */
//static void load_internal_cb_nets(INOUTP t_pb *top_level,
//		INP t_pb_graph_node *pb_graph_node, INOUTP t_rr_node *rr_graph,
//		INOUTP int * curr_net) {
//	int i, j, k;
//	const t_pb_type *pb_type;
//	int temp, size;
//	struct s_net * nets;
//
//	pb_type = pb_graph_node->pb_type;
//
//	nets = top_level->local_nets;
//
//	temp = 0;
//
//	if (pb_graph_node->parent_pb_graph_node == NULL) { /* determine nets driven from inputs at top level */
//		*curr_net = 0;
//		for (i = 0; i < pb_graph_node->num_input_ports; i++) {
//			for (j = 0; j < pb_graph_node->num_input_pins[i]; j++) {
//				if (rr_graph[pb_graph_node->input_pins[i][j].pin_count_in_cluster].net_num
//						!= OPEN) {
//					load_internal_cb_rr_graph_net_nums(
//							&rr_graph[pb_graph_node->input_pins[i][j].pin_count_in_cluster],
//							rr_graph, nets, curr_net, &temp);
//					assert(temp == nets[*curr_net].num_sinks);
//					temp = 0;
//					size =
//							strlen(pb_graph_node->pb_type->name)
//									+ pb_graph_node->placement_index / 10
//									+ i / 10 + j / 10
//									+ pb_graph_node->input_pins[i][j].pin_count_in_cluster
//											/ 10 + 26;
//					nets[*curr_net].name = (char *)my_calloc(size, sizeof(char));
//					sprintf(nets[*curr_net].name,
//							"%s[%d].input[%d][%d].pin[%d]",
//							pb_graph_node->pb_type->name,
//							pb_graph_node->placement_index, i, j,
//							pb_graph_node->input_pins[i][j].pin_count_in_cluster);
//					(*curr_net)++;
//				}
//			}
//		}
//		for (i = 0; i < pb_graph_node->num_clock_ports; i++) {
//			for (j = 0; j < pb_graph_node->num_clock_pins[i]; j++) {
//				if (rr_graph[pb_graph_node->clock_pins[i][j].pin_count_in_cluster].net_num
//						!= OPEN) {
//					load_internal_cb_rr_graph_net_nums(
//							&rr_graph[pb_graph_node->clock_pins[i][j].pin_count_in_cluster],
//							rr_graph, nets, curr_net, &temp);
//					assert(temp == nets[*curr_net].num_sinks);
//					temp = 0;
//					nets[*curr_net].is_global = TRUE;
//					size =
//							strlen(pb_graph_node->pb_type->name)
//									+ pb_graph_node->placement_index / 10
//									+ i / 10 + j / 10
//									+ pb_graph_node->clock_pins[i][j].pin_count_in_cluster
//											/ 10 + 26;
//					nets[*curr_net].name = (char *)my_calloc(size, sizeof(char));
//					sprintf(nets[*curr_net].name,
//							"%s[%d].clock[%d][%d].pin[%d]",
//							pb_graph_node->pb_type->name,
//							pb_graph_node->placement_index, i, j,
//							pb_graph_node->clock_pins[i][j].pin_count_in_cluster);
//					(*curr_net)++;
//				}
//			}
//		}
//	}
//
//	if (pb_type->blif_model != NULL) {
//		/* This is a terminal node so it might drive nets, find and map the rr_graph path for those nets */
//		for (i = 0; i < pb_graph_node->num_output_ports; i++) {
//			for (j = 0; j < pb_graph_node->num_output_pins[i]; j++) {
//				if (rr_graph[pb_graph_node->output_pins[i][j].pin_count_in_cluster].net_num
//						!= OPEN) {
//					load_internal_cb_rr_graph_net_nums(
//							&rr_graph[pb_graph_node->output_pins[i][j].pin_count_in_cluster],
//							rr_graph, nets, curr_net, &temp);
//					assert(temp == nets[*curr_net].num_sinks);
//					temp = 0;
//					size =
//							strlen(pb_graph_node->pb_type->name)
//									+ pb_graph_node->placement_index / 10
//									+ i / 10 + j / 10
//									+ pb_graph_node->output_pins[i][j].pin_count_in_cluster
//											/ 10 + 26;
//					nets[*curr_net].name = (char *)my_calloc(size, sizeof(char));
//					sprintf(nets[*curr_net].name,
//							"%s[%d].output[%d][%d].pin[%d]",
//							pb_graph_node->pb_type->name,
//							pb_graph_node->placement_index, i, j,
//							pb_graph_node->output_pins[i][j].pin_count_in_cluster);
//					(*curr_net)++;
//				}
//			}
//		}
//	} else {
//		/* Recurse down to primitives */
//		for (i = 0; i < pb_type->num_modes; i++) {
//			for (j = 0; j < pb_type->modes[i].num_pb_type_children; j++) {
//				for (k = 0; k < pb_type->modes[i].pb_type_children[j].num_pb;
//						k++) {
//					load_internal_cb_nets(top_level,
//							&pb_graph_node->child_pb_graph_nodes[i][j][k],
//							rr_graph, curr_net);
//				}
//			}
//		}
//	}
//
//	if (pb_graph_node->parent_pb_graph_node == NULL) { /* at top level */
//		assert(*curr_net == top_level->num_local_nets);
//	}
//}

/* allocate space to store nets internal to cb 
 two pass algorithm, pass 1 count and allocate # nets, pass 2 determine # sinks
 */
//static void alloc_internal_cb_nets(INOUTP t_pb *top_level,
//		INP t_pb_graph_node *pb_graph_node, INOUTP t_rr_node *rr_graph,
//		INP int pass) {
//	int i, j, k;
//	const t_pb_type *pb_type;
//	int num_sinks;
//
//	pb_type = pb_graph_node->pb_type;
//
//	if (pb_graph_node->parent_pb_graph_node == NULL) { /* determine nets driven from inputs at top level */
//		top_level->num_local_nets = 0;
//		if (pass == 1)
//			top_level->local_nets = NULL;
//		for (i = 0; i < pb_graph_node->num_input_ports; i++) {
//			for (j = 0; j < pb_graph_node->num_input_pins[i]; j++) {
//				if (rr_graph[pb_graph_node->input_pins[i][j].pin_count_in_cluster].net_num
//						!= OPEN) {
//					if (pass == 2) {
//						num_sinks =
//								count_sinks_internal_cb_rr_graph_net_nums(
//										&rr_graph[pb_graph_node->input_pins[i][j].pin_count_in_cluster],
//										rr_graph);
//						top_level->local_nets[top_level->num_local_nets].num_sinks =
//								num_sinks;
//						top_level->local_nets[top_level->num_local_nets].node_block = (int *)
//								my_calloc(num_sinks, sizeof(int));
//						top_level->local_nets[top_level->num_local_nets].node_block_port = (int *)
//								my_calloc(num_sinks, sizeof(int));
//						top_level->local_nets[top_level->num_local_nets].node_block_pin = (int *)
//								my_calloc(num_sinks, sizeof(int));
//					}
//					top_level->num_local_nets++;
//				}
//			}
//		}
//		for (i = 0; i < pb_graph_node->num_clock_ports; i++) {
//			for (j = 0; j < pb_graph_node->num_clock_pins[i]; j++) {
//				if (rr_graph[pb_graph_node->clock_pins[i][j].pin_count_in_cluster].net_num
//						!= OPEN) {
//					if (pass == 2) {
//						num_sinks =
//								count_sinks_internal_cb_rr_graph_net_nums(
//										&rr_graph[pb_graph_node->clock_pins[i][j].pin_count_in_cluster],
//										rr_graph);
//						top_level->local_nets[top_level->num_local_nets].num_sinks =
//								num_sinks;
//						top_level->local_nets[top_level->num_local_nets].node_block = (int *)
//								my_calloc(num_sinks, sizeof(int));
//						top_level->local_nets[top_level->num_local_nets].node_block_port = (int *)
//								my_calloc(num_sinks, sizeof(int));
//						top_level->local_nets[top_level->num_local_nets].node_block_pin = (int *)
//								my_calloc(num_sinks, sizeof(int));
//					}
//					top_level->num_local_nets++;
//				}
//			}
//		}
//	}
//
//	if (pb_type->blif_model != NULL) {
//		/* This is a terminal node so it might drive nets, find and map the rr_graph path for those nets */
//		for (i = 0; i < pb_graph_node->num_output_ports; i++) {
//			for (j = 0; j < pb_graph_node->num_output_pins[i]; j++) {
//				if (rr_graph[pb_graph_node->output_pins[i][j].pin_count_in_cluster].net_num
//						!= OPEN) {
//					if (pass == 2) {
//						num_sinks =
//								count_sinks_internal_cb_rr_graph_net_nums(
//										&rr_graph[pb_graph_node->output_pins[i][j].pin_count_in_cluster],
//										rr_graph);
//						top_level->local_nets[top_level->num_local_nets].num_sinks =
//								num_sinks;
//						top_level->local_nets[top_level->num_local_nets].node_block = (int *)
//								my_calloc(num_sinks, sizeof(int));
//						top_level->local_nets[top_level->num_local_nets].node_block_port = (int *)
//								my_calloc(num_sinks, sizeof(int));
//						top_level->local_nets[top_level->num_local_nets].node_block_pin = (int *)
//								my_calloc(num_sinks, sizeof(int));
//					}
//					top_level->num_local_nets++;
//				}
//			}
//		}
//	} else {
//		/* Recurse down to primitives */
//		for (i = 0; i < pb_type->num_modes; i++) {
//			for (j = 0; j < pb_type->modes[i].num_pb_type_children; j++) {
//				for (k = 0; k < pb_type->modes[i].pb_type_children[j].num_pb;
//						k++) {
//					alloc_internal_cb_nets(top_level,
//							&pb_graph_node->child_pb_graph_nodes[i][j][k],
//							rr_graph, pass);
//				}
//			}
//		}
//	}
//
//	if (pb_graph_node->parent_pb_graph_node == NULL) { /* at top level */
//		if (pass == 1) {
//			top_level->local_nets = (struct s_net *)my_calloc(top_level->num_local_nets,
//					sizeof(struct s_net));
//		}
//	}
//}

static void mark_constant_generators(INP int L_num_blocks,
		INP struct s_block block_list[], INP int ncount,
		INOUTP struct s_net nlist[]) {
	int i;
	for (i = 0; i < L_num_blocks; i++) {
		mark_constant_generators_rec(block_list[i].pb,
				block_list[i].pb_route, nlist);
	}
}

static void mark_constant_generators_rec(INP t_pb *pb, INP t_pb_route *pb_route,
		INOUTP struct s_net nlist[]) {
	int i, j;
	t_pb_type *pb_type;
	bool const_gen;
	if (pb->pb_graph_node->pb_type->blif_model == NULL) {
		for (i = 0;
				i
						< pb->pb_graph_node->pb_type->modes[pb->mode].num_pb_type_children;
				i++) {
			pb_type =
					&(pb->pb_graph_node->pb_type->modes[pb->mode].pb_type_children[i]);
			for (j = 0; j < pb_type->num_pb; j++) {
				if (pb->child_pbs[i][j].name != NULL) {
					mark_constant_generators_rec(&(pb->child_pbs[i][j]),
						pb_route, nlist);
				}
			}
		}
	} else if (strcmp(pb->pb_graph_node->pb_type->name, "inpad") != 0) {
		const_gen = true;
		for (i = 0; i < pb->pb_graph_node->num_input_ports && const_gen == true;
				i++) {
			for (j = 0;
					j < pb->pb_graph_node->num_input_pins[i]
							&& const_gen == true; j++) {
				if (pb_route[pb->pb_graph_node->input_pins[i][j].pin_count_in_cluster].atom_net_idx
						!= OPEN) {
					const_gen = false;
				}
			}
		}
		for (i = 0; i < pb->pb_graph_node->num_clock_ports && const_gen == true;
				i++) {
			for (j = 0;
					j < pb->pb_graph_node->num_clock_pins[i]
							&& const_gen == true; j++) {
				if (pb_route[pb->pb_graph_node->clock_pins[i][j].pin_count_in_cluster].atom_net_idx
						!= OPEN) {
					const_gen = false;
				}
			}
		}
		if (const_gen == true) {
			vpr_printf_info("%s is a constant generator.\n", pb->name);
			for (i = 0; i < pb->pb_graph_node->num_output_ports; i++) {
				for (j = 0; j < pb->pb_graph_node->num_output_pins[i]; j++) {
					if (pb_route[pb->pb_graph_node->output_pins[i][j].pin_count_in_cluster].atom_net_idx
							!= OPEN) {
						nlist[pb_route[pb->pb_graph_node->output_pins[i][j].pin_count_in_cluster].atom_net_idx].is_const_gen =
								true;
					}
				}
			}
		}
	}
}

/* Free logical blocks of netlist */
void free_logical_blocks(void) {
	int iblk, i;
	t_model_ports *port;
	struct s_linked_vptr *tvptr, *next;

	for (iblk = 0; iblk < num_logical_blocks; iblk++) {
		port = logical_block[iblk].model->inputs;
		i = 0;
		while (port) {
			if (!port->is_clock) {
				free(logical_block[iblk].nets->input_nets[i]);
				if (logical_block[iblk].nets->input_net_tnodes) {
					if (logical_block[iblk].nets->input_net_tnodes[i])
						free(logical_block[iblk].nets->input_net_tnodes[i]);
				}
				if (logical_block[iblk].input_pin_names != NULL && logical_block[iblk].input_pin_names[i] != NULL) {
					for (int j = 0; j < port->size; j++) {
						if (logical_block[iblk].input_pin_names[i][j] != NULL) {
							free(logical_block[iblk].input_pin_names[i][j]);
						}
					}
					free(logical_block[iblk].input_pin_names[i]);
				}
				i++;
			}
			port = port->next;
		}
		if (logical_block[iblk].nets->input_net_tnodes)
			free(logical_block[iblk].nets->input_net_tnodes);
		if (logical_block[iblk].input_pin_names != NULL) {
			free(logical_block[iblk].input_pin_names);
		}
		if (logical_block[iblk].clock_pin_name != NULL) {
			free(logical_block[iblk].clock_pin_name);
		}

		tvptr = logical_block[iblk].packed_molecules;
		while (tvptr != NULL) {
			next = tvptr->next;
			free(tvptr);
			tvptr = next;
		}

		free(logical_block[iblk].nets->input_nets);
		port = logical_block[iblk].model->outputs;
		i = 0;
		while (port) {
			free(logical_block[iblk].nets->output_nets[i]);
			if (logical_block[iblk].nets->output_net_tnodes) {
				if (logical_block[iblk].nets->output_net_tnodes[i])
					free(logical_block[iblk].nets->output_net_tnodes[i]);
			}
			if (logical_block[iblk].output_pin_names != NULL && logical_block[iblk].output_pin_names[i] != NULL) {
				for (int j = 0; j < port->size; j++) {
					if (logical_block[iblk].output_pin_names[i][j] != NULL) {
						free(logical_block[iblk].output_pin_names[i][j]);
					}
				}
				free(logical_block[iblk].output_pin_names[i]);
			}
			i++;
			port = port->next;
		}
		if (logical_block[iblk].nets->output_net_tnodes) {
			free(logical_block[iblk].nets->output_net_tnodes);
		}
		free(logical_block[iblk].nets->output_nets);
		free(logical_block[iblk].name);
		tvptr = logical_block[iblk].truth_table;
		while (tvptr != NULL) {
			if (tvptr->data_vptr)
				free(tvptr->data_vptr);
			next = tvptr->next;
			free(tvptr);
			tvptr = next;
		}
		if (logical_block[iblk].output_pin_names != NULL) {
			free(logical_block[iblk].output_pin_names);
		}
	}
	free(logical_block);
	logical_block = NULL;
}

/* Free  logical blocks of netlist */
void free_logical_nets(void) {
	int inet;

	for (inet = 0; inet < num_logical_nets; inet++) {
		free(vpack_net[inet].name);
		free(vpack_net[inet].node_block);
		free(vpack_net[inet].node_block_port);
		free(vpack_net[inet].node_block_pin);
	}
	free(vpack_net);
	vpack_net = NULL;
	if (vpack_net_power) {
		free(vpack_net_power);
		vpack_net_power = NULL;
	}
}



static t_pb_route *alloc_pb_route(t_pb_graph_node *pb_graph_node) {
	t_pb_route *pb_route;
	int num_pins = pb_graph_node->total_pb_pins;

	assert(pb_graph_node->parent_pb_graph_node == NULL); /* This function only operates on top-level pb_graph_node */

	pb_route = new t_pb_route[num_pins];

	return pb_route;
}

static void load_interal_to_block_net_nums(INP t_type_ptr type, INOUTP t_pb_route *pb_route) {
	int num_pins = type->pb_graph_head->total_pb_pins;

	for (int i = 0; i < num_pins; i++) {
		if (pb_route[i].prev_pb_pin_id != OPEN && pb_route[i].atom_net_idx == OPEN) {
			load_atom_index_for_pb_pin(pb_route, i);
		}
	}
}

static void load_atom_index_for_pb_pin(t_pb_route *pb_route, int ipin) {
	int driver = pb_route[ipin].prev_pb_pin_id;
	
	assert(driver != OPEN);
	assert(pb_route[ipin].atom_net_idx == OPEN);

	if (pb_route[driver].atom_net_idx == OPEN) {
		load_atom_index_for_pb_pin(pb_route, driver);
	}	
		
	pb_route[ipin].atom_net_idx = pb_route[driver].atom_net_idx;
}
