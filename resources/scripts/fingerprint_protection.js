// Strict fingerprint protection.
//
// Injected into every frame at document creation, and ONLY when the user
// chooses the Strict level, because each override here trades a little
// compatibility for a little less entropy (PLAN.md §22 puts compatibility
// second to entropy but ahead of breaking things blindly).
//
// What this does NOT do, deliberately:
// - It does not randomise per page load. Randomness is itself detectable and
//   makes a browser stand out; every value below is a fixed, common one.
// - It does not touch screen size, timezone or language. Overriding those
//   breaks layout, scheduling and localisation for very little gain.
// - It does not claim to defeat fingerprinting. It removes a handful of the
//   most-read high-entropy values; a determined script can still tell browsers
//   apart.

(function () {
    "use strict";

    var define = function (object, name, value) {
        try {
            Object.defineProperty(object, name, {
                get: function () { return value; },
                configurable: false,
                enumerable: true
            });
        } catch (error) {
            // A frame may already have frozen the property; leaving the real
            // value in place is better than throwing inside the page.
        }
    };

    // Common values rather than unusual ones: the aim is to look like many
    // machines, not like a machine that is hiding.
    define(navigator, "hardwareConcurrency", 8);
    if ("deviceMemory" in navigator) {
        define(navigator, "deviceMemory", 8);
    }

    // Plugin and MIME lists are a classic fingerprint and mean nothing in a
    // modern browser.
    try {
        var emptyList = Object.create(Object.getPrototypeOf(navigator.plugins));
        Object.defineProperty(emptyList, "length", { value: 0 });
        define(navigator, "plugins", emptyList);
    } catch (error) { /* keep the real list rather than break feature checks */ }

    // WebGL vendor and renderer identify the GPU exactly.
    try {
        var maskWebGl = function (contextPrototype) {
            if (!contextPrototype || !contextPrototype.getParameter) {
                return;
            }
            var original = contextPrototype.getParameter;
            contextPrototype.getParameter = function (parameter) {
                // UNMASKED_VENDOR_WEBGL / UNMASKED_RENDERER_WEBGL from the
                // WEBGL_debug_renderer_info extension.
                if (parameter === 0x9245) {
                    return "Generic";
                }
                if (parameter === 0x9246) {
                    return "Generic Renderer";
                }
                return original.apply(this, arguments);
            };
        };
        maskWebGl(window.WebGLRenderingContext && window.WebGLRenderingContext.prototype);
        maskWebGl(window.WebGL2RenderingContext && window.WebGL2RenderingContext.prototype);
    } catch (error) { /* leave WebGL untouched if anything is unexpected */ }

    // Audio fingerprinting reads tiny floating-point differences between audio
    // stacks. A fixed, minute offset removes the difference without making the
    // audio audibly wrong.
    try {
        var analyserPrototype = window.AnalyserNode && window.AnalyserNode.prototype;
        if (analyserPrototype && analyserPrototype.getFloatFrequencyData) {
            var originalFrequency = analyserPrototype.getFloatFrequencyData;
            analyserPrototype.getFloatFrequencyData = function (array) {
                originalFrequency.apply(this, arguments);
                for (var i = 0; i < array.length; i++) {
                    array[i] = Math.round(array[i] * 100) / 100;
                }
            };
        }

        var bufferPrototype = window.AudioBuffer && window.AudioBuffer.prototype;
        if (bufferPrototype && bufferPrototype.getChannelData) {
            var originalChannel = bufferPrototype.getChannelData;
            bufferPrototype.getChannelData = function () {
                var data = originalChannel.apply(this, arguments);
                for (var j = 0; j < data.length; j++) {
                    data[j] = Math.round(data[j] * 1e6) / 1e6;
                }
                return data;
            };
        }
    } catch (error) { /* audio is left alone if the API differs */ }
})();
