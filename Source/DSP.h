#pragma once

#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

// =============================================================================
//  BassRig DSP
//  Cadeia: Entrada -> Amp (voicing + gain + saturacao 4x + tone stack + master/power)
//          -> Caixa (filtros) -> Posicao no falante -> Microfone (+ proximidade)
//          -> Distancia (nivel + perda de agudos + reverb de quarto)
//
//  TODOS os numeros marcados com "AJUSTE" sao pontos de partida baseados em
//  especificacoes publicas e reviews. Mexa neles de ouvido para chegar no timbre.
// =============================================================================
namespace bassdsp
{
using Coeffs = juce::dsp::IIR::Coefficients<float>;

enum class FT { Off, HP, LP, Peak, LowShelf, HighShelf };
struct FSpec { FT type; float freq; float q; float gainDb; };

inline float dbToGain (float db) { return std::pow (10.0f, db * 0.05f); }

inline Coeffs::Ptr identityCoeffs() { return new Coeffs (1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f); }

inline Coeffs::Ptr makeCoeffs (double sr, const FSpec& s)
{
    const float f = juce::jlimit (15.0f, (float) (sr * 0.45), s.freq);
    switch (s.type)
    {
        case FT::HP:        return Coeffs::makeHighPass (sr, f, s.q);
        case FT::LP:        return Coeffs::makeLowPass (sr, f, s.q);
        case FT::Peak:      return Coeffs::makePeakFilter (sr, f, s.q, dbToGain (s.gainDb));
        case FT::LowShelf:  return Coeffs::makeLowShelf (sr, f, s.q, dbToGain (s.gainDb));
        case FT::HighShelf: return Coeffs::makeHighShelf (sr, f, s.q, dbToGain (s.gainDb));
        case FT::Off:       break;
    }
    return identityCoeffs();
}

// Biquad mono simples
class Biquad
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        filter.prepare (spec);
        filter.coefficients = identityCoeffs();
        filter.reset();
    }
    void reset() { filter.reset(); }
    void set (Coeffs::Ptr c) { filter.coefficients = c; }
    void process (float* data, int n)
    {
        float* ch[1] = { data };
        juce::dsp::AudioBlock<float> block (ch, 1, (size_t) n);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        filter.process (ctx);
    }

private:
    juce::dsp::IIR::Filter<float> filter;
};

// -----------------------------------------------------------------------------
//  AMPS
// -----------------------------------------------------------------------------
struct AmpVoicing
{
    const char* name;
    float gainMinDb, gainMaxDb, preDrive;                 // faixa do knob GAIN
    float bassHz, midHz, midQ, trebleHz, toneRangeDb;     // tone stack (+/- dB)
    float lowShelfHz, lowShelfDb;                         // voicing fixo
    float midBumpHz, midBumpQ, midBumpDb;
    float highShelfHz, highShelfDb;
    float bias;          // assimetria da valvula/transistor (0 = simetrico)
    int   stages;        // 1 ou 2 estagios de saturacao
    float powerDrive;    // 0 = power amp limpo (classe D), 1 = valvulado saturando
    float compAmount;    // compressao/sag
    float outLpHz;       // corte de agudos do estagio de saida
    float levelTrim;
};

inline const AmpVoicing& ampVoicing (int i)
{
    static const AmpVoicing v[3] = {
        // Markbass Little Mark 3: solid-state, limpo e transparente, mid "voicing" que corta no mix.
        { "Markbass Little Mark 3",
          -6.0f, 18.0f, 1.0f,
          40.0f, 500.0f, 0.60f, 6500.0f, 12.0f,
          70.0f, 1.5f,
          800.0f, 0.80f, 2.0f,
          7000.0f, 0.5f,
          0.05f, 1, 0.25f, 0.0f, 17000.0f, 1.0f },

        // Ampeg SVT: valvulado, grosso, low-mids cheios, satura e comprime (sag).
        { "Ampeg SVT",
          -6.0f, 24.0f, 1.5f,
          40.0f, 800.0f, 0.70f, 4000.0f, 12.0f,
          90.0f, 3.0f,
          250.0f, 0.90f, 2.0f,
          5000.0f, -1.5f,
          0.35f, 2, 1.0f, 0.30f, 11000.0f, 0.85f },

        // Hartke LX8500: pre valvulado 12AX7 + power classe D, grave largo, topo moderno, compressor leve.
        { "Hartke LX8500",
          -6.0f, 22.0f, 1.3f,
          50.0f, 600.0f, 0.70f, 4500.0f, 12.0f,
          60.0f, 2.5f,
          3000.0f, 0.80f, 1.5f,
          8000.0f, 1.5f,
          0.20f, 1, 0.40f, 0.55f, 16000.0f, 0.95f }
    };
    return v[juce::jlimit (0, 2, i)];
}

