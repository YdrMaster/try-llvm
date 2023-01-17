# try-llvm

- [一个比较老的中文版（2012）](https://llvm-tutorial-cn.readthedocs.io/en/latest/index.html)
- [一个比较新的中文板（2022）](https://zhuanlan.zhihu.com/p/430971659)
- [原文](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)

## 环境配置

为 vscode 配置 `LLVM_DIR` 环境变量以找到 clang 编译器。

编译需要调用 `llvm-config`，可以通过将 `$LLVM_DIR/bin` 加入 `PATH` 实现。

## 使用 Makefile

- 指令
  - `build`（默认）：编译；
  - `clean`：清理；
- 环境变量
  - `TYPE`：`Release`（默认） 或 `Debug`；
