import QtQuick 2.11

Item  {
    id: root1
    property real value : 0

    width: 300; height: 300

    Image  { id: speed_inactive; x: -9; y: 8; opacity: 0.8; z: 3; source: "qrc:/J2534/pics/gauge_inactive.png"

    }
    Image  {
        id: needle
        x: 136; y: 86
        clip: true
        opacity: root1.opacity
        z: 3
        smooth: true
        source: "qrc:/J2534/pics/needle.png"
        transform: Rotation  {
            id: needleRotation
            origin.x: 5; origin.y: 65
            angle: Math.min(Math.max(-130, root1.value*263/120 - 130), 133)

            Behavior on angle  {
                SpringAnimation  {
                    spring: 1.4
                    damping: .15
                }
            }
        }
    }
}
