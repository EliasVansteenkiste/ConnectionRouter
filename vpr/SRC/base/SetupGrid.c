/*
 Author: Jason Luu
 Date: October 8, 2008

 Initializes and allocates the physical logic block grid for VPR.

 */

#include <cstdio>
#include <cstring>
using namespace std;

#include <assert.h>

#include "util.h"
#include "vpr_types.h"
#include "globals.h"
#include "SetupGrid.h"
#include "read_xml_arch_file.h"

static void CheckGrid(void);
static t_type_ptr find_type_col(INP int x);

static void alloc_and_load_num_instances_type(
		t_grid_tile** L_grid, int L_nx, int L_ny,
		int* L_num_instances_type, int L_num_types);

/* Create and fill FPGA architecture grid.         */
void alloc_and_load_grid(INOUTP int *num_instances_type) {

#ifdef SHOW_ARCH
	FILE *dump;
#endif

	/* To remove this limitation, change ylow etc. in t_rr_node to        *
	 * * be ints instead.  Used shorts to save memory.                      */
	if ((nx > 32766) || (ny > 32766)) {
		vpr_throw(VPR_ERROR_OTHER, __FILE__, __LINE__, 
			"nx and ny must be less than 32767, since the router uses shorts (16-bit) to store coordinates.\n"
			"nx: %d, ny: %d\n", nx, ny);
	}

	assert(nx >= 1 && ny >= 1);

	grid = (struct s_grid_tile **) alloc_matrix(0, (nx + 1), 0, (ny + 1),
			sizeof(struct s_grid_tile));

	/* Clear the full grid to have no type (NULL), no capacity, etc */
	for (int x = 0; x <= (nx + 1); ++x) {
		for (int y = 0; y <= (ny + 1); ++y) {
			memset(&grid[x][y], 0, (sizeof(struct s_grid_tile)));
		}
	}

	for (int x = 0; x <= nx + 1; ++x) {
		for (int y = 0; y <= ny + 1; ++y) {

			t_type_ptr type = 0;
			if ((x == 0 && y == 0) || (x == 0 && y == ny + 1) || (x == nx + 1 && y == 0) || (x == nx + 1 && y == ny + 1)) {

				// Assume corners are empty type (by default)
				type = EMPTY_TYPE;

			} else if (x == 0 || y == 0 || x == nx + 1 || y == ny + 1) {

				// Assume edges are IO type (by default)
				type = IO_TYPE;

			} else {

				if (grid[x][y].width_offset > 0 || grid[x][y].height_offset > 0)
					continue;

				// Assume core are not empty and not IO types (by default)
				type = find_type_col(x);
			}

			if (x + type->width - 1 <= nx && y + type->height - 1 <= ny) {
				for (int x_offset = 0; x_offset < type->width; ++x_offset) {
					for (int y_offset = 0; y_offset < type->height; ++y_offset) {
						grid[x+x_offset][y+y_offset].type = type;
						grid[x+x_offset][y+y_offset].width_offset = x_offset;
						grid[x+x_offset][y+y_offset].height_offset = y_offset;
						grid[x+x_offset][y+y_offset].blocks = (int *) my_malloc(sizeof(int) * max(1,type->capacity));
						for (int i = 0; i < max(1,type->capacity); ++i) {
							grid[x+x_offset][y+y_offset].blocks[i] = EMPTY;
						}
					}
				}
			} else if (type == IO_TYPE ) {
				grid[x][y].type = type;
				grid[x][y].blocks = (int *) my_malloc(sizeof(int) * max(1,type->capacity));
				for (int i = 0; i < max(1,type->capacity); ++i) {
					grid[x][y].blocks[i] = EMPTY;
				}
			} else {
				grid[x][y].type = EMPTY_TYPE;
				grid[x][y].blocks = (int *) my_malloc(sizeof(int));
				grid[x][y].blocks[0] = EMPTY;
			}
		}
	}

	// And, refresh (ie. reset and update) the "num_instances_type" array
	// (while also forcing any remaining INVALID blocks to EMPTY_TYPE)
	alloc_and_load_num_instances_type(grid, nx, ny,	num_instances_type, num_types);

	CheckGrid();

#if 0
	for (int y = 0; y <= ny + 1; ++y) {
		for (int x = 0; x <= nx + 1; ++x) {
			vpr_printf_info("[%d][%d] %s\n", x, y, grid[x][y].type->name);
		}
	}
#endif

#ifdef SHOW_ARCH
	/* DEBUG code */
	dump = my_fopen("grid_type_dump.txt", "w", 0);
	for (j = (ny + 1); j >= 0; --j)
	{
		for (i = 0; i <= (nx + 1); ++i)
		{
			fprintf(dump, "%c", grid[i][j].type->name[1]);
		}
		fprintf(dump, "\n");
	}
	fclose(dump);
#endif
}

