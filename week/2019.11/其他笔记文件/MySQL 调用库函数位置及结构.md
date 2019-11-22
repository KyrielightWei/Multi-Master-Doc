### os0file.cc
```
os_is_o_direct_supported()::(fcntl.h::open())
os_is_o_direct_supported()::(unistd.h::close())
os_is_o_direct_supported()::(unistd.h::unlink())
os_file_lock()::(fcntl.h::fcntl())
os_file_create_tmpfile()::(stdio.h::fdopen()) // 涉及到 mf_tempfile.cc
os_file_create_tmpfile()::(unistd.h::close())
os_file_read_string()::(stdio.h::rewind())
os_file_read_string()::(stdio.h::fread())
SyncFileIO::execute()::(unistd.h::pread())
SyncFileIO::execute()::(unistd.h::pwrite())
os_file_punch_hole_posix()::(fcntl-linux.h::fallocate())
os_file_fsync_posix()::(unistd.h::fsync())
os_file_status_posix()::(stat.h::stat())
os_file_create_simple_func()::(fcntl.h::open())
os_file_create_simple_func()::(unistd.h::close())
os_file_set_eof_at_func()::(unistd::ftruncate())
os_file_create_directory()::(stat::mkdir())
os_file_scan_directory()::(dirent::opendir())
os_file_scan_directory()::(dirent::readdir())
os_file_scan_directory:():(dirent::closedir())
os_file_scan_directory()::(unistd::rmdir())
os_file_create_func()::(fcntl::open())
os_file_create_func()::(unistd::close())
os_file_create_simple_no_error_handling_func()::(fcntl::open
os_file_create_simple_no_error_handling_func()::(unistd::close())
os_file_delete_if_exists_func()::(unistd.h::unlink())
os_file_delete_func()::(unistd.h::unlink())
os_file_rename_func()::(stdio::rename())
os_file_close_func()::(unistd::close())
os_file_advise()::(fcntl::posix_fadvise())
os_file_get_size()::(stat::stat())
os_get_free_space_posix()::(statvfs::statvfs())
os_file_get_status_posix()::(stat::stat())
os_file_get_status_posix()::(fcntl::open())
os_file_get_status_posix()::(unistd::close())
os_file_truncate_posix()::(unistd::ftruncate())
os_file_set_eof()::(unistd::ftruncate())
os_file_close_no_error_handling_func()::(unistd::close())
Dir_Walker()::(walk_posix::dirent::opendir())
Dir_Walker()::(walk_posix::dirent::readdir())
os_file_set_nocache()::(fcntl::fcntl())
os_file_seek()::(unistd::lseek())
os_file_copy_func()::(sendfile.h::sendfile())
os_fusionio_get_sector_size()::(fcntl::open())
os_fusionio_get_sector_size()::(unistd::close())
os_fusionio_get_sector_size()::(unistd::unlink())
os_aio_print()::(stdio::fputs())
os_aio_print()::(stdio::putc())
os_aio_print()::(stdio::fprintf())
print_to_file()::(stdio::fprintf())

```

