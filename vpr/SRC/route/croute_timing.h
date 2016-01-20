bool try_timing_driven_route_conr(
    struct s_router_opts router_opts,
    float **net_delay,
    t_slack * slacks,
    t_ivec ** clb_opins_used_locally,
    bool timing_analysis_enabled, 
    const t_timing_inf &timing_inf);

bool try_timing_driven_route_con(
    int icon,
    int itry,
    float pres_fac,
    struct s_router_opts router_opts,
    float *net_delay,
    t_slack* slacks,
    t_rr_to_rg_node_hash_map* node_map);


