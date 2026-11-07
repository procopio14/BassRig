#pragma once

#include "PluginProcessor.h"
#include "Illustrations.h"

// Visual dos knobs (corpo escuro, aro prateado, marcas de 0 a 10)
class KnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
};

class BassRigEditor : public juce::AudioProcessorEditor
{
public:
    explicit BassRigEditor (BassRigProcessor&);
    ~BassRigEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    // geometria
    juce::Rectangle<int> cellRect (int row, int col) const;
    juce::Rectangle<int> frontArea() const;
    juce::Rectangle<int> chipRect (int i) const;
    juce::Rectangle<int> distArea() const;
    float trackX0() const;
    float trackX1() const;
    float xForDistance (float cm) const;
    float distanceForX (float x) const;

    void paintTable (juce::Graphics&);
    void paintKnobPanel (juce::Graphics&);
    void paintPlacementPanel (juce::Graphics&);
    void paintDistance (juce::Graphics&);
    void pickZone (juce::Point<int> p);
    void setDistanceFromX (float x);

    BassRigProcessor& proc;
    KnobLookAndFeel laf;

    int ampIdx = 1, cabIdx = 1, micIdx = 2, posIdx = 1;
    float dist = 15.0f;
    bool draggingDist = false, draggingZone = false;

    juce::Slider gainSlider, bassSlider, midSlider, trebleSlider, volumeSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAtt, bassAtt, midAtt, trebleAtt, volumeAtt;

    juce::ParameterAttachment ampAtt, cabAtt, micAtt, posAtt, distAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassRigEditor)
};
