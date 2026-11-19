# JOSHVM 升级到 Java ME 8 的详细设计

状态：P1（VM 核心）和 P2 的主体（语言特性所需的类库）已在 Linux i386 和 Win32 i386 目标上实现并通过测试，
见下面的"实施状态"。P3、P4 未开始。

## 实施状态（2026-11-19）

### 已实现并验证

验证环境：Ubuntu 24.04、gcc 13 和 Windows/MSVC、OpenJDK 8。Linux 上的 32 位程序在
`qemu-i386` 下运行。`test/vm/run-tests.sh` 在 debug（`cldc_vm_g`）和 product（`cldc_vm`）两个 VM、
动态 JIT、`-int` 和强制编译 `-comp` 三种模式下全部通过，两种类库配置都跑过。

| 项 | 实现 |
|---|---|
| 版本上限 51 | `JAVA_MAX_SUPPORTED_VERSION`；52 被拒绝 |
| `invokedynamic` | 常量标签 15、16、18 和字节码 0xBA 都以 "invokedynamic is not supported" 拒绝 |
| `StackMapTable` | `ClassFileParser::parse_stackmap_table()`，版本 50 及以上使用，解析时展开成原有内部格式；校验器未改 |
| `ldc` 类常量 | 解释器经 `quicken()` 分派到 `_fast_class_ldc`、`_fast_class_ldc_w`，调用运行时 `ldc_class()`；版本低于 49 的类被校验器拒绝 |
| 数组 `clone` | `ConstantPool::array_clone_method_at()` 把数组类上的 `clone()Object` 绑定到 `JVM.cloneArray()`；`Object` 没有增加方法 |
| 数组类所属的方法引用 | 原先会把数组类当实例类访问（debug 构建触发断言）。现在按 `Object` 的方法解析；字段引用和接口方法引用抛 `IncompatibleClassChangeError` |
| 类库 | `ENABLE_CLDC_8=true` 时用 `-source 1.7 -target 1.7` 编译、不预校验、整体 ROM 化 |
| 新增的类 | `StringBuilder`、`CharSequence`、`Comparable`、`Enum`、`Iterable`、`java.util.Iterator`、`AutoCloseable`、`AssertionError`、`LinkageError`、`IncompatibleClassChangeError`、`NoSuchFieldError`、`java.lang.annotation` 的 7 个类型、`Override`、`Deprecated`、`SuppressWarnings`、`SafeVarargs` |
| 修改的类 | 八个包装类加 `valueOf(基本类型)`；`String`、`StringBuffer` 实现 `CharSequence`（`String` 另实现 `Comparable<String>`）；`Throwable` 加 `addSuppressed`、`getSuppressed`；`Class` 泛型化并加 `desiredAssertionStatus()` |

### 构建和测试

```sh
# 默认配置（CLDC 1.1.1 类库）
./build-linux-i386-cldc.sh -C -J <JDK 8>
# Java 7 语言级别的类库
ENABLE_CLDC_8=true ./build-linux-i386-cldc.sh -C -J <JDK 8>
# 测试（针对最近一次构建）
JDK_DIR=<JDK 8> test/vm/run-tests.sh
```

- 两种配置共用输出目录，切换时必须带 `-C`。
- `PREVERIFY_ORIGINAL=<路径>`：换用自己编译的 `preverify`。仓库里预编译的那个在这台机器上不报错也不产生输出，
  需要从 `preverifier/build/linux` 编译（见 `tools/docker/linux-i386/build-and-test.sh`）。
- `HOST_EXE_RUN=qemu-i386`：在不能直接执行 32 位程序的机器上运行 preverify、loopgen、romgen 和测试。
- `tools/docker/linux-i386/` 和 `.github/workflows/build-i386.yml` 把上述 Linux 和 Win32 i386
  步骤打包了。后者的两个任务分别构建并测试默认类库和 `ENABLE_CLDC_8=true` 类库。

为了让基线在现在的工具链上能构建和运行，另外改了这些地方：

- `cldc/build/share/jvm.make`：gcc 构建加 `-fno-strict-aliasing -fno-delete-null-pointer-checks
  -fno-lifetime-dse -fwrapv -Wno-narrowing`（不加的话 romgen 直接段错误）。**这影响所有用 gcc 的目标**，
  包括没有在这里构建过的 ESP32、STM32 等。
- `jvm.make`、`root.make`：源文件列表不再经过 shell 命令行（超过单参数长度上限），需要 GNU make 4.0 以上。
- `cldc/src/vm/os/linux/OS_linux.cpp`：`MINSIGSTKSZ` 不再是编译期常量。
- `javacall/implementation/linux/cldc/`：去掉 `stropts.h`；Java 堆改用可执行的 `mmap`（原来 `malloc` 的堆上
  JIT 代码无法执行）；定时器线程用条件变量停止，不再用 `pthread_cancel`。

### 与下文设计的偏差

下文各节保留了动手之前的设计原文。实现与它不同的地方：

