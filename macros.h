#pragma once

#define CHECK_RET_BOOL(condition) \
    if (condition)                \
        return false;

#define CHECK_RET(condition)      \
    if (condition)                \
        return;

#define SINGLETON_DECLARE(CLASS) \
public:                          \
    static CLASS* instance() {   \
        static CLASS instance;   \
        return &instance;        \
    }                            \
private:                         \
    CLASS();                     \
    CLASS(const CLASS&);         \
CLASS& operator = (const CLASS&);
