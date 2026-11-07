# BassRig: simulador de amp + caixa + microfone para baixo (VST3)

Plugin em C++/JUCE. 3 amps, 3 caixas, 3 microfones, posição no falante (Centro / Meio / Cone),
distância do mic (1–100 cm) com reverb de quarto crescente, e interface cinza com tabela de opções.

## Como compilar

Requisitos: CMake 3.22+, Git, internet (o CMake baixa o JUCE sozinho) e um compilador C++17.

**Windows** (Visual Studio 2022 com "Desktop development with C++"):
```
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```
O `BassRig.vst3` é copiado para `C:\Program Files\Common Files\VST3` (rode o terminal como administrador
ou copie a pasta `build\BassRig_artefacts\Release\VST3\BassRig.vst3` manualmente).

**macOS** (Xcode Command Line Tools):
```
cmake -B build -G Xcode
cmake --build build --config Release
```
Copiado para `~/Library/Audio/Plug-Ins/VST3`.

**Linux**:
```
sudo apt install build-essential cmake git libasound2-dev libjack-jackd2-dev libfreetype-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev \
  libxrender-dev libgl1-mesa-dev libcurl4-openssl-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```
Copiado para `~/.vst3`.

Também é gerado um app **Standalone** para testar sem DAW.

## Onde ajustar o timbre

Tudo está em `Source/DSP.h`:

| O que | Onde |
|---|---|
| Caráter de cada amp (ganho, frequências do EQ, saturação, compressão) | `ampVoicing()` |
| Resposta de cada caixa | `cabModel()` (lista de filtros: tipo, freq, Q, ganho em dB) |
| Resposta de cada mic e efeito proximidade | `micModel()` |
| Diferença Centro / Meio / Cone | `shelfDb`, `bodyDb`, `presDb` em `refresh()` |
| Quantidade de reverb do quarto | `kRoomWet` e os parâmetros do `juce::dsp::Reverb` em `prepare()` |
| Perda de agudos com a distância | `18000 / (1 + d/90)` em `refresh()` |

## Limitações

- Caixas e mics são **modelados com filtros**, não com IRs reais. Dá um timbre coerente, mas não é uma captura fiel.
- Reverb de quarto = Freeverb do JUCE (simples). Para um quarto mais realista, troque por uma convolução com IR de sala.
- Os valores das tabelas foram calibrados a partir de especificações e reviews, não de medições.
