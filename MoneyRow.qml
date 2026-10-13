import QtQuick

Rectangle {
    id: row
    height: 64
    radius: 12
    color: "white"

    property string txDate
    property string txType
    property double txMontant
    property double txSolde

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

    readonly property color bgClr:   txType === "RETRAIT" ? "#FEE2E2" : (txType === "INTERET" ? "#DBEAFE" : "#D1FAE5")
    readonly property color icoClr:  txType === "RETRAIT" ? "#EF4444" : (txType === "INTERET" ? "#2563EB" : "#10B981")
    readonly property color amtClr:  txType === "RETRAIT" ? "#EF4444" : "#111827"
    readonly property string ico:    txType === "RETRAIT" ? "\u2212"   : (txType === "INTERET" ? "%" : "+")
    readonly property string prefix: txType === "RETRAIT" ? "\u2212 "  : "+ "

    Row {
        anchors { fill: parent; leftMargin: 14; rightMargin: 14 }
        spacing: 12

        Rectangle {
            width: 40; height: 40; radius: 12
            anchors.verticalCenter: parent.verticalCenter
            color: row.bgClr

            Text {
                anchors.centerIn: parent
                text: row.ico
                font.pixelSize: 18
                font.bold: true
                color: row.icoClr
            }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 3
            width: row.width - 40 - 12 - 100 - 12 - 28

            Text {
                text: txType
                font.pixelSize: 14
                font.bold: true
                color: "#111827"
                elide: Text.ElideRight
                width: parent.width
            }

            Text {
                text: txDate.substring(0, 16)
                font.pixelSize: 12
                color: "#9CA3AF"
            }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 3
            width: 100

            Text {
                anchors.right: parent.right
                text: row.prefix + fmt(txMontant) + " Ar"
                font.pixelSize: 13
                font.bold: true
                color: row.amtClr
            }

            Text {
                anchors.right: parent.right
                text: fmt(txSolde) + " Ariary"
                font.pixelSize: 11
                color: "#9CA3AF"
            }
        }
    }
}