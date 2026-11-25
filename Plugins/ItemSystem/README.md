# ItemSystem 插件（道具 / 装备 / 背包）

从 Unity `dark_knight/client` 迁移（Prop / Equip / Bag 模块），与战斗系统完全解耦：
插件不依赖 `basic_tps` 模块，`basic_tps` 也没有引用插件；运行时由 `ItemSystemUISubsystem` 自己挂组件、监听按键。

## 使用
PIE 中按 **I** 打开/关闭（Esc 也能关闭）。左键拿起 / 放下 / 交换，右键快速穿戴 / 卸下。
首次打开时：每个部位第一件测试装备穿上，其余进背包（Project Settings > Plugins > Item System UI 可关闭）。

## 目录
| 目录 | 内容 | 将来 |
|---|---|---|
| `Content/Data/ItemBase.csv` | 测试道具表（字段同 Unity PropBaseVo，EquipType 同 PropEquipEnum） | 可原样导入为 DataTable（行结构 `FItemBaseRow`） |
| `Content/Icons/*.png` | 图标（Unity 原图，136 像素/格），运行时直接读 PNG | 导入为 Texture2D，`Icon` 字段改软引用 |
| `Source/ItemSystem` | Runtime：配置库、道具实例、网格背包、装备栏、属性汇总 | 保留 |
| `Source/ItemSystemUI/.../Logic` | 交互逻辑（拿起/放下/交换/穿戴） | **保留** |
| `Source/ItemSystemUI/.../Widgets` | UI 的 C++ 基类（全部 Abstract，控件用 BindWidget 绑定） | **保留**，作为 WBP 父类 |
| `Source/ItemSystemUI/.../CodeLayout` | 【临时】代码搭建控件树的子类 `*_Code` | **转成 WBP 后整个删除** |
| `Source/ItemSystemUI/.../Subsystem` | 按键开关界面、挂组件、测试数据 | 保留（删 CodeLayout 时去掉对 `_Code` 的默认引用） |

## CodeLayout → WBP 对照表
新建 WBP，父类选左列基类，按右列建好同名控件即可（BindWidget 按名字自动绑定）：

| WBP（建议名） | 父类 | 必须的控件名（类型） |
|---|---|---|
| WBP_ItemIcon | ItemIconWidget | Icon(Image)；可选 Frame(Border) |
| WBP_InventoryGrid | InventoryGridWidget | GridCanvas(CanvasPanel)；可选 GridSizeBox(SizeBox)；类默认值 ItemWidgetClass = WBP_ItemIcon |
| WBP_BagPanel | BagPanelWidget | Grid(WBP_InventoryGrid)；可选 TitleText(TextBlock) |
| WBP_EquipSlot | EquipSlotWidget | Frame(Border)、Icon(Image)；可选 Label(TextBlock) |
| WBP_EquipPanel | EquipPanelWidget | Slot_Head / Slot_Body / Slot_Wrist / Slot_Feet / Slot_Finger1 / Slot_Finger2 / Slot_MainHand / Slot_Neck / Slot_MainHand2 / Slot_MainHand3 / Slot_SubHand / Slot_Belt（均为 WBP_EquipSlot，各自设 EquipSlot），StatsText(TextBlock) |
| WBP_InventoryScreen | InventoryScreenWidget | EquipPanel(WBP_EquipPanel)、BagPanel(WBP_BagPanel)、CarryIcon(WBP_ItemIcon，必须直接放在 CanvasPanel 里)、ItemInfoText(TextBlock) |

最后在 Project Settings > Plugins > Item System UI 把 ScreenWidgetClass 设为 WBP_InventoryScreen，
删除 `CodeLayout/` 两个文件，并去掉 `ItemSystemUISubsystem.cpp` 里对 `UInventoryScreenWidget_Code` 的引用。

## 暂未迁移
品质 / 随机词缀 / 耐久 / 存档 / 仓库与腰带药水栏 / 与战斗属性关联（预留在 FItemInstance.DynamicAttributes 与 UEquipmentStatsLibrary）。
