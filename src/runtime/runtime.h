#ifndef VEIL_RUNTIME_H
#define VEIL_RUNTIME_H

namespace veil {
    
    class Runtime {
    };
    
    class RuntimeConstituent {
    public:
        const Runtime *get_runtime();

        explicit RuntimeConstituent(Runtime &runtime);

    private:
        const Runtime *runtime;
    };
    
}

#endif //VEIL_RUNTIME_H
