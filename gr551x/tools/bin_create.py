# Copyright (c) 2021 GOODIX.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/usr/bin/env python
# -*- coding: utf-8 -*-
# author: zhangqf

"""
This module use arm-none-eabi-xxx function to deal the xxx.elf file. we recept two parameters, and the 
format of the usage: bin_create.py input_file.elf  out_file.bin. Thanks for use this module
"""

import os
from os import path
import sys
import sys
import time
import re
import os
import json

CUSTOM_CONFIG   = "../../../device/soc/goodix/gr551x/sdk_liteos/config/custom_config.h"

BufSize = 1024
PatternValue = 0x4744
BleToolVersion = "v1.0"
NumberOfOneLine = 0x10
MaxCommentsLen = 12

tool_usage = """\n
BLE Tool Usage:
after_build.py --mode=gen_fw --cfg=config_file --bin=xxx.bin --out_dir=example_dir --app_name=xxx [--dap]\n
"""

OPT_MODE_INVALID = 1
OPT_MODE_GEN = 2
OPT_MODE_MERGE_BIN = 3
OPT_MODE_HEX2BIN = 4
OPT_MODE_BIN2HEX = 5
OPT_MODE_ENC = 6
OPT_MODE_SPLIT_ROM = 7

XIP_SPEED_64M = 0
XIP_SPEED_48M = 1
XIP_SPEED_32M = 2
XIP_SPEED_24M = 3
XIP_SPEED_16M = 4

SYS_CLK_64M = 0
SYS_CLK_48M = 1
SYS_CLK_32M = 5
SYS_CLK_24M = 3
SYS_CLK_XO16M = 2
SYS_CLK_16M = 4

BIN2HEX_RES_OK = 0
BIN2HEX_RES_BIN_FILE_NOT_EXIST = 1
BIN2HEX_RES_HEX_FILE_PATH_ERROR = 2

HEX2BIN_RES_OK = 0
HEX2BIN_RES_DATA_TOO_LONG = 1
HEX2BIN_RES_DATA_TOO_SHORT = 2
HEX2BIN_RES_NO_COLON = 3
HEX2BIN_RES_TYPE_ERROR = 4
HEX2BIN_RES_LENGTH_ERROR = 5
HEX2BIN_RES_CHECK_ERROR = 6
HEX2BIN_RES_HEX_FILE_NOT_EXIST = 7
HEX2BIN_RES_BIN_FILE_PATH_ERROR = 8
HEX2BIN_RES_WRITE_ERROR = 9
HEX2BIN_RES_HEX_FILE_NO_END = 10

class BootInfo():
    def __init__(self):
        self.bin_size = 0
        self.check_sum = 0
        self.load_addr = 0
        self.run_addr = 0
        self.xqspi_xip_cmd = 0
        self.xqspi_speed = 0     # bit: 0..3  clock speed
        self.code_copy_mode = 0  # bit: 4 code copy mode
        self.boot_clk = 0        # bit: 5..7 reserved
        self.check_image = 0     # bit: 8 check image
        self.boot_delay = 0
        self.is_dap_boot = 0     # bit:11 check if boot dap mode
        self.reserved = 0        # bit: 24 reserved

    def to_bytes(self):
        boot_config = self.xqspi_speed ^ (self.code_copy_mode << 4) ^ (self.boot_clk << 5) ^ \
                      (self.check_image << 8) ^ (self.boot_delay << 9) ^ \
                      (self.is_dap_boot << 10) ^ (self.reserved << 11)
        load_addr = int(self.load_addr, base=16)
        run_addr = int(self.run_addr, base=16)
        items = [self.bin_size, self.check_sum, load_addr, run_addr, self.xqspi_xip_cmd, boot_config]
        boot_info_byte = list(map(lambda item: item.to_bytes(4, byteorder='little', signed=False), items))
        return boot_info_byte


class ImgInfo():
    def __init__(self):
        self.pattern = 0
        self.version = 0
        self.boot_info = BootInfo()
        self.comments = []
        self.reserved = []

    def to_bytes(self):
        img_info_byte = []
        img_info_byte.append(self.pattern.to_bytes(2, byteorder='little', signed=False))
        img_info_byte.append(int(self.version, base=16).to_bytes(2, byteorder='little', signed=False))
        boot_info_byte = self.boot_info.to_bytes()
        img_info_byte += boot_info_byte
        return img_info_byte

class BootInfoEnc():
    def __init__(self):
        self.CP = [0 for x in range(60)]
        self.cfg = 0
        self.swd = 0
        self.enc = 0
        self.CRC = 0
        self.key_bin = [0 for x in range(168)]

    def to_bytes(self):
        boot_info_enc_byte = []
        boot_info_enc_byte += list(map(lambda item: item.to_bytes(1, byteorder='little', signed=False), self.CP))
        boot_info_enc_byte.append(self.cfg.to_bytes(4, byteorder='little', signed=False))
        boot_info_enc_byte.append(self.swd.to_bytes(2, byteorder='little', signed=False))
        boot_info_enc_byte.append(self.enc.to_bytes(2, byteorder='little', signed=False))
        boot_info_enc_byte.append(self.CRC.to_bytes(4, byteorder='little', signed=False))
        boot_info_enc_byte += list(map(lambda item: item.to_bytes(1, byteorder='little', signed=False), self.key_bin))
        return boot_info_enc_byte


