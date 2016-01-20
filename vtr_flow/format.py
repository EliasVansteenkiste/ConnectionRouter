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
replace(path,".xml","")
replace(path,".blif","")
replace(path,".v","")
replace(path,"(s)","")
replace(path,".",",")
