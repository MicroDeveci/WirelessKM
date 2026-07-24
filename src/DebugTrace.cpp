#include "DebugTrace.h"

namespace {
bool g_enabled = true;
}

// Enable or disable debug trace output globally.
// 全局启用或禁用调试跟踪输出。
void DebugTrace::setEnabled(bool enabled)
{
    g_enabled = enabled;
}

// Return whether debug trace output is currently enabled.
// 返回调试跟踪输出当前是否启用。
bool DebugTrace::enabled()
{
    return g_enabled;
}
