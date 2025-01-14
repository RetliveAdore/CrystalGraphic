<!--
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-02 08:27:10
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-05 22:49:29
 * @FilePath: \CrystalGraphic\readme.md
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
-->
# 晶体引擎图形模块
## 如何配置编译环境
&emsp;本项目使用了gcc编译器以及make来构建，且涉及到vulkanSDK的使用。首先确保安装了vulkanSDK，然后如果是Windows系统的话，需要将其中的bin文件添加到系统的path目录（这一步通常安装程序会自动完成），然后还需要将include文件夹添加到环境变量中的C_INCLUDE_PATH条目（没有就手动创建一个），最后将Lib文件夹添加到LIBRARY_PATH条目（同样没有就手动创建一个），gcc会从上面两个条目中读取依赖信息；如果是Linux系统，安装脚本会自动执行上面所有操作。至此即可make此项目。