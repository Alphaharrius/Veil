# Veil

Veil is planned to be a gradually typed, and multi-paradigm programming language supports both object-oriented & functional programming.

This project serves as an exercise on ```C++``` and a way to learn how a programming language with a runtime could work under the hood.
My current goal of this project is to complete the runtime, then implement a simple compiler in whatever language I feel comfortable in
(```Python``` or ```Java```), and re-implement the compiler in Veil language as a proof of principle.

Ofcoz there is a much harder quest to implement a just-in-time compiler targeting branches (or I should say it is a trace JIT), which
based on a exposed compiler interface in the runtime, so that we can support any type of compiler backend. The JIT will most likely be
built with a language with memory-safety, maybe in ```Rust``` or even in ```Veil``` itself...

## The Fabric runtime

The Fabric runtime is for running the Fabric bytecode generated by a compiler from the Veil code, and is implemented
by C++, is designed from the ground up to support multi-threaded environment. And to be fully embeddable into other ```C++```
projects, there will be no globally defined states.

To keep the code structure as clear as possible, I planned to use the fully power of OOP provided by ```C++```, and leave all rooms of
performance enhancements & optimizations to the JIT compiler.

Future prospects includes adding a JIT compiler, rewrite part of the VM service code base using Veil... etc.

### Structure Diagram

![image](https://user-images.githubusercontent.com/47113671/218408370-62ccc500-b42c-4ad9-a5a5-44ebfab9e195.png)

### Naming conventions

- Namespaces: ```lowercase``` single word.
- Class: ```CamelCase```
- Methods: ```snake_case```, with single words for public action methods; multi-word for private methods or methods
           that are not supposed to be used or exposed outside of class scope.
- Variables: ```snake_case``` and better to be verbose with at least one ```_``` to avoid collision with comments.
- Attributes: ```snake_case``` and better to be verbose with at least one ```_``` to avoid collision with comments.
- Constants: ```ALL_CAPS``` and use keyword ```const``` but not macros.
- Structs: ```CamelCase``` as a multi-part wrapper; ```snake_case_t``` as a primitive capsule.
- Macros: ```CamelCase```
- Labels: ```CamelCase```

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

1. [ ] Implement utility structures.
   - [ ] ```ArrayList```

2. [ ] Implement the memory management.
    - [x] Implement the core modules.
        - [x] ```Management```
        - [x] ```Pointer```
        - [x] ```Allocator```
        - [x] Interface of ```Algorithm```
    - [ ] Implement the standard management algorithm.

3. [ ] Implement all encapsulations of OS specific threading primitives.
    - [ ] ```Thread```
      - [x] Implement the basic encapsulations.
      - [x] Able to execute a ```VMService```.
      - [ ] Priority settings for all OS threads.
    - [x] ```Mutex```
    - [x] ```ConditionVariable```
    - [x] Atomics
    - [x] Add handling for spurious wakeup of ```ConditionVariable```.

4. [x] Implement VM specific threading primitives.
    - [x] A low object memory footprint queue-based thread synchronization primitive ```OrderedQueue```.

5. [x] Exchange all existing usage of ```<atomic>``` and ```<condition_variable>``` to the encapsulations.
    - [x] ```OrderedQueue```

6. [ ] Implement the thread management.
    - [x] All management's functionality should be protected by a mutex.
      - This is a legacy requirement, since the management (aka. scheduler) is running a single threaded loop to process
        all thread controls.
    - [x] Service spawning & termination using the underlying thread.
    - [x] Thread sleep & wake.
    - [x] Thread interrupt.
    - [x] Thread pause & resume.
    - [ ] Thread joining with another thread.
    - [x] Scheduler termination.

7. [ ] Implement the Veil execution environment.
    - [ ] Design the Veil VM bytecode specifications.
    - [ ] Implement the Veil bytecode interpreter.
    - [ ] Implement ```VeilThread```.
