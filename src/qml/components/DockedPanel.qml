import QtQuick 2.0
import QtQuick.Controls 2.2

Drawer {
    id: panel

    property bool expanded:position >0
    property bool moving: position > 0 && position < 1
    property int dock: dockModes.left
    property bool _immediate

    property real _lastPos
    property real _direction

    property int _managedDock: dock
    property int rotation: 0

    function show(immediate) {
        _immediate = !!immediate
        panel.open()
        _immediate = false
    }

    function hide(immediate) {
        _immediate = !!immediate
        panel.close()
        _immediate = false
    }

    onDockChanged: {
        _immediate = true
        _managedDock = dock
        _immediate = false
    }
}