| 节 | 原设计 | 实际做法 | 原因 |
|---|---|---|---|
| 3 | `cldc8` 目录是 `cldc1.1.1` 的完整拷贝 | `cldc8` 只放新增和改动的文件，构建时覆盖 `cldc1.1.1`（以及 DIO 模块里的 `Comparable`）中同路径的文件 | 避免复制 3.6 万行不变的代码 |
| 4.4 | 方案 A 是"不改写，每次走慢路径"，但没说清解释器怎么分派 | 新增两个只存在于分派表里的字节码，方法里的 `ldc` 永远不被改写。`ldc`、`ldc_w` 的模板从"重做"改成"按返回值分派" | 开启 Isolate 时类镜像是每个任务一份，不能缓存在共享的常量池里；不改写方法也就不用动 GC 栈映射、ROM 写出和调试器 |
| 4.4 | 版本检查没说放在哪 | 类上加一个内部标志位 `JVM_ACC_LDC_CLASS_ALLOWED`（第 21 位），校验器检查 | 校验时已经拿不到 class 文件版本 |
| 4.4 | JIT 遇到类常量放弃编译整个方法 | 在该字节码处生成 uncommon trap，回到解释器 | 现成机制，方法其余部分仍然被编译 |
| 4.8 | 入口形式待定（倾向隐藏的 native） | `com.sun.cldchi.jvm.JVM` 里的 `private final native Object cloneArray()`，接收者就是数组 | 该类在隐藏包里，ROM 化时不改名，native 方法不会被当成死代码删掉 |
| 5 | `Enum.valueOf` 用方案 A：native 按名字读 `$VALUES` | 方案 B：每个常量在构造时登记到 `Enum` 的一张表里 | product 构建的 ROM 镜像不保留字段名，方案 A 对 ROM 里的枚举行不通；方案 B 也不依赖 javac 的命名 |
| 5 | 来源优先从 OpenJDK 7 移植 | 新增的类都是重新写的，改动的类基于 `cldc1.1.1` 的原文件 | 需要的类都很小 |

### 没有做或没有验证的

- **ARM 和 Thumb-2 解释器**：`TemplateTable_arm.cpp`、`TemplateTable_thumb2.cpp` 里的 `bc_fast_class_ldc`
  是照 `bc_new` 写的，**没有编译过也没有运行过**。在这些目标上要先验证类字面量。
- **其他目标**：只构建和测试了 Linux i386。其他平台的 `build-*.sh` 没有加 `ENABLE_CLDC_8`。
- **体积**：第 7 节要求的 Thumb-2 配置下的 ROM、RAM 增量没有测。i386 上的数字见第 7 节。
- **差分测试**（与 HotSpot 的校验结论对比）没有做。模糊测试只覆盖 `StackMapTable` 属性，
  单次运行过 400 个变体（默认 40 个）。
- **P2 里没做的**：`java.io.Closeable` 以及流、连接类实现 `AutoCloseable`；`Vector` 等实现 `Iterable`；
  包装类实现 `Comparable`；`Number`；`Appendable`；现有集合类的泛型化。
- **4.9 的布局约束清单和启动断言**没有做。改动的 `Throwable`、`Class`、`String` 只在末尾加了字段或方法。
- **调试器**（kdp）对新格式类的断点、单步没有验证。
- **应用侧 SDK**（`sdk/`、`test/rule.gmk`）没有改。
- VM 自己抛出的缺字段错误仍然是带文字的 `java.lang.Error`，不是新增的 `NoSuchFieldError`。

## 0. 阅读须知

本文的依据分两类，可信度不同：

- **已核实**：直接读本仓库代码得到的结论，文中给出了文件和行号。
- **待核对**：关于 Java ME 8 规范（CLDC 8 / JSR 360、MEEP 8 / JSR 361）要求的陈述，是凭记忆写的，
  没有对照规范原文。第 10 节列出了仍待核对的项，开工前需要逐条确认。

- **已确认**：项目负责人于 2026-11-18 确认的四项前提（见第 10 节）：class 文件版本上限为 51；
  不支持 `invokedynamic`；VM 必须直接接受 `StackMapTable`；`Object.clone` 不在 CLDC 8 的 `Object` 中。

第 1 节以下是动手之前写的设计，当时没有做过任何构建或运行验证；实现后的情况以上面的"实施状态"为准。

## 1. 目标与范围

### 目标

| 阶段 | 交付物 | 完成标准 |
|---|---|---|
| P1 | VM 能加载并校验 class 文件版本 49–51 | 用 `javac -source 1.7 -target 1.7` 编译、未经 preverify 的类可以运行 |
| P2 | Java 5–7 语言特性所需的基础类库 | 泛型、枚举、foreach、自动装箱、可变参数、断言、字符串 switch、try-with-resources、多重捕获全部可用 |
| P3 | CLDC 8 类库 | 集合框架、NIO 子集、日志、权限模型、GCF 扩展 |
| P4 | MEEP 8 应用模型 | 在 `jams` 之上实现新的应用管理 |

本文对 P1、P2 给出详细设计，对 P3、P4 只给出概要，待 P2 完成后另行细化。

### 非目标

- `invokedynamic`、Lambda、默认方法（class 文件版本 52）。
- 反射（`java.lang.reflect`）和运行时注解访问。
- Oracle TCK 认证。未通过 TCK 之前，对外只能表述为"实现了 CLDC 8 的 API 子集"。

## 2. 现状（已核实）

