# BP_Hero_Assassin 探索相机与移动配置

## 目标与范围

根据用户提供的《红色沙漠》截图调整：全身角色位于画面偏左下，保留较大的前方环境视野，跟随与转向带有适度缓动。
这些数值是本项目的近似调校，不是该游戏的原始参数。只覆盖 `/Game/AssassinGirl/BP_Hero_Assassin` 的默认值和继承组件模板。

## 相机（长度单位 cm，角度单位 °）

| 配置 | 原值 | 当前值 |
| --- | --- | --- |
| SpringArm.TargetArmLength | 180 | 480 |
| SpringArm.RelativeLocation | (0, 0, 8.49) | (0, 0, 20) |
| SpringArm.TargetOffset（世界空间） | (0, 0, 0) | (0, 0, 35) |
| SpringArm.SocketOffset（弹簧臂局部空间） | (0, 49, 57) | (0, 70, 130) |
| Camera.FieldOfView | 90 | 80 |
| Camera.RelativeRotation.Pitch | 0 | -8 |
| CameraLag | 关闭 | 开启，Speed 10，MaxDistance 35 |
| CameraRotationLag | 关闭 | 开启，Speed 16 |
| CameraLagSubstepping | 开启 | 开启，MaxTimeStep 1/60 s |
| SpringArm.ProbeSize | 12 | 14 |
| SpringArm.InheritRoll | 开启 | 关闭 |

弹簧臂仍使用控制器视角，继承 Pitch/Yaw 并启用碰撞回缩。相机自身不读取控制器旋转，保留相对于弹簧臂的 -8° 俯角。
正 Y 肩部偏移使角色出现在画面左侧；提高相机并使用适度俯角，使角色处于画面下部。

## 移动

| 配置 | 原值 | 当前值 |
| --- | --- | --- |
| BP MoveSpeed（移动逻辑的速度基准） | 360 | 420 cm/s |
| MaxWalkSpeed（初始值） | 350 | 420 cm/s |
| MaxWalkSpeedCrouched | 200 | 160 cm/s |
| MinAnalogWalkSpeed | 20 | 30 cm/s |
| MaxAcceleration | 1500 | 1100 cm/s² |
| BrakingDecelerationWalking | 2000 | 1000 cm/s² |
| GroundFriction | 8 | 5 |
| UseSeparateBrakingFriction | 开启 | 开启 |
| BrakingFriction / Factor | 0 / 1 | 1.5 / 1 |
| RotationRate.Yaw | 500 | 420 °/s |
| AirControl | 0.35 | 0.18 |
| JumpZVelocity | 500 | 450 cm/s |
| GravityScale | 1 | 1.25 |
| UseControllerRotationYaw / Pitch / Roll | 开 / 关 / 开 | 关 / 关 / 关 |

初始采用 OrientRotationToMovement，UseControllerDesiredRotation 保持关闭。
降低加速度、转向速度和制动减速度，让起步、转向和停止更平缓；更低空中控制与更大重力缩短漂浮感。
视觉上的重量感仍受现有动画、混合空间和技能根运动影响，需要在实际关卡试玩时判断。

## 父蓝图中的运行时规则

- `BP_Hero.Move` 会写入 `MaxWalkSpeed = MoveSpeed × TpsRunSpeedByDir(...)`。调整基础速度应同步检查 **MoveSpeed**，只修改 CharacterMovement.MaxWalkSpeed 会被覆盖。保留 Assassin 原有方向速度曲线。
- `AdjustCameraDisByMoveSpeed` 每帧插值修改 SocketOffset.X，Y/Z 保持不变；420 cm/s 时约额外后移 50 cm。这是已有的随速度拉远行为。
- 战斗紧张度超过 0.95 时，父蓝图会开启 UseControllerRotationYaw 并关闭 OrientRotationToMovement；退出时恢复。当前配置保留该战斗朝向规则。
- 现有 IA_Sprint 标志处理保持原样，本次仅调节已有移动配置。

## 维护和验证

在子蓝图的 CameraBoom、FollowCamera 和 CharacterMovement 中直接调节上述参数即可。
覆盖模板由子蓝图的 InheritableComponentHandler 持有，避免影响其他继承 BP_Hero 的角色。

`Scripts/configure_assassin_profile.py` 是可重复应用的配置入口：备份原资产、修改子蓝图、编译并保存、构造角色检查数值。
`Scripts/preview_assassin_profile.py` 在独立进程中重新加载资产，检查保存后的参数和真实相机变换，并输出构图预览。
两个脚本均用于独立 UE 5.7 进程；它们会创建空白验证关卡，不应在带有未保存工作的编辑器中执行。写入前关闭本项目编辑器。

```powershell
& 'E:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'E:\basic_tps_demo\basic_tps.uproject' -run=pythonscript '-script=E:\basic_tps_demo\Scripts\configure_assassin_profile.py' -EnablePlugins=PythonScriptPlugin -DisablePlugins=RiderLink -unattended -nosplash
```

验证输出在 `Saved/AssassinProfile/`：`applied.json`、`reload-verification.json`、`assassin-camera-preview.png`。
预览使用真实蓝图相机和临时静态场景，验证的是中立视角的构图，不代表战斗或移动中的最终动画效果。
