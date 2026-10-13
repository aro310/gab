import QtQuick

Item {
    id: page
    clip: true

    signal goHistory()
    property var stackView
    property string txFilter: ""

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

    function actionIcon(a) {
        if (a === "depot"   || a === "DEPOT")   return "+"
        if (a === "retrait" || a === "RETRAIT") return "\u2212"
        if (a === "interet" || a === "INTERET") return "%"
        return "\u2630"
    }

    function errLabel(code) {
        if (code === "montant_invalide")  return "Montant invalide"
        if (code === "solde_insuffisant") return "Solde insuffisant \u2014 " + fmt(atm.solde) + " Ariary"
        if (code === "solde_min")         return "Solde min requis : " + fmt(atm.soldeMin) + " Ariary"
        if (code === "limite_session")    return "Plafond session : " + fmt(atm.limSession) + " Ariary"
        return code
    }

    function runAction() {
        var v = parseFloat(amountInput.text)
        if (isNaN(v) || v <= 0) {
            showToast("Montant invalide", true)
            return
        }
        var m = inputPanel.mode
        var res = (m === "depot") ? atm.deposer(v) : atm.retirer(v)
        inputPanel.mode = ""
        amountInput.text = ""
        showToast(
            res === "ok"
                ? (m === "depot" ? "+ " : "\u2212 ") + fmt(v) + " Ariary"
                : errLabel(res),
            res !== "ok"
        )
    }

    function showToast(msg, isErr) {
        toastMsg.text    = msg
        toastBox.isErr   = isErr
        toastBox.visible = true
        toastBox.opacity = 1
        toastTimer.restart()
    }

    property var displayedTx: {
        var h = atm.historique
        var filtered = txFilter === "" ? h : h.filter(function(tx) { return tx.type === txFilter })
        var s = filtered.slice(Math.max(0, filtered.length - 8))
        s.reverse()
        return s
    }

    ListModel {
        id: actionModel
        ListElement { lbl: "D\u00e9pot";     act: "depot"   }
        ListElement { lbl: "Retrait";        act: "retrait" }
        ListElement { lbl: "Int\u00e9r\u00eats"; act: "interet" }
        ListElement { lbl: "Historique";     act: "history" }
    }

    Rectangle { anchors.fill: parent; color: "#F4F6FB" }

    // Blue gradient header
    Rectangle {
        id: header
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: Math.round(page.height * 0.40)
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1E3A8A" }
            GradientStop { position: 1.0; color: "#3B82F6" }
        }

        // Top bar
        Item {
            anchors { top: parent.top; left: parent.left; right: parent.right; topMargin: 20 }
            height: 40

            Text {
                anchors { left: parent.left; leftMargin: 22; verticalCenter: parent.verticalCenter }
                text: "SYSTEME ATM"
                color: "white"
                opacity: 0.65
                font.pixelSize: 11
                font.letterSpacing: 3
            }

            Rectangle {
                anchors { right: parent.right; rightMargin: 22; verticalCenter: parent.verticalCenter }
                width: 34; height: 34; radius: 17
                color: Qt.rgba(1, 1, 1, 0.2)

                Text {
                    anchors.centerIn: parent
                    text: atm.titulaire.length > 0 ? atm.titulaire.charAt(0).toUpperCase() : "?"
                    color: "white"
                    font.pixelSize: 14
                    font.bold: true
                }
            }
        }

        // Balance block
        Column {
            anchors {
                left: parent.left; leftMargin: 22
                top: parent.top; topMargin: Math.round(header.height * 0.28)
            }
            spacing: 4

            Text {
                text: "Solde disponible"
                color: "white"
                opacity: 0.75
                font.pixelSize: 13
            }

            Text {
                text: fmt(atm.solde) + " Ariary"
                color: "white"
                font.pixelSize: 28
                font.bold: true
                fontSizeMode: Text.HorizontalFit
                minimumPixelSize: 16
                width: header.width - 44
            }

            Text {
                text: atm.type
                color: "white"
                opacity: 0.65
                font.pixelSize: 12
                fontSizeMode: Text.HorizontalFit
                minimumPixelSize: 9
                width: header.width - 44
            }

            Text {
                visible: atm.limSession > 0
                text: "Session : " + fmt(atm.retSession) + " / " + fmt(atm.limSession) + " Ariary"
                color: "white"
                opacity: 0.55
                font.pixelSize: 11
            }
        }

        // 4 action buttons
        Row {
            id: actionRow
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right; bottomMargin: 18 }

            Repeater {
                model: actionModel

                Item {
                    width: actionRow.width / 4
                    height: 80

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        Rectangle {
                            id: iconBox
                            width: 50; height: 50; radius: 14
                            color: Qt.rgba(1, 1, 1, 0.18)
                            anchors.horizontalCenter: parent.horizontalCenter
                            scale: iconTap.pressed ? 0.88 : 1.0
                            Behavior on scale { NumberAnimation { duration: 90 } }

                            Text {
                                anchors.centerIn: parent
                                text: page.actionIcon(act)
                                color: "white"
                                font.pixelSize: 20
                                font.bold: true
                            }

                            MouseArea {
                                id: iconTap
                                anchors.fill: parent
                                onClicked: {
                                    if (act === "history") {
                                        page.goHistory()
                                    } else if (act === "interet") {
                                        var r = atm.appliquerInterets()
                                        page.showToast(r === "ok" ? "Int\u00e9r\u00eats appliqu\u00e9s" : page.errLabel(r), r !== "ok")
                                    } else {
                                        inputPanel.mode = act
                                        amountInput.forceActiveFocus()
                                    }
                                }
                            }
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: lbl
                            color: "white"
                            font.pixelSize: 12
                            fontSizeMode: Text.HorizontalFit
                            minimumPixelSize: 9
                            width: (actionRow.width / 4) - 10
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }
            }
        }
    }

    // White content card
    Rectangle {
        id: contentCard
        anchors {
            top: header.bottom; topMargin: -22
            left: parent.left; right: parent.right
            bottom: bottomNav.top
        }
        radius: 22
        color: "white"
        clip: true

        Item {
            id: sectionHeader
            anchors {
                top: parent.top; topMargin: 26
                left: parent.left; leftMargin: 22
                right: parent.right; rightMargin: 22
            }
            height: 22

            Text {
                anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                text: "Transactions r\u00e9centes"
                font.pixelSize: 16
                font.bold: true
                color: "#111827"
            }

            Text {
                anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                text: "Voir tout"
                font.pixelSize: 13
                color: "#2563EB"
                MouseArea { anchors.fill: parent; onClicked: page.goHistory() }
            }
        }

        Row {
            id: filterRow
            anchors { top: sectionHeader.bottom; topMargin: 14; left: parent.left; leftMargin: 22 }
            spacing: 8

            Repeater {
                model: ["Tous", "D\u00e9p\u00f4t", "Retrait"]

                Rectangle {
                    height: 30
                    width: pillLbl.width + 24
                    radius: 15
                    color:  (txFilter === (index === 0 ? "" : index === 1 ? "DEPOT" : "RETRAIT")) ? "#111827" : "transparent"
                    border {
                        width: 1
                        color: (txFilter === (index === 0 ? "" : index === 1 ? "DEPOT" : "RETRAIT")) ? "transparent" : "#E5E7EB"
                    }
                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        id: pillLbl
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 13
                        color: (txFilter === (index === 0 ? "" : index === 1 ? "DEPOT" : "RETRAIT")) ? "white" : "#9CA3AF"
                        Behavior on color { ColorAnimation { duration: 150 } }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: txFilter = index === 0 ? "" : (index === 1 ? "DEPOT" : "RETRAIT")
                    }
                }
            }
        }

        ListView {
            id: txList
            anchors {
                top: filterRow.bottom; topMargin: 14
                left: parent.left; right: parent.right
                bottom: parent.bottom; bottomMargin: 10
                leftMargin: 14; rightMargin: 14
            }
            clip: true
            spacing: 8
            model: page.displayedTx

            delegate: Rectangle {
                width: txList.width
                height: 64
                radius: 12
                color: "#F9FAFB"

                Row {
                    anchors { fill: parent; leftMargin: 14; rightMargin: 14 }
                    spacing: 12

                    Rectangle {
                        width: 40; height: 40; radius: 12
                        anchors.verticalCenter: parent.verticalCenter
                        color: modelData.type === "RETRAIT" ? "#FEE2E2" : (modelData.type === "INTERET" ? "#DBEAFE" : "#D1FAE5")

                        Text {
                            anchors.centerIn: parent
                            text: page.actionIcon(modelData.type)
                            font.pixelSize: 18
                            font.bold: true
                            color: modelData.type === "RETRAIT" ? "#EF4444" : (modelData.type === "INTERET" ? "#2563EB" : "#10B981")
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 3
                        width: txList.width - 40 - 12 - 96 - 12 - 28

                        Text {
                            text: modelData.type
                            font.pixelSize: 14
                            font.bold: true
                            color: "#111827"
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: modelData.date.substring(0, 10)
                            font.pixelSize: 12
                            color: "#9CA3AF"
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 3
                        width: 96

                        Text {
                            anchors.right: parent.right
                            text: (modelData.type === "RETRAIT" ? "\u2212 " : "+ ") + fmt(modelData.montant) + " Ar"
                            font.pixelSize: 13
                            font.bold: true
                            color: modelData.type === "RETRAIT" ? "#EF4444" : "#111827"
                        }

                        Text {
                            anchors.right: parent.right
                            text: fmt(modelData.solde) + " Ar"
                            font.pixelSize: 11
                            color: "#9CA3AF"
                        }
                    }
                }
            }

            Item {
                anchors.centerIn: parent
                visible: txList.count === 0

                Text {
                    anchors.centerIn: parent
                    text: "Aucune transaction"
                    font.pixelSize: 14
                    color: "#D1D5DB"
                }
            }
        }
    }

    // Bottom navigation
    Rectangle {
        id: bottomNav
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
        height: 64
        color: "white"
        z: 5

        Rectangle {
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 1
            color: "#F3F4F6"
        }

        Item {
            anchors { left: parent.left; leftMargin: 36; top: parent.top; bottom: parent.bottom }
            width: 80

            Column {
                anchors.centerIn: parent
                spacing: 2

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "\u2302"
                    font.pixelSize: 22
                    color: "#2563EB"
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Accueil"
                    font.pixelSize: 10
                    color: "#2563EB"
                }
            }
        }

        Item {
            anchors { right: parent.right; rightMargin: 36; top: parent.top; bottom: parent.bottom }
            width: 80

            Column {
                anchors.centerIn: parent
                spacing: 2

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "\u2630"
                    font.pixelSize: 22
                    color: "#9CA3AF"
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Historique"
                    font.pixelSize: 10
                    color: "#9CA3AF"
                }
            }

            MouseArea { anchors.fill: parent; onClicked: page.goHistory() }
        }
    }

    // FAB
    Rectangle {
        width: 52; height: 52; radius: 26
        color: "#2563EB"
        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 28 }
        z: 10
        scale: fabTap.pressed ? 0.9 : 1.0
        Behavior on scale { NumberAnimation { duration: 90 } }

        Text {
            anchors.centerIn: parent
            text: "+"
            color: "white"
            font.pixelSize: 26
            font.bold: true
        }

        MouseArea {
            id: fabTap
            anchors.fill: parent
            onClicked: { inputPanel.mode = "depot"; amountInput.forceActiveFocus() }
        }
    }

    // Dim overlay
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: inputPanel.mode !== "" ? 0.25 : 0
        visible: opacity > 0
        z: 15
        Behavior on opacity { NumberAnimation { duration: 200 } }
        MouseArea { anchors.fill: parent; onClicked: { amountInput.text = ""; inputPanel.mode = "" } }
    }

    // Input panel
    Rectangle {
        id: inputPanel
        property string mode: ""

        x: 0
        y: mode !== "" ? page.height - height : page.height
        width: page.width
        height: 200
        radius: 20
        color: "white"
        z: 20
        Behavior on y { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }

        Column {
            anchors { fill: parent; margins: 22 }
            spacing: 14

            Text {
                text: inputPanel.mode === "depot" ? "Montant \u00e0 d\u00e9poser (Ariary)" : "Montant \u00e0 retirer (Ariary)"
                font.pixelSize: 13
                color: "#6B7280"
            }

            Rectangle {
                width: parent.width
                height: 48
                radius: 10
                color: "#F9FAFB"
                border { color: amountInput.activeFocus ? "#2563EB" : "#E5E7EB"; width: 1 }

                TextInput {
                    id: amountInput
                    anchors { fill: parent; leftMargin: 16; rightMargin: 16 }
                    font.pixelSize: 18
                    color: "#111827"
                    verticalAlignment: TextInput.AlignVCenter
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    Keys.onReturnPressed: page.runAction()
                    Keys.onEscapePressed: { text = ""; inputPanel.mode = "" }
                }
            }

            Row {
                width: parent.width
                spacing: 12

                Rectangle {
                    width: (parent.width - 12) / 2
                    height: 44; radius: 10
                    color: confBtn.pressed ? "#1D4ED8" : "#2563EB"
                    Behavior on color { ColorAnimation { duration: 80 } }

                    Text {
                        anchors.centerIn: parent
                        text: "Confirmer"
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    MouseArea { id: confBtn; anchors.fill: parent; onClicked: page.runAction() }
                }

                Rectangle {
                    width: (parent.width - 12) / 2
                    height: 44; radius: 10
                    color: cancBtn.pressed ? "#F3F4F6" : "transparent"
                    border { color: "#E5E7EB"; width: 1 }
                    Behavior on color { ColorAnimation { duration: 80 } }

                    Text {
                        anchors.centerIn: parent
                        text: "Annuler"
                        color: "#6B7280"
                        font.pixelSize: 14
                    }

                    MouseArea { id: cancBtn; anchors.fill: parent; onClicked: { amountInput.text = ""; inputPanel.mode = "" } }
                }
            }
        }
    }

    // Toast
    Rectangle {
        id: toastBox
        property bool isErr: false
        visible: false
        opacity: 0
        z: 25
        anchors { top: parent.top; left: parent.left; right: parent.right; topMargin: 12; leftMargin: 16; rightMargin: 16 }
        height: 44
        radius: 10
        color: isErr ? "#FEF2F2" : "#F0FDF4"
        border { color: isErr ? "#FECACA" : "#BBF7D0"; width: 1 }

        Text {
            id: toastMsg
            anchors.centerIn: parent
            font.pixelSize: 13
            color: toastBox.isErr ? "#DC2626" : "#16A34A"
        }

        Timer {
            id: toastTimer
            interval: 2800
            onTriggered: toastBox.opacity = 0
        }

        Behavior on opacity { NumberAnimation { duration: 300 } }

        onOpacityChanged: {
            if (opacity === 0) visible = false
        }
    }
}