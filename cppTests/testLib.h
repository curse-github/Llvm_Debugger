extern int numFunctions;
extern const char* functionNames[];
extern int functionParamCounts[];
extern const char** functionParamNames[];
extern const char** functionParamTypes[];
typedef void(*wrapperFT)(void*);
extern wrapperFT functionPointers[];