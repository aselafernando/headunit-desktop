import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Dialogs 6.0
import QtQuick.Window 6.0
import HUDTheme 1.0

import org.freedesktop.gstreamer.Qt6GLVideoItem 1.0

ThemeRoot {
  id: __root
  property QtObject pluginContext

  GstGLQt6VideoItem {
      id: reverseVideo
      anchors.fill: parent
      objectName: "reverseVideoItem"
      anchors.centerIn: parent
      width: parent.width
      height: parent.height
      Component.onCompleted:  pluginContext.videoItemLoaded(this)
  }

}

/*
ApplicationWindow {
    id: window
    visible: true
    width: 1024
    height: 800
    x: 30
    y: 30
    color: "black"

    Item {
        anchors.fill: parent


        Rectangle {
            color: Qt.rgba(1, 1, 1, 0.7)
            border.width: 1
            border.color: "white"
            anchors.bottom: video.bottom
            anchors.bottomMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            width : parent.width - 30
            height: parent.height - 30
            radius: 8

            MouseArea {
                id: mousearea
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    parent.opacity = 1.0
                    hidetimer.start()
                }
            }

            Timer {
                id: hidetimer
                interval: 5000
                onTriggered: {
                    parent.opacity = 0.0
                    stop()
                }
            }
        }
    }
}
*/
