#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "DGLookAndFeel.h"

// Piezas visuales compartidas por la interfaz de DG-MasterBus y DG-Sender.
namespace dg
{
// Paleta propia de DG Audio (un color por seccion)
namespace pal
{
inline const juce::Colour sand   { 0xff9a8f7a };
inline const juce::Colour copper { 0xffb8693f };
inline const juce::Colour teal   { 0xff2a9d8f };
inline const juce::Colour gold   { 0xffc99a1b };
inline const juce::Colour blue   { 0xff3d7fb8 };
inline const juce::Colour peri   { 0xff7a7fe0 };
inline const juce::Colour danger { 0xffe0562f };
inline const juce::Colour amber  { 0xffffb347 };
} // namespace pal

// Aluminio cepillado: degradado vertical + vetas horizontales
inline juce::Image makeBrushed (int w, int h)
{
    juce::Image img (juce::Image::ARGB, w, h, false);
    juce::Image::BitmapData bd (img, juce::Image::BitmapData::writeOnly);
    juce::Random rng (0xD6A0D10);

    for (int y = 0; y < h; ++y)
    {
        const float t = (float) y / (float) h;
        const float base = juce::jmap (t, 0.0f, 1.0f, 0.47f, 0.34f);
        const float rowBias = (rng.nextFloat() - 0.5f) * 0.035f;
        float streak = 0.f;

        for (int x = 0; x < w; ++x)
        {
            streak = 0.93f * streak + 0.07f * (rng.nextFloat() - 0.5f);
            const float grain = (rng.nextFloat() - 0.5f) * 0.012f;
            const float v = juce::jlimit (0.f, 1.f, base + rowBias + streak * 0.9f + grain);
            bd.setPixelColour (x, y, juce::Colour::fromFloatRGBA (v * 0.98f, v, v * 1.03f, 1.0f));
        }
    }
    return img;
}

// Texto grabado (luz abajo, oscuro encima)
inline void drawEngraved (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> r, float size,
                          juce::Justification j = juce::Justification::centred)
{
    g.setFont (juce::FontOptions (size, juce::Font::bold));
    g.setColour (juce::Colours::white.withAlpha (0.22f));
    g.drawText (text, r.translated (0.f, 1.f), j, false);
    g.setColour (juce::Colour (0xff17191c).withAlpha (0.88f));
    g.drawText (text, r, j, false);
}

// Letras con separacion (rotulos y logotipo)
inline float drawSpaced (juce::Graphics& g, const juce::String& text, float x, float y, float h, float size,
                         float spacing, juce::Colour colour, bool bold = true)
{
    g.setFont (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
    g.setColour (colour);

    for (auto c : text)
    {
        float adv = size * 0.72f;
        if (c == ' ') adv = size * 0.45f;
        else if (c == 'I') adv = size * 0.38f;
        adv += spacing;

        g.drawText (juce::String::charToString (c),
                    juce::Rectangle<float> (x - size * 0.2f, y, size * 1.4f, h),
                    juce::Justification::centredLeft, false);
        x += adv;
    }
    return x;
}

// Logotipo DG AUDIO: emblema circular con monograma + rotulo
inline void drawLogo (juce::Graphics& g, juce::Rectangle<float> a)
{
    const float d = a.getHeight();
    const auto ring = juce::Rectangle<float> (d, d).withPosition (a.getPosition());
    const auto c = ring.getCentre();

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (ring.translated (0.f, 1.5f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff0f2f4), ring.getX(), ring.getY(),
                                             juce::Colour (0xff62666b), ring.getRight(), ring.getBottom(), false));
    g.fillEllipse (ring);

    const auto inner = ring.reduced (d * 0.09f);
    g.setColour (juce::Colour (0xff111316));
    g.fillEllipse (inner);

    juce::Path arc;
    arc.addCentredArc (c.x, c.y, inner.getWidth() * 0.5f - 3.f, inner.getHeight() * 0.5f - 3.f, 0.f,
                       juce::MathConstants<float>::pi * 0.62f, juce::MathConstants<float>::pi * 1.38f, true);
    g.setColour (pal::amber);
    g.strokePath (arc, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (juce::Colour (0xffeceff2));
    g.setFont (juce::FontOptions (d * 0.40f, juce::Font::bold));
    g.drawText ("DG", inner.translated (0.f, -d * 0.05f), juce::Justification::centred, false);

    const float x0 = ring.getRight() + 12.f;
    g.setFont (juce::FontOptions (d * 0.66f, juce::Font::bold));
    g.setColour (juce::Colour (0xffeceff2));
    g.drawText ("DG", juce::Rectangle<float> (x0, a.getY(), d * 1.05f, d), juce::Justification::centredLeft, false);
    drawSpaced (g, "AUDIO", x0 + d * 0.98f, a.getY(), d, d * 0.40f, 3.5f, juce::Colour (0xffb9bec4), false);
}

// Fondo comun: metal cepillado, marco, barra superior con logotipo y tornillos
inline void drawChrome (juce::Graphics& g, const juce::Image& brushed, int w, int h)
{
    g.drawImageAt (brushed, 0, 0);

    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.10f), 0.f, 0.f,
                                             juce::Colours::transparentWhite, 0.f, (float) h * 0.5f, false));
    g.fillRect (0, 0, w, h);

    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawRect (0, 0, w, h, 2);
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawRect (2, 2, w - 4, h - 4, 1);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff23262a), 0.f, 0.f,
                                             juce::Colour (0xff111316), 0.f, 62.f, false));
    g.fillRect (2, 2, w - 4, 60);
    g.setColour (juce::Colours::white.withAlpha (0.16f));
    g.fillRect (2, 62, w - 4, 1);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRect (2, 63, w - 4, 2);

    drawLogo (g, juce::Rectangle<float> (26.f, 9.f, 240.f, 44.f));

    const float sy = (float) h - 9.f;
    drawScrew (g, { 16.f, sy }, 5.f, 0.4f);
    drawScrew (g, { (float) w / 3.f, sy }, 5.f, 1.1f);
    drawScrew (g, { (float) w * 2.f / 3.f, sy }, 5.f, 0.2f);
    drawScrew (g, { (float) w - 16.f, sy }, 5.f, 0.9f);
}