// -----------------------------------------------------------------------------
//  CAIXAS (sem IR: filtros que imitam resposta de falante + gabinete)
// -----------------------------------------------------------------------------
struct CabModel { const char* name; std::vector<FSpec> filters; float trim; };

inline const CabModel& cabModel (int i)
{
    // LP de 4a ordem = dois biquads com Q 0.5412 e 1.3066
    static const CabModel m[3] = {
        { "1x15", {
            { FT::HP,   45.0f,   0.80f,  0.0f },
            { FT::Peak, 70.0f,   1.50f,  3.5f },
            { FT::Peak, 380.0f,  0.90f, -2.0f },
            { FT::Peak, 1250.0f, 1.10f,  2.5f },
            { FT::Peak, 2300.0f, 2.00f, -3.0f },
            { FT::LP,   3400.0f, 0.5412f, 0.0f },
            { FT::LP,   3400.0f, 1.3066f, 0.0f } }, 1.10f },

        { "4x10", {
            { FT::HP,   55.0f,   0.80f,  0.0f },
            { FT::Peak, 90.0f,   1.40f,  2.5f },
            { FT::Peak, 330.0f,  1.00f, -3.5f },
            { FT::Peak, 800.0f,  0.90f,  1.5f },
            { FT::Peak, 2200.0f, 1.30f,  3.0f },
            { FT::Peak, 3800.0f, 2.50f, -2.0f },
            { FT::LP,   5200.0f, 0.5412f, 0.0f },
            { FT::LP,   5200.0f, 1.3066f, 0.0f } }, 1.00f },

        { "8x10", {
            { FT::HP,   38.0f,   0.80f,  0.0f },
            { FT::Peak, 58.0f,   1.50f,  4.0f },
            { FT::Peak, 240.0f,  0.90f, -2.5f },
            { FT::Peak, 700.0f,  0.90f,  2.0f },
            { FT::Peak, 1100.0f, 3.00f, -2.0f },
            { FT::Peak, 2000.0f, 1.10f,  2.0f },
            { FT::LP,   4800.0f, 0.5412f, 0.0f },
            { FT::LP,   4800.0f, 1.3066f, 0.0f } }, 1.00f }
    };
    return m[juce::jlimit (0, 2, i)];
}

// -----------------------------------------------------------------------------
//  MICROFONES
// -----------------------------------------------------------------------------
struct MicModel
{
    const char* name;
    std::vector<FSpec> filters;
    float proximity;    // 0..1  quanto de efeito proximidade (graves ao chegar perto)
    float proxMaxDb;
    float trim;
};

inline const MicModel& micModel (int i)
{
    static const MicModel m[3] = {
        // AEA R84: ribbon, escuro/suave, forte efeito proximidade (figura-8).
        { "AEA R84", {
            { FT::HP,        35.0f,    0.70f,  0.0f },
            { FT::LowShelf,  180.0f,   0.70f,  1.5f },
            { FT::HighShelf, 6500.0f,  0.70f, -3.5f },
            { FT::LP,        12000.0f, 0.60f,  0.0f } }, 1.00f, 10.0f, 1.00f },

        // AKG C414 XLS: condensador, detalhe e ar, presenca elevada.
        { "AKG C414 XLS", {
            { FT::HP,        30.0f,    0.70f,  0.0f },
            { FT::Peak,      250.0f,   1.00f, -1.0f },
            { FT::Peak,      4500.0f,  0.80f,  2.0f },
            { FT::HighShelf, 9000.0f,  0.70f,  2.5f } }, 0.55f, 6.0f, 0.95f },

        // Heil PR40: dinamico, grave firme e tenso, pico de ataque nos 4-5 kHz.
        { "Heil PR40", {
            { FT::HP,        28.0f,    0.70f,  0.0f },
            { FT::LowShelf,  80.0f,    0.70f,  1.0f },
            { FT::Peak,      250.0f,   1.00f, -2.5f },
            { FT::Peak,      4800.0f,  0.90f,  3.5f },
            { FT::HighShelf, 12000.0f, 0.70f, -2.0f } }, 0.70f, 7.0f, 1.00f }
    };
    return m[juce::jlimit (0, 2, i)];
}

