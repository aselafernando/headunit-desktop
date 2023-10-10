import QtQuick 2.11
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.0

import HUDTheme 1.0

ThemeRoot {
    id: __root
    anchors.fill: parent

    ParallelAnimation{
        id: dialEffectStart
        running: true
        NumberAnimation{target: iat_active; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: stft_active; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: ect_active; property:  "opacity"; to: 1.0; duration: 5000}
    }

    TempDial  {
        id: iat_dial
        x: 33
        y: 140
        value: pluginContext.iat
        z: 7
        opacity: 1
        //anchors.centerIn: parent
        //value: slider.x * 100 / (container.width - 34)
        //value: 50

        Image {
            id: iatOverlay
            x: 123
            y: 137
            z: 7
            opacity: 1
            source: "qrc:/J2534/pics/overlay_active.png"
        }

        Image {
            id: iat_active
            x: -21
            y: -10
            z: 12
            opacity: 0
            visible: true
            source: "qrc:/J2534/pics/iat_active.png"
        }
    }

    TempDial  {
        id: ect_dial
        x: 366
        y: 140
        value: pluginContext.ect
        z: 7
        opacity: 1
        //anchors.centerIn: parent
        //value: slider.x * 100 / (container.width - 34)
        //value: 50

        Image {
            id: ectOverlay
            x: 123
            y: 137
            z: 7
            opacity: 1
            source: "qrc:/J2534/pics/overlay_active.png"
        }

        Image {
            id: ect_active
            x: -21
            y: -10
            z: 12
            opacity: 0
            visible: true
            source: "qrc:/J2534/pics/ect_active.png"
        }
    }

    FuelTrimDial  {
        id: stft_dial
        x: 699
        y: 140
        visible: true
        z: 7
        opacity: 1
        value: pluginContext.rpm
        //anchors.verticalCenterOffset: 8
        //anchors.horizontalCenterOffset: -207
        //anchors.centerIn: parent
       // value: slider.x * 100 / (container.width - 34)

        Image {
            id: stftOverlay
            x: 123
            y: 129
            z: 7
            opacity: 1
            source: "qrc:/J2534/pics/overlay_active.png"
        }

        Image {
            id: stft_active
            x: -22
            y: -17
            z: 11
            opacity: 0
            visible: true
            source: "qrc:/J2534/pics/stft_active.png"
        }
   }

    
}