// Seccion hundida con placa de titulo y franja de color
inline void drawSection (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title, juce::Colour accent)
{
    const auto r = bounds.toFloat();
    g.setColour (juce::Colours::black.withAlpha (0.22f));
    g.fillRoundedRectangle (r, 9.f);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawRoundedRectangle (r.translated (0.f, 1.f), 9.f, 1.f);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawRoundedRectangle (r, 9.f, 1.3f);

    const auto plate = juce::Rectangle<float> (r.getX() + 8.f, r.getY() + 7.f, r.getWidth() - 16.f, 20.f);
    g.setColour (juce::Colour (0xff17191c));
    g.fillRoundedRectangle (plate, 4.f);
    g.setColour (accent);
    g.fillRoundedRectangle (plate.withWidth (5.f), 2.f);

    g.setFont (juce::FontOptions (11.f, juce::Font::bold));
    g.setColour (accent.brighter (0.5f));
    g.drawText (title, plate.withTrimmedLeft (12.f), juce::Justification::centredLeft, false);
}

// Vumetro de aguja con bisel, esfera color papel, zona roja y reflejo de cristal
class NeedleMeter : public juce::Component
{
public:
    NeedleMeter (juce::String titleText, float minValue, float maxValue,
                 std::vector<float> tickValues, float redFrom)
        : title (std::move (titleText)), minV (minValue), maxV (maxValue),
          ticks (std::move (tickValues)), dangerFrom (redFrom), value (minValue)
    {
    }

