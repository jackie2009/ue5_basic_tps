net use \\192.168.0.239\InfiniteRes /user:InfiniteRes InfiniteRes
robocopy "Binaries" "\\192.168.0.239\InfiniteRes\Binaries" /IS /E /PURGE /R:1 /W:1 /NDL /NFL