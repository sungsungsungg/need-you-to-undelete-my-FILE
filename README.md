# need-you-to-undelete-my-FILE

This is a project given in CS202 class by Professor Tang at NYU.
This project demonstrates my ability to understand the internals of the FAT32 file system and key file system concepts, access and recover files from a raw disk.




## validate usage



```
[root@... cs202]# ./nyufile
Usage: ./nyufile disk <options>
  -i                     Print the file system information.
  -l                     List the root directory.
  -r filename [-s sha1]  Recover a contiguous file.
  -R filename -s sha1    Recover a possibly non-contiguous file.
The first argument is the filename of the disk image. After that, the options can be one of the following:
```
-i
-l
-r filename
-r filename -s sha1
-R filename -s sha1


## print the file system information
If the nyufile program is invoked with option -i, it prints the following information about the FAT32 file system:

Number of FATs;
Number of bytes per sector;
Number of sectors per cluster;
Number of reserved sectors.

```
[root@... cs202]# ./nyufile fat32.disk -i
Number of FATs = 2
Number of bytes per sector = 512
Number of sectors per cluster = 1
Number of reserved sectors = 32
```
For simplicity, nyufile is only invoked while the disk is unmounted.

## list the root directory
If the nyufile program is invoked with option -l, it lists all valid entries in the root directory with the following information:

Filename. Similar to /bin/ls -p, if the entry is a directory, it appends a / indicator.
File size if the entry is a file (not a directory).
Starting cluster if the entry is not an empty file.
It also prints the total number of entries at the end. 

```
[root@... cs202]# ./nyufile fat32.disk -l
HELLO.TXT (size = 14, starting cluster = 3)
DIR/ (starting cluster = 4)
EMPTY (size = 0)
Total number of entries = 3
```

Here are a few assumptions:

Entries marked as deleted are not listed.
The details are not printed inside subdirectories.
There are be no long filename (LFN) entries.
Any file or directory, including the root directory, may span more than one cluster.
There may be empty files.



## recover a small file
If the nyufile program is invoked with option -r filename, it should recover the deleted file with the specified name. The workflow is better illustrated through an example:

```
[root@... cs202]# mount fat32.disk /mnt/disk
[root@... cs202]# ls -p /mnt/disk
DIR/  EMPTY  HELLO.TXT
[root@... cs202]# cat /mnt/disk/HELLO.TXT
Hello, world.
[root@... cs202]# rm /mnt/disk/HELLO.TXT
rm: remove regular file '/mnt/disk/HELLO.TXT'? y
[root@... cs202]# ls -p /mnt/disk
DIR/  EMPTY
[root@... cs202]# umount /mnt/disk
[root@... cs202]# ./nyufile fat32.disk -l
DIR/ (starting cluster = 4)
EMPTY (size = 0)
Total number of entries = 2
[root@... cs202]# ./nyufile fat32.disk -r HELLO
HELLO: file not found
[root@... cs202]# ./nyufile fat32.disk -r HELLO.TXT
HELLO.TXT: successfully recovered
[root@... cs202]# ./nyufile fat32.disk -l
HELLO.TXT (size = 14, starting cluster = 3)
DIR/ (starting cluster = 4)
EMPTY (size = 0)
Total number of entries = 3
[root@... cs202]# mount fat32.disk /mnt/disk
[root@... cs202]# ls -p /mnt/disk
DIR/  EMPTY  HELLO.TXT
[root@... cs202]# cat /mnt/disk/HELLO.TXT
Hello, world.
```


## recover a large contiguously-allocated file
Now, the program recoveres a file that is larger than one cluster. Nevertheless, the assumption is that such a file is allocated contiguously. If no such entry exists, the program prints filename: file not found (replace filename with the actual file name).

## detect ambiguous file recovery requests
There could be more than one deleted directory entry matching the given filename. When that happens, the program prints filename: multiple candidates found (replace filename with the actual file name) and abort.

This scenario is illustrated in the following example:
```
[root@... cs202]# mount fat32.disk /mnt/disk
[root@... cs202]# echo "My last name is Tang." > /mnt/disk/TANG.TXT
[root@... cs202]# echo "My first name is Yang." > /mnt/disk/YANG.TXT
[root@... cs202]# sync
[root@... cs202]# rm /mnt/disk/TANG.TXT /mnt/disk/YANG.TXT
rm: remove regular file '/mnt/disk/TANG.TXT'? y
rm: remove regular file '/mnt/disk/YANG.TXT'? y
[root@... cs202]# umount /mnt/disk
[root@... cs202]# ./nyufile fat32.disk -r TANG.TXT
TANG.TXT: multiple candidates found
```
## recover a contiguously-allocated file with SHA-1 hash
To solve the aforementioned ambiguity, the user can provide a SHA-1 hash via command-line option -s sha1 to help identify which deleted directory entry should be the target file.

In short, a SHA-1 hash is a 160-bit fingerprint of a file, often represented as 40 hexadecimal digits. For the purpose of this lab, it is assumed that identical files always have the same SHA-1 hash, and different files always have vastly different SHA-1 hashes. Therefore, even if multiple candidates are found during recovery, at most one will match the given SHA-1 hash.

This scenario is illustrated in the following example:
```
[root@... cs202]# ./nyufile fat32.disk -r TANG.TXT -s c91761a2cc1562d36585614c8c680ecf5712e875
TANG.TXT: successfully recovered with SHA-1
[root@... cs202]# ./nyufile fat32.disk -l
HELLO.TXT (size = 14, starting cluster = 3)
DIR/ (starting cluster = 4)
EMPTY (size = 0)
TANG.TXT (size = 22, starting cluster = 5)
Total number of entries = 4
```
When the file is successfully recovered with SHA-1, the program prints filename: successfully recovered with SHA-1 (replace filename with the actual file name).

Note: use the sha1sum command to compute the SHA-1 hash of a file:
```
[root@... cs202]# sha1sum /mnt/disk/TANG.TXT
c91761a2cc1562d36585614c8c680ecf5712e875  /mnt/disk/TANG.TXT
```

