import os
import sys
import commands
import mmap
import errno

def make_sure_path_exists(path):
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise

def routeCircuit(circuitpath,channelWidth):
	print("circuit:"+circuitpath)
	words = circuitpath.split('/')
	lastword = words[-1]
	circuitname = lastword.split('.')[0]
	outputfile = "mcnc20logs/mtw_100its_vprtd.text"
	f = open(outputfile,"a")
	f.write("circuit:"+circuitname)
	f.close
	vpr = "vpr/vpr "
	archfile = " vtr_flow/arch/timing/k6_N10_40nm.xml "
	routing_options = " -route -router_algorithm timing_driven -max_router_iterations 100 -first_iter_pres_fac 0.5 -initial_pres_fac 0.5 "
	os.system(vpr+archfile+circuitpath+routing_options+" | tee -a "+outputfile)

#make_sure_path_exists("/recomp/tools/vtr/branches/conr_VTR7/mcnc20logs")
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/alu4.pre-vpr",58)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/apex2.pre-vpr",80)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/apex4.pre-vpr",80)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/bigkey.pre-vpr",46)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/clma.pre-vpr",104)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/des.pre-vpr",48)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/diffeq.pre-vpr",48)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/dsip.pre-vpr",44)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/elliptic.pre-vpr",62)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/ex1010.pre-vpr",120)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/ex5p.pre-vpr",70)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/frisc.pre-vpr",74)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/misex3.pre-vpr",60)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/pdc.pre-vpr",106)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/s298.pre-vpr",52)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/s38417.pre-vpr",52)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/s38584.1.pre-vpr",58)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/seq.pre-vpr",80)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/spla.pre-vpr",88)
routeCircuit("vtr_flow/benchmarks/blif/wiremap6/tseng.pre-vpr",40)

