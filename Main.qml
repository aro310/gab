import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 420
    height: 780
    minimumWidth: 360
    minimumHeight: 700
    visible: true
    title: "ATM"

    StackView {
        id: stack
        anchors.fill: parent

        pushEnter: Transition {
            XAnimator { from: root.width; to: 0; duration: 240; easing.type: Easing.OutCubic }
        }
        pushExit: Transition {
            XAnimator { from: 0; to: -(root.width / 4); duration: 240; easing.type: Easing.OutCubic }
        }
        popEnter: Transition {
            XAnimator { from: -(root.width / 4); to: 0; duration: 240; easing.type: Easing.OutCubic }
        }
        popExit: Transition {
            XAnimator { from: 0; to: root.width; duration: 240; easing.type: Easing.OutCubic }
        }

        Component.onCompleted: push(homeComp)
    }

    Component {
        id: homeComp
        MenuPage {
            stackView: stack
            onGoHistory: stack.push(histComp)
        }
    }

    Component {
        id: histComp
        HistoriquePage { stackView: stack }
    }
}