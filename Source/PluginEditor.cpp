#include "PluginEditor.h"

namespace
{
    juce::String T (const char* s) { return juce::String::fromUTF8 (s); }

    juce::Font F (float h, bool bold = false)
    {
        return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
    }

    const juce::Colour kAmber   { 0xfff0a030 };
    const juce::Colour kPanel   { 0xff2c2e32 };
    const juce::Colour kCell    { 0xff3d4045 };
    const juce::Colour kBorder  { 0xff565960 };
    const juce::Colour kTextHi  { 0xffeceef1 };
    const juce::Colour kTextLo  { 0xffa4a8b0 };

    constexpr int kRowH = 116, kLabelW = 120, kCellW = 274, kGap = 6, kTableY = 62, kTableX = 20;
    constexpr int kPanelY = 436, kPanelH = 294;
    constexpr int kRightX = 520;
}

// -----------------------------------------------------------------------------
void KnobLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                        float startAngle, float endAngle, juce::Slider&)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.0f);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();

    // marcas 0..10
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (endAngle - startAngle) * (float) i / 10.0f;
        const juce::Point<float> dir { std::sin (a), -std::cos (a) };
        g.setColour (i % 5 == 0 ? kTextHi : kTextLo);
        g.drawLine (juce::Line<float> (c + dir * (r * 0.86f), c + dir * (r * 0.98f)), i % 5 == 0 ? 2.0f : 1.2f);
    }

    const float kr = r * 0.74f;
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (c.x - kr - 2.0f, c.y - kr + 3.0f, 2.0f * kr + 4.0f, 2.0f * kr + 4.0f);
    g.setColour (juce::Colour (0xff0f1012));
    g.fillEllipse (c.x - kr, c.y - kr, 2.0f * kr, 2.0f * kr);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe4e6ea), c.x - kr * 0.5f, c.y - kr * 0.6f,
                                             juce::Colour (0xff6f737b), c.x + kr * 0.8f, c.y + kr * 0.8f, true));
    g.fillEllipse (c.x - kr * 0.88f, c.y - kr * 0.88f, kr * 1.76f, kr * 1.76f);

    const float a = startAngle + pos * (endAngle - startAngle);
    const juce::Point<float> dir { std::sin (a), -std::cos (a) };
    g.setColour (juce::Colour (0xff111214));
    g.drawLine (juce::Line<float> (c + dir * (kr * 0.25f), c + dir * (kr * 0.82f)), 3.5f);
}

// -----------------------------------------------------------------------------
BassRigEditor::BassRigEditor (BassRigProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      ampAtt  (*p.apvts.getParameter ("amp"),      [this] (float v) { ampIdx = (int) std::lround (v); repaint(); }),
      cabAtt  (*p.apvts.getParameter ("cab"),      [this] (float v) { cabIdx = (int) std::lround (v); repaint(); }),
      micAtt  (*p.apvts.getParameter ("mic"),      [this] (float v) { micIdx = (int) std::lround (v); repaint(); }),
      posAtt  (*p.apvts.getParameter ("pos"),      [this] (float v) { posIdx = (int) std::lround (v); repaint(); }),
      distAtt (*p.apvts.getParameter ("distance"), [this] (float v) { dist = v; repaint(); })
{
    auto setup = [this] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 20);
        s.setNumDecimalPlacesToDisplay (1);
        s.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        s.setLookAndFeel (&laf);
        s.setColour (juce::Slider::textBoxTextColourId, kTextHi);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (s);
    };
    setup (gainSlider); setup (bassSlider); setup (midSlider); setup (trebleSlider); setup (volumeSlider);

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    gainAtt   = std::make_unique<SA> (proc.apvts, "gain",   gainSlider);
    bassAtt   = std::make_unique<SA> (proc.apvts, "bass",   bassSlider);
    midAtt    = std::make_unique<SA> (proc.apvts, "mid",    midSlider);
    trebleAtt = std::make_unique<SA> (proc.apvts, "treble", trebleSlider);
    volumeAtt = std::make_unique<SA> (proc.apvts, "volume", volumeSlider);

    ampAtt.sendInitialUpdate();
    cabAtt.sendInitialUpdate();
    micAtt.sendInitialUpdate();
    posAtt.sendInitialUpdate();
    distAtt.sendInitialUpdate();

    setSize (1000, 740);
}

BassRigEditor::~BassRigEditor()
{
    for (auto* s : { &gainSlider, &bassSlider, &midSlider, &trebleSlider, &volumeSlider })
        s->setLookAndFeel (nullptr);
}

// ------------------------------- geometria ------------------------------------
juce::Rectangle<int> BassRigEditor::cellRect (int row, int col) const
{
    const int x = kTableX + kLabelW + kGap + col * (kCellW + kGap);
    const int y = kTableY + row * (kRowH + kGap);
    return { x, y, kCellW, kRowH };
}

