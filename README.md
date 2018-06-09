tac
===
`tac` is a utility that concatenates and prints files in reverse.

It aims to eventually become an ISC-licensed replacement for `GNU tac`.

Compiling
---------
```
$ ./configure
$ make
$ sudo make install
```

Requirements
------------
`libbsd`, if you don't have `reallocarray` or `getprogname` functions.

`configure` will figure this out for you.
