# device_gr551x

## 介绍

以下内容步骤参考[quickstart-lite-env-setup-linux](https://gitee.com/openharmony/docs/blob/master/zh-cn/device-dev/quick-start/quickstart-lite-env-setup-linux.md)

系统要求：Ubuntu16.04 或 Ubuntu18.04 64位系统版本。

编译环境搭建包含如下几步：


1. 安装必要的库和工具
2. 安装python3
3. 安装hb
4. 安装编译工具链
5. 获取源码
6. 确认目录结构
7. 编译流程
8. 烧录

## 工具安装

### 安装repo

1. 创建repo安装目录：命令行输入`mkdir ~/bin`
2. 下载repo：命令行输入`wget https://storage.googleapis.com/git-repo-downloads/repo -P ~/bin/`
3. 改变执行权限：命令行输入`chmod a+x ~/bin/repo`
4. 设置环境变量：在~/.bashrc文件的最后输入`export PATH=~/bin:$PATH`和`export REPO_URL=https://mirrors.tuna.tsinghua.edu.cn/git/git-repo/`
5. 重启shell


### 安装必要的库和工具

> - 通常系统默认安装samba、vim等常用软件，需要做适当适配以支持Linux服务器与Windows工作台之间的文件共享。

> - 使用如下apt-get命令安装编译所需的必要的库和工具：

```
sudo apt-get install build-essential gcc g++ make zlib* libffi-dev e2fsprogs pkg-config flex bison perl bc openssl libssl-dev libelf-dev libc6-dev-amd64 binutils binutils-dev libdwarf-dev u-boot-tools mtd-utils gcc-arm-linux-gnueabi
```

### 安装Python3

1. 打开Linux编译服务器终端。
2. 输入如下命令，查看python版本号：

   ```
   python3 --version
   ```

   如果低于python3.7版本，不建议直接升级，请按照如下步骤重新安装。以python3.8为例，按照以下步骤安装python。

   1. 运行如下命令，查看Ubuntu版本：

   ```
   cat /etc/issue
   ```

   1. 根据Ubuntu不同版本，安装python。
      - 如果Ubuntu 版本为18+，运行如下命令。

        ```
        sudo apt-get install python3.8
        ```
      - 如果Ubuntu版本为16。

        a. 安装依赖包

        ```
        sudo apt update && sudo apt install software-properties-common
        ```

        b. 添加deadsnakes PPA 源，然后按回车键确认安装。

        ```
        sudo add-apt-repository ppa:deadsnakes/ppa
        ```

        c. 安装python3.8

        ```
        sudo apt upgrade && sudo apt install python3.8
        ```
3. 设置python和python3软链接为python3.8。

   ```
   sudo update-alternatives --install /usr/bin/python python /usr/bin/python3.8 1
   sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.8 1
   ```
4. 安装并升级Python包管理工具（pip3），任选如下一种方式。

   - **命令行方式：**

     ```
     sudo apt-get install python3-setuptools python3-pip -y
     sudo pip3 install --upgrade pip
     ```
   - **安装包方式：**

     ```
     curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
     python get-pip.py
     ```

### 安装hb

#### 前提条件

请先安装Python 3.7.4及以上版本，请见[安装Python3](#section1238412211211)。

#### 安装方法

1. 运行如下命令安装hb

   ```
   python3 -m pip install --user ohos-build
   ```
2. 设置环境变量

   ```
   vim ~/.bashrc
   ```

   将以下命令拷贝到.bashrc文件的最后一行，保存并退出。

   ```
   export PATH=~/.local/bin:$PATH
   ```

   执行如下命令更新环境变量。

   ```
   source ~/.bashrc
   ```
3. 执行"hb -h"，有打印以下信息即表示安装成功：

   ```
   usage: hb

   OHOS build system

   positional arguments:
     {build,set,env,clean}
       build               Build source code
       set                 OHOS build settings
       env                 Show OHOS build env
       clean               Clean output

   optional arguments:
     -h, --help            show this help message and exit
   ```

### 安装编译工具链

1. 编译链工具推荐使用gcc-arm-none-eabi-10-2020-q4-major。(下载网站：https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads )。
2. 执行`vim ~/.bashrc`。
3. 将工具链的路径加入环境变量, 如:`export PATH=/home/tools/ARM/gcc-arm-none-eabi-10-2020-q4-major/bin:$PATH`。
3. 保存后执行`source ~/.bashrc`使环境变量生效。

## 获取代码流程

1. 新建代码存放目录(用户可以自行指定为其他目录)，并进入：
```
mkdir ~/openharmony
cd ~/openharmony
```
2. 打包下载所有文件，此时默认的Harmony版本为Master：
```
repo init -u https://gitee.com/openharmony/manifest --no-repo-verify
```
3. 下载好仓库后，输入：`repo sync -c`，也就是下载当前分支的代码。
4. 下载好代码后，输入：`repo forall -c 'git lfs pull'`，下载部分大容量二进制文件。

## 确认目录结构

在device文件夹下，确保device/soc目录结构如下

```shell
user:~/openharmony/device/soc$ tree -L 3
.
└── goodix
    ├── BUILD.gn                                  # GN构建脚本
    ├── gr551x                                    # gr551x soc适配目录
    │   ├── adapter                               # 外设驱动、BLE、文件系统适配
    │   ├── BUILD.gn                              # GN构建脚本
    │   ├── components                            # 组件
    │   ├── hcs                                   # 设备描述文件
    │   ├── Kconfig.liteos_m.defconfig.gr551x     # gr551x Kconfig默认配置
    │   ├── Kconfig.liteos_m.defconfig.series     # series Kconfig默认配置
    │   ├── Kconfig.liteos_m.series               # series Kconfig配置项
    │   ├── Kconfig.liteos_m.soc                  # soc Kconfig配置项
    │   ├── sdk_liteos                            # Liteos GR551x SDK适配
    │   └── tools                                 # 固件生成工具
    ├── Kconfig.liteos_m.defconfig                # liteos_m Kconfig默认配置
    ├── Kconfig.liteos_m.series                   # liteos_m series配置项
    └── Kconfig.liteos_m.soc                      # liteos_m soc配置项
```

在device文件夹下，确保device/board目录结构如下

```shell
user:~/openharmony/device/board$ tree -L 3
.
└── goodix
    ├── BUILD.gn                                  # GN构建脚本
    ├── drivers                                   # 板级驱动存放目录
    │   └── BUILD.gn                              # GN构建脚本
    ├── gr5515_sk                                 # GR5515 Starter Kit开发板配置目录
    │   ├── BUILD.gn                              # GN构建脚本
    │   ├── gr5515_sk_defconfig                   # GR5515 Starter Kit Kconfig默认配置
    │   ├── Kconfig.liteos_m.board                # Board liteos_m Kconfig配置项
    │   ├── Kconfig.liteos_m.defconfig.board      # Board liteos_m Kconfig默认配置
    │   └── liteos_m                              # 构建脚本目录
    ├── hcs                                       # hcs硬件描述配置目录
    │   ├── BUILD.gn                              # GN构建脚本
    │   └── gr5515_sk.hcs                         # GR5515 Starter Kit hcs硬件描述脚本
    ├── Kconfig.liteos_m.boards                   # Board liteos_m Kconfig配置项
    └── Kconfig.liteos_m.defconfig.boards         # Board liteos_m Kconfig默认配置
```

在vendor文件夹下，确保vendor文件夹目录结构如下

```shell
user:~/openharmony/vendor$ tree -L 3
.
└── goodix
    ├── gr5515_sk_iotlink_demo                    # BLE应用示例工程
    │   ├── ble_template                          # BLE示例代码
    │   ├── BUILD.gn                              # GN构建脚本
    │   ├── config.json                           # 子系统裁配置裁剪脚本
    │   ├── hals                                  # 产品参数配置
    │   ├── hdf_config                            # HDF硬件描述配置
    │   └── kernel_configs                        # Kconfig配置输出
    └── gr5515_sk_xts_demo                        # XTS测试示例工程
        ├── BUILD.gn                              # GN构建脚本
        ├── config.json                           # 子系统裁配置裁剪脚本
        ├── tests                                 # 测试用例
        ├── hals                                  # 产品参数配置
        ├── hdf_config                            # HDF硬件描述配置
        └── kernel_configs                        # Kconfig配置输出
```

## 编译工程

进入源码根目录编译工程

   ​	`hb set -root .`   选择当前路径为工程根目录

   ​	`hb set -p `       选择工程

    goodix
        >gr5515_sk_iotlink_demo
         gr5515_sk_xts_demo

   ​	`hb build -f`  开始编译，后续修改了文件只需要执行`hb build -f`即可，不需要重复以上步骤。

如果hb set命令提示报错，请先执行一次`python3 -m pip install build/lite`

## 固件烧录

生成的固件位于**out/{board_name}/{product_name}/bin/application_fw.bin**。

### Windows下固件烧录

#### 软件安装

GProgrammer仅支持在Windows平台下安装使用，其可执行安装程序为GProgrammer Setup Version.exe。[点击下载GProgrammer](https://product.goodix.com/zh/software_tool/gprogrammer)

用户可按照以下步骤安装GProgrammer：

1. 双击GProgrammer的可执行安装程序GProgrammer Setup Version.exe，进入GProgrammer安装界面，然后按照安装向导提示逐步完成安装操作。

![GProgrammer安装界面](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/2_1GProgrammer_insta_if.png)

2. GProgrammer安装完成后，将提示用户继续安装JLink驱动，如下图所示。

![安装JLink提示](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/2_2install_JLink_prom.png)

3. JLink安装完成后，用户可通过桌面或开始菜单的快捷方式启动GProgrammer软件。

#### 硬件连接

GProgrammer既支持JLink SWD方式烧录，也支持串口方式烧录，这里推荐使用SWD连接方式。

选择SWD连接方式时，用户需使用J-Link仿真器来连接PC和目标板。使用USB线将仿真器的一端与PC相连，使用杜邦线将仿真器的另一端与目标板上的芯片管脚相连。

![GProgrammer安装界面](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/3_1Host_obj_connect.png)


J-Link仿真器与芯片管脚的连接对应关系如下：

| J-Link仿真器管脚 | GR551x芯片管脚 |
| ---------------- | -------------- |
| VCC              | VCC            |
| GND              | GND            |
| SWDIO            | GPIO_1         |
| SWCLK            | GPIO_0         |


**提示：** 如过用户使用的是Goodix的GR5515 Starter Kit开发板，开发板已经集成J-Link OB芯片，用户只需使用一根Micro USB 2.0线连接开发板和PC，Mirco USB用于供电以及通过J-Link进行固件烧录。

#### 芯片选型

启动GProgrammer后，默认进入芯片选型页面，如下图所示，根据自己产品或者开发板使用的芯片型号选择对应的芯片型号。

![GProgrammer启动界面](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/3_4filterGR551x_soc.png)

选择芯片型号后，进入主界面：

![GProgrammer主界面](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/3_6GProgrammer_sw_if.png)

软件界面的左侧为功能导航栏（具体描述见下表），右侧为功能操作区域。

|                                                               图标                                                               |     	功能名称     |               描述               |
| ------------------------------------------------------------------------------------------------------------------------------- | ------------------ | -------------------------------- |
| ![Firmware](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_Firmware.png)         | Firmware           | 点击该图标，进入固件操作页面      |
| ![Flash](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_Flash.png)               | Flash              | 点击该图标，进入Flash操作页面     |
| ![Encrypt](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_encrypt_sign.png)      | Encrypt & Sign     | 点击该图标，进入加密加签操作页面   |
| ![eFuse Layout](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_efuse_layout.png) | eFuse Layout       | 点击该图标，进入eFuse展示操作页面 |
| ![Chip config](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_chip_config.png)   | Chip Configuration | 点击该图标，进入芯片配置操作页面   |
| ![Device Log](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_device_log.png)     | Device Log         | 点击该图标，进入设备日志操作页面   |
| ![Help](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_help.png)                 | Help               | 点击该图标，进入帮助操作页面      |


#### 连接设备

用户可管理控制目标板与主机之间的连接。

点击软件界面右上角的![打开连接面板_灰](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_disconnect.png) ，可展开连接管理面板；再次点击![隐藏连接面板_灰](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_disconnect.png) ，可收起并隐藏该面板。

GProgrammer支持SWD和串口两种连接方式。

* SWD连接

在SWD连接方式下，用户只需配置传输速率**Speed**，即可点击**Connect**按钮连接目标板。

<img src="https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/3_7SWD_connect.png" width = "323" height = "450" alt="SWD连接" align=“center” />

* 串口（UART）连接
在串口（UART）连接方式下，用户可根据实际情况配置串口号Port（需点击Refresh按钮获取串口号列表，再选择正确的串口号）和波特率，其他参数默认配置不可修改。

<img src="https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/3_8UART_connect.png" width = "323" height = "450" alt="串口连接" align=“center” />

参数配置完成后，点击**Connect**按钮连接目标板。

目标板连接成功后，连接管理面板将自动收起并隐藏。同时，按钮![打开连接面板_灰](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_disconnect.png)将变成![打开连接面板_彩](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_connect.png)，表示当前的连接状态为“已连接”。

如需断开连接，再次点击![打开连接面板_彩](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/icon_connect.png)，打开连接管理面板，然后点击Disconnect按钮即可。

<img src="https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/3_9disconnect.png" width = "323" height = "450" alt="断开连接" align=“center” />


#### 固件下载

GProgrammer以图形化方式展示Flash Firmware Layout（如图下图所示），可帮助用户直观了解Flash中固件区域的占用情况。

![FW Flash布局](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/3_11Flash_FW_layout.png)

![浅灰色](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/color_light_gray.png)（浅灰色）：可下载的Flash空间。

![深蓝色](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/color_dark_blue.png)（深蓝色）：软件默认配置的NVDS区域。固件不能下载至该区域。

![深灰色](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/color_dark_gray.png)（深灰色）：待删除的固件，如ble_app_ancs。

![浅绿色](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/color_light_green.png)（浅绿色）：待下载的固件，如ble_app_hrs。

![深绿色](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/color_dark_green.png)（深绿色）：已存在于Flash中的固件，如ble_app_bps。

![红色](https://docs.goodix.com/zh/docimg/gprogrammer_user_guide/227/gprogrammer_V2.3/zh//images/color_red.png)（红色）：两个固件的占用空间的重叠区域，如ble_app_T3u和ble_app_hts。

用户下载固件时，请按以下步骤操作：

1. 点击**Add**按钮，从本地添加需下载的固件文件。固件文件添加后，可查看到该固件的详细信息，例如固件文件路径、固件Image Info信息。

2. 在图形化展示的Flash Firmware Layout区域中，选中要启动的固件，点击**Startup**按钮，此时该选中的固件会出现一个小火箭，表示下载成功后立即启动该固件。

2. 点击**Commit**按钮，将固件文件下载到Flash。

下载完成后，若Layout示意图中的固件由浅绿色变为深绿色，则表示固件下载成功。

**提示：** 更详细的GProgrammer使用指导，参考[GProgrammer用户手册](https://docs.goodix.com/zh/online/detail/gprogrammer_user_guide/V2.3/49d7f26f7b5054f6b7ea83e073f8e6c6)

### Linux下固件烧录

#### 1. GProgrammer获取:

```
git clone https://gitee.com/sink-top/gprogrammer-linux.git

```
#### 2. 解压：

```
cd gprogrammer-linux/
tar -xjvf GProgrammer-1.2.15.tar.bz2

```

#### 3. 安装J-Link软件：

```
cd GProgrammer-1.2.15/
sudo dpkg -i JLink_Linux_V618c_x86_64.deb

```

#### 4. 固件下载

参考上面"编译工程"章节，选中**gr5515_sk_xts_demo**工程编译后，生成的固件存放在"~/openharmony/out/gr551x/gr5515_sk_xts_demo/bin/application_fw.bin"。

##### 1. 硬件连接

J-Link仿真器与开发板或者产品正确连接，且J-Link仿真器的USB与Ubuntu连接，确保Ubuntu USB列表能找到J-Link设备。

##### 2. 固件下载

在GProgrammer-1.2.15目录下：

1. 先擦除Flash上已经存在的固件

* 全部擦除，包括文件系统、NVDS区域保存的用户信息，如果用户不关心这些信息，执行此命令：

```
./GR551x_console eraseall
```

* 擦除部分区域，保留文件系统、NVDS区域保存的用户信息：

```
 ./GR551x_console erase 0x1000000 0x10c3000 y
```

上面两种擦除方式，二选一执行。

2. 烧录固件：

```
 ./GR551x_console program ~/openharmony/out/gr551x/gr5515_sk_xts_demo/bin/application_fw.bin y
```

**提示：** 更多GProgrammer命令的使用，请参考[GProgrammer命令行](https://docs.goodix.com/zh/online/detail/gprogrammer_user_guide/V2.3/c2309a4b419cd3bb0d5cb6b7335c077d)

## 相关仓库

[vendor_goodix](https://gitee.com/openharmony/vendor_goodix)

[device_board_goodix](https://gitee.com/openharmony/device_board_goodix)

