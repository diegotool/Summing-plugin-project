#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace dg
{
// Tornillo Phillips con degradado metálico
inline void drawScrew (juce::Graphics& g, juce::Point<float> c, float r, float angle)
{
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (juce::Rectangle<float> (r * 2.f, r * 2.f).withCentre (c + juce::Point<float> (0.f, 1.5f)));

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffc4c8ce), c.x - r * 0.5f, c.y - r * 0.6f,
                                             juce::Colour (0xff45484d), c.x + r * 0.6f, c.y + r * 0.7f, false));
    g.fillEllipse (juce::Rectangle<float> (r * 2.f, r * 2.f).withCentre (c));

    g.setColour (juce::Colour (0xff17181a));
    const juce::Point<float> d (std::cos (angle), std::sin (angle));
    const juce::Point<float> e (-d.y, d.x);
    g.drawLine (juce::Line<float> (c - d * (r * 0.7f), c + d * (r * 0.7f)), 1.8f);
    g.drawLine (juce::Line<float> (c - e * (r * 0.7f), c + e * (r * 0.7f)), 1.8f);
}

// Aspecto de hardware: perillas con falda metálica y tapa de color, teclas iluminadas
class DGLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider& s) override
    {
        const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (2.f);
        const float d = juce::jmin (bounds.getWidth(), bounds.getHeight());
        const float r = d * 0.5f;
        const auto c = bounds.getCentre();
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        const juce::Colour cap = s.findColour (juce::Slider::thumbColourId);

        // Sombra
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (juce::Rectangle<float> (d, d).withCentre (c + juce::Point<float> (1.5f, 3.f)));

        // Falda metálica
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6a6e73), c.x - r * 0.7f, c.y - r * 0.8f,
                                                 juce::Colour (0xff131416), c.x + r * 0.8f, c.y + r * 0.9f, false));
        g.fillEllipse (juce::Rectangle<float> (d, d).withCentre (c));

        // Moleteado del borde
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        for (int i = 0; i < 48; ++i)
        {
            const float a = (float) i * (juce::MathConstants<float>::twoPi / 48.f);
            const juce::Point<float> dir (std::sin (a), -std::cos (a));
            g.drawLine (juce::Line<float> (c + dir * (r * 0.88f), c + dir * (r * 0.99f)), 1.0f);
        }

        // Tapa de color
        const float rc = r * 0.72f;
        const auto capRect = juce::Rectangle<float> (rc * 2.f, rc * 2.f).withCentre (c);
        g.setGradientFill (juce::ColourGradient (cap.brighter (0.5f), c.x - rc * 0.6f, c.y - rc * 0.7f,
                                                 cap.darker (0.7f), c.x + rc * 0.7f, c.y + rc * 0.8f, false));
        g.fillEllipse (capRect);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawEllipse (capRect, 1.2f);

        // Brillo
        g.setColour (juce::Colours::white.withAlpha (0.16f));
        g.fillEllipse (juce::Rectangle<float> (rc * 1.2f, rc * 0.7f)
                           .withCentre (c + juce::Point<float> (-rc * 0.1f, -rc * 0.45f)));

        // Indicador
        const juce::Point<float> dir (std::sin (angle), -std::cos (angle));
        g.setColour (juce::Colours::white.withAlpha (0.95f));
        g.drawLine (juce::Line<float> (c + dir * (rc * 0.30f), c + dir * (rc * 0.92f)), juce::jmax (2.f, d * 0.04f));
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b,
                           bool /*highlighted*/, bool down) override
    {
        const auto r = b.getLocalBounds().toFloat().reduced (2.f);
        const bool on = b.getToggleState();
        const juce::Colour lit = b.findColour (juce::ToggleButton::tickColourId);

        // Marco
        g.setColour (juce::Colour (0xff0e0f11));
        g.fillRoundedRectangle (r, 5.f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff9a9ea4), r.getX(), r.getY(),
                                                 juce::Colour (0xff3b3e42), r.getX(), r.getBottom(), false));
        g.drawRoundedRectangle (r, 5.f, 2.f);

        // Tecla
        const auto k = r.reduced (4.f);
        if (on)
        {
            g.setColour (lit.withAlpha (0.35f));
            g.fillRoundedRectangle (k.expanded (3.f), 6.f);
            g.setGradientFill (juce::ColourGradient (lit.brighter (0.6f), k.getX(), k.getY(),
                                                     lit.darker (0.2f), k.getX(), k.getBottom(), false));
        }
        else
        {
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3d41), k.getX(), k.getY(),
                                                     juce::Colour (0xff1d1f22), k.getX(), k.getBottom(), false));
        }
        g.fillRoundedRectangle (k, 3.f);

        if (down)
        {
            g.setColour (juce::Colours::black.withAlpha (0.2f));
            g.fillRoundedRectangle (k, 3.f);
        }

        g.setColour (on ? (lit.getPerceivedBrightness() > 0.55f ? juce::Colour (0xff22190a) : juce::Colours::white)
                        : juce::Colour (0xffa0a6ac));
        g.setFont (juce::FontOptions (12.f, juce::Font::bold));
        g.drawFittedText (b.getButtonText(), k.toNearestInt(), juce::Justification::centred, 2);
    }
};
} // namespace dg
