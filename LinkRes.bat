net use \\192.168.0.239\InfiniteRes /user:InfiniteRes InfiniteRes
robocopy "\\192.168.0.239\InfiniteRes\Binaries" "Binaries" /IS /E /PURGE /R:1 /W:1 /NDL /NFL