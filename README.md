# NekoPCFramework
Windows code for communicating with a VC707 evaluation board running a MIAOW
compute unit. The code is currently hardcoded to use COM3 with a define but
this can be easily overridden. Loading of kernels is performed by filling in
the instr_mem array in kernel.cpp with a stream of instructions. Currently it
has the end kernel instruction in it. Similarly data memory can be filled in
by filling in the data_mem array in the same file.

This code is meant to be used in conjunction with the open source MIAOW
project located at: https://github.com/VerticalResearchGroup/miaow

For instructions bootstrapping the FPGA, please see the following link:
https://github.com/VerticalResearchGroup/miaow/wiki/Neko