// -----------------------------------------------------------------------------
//  ENGINE
// -----------------------------------------------------------------------------
class BassEngine
{
public:
    struct Settings
    {
        int   amp = 1, cab = 1, mic = 1, pos = 1;   // pos: 0 centro, 1 meio, 2 cone
        float gain = 5.0f, bass = 5.0f, mid = 5.0f, treble = 5.0f, volume = 5.0f;
        float distanceCm = 15.0f;
    };

    void prepare (double sr, int maxBlock)
    {
        sampleRate   = sr;
        maxBlockSize = maxBlock;

        work.assign ((size_t) maxBlock, 0.0f);
        revL.assign ((size_t) maxBlock, 0.0f);
        revR.assign ((size_t) maxBlock, 0.0f);

        const juce::dsp::ProcessSpec mono   { sr, (juce::uint32) maxBlock, 1 };
        const juce::dsp::ProcessSpec stereo { sr, (juce::uint32) maxBlock, 2 };

        for (auto& b : ampFilters) b.prepare (mono);
        for (auto& b : cabFilters) b.prepare (mono);
        for (auto& b : micFilters) b.prepare (mono);
        for (auto& b : posFilters) b.prepare (mono);
        proxFilter.prepare (mono);
        distLp.prepare (mono);
        revHP.prepare (mono);
        revLP.prepare (mono);

        oversampling.initProcessing ((size_t) maxBlock);

        reverb.prepare (stereo);
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize   = 0.30f;   // quarto pequeno
        rp.damping    = 0.60f;
        rp.wetLevel   = 0.33f;   // o Freeverb multiplica o wet por 3 internamente
        rp.dryLevel   = 0.0f;
        rp.width      = 1.0f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        atkCoef = 1.0f - std::exp (-1.0f / (0.004f * (float) sr));
        relCoef = 1.0f - std::exp (-1.0f / (0.150f * (float) sr));

        revHP.set (makeCoeffs (sr, { FT::HP, 150.0f, 0.707f, 0.0f }));
        revLP.set (makeCoeffs (sr, { FT::LP, 6000.0f, 0.707f, 0.0f }));

        invalidateCache();
        reset();
    }

    void reset()
    {
        for (auto& b : ampFilters) b.reset();
        for (auto& b : cabFilters) b.reset();
        for (auto& b : micFilters) b.reset();
        for (auto& b : posFilters) b.reset();
        proxFilter.reset();
        distLp.reset();
        revHP.reset();
        revLP.reset();
        oversampling.reset();
        reverb.reset();
        env = 0.0f;
        gainPrev = -1.0f;
        masterPrev = -1.0f;
    }

    int getLatency() const { return (int) std::ceil (oversampling.getLatencyInSamples()); }