| 项 | 现状 | 位置 |
|---|---|---|
| class 文件版本 | 接受 45–48 | `cldc/src/vm/share/utilities/GlobalDefinitions.hpp:775-776` |
| 常量池 | 只认 JVMS 第 2 版的标签，其余报 `unknown_constant_tag` | `ClassFileParser.cpp:178-259` |
| 栈映射 | 只解析 CLDC 的 `StackMap` 属性；未知属性直接跳过 | `ClassFileParser.cpp:1228-1257` |
| 栈映射内部格式 | 每个映射点展开成完整帧：`int` 数组（偏移、locals、stack）加类名数组；long/double 占两格 | `ClassFileParser.cpp` 的 `parse_stackmaps` |
| 校验器 | 约 2,700 行，消费上述内部格式 | `cldc/src/vm/share/verifier/` |
| `ldc` 校验 | 引用类型常量一律当作 `java.lang.String` | `VerifyMethodCodes.cpp:394-402` |
| 类属性 | 只处理 `SourceFile`、`InnerClasses`、`Synthetic` | `ClassFileParser.cpp:1401-1434` |
| 访问标志 | 读入时按白名单屏蔽，未识别的位被丢弃 | `ClassFileParser.cpp:624, 948, 1558`；`JvmConst.hpp:98-120` |
| VM 内部标志位 | 复用了 Java 5 新增标志的位置（见 4.5） | `AccessFlags.hpp:37-63` |
| 类库选择 | `ENABLE_CLDC_111` 为真时取 `cldc1.1.1` | `cldc/build/share/root.make:105-114` |
| 类库内容 | `java.lang` 45 个类、`java.util` 13 个类；缺 `StringBuilder`、`Enum`、`Iterable`、`Comparable`、`AutoCloseable`、`AssertionError`；`Boolean` 没有 `valueOf` | `cldc/src/javaapi/cldc1.1.1/` |
| 编译参数 | `-source 1.4 -target 1.4`，共三处 | `jvm.make:59`、`tests.make:44`、`test/rule.gmk:52` |
| 预校验 | 系统类编译后统一过 `preverify` | `jvm.make:699-704` |
| `jcc` | 常量 `JAVA_VERSION = 45`，只认 `StackMap` | `cldc/src/tools/jcc/util/ClassFileConst.java:69`、`components/MethodInfo.java:95` |
| ROM 配置 | 按类名写死初始化和改名规则 | `cldc/src/vm/cldc_rom.cfg` |
| 测试 | `tests.make` 引用的测试源码不在仓库里；`test/` 只有两个示例 | `cldc/build/share/tests.make` |

## 3. 总体策略

1. **新增配置而不是替换**。增加构建开关 `ENABLE_CLDC_8`（默认关），新增类库目录
   `cldc/src/javaapi/cldc8/`，在 `root.make:105` 的选择链最前面加一支。原有 CLDC 1.1.1 配置保持可构建，
   用于低端 MCU 和回归对比。
2. **VM 改动尽量不分叉**。P1 的 VM 改动（版本号、`StackMapTable`、`ldc` 类常量）对旧 class 文件没有副作用，
   不放在 `ENABLE_CLDC_8` 后面，这样两种配置共用同一套校验路径，减少要测的组合。
   只有会增加体积的部分（如数组 `clone`、枚举支持的 native）才受开关控制。
3. **复用现有校验器**。把 `StackMapTable` 在解析时转换成现有内部格式，校验器和 GC 栈映射压缩
   （`StackmapGenerator`）都不用改。详见 4.3。
4. **先搭测试再改代码**。校验器是安全边界，P1 的第一项工作是测试框架（第 8 节）。

## 4. P1：VM 核心

### 4.1 版本号

- `JAVA_MAX_SUPPORTED_VERSION` 从 48 改为 51。
- `ClassFileParser.cpp:719` 有一处对 45.3 的特判，需要读懂上下文后确认对新版本的行为（未细读）。
- 解析状态里已经保存了 `major_version`，后续按版本分支的逻辑都从这里取。

### 4.2 常量池

版本 51 新增三种标签：`MethodHandle`(15)、`MethodType`(16)、`InvokeDynamic`(18)。

设计：**不支持，明确拒绝**（已确认）。不使用 Lambda 的 Java 7 源码不会生成这三种常量。
解析到时抛 `ClassFormatError`，并新增一条专门的错误信息（"不支持 invokedynamic"），
不要落到笼统的 `unknown_constant_tag`，否则用户无法判断原因。

`invokedynamic` 字节码（0xBA）同样在校验阶段拒绝。需要确认 `Bytecodes.hpp` 中 0xBA 当前是否被
VM 内部快速字节码占用（未查）。

### 4.3 StackMapTable

这是 P1 的核心。

**格式差异**

| | CLDC `StackMap`（现有） | `StackMapTable`（新增） |
|---|---|---|
| 偏移 | 每帧绝对偏移 | `offset_delta`，首帧直接取值，其后为"上一帧偏移 + delta + 1" |
| 内容 | 每帧完整列出 locals 和 stack | 相对上一帧的增量：`same`、`same_locals_1_stack_item`、`chop`、`append`、`full_frame` 及扩展形式 |
| 初始帧 | 无 | 由方法描述符隐式给出 |
| 类型项 | 标签 0–8 | 标签 0–8 |

**设计：解析时展开**

在 `ClassFileParser` 中新增 `parse_stackmap_table()`，与 `parse_stackmaps()` 并列，输出同样的
`ObjArray`（每个映射点一对 `stackmap_scalars`、`stackmap_classes`）：

