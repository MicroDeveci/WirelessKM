#pragma once

class DebugTrace
{
public:
    // Enable or disable debug trace output globally.
    // 全局启用或禁用调试跟踪输出。
    static void setEnabled(bool enabled);
    // Return whether debug trace output is currently enabled.
    // 返回调试跟踪输出当前是否启用。
    static bool enabled();
};
