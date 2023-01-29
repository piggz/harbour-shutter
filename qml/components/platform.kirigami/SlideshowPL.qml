import QtQuick 2.0
import QtQuick.Controls 2.2

SwipeView{
    id:slider
    readonly property bool canShare: false
    property var model

    anchors.fill:parent
    clip:true//setting it make item outside of view invisible
    z: -1
    Repeater {
      model:slider.model
      Image{
         width: slider.width
         height: slider.height
         source: filePath //we use this name in ListModel
         fillMode: Image.Stretch
      }
   }
}