juce::Rectangle<int> BassRigEditor::frontArea() const { return { kRightX + 14, kPanelY + 40, 196, 196 }; }
juce::Rectangle<int> BassRigEditor::chipRect (int i) const { return { kRightX + 14 + i * 66, kPanelY + 246, 62, 28 }; }
juce::Rectangle<int> BassRigEditor::distArea() const { return { kRightX + 226, kPanelY + 40, 222, 234 }; }

float BassRigEditor::trackX0() const { return (float) distArea().getX() + 44.0f; }
float BassRigEditor::trackX1() const { return (float) distArea().getRight() - 26.0f; }

float BassRigEditor::xForDistance (float cm) const
{
    return trackX0() + (cm - 1.0f) / 99.0f * (trackX1() - trackX0());
}

float BassRigEditor::distanceForX (float x) const
{
    return juce::jlimit (1.0f, 100.0f, 1.0f + (x - trackX0()) / (trackX1() - trackX0()) * 99.0f);
}

// ------------------------------- layout ---------------------------------------
void BassRigEditor::resized()
{
    auto place = [] (juce::Slider& s, int i)
    {
        s.setBounds (20 + i * 96, kPanelY + 62, 96, 150);
    };
    place (gainSlider, 0); place (bassSlider, 1); place (midSlider, 2);
    place (trebleSlider, 3); place (volumeSlider, 4);
}

// ------------------------------- desenho --------------------------------------
void BassRigEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3b3d41), 0.0f, 0.0f,
                                             juce::Colour (0xff232528), 0.0f, (float) getHeight(), false));
    g.fillAll();

    g.setColour (kTextHi);
    g.setFont (F (24.0f, true));
    g.drawText ("BASS RIG", 24, 12, 240, 34, juce::Justification::centredLeft);
    g.setColour (kTextLo);
    g.setFont (F (13.0f));
    g.drawText (T ("Amplificador  •  Caixa  •  Microfone  •  Sala"), 190, 16, 500, 28, juce::Justification::centredLeft);

    paintTable (g);
    paintKnobPanel (g);
    paintPlacementPanel (g);
}

void BassRigEditor::paintTable (juce::Graphics& g)
{
    const juce::String rowNames[3] = { T ("AMPLIFICADOR"), T ("CAIXA"), T ("MICROFONE") };

    const juce::String titles[3][3] = {
        { "Markbass\nLittle Mark 3", "Ampeg\nSVT", "Hartke\nLX8500" },
        { "1x15", "4x10", "8x10" },
        { "AEA R84", "AKG\nC414 XLS", "Heil PR40" } };

    const juce::String subs[3][3] = {
        { T ("Limpo, claro e transparente"), T ("Valvulado, grosso e saturado"), T ("Pré valvulado, moderno e largo") },
        { T ("Grave redondo, topo suave"), T ("Punch e definição"), T ("Geladeira: grave pesado") },
        { T ("Ribbon: quente e suave"), T ("Condensador: detalhe e brilho"), T ("Dinâmico: firme e com ataque") } };

    for (int r = 0; r < 3; ++r)
    {
        auto label = juce::Rectangle<int> (kTableX, kTableY + r * (kRowH + kGap), kLabelW, kRowH);
        g.setColour (kPanel);
        g.fillRoundedRectangle (label.toFloat(), 6.0f);
        g.setColour (kBorder);
        g.drawRoundedRectangle (label.toFloat(), 6.0f, 1.0f);
        g.setColour (kTextHi);
        g.setFont (F (13.0f, true));
        g.drawFittedText (rowNames[r], label.reduced (6), juce::Justification::centred, 2);

        const int selected = (r == 0 ? ampIdx : (r == 1 ? cabIdx : micIdx));

        for (int c = 0; c < 3; ++c)
        {
            auto cell = cellRect (r, c);
            const bool sel = (c == selected);

            g.setGradientFill (juce::ColourGradient (kCell.brighter (0.08f), 0.0f, (float) cell.getY(),
                                                     kCell.darker (0.15f), 0.0f, (float) cell.getBottom(), false));
            g.fillRoundedRectangle (cell.toFloat(), 6.0f);

            if (sel)
            {
                g.setColour (kAmber.withAlpha (0.20f));
                g.drawRoundedRectangle (cell.toFloat().expanded (2.0f), 8.0f, 4.0f);
            }
            g.setColour (sel ? kAmber : kBorder);
            g.drawRoundedRectangle (cell.toFloat(), 6.0f, sel ? 2.0f : 1.0f);

            auto img = cell.withWidth (158).reduced (6).toFloat();
            if (r == 0)      art::ampHead (g, img, c);
            else if (r == 1) art::cabinet (g, img, c);
            else             art::microphone (g, img, c);

            auto txt = cell.withTrimmedLeft (160).reduced (4, 10);
            g.setColour (sel ? kAmber : kTextHi);
            g.setFont (F (15.0f, true));
            g.drawFittedText (titles[r][c], txt.removeFromTop (44), juce::Justification::topLeft, 2);
            g.setColour (kTextLo);
            g.setFont (F (11.5f));
            g.drawFittedText (subs[r][c], txt, juce::Justification::topLeft, 4);
        }
    }
}