class UtilityFunc():
    def __init__(self):
        pass

    def get_spec_macro_arg_str(self, file_path, macro_str):
        # print("get_spec_macro_arg_str")
        with open(file_path, "r+", encoding="utf-8") as f_r:
            lines = f_r.readlines()
        # arg_cache = None
        for line in lines:
            if "#define" in line:
                line = line[7:]
                # print(line)
                arg_head = line.split()[0]
                # print(arg_head)
                if arg_head == macro_str:
                    # arg_cache.append(line.split()[1])
                    # print(arg_cache)
                    return line.split()[1]
                else:
                    continue
            else:
                continue
        # arg_cache.append('')
        return ''

    def do_pad_with_bin(self, bin_file):
        if bin_file:
            bin_size = os.path.getsize(bin_file)
            add_size = 16 - bin_size % 16 if bin_size % 16 != 0 else 0
            # print(bin_size, add_size)

            with open(bin_file, "ab") as f_w:
                for i in range(add_size):
                    fill_value = 0x00
                    f_w.write(fill_value.to_bytes(1, byteorder='little', signed=False))
                return True
        return False

    def check_image_sum(self, data, len):
        check_sum = 0
        for i in range(len):
            check_sum += data[i]
        return check_sum


class GenFirmware():
    def __init__(self):
        self.utilityFunc = UtilityFunc()
        self.boot_info = BootInfo()

        self.config_file = ''
        self.input_bin = ''
        self.out_dir = ''
        self.app_name = ''
        self.dap_mode = ''

    def opt_gen_mode_handler(self):
        # print("gen_firmware")
        run_mode = 0x0b
        hex_addr = 0x01000000
        boot_clock = SYS_CLK_XO16M

        xqspi_mode = 0
        sdk_version = 0
        is_dap_boot = 0
        code_copy_mode = 0

        macro_values = ["APP_CODE_RUN_ADDR", "APP_CODE_LOAD_ADDR", "VERSION", "BOOT_CLOCK", "BOOT_CHECK_IMAGE",
                        "BOOT_LONG_TIME", "COMMENTS"]

        arg_caches = list(map(lambda x: self.utilityFunc.get_spec_macro_arg_str(self.config_file, x), macro_values))

        if arg_caches[3] != '':
            boot_clock = arg_caches[3]

        if boot_clock == 0:  # 64MHz
            xqspi_mode = XIP_SPEED_64M
        elif boot_clock == 1:  # 48MHz
            xqspi_mode = XIP_SPEED_48M
        elif boot_clock == 2 or boot_clock == 4:
            xqspi_mode = XIP_SPEED_16M
        elif boot_clock == 3:
            xqspi_mode = XIP_SPEED_24M
        elif boot_clock == 5:
            xqspi_mode = XIP_SPEED_32M

        # 16 bytes alignment
        self.utilityFunc.do_pad_with_bin(self.input_bin)

        if arg_caches[2] != '':
            sdk_version = arg_caches[2]

        # fill the boot info
        self.boot_info.bin_size = 0x00000000
        self.boot_info.check_sum = 0x00000000
        if arg_caches[0] != '':
            self.boot_info.run_addr = arg_caches[0]
        if arg_caches[1] != '':
            self.boot_info.load_addr = arg_caches[1]
        self.boot_info.xqspi_xip_cmd = run_mode
        self.boot_info.xqspi_speed = xqspi_mode & 0x0F
        self.boot_info.code_copy_mode = code_copy_mode & 0x01
        self.boot_info.boot_clk = boot_clock & 0x07
        if arg_caches[4] != '':
            self.boot_info.check_image = int(arg_caches[4]) & 0x01
        if arg_caches[5] != '':
            self.boot_info.boot_delay = int(arg_caches[5]) & 0x01
        self.boot_info.is_dap_boot = int(is_dap_boot) & 0x01

        info_bin_file = os.path.join(self.out_dir, "info.bin")
        header_bin_file = os.path.join(self.out_dir, "header.bin")

        if not self.gen_fw_info_file(self.input_bin, info_bin_file):
            print("Generate info files fail\n")
            return False
        # fill the image info
        self.img_info = ImgInfo()
        self.img_info.pattern = PatternValue
        self.img_info.version = sdk_version
        self.img_info.boot_info = self.boot_info
        if arg_caches[6] != '':
            if arg_caches[6][0] == '"':
                arg_caches[6] = arg_caches[6][1:]
            if arg_caches[6][-1] == '"':
                arg_caches[6] = arg_caches[6][:-2]
            self.img_info.comments = arg_caches[6]
        else:
            self.img_info.comments = self.app_name[0:MaxCommentsLen]
            # print(self.img_info.comments)

        if not self.gen_fw_header_bin(header_bin_file, info_bin_file):
            print("Generate header bin fail!\n")
            return False

        out_app_fw_bin = os.path.join(self.out_dir, "{}.bin".format(self.app_name))  # ?
        self.gen_app_fw_bin(out_app_fw_bin, header_bin_file, self.input_bin)

        # if os.path.exists(self.input_bin):
        #     os.remove(self.input_bin) 

        if os.path.exists(info_bin_file):
            os.remove(info_bin_file) 

        if os.path.exists(header_bin_file):
            os.remove(header_bin_file) 


    def gen_fw_info_file(self, input_bin, output_info_bin):
        bin_size = os.path.getsize(input_bin)

        with open(input_bin, "rb") as f_r:
            buf = f_r.read()

        self.boot_info.bin_size = bin_size
        self.boot_info.check_sum = self.utilityFunc.check_image_sum(buf, bin_size)

        print("---------------------------------------------------------------")
        print("bin_size       = 0x%08x," %(self.boot_info.bin_size))
        print("check_sum      = 0x%08x," %(self.boot_info.check_sum))
        print("load_addr      = {},".format(self.boot_info.load_addr))
        print("run_addr       = {},".format(self.boot_info.run_addr))
        print("xqspi_xip_cmd  = 0x%02x," %(self.boot_info.xqspi_xip_cmd))
        print("xqspi_speed    = 0x%02x," %(self.boot_info.xqspi_speed))
        print("code_copy_mode = 0x%02x," %(self.boot_info.code_copy_mode))
        print("boot_clk       = 0x%02x," %(self.boot_info.boot_clk))
        print("check_image    = 0x%02x," %(self.boot_info.check_image))
        print("boot_delay     = 0x%02x," %(self.boot_info.boot_delay))
        print("---------------------------------------------------------------")

        with open(output_info_bin, "wb") as f_w:
            bin_info_byte = self.boot_info.to_bytes()
            for x in bin_info_byte:
                f_w.write(x)
        return True

    def gen_fw_header_bin(self, header_bin, info_bin):
        with open(header_bin, "wb") as f_w:
            img_info_byte = self.img_info.to_bytes()
            for x in img_info_byte:
                f_w.write(x)
            for i in range(len(self.img_info.comments)):
                f_w.write(self.img_info.comments[i].encode('utf-8'))
            if (len(self.img_info.comments) < MaxCommentsLen):
                for i in range(MaxCommentsLen - len(self.img_info.comments)):
                    pad=' ';
                    f_w.write(str(pad).encode('utf-8'))
            for i in range(8):
                a = 0
                f_w.write(a.to_bytes(1, byteorder='little', signed=False))
            return True
        return False

    def gen_app_fw_bin(self, app_fw_bin, header_bin, ori_app_bin):
        ori_app_bin_size = os.path.getsize(ori_app_bin)

        with open(app_fw_bin, "wb") as f_w:
            with open(ori_app_bin, "rb") as f_r_1:
                temp_buf_1 = f_r_1.read()
            f_w.write(temp_buf_1)
            if ori_app_bin_size % 16:
                for i in range(16 - ori_app_bin_size % 16):
                    fill_value = 0x00
                    f_w.write(fill_value.to_bytes(1, byteorder='little', signed=False))

            with open(header_bin, "rb") as f_r_2:
                temp_buf_3 = f_r_2.read()
            f_w.write(temp_buf_3)

