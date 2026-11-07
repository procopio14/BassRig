#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Ilustracoes vetoriais ORIGINAIS (estilizadas, sem logos) desenhadas em codigo.
namespace art
{
using namespace juce;

inline void knobDot (Graphics& g, Point<float> c, float r, Colour body = Colour (0xffd8dadf))
{
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillEllipse (c.x - r - 1.0f, c.y - r + 1.0f, 2.0f * r + 2.0f, 2.0f * r + 2.0f);
    g.setGradientFill (ColourGradient (body.brighter (0.3f), c.x - r * 0.5f, c.y - r * 0.5f,
                                       body.darker (0.6f), c.x + r, c.y + r, true));
    g.fillEllipse (c.x - r, c.y - r, 2.0f * r, 2.0f * r);
    g.setColour (Colours::black.withAlpha (0.85f));
    g.drawLine (c.x, c.y, c.x + r * 0.45f, c.y - r * 0.8f, 1.2f);
}

// idx: 0 = cabeçote slim solid-state, 1 = cabeçote valvulado classico, 2 = cabeçote slim com janela de valvula
inline void ampHead (Graphics& g, Rectangle<float> area, int idx)
{
    const float w = area.getWidth();
    const float h = (idx == 1) ? jmin (area.getHeight(), w * 0.66f) : w * 0.36f;
    auto body = Rectangle<float> (w, h).withCentre (area.getCentre());

    g.setColour (Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (body.translated (2.0f, 3.0f), 5.0f);

    g.setGradientFill (ColourGradient (Colour (0xff36373c), body.getX(), body.getY(),
                                       Colour (0xff141517), body.getX(), body.getBottom(), false));
    g.fillRoundedRectangle (body, 5.0f);
    g.setColour (Colour (0xff6a6d74));
    g.drawRoundedRectangle (body, 5.0f, 1.0f);

    if (idx == 0)
    {
        g.setColour (Colour (0xff3aa0ff));
        g.fillEllipse (body.getX() + 8.0f, body.getY() + 7.0f, 5.0f, 5.0f);
        g.setColour (Colour (0xff23242a));
        g.fillRect (body.getX() + 18.0f, body.getY() + 8.0f, body.getWidth() - 36.0f, 3.0f);
        const int n = 9;
        for (int i = 0; i < n; ++i)
            knobDot (g, { body.getX() + body.getWidth() * (i + 1.0f) / (n + 1.0f), body.getCentreY() + 5.0f }, 4.2f);
    }
    else if (idx == 1)
    {
        auto face = body.reduced (5.0f);
        auto grille = face.removeFromTop (face.getHeight() * 0.58f);
        g.setGradientFill (ColourGradient (Colour (0xffb9bdc3), grille.getX(), grille.getY(),
                                           Colour (0xff767a82), grille.getX(), grille.getBottom(), false));
        g.fillRoundedRectangle (grille, 3.0f);
        g.setColour (Colours::black.withAlpha (0.28f));
        for (float x = grille.getX() + 3.0f; x < grille.getRight() - 1.0f; x += 3.0f)
            g.drawVerticalLine ((int) x, grille.getY() + 1.0f, grille.getBottom() - 1.0f);
        g.setColour (Colour (0xff9a9ea6));
        g.drawRoundedRectangle (grille, 3.0f, 1.0f);

        const int n = 7;
        for (int i = 0; i < n; ++i)
            knobDot (g, { face.getX() + face.getWidth() * (i + 1.0f) / (n + 2.0f), face.getCentreY() + 3.0f }, 5.6f, Colour (0xfff0f0f0));
        g.setColour (Colour (0xffe04040));
        g.fillEllipse (face.getRight() - 16.0f, face.getCentreY() - 2.0f, 5.0f, 5.0f);
        g.setColour (Colour (0xffe0a040));
        g.fillEllipse (face.getRight() - 8.0f, face.getCentreY() - 2.0f, 5.0f, 5.0f);
    }
    else
    {
        g.setColour (Colour (0xffc0392b));
        g.fillRect (body.getX() + 6.0f, body.getY() + 5.0f, body.getWidth() - 12.0f, 2.5f);

        const float r = h * 0.27f;
        const Point<float> tc { body.getX() + h * 0.62f, body.getCentreY() + 3.0f };
        g.setColour (Colour (0xff0a0a0b));
        g.fillEllipse (tc.x - r - 2.0f, tc.y - r - 2.0f, 2.0f * r + 4.0f, 2.0f * r + 4.0f);
        g.setGradientFill (ColourGradient (Colour (0xffffb347), tc.x, tc.y, Colour (0xff3a1a05), tc.x + r, tc.y + r, true));
        g.fillEllipse (tc.x - r, tc.y - r, 2.0f * r, 2.0f * r);

        const int n = 6;
        const float x0 = body.getX() + h * 1.3f;
        for (int i = 0; i < n; ++i)
            knobDot (g, { x0 + (body.getRight() - 10.0f - x0) * (float) i / (n - 1.0f), body.getCentreY() + 4.0f }, 4.2f);
    }
}

// idx: 0 = 1x15, 1 = 4x10, 2 = 8x10
inline void cabinet (Graphics& g, Rectangle<float> area, int idx)
{
    const float aspect = (idx == 2) ? 0.56f : 0.95f;   // largura / altura
    float h = area.getHeight();
    float w = h * aspect;
    if (w > area.getWidth()) { w = area.getWidth(); h = w / aspect; }
    auto box = Rectangle<float> (w, h).withCentre (area.getCentre());

    g.setColour (Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (box.translated (2.0f, 3.0f), 3.0f);
    g.setColour (Colour (0xff1a1a1c));
    g.fillRoundedRectangle (box, 3.0f);
    g.setColour (Colour (0xff5d6066));
    g.drawRoundedRectangle (box, 3.0f, 1.0f);

    auto grille = box.reduced (w * 0.06f);
    g.setGradientFill (ColourGradient (Colour (0xff3b3c40), grille.getX(), grille.getY(),
                                       Colour (0xff232427), grille.getX(), grille.getBottom(), false));
    g.fillRoundedRectangle (grille, 2.0f);

    const int cols = (idx == 0) ? 1 : 2;
    const int rows = (idx == 0) ? 1 : (idx == 1 ? 2 : 4);
    const float cw = grille.getWidth() / (float) cols;
    const float ch = grille.getHeight() / (float) rows;
    const float r = jmin (cw, ch) * 0.44f;

    for (int row = 0; row < rows; ++row)
        for (int col = 0; col < cols; ++col)
        {
            const Point<float> c { grille.getX() + cw * (col + 0.5f), grille.getY() + ch * (row + 0.5f) };
            g.setColour (Colours::black.withAlpha (0.6f));
            g.fillEllipse (c.x - r, c.y - r, 2.0f * r, 2.0f * r);
            g.setGradientFill (ColourGradient (Colour (0xff7a7d85), c.x, c.y, Colour (0xff2c2d31), c.x + r, c.y + r, true));
            g.fillEllipse (c.x - r * 0.88f, c.y - r * 0.88f, r * 1.76f, r * 1.76f);
            g.setColour (Colour (0xff15161a));
            g.fillEllipse (c.x - r * 0.28f, c.y - r * 0.28f, r * 0.56f, r * 0.56f);
            g.setColour (Colour (0xff8c9097));
            g.drawEllipse (c.x - r, c.y - r, 2.0f * r, 2.0f * r, 1.2f);
        }

    g.setColour (Colours::black.withAlpha (0.18f));
    for (float y = grille.getY() + 2.0f; y < grille.getBottom(); y += 3.0f)
        g.drawHorizontalLine ((int) y, grille.getX(), grille.getRight());
}

// idx: 0 = R84 (ribbon), 1 = C414 (condensador), 2 = PR40 (dinamico). Desenhado na horizontal, apontando para a esquerda.
inline void microphone (Graphics& g, Rectangle<float> area, int idx)
{
    const float cy = area.getCentreY();
    const float x0 = area.getX() + 6.0f;
    const float x1 = area.getRight() - 6.0f;
    const float len = x1 - x0;

    auto fillBody = [&g] (Rectangle<float> r, Colour c1, Colour c2, float corner)
    {
        g.setGradientFill (ColourGradient (c1, r.getX(), r.getY(), c2, r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, corner);
        g.setColour (Colours::black.withAlpha (0.5f));
        g.drawRoundedRectangle (r, corner, 1.0f);
    };

    if (idx == 0)
    {
        Rectangle<float> head (x0, cy - 36.0f, 52.0f, 72.0f);
        fillBody (head, Colour (0xffa5abb3), Colour (0xff50545b), 8.0f);
        g.setColour (Colours::black.withAlpha (0.35f));
        for (float y = head.getY() + 6.0f; y < head.getBottom() - 4.0f; y += 5.0f)
            g.drawHorizontalLine ((int) y, head.getX() + 5.0f, head.getRight() - 5.0f);

        fillBody ({ x0 + 52.0f, cy - 18.0f, 20.0f, 36.0f }, Colour (0xff35363a), Colour (0xff1b1c1e), 3.0f);
        fillBody ({ x0 + 72.0f, cy - 22.0f, len - 72.0f - 14.0f, 44.0f }, Colour (0xff2b2c30), Colour (0xff111214), 6.0f);
        g.setColour (Colour (0xffc9a24a));
        g.fillRect (x0 + 80.0f, cy - 22.0f, 4.0f, 44.0f);
    }
    else if (idx == 1)
    {
        Rectangle<float> head (x0, cy - 34.0f, 48.0f, 68.0f);
        fillBody (head, Colour (0xffe0cf9f), Colour (0xff8a7a4c), 22.0f);
        {
            Graphics::ScopedSaveState ss (g);
            Path p; p.addRoundedRectangle (head, 22.0f);
            g.reduceClipRegion (p, {});
            g.setColour (Colours::black.withAlpha (0.35f));
            for (float x = head.getX() + 4.0f; x < head.getRight(); x += 5.0f)
                g.drawVerticalLine ((int) x, head.getY(), head.getBottom());
            for (float y = head.getY() + 4.0f; y < head.getBottom(); y += 5.0f)
                g.drawHorizontalLine ((int) y, head.getX(), head.getRight());
        }
        fillBody ({ x0 + 48.0f, cy - 26.0f, len - 48.0f - 14.0f, 52.0f }, Colour (0xff50545c), Colour (0xff25272b), 4.0f);
        g.setColour (Colour (0xff9aa0a8));
        g.fillRect (x0 + 60.0f, cy - 26.0f, 3.0f, 52.0f);
        g.setColour (Colour (0xffe0e0e0));
        for (int i = 0; i < 3; ++i)
            g.fillRect (x0 + 74.0f + 8.0f * (float) i, cy - 4.0f, 4.0f, 8.0f);
    }
    else
    {
        Rectangle<float> head (x0, cy - 30.0f, 70.0f, 60.0f);
        fillBody (head, Colour (0xff2a2b2f), Colour (0xff0e0e10), 14.0f);
        g.setColour (Colour (0xffb8bcc3).withAlpha (0.4f));
        for (float x = head.getX() + 5.0f; x < head.getRight() - 3.0f; x += 4.0f)
            g.drawVerticalLine ((int) x, head.getY() + 4.0f, head.getBottom() - 4.0f);
        fillBody ({ x0 + 70.0f, cy - 20.0f, len - 70.0f - 14.0f, 40.0f }, Colour (0xff3a3b3f), Colour (0xff151517), 6.0f);
        g.setColour (Colour (0xffc0c4cb));
        g.fillRect (x0 + 70.0f, cy - 20.0f, 3.0f, 40.0f);
    }

    // conector XLR
    g.setColour (Colour (0xff17181a));
    g.fillRoundedRectangle (x1 - 14.0f, cy - 8.0f, 14.0f, 16.0f, 2.0f);
}

// Marcador do microfone sobre o falante / na regua
inline void micMarker (Graphics& g, Point<float> c, float r)
{
    g.setColour (Colour (0xfff0a030).withAlpha (0.28f));
    g.fillEllipse (c.x - r * 1.8f, c.y - r * 1.8f, r * 3.6f, r * 3.6f);
    g.setColour (Colour (0xff111214));
    g.fillEllipse (c.x - r - 1.5f, c.y - r - 1.5f, 2.0f * r + 3.0f, 2.0f * r + 3.0f);
    g.setColour (Colour (0xfff0a030));
    g.fillEllipse (c.x - r, c.y - r, 2.0f * r, 2.0f * r);
    g.setColour (Colour (0xff111214));
    g.fillEllipse (c.x - r * 0.35f, c.y - r * 0.35f, r * 0.7f, r * 0.7f);
}

// Falante visto de frente com as 3 zonas e o microfone na posicao escolhida. pos: 0 centro, 1 meio, 2 cone
inline void speakerFront (Graphics& g, Rectangle<float> area, int pos)
{
    const auto c = area.getCentre();
    const float R = jmin (area.getWidth(), area.getHeight()) * 0.5f;

    auto disc = [&g, &c] (float r, Colour c1, Colour c2)
    {
        g.setGradientFill (ColourGradient (c1, c.x - r * 0.4f, c.y - r * 0.4f, c2, c.x + r, c.y + r, true));
        g.fillEllipse (c.x - r, c.y - r, 2.0f * r, 2.0f * r);
    };

    disc (R,         Colour (0xff202124), Colour (0xff0d0d0f));      // aro
    disc (R * 0.94f, Colour (0xff34363a), Colour (0xff1c1d20));      // suspensao
    disc (R * 0.88f, Colour (0xff5a5e66), Colour (0xff35373c));      // cone (borda)
    disc (R * 0.62f, Colour (0xff6d717a), Colour (0xff42454b));      // meio
    disc (R * 0.28f, Colour (0xff8d919b), Colour (0xff50535a));      // centro (dust cap)

    g.setColour (Colours::white.withAlpha (0.22f));
    g.drawEllipse (c.x - R * 0.62f, c.y - R * 0.62f, R * 1.24f, R * 1.24f, 1.0f);
    g.drawEllipse (c.x - R * 0.28f, c.y - R * 0.28f, R * 0.56f, R * 0.56f, 1.0f);

    const float mx = c.x + R * (pos == 0 ? 0.0f : (pos == 1 ? 0.45f : 0.76f));
    micMarker (g, { mx, c.y }, 8.0f);
}

} // namespace art
