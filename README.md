# Juli Programming Language

README v. 0.0.2

## 1. Getting started

### a) Linux

This procedure was tested only on Ubuntu 12.04 so far. Please report any experiences with other systems.

#### SCons Build System

The project and some dependencies require the scons build system. It can be installed as follows:
* Download scons from http://www.scons.org/
* Extract the archive
* `cd` to the scons directory
* `sudo python setup.py install`

#### Dependencies

In order to be able to compile the project, some dependencies need to be installed:

##### 1. LLVM

* `cd where-you-want-llvm-to-live`
* `svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm`
* `cd llvm/tools`
* `svn co http://llvm.org/svn/llvm-project/cfe/trunk clang`
* `cd ../projects`
* `svn co http://llvm.org/svn/llvm-project/compiler-rt/trunk compiler-rt`
* `cd ..`
* `configure`
* `make`

##### 2. ANTLR3C

* Download `libantlr3c-3.4.tar.gz` from http://www.antlr.org/download/C
* Extract the files to the directoy of your choice. `antlr3c-dir` will refer to this directory in the following.
* `cd antlr3c-dir`
* `mkdir build`
* `cd build`
* `../configue`
 * `../configure --enable-64bit` on 64-bit systems
* `make`

##### 3. cpputils

* `cd where-you-want-cpputils-to-live`
* `git clone git://github.com/Haensel2000/cpputils.git cpputils`
* `cd cpputils`
* scons

#### Compile

* `cd where-you-want-juli-to-live`
* `git clone git://github.com/Haensel2000/juli.git juli`
* `cd juli/compiler`
* `scons llvm=path-to-llvm llvm-build=path-to-llvm/Debug+Asserts cpputils=path-to-cpputils antlr3c=path-to-antlr3c`

#### Running the samples:

* Add `path-to-juli/compiler/build/` to your PATH
* `cd path-to-juli/builder`

To compile the linked_list test sample:
* java -cp bin/:lib/commons-cli-1.2.jar juli.builder.Builder -o test.out -b ../samples/build/ ../samples/src/test.jl ../samples/src/linked_list.jl` 

You can write your own code and compile it the same way.