    void setValue (float v)
    {
        v = juce::jlimit (minV, maxV, v);
        if (std::abs (v - value) > 0.01f)
        {
            value = v;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();

        // Bisel
        g.setColour (juce::Colour (0xff0d0e10));
        g.fillRoundedRectangle (b, 9.f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff8a8e94), b.getX(), b.getY(),
                                                 juce::Colour (0xff2c2f33), b.getX(), b.getBottom(), false));
        g.drawRoundedRectangle (b.reduced (1.f), 9.f, 2.f);

        // Esfera
        const auto face = b.reduced (7.f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xfffbefc2), face.getCentreX(), face.getBottom(),
                                                 juce::Colour (0xffd2bf80), face.getX(), face.getY(), true));
        g.fillRoundedRectangle (face, 5.f);

        {
            juce::Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (face.toNearestInt());

            const auto pivot = juce::Point<float> (face.getCentreX(), face.getBottom() - 10.f);
            const float radius = juce::jmin (face.getHeight() * 0.82f, face.getWidth() * 0.46f);
            const float a0 = 0.85f;

            auto angleFor = [this, a0] (float v) { return juce::jmap (v, minV, maxV, -a0, a0); };

            juce::Path base;
            base.addCentredArc (pivot.x, pivot.y, radius, radius, 0.f, -a0, angleFor (dangerFrom), true);
            g.setColour (juce::Colour (0xff2b2b2b));
            g.strokePath (base, juce::PathStrokeType (2.f));

            juce::Path red;
            red.addCentredArc (pivot.x, pivot.y, radius, radius, 0.f, angleFor (dangerFrom), a0, true);
            g.setColour (juce::Colour (0xffc0392b));
            g.strokePath (red, juce::PathStrokeType (4.f));

            g.setFont (12.f);
            for (auto t : ticks)
            {
                const float a = angleFor (t);
                const juce::Point<float> dir (std::sin (a), -std::cos (a));
                const auto p1 = pivot + dir * radius;
                const auto p2 = pivot + dir * (radius - 8.f);
                const auto lp = pivot + dir * (radius - 21.f);

                g.setColour (t >= dangerFrom ? juce::Colour (0xffc0392b) : juce::Colour (0xff2b2b2b));
                g.drawLine (juce::Line<float> (p1, p2), 1.6f);

                const juce::String text = (t > 0.f && minV < 0.f ? "+" : "") + juce::String ((int) t);
                g.drawText (text, juce::Rectangle<float> (30.f, 14.f).withCentre (lp),
                            juce::Justification::centred, false);
            }

            g.setColour (juce::Colour (0xff2b2b2b));
            g.setFont (juce::FontOptions (11.f, juce::Font::bold));
            g.drawText (title, juce::Rectangle<float> (face.getX() + 8.f, face.getY() + 4.f, face.getWidth() - 16.f, 14.f),
                        juce::Justification::centredLeft, false);

            const float a = angleFor (value);
            const auto tip = pivot + juce::Point<float> (std::sin (a), -std::cos (a)) * (radius - 4.f);
            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.drawLine (juce::Line<float> (pivot + juce::Point<float> (2.f, 2.f), tip + juce::Point<float> (2.f, 2.f)), 2.2f);
            g.setColour (juce::Colour (0xff111111));
            g.drawLine (juce::Line<float> (pivot, tip), 2.2f);
            g.fillEllipse (juce::Rectangle<float> (12.f, 12.f).withCentre (pivot));
        }

        // Reflejo de cristal
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.22f), face.getX(), face.getY(),
                                                 juce::Colours::transparentWhite, face.getX(), face.getCentreY(), false));
        g.fillRoundedRectangle (face.withHeight (face.getHeight() * 0.5f), 5.f);
    }

private:
    juce::String title;
    float minV, maxV;
    std::vector<float> ticks;
    float dangerFrom;
    float value;
};