### my_sys.h
```
// percona-server/mysys/mf_tempfile.cc
create_temp_file()::(fcntl.h::open())
create_temp_file()::(stdlib.h::mkstemp())

// percona-server/mysys/my_symlink.cc`
my_readlink()::(unistd.h::readlink()
my_symlink()::(unistd.h::symlink())
my_is_symlink()::(stat.h::lstat())
my_realpath()::(stdlib.h::realpath())

// percona-server/mysys/my_lib.cc`
my_fstat()::(stat.h::fstat())
my_stat()::(stat.h::stat())

// percona-server/mysys/my_delete.cc`
my_delete()::(unistd.h::unlink())

// TODO : more ...
```

#### 可能用到库函数的地方：
```
ut_free()
```

#### 其他需要注意的：
难处理的参数：
1. FILE
2. struct stat
3. DIR
4. struct flock
5. struct statvfs


需要注意的几个点
```
1. error.h::errno   // 暂不清楚要不要改动
2. stat 可能不需要全部信息都传输，要看具体函数，根据需要打包对应信息就好了。
3. dirent 目录操作也是返回的复杂的结构体
4. os_has_said_disk_full
5. aio 相关库函数   // 网络传输用模拟aio, 这些库函数上面没有列出来，
6. fprintf 调用的地方很多，主要是 os0file 中打印函数用到
```

### 库函参数数结构体：
```
1. 
fcntl.h::open {
    send {
        const char *file;
        int oflag;
    }
    return {
        int fd;
    }
}
2. 
unistd.h::close {
    send {
        int __fd;
    }
    return {
        int ret;
    }
}
3. 
unistd.h::unlink {
    send {
        const char *__name;
    }
    return {
        int ret;
    }
}
4.
fcntl.h::fcntl {
    send {
        int __fd;
        int __cmd;
        struct flock lk;
    }
    return {
        int ret;
    }
}
5. 
stdio.h::fdopen {
    send {
        int fd;
        const char *modes;
    }
    return {
        FILE stream;
    }
}
6.
stdio.h::rewind {
    send {
        FILE *stream;
    }
    return {
    }
}
7.
stdio.h::fread {    // 读取 size * n 个字节
    send {
        void *ptr;  // 网络传输可以不传这个参数过去
        size_t size;
        size_t n;
        FILE file;
    }
    return {
        void *ptr;
        size_t n;
    }
}
8. 
unistd.h::pread {
    send {
        int fd;
        void *buf;    // 网络传输可以不传这个参数过去
        size_t nbytes;
        off64_t offset;
    }
    return {
        ssize_t nbytes;
        void *buf;
    }
}
9. 
unistd.h::pwrite {
    send {
        int fd;
        void *buf;    
        size_t nbytes;
        off64_t offset;
    }
    return {
        ssize_t nbytes;
        void *buf;    // 正常情况下，网络传输可以不传这个参数回来
    }
}
10. 
fcntl-linux.h::fallocate {  // 文件打洞
    send {
        int fd;
        int mode;
        off64_t offset;
        off64_t len;
    }
    return {
        int ret;
    }
}
11. 
unistd.h::fsync {   // 刷新系统缓冲区到磁盘
    send {
        int fd;
    }
    return {
        int ret;
    }
}
12. 
stat.h::stat {
    send {
        const char *file;
		struct stat *buf;   // 部分函数调用时，网络传输可以不传这个参数过去
    }
    return {
        struct stat *buf;
        ret;
    }
}
13. 
unistd::ftruncate {
    send {
        int fd；
        off64_t length；
    }
    return {
        int ret;
    }
}
14. 
stat::mkdir {
    send {
        const char *path; 
        usigned int mode;
    }
    return {
        int ret;
    }
}
15.
dirent::opendir {
    send {
        const char *name;
    }
    return {
        DIR *dir;
    }
}
16.
dirent::readdir {
    send {
        const char *name;
    }
    return {
        dirent *direntry;
    }
}
17.
dirent::closedir {
    send {
        DIR *dir;
    }
    return {
        int ret;
    }
}
18.
unistd::rmdir {
    send {
        const char *name;
    }
    return {
        int ret;
    }
}
19.
stdio::rename {
    send {
        const char *old;
        const char *new;
    }
    return {
        int ret;
    }
}
20.
fcntl::posix_fadvise {
    send {
        int fd;
        off64_t offset;
		off64_t len;
		int advise;
    }
    return {
        int ret;
    }
}
21.
statvfs::statvfs {
    send {
        const char *path;
		struct statvfs *stat;
    }
    return {
        struct statvfs *stat;
        int ret;
    }
}
22.
unistd::lseek {
    send {
        int fd;
        off64_t offset;
        int whence;
    }
    return {
        off64_t pos;
    }
}
23.
sendfile.h::sendfile {
    send {
        int out_fd;
        int in_fd;
        off64_t offset;
		size_t count;
    }
    return {
        ssize_t size;
    }
}
24.
stdio::fputs {
    send {
        const char* str;
        FILE *file;
    }
    return {
        int ret;
    }
}
25. 
stdio::putc {
    send {
        const char ch;
        FILE *file;
    }
    return {
        int ret;
    }
}
26.
stdio::fprintf {    // 这个函数参数比较难处理，可以先用snprint 输出到一个字符串中，再用 fputs 调用
    send {
        FILE *file;
        const char *format, ...;
    }
    return {
        int ret;
    }
}

```

### TODO

```
1. 全局查下一以上库函数的头文件，到底哪些 MySQL 源码文件调用了库函数。（扔不能保证是否还有遗漏）
> 已试，头文件包含关系太复杂，不好分析。
2. 在各个库函数调用出打印日志，跑一边日志，看看哪些没有用到。
3. my_sys.h 定义的函数分散在很多 .cc 文件里，需要慢慢找它调库文件的地方。
```

