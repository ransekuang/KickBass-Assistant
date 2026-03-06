# KickBass Assistant 使用说明

## 这是什么

KickBass Assistant 是一个专门处理 `kick` 和 `bass` 低频关系的插件。

它不是通用的美化插件，也不是一键自动混音工具。它的目标很明确：

- 让你更快看见 kick 和 bass 的低频冲突
- 提供一组可控的手动修正工具
- 帮你更快找到更稳的低频 pocket

推荐使用方式：

- 插在 `bass` 轨道上
- 把 `kick` 发送到插件的 `sidechain` 输入

## 最快上手

1. 把插件插在 `bass` 轨道。
2. 把 `kick` 路由到插件的 `sidechain`。
3. 循环播放 kick 和 bass 最密集的一段。
4. 先把 `Listen` 设为 `Both`。
5. 用下面这组起手参数开始：

```text
Crossover = 90 Hz
Duck = 0.35
Attack = 8 ms
Release = 120 ms
Offset = 0.00 ms
Polarity = Off
```

6. 先调 `Duck`，让 kick 更清楚。
7. 再微调 `Offset`，让 kick 和 bass 的落点更顺。
8. 如果单声道下低频发虚，再试 `Flip Low Polarity`。
9. 最后用 `Output` 对齐处理前后的音量，再判断是否真的更好。

## 信号路由

正确路由是这个结构：

```text
Kick track  ->  plugin sidechain input
Bass track  ->  plugin main input
Plugin out  ->  processed bass
```

注意：

- 这个插件默认处理的是 `bass` 主输入
- `kick` 只作为 sidechain 控制和分析来源
- 如果右侧显示 `Sidechain: waiting for kick input`，说明 sidechain 还没有正确送进来

## 界面说明

### Header

- `Language / 语言`
  - 切换界面语言
  - 语言设置会跟随工程一起保存

### Controls

- `Listen`
  - `Bass`：只听处理后的 bass
  - `Kick`：只听 sidechain kick
  - `Both`：同时听 kick 和 bass
  - `Mono`：把处理后的 bass 折叠为单声道检查
  - `Delta`：只听插件改动掉的部分

- `Flip Low Polarity`
  - 只翻转低频段极性
  - 适合处理 kick 和 bass 互相抵消、或者 mono 下低频变薄的问题

- `Offset`
  - 微调 bass 相对 kick 的时间位置
  - 正值表示 bass 更晚
  - 负值表示 bass 更早
  - 常用微调范围：`0.10 ms` 到 `1.50 ms`

- `Crossover`
  - 设定从哪个频率以下开始参与分析、极性翻转和 ducking
  - 推荐起手值：`80 Hz` 到 `100 Hz`

- `Duck`
  - 控制 kick 出现时，bass 低频被压下去多少
  - 数值越大，kick 越容易穿出来
  - 太大时 bass 会变空

- `Attack`
  - 控制 ducking 启动速度
  - 更快会让 kick 前沿更清楚
  - 太快可能会显得过硬

- `Release`
  - 控制 ducking 松开速度
  - 太短会抽动
  - 太长会让 bass 长时间发虚

- `Output`
  - 最终输出补偿
  - 用来做处理前后的公平音量对比

### Analysis

- `Sidechain status`
  - 显示当前是否收到 kick 的 sidechain

- `Conflict`
  - 低频重叠程度
  - 不是越低越好，目标是既分开又保留厚度

- `Mono`
  - 单声道兼容性
  - 越稳越好
  - 如果这里明显变差，优先试 `Polarity`，再试 `Offset`

- `Correlation`
  - kick 和 bass 低频同向程度
  - 偏低通常表示更容易打架或互相抵消

- `Detector`
  - 当前 kick 对 ducking 的触发强度
  - 越高说明当前低频 ducking 越强

- `Alignment hint`
  - 根据当前分析窗口给出的 `Offset` 起点建议
  - 它只是建议，不是自动应用

- `Polarity hint`
  - 根据当前分析结果给出的极性建议
  - 只有当听感更稳、更厚时才保留这个设置

### Help

- 鼠标移到参数或 meter 上
- 底部帮助区会显示当前模块的说明

## 推荐工作流

### 场景一：kick 被 bass 吃掉了

先这样试：

1. `Listen` 设为 `Both`
2. 提高 `Duck`
3. 把 `Attack` 稍微调快
4. 把 `Offset` 往正值方向推一点

观察：

- `Conflict` 是否下降
- kick 是否更清楚
- bass 是否仍然有重量

### 场景二：bass 太空、太薄

先这样试：

1. 降低 `Duck`
2. 把 `Crossover` 稍微往下调
3. 缩短 `Release`
4. 对比 bypass，确认是不是只是音量变小了

### 场景三：单声道下低频发虚

先这样试：

1. `Listen` 切到 `Mono`
2. 切换 `Flip Low Polarity`
3. 微调 `Offset`
4. 观察 `Mono` 和 `Correlation`

### 场景四：抽吸感太明显

先这样试：

1. 降低 `Duck`
2. 把 `Release` 调短一点
3. 如果 kick 前沿不够清楚，再把 `Attack` 略微调快

## 三套实用起手参数

### 通用起手

```text
Crossover = 90 Hz
Duck = 0.35
Attack = 8 ms
Release = 120 ms
Offset = 0.00 ms
```

### 更强调 kick 穿透

```text
Crossover = 95 Hz
Duck = 0.45
Attack = 5 ms
Release = 100 ms
Offset = +0.30 ms
```

### 更保守、更自然

```text
Crossover = 80 Hz
Duck = 0.22
Attack = 10 ms
Release = 140 ms
Offset = 0.00 ms
```

## 常见问题

### 1. 为什么 meter 没怎么动？

可能原因：

- 没有正确送入 sidechain
- kick 本身低频不多
- `Crossover` 设得太低或太高

先检查右侧 `Sidechain` 状态，再检查 DAW 路由。

### 2. 为什么听起来变化很小？

可能原因：

- 当前 kick 和 bass 本来就没有太强冲突
- `Duck` 太小
- 你在 `Bass` 模式下只听处理后 bass，差异不容易感知

建议切到 `Both` 或 `Delta` 再判断。

### 3. 为什么建议值和我的耳朵不一致？

正常。

这个插件的 `hint` 是启发式建议，不是绝对正确答案。最终仍然应该以：

- 整体 groove
- mono 稳定性
- 和主歌/副歌上下文的匹配

为准。

### 4. `Output` 要怎么用？

处理后如果整体更小声，人耳很容易误判成“变差”。

所以最后一步建议：

1. 调 `Output`
2. 让处理前后主观音量接近
3. 再做开关对比

## 使用建议

- 不要一上来就大幅调 `Offset`
- 不要默认 `Conflict` 越低越好
- 不要只看 meter，不听上下文
- 先让 kick 和 bass 在主段落里成立，再去追求极致分离

## 当前版本边界

当前版本更适合做这些事：

- kick / bass 低频冲突处理
- 低频 pocket 微调
- 低频单声道稳定性检查

当前版本不负责这些事：

- 自动帮你完成完整混音
- 替代动态 EQ、总线压缩器或总控处理
- 自动识别最优艺术判断

## 文档版本

- Product: `KickBass Assistant`
- Guide: `v0.1`

