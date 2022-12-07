import os
import sys
import shutil


if __name__ == "__main__":
    script_folder = os.path.abspath(os.path.dirname(__file__))    
    proj_folder = os.path.join(script_folder, '..')
    output_folder = os.path.join(proj_folder, 'bin')

    os.chdir(proj_folder)
    os.system('git clone https://github.com/uilnauyisDP/ecs251_riscv_console_group_2')
    os.system('mv ecs251_riscv_console_group_2 port_group_2')
    os.chdir(proj_folder + '/port_group_2')
    os.system('git clean -df; git reset origin/main --hard;')

    os.chdir(proj_folder)
    os.system('git clone https://github.com/uilnauyisDP/ecs251_riscv_console_group_5')
    os.system('mv ecs251_riscv_console_group_5 port_group_5')
    os.chdir(proj_folder + '/port_group_5')
    os.system('git clean -df; git reset origin/main --hard;')

    target_folders = [
        proj_folder + '/riscv-firmware', 
        proj_folder + '/riscv-cart', 
        proj_folder + '/cartridge_baseline', 
        proj_folder + '/os_kernel',
        proj_folder + '/port_group_2/birdApp',
        proj_folder + '/port_group_5/birdApp'
        ]
    for target_folder in target_folders:
        shutil.rmtree(os.path.join(proj_folder, target_folder, 'bin'), ignore_errors=True)
        shutil.rmtree(os.path.join(proj_folder, target_folder, 'obj'), ignore_errors=True)

    target_folders.append(proj_folder + '/docs/ECS251_Group1_Project_Final_Phase.pdf')
    target_folders.append(proj_folder + '/README.md')

    os.chdir(proj_folder)
    

    if not os.path.exists(output_folder):
        os.mkdir(output_folder)
    
    os.system('tar czvf bin/project_phase4_group1_handover.tgz ' + ' '.join(target_folders))

    shutil.rmtree(proj_folder + '/port_group_2', ignore_errors=True)
    shutil.rmtree(proj_folder + '/port_group_5', ignore_errors=True)

    