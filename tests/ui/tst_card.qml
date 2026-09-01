// GlassCard has to lay its content out in a column, including content produced
// by a Repeater. The first build of the browser got this wrong - every row of
// the welcome screen was drawn on top of the others - so it is a test now.

import QtQuick
import QtTest
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Theme

Item {
    width: 600
    height: 500

    GlassCard {
        id: card

        width: 500
        title: "Title"
        subtitle: "Subtitle"

        Text {
            id: first
            objectName: "first"
            text: "first row"
        }

        Repeater {
            id: repeater
            model: 3
            delegate: Text {
                required property int index
                objectName: "repeated" + index
                text: "repeated row " + index
            }
        }
    }

    TestCase {
        name: "GlassCard"
        when: windowShown

        function findRow(name) {
            const items = card.contentItem.children;
            for (let i = 0; i < items.length; ++i) {
                if (items[i].objectName === name)
                    return items[i];
            }
            return null;
        }

        function test_content_lands_in_the_content_column() {
            verify(findRow("first") !== null, "a plain child reaches the content column");
            for (let i = 0; i < 3; ++i)
                verify(findRow("repeated" + i) !== null, "repeated row " + i + " reaches it too");
        }

        function test_rows_are_stacked_not_overlapping() {
            const rows = [findRow("first"), findRow("repeated0"),
                          findRow("repeated1"), findRow("repeated2")];
            for (let i = 1; i < rows.length; ++i) {
                verify(rows[i].y > rows[i - 1].y,
                       "row " + i + " sits below row " + (i - 1)
                       + " (y=" + rows[i].y + " vs " + rows[i - 1].y + ")");
            }
        }

        function test_card_is_tall_enough_for_its_content() {
            const rows = [findRow("first"), findRow("repeated0"),
                          findRow("repeated1"), findRow("repeated2")];
            let lowest = 0;
            for (let i = 0; i < rows.length; ++i)
                lowest = Math.max(lowest, rows[i].y + rows[i].height);
            verify(card.height >= lowest,
                   "the card is " + card.height + " tall and its content reaches " + lowest);
        }
    }
}
