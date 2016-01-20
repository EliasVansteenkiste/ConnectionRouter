from tempfile import mkstemp 
from shutil import move 
from os import remove, close 
import sys
import subprocess

def replace(file_path, pattern, subst):
    #Create temp file
    fh, abs_path = mkstemp()
    with open(abs_path,'w') as new_file:
        with open(file_path) as old_file:
            for line in old_file:
                new_file.write(line.replace(pattern,subst))
    close(fh)
    #Remove original file
    remove(file_path)
    #Move new file
    move(abs_path, file_path)

path = sys.argv[1]

print "path", path
p_find = subprocess.Popen('find '+path+' -name "*parse_results.txt"', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

for path_results in p_find.stdout.readlines():
	path_results = path_results.rstrip()
        print path_results
	replace(path_results,".xml","")
	replace(path_results,".blif","")
	replace(path_results,".v","")
	replace(path_results,"(s)","")
	replace(path_results,".",",")
