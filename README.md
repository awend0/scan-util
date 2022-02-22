# scan-util

C++ multithreaded files scanner

* ```make``` to build
* ```./scan-util.exe [path to scan] [threads count, default: 16] [signatures files, default: ./signatures.txt]``` to run

## signatures file syntax
* ```;``` delimeter for extensions/strings to search
* ```:``` delimeter for groups

```
.ext;.ext2:string to search;string to search 2
```

## 23GB perfomance 
![perfomance](https://github.com/awend0/scan-util/blob/main/screenshots/perfomance.jpg?raw=true)
