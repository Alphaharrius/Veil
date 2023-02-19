# Veil

The implementation of the Veil programming language.

## The Veil virtual machine

The Veil virtual machine is for running the Veil bytecode generated by a compiler from the Veil code, and is implemented
by C++.
Future prospects includes adding a JIT compiler, rewrite part of the VM service code base using Veil... etc.

### Structure Diagram

![image](https://user-images.githubusercontent.com/47113671/218408370-62ccc500-b42c-4ad9-a5a5-44ebfab9e195.png)

### Code structure

- Integer like types in the code should use the ```typedef``` defined types in ```fabric/src/typedefs.hpp```.
- All error code of the VM runtime should be type ```uint32``` and reside in a namespace specific for their usage, and
  defined only in ```fabric/src/errors.hpp```.
- Except for special header files, all code under a subdirectory of ```fabric/src``` should be defined in a
  specific ```namespace```.
- All native or OS related platform specific code of a subdirectory of ```fabric/src``` should all be placed
  within ```os.hpp``` & ```os.cpp```, and reside in the namespace of ```veil::os```.
- Platform specific methods under the namespace of ```veil::os``` should all contain an error return
  parameter: ```some_native_method(..., uint32 &error)```.
- All ```class``` that will be part of the VM structure should either be a subtype of ```HeapObject```
  , ```Arenaobject``` or ```ValueObject```.

### Implementation roadmap

1. [ ] Implement the memory management.
    - [x] Implement the core modules.
        - [x] ```Management```
        - [x] ```Pointer```
        - [x] ```Allocator```
        - [x] Interface of ```Algorithm```
    - [ ] Implement the standard management algorithm.

2. [ ] Implement all encapsulations of OS specific threading primitives.
    - [x] ```OSThread```
    - [x] ```OSMutex```
    - [ ] ```OSConditionVariable```

3. [ ] Implement VM specific threading primitives.
    - [x] A low object memory footprint queue-based thread synchronization primitive ```Queue```.
    - [ ] More to be added.

4. [ ] Exchange all existing usage of ```<atomic>``` to the encapsulations.
    - [ ] ```Queue```
        - Suspect the use of mutex alone can already power the ```Queue```.

5. [ ] Implement the thread management.
    - [ ] All management's functionality should be protected by a mutex.
    - [x] Register all ```VMThread``` upon their start of execution.
        - This is done by calling the method ```threading::Management::register``` at the start of ```VMThread::start```
          .
    - [ ] Wait for all registered threads to complete their execution from the first to the last.
    - [ ] Method to interrupt all thread at once.

6. [ ] Implement the Veil execution environment.
    - [ ] Design the Veil VM bytecode specifications.
    - [ ] Implement the Veil bytecode interpreter.
    - [ ] Implement ```VeilThread```.
