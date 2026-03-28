import QtQuick
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.0
import QtMultimedia 5.7
import Qt.labs.settings 1.0
import Qt5Compat.GraphicalEffects
import QtQml 2.3

import HUDTheme 1.0

ThemeRoot {
    id: __root

    clip: true

    Item {
        id: main
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: stationDrawer.right

        Item {
            id: wrapper
            anchors.fill: parent
            anchors.leftMargin: parent.width * 0.1
            anchors.rightMargin: parent.width * 0.1
            anchors.bottomMargin: parent.height * 0.1
            anchors.topMargin: parent.height * 0.1
            Rectangle {
                color: HUDStyle.colors.formBackground
                anchors.fill: parent
            }
            Item {
                id: track_info
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: buttons.top
                anchors.bottomMargin: 16
                anchors.topMargin: 16
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                anchors.left: parent.left
                RowLayout {
                    id: signalStrength
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    spacing: 2

                    Accessible.name: qsTr("Signal noise ratio: " + pluginContext.RadioController.snr )

                    Rectangle{
                        height: 4
                        width: 4
                        color: (pluginContext.RadioController.snr > 2) ? "green" : "dimgrey"
                        Accessible.ignored: true
                    }
                    Rectangle{
                        height: 8
                        width: 4
                        color: (pluginContext.RadioController.snr > 5) ? "green" : "dimgrey"
                        Accessible.ignored: true
                    }
                    Rectangle{
                        height: 12
                        width: 4
                        color: (pluginContext.RadioController.snr > 8) ? "green" : "dimgrey"
                        Accessible.ignored: true
                    }
                    Rectangle{
                        height: 16
                        width: 4
                        color: (pluginContext.RadioController.snr > 11) ? "green" : "dimgrey"
                        Accessible.ignored: true
                    }

                    Rectangle{
                        height: 20
                        width: 4
                        color: (pluginContext.RadioController.snr > 15) ? "green" : "dimgrey"
                        Accessible.ignored: true
                    }
                }

                ThemeText {
                    id: dabMode
                    text: (pluginContext.RadioController.isDAB ? "DAB" : "DAB+")
                          + " " + pluginContext.RadioController.audioMode
                    anchors.left: signalStrength.right
                    verticalAlignment: Text.AlignTop
                    horizontalAlignment: Text.AlignRight
                    wrapMode: Text.WordWrap
                    anchors.leftMargin: 16
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.rightMargin: 0
                    renderType: Text.NativeRendering;
                    font.hintingPreference: Font.PreferVerticalHinting
                }

                ThemeText {
                    id: ensemble
                    text : pluginContext.RadioController.ensemble.trim()
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    anchors.topMargin: 8
                    anchors.right: parent.right
                    anchors.top: dabMode.bottom
                    anchors.rightMargin: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    renderType: Text.NativeRendering;
                    font.hintingPreference: Font.PreferVerticalHinting
                }

                ThemeHeaderText {
                    id: title
                    level: 3
                    text : pluginContext.RadioController.title.trim()
                    
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    anchors.topMargin: 16
                    anchors.right: parent.right
                    anchors.top: ensemble.bottom
                    anchors.rightMargin: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    renderType: Text.NativeRendering;
                    font.hintingPreference: Font.PreferVerticalHinting
                }
            }

            RowLayout {
                id: rowStationContent
                anchors.top: title.bottom
                anchors.bottom: buttons.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.topMargin: 8
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Image {
                    id: motImage
                    //Collapse if no image is available, give the space for the radioText instead
                    visible: (status === Image.Ready) && (source.toString() !== "") && (source.toString().toLowerCase() !== "image://sls/empty")

                    // Set target dimensions based on visibility
                    property real targetWidth: visible ? 320 : 0
                    property real targetHeight: visible ? 240 : 0

                    // Bind Layout properties to the target dimensions
                    Layout.preferredWidth: targetWidth
                    Layout.preferredHeight: targetHeight
                    Layout.minimumWidth: targetWidth
                    Layout.minimumHeight: targetHeight
                    Layout.maximumWidth: 320

                    fillMode: Image.PreserveAspectFit
                    Component.onCompleted:  pluginContext.imageItemLoaded(this)

                    // Add smooth animation for the collapse/expand effect
                    Behavior on targetWidth { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad } }
                    Behavior on targetHeight { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad } }

                   // Clip ensures the image doesn't "bleed" outside the shrinking bounds during animation
                   clip: true
                }

                ThemeHeaderText {
                    id: radioText
                    level: 1
                    text: pluginContext.RadioController.text.trim()
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    Layout.minimumWidth: 100
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 100
               }
           }

            RowLayout {
                id: buttons
                width: height * 5
                height: parent.height*0.15
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16

                ImageButton {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    imageSource: pluginContext.RadioController.isPlaying ? "qrc:/qml/icons/pause.png" : "qrc:/qml/icons/play.png"
                    id:playButton
                    onClicked: {
                        if(pluginContext.RadioController.isPlaying){
                            pluginContext.RadioController.stop();
                        } else {
                            pluginContext.playLastStation();
                        }
                    }
                }
            }
        }

    }
    StationDrawer {
        id: stationDrawer
        width: parent.width * 0.3
        anchors.bottom: parent.bottom
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        anchors.top: parent.top
    }
    Connections{
        target: pluginContext.RadioController

        onShowErrorMessage:{
        }

        onShowInfoMessage:{
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorColor:"#808080";height:480;width:640}
}
##^##*/
