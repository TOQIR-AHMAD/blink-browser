// QML tests for the design system (PLAN.md §41).

import QtQuick
import QtTest
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Theme

Item {
    width: 400
    height: 300

    GlassSurface {
        id: surface
        width: 200
        height: 60
        interactive: true
    }

    GlassToggle {
        id: toggle
        label: "Test"
    }

    TestCase {
        name: "Theme"
        when: windowShown

        function test_dark_follows_the_mode() {
            Theme.mode = 2;
            compare(Theme.dark, true, "mode 2 is dark");
            compare(Colors.dark, true, "the palette follows the theme");

            Theme.mode = 1;
            compare(Theme.dark, false, "mode 1 is light");
            compare(Colors.dark, false, "the palette follows again");

            Theme.mode = 0;
            Theme.systemPrefersDark = true;
            compare(Theme.dark, true, "mode 0 follows the system");
            Theme.systemPrefersDark = false;
            compare(Theme.dark, false, "and follows it the other way");
        }

        function test_reduced_motion_removes_every_duration() {
            Theme.reducedMotion = false;
            verify(Theme.quick > 0);
            verify(Theme.gentle > 0);

            Theme.reducedMotion = true;
            compare(Theme.instant, 0, "instant");
            compare(Theme.quick, 0, "quick");
            compare(Theme.gentle, 0, "gentle");
            compare(Theme.slow, 0, "slow");

            Theme.reducedMotion = false;
        }

        function test_text_scale_reaches_the_type_scale() {
            const base = Typography.body;
            Theme.textScale = 1.5;
            verify(Typography.body > base, "body text grows with the scale");
            Theme.textScale = 1.0;
            compare(Typography.body, base, "and returns");
        }

        function test_light_and_dark_palettes_are_distinct() {
            Theme.mode = 1;
            const lightText = Colors.text.toString();
            const lightBackground = Colors.background.toString();
            Theme.mode = 2;
            compare(Colors.dark, true, "the palette followed the theme");
            verify(Colors.text.toString() !== lightText, "text colour changes with the theme");
            verify(Colors.background.toString() !== lightBackground, "so does the background");
            Theme.mode = 0;
        }
    }

    TestCase {
        name: "GlassComponents"
        when: windowShown

        function test_surface_reacts_to_interaction() {
            const resting = surface.effectiveFill.toString();
            surface.hovered = true;
            verify(surface.effectiveFill.toString() !== resting, "hover changes the fill");
            surface.hovered = false;
            surface.down = true;
            verify(surface.effectiveFill.toString() !== resting, "pressed changes the fill");
            surface.down = false;
            compare(surface.effectiveFill.toString(), resting, "and returns when released");
        }

        function test_toggle_switches() {
            toggle.checked = false;
            toggle.toggle();
            compare(toggle.checked, true, "toggling turns it on");
            toggle.toggle();
            compare(toggle.checked, false, "and off again");
        }
    }
}
