#!/usr/bin/python3
# General Build Script Template
import sys
import os
import shutil
import argparse
import docker
from time import time
sys.path.append( os.path.join(os.path.abspath(os.path.dirname(__file__)), 'scripts'))

UNIX_CMAKE_STR = 'cmake {} -B{} -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE={}'
MSVC_CMAKE_STR = 'cmake {} -B{} -G "Visual Studio 16 2019"'
TEST_EXCUTABLE_NAME = 'test_main.elf'
CONTAINER_NAME = 'riscv-toolchain-gdbgui'
IMAGE_TAG = ''

def handleAutoComplete():
    if sys.platform == 'linux':
        complete_cmd = 'complete -F _longopt {}'.format(os.path.basename(__file__))
        bashrc_path = os.path.expanduser('~/.bashrc')
        with open(bashrc_path) as f:
            if not complete_cmd in f.read():
                os.system('echo "{}" >> {}'.format(complete_cmd, bashrc_path))
    else:
        pass

class Dir():
    # script_folder = os.path.abspath(os.path.dirname(__file__))
    script_folder = "/code/"
    build_root = os.path.join(script_folder, 'build')
    cmake_project = script_folder
    lfs_yaml = os.path.join(script_folder, 'lfs.yaml')
    cmake_toolchain = os.path.join(script_folder, 'cmake', 'riscv.cmake')

class HostDir():
    script_folder = os.path.abspath(os.path.dirname(__file__))
    build_root = os.path.join(script_folder, 'build')
    cmake_project = script_folder
    lfs_yaml = os.path.join(script_folder, 'lfs.yaml')
    cmake_toolchain = os.path.join(script_folder, 'cmake', 'riscv.cmake')

class BuildTarget():
    def __init__(self):
        self.require_test = False
        self.require_build = False
        self.host_exec_relative = None
        self.exec_args = None
        self.cmake_options = ''

    @property
    def host_exec(self):
        if self.host_exec_relative is None:
            return None
        return os.path.join(self.build_dir, self.host_exec_relative)

    def updateFromArgs(self, args):
        self.require_test = args.test_all or args.__getattribute__(self.parser_test_var)
        platform_exe = args.__getattribute__(self.parser_run_var)

        self.require_build = True
        if self.require_test:
            self.host_exec_relative = os.path.join('os_test_qemu', TEST_EXCUTABLE_NAME)
        elif platform_exe:
            self.host_exec_relative = platform_exe[0]
            self.exec_args = ' '.join(platform_exe[1:])
        elif args.__getattribute__(self.platform.replace('-', '_')):
            pass
        else:
            self.require_build = False

        if args.cmake_options:
            self.cmake_options = args.cmake_options

    def cmakeBaseOptions(self):
        CMAKE_OPTION_STR = ''
        if self.require_test:
            CMAKE_OPTION_STR += ' -DBUILD_TEST=1'
        if self.cmake_options:
            CMAKE_OPTION_STR += self.cmake_options
        return CMAKE_OPTION_STR

    def updateArgParser(self, parser):
        self.parser_build_cmd = ('--' + self.platform)

        self.parser_run_cmd = '--run' + ('' if self.platform == 'native' else ('-' + self.platform))
        self.parser_run_var = (self.platform) + '_exe'
        # self.parser_run_var.replace('-','_')

        self.parser_test_cmd = '--test' + ('' if self.platform == 'native' else ('-' + self.platform))
        self.parser_test_var = self.parser_test_cmd[2:].replace('-','_')

        parser.add_argument(self.parser_build_cmd, action='store_true', \
            help='Build ' + self.platform + ' platform')
        parser.add_argument(self.parser_run_cmd, dest=self.parser_run_var, \
            nargs='+', help='run ' + self.platform + ' executable')
        parser.add_argument(self.parser_test_cmd, action='store_true', \
            help='Run test on ' + self.platform + ' platform')


    def checkExecutableValid(self):
        if self.host_exec is None:
            return False
        # if not os.path.isfile(self.host_exec):
        #     print('Error: {} executable {} not found!'.format(self.platform, self.host_exec))
        #     return False
        return True

    @property
    def build_dir(self):
        return os.path.join(Dir.build_root, self.platform)

class LinuxCrossCompileTarget(BuildTarget):
    platform = 'rv32'

    def runBuild(self):
        if not self.require_build:
            return None
        cmake_generate_cmd = UNIX_CMAKE_STR.format(Dir.cmake_project, self.build_dir, Dir.cmake_toolchain)
        cmake_generate_cmd += self.cmakeBaseOptions()

        cmake_build_cmd = 'cmake --build {}'.format(self.build_dir)

        exec_ret = containerRun(cmake_generate_cmd, Dir.cmake_project)
        
        # print(str(exec_ret.output, encoding='utf-8'))
        if exec_ret != 0:
            print(exec_ret)
            exit(1)

        exec_ret = containerRun(cmake_build_cmd, Dir.cmake_project)

        if exec_ret != 0:
            print(exec_ret)
            exit(1)

    def runExecutable(self):
        if not self.checkExecutableValid():
            return None
        qemu_cmd = "qemu-system-riscv32 -nographic -machine virt -device loader,file=/code/build/{}/os_test_qemu/test_main.elf -bios none".format(self.platform)
        exec_ret = containerRun(qemu_cmd, Dir().script_folder)
        if exec_ret != 0:
            exit(1)

def createParser():
    # initialize argparse options
    parser = argparse.ArgumentParser()

    parser.add_argument('--clean', action='store_true', help='Clean build folder')
    parser.add_argument('--all', action='store_true', help='Compile for all platforms')
    parser.add_argument('--test-all', action='store_true', help='Run test on all platforms')
    parser.add_argument('--cmake-options', dest='cmake_options', help='additional cmake options. Put in quote, start with space: " -DCMAKE_BUILD_TYPE=Debug"')
    parser.add_argument('--sync-lfs', action='store_true', help='Synchronize large file storage')

    return parser

# def prepareContainer():
#     client = docker.from_env()
    
#     test_container = None
#     for container in client.containers.list():
#         if container.name == CONTAINER_NAME:
#             test_container = container
#             break
    

def containerRun(command, work_dir_in_container):
    client = docker.from_env()
    exit_code = 0
    try:
        log = client.containers.run('shawsheenchen/riscv_qemu_dev', command, volumes=[HostDir.cmake_project + ':/code'], remove=True, working_dir=work_dir_in_container)
        log = str(log, encoding='utf-8')
    except docker.errors.ContainerError as e:
        log = e
        exit_code = 1
    print(log)
    return exit_code



def run(build_script_folder=os.path.abspath(os.path.dirname(__file__))):
    parser = createParser()
    handleAutoComplete()

    targets = [LinuxCrossCompileTarget()]
    for target in targets:
        target.updateArgParser(parser)

    args = parser.parse_args()

    if args.clean:
        shutil.rmtree(Dir.build_root, ignore_errors=True)
        quit()

    if args.sync_lfs:
        WebDrive.sync(Dir.lfs_yaml, Dir.lfs_asset)

    t_start = time()

    build_target_cnt = 0
    for target in targets:
        target.updateFromArgs(args)
        target.runBuild()
        build_target_cnt += int(target.require_build)

    if 0 == build_target_cnt:
        targets[0].require_build = True
        targets[0].runBuild()

    t_finish = time()
    print("Total build time: {:.3f}s".format(t_finish - t_start))

    for target in targets:
        target.runExecutable()


if __name__ == "__main__":
    run()