1. 根据方法的访问标志和描述符构造初始帧（实例方法的 0 号局部变量是 `this`；`<init>` 中是未初始化的 `this`）。
2. 维护一个"当前帧"，逐条读取帧记录并就地更新，每条记录产出一个完整帧写入输出数组。
3. long/double 展开成两格（`ITEM_Long` 加 `ITEM_Long_2`），与现有约定一致。
4. 未初始化对象（标签 8，带 `new` 指令偏移）用现有的 `ENCODE_NEWOBJECT` 编码。
5. 未初始化的 `this`（标签 6）映射到现有的 `ITEM_InitObject`。

**解析时必须做的检查**（每一项都要有对应的反例测试）：

- 属性长度与实际消费的字节数一致；一个 `Code` 里不能有两个栈映射属性。
- 每个偏移都小于 `code_length`，且严格递增。
- `chop` 不能削减到负数；`append` 和 `full_frame` 不能超过 `max_locals`、`max_stack`（含 long/double 的第二格）。
- 类型标签在 0–8 之内；`Object` 项的常量池索引有效且指向类。
- 标签 8 的偏移指向一条 `new` 指令（现有的 `check_news` 已经做这件事，确认新路径也经过它）。

**选择哪种属性**

| class 版本 | 行为 |
|---|---|
| 45–49 | 沿用现状，要求 `StackMap`（即必须经过 preverify） |
| 50–51 | 使用 `StackMapTable`；若同时存在 `StackMap` 则忽略后者 |

版本 50 及以上不做类型推导回退：没有映射而又有分支目标的方法直接校验失败。
`jsr`、`ret` 在新版本中不允许；现有校验器对这两条指令的处理未查，需要确认它们会被拒绝。

**内存影响**：展开后的内部格式与现状相同，预计峰值内存没有变化（未测量）。

**待核对**：现有校验器的类型规则是否与 JVMS 第 7 版的类型检查规则等价，尤其是接口赋值
（`VerifierFrame.cpp:629` 对接口目标一律放行）、受保护成员访问、以及 `<init>` 中
`this` 未初始化时的 `putfield`（`VerifyMethodCodes.cpp:774-815` 已有处理）。建议用第 8 节的测试集
与 HotSpot 的校验结果做差分对比。

### 4.4 `ldc` 加载类常量

从版本 49 起，`ldc`、`ldc_w` 可以引用 `CONSTANT_Class`，结果是该类的 `java.lang.Class` 对象。
javac 对 `Foo.class` 就生成这种指令（旧版本生成的是 `Class.forName` 调用）。

需要改动的地方：

| 位置 | 改动 |
|---|---|
| 校验器 `VerifyMethodCodes::push_obj` | 目前把非空引用常量都当 `String`。需要让字节码遍历层把常量标签传下来，类常量压入 `java.lang.Class`。并且仅当 class 版本 ≥ 49 时才允许。 |
| 解释器 | `ldc` 的首次执行路径上增加类常量分支：解析类（可能触发加载并抛 `NoClassDefFoundError`），压入类镜像（`JavaClassObj`）。 |
| 字节码快速化 | `Method.cpp:1380` 附近把 `ldc` 改写成 `_fast_*ldc`。类常量有两个方案：<br>A. 不改写，每次走运行时慢路径。实现最小，性能差。<br>B. 新增一个快速字节码，解析后从常量池直接取镜像。<br>建议先做 A 保证正确，测出热点后再做 B。 |
| C 解释器 | `Interpreter_c.cpp` 的 `fast_ldc` 及其调用处。 |
| 汇编解释器 | `TemplateTable_{arm,thumb,thumb2,i386}.cpp` 中的 `ldc` 模板。方案 A 下只需在慢路径调用运行时。 |
| JIT | `BytecodeCompileClosure` 的常量加载。方案 A 下遇到类常量可以直接放弃编译该方法（回退到解释器），后续再优化。 |
| ROM 化 | `BytecodeOptimizer`、`ConstantPoolRewriter` 需要认识这种用法，保证类常量项在常量池重排和合并后仍然正确。 |

注意：`ldc` 类常量**不**触发类初始化，只触发加载和链接。

### 4.5 访问标志

Java 5 新增的标志与 VM 内部标志位置冲突（`AccessFlags.hpp`）：

| 标准标志 | 值 | 同一位上的 VM 内部标志 |
|---|---|---|
| `ACC_BRIDGE`（方法） | 0x0040 | `JVM_ACC_HAS_MONITOR_BYTECODES` |
| `ACC_VARARGS`（方法） | 0x0080 | `JVM_ACC_HAS_INVOKE_BYTECODES` |
| `ACC_SYNTHETIC` | 0x1000 | `JVM_ACC_HAS_NO_STACKMAPS` |
| `ACC_ANNOTATION`（类） | 0x2000 | `JVM_ACC_SYNTHETIC`（VM 内部定义在第 13 位） |
| `ACC_ENUM` | 0x4000 | `JVM_ACC_HAS_NO_EXCEPTION_TABLE` |

目前读入时按白名单屏蔽，所以这些位会被丢弃，不会污染内部标志。设计上：

- **保持白名单不变**。没有反射，`BRIDGE`、`VARARGS`、`ENUM`、`ANNOTATION` 在运行时没有消费者。
- **唯一要处理的是 `ACC_SYNTHETIC`**：版本 49 起 javac 用标志位而不是 `Synthetic` 属性来标记合成成员。
  读入时若原始标志含 0x1000，则设置内部的 `JVM_ACC_SYNTHETIC`。需要先查清内部 synthetic 标志有哪些消费者
  （调试器、ROM 优化器），如果没有实际用途可以不做。
