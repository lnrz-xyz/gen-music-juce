#include <JuceHeader.h>
#include <iostream>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <functional>
#include <vector>
#include <utility>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <numeric>
#include "Note.h"
#include "Song.h"
#include "Voices.h"
#include "SampleProcessor.h"
#include "RepitchingSingleInstrumentSampleProcessor.h"
#include "MultiInstrumentSampleProcessor.h"
#include "Utilities.h"
#include "WidthProcessor.h"
#include "GrooveTrackGenerator.h"
#include "ChordalGenerator.h"
#include "MelodicGenerator.h"
#include "AudioProcessingBus.h"
#include "MelodicComponentsEffectProcessor.h"
#include "NoteGenerator.h"
#include "DrumsEffectProcessor.h"

double SAMPLE_RATE = 44100.0;


// the chance of each potential subdivision being played
const std::vector<double> kickWeights = {1.0, 0.5, 0.5, 0.5, 0.7, 0.6, 0.5, 0.5};
const std::vector<double> hitWeights = {0.15, 0.15, 0.5, 0.15, 0.15, 0.15, 0.8, 0.15};

// TODO bugs: some big jumps in melodes, normalize the note ranges, compression, audio bus, move gain from synth voice to a chain, maybe even abstract out the synthesisers at this point, match output volume to input volume, multiband compression, clip right at the end of a track??, beginning and end are quieter??
int main(int argc, char *argv[]) {
    
    std::vector<unsigned char> seed;
    if (argc > 1) {
        seed = generateRandomBytes(250, argv[1]);
    } else {
        seed = generateRandomBytes(250, "the next best thing");
    }
    
    int index = 0;
    
    const double bpm = 60 + (4 * (seed[index++] % 16));
    
    const auto isMajor = static_cast<int>(seed[index++]) % 2 == 0;
    
    const auto rootStart = index++;
    const auto slice = std::vector<unsigned char>(seed.begin()+rootStart, seed.begin()+rootStart+4);
    ChordalGenerator chordalGenerator(slice, isMajor);
    const auto roots = chordalGenerator.getRoots();
    
    
    const auto chords = chordalGenerator.getChords(roots);
    
    MelodicGenerator melodyGenerator(std::vector<unsigned char>(seed.begin()+index, seed.end()), roots, chords, isMajor);

    const auto allMelodyNotes = melodyGenerator.generate();
    
    std::vector<Note> allChordNotes;
    
    
    for (const auto& chord: chords) {
        for (const auto& note: chord.getNotes()) {
            allChordNotes.push_back(note);
        }
    }

    GrooveTrackGenerator kickGenerator(0, std::vector<unsigned char>(seed.begin()+allMelodyNotes.size(), seed.end()), kickWeights, 4.0, {0.5}, {3});
    std::vector<Note> kickNotes = kickGenerator.generate();
    GrooveTrackGenerator hitGenerator(1, std::vector<unsigned char>(seed.begin()+allMelodyNotes.size()+kickNotes.size(), seed.end()), hitWeights, 4.0, {0.5}, {1});
    std::vector<Note> hitNotes = hitGenerator.generate();
    
    for (auto note : hitNotes) {
        fmt::print("start {} {} {} {}\n", note.startTimeInBeats, note.velocity, note.durationInBeats, note.midiNoteNumber);
    }
    
    // TODO the note parameter doesn't really work right
    std::shared_ptr<SampleProcessor> melodySampleProcessor = std::make_shared<RepitchingSingleInstrumentSampleProcessor>("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JPIANO-C4.mp3", 36, allMelodyNotes);
    
    std::shared_ptr<SampleProcessor> chordSampleProcessor = std::make_shared<RepitchingSingleInstrumentSampleProcessor>("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JPIANO-C2.mp3", 36, allChordNotes);
    
    std::map<int, std::string> drumSamples = {{0, "/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/samples/kick/KICK - nudy.wav"}, {1,"/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/samples/perc/PERC - stick.wav"}};
    
    std::shared_ptr<SampleProcessor> drumSampleProcessor = std::make_shared<MultiInstrumentSampleProcessor>(drumSamples);
    
    juce::Synthesiser melodySynth;
    melodySynth.setCurrentPlaybackSampleRate(SAMPLE_RATE);
    melodySynth.setNoteStealingEnabled(true);
    for (int i = 0; i < 3; ++i) {
        // 4 potential notes played at once
        melodySynth.addVoice(new SampleVoice(melodySampleProcessor, i, 1.0f));
    }
    melodySynth.addSound(new DefaultSynthSound());
    
    juce::Synthesiser chordsSynth;
    chordsSynth.setCurrentPlaybackSampleRate(SAMPLE_RATE);
    chordsSynth.setNoteStealingEnabled(true);
    for (int i = 0; i < 6; ++i) {
        chordsSynth.addVoice(new SampleVoice(chordSampleProcessor, i, 0.8f));
    }
    chordsSynth.addSound(new DefaultSynthSound());
    
    juce::Synthesiser drumSynth;
    drumSynth.setCurrentPlaybackSampleRate(SAMPLE_RATE);
    drumSynth.setNoteStealingEnabled(true);
    for (int i = 0; i < 4; ++i) {
        drumSynth.addVoice(new SampleVoice(drumSampleProcessor, i, 0.5f));
    }
    drumSynth.addSound(new DefaultSynthSound());
    
    auto melodicProcessor = MelodicComponentEffectProcessor(SAMPLE_RATE);
//    auto melodicProcessor = DrumsEffectProcessor();
    auto drumsProcessor = DrumsEffectProcessor();

    auto song = Song(bpm, SAMPLE_RATE);
    std::vector<std::pair<int, std::pair<NoteGenerator*, juce::Synthesiser*>>> noteGenerators;
    noteGenerators.push_back(std::make_pair(0, std::make_pair(&melodyGenerator, &melodySynth)));
    noteGenerators.push_back(std::make_pair(0, std::make_pair(&chordalGenerator, &chordsSynth)));
    noteGenerators.push_back(std::make_pair(1, std::make_pair(&hitGenerator, &drumSynth)));
    noteGenerators.push_back(std::make_pair(1, std::make_pair(&kickGenerator, &drumSynth)));
    
    std::map<int, AudioProcessingBus> busses;
    std::map<int, EffectProcessor*> effects;
    
    
    busses.emplace(0, AudioProcessingBus(SAMPLE_RATE));
    busses.emplace(1, AudioProcessingBus(SAMPLE_RATE));
    
    effects[0] = &melodicProcessor;
    effects[1] = &drumsProcessor;
    
    
    juce::File outputFile("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/output-v3.wav");
    juce::File midiFile("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/output-v3.midi");
    auto buffer = song.generateSong(noteGenerators, busses, effects);
    song.renderToFile(outputFile, buffer);
    std::vector<NoteGenerator*> allGenerators;
    for (auto& noteGenerator: noteGenerators) {
        allGenerators.push_back(noteGenerator.second.first);
    }
    auto midiSequence = song.generateMidi(allGenerators);
    song.renderToMidiFile(midiFile, midiSequence);
    return 0;
}

