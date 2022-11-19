import os
import sys
import shutil


if __name__ == "__main__":
    script_folder = os.path.abspath(os.path.dirname(__file__))
    proj_folder = os.path.join(script_folder, '..')
    target_folders = ['riscv-firmware', 'riscv-cart', 'cartridge_baseline', 'os_kernel']
    for target_folder in target_folders:
        shutil.rmtree(os.path.join(proj_folder, target_folder, 'bin'), ignore_errors=True)
        shutil.rmtree(os.path.join(proj_folder, target_folder, 'obj'), ignore_errors=True)

    os.chdir(proj_folder)
    output_folder = os.path.join(proj_folder, 'bin')

    if not os.path.exists(output_folder):
        os.mkdir(output_folder)
    
    os.system('tar czvf bin/project_phase3_group1_handover.tgz ' + ' '.join(target_folders))

    