    // in: mono. outL/outR: saida estereo. n <= maxBlock.
    void process (const float* in, float* outL, float* outR, int n, const Settings& s)
    {
        if (n <= 0) return;
        n = std::min (n, maxBlockSize);

        refresh (s);

        float* x = work.data();
        std::copy (in, in + n, x);

        const AmpVoicing& a = ampVoicing (s.amp);

        // 1) Entrada + voicing fixo do amp
        ampFilters[inHP].process (x, n);
        ampFilters[vLow].process (x, n);
        ampFilters[vMid].process (x, n);
        ampFilters[vHigh].process (x, n);

        // 2) Gain
        const float gDb = a.gainMinDb + (s.gain / 10.0f) * (a.gainMaxDb - a.gainMinDb);
        const float gTarget = dbToGain (gDb) * a.preDrive;
        if (gainPrev < 0.0f) gainPrev = gTarget;
        ramp (x, n, gainPrev, gTarget);
        gainPrev = gTarget;

        // 3) Saturacao de pre-amp (4x oversampling)
        {
            float* ch[1] = { x };
            juce::dsp::AudioBlock<float> block (ch, 1, (size_t) n);
            auto up = oversampling.processSamplesUp (block);
            float* u = up.getChannelPointer (0);
            const size_t un = up.getNumSamples();

            const float bias = a.bias;
            const float biasT = std::tanh (bias);
            const float st2T = std::tanh (0.2f);
            const bool two = a.stages > 1;

            for (size_t i = 0; i < un; ++i)
            {
                float v = std::tanh (u[i] + bias) - biasT;
                if (two) v = std::tanh (1.5f * v + 0.2f) - st2T;
                u[i] = v;
            }
            oversampling.processSamplesDown (block);
        }

        ampFilters[dcBlock].process (x, n);

        // 4) Tone stack
        ampFilters[tBass].process (x, n);
        ampFilters[tMid].process (x, n);
        ampFilters[tTreble].process (x, n);

        // 5) Compressao / sag
        compress (x, n, a.compAmount);

        // 6) Master (empurra o power amp) + power amp
        const float vol = s.volume;
        const float mDb = (vol >= 5.0f) ? (vol - 5.0f) * 2.4f : (vol - 5.0f) * 8.0f;
        const float mTarget = (vol < 0.05f) ? 0.0f : dbToGain (mDb);
        if (masterPrev < 0.0f) masterPrev = mTarget;
        ramp (x, n, masterPrev, mTarget);
        masterPrev = mTarget;

        const float w = a.powerDrive;
        for (int i = 0; i < n; ++i)
            x[i] = (1.0f - w) * x[i] + w * std::tanh (x[i] * 1.2f);

        ampFilters[outLP].process (x, n);

        // 7) Caixa + posicao + microfone
        for (auto& f : cabFilters) f.process (x, n);
        for (auto& f : posFilters) f.process (x, n);
        for (auto& f : micFilters) f.process (x, n);
        proxFilter.process (x, n);
        distLp.process (x, n);

        const float trim = a.levelTrim * cabModel (s.cab).trim * micModel (s.mic).trim * 0.8f;

        // 8) Distancia: nivel direto + reverb de quarto
        const float d = s.distanceCm;
        const float direct = (1.0f + 2.0f / 30.0f) / (1.0f + d / 30.0f);
        const float makeup = 1.0f / std::sqrt (direct);
        const float wetAmt = kRoomWet * (1.0f - std::exp (-d / 35.0f)) * makeup;
        const float dryAmt = direct * makeup * trim;

        for (int i = 0; i < n; ++i) revL[(size_t) i] = x[i] * trim;
        revHP.process (revL.data(), n);
        revLP.process (revL.data(), n);
        std::copy (revL.begin(), revL.begin() + n, revR.begin());

        {
            float* rch[2] = { revL.data(), revR.data() };
            juce::dsp::AudioBlock<float> rb (rch, 2, (size_t) n);
            juce::dsp::ProcessContextReplacing<float> ctx (rb);
            reverb.process (ctx);
        }

        for (int i = 0; i < n; ++i)
        {
            const float dry = x[i] * dryAmt;
            outL[i] = std::tanh (dry + revL[(size_t) i] * wetAmt);   // tanh final = limitador suave
            outR[i] = std::tanh (dry + revR[(size_t) i] * wetAmt);
        }
    }

private:
    // AJUSTE: nivel maximo do reverb de quarto (se ficar alto/baixo demais, mude aqui)
    static constexpr float kRoomWet = 0.30f;

    enum AmpSlot { inHP, vLow, vMid, vHigh, dcBlock, tBass, tMid, tTreble, outLP, numAmpSlots };

    static void ramp (float* d, int n, float g0, float g1)
    {
        if (std::abs (g1 - g0) < 1.0e-7f)
        {
            for (int i = 0; i < n; ++i) d[i] *= g1;
            return;
        }
        const float step = (g1 - g0) / (float) n;
        float g = g0;
        for (int i = 0; i < n; ++i) { d[i] *= g; g += step; }
    }

    void compress (float* x, int n, float amount)
    {
        if (amount <= 0.0f) return;
        for (int i = 0; i < n; ++i)
        {
            const float a = std::abs (x[i]);
            env += (a > env ? atkCoef : relCoef) * (a - env);
            const float gr = 1.0f / (1.0f + amount * 1.5f * env);
            x[i] *= gr * (1.0f + amount * 0.5f);
        }
    }

    void invalidateCache()
    {
        lastAmp = lastCab = lastMic = lastPos = -1;
        lastBass = lastMid = lastTreble = -1000.0f;
        lastDist = -1000.0f;
    }

    static void setSlots (std::array<Biquad, 8>& slots, const std::vector<FSpec>& specs, double sr)
    {
        for (size_t i = 0; i < slots.size(); ++i)
            slots[i].set (i < specs.size() ? makeCoeffs (sr, specs[i]) : identityCoeffs());
    }

