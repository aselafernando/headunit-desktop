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