class MainFunc():
    def __init__(self, argc, argv):
        self.argc = argc
        self.argv = argv

        self.utilityFunc = UtilityFunc()

    def deal_input_cmd(self, opt_mode, input):
        if opt_mode == OPT_MODE_GEN:

            out_dir = os.path.dirname(input)
            bin_file_name = os.path.basename(input)
            tmp = bin_file_name.split('.')    
            targe_name = "OHOS_Image"
            self.opt_gen_param = GenFirmware()
            self.opt_gen_param.config_file = CUSTOM_CONFIG
            self.opt_gen_param.input_bin = input
            self.opt_gen_param.out_dir = out_dir
            self.opt_gen_param.app_name = targe_name
            return self.opt_gen_param.opt_gen_mode_handler()


def main(input = ""):
    TIME = time.strftime("%H:%M:%S")
    DATE = time.strftime("%d %h %Y")
    print("Goodix BLE Tools {} [build time: {}, {}]".format(BleToolVersion, TIME, DATE))
    
    mainFunc = MainFunc(0, 0)
    opt_mode = OPT_MODE_GEN

    if opt_mode == OPT_MODE_INVALID:
        print(tool_usage)
        sys.exit(0)

    mainFunc.deal_input_cmd(opt_mode, input)

def make_bin(input_file = "", output_file = "", list_file=""):
    shell_script =  '''arm-none-eabi-objcopy -O binary -S {src_file} {dst_file}'''.format(src_file = input_file, dst_file = output_file)
    cmd_output = os.system(shell_script)
    
    main(output_file)

    # shell_script =  '''arm-none-eabi-size {src_file}
    #     arm-none-eabi-objdump -D {src_file} > {list_file}'''.format(src_file = input_file, dst_file = output_file,
    #     list_file=list_file)
    # cmd_output = os.system(shell_script)

    return   

if __name__ == "__main__":
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    list_file = sys.argv[3]
    make_bin(input_file, output_file, list_file)