//===========================================================================//
static void alloc_and_load_num_instances_type(
		t_grid_tile** L_grid, int L_nx, int L_ny,
		int* L_num_instances_type, int L_num_types) {

	for (int i = 0; i < L_num_types; ++i) {
		L_num_instances_type[i] = 0;
	}

	for (int x = 0; x <= (L_nx + 1); ++x) {
		for (int y = 0; y <= (L_ny + 1); ++y) {

			if (!L_grid[x][y].type)
				continue;

			bool isValid = false;
			for (int z = 0; z < L_grid[x][y].type->capacity; ++z) {

				if (L_grid[x][y].blocks[z] == INVALID) {
					L_grid[x][y].blocks[z] = EMPTY;
				} else {
					isValid = true;
				}
			}
			if (!isValid) {
				L_grid[x][y].type = EMPTY_TYPE;
				L_grid[x][y].width_offset = 0;
				L_grid[x][y].height_offset = 0;
			}

			if (L_grid[x][y].width_offset > 0 || L_grid[x][y].height_offset > 0) {
				continue;
			}

			if (L_grid[x][y].type == EMPTY_TYPE) {
				++L_num_instances_type[EMPTY_TYPE->index];
				continue;
			}

			for (int z = 0; z < L_grid[x][y].type->capacity; ++z) {

				if (L_grid[x][y].type == IO_TYPE) {
					++L_num_instances_type[IO_TYPE->index];
				} else {
					++L_num_instances_type[L_grid[x][y].type->index];
				}
			}
		}
	}
}

void freeGrid(void) {

	int i, j;
	if (grid == NULL) {
		return;
	}

	for (i = 0; i <= (nx + 1); ++i) {
		for (j = 0; j <= (ny + 1); ++j) {
			free(grid[i][j].blocks);
		}
	}
	free_matrix(grid, 0, nx + 1, 0, sizeof(struct s_grid_tile));
	grid = NULL;
}

static void CheckGrid(void) {

	int i, j;

	/* Check grid is valid */
	for (i = 0; i <= (nx + 1); ++i) {
		for (j = 0; j <= (ny + 1); ++j) {
			if (NULL == grid[i][j].type) {
				vpr_throw(VPR_ERROR_OTHER, __FILE__, __LINE__, "grid[%d][%d] has no type.\n", i, j);
			}

			if (grid[i][j].usage != 0) {
				vpr_throw(VPR_ERROR_OTHER, __FILE__, __LINE__, "grid[%d][%d] has non-zero usage (%d) before netlist load.\n", i, j, grid[i][j].usage);
			}

			if ((grid[i][j].width_offset < 0)
					|| (grid[i][j].width_offset >= grid[i][j].type->width)) {
				vpr_throw(VPR_ERROR_OTHER, __FILE__, __LINE__, "grid[%d][%d] has invalid width offset (%d).\n", i, j, grid[i][j].width_offset);
			}
			if ((grid[i][j].height_offset < 0)
					|| (grid[i][j].height_offset >= grid[i][j].type->height)) {
				vpr_throw(VPR_ERROR_OTHER, __FILE__, __LINE__, "grid[%d][%d] has invalid height offset (%d).\n", i, j, grid[i][j].height_offset);
			}

			if ((NULL == grid[i][j].blocks)
					&& (grid[i][j].type->capacity > 0)) {
				vpr_throw(VPR_ERROR_OTHER, __FILE__, __LINE__, "grid[%d][%d] has no block list allocated.\n", i, j);
			}
		}
	}
}

static t_type_ptr find_type_col(INP int x) {

	int i, j;
	int start, repeat;
	float rel;
	bool match;
	int priority, num_loc;
	t_type_ptr column_type;

	priority = FILL_TYPE->grid_loc_def[0].priority;
	column_type = FILL_TYPE;

	for (i = 0; i < num_types; i++) {
		if (&type_descriptors[i] == IO_TYPE
				|| &type_descriptors[i] == EMPTY_TYPE
				|| &type_descriptors[i] == FILL_TYPE)
			continue;
		num_loc = type_descriptors[i].num_grid_loc_def;
		for (j = 0; j < num_loc; j++) {
			if (priority < type_descriptors[i].grid_loc_def[j].priority) {
				match = false;
				if (type_descriptors[i].grid_loc_def[j].grid_loc_type
						== COL_REPEAT) {
					start = type_descriptors[i].grid_loc_def[j].start_col;
					repeat = type_descriptors[i].grid_loc_def[j].repeat;
					if (start < 0) {
						start += (nx + 1);
					}
					if (x == start) {
						match = true;
					} else if (repeat > 0 && x > start && start > 0) {
						if ((x - start) % repeat == 0) {
							match = true;
						}
					}
				} else if (type_descriptors[i].grid_loc_def[j].grid_loc_type
						== COL_REL) {
					rel = type_descriptors[i].grid_loc_def[j].col_rel;
					if (nint(rel * nx) == x) {
						match = true;
					}
				}
				if (match) {
					priority = type_descriptors[i].grid_loc_def[j].priority;
					column_type = &type_descriptors[i];
				}
			}
		}
	}
	return column_type;
}