- 增加一条断言或测试，保证白名单以外的位永远不会进入 `AccessFlags`，防止以后有人放宽白名单时踩到冲突。

### 4.6 新属性

`Signature`、`EnclosingMethod`、`RuntimeVisibleAnnotations`、`RuntimeInvisibleAnnotations`、
`AnnotationDefault`、`LocalVariableTypeTable`、`SourceDebugExtension`、`BootstrapMethods`。

设计：全部**跳过**，现有代码对未知属性已经是这样处理的，不需要改动。
唯一例外是 `BootstrapMethods`：出现即说明类使用了 `invokedynamic`，可以在这里提前报错。

如果 P3 确认 CLDC 8 要求运行时可见注解（待核对），届时再设计存储方式。

### 4.7 桥方法与协变返回

javac 为泛型和协变返回生成桥方法：同名、参数相同、返回类型不同的两个方法并存。
虚方法表若按"名字 + 完整描述符"匹配则不受影响。需要验证两处：

- 虚方法表和接口表的构建代码确实用完整描述符比较，而不是只比参数部分。
- ROM 优化器的方法改名、内联和死方法消除不会把桥方法与目标方法混淆。

这一项预计只需要测试覆盖，不需要改代码。

### 4.8 数组 `clone`

javac 为每个枚举生成的 `values()` 方法体是 `$VALUES.clone()`，即对数组类型调用 `clone`。
`Object.clone` 不在 CLDC 8 的 `Object` 中（已确认），所以**只支持数组 `clone`，`Object` 不增加任何方法**。
这样 `Object` 的虚方法表不变，所有类的 vtable 布局和现有 ROM 镜像结构都不受影响。

**字节码形态**：javac 对 `$VALUES.clone()` 生成 `invokevirtual`，方法引用的所属类是数组类
（如 `[LColor;`），名字 `clone`，描述符 `()Ljava/lang/Object;`，随后跟一条 `checkcast`。

**设计**

- 识别条件：方法引用的所属类是数组类，且名字和描述符如上。三个条件必须同时满足。
- 方法解析：满足条件时不做常规查找（在 `Object` 里找不到），直接解析到 VM 内部的数组拷贝入口。
  入口的实现形式有两个候选，需读过方法解析和快速化代码后选定：<br>
  A. 快速化时把这条 `invokevirtual` 改写成一个新的内部快速字节码，解释器直接调用运行时拷贝。<br>
  B. 解析到一个应用不可见的内部 native 方法，复用现有的调用路径。<br>
  A 不占用方法表、不依赖类库，但四个 CPU 后端和 JIT 都要加一个模板；B 改动面小。倾向 B。
- 运行时行为：接收者为 `null` 抛 `NullPointerException`；否则分配同类型同长度的数组并浅拷贝元素。
  基本类型数组和引用数组都要支持。分配可能触发 GC，实现时接收者必须用句柄保护。
- 校验器：对满足识别条件的调用，接收者必须是数组类型，返回类型按 `Object` 处理。
  其他任何类型上的 `clone` 调用按常规规则处理，即因找不到方法而失败。
- JIT：先不编译含这种调用的方法（回退解释器），或走通用的运行时调用；枚举的 `values()` 不是热点。
- ROM 化：`ROMOptimizer` 和 `ConstantPoolRewriter` 不能把这个方法引用当作"无法解析"而报错或删除。
- 应用代码写 `arr.clone()` 也会生成同样的字节码，因此数组 `clone` 对应用实际可用，
  文档中要写明；非数组对象不能 `clone`。

**编译侧待验证**：用 `-bootclasspath` 指向没有 `Object.clone`、没有 `java.lang.Cloneable` 的类库时，
javac 能否编译枚举和 `arr.clone()`。我的印象是 javac 内部为数组类型自带 `clone` 成员，
并会在缺少 `Cloneable` 时自行补一个空接口，但没有验证过。这是 M0 要做的一个小实验；
如果不成立，需要在类库里提供 `Cloneable`（仅作标记接口）。

### 4.9 VM 与类库的布局耦合

VM 按固定偏移访问部分 Java 类的字段，例如 `String` 的 `value`、`offset`、`count`
（`cldc/src/vm/share/handles/String.hpp:32-64`）和类镜像（`JavaClassObj.hpp:35-46`）；
`Throwable`、`Thread` 预计也类似（未逐一确认）。

规则：在 `cldc8` 类库中修改这些类时，**VM 已知的字段保持原有顺序和类型，新字段只能加在后面**。
P2 开始前先列出全部受约束的类（从 `Universe.hpp` 的系统类模板表和各 `handles/*.hpp` 的偏移定义整理），
写入类库目录下的说明文件，并在 VM 启动自检里加偏移断言（调试构建）。

## 5. P2：语言特性所需的类库

新建 `cldc/src/javaapi/cldc8/`，初始内容从 `cldc1.1.1` 复制，然后按下表增补。

