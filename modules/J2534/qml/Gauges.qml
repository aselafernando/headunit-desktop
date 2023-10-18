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
        NumberAnimation{target: iat_text; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: iat_header; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: ect_active; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: ect_text; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: ect_header; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: stftb1_active; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: stftb1_text; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: stftb1_header; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: stftb2_active; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: stftb2_text; property:  "opacity"; to: 1.0; duration: 5000}
        NumberAnimation{target: stftb2_header; property:  "opacity"; to: 1.0; duration: 5000}

    }

    TempDial  {
        id: iat_dial
        x: 10
        y: 200
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
        
        ThemeText {
            id: iat_text
            text:pluginContext.iat + qsTr("°C")
            renderType: Text.NativeRendering;
            x: 89
            y: 190
            z: 20
            width: 100
            opacity: 0
            font.pointSize: 15
            font.hintingPreference: Font.PreferVerticalHinting
            horizontalAlignment: Text.AlignHCenter
        }

        ThemeText {
            id: iat_header
            text:"IAT"
            renderType: Text.NativeRendering;
            x: 89
            y: 220
            z: 20
            font.bold: false
            font.pointSize: 20
            width: 100
            opacity: 0
            font.hintingPreference: Font.PreferVerticalHinting
            horizontalAlignment: Text.AlignHCenter
        }
    }

    TempDial  {
        id: ect_dial
        x: 250
        y: 33
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

        ThemeText {
            id: ect_text
            text:pluginContext.ect + qsTr("°C")
            renderType: Text.NativeRendering;
            x: 89
            y: 190
            z: 20
            width: 100
            opacity: 0
            font.pointSize: 15
            font.hintingPreference: Font.PreferVerticalHinting
            horizontalAlignment: Text.AlignHCenter
        }

        ThemeText {
            id: ect_header
            text:"ECT"
            renderType: Text.NativeRendering;
            x: 89
            y: 220
            z: 20
            font.bold: false
            font.pointSize: 20
            width: 100
            opacity: 0
            font.hintingPreference: Font.PreferVerticalHinting
            horizontalAlignment: Text.AlignHCenter
        }

    }

    FuelTrimDial  {
        id: stftb1_dial
        x: 490
        y: 200
        visible: true
        z: 7
        opacity: 1
        value: pluginContext.stftb1
        //anchors.verticalCenterOffset: 8
        //anchors.horizontalCenterOffset: -207
        //anchors.centerIn: parent
       // value: slider.x * 100 / (container.width - 34)

        Image {
            id: stftb1Overlay
            x: 123
            y: 129
            z: 7
            opacity: 1
            source: "qrc:/J2534/pics/overlay_active.png"
        }

        Image {
            id: stftb1_active
            x: -22
            y: -17
            z: 11
            opacity: 0
            visible: true
            source: "qrc:/J2534/pics/stft_active.png"
        }

        ThemeText {
            id: stftb1_text
            text:pluginContext.stftb1
            renderType: Text.NativeRendering;
            x: 89
            y: 190
            z: 20
            width: 100
            opacity: 0
            font.pointSize: 15
            font.hintingPreference: Font.PreferVerticalHinting
            horizontalAlignment: Text.AlignHCenter
        }

        ThemeText {
            id: stftb1_header
            text:"STFT1"
            renderType: Text.NativeRendering;
            x: 89
            y: 220
            z: 20
            font.bold: false
            font.pointSize: 20
            width: 100
            opacity: 0
            font.hintingPreference: Font.PreferVerticalHinting
            horizontalAlignment: Text.AlignHCenter
        }
   }

    FuelTrimDial  {
        id: stftb2_dial
        x: 730
        y: 33
        visible: true
        z: 7
        opacity: 1
        value: pluginContext.stftb2
        //anchors.verticalCenterOffset: 8
        //anchors.horizontalCenterOffset: -207
        //anchors.centerIn: parent
       // value: slider.x * 100 / (container.width - 34)

        Image {
            id: stftb2Overlay
            x: 123
            y: 129
            z: 7
            opacity: 1
            source: "qrc:/J2534/pics/overlay_active.png"
        }

        Image {
            id: stftb2_active
            x: -22
            y: -17
            z: 11
            opacity: 0
            visible: true
            source: "qrc:/J2534/pics/stft_active.png"
        }

        ThemeText {
            id: stftb2_text
            text:pluginContext.stftb1
            renderType: Text.NativeRendering;
            x: 89
            y: 190
            z: 20
            width: 100
            opacity: 0
            font.pointSize: 15
            font.hintingPreference: Font.PreferVerticalHinting
            horizontalAlignment: Text.AlignHCenter
        }

        ThemeText {
            id: stftb2_header
            text:"STFT2"
            renderType: Text.NativeRendering;
            x: 89
            y: 220
            z: 20
            font.bold: false
            font.pointSize: 20
            width: 100
            opacity: 0
            font.hintingPreference: Font.PreferVerticalHinting
            horizontalAlignment: Text.AlignHCenter
        }
   }

    
}
