import QtQuick
import QtQuick.Controls

Item {
    id: page
    property var stackView
    property string filter: ""

    function fmt(v) {
        var s = Math.floor(Math.abs(v)).toString()
        var r = ""
        var c = 0
        for (var i = s.length - 1; i >= 0; i--) {
            if (c > 0 && c % 3 === 0) r = "\u202f" + r
            r = s[i] + r
            c++
        }
        return r
    }

    property var filteredHist: {
        var h = atm.historique
        var result = filter === ""
            ? h.slice().reverse()
            : h.filter(function(tx) { return tx.type === filter }).reverse()
        return result
    }

    Rectangle { anchors.fill: parent; color: "#F4F6FB" }

    Rectangle {
        id: hdr
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 70
        color: "white"

        Rectangle {
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            height: 1
            color: "#F3F4F6"
        }

        Rectangle {
            id: backBtn
            anchors { left: parent.left; leftMargin: 20; verticalCenter: parent.verticalCenter }
            width: 36; height: 36; radius: 10
            color: backTap.pressed ? "#F3F4F6" : "transparent"
            border { color: "#E5E7EB"; width: 1 }

            Text {
                anchors.centerIn: parent
                text: "\u2190"
                font.pixelSize: 18
                color: "#374151"
            }

            MouseArea { id: backTap; anchors.fill: parent; onClicked: stackView.pop() }
        }

        Text {
            anchors { left: backBtn.right; leftMargin: 14; verticalCenter: parent.verticalCenter }
            text: "HISTORIQUE"
            font.pixelSize: 13
            font.letterSpacing: 3
            font.bold: true
            color: "#111827"
        }

        Text {
            anchors { right: parent.right; rightMargin: 22; verticalCenter: parent.verticalCenter }
            text: atm.historique.length + " transactions"
            font.pixelSize: 12
            color: "#9CA3AF"
        }
    }

    Row {
        id: filterRow
        anchors { top: hdr.bottom; topMargin: 16; left: parent.left; leftMargin: 20 }
        spacing: 8

        Repeater {
            model: ["Tous", "D\u00e9p\u00f4t", "Retrait", "Int\u00e9r\u00eat"]

            Rectangle {
                height: 30
                width: fPillLbl.width + 24
                radius: 15

                property string tgt: index === 0 ? "" : (index === 1 ? "DEPOT" : (index === 2 ? "RETRAIT" : "INTERET"))

                color: filter === tgt ? "#111827" : "transparent"
                border { color: filter === tgt ? "transparent" : "#E5E7EB"; width: 1 }
                Behavior on color { ColorAnimation { duration: 150 } }

                Text {
                    id: fPillLbl
                    anchors.centerIn: parent
                    text: modelData
                    font.pixelSize: 13
                    color: filter === parent.tgt ? "white" : "#9CA3AF"
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: filter = parent.tgt
                }
            }
        }
    }

    ListView {
        id: histList
        anchors {
            top: filterRow.bottom; topMargin: 14
            left: parent.left; right: parent.right
            bottom: parent.bottom; bottomMargin: 14
            leftMargin: 14; rightMargin: 14
        }
        clip: true
        spacing: 8
        model: page.filteredHist

        delegate: MoneyRow {
            width: histList.width
            txDate:    modelData.date
            txType:    modelData.type
            txMontant: modelData.montant
            txSolde:   modelData.solde
        }

        Item {
            anchors.centerIn: parent
            visible: histList.count === 0

            Text {
                anchors.centerIn: parent
                text: "Aucune transaction"
                font.pixelSize: 14
                color: "#D1D5DB"
            }
        }

        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    }
}