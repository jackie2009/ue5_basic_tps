# 战斗目标管理与白色描边维护指南

适用版本：UE 5.7，第三人称法师分支 `codex/third-person-mage`。

系统为玩家自动选择一个敌对目标，以白色外轮廓提示，并向技能提供目标快照。Hero 与敌人共享目标查询和瞬时朝向函数。目标选择不操纵相机、不自动释放技能，也不负责伤害、仇恨或 AI 决策。

## 参考范围

《霍格沃茨之遗》的 Camera Relative Targeting 设置提供两种方向意图：开启时按镜头方向选择，关闭时按移动输入选择，后者可选屏幕外目标。选中的对象以白色轮廓提示；手动锁定是另一种明确输入。[游戏设置条目](https://hogwarts-legacy.fandom.com/wiki/Settings)、[设置操作演示](https://www.youtube.com/watch?v=ELFEmYW5PrM)。

本项目参考上述交互。下面的距离、评分、切换优势、防抖时间、遮挡宽限都是本项目可维护的设计值，不能视作原作算法或内部参数。当前不实现锁定镜头跟随、目标盾色识别、交互物优先级、目标吸附投射物或原作全部辅助瞄准行为。

## 模块职责

| Module | Interface 与职责 | 不应放入的逻辑 |
| --- | --- | --- |
| `UCombatTargetRegistry` | 每个 World 独立登记目标的弱引用，处理卸载和销毁通知 | 玩家选中状态、全局单例、关卡遍历 |
| `UCombatTargetableComponent` | `CanBeTargetedBy`、`GetTargetLocation`、`SetTargetable`；物体加入系统的入口 | 相机、技能和描边 |
| `UCharacterTargetableComponent` | 将角色存活、阵营关系适配成目标资格 | 镜头打分 |
| `UCombatTargetingComponent` | 保存单个选择，自动评估、锁定、切换、提供 `GetActionTarget` | 角色旋转、渲染、技能施放 |
| `UTargetOutlineComponent` | 将本地玩家有效选择显示为白色外轮廓，释放时恢复网格状态 | 目标资格和选择算法 |
| `UCombatFacingUtils` | Hero 与 AI 共享的瞬时 Yaw 朝向 | 持续跟踪、移动锁、自动选目标 |

目标资格是扩展 Seam；人物适配器是知道 `CharacterDataComponent` 的唯一选目标 Adapter。机关、训练假人等对象可添加普通 `CombatTargetableComponent` 并覆盖两个 BlueprintNativeEvent，不必继承 Hero 或修改选择器。普通目标默认只排除自身、隐藏、销毁及禁用对象，业务方必须按需收紧资格。

角色的两个逻辑组件通过 `CreateDefaultSubobject` 创建在 `ACombatCharacter`。Hero 默认关闭自动选目标和描边，可在角色蓝图的 `CombatTargeting` 和 `TargetOutline` 组件上分别开启 `Auto Select` 和 `Show Outline`。AI 可显式调用 `SetSelectedTarget`。这些组件无网络复制，选择是每个控制者的本地状态。

## 自动切换规则

### 资格与目标点

1. 目标必须在当前 World 登记，启用且未隐藏、未销毁；自身不可选。
2. 角色双方都必须存活。Hero 与 Pet 为友方，双方都与 Monster 敌对；Neutral 不参与自动敌对选择。
3. Hero 与 Monster 在 `PreInitializeComponents` 为已有数据组件分别设置默认阵营。运行时变更仍由 `CharacterDataComp.Faction` 决定，资格检查会读取当前值。
4. 设置 `AimSocket` 后优先用网格 Socket；找不到 Socket 则用网格 Bounds 中心，没有有效网格时用 Actor 位置。距离按施法者 Actor 位置到目标点的三维直线距离计算，单位为厘米。
5. 新目标必须同时通过施法者眼睛和相机到目标点的可见性射线。自身与目标本体忽略；默认 `Visibility` 通道。墙必须阻挡该通道。其他角色是否遮挡取决于其碰撞配置。

### 方向与评分

默认每 0.05 秒评估一次，运行在 `TG_PostUpdateWork`。注册表使用弱引用；不会每帧 `GetAllActorsOfClass`。

Camera 模式使用所属玩家控制器的 `GetPlayerViewPoint`，不是全局 Player 0。候选需位于屏幕内且在镜头前方的获取角锥中。转动镜头可以改变候选排名。

Movement 模式使用 Pawn 的 `GetLastMovementInputVector` 的 XY 方向；无输入时退回角色前向。此模式忽略屏幕范围，以水平夹角排序，因此支持屏幕外目标。不能用速度替代输入，否则击退、惯性和 Root Motion 会误选目标。

评分越小越优：

```text
Score = AngleWeight × (夹角 / AcquireHalfAngle)
      + DistanceWeight × (距离 / AcquireRange)
```

夹角贡献优先，距离辅助。同分按 Actor 路径排序，使同一世界的重复评估稳定；不承诺不同机器的网络确定性。对通过几何筛选的候选排序后，仅向排名靠前的对象依次检测遮挡，找到第一个可见候选即可。

### 获取 保留 切换

| 当前状态 | 处理 |
| --- | --- |
| 没有目标 | 立即选取分数最低的可见候选，没有候选则为空 |
| 当前目标有效 | 保留较宽的 RetainRange 和 RetainHalfAngle，不因边缘微小抖动立刻丢失 |
| 出现更优目标 | 必须满足 `新分数 + SwitchAdvantage < 当前分数`，且同一个挑战者持续满足 `SwitchDwellSeconds` 才切换 |
| 挑战者换人或优势消失 | 重新计时，不累计不同敌人的优势时长 |
| 当前目标死亡、失效、超出保留范围或保留角度 | 清除旧目标和锁定；在本次可用评估中重新获取 |
| 死亡或卸载通知 | 立即清除选择；下次 0.05 秒更新重新获取，避免销毁回调里重入注册表 |
| 短暂遮挡 | 最多保留选择 0.2 秒，此时 `GetActionTarget` 返回空、描边消失，不允许靠宽限穿墙施法 |
| 遮挡超过宽限 | 清除后重新找可见候选 |
| 没有本地玩家控制器 | 不进行玩家自动选择；失去控制的 Hero 清除旧选择 |

手动或 AI 模式不自行替换目标。每次更新检查资格和保留距离；可以保留被遮挡的感知目标，但 `GetActionTarget` 会阻止将它当作当前可用施法目标。

### 手动锁定与切换

`SetTargetLocked(true)` 锁住当前可用目标。锁定时镜头偏移、角度变化和更优候选不会自动切换；距离、存活、敌对关系和遮挡仍生效。失效后解除锁定，再恢复自动获取。`SetTargetLocked(false)` 恢复普通选择。锁定只锁选择，镜头仍由现有相机系统控制。

`SwitchTarget(+1)` 选择当前目标屏幕右侧最近的可见敌人，`-1` 选择左侧；同侧没有候选则保持原目标，不循环到另一端。成功切换后进入锁定，防止下一次自动评估立刻改回。应连接 Enhanced Input 的按下事件或摇杆越过阈值事件，每次输入只调用一次。当前没有强制改动项目输入映射。

### 默认调参表

在 BP_Hero 的继承组件 `CombatTargeting` 的 `Settings` 修改。

| 参数 | 默认值 | 调整影响 |
| --- | --- | --- |
| Direction | Camera | 切换镜头意图或移动意图 |
| AcquireRange | 2500 cm | 新目标最大距离 |
| RetainRange | 2800 cm | 已选中目标保留距离，运行时至少等于获取距离 |
| AcquireHalfAngle | 35° | 获取角锥半角，不是全角 |
| RetainHalfAngle | 50° | 保留角锥半角，锁定时忽略角度 |
| AngleWeight | 0.85 | 越大越偏向意图方向 |
| DistanceWeight | 0.15 | 越大越偏向近处敌人 |
| SwitchAdvantage | 0.12 | 越大越黏住当前目标 |
| SwitchDwellSeconds | 0.15 s | 越大越稳定但切换更慢 |
| OcclusionGraceSeconds | 0.20 s | 缓解短暂遮挡，不赋予穿墙攻击能力 |
| VisibilityChannel | Visibility | 与场景碰撞设置共同决定视线 |

若修改评分权重总量，应同步检查 SwitchAdvantage，因为它是绝对分差。目标切换太灵敏时优先增加分差或持续时间；不要用扩大角锥来解决抖动。技能射程仍由技能系统处理，目标管理的 25 米范围不代表所有技能都能命中 25 米。

## 技能与公共朝向函数

`UseSkill` 保留显式 `TryToTarget` 的优先级。未提供目标且数据表 `TargetType == 0`（敌方）时，读取 `TargetingComp.GetActionTarget()`，写入 `InitEffectContext.TargetActor`。自施法与友方技能不填入敌人；无候选仍允许原有空目标施法。当前 EffectContext 的目标类型仍是 `ACombatCharacter*`，新增普通物体目标若需被技能命中，需要另外扩展现有技能上下文。

每次技能启动只捕获一次。技能对象保存的上下文不跟随白描边实时更新；锁定或改变选择不会追踪修改已创建的技能对象。现有武器逻辑若复用同一个 `USkillLogicBase` 实例，不应让多次重叠施法通过可变的 `ActiveSkillLogic` 读取上下文；异步弹体应持有自己的 `FEffectContext` 副本。

在技能蓝图 `ExecuteOnStart` 中使用：

```text
Face To Target Immediately
  Source = Owner
  Target = InitEffectContext.TargetActor
  Update Controller Yaw = false
→ 播放 Montage / 生成技能效果
```

需要“现在就朝当前选择转身”的普通角色逻辑使用：

```text
Face To Selected Target Immediately
  Source = Self 或技能 Owner
```

两个节点都位于 `Combat > Facing`，定义于 `CombatFacingUtils`，不需要 Cast BP_Hero，敌人同样能调用。AI 示例：`TargetingComp.SetSelectedTarget(仇恨系统目标)` 成功后调用 `FaceToSelectedTargetImmediately`，或在技能中传入明确的目标快照。

函数只改 Actor 的 Yaw，保留 Pitch/Roll，不改变相机、移动模式或控制旋转；空目标、自身、销毁目标、无水平朝向、已失效角色返回 false，不回退为朝向镜头。默认不改 Controller。如果角色使用 `bUseControllerRotationYaw`，下一帧可能被控制器覆盖；该角色可显式选择 `Update Controller Yaw=true`（玩家可能因此转动镜头），或由现有 Montage/动作锁管理朝向。这是瞬时函数，不能代替持续面向目标系统。

选中目标版本会再次验证视线和距离；显式目标版本只验证对象和资格，允许技能自行决定射程、延迟和视线规则。现有 `FaceToViewImmediately` 保留，只有需要目标朝向的技能调用新节点，二者不要在同一执行链连续互相覆盖。

## 白色外轮廓

使用 UE 的 Custom Depth/Stencil 与 Post Process Material，材质只输出 Emissive 处理后的场景颜色。[Epic 后处理材质说明](https://dev.epicgames.com/documentation/en-us/unreal-engine/post-process-materials-in-unreal-engine)。

资源为 `/Game/Combat/Targeting/M_TargetOutline`，源码生成器为 `Scripts/create_target_outline.py`。生成器只覆盖这个专用材质；美术若要手改可复制成新材质，然后在 `TargetOutline` 组件指定新资源。

`DefaultEngine.ini` 开启 `r.CustomDepth=3`。选中可用目标的可见 Mesh 写入保留值 **250**；其他系统不得使用该值。材质对八个相邻方向采样，将选中区域向外扩张，再减去原区域，得到白色外轮廓，没有全身白色填充。默认宽度 2，组件 `WidthPixels` 可调 1–6；后处理在 Tonemapping 后执行，白色不随曝光变色。宽度按 CustomDepth 像素定义，改变屏幕百分比或动态分辨率后需要重新验收视觉宽度。

材质同时比较采样处及当前像素的场景深度，隐藏部分不透过墙绘制。逻辑层视线不通过时，本地描边整体撤销。正常取消、切换、停用、销毁、相机切换和失去控制时恢复网格原来的 CustomDepth 开关、Stencil 值及写入掩码；如果其他系统已改走值 250，则不覆盖它的新状态。

材质只挂到当前本地 Hero 的活动 CameraComponent，不创建全局无限 PostProcessVolume；AI 不添加表现组件，Dedicated Server 不加载描边材质。需要该相机的 PostProcessBlendWeight 大于 0。普通不透明/Masked 网格可直接描边，透明材质必须允许 Custom Depth Writes。当前选择时枚举附属网格；选择期间动态新增装备需重新选择以刷新网格集合。

当前按单机单视口实现。CustomDepth 是同一世界共享缓冲，因此存在多个 LocalPlayer 时明确关闭描边，避免分屏互相看到对方目标。未来分屏需要单独的视图掩码方案；联网施法还需要服务器重新验证目标、射程和视线，不能信任客户端选择。

## 加载与维护

新增了原生反射类型和默认子对象。保存当前编辑内容后，关闭并重新打开 UE 编辑器以加载新模块；不要依赖 Live Coding 注入这些结构变化。既有 BP_Hero 和敌人蓝图会继承新增组件，无需在关卡逐个添加。无需修改用户已有的技能表或角色蓝图资产即可获得自动选择和提示。

目标死亡由 `ACombatCharacter.SelfOnDead()` 通知 `SetTargetable(false)`；直接改 HP 也会在下一次评估或技能查询时被排除。复活流程须在恢复 HP 后调用 `TargetableComp.SetTargetable(true)`。停用选择器用组件 `Deactivate`，重新启用用 `Activate`；关闭自动选择和停用组件是两个不同操作。

自动化测试：`Automation RunTests BasicTPS.Targeting`。测试源 `Source/basic_tps/Test/CombatTargetingTests.cpp` 通过公开 Interface 检验敌我资格、遮挡拒绝、死亡和销毁、持续优势切换、显式锁定、距离丢失、Hero/敌人朝向。GPU 对照脚本 `Scripts/verify_target_outline.py` 输出到 `Saved/Tests/TargetOutline`，不保存测试地图。

编辑器人工验收建议：

1. 放置三个有效敌人，镜头经过它们时只出现一个白描边；两个目标边缘小幅晃动不频繁跳动，明确转镜头后可切换。
2. 在获取范围边缘前后移动，观察保留范围的稳定性；转身、拉远、击杀目标后能够清理并重新获取。
3. 隔墙不可新选；当前目标进入墙后时轮廓消失，技能查询为空，超过宽限再换目标。
4. 切换为 Movement，确认移动输入可以指定屏幕外敌人；切回 Camera 后仅选屏幕内敌人。
5. 明确锁定后转镜头不换人，左右切换节点按屏幕顺序选择；解除锁定恢复自动规则。
6. 技能开始后移动镜头，已保存的 TargetActor 不随描边改变；敌人使用同一朝向节点能面向 Hero。
7. 试用角色蒙太奇、Root Motion、不同屏幕百分比与实际透明装备，确认现有角色朝向控制和描边观感。

修改规则时，先更新本文件对应条目，再更新配置默认值及行为测试。不要把特殊技能规则加进通用评分器；技能在自己的目标快照、射程或资格扩展点处理。

## 本次验证记录

2026 年 10 月 29 日在本机 UE 5.7.4 完成 UHT 与 C++ 编译。因既有编辑器持有 DLL，本次用 `-ModuleWithSuffix` 生成独立名称模块，并仅在验证构建时排除 RiderLink 插件；项目插件设置未变。重启编辑器后会按模块清单加载新代码。

`BasicTPS.Targeting` 的三组行为测试通过，报告位于 `Saved/Tests/Targeting/index.json`。BP_Hero 在独立进程中编译，并检查继承组件和 `CombatFacingUtils` 蓝图节点，记录位于 `Saved/Logs/TargetingBlueprints.log`；该检查没有保存现有角色蓝图。

在独立编辑器进程中，用 Manny 骨骼网格与真实 DX12 渲染检查了白色外轮廓、取消选中及完全遮挡的图像；结果在 `Saved/Tests/TargetOutline`。SceneCapture 验证须开启 `always_persist_rendering_state` 并分帧捕获，否则不能作为后处理生效的证据。测试进程自动退出，未保存测试地图。

原有战斗地图的完整技能手感、动态分辨率及特定装备透明材质仍按上方人工验收项检查；自动化通过不代表已完成这些实机操作。
