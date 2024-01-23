#include <JuceHeader.h>
#include <iostream>
#include <sstream>
#include "Notes.h"
#include "Track.h"
#include "Song.h"

template<typename T>
void log(const T& value) {
    std::cout << value << std::endl;
}

template<typename T, typename... Args>
void log(const T& first, const Args&... args) {
    std::cout << first << " ";
    log(args...);
}


class DefaultSynthSound : public juce::SynthesiserSound {
public:
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};


class SampleVoice : public juce::SynthesiserVoice {
public:
    SampleVoice(const std::string& samplePath, int midiNoteForSample)
    : sampleRate(0), rootMidiNote(midiNoteForSample), isPlaying(false)
    {
        juce::File file(samplePath);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        
        if (reader.get() != nullptr) {
            sampleRate = reader->sampleRate;
            sample.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
            reader->read(&sample, 0, (int)reader->lengthInSamples, 0, true, true);
            log(sampleRate, reader->numChannels, reader->lengthInSamples, "reader details");
        }
    }
    
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        auto result = dynamic_cast<DefaultSynthSound*>(sound) != nullptr;
        log(result, "can play");
        return result;
    }
    
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound, int currentPitchWheelPosition) override {
        // Calculate pitch ratio based on root note and incoming note
        double frequencyOfNote = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        double frequencyOfRootNote = juce::MidiMessage::getMidiNoteInHertz(rootMidiNote);
        pitchRatio = frequencyOfNote / frequencyOfRootNote;
        
        // Set the position and other parameters as needed
        currentPosition = 0;
        log("starting note");
        isPlaying = true;
    }
    
    void stopNote(float velocity, bool allowTailOff) override {
        log("stopping note");
        isPlaying = false;
    }
    
    
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        if (isPlaying) {
            log("playing");
            // Fill the output buffer with your sample data
            for (int sampleIdx = startSample; sampleIdx < startSample + numSamples; ++sampleIdx) {
                if (currentPosition < sample.getNumSamples()) {
                    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
                        outputBuffer.addSample(channel, sampleIdx, sample.getSample(channel, static_cast<int>(currentPosition)) * pitchRatio);
                    }
                    currentPosition += pitchRatio;
                } else {
                    stopNote(0.0f, false);
                    break;
                }
            }
        } else {
            log("not playing");
        }
    }
    
    void pitchWheelMoved(int newValue) override {
        // Handle pitch wheel changes if needed
    }
    
    void controllerMoved(int controllerNumber, int newValue) override {
        // Handle other MIDI controllers if needed
    }
    
private:
    juce::AudioSampleBuffer sample;
    double sampleRate;
    int rootMidiNote;
    double currentPosition;
    double pitchRatio;
    bool isPlaying;
};


int main(int argc, char *argv[]) {
    
    const double bpm = 60;
    const double sampleRate = 44100.0;
    
    // Create a Synthesiser for the custom sample
    juce::Synthesiser synth;
    synth.setCurrentPlaybackSampleRate(sampleRate); // Set to the desired sample rate, e.g., 44100 Hz
    synth.setNoteStealingEnabled(true);
    for (int i = 0; i < 4; ++i) {
        // 4 potential notes played at once
        synth.addVoice(new SampleVoice("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JPIANO-C2.mp3", 48));
    }
    synth.addSound(new DefaultSynthSound());
    
   
    auto song = std::make_unique<Song>(Song(bpm, sampleRate));
    
    auto track1 = std::make_unique<Track>(Track(song.get(), &synth));

    // first number is note pitch, second is start in beats, last is duration of note
    track1->addNote(Note(57, 0.0, 1.0));
    track1->addNote(Note(40, 1.0, 2.0));
    track1->addNote(Note(48, 3.0, 1.5));
    track1->addNote(Note(43, 4.5, 3.0));
    
    song->addTrack(track1.get());
    
    juce::File outputFile("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/output.wav");
    
    song->renderToFile(outputFile);
    
    return 0;
}
