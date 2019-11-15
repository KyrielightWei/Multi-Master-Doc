# Percona-Server Script

> Percona-Share-Storage/script/
>
> 注：脚本传参数的方式是运行脚本时，在脚本后直接加参数，如 `sh percona_make.sh 16`

## script/percona_build_env.sh

处理编译后文件存放位置的build的目录，默认新建build文件夹于git项目的根目录下。

使用export  PERCONA_BUILD_PATH定义环境变量后，会在环境变量指定路径下新建build目录，会将编译结果输出到build中。

> 该脚本是后续脚本的基础，也可以通过定义环境变量的方式指定包含编译结果的build目录所在的位置，使用后续的脚本进行启动和连接

## script/percona_cmake.sh

cmake脚本，生成Makefile到build目录

## script/percona_make.sh

在build目录下执行make命令

+ 参数1：并行编译的线程数

## script/percona_server_init.sh

初始化Percona-Server

+ 参数1：OUTPUT_DIR，输出目录，即percona的错误日志、数据文件、socket文件、pid文件等存放的位置。【默认值：/tmp/percona_output】
+ 参数2：PORT，percona接收连接的端口。【默认值：9898】

## script/percona_server_run.sh

启动percona服务进程

+ 参数1：USER，当前启动进程的用户名，使用root用户启动时必须传入root，其余场景依据实际情况确定。【默认值：空值】

## script/percona_server_connect.sh

连接到percona-server，进入mysql shell界面

+ 参数1：MYSQL_USER，mysql用户名
+ 参数2：MYSQL_PORT，mysql访问端口
+ 参数3：MYSQL_IP，mysql所在ip地址

三个参数是连接必需的，输入过一次以后，脚本会将参数的值记录到percona_connect_impl.json文件中，后续调用脚本时可以不用输入参数，默认会从json文件读取参数值。

json文件中只记录最近一次参数的输入，也可以通过手动更改json文件的方式改变连接时的参数

> mysql密码需要运行脚本后手动输入

## script/percona_server_shutdown.sh

关闭percona-server进程