| 语言特性 | javac 生成的依赖 | 需要新增或修改 |
|---|---|---|
| 字符串拼接 | `StringBuilder` | 新增 `StringBuilder`、`CharSequence`、`Appendable`；`String`、`StringBuffer` 实现 `CharSequence` |
| 自动装箱 | 各包装类的 `valueOf(基本类型)` 和 `xxxValue()` | `Boolean` 缺 `valueOf`；其余包装类逐个核对签名；`Number` 抽象类（未查是否已有） |
| foreach | `Iterable.iterator()`、`Iterator` | 新增 `Iterable`、`java.util.Iterator`；`Vector` 等实现 `Iterable` |
| 枚举 | `Enum` 基类、`Enum.valueOf(Class, String)`、`$VALUES.clone()`、枚举 switch 用的 `ordinal()` | 新增 `Enum`、`Comparable`；依赖 4.8；`valueOf` 的实现见下 |
| 断言 | `Class.desiredAssertionStatus()`、`AssertionError` | 新增 `AssertionError`；`desiredAssertionStatus` 先固定返回 `false`，后续可接 VM 选项 |
| 字符串 switch | `String.hashCode()`、`equals()` | 已有 |
| try-with-resources | `AutoCloseable`、`Throwable.addSuppressed()` | 新增 `AutoCloseable`、`java.io.Closeable`；`Throwable` 增加 suppressed 列表（字段加在末尾，见 4.9）；流和连接类实现 `Closeable` |
| 多重捕获、菱形、可变参数、泛型 | 无运行时依赖 | 无 |
| 注解 | 编译期需要注解类型存在 | 新增 `java.lang.annotation` 的基础类型以及 `Override`、`Deprecated`、`SuppressWarnings`、`SafeVarargs` |
| 类字面量 | `ldc` 类常量 | 依赖 4.4 |

**`Enum.valueOf` 的实现**

OpenJDK 用反射调用 `values()`。本项目没有反射，两个方案：

- A. native 方法按名字读取目标类的静态字段 `$VALUES`。不增加 RAM，但依赖 javac 的命名约定，
  并且 ROM 优化器不能改名或删除该字段，需要在 `cldc_rom.cfg` 机制里加一条针对枚举类的通用规则。
- B. `Enum` 构造时把自己登记到一张按类索引的表里。不依赖命名约定，但每个枚举常量多占 RAM。

建议方案 A。其他编译器（如 ecj）生成的字段名可能不同，文档中注明只支持 javac。

**泛型化现有 API**：`Vector`、`Hashtable`、`Enumeration`、`Class` 等加上类型参数。
擦除后二进制兼容，不影响已有应用。

**来源与许可**：新增类优先从 OpenJDK 7 移植并裁剪，其许可（GPLv2 加 Classpath 例外）与本项目的 GPLv2 兼容。
保留原版权头。不得参考或复制 Oracle Java ME Embedded 8 的非开源实现。

## 6. 工具链与构建

| 项 | 设计 |
|---|---|
| javac 参数 | `ENABLE_CLDC_8` 下改为 `-source 1.7 -target 1.7`，三处同步（`jvm.make:59`、`tests.make:44`、`test/rule.gmk:52`），抽成一个变量 |
| 构建用 JDK | 建议固定 JDK 8：同时支持 1.4 和 1.7 目标，且可以用 `-bootclasspath` 编译 `java.lang`。更新的 JDK 编译系统类需要模块相关参数，且较新版本已不支持 1.7 目标（待核对具体版本） |
| preverify | `jvm.make:699-704` 的预校验步骤在 `ENABLE_CLDC_8` 下跳过，类直接带 `StackMapTable` 进入 ROM 化。仓库自带的 `preverify` 不改造（未确认它对新版本 class 文件的行为） |
| romgen | romgen 是以生成器模式构建的 VM，共用 `ClassFileParser`，因此自动获得 P1 的解析能力。需要验证的是 ROM 优化各遍（4.4、4.7） |
| `jcc` | 常量写死为版本 45 且只认 `StackMap`。先确认当前构建流程是否还用到它；若用到则需同步改造，若未用到则不动 |
| ROM 配置 | `cldc_rom.cfg` 增加 `If CLDC_8` 段：新类的 `InitAtBuild`、枚举 `$VALUES` 的保留规则、新包的 `RestrictedPackage` |
| 调试器 | `kdp` 和 `share/debugger` 依赖行号表和局部变量表，格式未变。新增验证项：对含 `StackMapTable` 的类设断点、单步 |
| 应用侧 SDK | `sdk/` 和 `test/rule.gmk` 的应用构建流程去掉 preverify 步骤，文档同步 |

顶层 `build-*.sh` 增加 `ENABLE_CLDC_8` 变量并传给 make。这些脚本的重复问题不在本设计范围内。

## 7. 体积与配置

`Readme.txt` 给出的基线是 192 KB RAM、700 KB ROM。

- P1 只增加解析和校验代码，预计 ROM 增量很小，RAM 无变化。
- P2 增加约二十个类，预计 ROM 有可感知的增长。
- P3 会显著增大 ROM，必须可裁剪。

以上都是预估，没有数据。要求：**每个阶段结束时**在 `javacall_armv7m_gcc`（Thumb-2）配置下测量
ROM 和静态 RAM 的增量并记录在本文档。P3 的类库按包分组，每组一个构建开关。

**已测到的数字**（Linux i386 的 product 版 `cldc_vm`，`size` 输出，2026-11-18；不是 Thumb-2 配置，
只能当作量级参考）：

