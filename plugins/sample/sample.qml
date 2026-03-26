import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Dialogs 6.0
import QtQuick.Window 6.0

import HUDTheme 1.0

ThemeRoot {
  id: __root
  property QtObject pluginContext

  Rectangle {
      id: testRect
      anchors.fill: parent
      objectName: "testRect"
      anchors.centerIn: parent
      width: parent.width
      height: parent.height
  }

}
