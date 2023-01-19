# try-llvm

跟着 [llvm 官方教程](https://github.com/llvm/llvm-project/blob/release/15.x/llvm/docs/tutorial/MyFirstLanguageFrontend)实现的 Kaleidoscope 语言编译器。

> 链接的文档虽然是 github 仓库里的，但嵌入的讲解代码仍然不够新，只能看文字部分。实现需要看[对应版本的示例](https://github.com/llvm/llvm-project/blob/release/15.x/llvm/examples/Kaleidoscope)。

和原版的区别是此处将整个项目整理为一个 CMAKE 项目，从一开始就把代码分散到多个文件中，这样看起来更清晰一些。
由于使用 CMAKE，编译相应也有些变化。

另外，这个项目使用了我更习惯的命名规范（rust 默认命名规范，变量名 snake_mode，常量全大写等等），这种规范更像 STL，但和 llvm 不一致。

原教程的各个章节现在都在这一个仓库中，从第二章开始每一章结束时会打一个 tag：

- [x] [ch2](https://github.com/YdrMaster/try-llvm/releases/tag/ch2)
- [x] [ch3](https://github.com/YdrMaster/try-llvm/releases/tag/ch3)
- [x] [ch4](https://github.com/YdrMaster/try-llvm/releases/tag/ch4)
- [ ] ch5
- [ ] ch6
- [ ] ch7
- [ ] ch8
- [ ] ch8
- [ ] ch10

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
  - `OPT`：是否进行对 ir 进行优化，`on`（默认） 或 `off`。优化前的 ir 可以 llvm api 对应上，有助于理解 codegen 的代码；
  - `LLVM_VERSION`：`llvm` 命令下载的版本，当前默认 15.0.6；
  - `LLVM_TARGET`：`llvm` 下载的目标平台，默认 `clang+llvm-$(LLVM_VERSION)-x86_64-linux-gnu-ubuntu-18.04`；
  - `LLVM_DIR`：llvm 所在目录，如果使用 `llvm` 命令下载可使用默认值 `llvm-$(LLVM_VERSION)/$(LLVM_TARGET)`；

## 其他参考资料

- [llvm ir 语法学习](https://github.com/Evian-Zhang/llvm-ir-tutorial)