| 配置 | text | data | bss |
|---|---|---|---|
| 默认（CLDC 1.1.1 类库） | 1,404,926 | 1,418,580 | 61,252 |
| `ENABLE_CLDC_8=true` | 1,415,198 | 1,434,612 | 61,956 |
| 差值 | +10,272 | +16,032 | +704 |

两行都包含 P1 的 VM 改动，差值只反映类库的变化。P1 的 VM 改动本身相对于改动前的增量没有测。

## 8. 测试

现状：仓库里没有可用的 VM 测试集（`tests.make` 引用的源码缺失）。

### 8.1 框架（P1 第一项工作）

- 新建顶层 `test/vm/`，或放到 `tests.make` 期望的位置（需先读 `tests.make` 确认路径）。
- 运行平台：Linux i386，同时跑 C 解释器（`arch = c`）和 i386 汇编解释器加 JIT 两种配置。
- 每个用例是一个带 `main` 的类，输出固定的通过标记；脚本汇总结果。
- 在此基础上接入 CI（Linux i386 和 Win32 i386 构建加冒烟测试）。

### 8.2 用例

| 类别 | 内容 |
|---|---|
| 正向：语言特性 | 第 5 节表格每行至少一个用例；另加泛型桥方法、协变返回、内部类访问合成方法、嵌套 try-with-resources、枚举 switch |
| 正向：StackMapTable | 覆盖全部帧类型，含 long/double 局部变量、`<init>` 中的未初始化 `this`、`new` 后未初始化对象跨分支 |
| 反向：StackMapTable | 4.3 列出的每条检查一个畸形 class 文件，全部必须被拒绝且不崩溃。用字节数组或汇编工具生成 |
| 反向：版本与常量池 | 版本 52 被拒绝；含 `InvokeDynamic` 常量或 0xBA 字节码被拒绝并给出明确信息；版本 48 的类使用 `ldc` 类常量被拒绝 |
| 数组 `clone` | 基本类型数组、引用数组、多维数组、长度为 0 的数组；结果是独立副本；`null` 接收者抛 NPE；枚举 `values()` 两次调用返回不同数组。反向：在非数组类型上的 `clone` 方法引用被拒绝；数组类上名字或描述符不符的方法引用被拒绝 |
| 回归 | 用 1.4 目标编译并 preverify 的旧类在新 VM 上行为不变；`ENABLE_CLDC_8=false` 的构建产物可运行 |
| 差分 | 同一批 class 文件分别交给 HotSpot（`-Xverify:all`）和本 VM，校验结论必须一致 |
| 模糊 | 对合法 class 文件做字节变异后喂给解析器，在 AddressSanitizer 构建下运行，不允许内存错误 |
| ROM | 新类库整体 ROM 化后启动；枚举 `valueOf` 在 ROM 化的系统类和动态加载的应用类上都正确 |

反向用例和模糊测试是硬性要求：`StackMapTable` 解析器处理的是不可信输入，出错即是内存安全问题。

## 9. P3、P4 概要

### P3：CLDC 8 类库

按包分批，每批独立可裁剪。建议顺序（按对应用的价值）：

1. `java.util` 集合框架：`Collection`、`List`、`Map`、`Set`、`ArrayList`、`HashMap`、`Arrays`、`Collections` 等。
2. `java.lang` 补全：`Character` 的 Unicode 支持扩展、`String` 新方法、`ThreadLocal`、`StackTraceElement` 等。
3. `java.io` 补全：`Reader`、`Writer` 系列的缺失类。
4. `java.nio` 子集：`ByteBuffer` 等缓冲区类。直接缓冲区涉及 VM 和 DIO，需要单独设计。
5. `java.security` 权限模型：`Permission`、`AccessController`、`Policy`。与现有 `org.joshvm.security.internal` 的关系需要单独设计。
6. `java.util.logging`。
7. GCF（`javax.microedition.io`）扩展。

具体类和方法清单必须以 JSR 360 规范为准，上面的列表是凭记忆写的。

### P4：MEEP 8

范围最不确定，依赖规范核对。预计涉及应用模型、应用间通信、共享库、事件机制，
以及与现有 `jams`、`ENABLE_ISOLATES` 多任务机制的整合。P3 进行中再启动设计。

## 10. 待核对项

### 已确认（项目负责人，2026-11-18）

| # | 问题 | 结论 | 对设计的影响 |
|---|---|---|---|
| 1 | class 文件版本上限 | 51 | 与 4.1 一致 |
| 2 | 是否支持 `invokedynamic` | 不支持 | 与 4.2 一致 |
| 3 | VM 是否直接接受 `StackMapTable` | 是 | 与 4.3 一致 |
| 4 | `Object.clone` 是否在 CLDC 8 的 `Object` 中 | 不在 | 4.8 改为只支持数组 `clone` |

### 仍待核对

开工前对照规范原文确认。

| # | 问题 | 影响 |
|---|---|---|
| 5 | CLDC 8 是否要求运行时可见注解或任何反射能力 | 决定 4.6 是否需要保存注解 |
| 6 | CLDC 8 对版本 50 以下、没有任何栈映射的类是否要求类型推导校验 | 决定是否需要实现推导校验器（工作量大） |
| 7 | P3 的完整 API 清单 | P3 的工作量 |
| 8 | 较新的 JDK 对 `-target 1.7` 的支持截止版本 | 构建环境文档 |
| 9 | TCK 的获取条件和许可 | 对外的兼容性表述 |