// Perilla: titulo grabado, escala (marcas o posiciones con etiqueta), pantalla de valor
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
          const juce::String& titleText, juce::Colour capColour,
          const juce::String& unitText = {}, const juce::StringArray& labels = {})
        : title (titleText), unit (unitText), stepLabels (labels)
    {
        stepped = labels.size() > 1;

        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f, true);
        slider.setColour (juce::Slider::thumbColourId, capColour);
        slider.setMouseDragSensitivity (220);
        slider.onValueChange = [this] { repaint(); };
        addAndMakeVisible (slider);

        att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);
    }

    // Permite mostrar un texto propio en la pantalla (por ejemplo, el Drive en pasos de 4 dB)
    std::function<juce::String (double)> displayOverride;

    void resized() override
    {
        auto b = getLocalBounds();
        titleR = b.removeFromTop (18);
        valueR = b.removeFromBottom (20).reduced (6, 0);
        const int side = juce::jmin (b.getWidth(), b.getHeight());
        slider.setBounds (b.withSizeKeepingCentre (side, side).reduced (11));
    }

    void paint (juce::Graphics& g) override
    {
        drawEngraved (g, title, titleR.toFloat(), 11.f);

        const auto c = slider.getBounds().getCentre().toFloat();
        const float r = (float) juce::jmin (slider.getWidth(), slider.getHeight()) * 0.5f;
        const float a0 = juce::MathConstants<float>::pi * 1.25f;
        const float a1 = juce::MathConstants<float>::pi * 2.75f;

        g.setColour (juce::Colour (0xff17191c).withAlpha (0.88f));

        if (stepped)
        {
            const int n = stepLabels.size();
            g.setFont (juce::FontOptions (9.f, juce::Font::bold));

            for (int i = 0; i < n; ++i)
            {
                const float a = a0 + (a1 - a0) * (float) i / (float) (n - 1);
                const juce::Point<float> dir (std::sin (a), -std::cos (a));
                g.drawLine (juce::Line<float> (c + dir * (r + 2.f), c + dir * (r + 6.f)), 1.4f);
                g.drawText (stepLabels[i], juce::Rectangle<float> (26.f, 11.f).withCentre (c + dir * (r + 13.f)),
                            juce::Justification::centred, false);
            }
        }
        else
        {
            for (int i = 0; i <= 10; ++i)
            {
                const float a = a0 + (a1 - a0) * (float) i / 10.f;
                const juce::Point<float> dir (std::sin (a), -std::cos (a));
                const float len = (i % 5 == 0) ? 8.f : 5.f;
                g.drawLine (juce::Line<float> (c + dir * (r + 2.f), c + dir * (r + 2.f + len)), (i % 5 == 0) ? 1.6f : 1.1f);
            }
        }

        // Pantalla de valor
        const auto lcd = valueR.toFloat();
        g.setColour (juce::Colour (0xff0b0d0e));
        g.fillRoundedRectangle (lcd, 4.f);
        g.setColour (juce::Colours::white.withAlpha (0.14f));
        g.drawRoundedRectangle (lcd, 4.f, 1.f);
        g.setColour (pal::amber);
        g.setFont (juce::FontOptions (12.f, juce::Font::bold));
        g.drawText (valueText(), valueR, juce::Justification::centred, false);
    }

private:
    juce::String valueText()
    {
        if (displayOverride)
            return displayOverride (slider.getValue());

        const auto t = slider.getTextFromValue (slider.getValue());
        return unit.isEmpty() ? t : t + " " + unit;
    }

    juce::String title, unit;
    juce::StringArray stepLabels;
    bool stepped = false;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    juce::Rectangle<int> titleR, valueR;
};

// Tecla iluminada ligada a un parametro booleano
struct LedButton
{
    juce::ToggleButton button;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> att;

    void init (juce::Component& parent, juce::AudioProcessorValueTreeState& apvts,
               const juce::String& id, const juce::String& text, juce::Colour lit)
    {
        button.setButtonText (text);
        button.setColour (juce::ToggleButton::tickColourId, lit);
        parent.addAndMakeVisible (button);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, id, button);
    }
};
} // namespace dg
