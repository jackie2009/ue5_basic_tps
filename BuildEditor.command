#!/bin/bash
# 正式编译 basic_tps 编辑器目标（先关闭 UE 编辑器再运行）
UE="/Volumes/SSD_UE_Game/soft/EpicGames/UE_5.8"
PROJ="/Volumes/SSD_UE_Game/projects/ue5_basic_tps/basic_tps.uproject"
"$UE/Engine/Build/BatchFiles/Mac/Build.sh" basic_tpsEditor Mac Development -Project="$PROJ" -WaitMutex 2>&1 | tee "$(dirname "$PROJ")/Saved/BuildEditor.log"
echo; echo "编译结束（日志：Saved/BuildEditor.log），按回车关闭窗口"; read
