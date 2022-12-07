
# FlitOS

FlitOS is a real-time operating system kernel for RISC-V machine. 
The kernel currently supports 32-bit device only. 
This project is part of UCDavis ECS251.
This repo contains a simulator from UCDClassNitta/riscv-console.

## Table of Contents

* [FlitOS](#risc-v-console-simulator)
    * [Table of Contents](#table-of-contents)
    * [Getting Started](#getting-started)
    * [Test on riscv-console](#test-on-riscv-console)
    * [FlitOS Features](#flitos-features)
    * [Authors](#authors)


## Getting Started
The simulation environment and toolchain have been setup to run within a Docker container. The directions assume that you have git, Docker, X-11 support and a bash shell on your machine. If you are running on Windows and wish to use PowerShell follow the directions [here](docs/powershell.md). 

### Extract Source Code

```
tar xzvf project_phase4_group1_handover.tgz
```

### Launch Docker Container
Once the repository has been cloned, change directories into the riscv-console and run the console launch script with the command:
```
cd project_phase4_group1_handover
./rvconsole.sh
```
Once the container is launched you should see a prompt like:
```
root@fedcba9876543210:/code#
```

## Test on riscv-console

### Compile the kernel

```
root@fedcba9876543210:/code# cd riscv-firmware
root@fedcba9876543210:/code/riscv-firmware# make
```

### Compile the Game Sample

```
root@fedcba9876543210:/code# cd riscv-cart
root@fedcba9876543210:/code/riscv-cart# make
```

### Compile the Game for OS from Group 2 

```
root@fedcba9876543210:/code# cd port_group_2/birdApp
root@fedcba9876543210:/code/port_group_2/birdApp# make
```

### Compile the Game for OS from Group 5

```
root@fedcba9876543210:/code# cd port_group_5/birdApp
root@fedcba9876543210:/code/port_group_5/birdApp# make
```


### Launch Simulator

```
root@fedcba9876543210:/code/runsim.sh
```

See the original [document](https://github.com/UCDClassNitta/riscv-console/blob/main/README.md#getting-started) of riscv-console
to find how to load the firmware and cartridge.

Here is the effect of our sample game.


![](https://user-images.githubusercontent.com/16934019/205567793-e140dccb-6c8a-48c9-9d58-3fa1a6dfb489.gif)


## FlitOS Features
### Thread Control

FlitOS supports thread `create`, `yield`, `join` and `sleep`.

### Synchronization

Mutex and condition variables are provided as synchronization objects.

### Inter-thread Communication

A thread safe queue called `pipe` is provided for inter-thread communication.

## Authors

- [Xiaoxing Chen](https://github.com/XiaoxingChen)
- [Yifeng Shi](https://github.com/Sterfan-shi)
- [Siyuan Liu](https://github.com/uilnauyisDP)
- [Tianxiao Cheng](https://github.com/tttxcheng) 

## Related Links

- [UCDClassNitta riscv-console](https://github.com/UCDClassNitta/riscv-console) 
- [RISC-V QEMU Docker Environment](https://github.com/zer0pwned/RISCVDockerEnv)