### 代码侧已查清（读代码得出，未经运行验证）

| 问题 | 结论 | 位置 |
|---|---|---|
| 45.3 特判 | 只针对 JDK 1.1 编译的类里 "abstract + strictfp" 方法的放行，与新版本无关，不用改 | `ClassFileParser.cpp:716-722` |
| 0xBA 是否被占用 | 没有，定义为 `_xxxunusedxxx`；字节码遍历里没有它的分支，会落到 `illegal_code` 被拒绝。只需加一条明确的错误信息 | `Bytecodes.hpp:228`、`Method.cpp:2153-2159` |
| `jsr`、`ret` | 字节码遍历里同样没有分支，已经被当作非法指令拒绝，不用改 | `Method.cpp` 的 `iterate_bytecode` |
| 内部 synthetic 标志的消费者 | 只有 `Field::is_synthetic()` 一个访问器，没有找到调用者。结论：4.5 中 `ACC_SYNTHETIC` 的映射可以不做 | `Field.hpp:62` |
| `jcc` 是否在构建流程中 | 在：用它从 `classes.zip` 生成 native 函数表。它读 class 文件时版本检查已被注释掉，但只认 `StackMap` 属性；需要确认它遇到 `StackMapTable` 是跳过还是报错 | `jvm.make:780-782`、`jcc/util/ClassFile.java:104-110` |
| 虚方法表的覆盖匹配 | 同时比较名字和完整描述符，桥方法不会与目标方法混淆 | `InstanceClass.cpp:825` |
| 方法解析路径 | `resolve_invoke_virtual_at` → `lookup_method_at` → `resolve_helper` 取出所属类，再调 `lookup_method`。4.8 的数组 `clone` 识别应放在 `lookup_method_at` 中、调用 `lookup_method` 之前 | `ConstantPool.cpp:381-408, 531-566` |

**调查中发现的一个既有隐患**：`resolve_helper` 把方法引用的所属类直接放进 `InstanceClass` 句柄
（`HotRoutines.incl.hpp:632-647`），没有检查它是不是数组类；随后 `lookup_method_at` 在其上调用
`InstanceClass::lookup_method`。如果 class 文件里的方法引用以数组类为所属类，产品构建下可能把
数组类当成实例类访问。我没有验证它能否实际触发，也没有查校验器是否在更早的地方拦住了。
实现 4.8 时必须一并处理：所属类为数组类时，`clone` 走专用入口，其他名字一律按 `Object` 查找或抛
`NoSuchMethodError`，不能落到现有路径上。

### 代码侧待确认项的结论

- `jcc` 遇到 `StackMapTable`：能正常生成 native 表（`ENABLE_CLDC_8` 构建通过）。
- 4.8 的入口形式：见"实施状态"的偏差表。
- javac 在类库没有 `Object.clone` 和 `Cloneable` 时能编译数组 `clone` 和枚举；但枚举的 `switch`
  需要类库里有 `NoSuchFieldError`。
- 数组类方法引用的隐患能实际触发：改动前 debug VM 在这样的类上断言失败，已修复并有测试。
- 受 4.9 约束的类的完整清单：没有做。

### 构建环境

见"实施状态"一节的"构建和测试"。

## 11. 里程碑与依赖

工作量用相对规模表示（S、M、L），没有做人日估算。

| # | 任务 | 规模 | 依赖 |
|---|---|---|---|
| M0 | 核对第 10 节剩余项；javac 编译枚举的小实验（4.8）；搭测试框架和 CI | M | — |
| M1 | 版本号、常量池拒绝路径、访问标志（4.1、4.2、4.5、4.6） | S | M0 |
| M2 | `StackMapTable` 解析及正反向测试（4.3） | L | M1 |
| M3 | `ldc` 类常量，方案 A（4.4） | M | M1 |
| M4 | 构建开关、`cldc8` 目录、javac 1.7、跳过 preverify（第 3、6 节） | M | M2 |
| M5 | 数组 `clone`（4.8）；布局约束清单和断言（4.9） | M | M4 |
| M6 | P2 类库（第 5 节） | L | M3、M5 |
| M7 | ROM 优化器验证、桥方法验证、调试器验证、体积测量 | M | M6 |
| M8 | `ldc` 类常量方案 B 及 JIT 支持（按测量结果决定是否做） | M | M7 |

M2 与 M3 可以并行。M7 结束即 P1、P2 交付，此时用 Java 7 语法写的应用可以在 JOSHVM 上运行。

## 12. 主要风险

| 风险 | 应对 |
|---|---|
| 校验器规则与 JVMS 7 有细微出入，造成漏洞或误拒 | 差分测试加模糊测试（8.2）；M2 单独评审 |
| ROM 优化器对新模式（桥方法、类常量、枚举字段）处理错误，且只在 ROM 化后暴露 | M7 专门验证；必要时对新类库先关闭相关优化 |
| 体积超出低端 MCU 承受范围 | 保留 CLDC 1.1.1 配置；每阶段测量；P3 按包裁剪 |
| 四个 CPU 后端的 `ldc` 模板改动无法全部实机验证 | 方案 A 把改动限制在慢路径；i386 在 CI 中覆盖，ARM 系列需要硬件或模拟器 |
| 规范理解有误 | M0 先核对第 10 节 |
