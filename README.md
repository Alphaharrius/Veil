# Veil
The implementation of the Veil programming language.

## The Veil virtual machine
The Veil virtual machine is for running the Veil bytecode generated by a compiler from the Veil code, and is implemented by C++.
Future prospects includes adding a JIT compiler, rewrite part of the VM service code base using Veil... etc.

### Structure Diagram
![image](https://user-images.githubusercontent.com/47113671/218408370-62ccc500-b42c-4ad9-a5a5-44ebfab9e195.png)

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
   - [ ] ```OSMutex```
   - [ ] ```OSConditionVariable```

3. [ ] Implement VM specific threading primitives.
    - [x] A low object memory footprint queue-based thread synchronization primitive ```Queue```.
    - [ ] More to be added.

4. [ ] Exchange all existing usage of ```<atomic>``` to the encapsulations.
   - [ ] ```Queue```

5. [ ] Implement the thread management.
   - [ ] All management's functionality should be protected by a mutex.
   - [x] Register all ```VMThread``` upon their start of execution.
     - This is done by calling the method ```threading::Management::register``` at the start of ```VMThread::start```.
   - [ ] Wait for all registered threads to complete their execution from the first to the last.
   - [ ] Method to interrupt all thread at once.

6. [ ] Implement the Veil execution environment.
   - [ ] Design the Veil VM bytecode specifications.
   - [ ] Implement the Veil bytecode interpreter.
   - [ ] Implement ```VeilThread```.