void BassRigEditor::paintKnobPanel (juce::Graphics& g)
{
    auto panel = juce::Rectangle<float> (20.0f, (float) kPanelY, 480.0f, (float) kPanelH);
    g.setColour (kPanel);
    g.fillRoundedRectangle (panel, 8.0f);
    g.setColour (kBorder);
    g.drawRoundedRectangle (panel, 8.0f, 1.0f);

    g.setColour (kTextHi);
    g.setFont (F (13.0f, true));
    g.drawText ("CONTROLES DO AMP", 36, kPanelY + 10, 300, 22, juce::Justification::centredLeft);

    const char* names[5] = { "GAIN", "BASS", "MID", "TREBLE", "VOLUME" };
    g.setColour (kTextLo);
    g.setFont (F (12.0f, true));
    for (int i = 0; i < 5; ++i)
        g.drawText (names[i], 20 + i * 96, kPanelY + 40, 96, 20, juce::Justification::centred);

    // resumo do rig
    const char* ampN[3] = { "Markbass Little Mark 3", "Ampeg SVT", "Hartke LX8500" };
    const char* cabN[3] = { "1x15", "4x10", "8x10" };
    const char* micN[3] = { "AEA R84", "AKG C414 XLS", "Heil PR40" };
    const char* posN[3] = { "Centro", "Meio", "Cone" };

    g.setColour (kAmber);
    g.setFont (F (13.0f, true));
    g.drawText ("SINAL", 36, kPanelY + 232, 80, 22, juce::Justification::centredLeft);
    g.setColour (kTextHi);
    g.setFont (F (13.0f));
    g.drawFittedText (juce::String (ampN[juce::jlimit (0, 2, ampIdx)]) + "  >  " + cabN[juce::jlimit (0, 2, cabIdx)]
                          + "  >  " + micN[juce::jlimit (0, 2, micIdx)] + " (" + posN[juce::jlimit (0, 2, posIdx)]
                          + ", " + juce::String (dist, 1) + " cm)",
                      juce::Rectangle<int> (100, kPanelY + 226, 392, 40), juce::Justification::centredLeft, 2);
}

void BassRigEditor::paintPlacementPanel (juce::Graphics& g)
{
    auto panel = juce::Rectangle<float> ((float) kRightX, (float) kPanelY, 460.0f, (float) kPanelH);
    g.setColour (kPanel);
    g.fillRoundedRectangle (panel, 8.0f);
    g.setColour (kBorder);
    g.drawRoundedRectangle (panel, 8.0f, 1.0f);

    g.setColour (kTextHi);
    g.setFont (F (13.0f, true));
    g.drawText (T ("POSIÇÃO DO MIC"), kRightX + 14, kPanelY + 10, 200, 22, juce::Justification::centredLeft);
    g.drawText (T ("DISTÂNCIA"), kRightX + 226, kPanelY + 10, 200, 22, juce::Justification::centredLeft);

    art::speakerFront (g, frontArea().toFloat().reduced (4.0f), posIdx);

    const char* chips[3] = { "CENTRO", "MEIO", "CONE" };
    for (int i = 0; i < 3; ++i)
    {
        auto r = chipRect (i).toFloat();
        const bool sel = (i == posIdx);
        g.setColour (sel ? kAmber : kCell);
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (sel ? kAmber : kBorder);
        g.drawRoundedRectangle (r, 5.0f, 1.0f);
        g.setColour (sel ? juce::Colour (0xff1a1b1d) : kTextHi);
        g.setFont (F (11.5f, true));
        g.drawText (chips[i], r.toNearestInt(), juce::Justification::centred);
    }

    paintDistance (g);
}

