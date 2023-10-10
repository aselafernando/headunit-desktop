import QtQuick 2.11

Item  {
    id: root
    property real value : 0

    width: 300; height: 300

    Image  { id: rpm_inactive; x: -10; y: 0; opacity: 0.8; z: 3; source: "qrc:/J2534/pics/gauge_inactive.png"

    }
    Image  {
        id: needle
        x: 135; y: 76
        clip: true
        opacity: root.opacity
        z: 3
        smooth: true
        source: "qrc:/J2534/pics/needle.png"
        transform: Rotation  {
            id: needleRotation
            origin.x: 5; origin.y: 65
            angle: Math.min(Math.max(-130, root.value*2.6), 133)

            Behavior on angle  {
                SpringAnimation  {
                    spring: 1.4
                    damping: .15
                }
            }
        }
    }
}
