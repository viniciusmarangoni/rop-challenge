## ROP Challenge

This is an intentionally vulnerable TCP server that can be used to practice ROP.

Since it provides a command to load arbitrary libraries and leak the contents of /proc/self/maps, 
you can practice ROP with different libraries.

### How to use

Use compile.sh to compile both 32 and 64 bits versions of the vulnerable server.

Then run the server and enjoy.


### The provided exploit

In the exploit folder you can find an exploit I made using ROP gadgets available in the libc of my OS.

It is unlikely that this exploit works for you, but maybe you can read the source code and learn something useful.

I made this exploit with the help of SmartChainer (https://github.com/viniciusmarangoni/smartchainer)