    void refresh (const Settings& s)
    {
        const bool ampChanged = (s.amp != lastAmp);
        const bool micChanged = (s.mic != lastMic);

        if (ampChanged)
        {
            const auto& a = ampVoicing (s.amp);
            ampFilters[inHP].set    (makeCoeffs (sampleRate, { FT::HP, 28.0f, 0.707f, 0.0f }));
            ampFilters[vLow].set    (makeCoeffs (sampleRate, { FT::LowShelf, a.lowShelfHz, 0.7f, a.lowShelfDb }));
            ampFilters[vMid].set    (makeCoeffs (sampleRate, { FT::Peak, a.midBumpHz, a.midBumpQ, a.midBumpDb }));
            ampFilters[vHigh].set   (makeCoeffs (sampleRate, { FT::HighShelf, a.highShelfHz, 0.7f, a.highShelfDb }));
            ampFilters[dcBlock].set (makeCoeffs (sampleRate, { FT::HP, 18.0f, 0.707f, 0.0f }));
            ampFilters[outLP].set   (makeCoeffs (sampleRate, { FT::LP, a.outLpHz, 0.707f, 0.0f }));
            lastAmp = s.amp;
            lastBass = lastMid = lastTreble = -1000.0f;   // forca atualizar o tone stack
        }

        if (s.bass != lastBass || s.mid != lastMid || s.treble != lastTreble)
        {
            const auto& a = ampVoicing (s.amp);
            auto dbFor = [&a] (float k) { return (k - 5.0f) / 5.0f * a.toneRangeDb; };
            ampFilters[tBass].set   (makeCoeffs (sampleRate, { FT::LowShelf,  a.bassHz,   0.7f,   dbFor (s.bass) }));
            ampFilters[tMid].set    (makeCoeffs (sampleRate, { FT::Peak,      a.midHz,    a.midQ, dbFor (s.mid) }));
            ampFilters[tTreble].set (makeCoeffs (sampleRate, { FT::HighShelf, a.trebleHz, 0.7f,   dbFor (s.treble) }));
            lastBass = s.bass; lastMid = s.mid; lastTreble = s.treble;
        }

        if (s.cab != lastCab)
        {
            setSlots (cabFilters, cabModel (s.cab).filters, sampleRate);
            lastCab = s.cab;
        }

        if (micChanged)
        {
            setSlots (micFilters, micModel (s.mic).filters, sampleRate);
            lastMic = s.mic;
        }

        if (s.pos != lastPos)
        {
            // 0 = centro (mais brilho/ataque), 1 = meio (neutro), 2 = cone (mais escuro e corpudo)
            static const float shelfDb[3] = {  4.0f, 0.0f, -5.0f };
            static const float bodyDb[3]  = { -1.5f, 0.0f,  2.5f };
            static const float presDb[3]  = {  2.5f, 0.0f, -1.0f };
            const int p = juce::jlimit (0, 2, s.pos);
            posFilters[0].set (makeCoeffs (sampleRate, { FT::HighShelf, 2800.0f, 0.7f, shelfDb[p] }));
            posFilters[1].set (makeCoeffs (sampleRate, { FT::Peak,      200.0f,  0.8f, bodyDb[p] }));
            posFilters[2].set (makeCoeffs (sampleRate, { FT::Peak,      4500.0f, 1.0f, presDb[p] }));
            lastPos = s.pos;
        }

        if (micChanged || std::abs (s.distanceCm - lastDist) > 0.02f)
        {
            const auto& m = micModel (s.mic);
            const float d = s.distanceCm;
            const float proxDb = m.proximity * m.proxMaxDb * std::exp (-d / 15.0f);
            proxFilter.set (makeCoeffs (sampleRate, { FT::LowShelf, 170.0f, 0.7f, proxDb }));
            const float lp = 18000.0f / (1.0f + d / 90.0f);   // perda de agudos com a distancia
            distLp.set (makeCoeffs (sampleRate, { FT::LP, lp, 0.707f, 0.0f }));
            lastDist = d;
        }
    }

    double sampleRate = 44100.0;
    int maxBlockSize = 512;

    std::array<Biquad, numAmpSlots> ampFilters;
    std::array<Biquad, 8> cabFilters;
    std::array<Biquad, 8> micFilters;
    std::array<Biquad, 3> posFilters;
    Biquad proxFilter, distLp, revHP, revLP;

    juce::dsp::Oversampling<float> oversampling { 1, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false };
    juce::dsp::Reverb reverb;

    std::vector<float> work, revL, revR;

    float env = 0.0f, atkCoef = 0.01f, relCoef = 0.001f;
    float gainPrev = -1.0f, masterPrev = -1.0f;

    int lastAmp = -1, lastCab = -1, lastMic = -1, lastPos = -1;
    float lastBass = -1000.0f, lastMid = -1000.0f, lastTreble = -1000.0f, lastDist = -1000.0f;
};

} // namespace bassdsp
