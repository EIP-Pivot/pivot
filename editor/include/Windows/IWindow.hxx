#pragma once

namespace pivot::editor
{

class WindowsManager;

class IWindow
{
public:
    explicit IWindow(WindowsManager &manager, bool open): m_manager(manager), m_open(open){};
    virtual void render() = 0;
    void setOpen(bool open) { m_open = open; };
    bool isOpen() const { return m_open; };

protected:
    WindowsManager &m_manager;
    bool m_open;
};

}    // namespace pivot::editor