void BassRigEditor::paintDistance (juce::Graphics& g)
{
    const auto a = distArea();
    const float x0 = trackX0(), x1 = trackX1();
    const float cy = (float) a.getY() + 80.0f;

    // parede do fundo do quarto
    g.setColour (juce::Colour (0xff1e2023));
    g.fillRect ((float) a.getRight() - 18.0f, cy - 70.0f, 10.0f, 140.0f);
    g.setColour (kTextLo);
    g.setFont (F (10.0f));
    g.drawText ("parede", a.getRight() - 40, (int) cy + 72, 40, 14, juce::Justification::centredRight);

    // caixa (vista lateral)
    g.setColour (juce::Colour (0xff17181a));
    g.fillRoundedRectangle ((float) a.getX() + 2.0f, cy - 38.0f, 38.0f, 76.0f, 3.0f);
    g.setColour (kBorder);
    g.drawRoundedRectangle ((float) a.getX() + 2.0f, cy - 38.0f, 38.0f, 76.0f, 3.0f, 1.0f);
    juce::Path cone;
    cone.addTriangle (x0 - 4.0f, cy - 30.0f, x0 - 4.0f, cy + 30.0f, (float) a.getX() + 14.0f, cy);
    g.setColour (juce::Colour (0xff6d717a));
    g.fillPath (cone);

    // eixo + regua
    g.setColour (kBorder);
    g.drawLine (x0, cy, x1, cy, 1.5f);
    g.setFont (F (10.0f));
    for (int cm : { 1, 25, 50, 75, 100 })
    {
        const float x = xForDistance ((float) cm);
        g.setColour (kTextLo);
        g.drawLine (x, cy - 5.0f, x, cy + 5.0f, 1.0f);
        g.drawText (juce::String (cm), (int) x - 14, (int) cy + 8, 28, 14, juce::Justification::centred);
    }

    // microfone (marcador) + linha de captacao
    const float mx = xForDistance (dist);
    g.setColour (kAmber.withAlpha (0.45f));
    g.drawLine (x0 - 4.0f, cy, mx, cy, 2.0f);
    art::micMarker (g, { mx, cy }, 8.0f);

    // leitura
    g.setColour (kAmber);
    g.setFont (F (22.0f, true));
    g.drawText (juce::String (dist, 1) + " cm", a.getX(), a.getY() + 134, a.getWidth(), 30, juce::Justification::centred);

    // medidor de reverb do quarto
    const float wet = 1.0f - std::exp (-dist / 35.0f);
    g.setColour (kTextLo);
    g.setFont (F (11.0f, true));
    g.drawText ("REVERB DO QUARTO", a.getX() + 4, a.getY() + 176, a.getWidth() - 8, 16, juce::Justification::centredLeft);
    auto bar = juce::Rectangle<float> ((float) a.getX() + 4.0f, (float) a.getY() + 196.0f, (float) a.getWidth() - 8.0f, 12.0f);
    g.setColour (juce::Colour (0xff1a1b1d));
    g.fillRoundedRectangle (bar, 4.0f);
    g.setColour (kAmber);
    g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * wet), 4.0f);
    g.setColour (kTextLo);
    g.setFont (F (10.0f));
    g.drawText ("perto", a.getX() + 4, a.getY() + 212, 60, 14, juce::Justification::centredLeft);
    g.drawText ("longe", a.getRight() - 64, a.getY() + 212, 60, 14, juce::Justification::centredRight);
}

// ------------------------------- mouse ----------------------------------------
void BassRigEditor::pickZone (juce::Point<int> p)
{
    const auto area = frontArea().toFloat().reduced (4.0f);
    const auto c = area.getCentre();
    const float R = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f;
    const float r = juce::Point<float> ((float) p.x, (float) p.y).getDistanceFrom (c) / R;
    const int idx = (r < 0.30f) ? 0 : (r < 0.64f ? 1 : 2);
    if (idx != posIdx)
        posAtt.setValueAsCompleteGesture ((float) idx);
}

void BassRigEditor::setDistanceFromX (float x)
{
    distAtt.setValueAsPartOfGesture (distanceForX (x));
}

void BassRigEditor::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.getPosition();

    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (cellRect (r, c).contains (p))
            {
                (r == 0 ? ampAtt : (r == 1 ? cabAtt : micAtt)).setValueAsCompleteGesture ((float) c);
                return;
            }

    for (int i = 0; i < 3; ++i)
        if (chipRect (i).contains (p))
        {
            posAtt.setValueAsCompleteGesture ((float) i);
            return;
        }

    if (frontArea().contains (p))
    {
        draggingZone = true;
        pickZone (p);
        return;
    }

    const auto a = distArea();
    if (juce::Rectangle<int> (a.getX(), a.getY() + 20, a.getWidth(), 120).contains (p))
    {
        draggingDist = true;
        distAtt.beginGesture();
        setDistanceFromX ((float) p.x);
    }
}

void BassRigEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (draggingDist)      setDistanceFromX ((float) e.getPosition().x);
    else if (draggingZone) pickZone (e.getPosition());
}

void BassRigEditor::mouseUp (const juce::MouseEvent&)
{
    if (draggingDist) distAtt.endGesture();
    draggingDist = false;
    draggingZone = false;
}
