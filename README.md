# KickBass Assistant

KickBass Assistant 是一个面向现代制作流程的低频协同插件，专门处理 `kick` 和 `bass` 之间的冲突、对齐、ducking 与单声道稳定性。

它不是“再来一个通用 sidechain 插件”，而是一个更聚焦的 low-end workflow tool：

- 插在 `bass` 轨道
- `kick` 从 `sidechain` 输入
- 在一个窗口里完成 `Listen / Offset / Polarity / Duck / Analysis`

## 当前能力

- `VST3` 和 `Standalone` 构建目标
- 低频 polarity 翻转
- bass 微小时序偏移 `Offset`
- sidechain 驱动的低频 ducking
- `Conflict / Mono / Correlation / Detector` 实时分析
- `Alignment hint / Polarity hint` 启发式建议
- 中英双语界面
- 静态产品落地页

## 仓库结构

```text
.
├── CMakeLists.txt
├── README.md
├── .gitignore
├── src/
│   ├── PluginProcessor.h
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h
│   └── PluginEditor.cpp
├── docs/
│   ├── PRD.md
│   └── USER_GUIDE_ZH.md
└── site/
    ├── index.html
    └── styles.css
```

## 构建

环境要求：

- macOS
- Xcode 26.2 或更高
- CMake 3.24 或更高
- 首次配置时需要联网拉取 JUCE

配置：

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
```

构建 VST3：

```bash
cmake --build build --config Debug --target KickBassAssistant_VST3
```

构建 Standalone：

```bash
cmake --build build --config Debug --target KickBassAssistant_Standalone
```

## 使用方式

1. 把插件插在 `bass` 轨道。
2. 把 `kick` 路由到插件的 `sidechain` 输入。
3. 用 `Listen = Both` 监听 kick 和 bass 的关系。
4. 先调 `Duck`，再调 `Offset`，必要时切换 `Flip Low Polarity`。
5. 结合右侧分析区确认：
   - `Conflict` 是否降低
   - `Mono` 是否更稳
   - `Correlation` 是否更安全

推荐起手参数：

```text
Crossover = 90 Hz
Duck = 0.35
Attack = 8 ms
Release = 120 ms
Offset = 0.00 ms
Polarity = Off
```

## 文档

- 产品说明：[docs/PRD.md](/Users/balorclub/Documents/small%20tasks/docs/PRD.md)
- 中文使用手册：[docs/USER_GUIDE_ZH.md](/Users/balorclub/Documents/small%20tasks/docs/USER_GUIDE_ZH.md)
- 静态落地页：[site/index.html](/Users/balorclub/Documents/small%20tasks/site/index.html)

## 开源上传建议

推荐上传这些内容：

- `CMakeLists.txt`
- `README.md`
- `.gitignore`
- `src/`
- `docs/`
- `site/`

不要上传这些内容：

- `build/`
- `.DS_Store`
- 本地生成的 `*.app`、`*.vst3`、`*.dSYM`

## 说明

当前仓库还没有附带开源许可证。如果你准备公开发布到 GitHub，建议在上传前补一个 `LICENSE` 文件。
