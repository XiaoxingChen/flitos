import os
import sys
import re

def extractFunctionNames(filename):
    pattern = "[0-9,a-f]{8}\s[TW]\s(.*)\n"
    ret = []
    with open(filename, 'r') as f:
        s = f.read()
        ret = re.findall(pattern,s)
    return ret

def main(elf_filename):
    nm_output_filename = elf_filename + "_symbols.txt"
    output_filename = elf_filename + '.txt'
    os.system("nm {} > {}".format(elf_filename, nm_output_filename))
    function_names = extractFunctionNames(nm_output_filename)
    os.system("> " + output_filename)
    for f_name in function_names:
        os.system("riscv32-unknown-elf-gdb -batch -ex 'file {}' -ex 'set print asm-demangle on'  -ex 'disassemble /m {}' >> {}".format(elf_filename, f_name, output_filename))
    
    

if __name__ == "__main__":
    if(len(sys.argv) < 2):
        quit()
    main(sys.argv[1])
    