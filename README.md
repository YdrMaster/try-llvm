# try-llvm

跟着 llvm 官方教程实现的 Kaleidoscope 语言编译器。

和原版的区别是此处将整个项目整理为一个 CMAKE 项目，从一开始就把代码分散到多个文件中，这样看起来更清晰一些。
由于使用 CMAKE，编译相应也有些变化。

另外，这个项目使用了我更习惯的命名规范（rust 默认命名规范，变量名 snake_mode，常量全大写等等），这种规范更像 STL，但和 llvm 不一致。

原教程的各个章节现在都在这一个仓库中，从第二章开始每一章结束时会打一个 tag：

- [x] [ch2](https://github.com/YdrMaster/try-llvm/releases/tag/ch2)
- [x] [ch3](https://github.com/YdrMaster/try-llvm/releases/tag/ch3)
- [ ] ch4
- [ ] ch5
- [ ] ch6
- [ ] ch7
- [ ] ch8
- [ ] ch8
- [ ] ch10

---

- [原文](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)
- [一个比较老的中文版（2012）](https://llvm-tutorial-cn.readthedocs.io/en/latest/index.html)
- [一个比较新的中文板（2022）](https://zhuanlan.zhihu.com/p/430971659)

## 环境配置

为 vscode 配置 `LLVM_DIR` 环境变量以找到 clang 编译器。

## 使用 Makefile

- 指令
  - `build`（默认）：编译；
  - `run`：编译并立即执行；
  - `clean`：清理 `build` 的产物；
  - `llvm`：下载 llvm 到当前目录内使用，如果本机未安装 llvm 可执行此命令；
- 环境变量
  - `TYPE`：编译模式，`release`（默认） 或 `debug`；
  - `LLVM_VERSION`：`llvm` 命令下载的版本，当前默认 15.0.6；
  - `LLVM_TARGET`：`llvm` 下载的目标平台，默认 `clang+llvm-$(LLVM_VERSION)-x86_64-linux-gnu-ubuntu-18.04`；
  - `LLVM_DIR`：llvm 所在目录，如果使用 `llvm` 命令下载可使用默认值 `llvm-$(LLVM_VERSION)/$(LLVM_TARGET)`；
