#pragma once
#include <vector>
#include <limits>
#include <algorithm>
#include "vpr_types.h"

void routing_stats(bool full_stats, enum e_route_type route_type,
		int num_rr_switch, t_segment_inf * segment_inf, int num_segment,
		float R_minW_nmos, float R_minW_pmos,
		enum e_directionality directionality, int wire_to_ipin_switch,
		bool timing_analysis_enabled,
		float **net_delay, t_slack * slacks, const t_timing_inf &timing_inf);

void print_wirelen_prob_dist(void);

void print_lambda(void);

void get_num_bends_and_length(int inet, int *bends, int *length, int *segments);

int count_netlist_clocks(void);

// template functions must be defined in header, or explicitely instantiated in definition file (defeats the point of template)
template <typename T>
double linear_regression_vector(const std::vector<T>& vals, size_t start_x = 0) {
	// returns slope; index is x, val is y
	size_t n {vals.size() - start_x};

	double x_avg {0}, y_avg {0};
	for (size_t x = start_x; x < vals.size(); ++x) {
		x_avg += x;
		y_avg += vals[x];
	} 
	x_avg /= (double) n;
	y_avg /= (double) n;

	double numerator = 0, denominator = 0;
	for (size_t x = start_x; x < vals.size(); ++x) {
		numerator += (x - x_avg) * (vals[x] - y_avg);
		denominator += (x - x_avg) * (x - x_avg);
	}

	if (denominator == 0) return std::numeric_limits<double>::max();
	return numerator / denominator;
}