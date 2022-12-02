import os
import sys
import shutil


if __name__ == "__main__":
    script_folder = os.path.abspath(os.path.dirname(__file__))
    proj_folder = os.path.join(script_folder, '..')
    target_folders = ['riscv-firmware/bin/riscv-console-firmware.strip', 'cartridge_baseline']

    os.chdir('/code/riscv-firmware')
    os.system('make clean')
    os.system('make')
    for target_folder in target_folders[1:]:
        shutil.rmtree(os.path.join(proj_folder, target_folder, 'bin'), ignore_errors=True)
        shutil.rmtree(os.path.join(proj_folder, target_folder, 'obj'), ignore_errors=True)

    os.chdir(proj_folder)
    output_folder = os.path.join(proj_folder, 'bin')

    if not os.path.exists(output_folder):
        os.mkdir(output_folder)
    
    os.system('tar czvf bin/ecs251_group1_sdk_handover.tgz ' + ' '.join(target_folders))

    